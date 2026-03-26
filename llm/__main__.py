from .bridge import ChessEngineWrapper
from .model import get_llm_explanation

if __name__ == "__main__":
    fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
    bridge = ChessEngineWrapper("./irida")
    move = bridge.get_best_move(fen)
    print(f"Engine says: {move}")

    explanation = get_llm_explanation(fen, move["move"], move["score"], move["pv"])
    print(f"LLM says: {explanation}")

