#!/usr/bin/env python3
# @module __main__
# @desc Unified CLI for training and tuning. Dispatches to train, texel, texel-weights, data, convert, pgn2texel.
"""
Unified CLI for training and tuning.

Uses each module as a library: add_arguments(parser) + run(args).

  python3 -m training                    # show help and subcommands
  python3 -m training train ...          # NNUE-style net training
  python3 -m training texel ...          # Texel piece-value tuning
  python3 -m training texel-weights ...  # PeSTO term-weight tuning
  python3 -m training data ...           # generate FEN,score CSV from engine
  python3 -m training convert ...        # .pt -> .nnue
  python3 -m training pgn2texel ...      # PGN -> fen,result CSV for Texel
"""

from __future__ import annotations

import argparse
import sys


def main() -> None:
    parser = argparse.ArgumentParser(
        prog="training",
        description="Training and tuning CLI: NNUE training, Texel tuning, data generation, conversions.",
    )
    subparsers = parser.add_subparsers(dest="cmd", metavar="COMMAND", help="Subcommand to run")

    from training import train
    from training.texel import tuning as texel_tuning
    from training.texel import tune_weights as texel_tune_weights
    from training.generators import nnue_training_data as data_gen
    from training.conversions import pt2nnue as convert_mod
    from training.conversions import pgn2texel as pgn2texel_mod

    train.add_arguments(subparsers.add_parser("train", help="Train NNUE-style net from CSV (fen, score_cp)"))
    texel_tuning.add_arguments(subparsers.add_parser("texel", help="Texel tuning: tune PeSTO mg_value/eg_value from fen,result CSV"))
    texel_tune_weights.add_arguments(subparsers.add_parser("texel-weights", help="Tune PeSTO term weights from eval breakdown"))
    data_gen.add_arguments(subparsers.add_parser("data", help="Generate FEN,score_cp CSV from engine via UCI"))
    convert_mod.add_arguments(subparsers.add_parser("convert", help="Convert .pt model to legacy .nnue format (for this engine)"))
    pgn2texel_mod.add_arguments(subparsers.add_parser("pgn2texel", help="Extract fen,result CSV from PGN for Texel tuning"))

    args = parser.parse_args()
    if args.cmd is None:
        parser.print_help()
        sys.exit(0)

    if args.cmd == "train":
        train.run(args)
    elif args.cmd == "texel":
        texel_tuning.run(args)
    elif args.cmd == "texel-weights":
        texel_tune_weights.run(args)
    elif args.cmd == "data":
        data_gen.run(args)
    elif args.cmd == "convert":
        convert_mod.run(args)
    elif args.cmd == "pgn2texel":
        pgn2texel_mod.run(args)
    else:
        sys.exit(f"Unknown subcommand: {args.cmd}")


if __name__ == "__main__":
    main()
