#!/usr/bin/env python3
# @module train
# @desc Custom NNUE-style trainer. Reads CSV (fen, score_cp); supports mlp (768->256->32->1)
# and halfkp (HalfKP FT -> 512->32->1). Saves .pt for the convert command.
"""
Custom NNUE-style trainer without nnue-pytorch.

Reads CSV (fen, score_cp) from generate_data. Two architectures:
- mlp: dense 768-dim board -> 256 -> 32 -> 1 (simple).
- halfkp: HalfKP feature indices -> feature transformer (512) -> 32 -> 1 (Stockfish-compatible).

Invoked via: python3 -m training train --data data.csv --output model.pt
"""

from __future__ import annotations

import argparse
import csv
import sys
from pathlib import Path

try:
    import torch
    import torch.nn as nn
except ImportError:
    print("train requires PyTorch: pip install torch", file=sys.stderr)
    sys.exit(1)

from training.features.halfkp import (
    HalfKP,
    halfkp_indices_from_fen,
)

# Piece codes for FEN (uppercase = white, lowercase = black). We map to indices.
FEN_PIECE = {
    "P": 0, "N": 1, "B": 2, "R": 3, "Q": 4, "K": 5,
    "p": 6, "n": 7, "b": 8, "r": 9, "q": 10, "k": 11,
}
# Simple encoding: 12 piece types * 64 squares = 768 dimensions (sparse in practice)
BOARD_DIM = 12 * 64  # 768
HIDDEN = 256
OUTPUT = 1
# HalfKP: 41024 features per half, 256 output per half -> 512
HALFKP_FEATURES = HalfKP.FEATURES_PER_HALF
FT_OUTPUT_PER_HALF = 256
FT_OUTPUT_DIM = FT_OUTPUT_PER_HALF * 2  # 512


# @method fen_to_board_tensor
# @desc Convert FEN to a flat board vector of shape (768,). One-hot over 12*64;
# assumes FEN has at least the piece-placement field. Used only for the mlp architecture.
# @param fen FEN string (at least piece placement).
# @returns torch.Tensor Shape (768,) float32 one-hot board vector.
def fen_to_board_tensor(fen: str) -> torch.Tensor:
    """Convert FEN to a flat board vector (12*64). Assumes FEN has 6 fields."""
    parts = fen.split()
    if len(parts) < 1:
        raise ValueError("Invalid FEN")
    board = [0.0] * (12 * 64)
    row, col = 0, 0
    for c in parts[0]:
        if c == "/":
            row += 1
            col = 0
            continue
        if c.isdigit():
            col += int(c)
            continue
        if c in FEN_PIECE:
            idx = FEN_PIECE[c] * 64 + (row * 8 + col)
            board[idx] = 1.0
            col += 1
    return torch.tensor(board, dtype=torch.float32)


# @method load_csv
# @desc Load CSV with columns fen and score_cp. Returns list of board tensors and
# list of scores; skips invalid rows. Used when --arch mlp.
# @param path Path to CSV file.
# @returns tuple[list[torch.Tensor],list[float]] Board tensors and scores.
def load_csv(path: str) -> tuple[list[torch.Tensor], list[float]]:
    """Load (fen, score_cp) CSV; return list of board tensors and list of scores."""
    xs, ys = [], []
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            fen = row.get("fen", "").strip('"')
            try:
                score = float(row.get("score_cp", 0))
            except ValueError:
                continue
            try:
                xs.append(fen_to_board_tensor(fen))
                ys.append(score)
            except ValueError:
                continue
    return xs, ys


# @class SmallNet
# @desc MLP for mlp architecture: 768 -> 256 -> 32 -> 1. Dense board input;
# converted .nnue has zero feature transformer.
class SmallNet(nn.Module):
    """Small MLP: BOARD_DIM -> HIDDEN -> 32 -> 1. Matches typical NNUE hidden sizes."""

    # @constructor SmallNet
    # @desc Builds fc1 (768->256), fc2 (256->32), fc3 (32->1).
    def __init__(self) -> None:
        super().__init__()
        self.fc1 = nn.Linear(BOARD_DIM, HIDDEN)
        self.fc2 = nn.Linear(HIDDEN, 32)
        self.fc3 = nn.Linear(32, OUTPUT)

    # @method forward
    # @param x Board tensor (batch, 768).
    # @returns Predicted score (batch,).
    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        return self.fc3(x).squeeze(-1)


