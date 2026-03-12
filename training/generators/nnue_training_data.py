#!/usr/bin/env python3
# @module generators.nnue_training_data
# @desc Generate FEN,score_cp CSV by running the engine in UCI (go depth N) per position.
"""
Generate NNUE training data from this engine via UCI.

Reads FENs from a file (or stdin), runs the engine with "go depth N" for each
position, parses the score from the last "info ... score cp N" line, and
writes output as CSV: fen,score_cp (centipawns from engine's perspective).

The output format (fen,score_cp) can be converted to the binary format
expected by nnue-pytorch; see nnue-pytorch docs for the exact schema.
"""

from __future__ import annotations

import argparse
import subprocess
import sys
import re


import re


# @method run_engine_score
# @desc Run engine in UCI: position fen, go depth N; parse last info score cp/mate.
# Returns centipawn score (or large value for mate) or None on failure.
# @param engine_path Path to engine executable.
# @param fen Position FEN.
# @param depth Search depth.
# @returns int|None Centipawn score or None.
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


    return last_cp


# @method add_arguments
# @desc Registers --engine, --depth, --fen-file, --output for the data command.
# @param parser ArgumentParser or subparser.
# @returns None
def add_arguments(parser: argparse.ArgumentParser) -> None:
    """Add data-generation arguments to a parser or subparser."""
    parser.add_argument("--engine", required=True, help="Path to engine executable (UCI)")
    parser.add_argument("--depth", type=int, default=6, help="Search depth for each position (default: 6)")
    parser.add_argument("--fen-file", help="Input file with one FEN per line (default: stdin)")
    parser.add_argument("--output", "-o", default="-", help="Output CSV file (default: stdout)")


# @method run
# @desc Generate FEN,score_cp CSV: read FENs, run engine per position, write CSV.
# @param args Parsed namespace from add_arguments.
# @returns None
def run(args: argparse.Namespace) -> None:
    """Generate FEN,score CSV from parsed arguments (from add_arguments)."""
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


def main() -> None:
    parser = argparse.ArgumentParser(description="Generate FEN,score data from engine via UCI.")
    add_arguments(parser)
    run(parser.parse_args())


if __name__ == "__main__":
    main()
