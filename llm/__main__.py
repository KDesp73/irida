import json
import argparse
from .wrapper import ChessEngineWrapper
from .model import LLM

STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

def parse_cli():
    parser = argparse.ArgumentParser()
    parser.add_argument("--fen", default=STARTING_FEN, type=str, help="Specify the fen position")
    parser.add_argument("--engine", default="./irida", type=str, help="Specify the engine we will use")
    return parser.parse_args()


def main():
    args = parse_cli()

    fen = args.fen
    bridge = ChessEngineWrapper(args.engine)
    move = bridge.get_best_move(fen)

    llm = LLM()
    explanation = llm.explain(fen, move["move"], move["score"], move["pv"])

    print(json.dumps({
        "bestmove": move,
        "explanation": explanation
    }, indent=2, ensure_ascii=False))


if __name__ == "__main__":
    main()
