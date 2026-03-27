from fastapi import FastAPI, HTTPException
from pydantic import BaseModel
from fastapi.middleware.cors import CORSMiddleware
from typing import Optional, Any

from .model import LLM

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"], # TODO: restrict this in production
    allow_methods=["*"],
    allow_headers=["*"],
)

llm = LLM()

class MoveAnalysisRequest(BaseModel):
    fen: str
    move: str
    score: Any
    pv: Optional[str] = ""

@app.post("/analyze")
async def analyze_move(request: MoveAnalysisRequest):
    try:
        explanation = llm.explain(
            request.fen,
            request.move,
            request.score,
            request.pv
        )

        return {
            "move": request.move,
            "explanation": explanation,
            "evaluation": request.score
        }
    except Exception as e:
        print(f"Error: {e}")
        raise HTTPException(status_code=500, detail=str(e))


def main(host: str, port: int):
    import uvicorn
    uvicorn.run(app, host=host, port=port)


if __name__ == "__main__":
    main("0.0.0.0", 8000)