# @class HalfKPNet
# @desc HalfKP NNUE: sparse indices -> two EmbeddingBags -> 512 -> 32 -> 1.
# Compatible with .nnue conversion when using --arch halfkp.
class HalfKPNet(nn.Module):
    """
    HalfKP NNUE: sparse feature indices -> feature transformer (512) -> 32 -> 1.

    One shared feature table (41024, 256) indexed by white-half and black-half indices;
    we use two EmbeddingBags (one per half) so each half sums 11 rows to (256,). Concat -> 512.
    """

    # @constructor HalfKPNet
    # @desc Creates ft_white, ft_black EmbeddingBags and fc2, fc3.
    def __init__(self) -> None:
        super().__init__()
        self.ft_white = nn.EmbeddingBag(
            HALFKP_FEATURES, FT_OUTPUT_PER_HALF, mode="sum"
        )
        self.ft_black = nn.EmbeddingBag(
            HALFKP_FEATURES, FT_OUTPUT_PER_HALF, mode="sum"
        )
        self.fc2 = nn.Linear(FT_OUTPUT_DIM, 32)
        self.fc3 = nn.Linear(32, OUTPUT)

    # @method forward
    # @param white_indices Flattened white-half feature indices (11 per sample).
    # @param black_indices Flattened black-half feature indices.
    # @param white_offsets Offsets into white_indices per sample (0, 11, 22, ...).
    # @param black_offsets Offsets into black_indices per sample.
    # @returns Predicted score per sample, shape (batch,).
    def forward(
        self,
        white_indices: torch.Tensor,
        black_indices: torch.Tensor,
        white_offsets: torch.Tensor,
        black_offsets: torch.Tensor,
    ) -> torch.Tensor:
        w = self.ft_white(white_indices, white_offsets)  # (batch, 256)
        b = self.ft_black(black_indices, black_offsets)  # (batch, 256)
        x = torch.cat([w, b], dim=1)  # (batch, 512)
        x = torch.relu(self.fc2(x))
        return self.fc3(x).squeeze(-1)


# @method load_csv_halfkp
# @desc Load CSV (fen, score_cp); return (white_indices, black_indices) per row and scores.
# @param path Path to CSV file.
# @returns tuple[list[tuple[list[int],list[int]]],list[float]] List of (white_indices, black_indices) and scores.
def load_csv_halfkp(path: str) -> tuple[list[tuple[list[int], list[int]]], list[float]]:
    """Load (fen, score_cp) CSV; return list of (white_indices, black_indices) and list of scores."""
    samples, ys = [], []
    with open(path, newline="", encoding="utf-8") as f:
        reader = csv.DictReader(f)
        for row in reader:
            fen = row.get("fen", "").strip('"')
            try:
                score = float(row.get("score_cp", 0))
            except ValueError:
                continue
            try:
                wi, bi = halfkp_indices_from_fen(fen)
                samples.append((wi, bi))
                ys.append(score)
            except ValueError:
                continue
    return samples, ys


