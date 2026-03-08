#!/usr/bin/env python3
"""
Build a Texel tuning dataset from PGN: one FEN per game with the game result.

Result is from White's perspective: 1 = white wins, 0.5 = draw, 0 = black wins.
We sample one position per game (default: after 10 full moves, or use --ply).
Requires: pip install python-chess
"""

from __future__ import annotations

import argparse
import sys

try:
    import chess
    import chess.pgn
except ImportError:
    print("python-chess required: pip install python-chess", file=sys.stderr)
    sys.exit(1)


def result_to_white_score(result: str) -> float | None:
    if result in ("1-0", "1–0"):
        return 1.0
    if result in ("0-1", "0–1"):
        return 0.0
    if result in ("1/2-1/2", "½-½", "0.5-0.5"):
        return 0.5
    return None


def main() -> None:
    parser = argparse.ArgumentParser(description="Extract FEN,result from PGN for Texel tuning")
    parser.add_argument("--pgn", "-p", required=True, help="Input PGN file")
    parser.add_argument("--output", "-o", required=True, help="Output CSV: fen,result")
    parser.add_argument("--ply", type=int, default=20, help="Sample position after this many half-moves (default 20)")
    parser.add_argument("--max", type=int, default=0, help="Max games to process (0 = all)")
    args = parser.parse_args()

    count = 0
    with open(args.pgn) as pgn_file, open(args.output, "w") as out:
        out.write("fen,result\n")
        while True:
            game = chess.pgn.read_game(pgn_file)
            if game is None:
                break
            res = result_to_white_score(game.headers.get("Result", ""))
            if res is None:
                continue
            board = game.board()
            moves = list(game.mainline_moves())
            if len(moves) < args.ply:
                continue
            for i in range(args.ply):
                board.push(moves[i])
            fen = board.fen()
            out.write(f"{fen},{res}\n")
            count += 1
            if args.max and count >= args.max:
                break
    print(f"Wrote {count} positions to {args.output}", file=sys.stderr)


if __name__ == "__main__":
    main()
