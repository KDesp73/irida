from llama_cpp import Llama
from .translator import translate_move

class LLM():
    def __init__(
        self,
        gguf: str = "./data/models/Llama-3.2-1B-Instruct-Q3_K_M.gguf",
        n_ctx: int = 2048,
        n_threads: int = 4,
        temperature: float = 0.2
    ):
        self.llm = Llama(
            model_path=gguf,
            n_ctx=n_ctx,
            n_threads=n_threads,
            verbose=False
        )
        self.temperature = temperature
        
    def explain(self, fen, move, score, pv):
        readable_move = translate_move(fen, move)
        side_to_move = "White" if " w " in fen else "Black"
        
        prompt = f"""<|start_header_id|>system<|end_header_id|>
You are a direct, no-nonsense Chess Coach.
Rules:
1. Do not use flowery language like "masterstroke" or "testament".
2. Use exactly 2 or 3 short sentences.
3. If the move is a simple development or trade, say so plainly.
4. Do not mention "points" or "engine score" in the text.<|eot_id|>
<|start_header_id|>user<|end_header_id|>
Explain this move briefly:
FEN: {fen}
Move: {readable_move}
Side: {side_to_move}
Engine Evaluation: {score}
Predicted Line: {pv}

Why is this move played?<|eot_id|>
<|start_header_id|>assistant<|end_header_id|>"""
        
        response = self.llm(
            prompt,
            max_tokens=256,
            stop=["<|eot_id|>"],
            echo=False,
            temperature=self.temperature
        )

        return response['choices'][0]['text'].strip()
