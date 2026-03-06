#!/usr/bin/env python3
"""
Custom NNUE-style trainer without nnue-pytorch.

Reads CSV (fen, score_cp) from generate_data.py, converts FENs to a simple
board feature vector, trains a small MLP, and saves weights to a .pt file.

The model architecture is chosen so it can later be exported to Stockfish
.nnue format (feature transformer output 512, then 512->32->1). This script
uses a simplified dense board encoding instead of full HalfKP; for
Stockfish-compatible .nnue you still need to implement HalfKP features and
the exact .nnue binary serializer (see README).

Usage:
  python3 -m nnue_training.train --data data.csv --epochs 10 --output model.pt
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
    print("train.py requires PyTorch: pip install torch", file=sys.stderr)
    sys.exit(1)


# Piece codes for FEN (uppercase = white, lowercase = black). We map to indices.
FEN_PIECE = {
    "P": 0, "N": 1, "B": 2, "R": 3, "Q": 4, "K": 5,
    "p": 6, "n": 7, "b": 8, "r": 9, "q": 10, "k": 11,
}
# Simple encoding: 12 piece types * 64 squares = 768 dimensions (sparse in practice)
# We use a dense 768-dim vector: one-hot piece on square (flattened).
BOARD_DIM = 12 * 64  # 768
HIDDEN = 256
OUTPUT = 1


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


class SmallNet(nn.Module):
    """Small MLP: BOARD_DIM -> HIDDEN -> 32 -> 1. Matches typical NNUE hidden sizes."""

    def __init__(self) -> None:
        super().__init__()
        self.fc1 = nn.Linear(BOARD_DIM, HIDDEN)
        self.fc2 = nn.Linear(HIDDEN, 32)
        self.fc3 = nn.Linear(32, OUTPUT)

    def forward(self, x: torch.Tensor) -> torch.Tensor:
        x = torch.relu(self.fc1(x))
        x = torch.relu(self.fc2(x))
        return self.fc3(x).squeeze(-1)


def main() -> None:
    parser = argparse.ArgumentParser(description="Train a small NNUE-style net from CSV data.")
    parser.add_argument("--data", "-d", required=True, help="CSV file (fen, score_cp) from generate_data.py")
    parser.add_argument("--epochs", "-e", type=int, default=20, help="Training epochs (default: 20)")
    parser.add_argument("--lr", type=float, default=1e-3, help="Learning rate (default: 1e-3)")
    parser.add_argument("--batch-size", type=int, default=256, help="Batch size (default: 256)")
    parser.add_argument("--output", "-o", default="model.pt", help="Output .pt file (default: model.pt)")
    parser.add_argument("--val-frac", type=float, default=0.1, help="Fraction of data for validation (default: 0.1)")
    args = parser.parse_args()

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
    print(f"Saved to {args.output}")


if __name__ == "__main__":
    main()
