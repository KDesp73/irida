import json
import argparse
from .wrapper import ChessEngineWrapper
from .model import LLM
from .server import main as api_main
from .config import CONFIG

STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

def parse_cli():
    parser = argparse.ArgumentParser()
    parser.add_argument("--fen", default=STARTING_FEN, type=str, help="Specify the fen position")
    parser.add_argument("--engine", default="./irida", type=str, help="Specify the engine we will use")
    parser.add_argument("--serve", action="store_true", help="Start the REST API")
    parser.add_argument("--host", default="0.0.0.0", type=str, help="Specify the host")
    parser.add_argument("--port", default=CONFIG["server"]["port"], type=int, help="Specify the port")
    return parser.parse_args()


def main():
    args = parse_cli()

    if args.serve:
        api_main(args.host, args.port)
        return

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
