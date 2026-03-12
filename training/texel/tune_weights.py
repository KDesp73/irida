#!/usr/bin/env python3
# @module texel.tune_weights
# @desc Tune PeSTO term weights (8 scale factors) from engine eval-breakdown-batch.
"""
Tune PeSTO term weights from eval breakdown.

Loads CSV (fen, result), runs the engine in eval-breakdown-batch mode to get
the 8 term values per position (material_pst, pawn_structure, mobility,
king_safety, piece_activity, space, threats, endgame), then minimizes
cross-entropy between sigmoid(weights . terms) and game result.

Usage:
  python3 -m training texel-weights --data positions.csv --engine ./engine --output weights.json
"""

from __future__ import annotations

import argparse
import json
import subprocess
import sys
import numpy as np

try:
    from scipy.optimize import minimize as scipy_minimize
    HAS_SCIPY = True
except ImportError:
    HAS_SCIPY = False

# Term order from engine: material_pst, pawn_structure, mobility, king_safety, piece_activity, space, threats, endgame
TERM_NAMES = [
    "material_pst", "pawn_structure", "mobility", "king_safety",
    "piece_activity", "space", "threats", "endgame",
]


# @method sigmoid
# @desc P(white wins) = 1 / (1 + exp(-k * x / 400)). x in centipawns.
# @param x Combined eval (e.g. breakdown @ weights).
# @param k Sigmoid scale (default 1).
# @returns np.ndarray Probability per position.
def sigmoid(x: np.ndarray, k: float = 1.0) -> np.ndarray:
    """P(white wins) = 1 / (1 + exp(-k * x / 400)). x in centipawns."""
    return 1.0 / (1.0 + np.exp(-k * x / 400.0))


    return 1.0 / (1.0 + np.exp(-k * x / 400.0))


# @method load_breakdown
# @desc Run engine eval-breakdown-batch; return (N, 8) array of term values per position.
# @param engine_path Path to engine executable.
# @param fens List of FEN positions.
# @returns np.ndarray Shape (N, 8) term values (White perspective).
def load_breakdown(engine_path: str, fens: list[str]) -> np.ndarray:
    """Run engine eval-breakdown-batch; return (N, 8) array of term values (white perspective)."""
    proc = subprocess.Popen(
        [engine_path, "eval-breakdown-batch"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1,
    )
    proc.stdin.write(f"{len(fens)}\n")
    for fen in fens:
        proc.stdin.write(fen + "\n")
    proc.stdin.close()
    rows = []
    for _ in range(len(fens)):
        line = proc.stdout.readline()
        if not line:
            break
        parts = line.split()
        if len(parts) >= 8:
            rows.append([int(parts[i]) for i in range(8)])
    proc.wait()
    return np.array(rows, dtype=np.float64)


    return np.array(rows, dtype=np.float64)


# @method add_arguments
# @desc Registers --data, --engine, --output, --iter, --tune-k, --seed.
# @param parser ArgumentParser or subparser.
# @returns None
def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Add texel-weights arguments to a parser or subparser."""
    parser.add_argument("--data", "-d", required=True, help="CSV file: fen,result (result 0, 0.5, or 1)")
    parser.add_argument("--engine", "-e", required=True, metavar="PATH", help="Engine binary (e.g. ./engine)")
    parser.add_argument("--output", "-o", help="Write tuned weights to JSON")
    parser.add_argument("--iter", "-n", type=int, default=500, help="Max optimization iterations (default 500)")
    parser.add_argument("--tune-k", action="store_true", help="Also tune sigmoid K")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")


# @method run
# @desc Load fen,result; get breakdown from engine; minimize cross-entropy over 8 weights; write JSON.
# @param args Parsed namespace from add_arguments.
# @returns None
def run(args: argparse.Namespace) -> None:
    """Tune term weights from parsed arguments (from add_arguments)."""
    np.random.seed(args.seed)
    fens = []
    results = []
    with open(args.data) as f:
        f.readline()
        for line in f:
            line = line.strip()
            if not line or line.startswith("#"):
                continue
            parts = line.split(",")
            if len(parts) < 2:
                continue
            fen = parts[0].strip()
            try:
                r = float(parts[1].strip())
            except ValueError:
                continue
            if not (0 <= r <= 1):
                continue
            fens.append(fen)
            results.append(r)
    results = np.array(results, dtype=np.float64)
    if len(fens) == 0:
        print("No valid fen,result rows found.", file=sys.stderr)
        sys.exit(1)
    print(f"Loaded {len(fens)} positions from {args.data}", file=sys.stderr)

    print("Fetching eval breakdown from engine...", file=sys.stderr)
    breakdown = load_breakdown(args.engine, fens)
    if breakdown.shape[0] != len(fens):
        print(f"Engine returned {breakdown.shape[0]} rows, expected {len(fens)}", file=sys.stderr)
        sys.exit(1)
    print(f"Breakdown shape: {breakdown.shape}", file=sys.stderr)

    # Weights: scale factors for each term. Initial 1 for material_pst, rest from current PeSTO (relative scale).
    # Current PeSTO uses all terms with implicit weight 1 (they're already in cp). We tune 8 multipliers.
    x0 = np.ones(8, dtype=np.float64)
    if args.tune_k:
        x0 = np.append(x0, 1.0)
    bounds = [(0.01, 10.0)] * 8  # allow scale down/up per term
    if args.tune_k:
        bounds.append((0.1, 10.0))

    def loss(x: np.ndarray) -> float:
        w = x[:8]
        k = x[8] if args.tune_k else 1.0
        if args.tune_k and (k <= 0.01 or k > 10.0):
            return 1e10
        evals = breakdown @ w
        p = sigmoid(evals, k)
        p = np.clip(p, 1e-6, 1 - 1e-6)
        return float(-np.sum(results * np.log(p) + (1 - results) * np.log(1 - p)))

    if HAS_SCIPY:
        res = scipy_minimize(
            loss,
            x0,
            method="L-BFGS-B",
            bounds=bounds,
            options={"maxiter": args.iter, "disp": True},
        )
        weights = res.x[:8]
        k = float(res.x[8]) if args.tune_k else 1.0
        final_loss = res.fun
    else:
        print("scipy required for tune_weights. pip install scipy", file=sys.stderr)
        sys.exit(1)

    print(f"\nFinal loss: {final_loss:.2f}", file=sys.stderr)
    for i, name in enumerate(TERM_NAMES):
        print(f"  {name}: {weights[i]:.4f}", file=sys.stderr)
    if args.tune_k:
        print(f"  K: {k:.4f}", file=sys.stderr)

    out = {name: float(weights[i]) for i, name in enumerate(TERM_NAMES)}
    if args.tune_k:
        out["k"] = k
    out["loss"] = float(final_loss)
    out["n_positions"] = len(fens)
    if args.output:
        with open(args.output, "w") as f:
            json.dump(out, f, indent=2)
        print(f"Wrote {args.output}", file=sys.stderr)
    else:
        print(json.dumps(out, indent=2))


def main() -> None:
    parser = argparse.ArgumentParser(description="Tune PeSTO term weights from eval breakdown")
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
