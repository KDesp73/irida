import subprocess
import select

class ChessEngineWrapper:
    def __init__(self, executable_path):
        self.engine = subprocess.Popen(
            [executable_path],
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            stderr=subprocess.PIPE,
            text=True,
            bufsize=1  # Line buffered
        )

    def send_command(self, command):
        """Sends a string command to the C engine."""
        if self.engine.poll() is None:  # Check if process is still running
            print(f"> {command}")
            self.engine.stdin.write(f"{command}\n")
            self.engine.stdin.flush()

    def get_output(self, timeout=2.0):
        """Reads the engine's response until it goes quiet."""
        lines = []
        while True:
            # Wait to see if there is data to read
            rlist, _, _ = select.select([self.engine.stdout], [], [], timeout)
            if rlist:
                line = self.engine.stdout.readline().strip()
                if line:
                    lines.append(line)
                else:
                    break
            else:
                break # Timeout reached
        return lines

    def get_best_move(self, fen, depth: int = 6):
        self.send_command(f"position fen {fen}")
        self.send_command(f"go depth {depth}")
        
        engine_stats = {
            "move": None,
            "score": "0",
            "pv": ""
        }
        
        while True:
            line = self.engine.stdout.readline().strip()
            if not line:
                continue
                
            print(f"< {line}") # Debugging: watch irida think
            
            if line.startswith("info"):
                parts = line.split()
                # Capture score
                if "score" in parts:
                    try:
                        idx = parts.index("cp")
                        engine_stats["score"] = parts[idx + 1]
                    except ValueError:
                        # Handle 'mate' scores if they occur
                        if "mate" in parts:
                            engine_stats["score"] = "Mate in " + parts[parts.index("mate") + 1]

                # Capture Principal Variation (the 'why')
                if "pv" in parts:
                    engine_stats["pv"] = " ".join(parts[parts.index("pv") + 1:])
            
            elif line.startswith("bestmove"):
                engine_stats["move"] = line.split()[1]
                return engine_stats

    def quit(self):
        self.send_command("quit")
        self.engine.terminate()
