#!/usr/bin/env python3 

import chess, chess.pgn
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser("pgntofen")
    parser.add_argument("pgn", help="A pgn game file", type=str)
    args = parser.parse_args()

    pgn = open(args.pgn)
    game = chess.pgn.read_game(pgn)
    board = game.board()

    for move in game.mainline_moves():
        board.push(move)
        print(board.fen())


