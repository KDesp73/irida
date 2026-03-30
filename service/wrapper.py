from __future__ import annotations

import queue
import subprocess
import threading
import time
from typing import Optional


class ChessEngineWrapper:
    """
    UCI engine subprocess. Uses a stdout reader thread so we are not blocked on
    select() (Windows-incompatible for pipes) and stderr is discarded to avoid
    deadlocks if the engine logs to stderr.
    """

    def __init__(self, executable_path):
        self.engine = subprocess.Popen(
            [executable_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.DEVNULL,
            text=True,
            bufsize=1,
        )
        self._line_queue: queue.Queue[Optional[str]] = queue.Queue()

        def _read_stdout() -> None:
            try:
                assert self.engine.stdout is not None
                for line in iter(self.engine.stdout.readline, ""):
                    self._line_queue.put(line)
            finally:
                self._line_queue.put(None)

        self._stdout_thread = threading.Thread(target=_read_stdout, daemon=True)
        self._stdout_thread.start()
        self._init_uci()

    def _get_line(self, timeout: float) -> Optional[str]:
        try:
            raw = self._line_queue.get(timeout=timeout)
        except queue.Empty:
            return None
        if raw is None:
            return None
        return raw.strip()

    def _init_uci(self) -> None:
        self.send_command("uci")
        deadline = time.time() + 15.0
        while time.time() < deadline:
            line = self._get_line(timeout=min(1.0, deadline - time.time()))
            if line is None:
                continue
            if "uciok" in line:
                break
        self.send_command("isready")
        deadline = time.time() + 15.0
        while time.time() < deadline:
            line = self._get_line(timeout=min(1.0, deadline - time.time()))
            if line is None:
                continue
            if "readyok" in line:
                return

    def send_command(self, command: str) -> None:
        if self.engine.poll() is not None:
            return
        if self.engine.stdin is None:
            return
        self.engine.stdin.write(f"{command}\n")
        self.engine.stdin.flush()

    def get_output(self, timeout: float = 0.1) -> list[str]:
        lines: list[str] = []
        end = time.time() + timeout
        while time.time() < end:
            remaining = end - time.time()
            if remaining <= 0:
                break
            line = self._get_line(timeout=remaining)
            if line is None:
                break
            if line:
                lines.append(line)
        return lines

    def search_position(self, position_cmd: str, go_cmd: str, deadline_sec: float = 120.0) -> dict:
        """
        Send a full UCI position line (e.g. position startpos moves e2e4 or position fen ...)
        and a go line (e.g. go depth 6). Returns move/score/pv from engine output.
        """
        self.send_command(position_cmd.strip())
        self.send_command(go_cmd.strip())

        engine_stats: dict = {"move": None, "score": "0", "pv": ""}
        end = time.time() + deadline_sec

        while time.time() < end:
            remaining = min(2.0, end - time.time())
            if remaining <= 0:
                break
            line = self._get_line(timeout=remaining)
            if line is None:
                continue

            if line.startswith("info"):
                parts = line.split()
                if "score" in parts:
                    try:
                        idx = parts.index("cp")
                        engine_stats["score"] = parts[idx + 1]
                    except ValueError:
                        if "mate" in parts:
                            engine_stats["score"] = "Mate in " + parts[parts.index("mate") + 1]
                if "pv" in parts:
                    engine_stats["pv"] = " ".join(parts[parts.index("pv") + 1 :])

            elif line.startswith("bestmove"):
                parts = line.split()
                if len(parts) >= 2 and parts[1] != "(none)":
                    engine_stats["move"] = parts[1]
                return engine_stats

        return engine_stats

    def get_best_move(self, fen: str, depth: int = 6) -> dict:
        """Convenience: analyze from a FEN string."""
        return self.search_position(f"position fen {fen}", f"go depth {depth}")

    def quit(self) -> None:
        self.send_command("quit")
        try:
            self.engine.wait(timeout=2.0)
        except subprocess.TimeoutExpired:
            self.engine.terminate()
