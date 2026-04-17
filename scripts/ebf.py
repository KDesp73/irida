#!/usr/bin/env python3
"""
Effective branching / search growth from node counts at each iterative-deepening depth.

Uses UCI `info depth ... nodes ...` lines (cumulative nodes). For each completed depth d:

  delta[d] = nodes[d] - nodes[d-1]   (extra work for ID layer d)
  ratio[d] = delta[d] / delta[d-1]

Also prints N^(1/D) as a crude summary (not a true per-ply EBF for alpha-beta pruning).

Implementation:
  - If `chess` is installed: python-chess analysis API (same as scripts/arena).
  - Else: stdlib-only UCI over a pseudo-TTY so line-buffered output works.

Optional: pip install chess

Usage:
  ./scripts/ebf.py ./irida --depth 12
  ./scripts/ebf.py ./irida-q-tt-nmp-lmr-aw --depth 10 --fen "8/8/8/8/8/8/8/k6K w - - 0 1"
"""

from __future__ import annotations

import argparse
import math
import os
import re
import select
import subprocess
import sys
from dataclasses import dataclass
from pathlib import Path


@dataclass
class DepthSeries:
    depths: list[int]
    cum_nodes: list[int]


INFO_LINE = re.compile(r"info\s+depth\s+(\d+).*?nodes\s+(\d+)", re.IGNORECASE)


def parse_uci_log(text: str) -> DepthSeries:
    """Last nodes value wins for each depth (handles aspiration repeats)."""
    last: dict[int, int] = {}
    for line in text.splitlines():
        m = INFO_LINE.search(line)
        if m:
            last[int(m.group(1))] = int(m.group(2))
    depths = sorted(last.keys())
    return DepthSeries(depths=depths, cum_nodes=[last[d] for d in depths])


def collect_series_pty(engine_path: str, fen: str, max_depth: int, hash_mb: int) -> DepthSeries:
    """UCI session over PTY (no python-chess required). Unix/macOS only."""
    if sys.platform == "win32":
        raise RuntimeError("PTY driver is Unix-only; install python-chess: pip install chess")
    import pty

    master, slave = pty.openpty()
    proc = subprocess.Popen(
        [engine_path],
        stdin=slave,
        stdout=slave,
        stderr=slave,
        close_fds=True,
    )
    os.close(slave)
    buf = b""
    timeout = max(120.0, max_depth * 30.0)
    try:

        def w(s: str) -> None:
            os.write(master, (s + "\n").encode())

        w("uci")
        w(f"setoption name Hash value {hash_mb}")
        w("isready")
        if not fen or fen.lower() in ("startpos", "start"):
            w("position startpos")
        else:
            w("position fen " + fen.strip())
        w(f"go depth {max_depth}")

        while b"bestmove " not in buf:
            r, _, _ = select.select([master], [], [], timeout)
            if not r:
                raise TimeoutError(f"no engine output within {timeout}s")
            chunk = os.read(master, 65536)
            if not chunk:
                break
            buf += chunk
    finally:
        try:
            os.write(master, b"quit\n")
        except OSError:
            pass
        try:
            proc.wait(timeout=8)
        except subprocess.TimeoutExpired:
            proc.kill()
        os.close(master)

    return parse_uci_log(buf.decode("utf-8", errors="replace"))


def collect_series_chess(engine_path: str, board, max_depth: int, hash_mb: int) -> DepthSeries:
    import chess.engine

    last: dict[int, int] = {}
    with chess.engine.SimpleEngine.popen_uci(engine_path) as engine:
        engine.configure({"Hash": hash_mb})
        import chess as chess_mod

        limit = chess_mod.engine.Limit(depth=max_depth)
        with engine.analysis(board, limit) as analysis:
            for info in analysis:
                d = info.get("depth")
                n = info.get("nodes")
                if d is not None and n is not None:
                    last[int(d)] = int(n)

    depths = sorted(last.keys())
    return DepthSeries(depths=depths, cum_nodes=[last[d] for d in depths])


def summarize(series: DepthSeries, label: str) -> None:
    if not series.depths:
        print(f"{label}: no depth/nodes samples.", file=sys.stderr)
        return

    D = series.depths[-1]
    N = series.cum_nodes[-1]

    print(f"\n=== {label} ===")
    print(f"max depth reached: {D}, cumulative nodes: {N}")
    print(f"crude N^(1/D): {N ** (1.0 / D):.4f}  (weak single-number summary)\n")

    print(f"{'depth':>6} {'cum_nodes':>14} {'delta':>14} {'delta_ratio':>12}")
    print("-" * 52)
    prev_n = 0
    prev_delta = None
    ratios: list[float] = []
    for d, cum in zip(series.depths, series.cum_nodes):
        delta = cum - prev_n
        rtxt = ""
        if prev_delta is not None and prev_delta > 0:
            r = delta / prev_delta
            ratios.append(r)
            rtxt = f"{r:12.4f}"
        print(f"{d:>6} {cum:>14} {delta:>14} {rtxt}")
        prev_n = cum
        prev_delta = delta

    if ratios:
        gm = math.exp(sum(math.log(r) for r in ratios) / len(ratios))
        print(
            f"\ngeom mean of delta[d]/delta[d-1] for d=2..{D}: {gm:.4f}\n"
            "  (growth of incremental nodes per ID step; not ply-wise branching factor)"
        )


def main() -> int:
    ap = argparse.ArgumentParser(description="Search growth / EBF-style stats from UCI node counts.")
    ap.add_argument("engine", type=str, help="Path to UCI engine binary")
    ap.add_argument("--depth", type=int, default=12, help="Fixed-depth search (go depth N)")
    ap.add_argument("--fen", type=str, default="", help="FEN; omit for startpos")
    ap.add_argument("--hash", type=int, default=16, metavar="MB", help="Hash table size (MB)")
    ap.add_argument(
        "--pty",
        action="store_true",
        help="Force PTY UCI driver (ignore python-chess even if installed)",
    )
    args = ap.parse_args()

    engine_path = Path(args.engine).resolve()
    if not engine_path.is_file():
        print(f"Engine not found: {engine_path}", file=sys.stderr)
        return 1

    fen = args.fen.strip()

    use_chess = not args.pty
    if use_chess:
        try:
            import chess  # noqa: F401  # type: ignore
        except ImportError:
            use_chess = False

    label = f"{engine_path.name} depth={args.depth}"
    if use_chess:
        import chess

        if not fen or fen.lower() in ("startpos", "start"):
            board = chess.Board()
        else:
            board = chess.Board(fen)
        series = collect_series_chess(str(engine_path), board, args.depth, args.hash)
    else:
        series = collect_series_pty(str(engine_path), fen, args.depth, args.hash)

    summarize(series, label)
    return 0


if __name__ == "__main__":
    sys.exit(main())
