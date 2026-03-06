#!/usr/bin/env python3
"""
Generate NNUE training data from this engine via UCI.

Reads FENs from a file (or stdin), runs the engine with "go depth N" for each
position, parses the score from the last "info ... score cp N" line, and
writes output as CSV: fen,score_cp (centipawns from engine's perspective).

Usage:
  python3 -m training.generate_data --engine ./engine --depth 6 --fen-file positions.txt --output data.csv
  cat fens.txt | python3 -m training.generate_data --engine ./engine --depth 4 --output data.csv

The output format (fen,score_cp) can be converted to the binary format
expected by nnue-pytorch; see nnue-pytorch docs for the exact schema.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import re


def run_engine_score(engine_path: str, fen: str, depth: int) -> int | None:
    """Send position and go depth to engine; return centipawn score or None on failure."""
    proc = subprocess.Popen(
        [engine_path],
        stdin=subprocess.PIPE,
        stdout=subprocess.PIPE,
        stderr=subprocess.DEVNULL,
        text=True,
        bufsize=1,
    )
    # UCI handshake
    proc.stdin.write("uci\n")
    proc.stdin.write("isready\n")
    proc.stdin.write(f"position fen {fen}\n")
    proc.stdin.write(f"go depth {depth}\n")
    proc.stdin.flush()

    last_cp: int | None = None
    cp_re = re.compile(r"info .* score (?:cp (\d+)|mate ([+-]?\d+))")
    for line in proc.stdout:
        line = line.strip()
        m = cp_re.search(line)
        if m:
            if m.group(1) is not None:
                last_cp = int(m.group(1))
            else:
                mate = int(m.group(2))
                last_cp = 100000 if mate > 0 else -100000
        if line.startswith("bestmove "):
            break
    proc.stdin.write("quit\n")
    proc.stdin.flush()
    proc.stdin.close()
    proc.wait()
    return last_cp


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate FEN,score data from engine via UCI.")
    parser.add_argument("--engine", required=True, help="Path to engine executable (UCI)")
    parser.add_argument("--depth", type=int, default=6, help="Search depth for each position (default: 6)")
    parser.add_argument("--fen-file", help="Input file with one FEN per line (default: stdin)")
    parser.add_argument("--output", "-o", default="-", help="Output CSV file (default: stdout)")
    args = parser.parse_args()

    fen_source = open(args.fen_file) if args.fen_file else sys.stdin
    out = open(args.output, "w") if args.output != "-" else sys.stdout

    try:
        out.write("fen,score_cp\n")
        for line in fen_source:
            fen = line.strip()
            if not fen or fen.startswith("#"):
                continue
            score = run_engine_score(args.engine, fen, args.depth)
            if score is not None:
                out.write(f'"{fen}",{score}\n')
            else:
                sys.stderr.write(f"Warning: no score for FEN: {fen[:50]}...\n")
    finally:
        if args.fen_file:
            fen_source.close()
        if args.output != "-":
            out.close()


if __name__ == "__main__":
    main()
