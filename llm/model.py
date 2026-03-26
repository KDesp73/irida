from llama_cpp import Llama

class LLM():
    def __init__(self, gguf: str = "./data/models/Llama-3.2-1B-Instruct-Q3_K_M.gguf"):
        self.llm = Llama(
            model_path=gguf,
            n_ctx=2048,
            n_threads=4
        )
        
    def explain(self, fen, move, score, pv):
        side_to_move = "White" if " w " in fen else "Black"
        
        prompt = f"""<|start_header_id|>system<|end_header_id|>
You are an elite Chess Grandmaster and world-class coach. 
Your task is to provide high-level strategic commentary based on engine analysis.
Avoid stating the obvious. Instead, translate the evaluation score and the 
Principal Variation (PV) into human concepts like 'king safety', 'pawn structure', 
'tempo', and 'piece activity'.<|eot_id|>
<|start_header_id|>user<|end_header_id|>
Context for Analysis:
- **Board State (FEN):** {fen}
- **Engine Recommendation:** {move}
- **Evaluation:** {score}
- **Engine's Calculated Line (PV):** {pv}
- **Player to Move:** {side_to_move}

In a single, professional paragraph, explain the strategic "why" behind {move}. 
Connect the move to the predicted continuation {pv}. 
Speak with the authority and conciseness of a Grandmaster.<|eot_id|>
<|start_header_id|>assistant<|end_header_id|>"""
        
        # Change stop token to <|eot_id|> for Llama 3.2
        response = self.llm(
            prompt,
            max_tokens=256,
            stop=["<|eot_id|>"],
            echo=False,
            temperature=0.7
        )

        return response['choices'][0]['text'].strip()
