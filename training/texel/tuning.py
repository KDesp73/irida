"""
Texel tuning: minimize cross-entropy between sigmoid(eval) and game result.

Dataset: CSV with columns fen,result (result from White's view: 1=white wins,
0.5=draw, 0=black wins). We tune mg_value[0..5] and eg_value[0..5] (piece values
for material+PST). King (index 5) is fixed at 0. With --tune-weights, also tune
the 8 PeSTO term scale factors (material_pst, pawn_structure, mobility, ...).

Uses the C engine's eval-batch only (no Python eval clone). Build the engine first.

Usage:
  python3 -m training texel --data positions.csv --engine ./engine --output params.json
  python3 -m training texel --data positions.csv --engine ./engine --tune-weights --iter 1000
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

# Default (PeSTO) piece values; king = 0
DEFAULT_MG = np.array([82, 337, 365, 477, 1025, 0], dtype=np.float64)
DEFAULT_EG = np.array([94, 281, 297, 512, 936, 0], dtype=np.float64)

TERM_NAMES = [
    "material_pst", "pawn_structure", "mobility", "king_safety",
    "piece_activity", "space", "threats", "endgame",
]


def sigmoid(x: np.ndarray, k: float) -> np.ndarray:
    """P(white wins) = 1 / (1 + exp(-k * x / 400)). x in centipawns."""
    return 1.0 / (1.0 + np.exp(-k * x / 400.0))


# @method _engine_evals
# @desc Send one batch (params + FENs) to engine eval-batch; return array of scores (white cp).
# Protocol: N, then one line with 10 ints (mg0..eg4) or 18 ints (mg0..eg4 w0..w7), then N FENs.
def _engine_evals(
    pipe_in,
    pipe_out,
    fens: list[str],
    params: np.ndarray,
    tune_k: bool,
    tune_weights: bool,
) -> np.ndarray:
    """Send one batch to engine eval-batch; return array of scores (white cp)."""
    n = len(fens)
    mg = np.zeros(6)
    eg = np.zeros(6)
    mg[:5] = params[:5]
    eg[:5] = params[5:10]
    if tune_weights:
        w = np.clip(np.round(params[10:18]), 1, 1000).astype(int)
        param_line = " ".join(str(int(round(x))) for x in list(mg[:5]) + list(eg[:5]) + list(w))
    else:
        param_line = " ".join(str(int(round(x))) for x in list(mg[:5]) + list(eg[:5]))
    pipe_in.write(f"{n}\n")
    pipe_in.write(param_line + "\n")
    for fen in fens:
        pipe_in.write(fen + "\n")
    pipe_in.flush()
    scores = []
    for _ in range(n):
        line = pipe_out.readline()
        if not line:
            break
        scores.append(int(line.strip()))
    return np.array(scores, dtype=np.float64)


# @method cross_entropy_loss
# @desc Negative sum of result*log(p) + (1-result)*log(1-p). Uses get_evals(params) for scores.
def cross_entropy_loss(
    params: np.ndarray,
    fens: list[str],
    results: np.ndarray,
    tune_k: bool,
    get_evals,
) -> float:
    """Cross-entropy between sigmoid(evals) and results. get_evals(params) returns white cp array."""
    k = params[-1] if tune_k else 1.0
    if tune_k and (k <= 0.01 or k > 10.0):
        return 1e10
    evals = get_evals(params)
    p = sigmoid(evals, k)
    p = np.clip(p, 1e-6, 1 - 1e-6)
    return float(-np.sum(results * np.log(p) + (1 - results) * np.log(1 - p)))


# @method add_arguments
# @desc Registers --data, --output, --iter, --tune-k, --engine, --seed for texel command.
# @param parser ArgumentParser or subparser.
def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Add texel tuning arguments to a parser or subparser."""
    parser.add_argument("--data", "-d", required=True, help="CSV file: fen,result (result 0, 0.5, or 1)")
    parser.add_argument("--output", "-o", help="Write tuned params to JSON (and C snippet to stdout)")
    parser.add_argument("--iter", "-n", type=int, default=1000, help="Max optimization iterations (default 1000)")
    parser.add_argument("--tune-k", action="store_true", help="Also tune sigmoid K (default: K=1)")
    parser.add_argument("--tune-weights", action="store_true", help="Also tune 8 PeSTO term scale factors (uses full C eval)")
    parser.add_argument("--engine", "-e", required=True, metavar="PATH", help="Engine binary for eval-batch (e.g. ./engine)")
    parser.add_argument("--seed", type=int, default=42, help="Random seed")


