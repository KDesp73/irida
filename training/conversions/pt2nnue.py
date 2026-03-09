#!/usr/bin/env python3
"""
Convert a .pt model (from train.py) to Stockfish .nnue binary format.

Supports:
- small_mlp: 768->256->32->1; writes placeholder FT and padded 512->32->1.
- halfkp: HalfKP feature transformer (41024*2 x 256) + 512->32->1; writes
  real FT weights and dense layers so the .nnue is fully usable.
"""

from __future__ import annotations

import argparse
import struct
import sys
from pathlib import Path

import numpy as np

try:
    import torch
except ImportError:
    print("convert_pt_to_nnue.py requires PyTorch: pip install torch", file=sys.stderr)
    sys.exit(1)


# Stockfish nnue_common.h
NNUE_VERSION = 0x7AF32F20
LEB128_MAGIC = b"COMPRESSED_LEB128"
# Hash for a simple architecture (may need to match nnue-probe build)
NNUE_HASH = 0xEC42E90D

# Dense layer dimensions we target (nnue-probe / older Stockfish: 512 -> 32 -> 1)
FT_OUTPUT_DIM = 512
L1_OUTPUT = 32
L2_OUTPUT = 1

# Our .pt layout from train.py (mlp)
BOARD_DIM = 768
HIDDEN = 256
# HalfKP
HALFKP_FEATURES_PER_HALF = 64 * 641  # 41024
FT_OUTPUT_PER_HALF = 256


def write_uint32_le(f: object, value: int) -> None:
    f.write(struct.pack("<I", value & 0xFFFFFFFF))


def write_int16_le(f: object, value: int) -> None:
    f.write(struct.pack("<h", max(-32768, min(32767, value))))


def write_int8_le(f: object, value: int) -> None:
    f.write(struct.pack("<b", max(-128, min(127, value))))


def leb128_encode_signed(value: int) -> list[int]:
    """Encode a signed int as LEB128 bytes."""
    out = []
    while True:
        byte = value & 0x7F
        value >>= 7
        if (value == 0 and (byte & 0x40) == 0) or (value == -1 and (byte & 0x40) != 0):
            out.append(byte)
            break
        out.append(byte | 0x80)
    return out


def write_leb128_block(f: object, values: list[int]) -> None:
    """Write LEB128 magic, 4-byte byte count, then LEB128-encoded values."""
    f.write(LEB128_MAGIC)
    bytes_list = []
    for v in values:
        bytes_list.extend(leb128_encode_signed(v))
    write_uint32_le(f, len(bytes_list))
    f.write(bytes(bytes_list))


def quantize_scale(w: float, scale_bits: int = 6) -> int:
    """Scale float weight to int16 (Stockfish uses WeightScaleBits=6)."""
    return int(round(w * (1 << scale_bits)))


def load_pt(path: str) -> tuple[dict, str]:
    """Load .pt; return (state_dict, arch). arch is 'small_mlp' or 'halfkp'."""
    data = torch.load(path, map_location="cpu", weights_only=False)
    if isinstance(data, dict) and "state_dict" in data:
        sd = data["state_dict"]
        arch = data.get("arch", "small_mlp")
    else:
        sd = data if isinstance(data, dict) else {}
        arch = "small_mlp"
    if arch == "halfkp":
        if "ft_white.weight" not in sd or "fc2.weight" not in sd or "fc3.weight" not in sd:
            raise ValueError(
                "Expected state_dict with ft_white.weight, ft_black.weight, fc2, fc3 (from training.train --arch halfkp)"
            )
        return sd, arch
    if "fc1.weight" not in sd or "fc2.weight" not in sd or "fc3.weight" not in sd:
        raise ValueError(
            "Expected state_dict with fc1.weight, fc2.weight, fc3.weight (from training.train)"
        )
    return sd, "small_mlp"


