#!/usr/bin/env python3
"""
Texel tuning: minimize cross-entropy between sigmoid(eval) and game result.

Dataset: CSV with columns fen,result (result from White's view: 1=white wins,
0.5=draw, 0=black wins). We tune mg_value[0..5] and eg_value[0..5] (piece values
for material+PST). King (index 5) is fixed at 0.

Usage:
  python3 -m training.texel_tuning --data positions.csv --output params.json
  python3 -m training.texel_tuning --data positions.csv --iter 500 --k 1.0
"""

from __future__ import annotations

import argparse
import json
import sys
import numpy as np

try:
    from scipy.optimize import minimize as scipy_minimize
    HAS_SCIPY = True
except ImportError:
    HAS_SCIPY = False

from .eval import eval_fen

# Default (PeSTO) piece values; king = 0
DEFAULT_MG = np.array([82, 337, 365, 477, 1025, 0], dtype=np.float64)
DEFAULT_EG = np.array([94, 281, 297, 512, 936, 0], dtype=np.float64)


def sigmoid(x: np.ndarray, k: float) -> np.ndarray:
    """P(white wins) = 1 / (1 + exp(-k * x / 400)). x in centipawns."""
    return 1.0 / (1.0 + np.exp(-k * x / 400.0))


def cross_entropy_loss(
    params: np.ndarray,
    fens: list[str],
    results: np.ndarray,
    tune_k: bool,
) -> float:
    """Params: [mg0..mg5, eg0..eg5] or [mg0..mg5, eg0..eg5, k]. King mg5=eg5=0 fixed."""
    mg = np.zeros(6)
    eg = np.zeros(6)
    mg[:5] = params[:5]
    eg[:5] = params[5:10]
    # mg[5]=eg[5]=0
    k = params[10] if tune_k else 1.0
    if tune_k and (k <= 0.01 or k > 10.0):
        return 1e10
    evals = np.array([eval_fen(fen, mg, eg) for fen in fens], dtype=np.float64)
    p = sigmoid(evals, k)
    # Clip to avoid log(0)
    p = np.clip(p, 1e-6, 1 - 1e-6)
    return float(-np.sum(results * np.log(p) + (1 - results) * np.log(1 - p)))


def main() -> None:
    parser = argparse.ArgumentParser(description="Texel tuning for PeSTO piece values")
    parser.add_argument("--data", "-d", required=True, help="CSV file: fen,result (result 0, 0.5, or 1)")
    parser.add_argument("--output", "-o", help="Write tuned params to JSON (and C snippet to stdout)")
    parser.add_argument("--iter", "-n", type=int, default=1000, help="Max optimization iterations (default 1000)")
    parser.add_argument("--tune-k", action="store_true", help="Also tune sigmoid K (default: K=1)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")
    args = parser.parse_args()

    np.random.seed(args.seed)
    fens = []
    results = []
    with open(args.data) as f:
        header = f.readline().strip().lower()
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

    # Initial point: default PeSTO values (only tune pawn..queen; king=0)
    x0 = np.concatenate([DEFAULT_MG[:5], DEFAULT_EG[:5]])
    if args.tune_k:
        x0 = np.append(x0, 1.0)
    bounds = [(1, 2000)] * 5 + [(1, 2000)] * 5  # mg pawn..queen, eg pawn..queen
    if args.tune_k:
        bounds.append((0.1, 10.0))

    def obj(x: np.ndarray) -> float:
        return cross_entropy_loss(x, fens, results, tune_k=args.tune_k)

    if HAS_SCIPY:
        res = scipy_minimize(
            obj,
            x0,
            method="L-BFGS-B",
            bounds=bounds,
            options={"maxiter": args.iter, "disp": True},
        )
        if not res.success:
            print(f"Optimization warning: {res.message}", file=sys.stderr)
        x_best = res.x
        loss_best = res.fun
    else:
        # Fallback: coordinate descent (no scipy). Install scipy for faster L-BFGS-B.
        print("scipy not found; using coordinate-descent fallback (install scipy for L-BFGS-B).", file=sys.stderr)
        x_best = x0.copy()
        loss_best = obj(x_best)
        for _ in range(args.iter):
            improved = False
            for i in range(len(x_best)):
                lo, hi = bounds[i]
                step = max(1.0, 0.02 * x_best[i]) if x_best[i] != 0 else 1.0
                for delta in (-step, step):
                    x_new = x_best.copy()
                    x_new[i] = np.clip(x_best[i] + delta, lo, hi)
                    L = obj(x_new)
                    if L < loss_best:
                        loss_best = L
                        x_best = x_new
                        improved = True
            if not improved:
                break
        res = type('Res', (), {'fun': loss_best, 'x': x_best})()

    mg = np.zeros(6)
    eg = np.zeros(6)
    mg[:5] = res.x[:5]
    eg[:5] = res.x[5:10]
    k = float(res.x[10]) if args.tune_k else 1.0
    print(f"\nFinal loss: {res.fun:.2f}", file=sys.stderr)
    print("Tuned mg_value (pawn..king):", np.round(mg).astype(int).tolist(), file=sys.stderr)
    print("Tuned eg_value (pawn..king):", np.round(eg).astype(int).tolist(), file=sys.stderr)
    if args.tune_k:
        print(f"Tuned K: {k:.4f}", file=sys.stderr)

    out = {
        "mg_value": [int(round(x)) for x in mg],
        "eg_value": [int(round(x)) for x in eg],
        "k": k,
        "loss": float(res.fun),
        "n_positions": len(fens),
    }
    if args.output:
        with open(args.output, "w") as f:
            json.dump(out, f, indent=2)
        print(f"Wrote {args.output}", file=sys.stderr)

    # Print C snippet for pesto.c
    print("\n/* Paste into src/eval/pesto.c (replace static const int mg_value[6] / eg_value[6]) */")
    print("static const int mg_value[6] = {", ", ".join(str(int(round(x))) for x in mg), "};")
    print("static const int eg_value[6] = {", ", ".join(str(int(round(x))) for x in eg), "};")


if __name__ == "__main__":
    main()