# @method run
# @desc Load fen,result CSV; run Texel tuning (L-BFGS-B or coordinate descent); write JSON and print C snippet.
# @param args Parsed namespace from add_arguments.
def run(args: argparse.Namespace) -> None:
    """Run Texel tuning from parsed arguments (from add_arguments)."""
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

    engine_proc = subprocess.Popen(
        [args.engine, "eval-batch"],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        text=True,
        bufsize=1,
    )

    def get_evals_fn(params: np.ndarray) -> np.ndarray:
        n_engine = 10 + (8 if args.tune_weights else 0)
        params_for_engine = params[:n_engine]
        return _engine_evals(
            engine_proc.stdin,
            engine_proc.stdout,
            fens,
            params_for_engine,
            args.tune_k,
            args.tune_weights,
        )

    print(f"Using engine for eval: {args.engine}", file=sys.stderr)
    if args.tune_weights:
        print("Tuning mg/eg + 8 term weights (full PeSTO eval).", file=sys.stderr)

    # Initial point and bounds
    x0 = np.concatenate([DEFAULT_MG[:5], DEFAULT_EG[:5]])
    bounds = [(1, 2000)] * 5 + [(1, 2000)] * 5
    if args.tune_weights:
        x0 = np.concatenate([x0, np.ones(8)])
        bounds.extend([(0.01, 10.0)] * 8)
    if args.tune_k:
        x0 = np.append(x0, 1.0)
        bounds.append((0.1, 10.0))

    def obj(x: np.ndarray) -> float:
        return cross_entropy_loss(x, fens, results, tune_k=args.tune_k, get_evals=get_evals_fn)

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

    engine_proc.stdin.close()
    engine_proc.wait()

    mg = np.zeros(6)
    eg = np.zeros(6)
    mg[:5] = res.x[:5]
    eg[:5] = res.x[5:10]
    off = 10
    weights = None
    if args.tune_weights:
        weights = np.array(res.x[off:off + 8], dtype=np.float64)
        off = 18
    k = float(res.x[off]) if args.tune_k else 1.0

    print(f"\nFinal loss: {res.fun:.2f}", file=sys.stderr)
    print("Tuned mg_value (pawn..king):", np.round(mg).astype(int).tolist(), file=sys.stderr)
    print("Tuned eg_value (pawn..king):", np.round(eg).astype(int).tolist(), file=sys.stderr)
    if args.tune_weights:
        print("Tuned term weights:", file=sys.stderr)
        for i, name in enumerate(TERM_NAMES):
            print(f"  {name}: {weights[i]:.4f}", file=sys.stderr)
    if args.tune_k:
        print(f"Tuned K: {k:.4f}", file=sys.stderr)

    out = {
        "mg_value": [int(round(x)) for x in mg],
        "eg_value": [int(round(x)) for x in eg],
        "loss": float(res.fun),
        "n_positions": len(fens),
    }
    if args.tune_weights:
        out["term_weights"] = {name: float(weights[i]) for i, name in enumerate(TERM_NAMES)}
    if args.tune_k:
        out["k"] = k
    if args.output:
        with open(args.output, "w") as f:
            json.dump(out, f, indent=2)
        print(f"Wrote {args.output}", file=sys.stderr)

    # Print C snippet for pesto.c (mg/eg only; term weights are used in eval-batch)
    print("\n/* Paste into src/eval/pesto.c (replace static const int mg_value[6] / eg_value[6]) */")
    print("static const int mg_value[6] = {", ", ".join(str(int(round(x))) for x in mg), "};")
    print("static const int eg_value[6] = {", ", ".join(str(int(round(x))) for x in eg), "};")


def main() -> None:
    parser = argparse.ArgumentParser(description="Texel tuning for PeSTO piece values")
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