# @method add_arguments
# @desc Registers --data, --arch, --epochs, --lr, --batch-size, --output, --val-frac.
# @param parser ArgumentParser or subparser to add flags to.
def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Add train subcommand arguments to a parser or subparser."""
    parser.add_argument("--data", "-d", required=True, help="CSV file (fen, score_cp) from data command")
    parser.add_argument("--arch", choices=("mlp", "halfkp"), default="mlp", help="Architecture: mlp (768->256->32->1) or halfkp (HalfKP FT -> 512->32->1)")
    parser.add_argument("--epochs", "-e", type=int, default=20, help="Training epochs (default: 20)")
    parser.add_argument("--lr", type=float, default=1e-3, help="Learning rate (default: 1e-3)")
    parser.add_argument("--batch-size", type=int, default=256, help="Batch size (default: 256)")
    parser.add_argument("--output", "-o", default="model.pt", help="Output .pt file (default: model.pt)")
    parser.add_argument("--val-frac", type=float, default=0.1, help="Fraction of data for validation (default: 0.1)")


# @method run
# @desc Train from args: load data, build model, run epochs, save .pt.
# @param args Parsed namespace from add_arguments.
def run(args: argparse.Namespace) -> None:
    """Train from parsed arguments (from add_arguments)."""
    if args.arch == "mlp":
        xs, ys = load_csv(args.data)
        if not xs:
            print("No valid rows in CSV.", file=sys.stderr)
            sys.exit(1)
        X = torch.stack(xs)
        y = torch.tensor(ys, dtype=torch.float32)
        n = len(xs)
        perm = torch.randperm(n)
        X, y = X[perm], y[perm]
        nval = max(1, int(n * args.val_frac))
        X_val, y_val = X[:nval], y[:nval]
        X_train, y_train = X[nval:], y[nval:]
        model = SmallNet()
        opt = torch.optim.Adam(model.parameters(), lr=args.lr)
        criterion = nn.MSELoss()

        for epoch in range(args.epochs):
            model.train()
            total_loss = 0.0
            batches = 0
            for i in range(0, len(X_train), args.batch_size):
                batch_x = X_train[i : i + args.batch_size]
                batch_y = y_train[i : i + args.batch_size]
                pred = model(batch_x)
                loss = criterion(pred, batch_y)
                opt.zero_grad()
                loss.backward()
                opt.step()
                total_loss += loss.item()
                batches += 1
            train_loss = total_loss / batches if batches else 0
            model.eval()
            with torch.no_grad():
                val_pred = model(X_val)
                val_loss = criterion(val_pred, y_val).item()
            print(f"Epoch {epoch+1}/{args.epochs}  train_loss={train_loss:.4f}  val_loss={val_loss:.4f}")

        Path(args.output).parent.mkdir(parents=True, exist_ok=True)
        torch.save(
            {"state_dict": model.state_dict(), "board_dim": BOARD_DIM, "hidden": HIDDEN, "arch": "small_mlp"},
            args.output,
        )
    else:
        # HalfKP
        samples, ys = load_csv_halfkp(args.data)
        if not samples:
            print("No valid rows in CSV.", file=sys.stderr)
            sys.exit(1)
        y = torch.tensor(ys, dtype=torch.float32)
        n = len(samples)
        perm = torch.randperm(n)
        indices_perm = [samples[i] for i in perm]
        y = y[perm]
        nval = max(1, int(n * args.val_frac))
        val_samples = indices_perm[:nval]
        y_val = y[:nval]
        train_samples = indices_perm[nval:]
        y_train = y[nval:]

        model = HalfKPNet()
        opt = torch.optim.Adam(model.parameters(), lr=args.lr)
        criterion = nn.MSELoss()
        device = torch.device("cuda" if torch.cuda.is_available() else "cpu")
        model = model.to(device)
        y_train = y_train.to(device)
        y_val = y_val.to(device)

        def batch_halfkp(samp_list: list, score_tensor: torch.Tensor, start: int, end: int) -> tuple:
            B = end - start
            w_flat, b_flat = [], []
            for j in range(start, end):
                wj, bj = samp_list[j]
                w_flat.extend(wj)
                b_flat.extend(bj)
            per_sample = 11
            white_indices = torch.tensor(w_flat, dtype=torch.long, device=device)
            black_indices = torch.tensor(b_flat, dtype=torch.long, device=device)
            white_offsets = torch.arange(0, B * per_sample, per_sample, dtype=torch.long, device=device)
            black_offsets = torch.arange(0, B * per_sample, per_sample, dtype=torch.long, device=device)
            return white_indices, black_indices, white_offsets, black_offsets, score_tensor[start:end]

        for epoch in range(args.epochs):
            model.train()
            total_loss = 0.0
            batches = 0
            for i in range(0, len(train_samples), args.batch_size):
                end = min(i + args.batch_size, len(train_samples))
                w_idx, b_idx, w_off, b_off, batch_y = batch_halfkp(train_samples, y_train, i, end)
                pred = model(w_idx, b_idx, w_off, b_off)
                loss = criterion(pred, batch_y)
                opt.zero_grad()
                loss.backward()
                opt.step()
                total_loss += loss.item()
                batches += 1
            train_loss = total_loss / batches if batches else 0
            model.eval()
            with torch.no_grad():
                val_loss_sum = 0.0
                v_batches = 0
                for vi in range(0, len(val_samples), args.batch_size):
                    ve = min(vi + args.batch_size, len(val_samples))
                    w_idx, b_idx, w_off, b_off, batch_y = batch_halfkp(val_samples, y_val, vi, ve)
                    val_pred = model(w_idx, b_idx, w_off, b_off)
                    val_loss_sum += criterion(val_pred, batch_y).item()
                    v_batches += 1
                val_loss = val_loss_sum / v_batches if v_batches else 0
            print(f"Epoch {epoch+1}/{args.epochs}  train_loss={train_loss:.4f}  val_loss={val_loss:.4f}")

        Path(args.output).parent.mkdir(parents=True, exist_ok=True)
        torch.save(
            {
                "state_dict": model.state_dict(),
                "arch": "halfkp",
                "halfkp_features": HALFKP_FEATURES,
                "ft_output_per_half": FT_OUTPUT_PER_HALF,
                "ft_output_dim": FT_OUTPUT_DIM,
            },
            args.output,
        )
    print(f"Saved to {args.output}")


def main() -> None:
    parser = argparse.ArgumentParser(description="Train a small NNUE-style net from CSV data.")
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
