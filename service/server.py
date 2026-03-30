import asyncio
import os
from typing import Optional

from fastapi import FastAPI, WebSocket, WebSocketDisconnect
from fastapi.staticfiles import StaticFiles
from fastapi.middleware.cors import CORSMiddleware
from fastapi.responses import FileResponse

from .config import CONFIG
from .model import LLM
from .wrapper import ChessEngineWrapper

try:
    import chess
except ImportError:
    chess = None

STARTING_FEN = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"


def fen_from_uci_position(line: str, fallback: str) -> str:
    """Derive FEN from a UCI position line for the LLM / translator."""
    s = line.strip()
    if s.startswith("position fen "):
        rest = s[len("position fen ") :].strip()
        return rest.split(" moves ")[0].strip()
    if chess and s.startswith("position startpos"):
        board = chess.Board()
        tail = s[len("position startpos") :].strip()
        if tail.startswith("moves"):
            for tok in tail[5:].split():
                if tok:
                    try:
                        board.push_uci(tok)
                    except ValueError:
                        break
        return board.fen()
    return fallback


app = FastAPI()

app.add_middleware(
    CORSMiddleware,
    allow_origins=["*"],
    allow_methods=["*"],
    allow_headers=["*"],
)

llm = None
if CONFIG.get("llm", {}).get("enabled"):
    llm = LLM()

BASE_DIR = os.path.dirname(os.path.abspath(__file__))
FRONT_DIR = os.path.join(BASE_DIR, "front")
STATIC_DIR = os.path.join(FRONT_DIR, "static")


@app.websocket("/ws")
async def uci_bridge(websocket: WebSocket):
    await websocket.accept()

    engine: Optional[ChessEngineWrapper] = None
    last_position = "position startpos"
    current_fen = STARTING_FEN

    engines = [{"name": e.split("/")[-1], "path": e} for e in CONFIG["engines"]]
    await websocket.send_json({"type": "engineList", "engines": engines})

    try:
        while True:
            message = await websocket.receive_text()
            line = message.strip()

            if line.startswith("position "):
                last_position = line
                current_fen = fen_from_uci_position(line, current_fen)

            elif line.startswith("load "):
                engine_name = line[5:]
                target = next((e for e in engines if e["name"] == engine_name), None)

                if target:
                    if engine:
                        engine.quit()

                    raw_path = target["path"]
                    if os.path.exists(raw_path):
                        engine_path = os.path.abspath(raw_path)
                    else:
                        engine_path = raw_path

                    try:
                        engine = await asyncio.to_thread(ChessEngineWrapper, engine_path)
                        await websocket.send_text(f"info string engine loaded: {engine_path}")
                    except Exception as e:
                        await websocket.send_text(f"info string error loading engine: {str(e)}")
                continue

            if engine:
                if line.startswith("go"):
                    depth = 6
                    if "depth" in line:
                        try:
                            depth = int(line.split("depth", 1)[-1].strip().split()[0])
                        except (ValueError, IndexError):
                            pass

                    go_cmd = line if line.startswith("go ") else f"go depth {depth}"

                    stats = await asyncio.to_thread(
                        engine.search_position, last_position, go_cmd
                    )

                    mv = stats.get("move")
                    if mv:
                        await websocket.send_text(f"bestmove {mv}")
                    else:
                        await websocket.send_text("bestmove (none)")

                    if llm and mv:
                        explanation = await asyncio.to_thread(
                            llm.explain,
                            current_fen,
                            mv,
                            stats.get("score", "0"),
                            stats.get("pv", ""),
                        )
                        await websocket.send_json(
                            {
                                "type": "explanation",
                                "move": mv,
                                "text": explanation,
                                "score": stats.get("score", "0"),
                            }
                        )
                else:
                    await asyncio.to_thread(engine.send_command, line)
                    out_lines = await asyncio.to_thread(engine.get_output, 0.25)
                    for out_line in out_lines:
                        await websocket.send_text(out_line)

    except WebSocketDisconnect:
        if engine:
            await asyncio.to_thread(engine.quit)
        print("Client disconnected")


@app.get("/")
async def get_index():
    return FileResponse(os.path.join(FRONT_DIR, "index.html"))


app.mount("/static", StaticFiles(directory=STATIC_DIR), name="static")


def main(host: str, port: int):
    import uvicorn

    uvicorn.run(app, host=host, port=port)


if __name__ == "__main__":
    main("0.0.0.0", 8000)
