from typing import Optional
from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse
import os

from .config import CONFIG
from .model import LLM
from .wrapper import ChessEngineWrapper

app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

# Initialize LLM only if enabled
llm = None
if CONFIG.get("llm", {}).get("enabled"):
    llm = LLM()

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
# Adjusted to match your "front" folder structure
FRONT_DIR = os.path.join(BASE_DIR, "front")
STATIC_DIR = os.path.join(FRONT_DIR, "static")

@app.websocket("/ws")
async def uci_bridge(websocket: WebSocket):
    # Accept any origin to prevent the 403 Forbidden error
    await websocket.accept()
    
    engine: Optional[ChessEngineWrapper] = None
    current_fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

    engines = [{"name": e.split("/")[-1], "path": e} for e in CONFIG["engines"]]
    await websocket.send_json({"type": "engineList", "engines": engines})

    try:
        while True:
            message = await websocket.receive_text()
            line = message.strip()

            if line.startswith("position fen "):
                current_fen = line.replace("position fen ", "").split(" moves")[0]

            elif line.startswith("load "):
                engine_name = line[5:]
                target = next((e for e in engines if e["name"] == engine_name), None)
                
                if target:
                    if engine: engine.quit()
                    
                    raw_path = target["path"]
                    # If it's a direct path to a file, make it absolute.
                    # If it's just a command (like 'stockfish'), leave it alone.
                    if os.path.exists(raw_path):
                        engine_path = os.path.abspath(raw_path)
                    else:
                        engine_path = raw_path
                    
                    try:
                        engine = ChessEngineWrapper(engine_path)
                        await websocket.send_text(f"info string engine loaded: {engine_path}")
                    except Exception as e:
                        await websocket.send_text(f"info string error loading engine: {str(e)}")
                continue

            if engine:
                if line.startswith("go"):
                    depth = 6
                    if "depth" in line:
                        try:
                            depth = int(line.split("depth")[-1].strip())
                        except: pass
                    
                    stats = engine.get_best_move(current_fen, depth=depth)
                    await websocket.send_text(f"bestmove {stats['move']}")

                    if llm:
                        explanation = llm.explain(
                            current_fen, 
                            stats["move"], 
                            stats["score"], 
                            stats["pv"]
                        )
                        await websocket.send_json({
                            "type": "explanation",
                            "move": stats["move"],
                            "text": explanation,
                            "score": stats["score"]
                        })
                else:
                    engine.send_command(line)
                    output = engine.get_output(timeout=0.1)
                    for out_line in output:
                        await websocket.send_text(out_line)

    except WebSocketDisconnect:
        if engine: engine.quit()
        print("Client disconnected")

# Specific route for index.html
@app.get("/")
async def get_index():
    return FileResponse(os.path.join(FRONT_DIR, "index.html"))

# Mount static files (ensure your index.html uses /static/script.js)
app.mount("/static", StaticFiles(directory=STATIC_DIR), name="static")

def main(host: str, port: int):
    import uvicorn
    uvicorn.run(app, host=host, port=port)

if __name__ == "__main__":
    main("0.0.0.0", 8000)
