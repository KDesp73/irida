#!/usr/bin/env python3
"""
Convert a .pt model (from train.py) to Stockfish .nnue binary format.

The engine loads .nnue via nnue-probe, which expects a specific binary layout
(version, hash, description, feature transformer in LEB128, then dense layers).
Our .pt has architecture 768->256->32->1; Stockfish uses a feature transformer
plus 512->32->1 (or 1024->31->32->1 in current SF). This script writes a
.nnue that matches the layout expected by nnue-probe when built against
Stockfish-style nets. We map our dense part (256->32->1) by padding to
512->32->1 and write a placeholder feature transformer so the file parses.

Usage:
  python3 -m nnue_training.convert_pt_to_nnue model.pt output.nnue

Note: The resulting .nnue may load in the engine, but evaluation quality
depends on the feature transformer matching. For production nets, train
with HalfKP (e.g. nnue-pytorch) and use its serialize script, or implement
HalfKP in our trainer and fill the FT section from our fc1.
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

# Our .pt layout from train.py
BOARD_DIM = 768
HIDDEN = 256


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


def load_pt(path: str) -> dict:
    """Load .pt and return state_dict; validate layout."""
    data = torch.load(path, map_location="cpu", weights_only=False)
    if isinstance(data, dict) and "state_dict" in data:
        sd = data["state_dict"]
    else:
        sd = data if isinstance(data, dict) else {}
    if "fc1.weight" not in sd or "fc2.weight" not in sd or "fc3.weight" not in sd:
        raise ValueError(
            "Expected state_dict with fc1.weight, fc2.weight, fc3.weight (from nnue_training.train)"
        )
    return sd


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert .pt (from train.py) to Stockfish .nnue format."
    )
    parser.add_argument("input", help="Input .pt file")
    parser.add_argument("output", help="Output .nnue file")
    parser.add_argument(
        "--description",
        default="custom_pt",
        help="Description string stored in .nnue (default: custom_pt)",
    )
    args = parser.parse_args()

    sd = load_pt(args.input)
    fc1_w = sd["fc1.weight"].numpy()
    fc1_b = sd["fc1.bias"].numpy()
    fc2_w = sd["fc2.weight"].numpy()
    fc2_b = sd["fc2.bias"].numpy()
    fc3_w = sd["fc3.weight"].numpy()
    fc3_b = sd["fc3.bias"].numpy()

    # We need to produce: header, feature transformer (LEB128), then dense 512->32->1.
    # Our dense part is 256->32->1. Pad fc2 to 512->32 by repeating rows (so each 256 row
    # is duplicated to get 512 rows).
    w1 = np.zeros((L1_OUTPUT, FT_OUTPUT_DIM), dtype=np.float32)
    # fc2.weight is (32, 256); we need (32, 512), so repeat columns
    w1[:, :HIDDEN] = fc2_w
    w1[:, HIDDEN:FT_OUTPUT_DIM] = fc2_w
    b1 = fc2_b.astype(np.float32)
    # fc3 is Linear(32, 1): weight (1, 32), bias (1,)
    w2 = fc3_w.astype(np.float32)  # (1, 32)
    b2 = fc3_b.astype(np.float32)

    # Quantize: Stockfish uses int16 for first dense layer, int8 for output layer (with scale).
    scale_bits = 6
    w1_int = np.clip(np.round(w1 * (1 << scale_bits)), -32768, 32767).astype(np.int16)
    b1_int = np.clip(np.round(b1 * (1 << scale_bits)), -32768, 32767).astype(np.int16)
    w2_int = np.clip(np.round(w2 / (1 << scale_bits)), -128, 127).astype(np.int8)
    b2_int = np.clip(np.round(b2 / (1 << scale_bits)), -128, 127).astype(np.int8)

    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)

    with open(out_path, "wb") as f:
        # 1. Header: version, hash, description length, description
        write_uint32_le(f, NNUE_VERSION)
        write_uint32_le(f, NNUE_HASH)
        desc = args.description.encode("utf-8")
        write_uint32_le(f, len(desc))
        f.write(desc)

        # 2. Feature transformer: LEB128-compressed int16. The exact count must match
        # what nnue-probe was compiled for (HalfKP is huge). We write zeros; if the
        # loader expects a different count, loading will fail. Try a common size.
        ft_zeros_count = 512 * 2
        write_leb128_block(f, [0] * ft_zeros_count)

        # 3. Dense layers: 8 layer stacks (Stockfish LayerStacks = 8), each 512->32 (int16), 32->1 (int8)
        for _ in range(8):
            for i in range(L1_OUTPUT):
                for j in range(FT_OUTPUT_DIM):
                    write_int16_le(f, int(w1_int[i, j]))
            for i in range(L1_OUTPUT):
                write_int16_le(f, int(b1_int[i]))
            for j in range(L1_OUTPUT):
                write_int8_le(f, int(w2_int[0, j]))  # w2_int shape (1, 32)
            write_int8_le(f, int(b2_int[0]))

    print(f"Wrote {out_path}")
    print(
        "Note: The feature transformer was filled with zeros; eval may be poor. "
        "For correct eval, train with HalfKP and export via nnue-pytorch serialize, "
        "or implement HalfKP in our trainer."
    )


if __name__ == "__main__":
    main()