def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Add convert (.pt -> .nnue) arguments to a parser or subparser."""
    parser.add_argument("input", help="Input .pt file")
    parser.add_argument("output", help="Output .nnue file")
    parser.add_argument(
        "--description",
        default="custom_pt",
        help="Description string stored in .nnue (default: custom_pt)",
    )


def run(args: argparse.Namespace) -> None:
    """Convert .pt to .nnue from parsed arguments (from add_arguments)."""
    sd, arch = load_pt(args.input)
    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    scale_bits = 6

    if arch == "halfkp":
        ft_w = sd["ft_white.weight"].numpy()   # (41024, 256)
        ft_b = sd["ft_black.weight"].numpy()   # (41024, 256)
        fc2_w = sd["fc2.weight"].numpy()      # (32, 512)
        fc2_b = sd["fc2.bias"].numpy()
        fc3_w = sd["fc3.weight"].numpy()      # (1, 32)
        fc3_b = sd["fc3.bias"].numpy()
        w1_int = np.clip(np.round(fc2_w * (1 << scale_bits)), -32768, 32767).astype(np.int16)
        b1_int = np.clip(np.round(fc2_b * (1 << scale_bits)), -32768, 32767).astype(np.int16)
        w2_int = np.clip(np.round(fc3_w / (1 << scale_bits)), -128, 127).astype(np.int8)
        b2_int = np.clip(np.round(fc3_b / (1 << scale_bits)), -128, 127).astype(np.int8)
        # Feature transformer: int16, white then black, row-major
        ft_vals = []
        for row in ft_w:
            ft_vals.extend(np.clip(np.round(row * (1 << scale_bits)), -32768, 32767).astype(np.int16).tolist())
        for row in ft_b:
            ft_vals.extend(np.clip(np.round(row * (1 << scale_bits)), -32768, 32767).astype(np.int16).tolist())
        with open(out_path, "wb") as f:
            write_uint32_le(f, NNUE_VERSION)
            write_uint32_le(f, NNUE_HASH)
            desc = args.description.encode("utf-8")
            write_uint32_le(f, len(desc))
            f.write(desc)
            write_leb128_block(f, ft_vals)
            for _ in range(8):
                for i in range(L1_OUTPUT):
                    for j in range(FT_OUTPUT_DIM):
                        write_int16_le(f, int(w1_int[i, j]))
                for i in range(L1_OUTPUT):
                    write_int16_le(f, int(b1_int[i]))
                for j in range(L1_OUTPUT):
                    write_int8_le(f, int(w2_int[0, j]))
                write_int8_le(f, int(b2_int[0]))
        print(f"Wrote {out_path} (HalfKP)")
        return

    # small_mlp
    fc1_w = sd["fc1.weight"].numpy()
    fc1_b = sd["fc1.bias"].numpy()
    fc2_w = sd["fc2.weight"].numpy()
    fc2_b = sd["fc2.bias"].numpy()
    fc3_w = sd["fc3.weight"].numpy()
    fc3_b = sd["fc3.bias"].numpy()
    w1 = np.zeros((L1_OUTPUT, FT_OUTPUT_DIM), dtype=np.float32)
    w1[:, :HIDDEN] = fc2_w
    w1[:, HIDDEN:FT_OUTPUT_DIM] = fc2_w
    b1 = fc2_b.astype(np.float32)
    w2 = fc3_w.astype(np.float32)
    b2 = fc3_b.astype(np.float32)
    w1_int = np.clip(np.round(w1 * (1 << scale_bits)), -32768, 32767).astype(np.int16)
    b1_int = np.clip(np.round(b1 * (1 << scale_bits)), -32768, 32767).astype(np.int16)
    w2_int = np.clip(np.round(w2 / (1 << scale_bits)), -128, 127).astype(np.int8)
    b2_int = np.clip(np.round(b2 / (1 << scale_bits)), -128, 127).astype(np.int8)
    with open(out_path, "wb") as f:
        write_uint32_le(f, NNUE_VERSION)
        write_uint32_le(f, NNUE_HASH)
        desc = args.description.encode("utf-8")
        write_uint32_le(f, len(desc))
        f.write(desc)
        ft_zeros_count = 512 * 2
        write_leb128_block(f, [0] * ft_zeros_count)
        for _ in range(8):
            for i in range(L1_OUTPUT):
                for j in range(FT_OUTPUT_DIM):
                    write_int16_le(f, int(w1_int[i, j]))
            for i in range(L1_OUTPUT):
                write_int16_le(f, int(b1_int[i]))
            for j in range(L1_OUTPUT):
                write_int8_le(f, int(w2_int[0, j]))
            write_int8_le(f, int(b2_int[0]))
    print(f"Wrote {out_path}")
    print(
        "Note: Feature transformer is zeros; for full HalfKP train with --arch halfkp and convert."
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert .pt (from train) to Stockfish .nnue format."
    )
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
