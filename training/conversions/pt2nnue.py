#!/usr/bin/env python3
"""
Convert a .pt model (from train.py) to the legacy .nnue binary format
expected by this repo's nnue-probe (halfkp_256x2-32-32, 21022697 bytes).

Supports:
- small_mlp: 768->256->32->1; writes zero feature transformer + 512->32->1.
- halfkp: HalfKP feature transformer (41024 x 256) + 512->32->1; writes
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


# Legacy nnue-probe format (vendor/nnue-probe/src/nnue.cpp)
LEGACY_VERSION = 0x7AF32F16
LEGACY_HASH = 0x3E5AA6EE
LEGACY_DESC_LEN = 177
LEGACY_TRANSFORMER_MAGIC = 0x5D69D7B8
LEGACY_NETWORK_MAGIC = 0x63337156
LEGACY_FILE_SIZE = 21_022_697

# Dense layer dimensions (nnue-probe: 512 -> 32 -> 1)
FT_OUTPUT_DIM = 512
L1_OUTPUT = 32
L2_OUTPUT = 1

# Our .pt layout from train.py (mlp)
BOARD_DIM = 768
HIDDEN = 256
# HalfKP (one half: probe uses same weights for both perspectives)
HALFKP_FEATURES_PER_HALF = 64 * 641  # 41024
FT_OUTPUT_PER_HALF = 256
FT_IN_DIMS = HALFKP_FEATURES_PER_HALF


def write_uint32_le(f: object, value: int) -> None:
    f.write(struct.pack("<I", value & 0xFFFFFFFF))


def write_int16_le(f: object, value: int) -> None:
    f.write(struct.pack("<h", max(-32768, min(32767, value))))


def write_int8_le(f: object, value: int) -> None:
    f.write(struct.pack("<b", max(-128, min(127, value))))


def write_int32_le(f: object, value: int) -> None:
    f.write(struct.pack("<i", value & 0xFFFFFFFF))


def _inv_permute_biases(b: np.ndarray) -> np.ndarray:
    """Inverse of nnue-probe permute_biases so that after load they match our order."""
    out = np.empty_like(b)
    # permute_biases: tmp[0]=b[0], tmp[1]=b[4], tmp[2]=b[1], tmp[3]=b[5], ...
    # So we write tmp so that after permute: b[i] = our_bias[i].
    # tmp[0]=our[0], tmp[1]=our[4], tmp[2]=our[1], tmp[3]=our[5], ...
    for i in range(0, len(b), 8):
        block = b[i : i + 8]
        out[i + 0] = block[0]
        out[i + 1] = block[4]
        out[i + 2] = block[1]
        out[i + 3] = block[5]
        out[i + 4] = block[2]
        out[i + 5] = block[6]
        out[i + 6] = block[3]
        out[i + 7] = block[7]
    return out


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


def _write_legacy_nnue(
    f: object,
    ft_biases: np.ndarray,
    ft_weights: np.ndarray,
    fc2_w: np.ndarray,
    fc2_b: np.ndarray,
    fc3_w: np.ndarray,
    fc3_b: np.ndarray,
    description: str,
    scale_bits: int = 6,
) -> None:
    """Write legacy nnue-probe format (exact layout + padding to LEGACY_FILE_SIZE).
    Probe expects 512->32->32->1; we have 512->32->1 so we insert identity 32->32.
    """
    # Quantize: probe uses (sum >> scale_bits), int8 weights, int32 biases
    fc2_int8 = np.clip(
        np.round(fc2_w.astype(np.float64) * (1 << scale_bits)), -128, 127
    ).astype(np.int8)
    b1_int = np.clip(
        np.round(fc2_b.astype(np.float64) * (1 << scale_bits)),
        -2147483648,
        2147483647,
    ).astype(np.int32)
    # hidden2 = identity so hidden2_out ≈ hidden1_out; then output = fc3(hidden2_out)
    b2_int = np.zeros(32, dtype=np.int32)
    out_bias_int = np.clip(
        np.round(fc3_b.astype(np.float64) * (1 << scale_bits)),
        -2147483648,
        2147483647,
    ).astype(np.int32)
    out_w_int8 = np.clip(
        np.round(fc3_w.astype(np.float64) / (1 << scale_bits)), -128, 127
    ).astype(np.int8)

    # Header
    write_uint32_le(f, LEGACY_VERSION)
    write_uint32_le(f, LEGACY_HASH)
    write_uint32_le(f, LEGACY_DESC_LEN)
    desc_bytes = description.encode("utf-8")[:LEGACY_DESC_LEN].ljust(LEGACY_DESC_LEN, b"\x00")
    f.write(desc_bytes)

    # Transformer block (magic + 256 int16 biases + 256*41024 int16 weights)
    write_uint32_le(f, LEGACY_TRANSFORMER_MAGIC)
    for i in range(256):
        write_int16_le(f, int(ft_biases[i]))
    for feat in range(FT_IN_DIMS):
        for out_dim in range(256):
            write_int16_le(f, int(ft_weights[feat, out_dim]))

    # Network block (probe: 512->32->32->1)
    write_uint32_le(f, LEGACY_NETWORK_MAGIC)
    # hidden1: 32 int32 biases (inverse permute for AVX2), then 32*512 int8 weights
    b1_perm = _inv_permute_biases(b1_int)
    for i in range(32):
        write_int32_le(f, int(b1_perm[i]))
    for r in range(32):
        for c in range(512):
            write_int8_le(f, int(fc2_int8[r, c]))
    # hidden2: identity (64 on diagonal) so layer passes through
    b2_perm = _inv_permute_biases(b2_int)
    for i in range(32):
        write_int32_le(f, int(b2_perm[i]))
    identity_scale = 1 << scale_bits  # 64 so (x*64)>>6 = x
    for r in range(32):
        for c in range(32):
            write_int8_le(f, identity_scale if r == c else 0)
    # output: 1 int32 bias, 32 int8 weights
    write_int32_le(f, int(np.asarray(out_bias_int).flat[0]))
    for j in range(32):
        write_int8_le(f, int(out_w_int8.flat[j]))

    # Pad to exact legacy file size
    written = f.tell()
    if written > LEGACY_FILE_SIZE:
        raise RuntimeError(f"Legacy layout overflow: {written} > {LEGACY_FILE_SIZE}")
    f.write(b"\x00" * (LEGACY_FILE_SIZE - written))


def run(args: argparse.Namespace) -> None:
    """Convert .pt to .nnue from parsed arguments (from add_arguments)."""
    sd, arch = load_pt(args.input)
    out_path = Path(args.output)
    out_path.parent.mkdir(parents=True, exist_ok=True)
    scale_bits = 6
    desc = args.description[:LEGACY_DESC_LEN] if len(args.description) > LEGACY_DESC_LEN else args.description

    if arch == "halfkp":
        ft_w = sd["ft_white.weight"].numpy()  # (41024, 256)
        ft_quant = np.clip(
            np.round(ft_w * (1 << scale_bits)), -32768, 32767
        ).astype(np.int16)
        ft_biases = np.zeros(256, dtype=np.int16)
        ft_weights = ft_quant  # (41024, 256)
        fc2_w = sd["fc2.weight"].numpy()
        fc2_b = sd["fc2.bias"].numpy()
        fc3_w = sd["fc3.weight"].numpy()
        fc3_b = sd["fc3.bias"].numpy()
        with open(out_path, "wb") as f:
            _write_legacy_nnue(
                f, ft_biases, ft_weights, fc2_w, fc2_b, fc3_w, fc3_b, desc, scale_bits
            )
        print(f"Wrote {out_path} (HalfKP, legacy {LEGACY_FILE_SIZE} bytes)")
        return

    # small_mlp: zero feature transformer, duplicate fc2 input for 512->32->1
    ft_biases = np.zeros(256, dtype=np.int16)
    ft_weights = np.zeros((FT_IN_DIMS, 256), dtype=np.int16)
    fc2_w_orig = sd["fc2.weight"].numpy()
    fc2_w = np.zeros((L1_OUTPUT, FT_OUTPUT_DIM), dtype=np.float32)
    fc2_w[:, :HIDDEN] = fc2_w_orig
    fc2_w[:, HIDDEN:FT_OUTPUT_DIM] = fc2_w_orig
    fc2_b = sd["fc2.bias"].numpy()
    fc3_w = sd["fc3.weight"].numpy()
    fc3_b = sd["fc3.bias"].numpy()
    with open(out_path, "wb") as f:
        _write_legacy_nnue(
            f, ft_biases, ft_weights, fc2_w, fc2_b, fc3_w, fc3_b, desc, scale_bits
        )
    print(f"Wrote {out_path} (small_mlp, legacy {LEGACY_FILE_SIZE} bytes)")
    print(
        "Note: Feature transformer is zeros; for full HalfKP train with --arch halfkp and convert."
    )


def main() -> None:
    parser = argparse.ArgumentParser(
        description="Convert .pt (from train) to legacy .nnue format for this engine's nnue-probe."
    )
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
