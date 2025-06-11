#!/usr/bin/env python3 

import chess
import chess.pgn
import argparse

if __name__ == "__main__":
    parser = argparse.ArgumentParser("pgntofen")
    parser.add_argument("pgn", help="A PGN game file", type=str)
    args = parser.parse_args()

    with open(args.pgn) as pgn_file:
        game_number = 1
        while True:
            game = chess.pgn.read_game(pgn_file)
            if game is None:
                break  # End of file

            print(f"# Game {game_number}")
            board = game.board()
            for move in game.mainline_moves():
                board.push(move)
                print(board.fen())
            print()
            game_number += 1
