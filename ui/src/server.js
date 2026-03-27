const WebSocket = require("ws");
const { spawn } = require("child_process");
const fs = require('fs');
const yaml = require('js-yaml');
const path = require("path");
const axios = require("axios");

const CONFIG_DIR = path.resolve(__dirname, "..", "config.yml");
let CONFIG = null;

function load_config() {
  try {
    const fileContents = fs.readFileSync(CONFIG_DIR, 'utf8');
    CONFIG = yaml.load(fileContents);
  } catch (e) {
    console.error('Error loading YAML config:', e);
  }
}
load_config();

const PORT = CONFIG.server.port || 8765;
const wss = new WebSocket.Server({ port: PORT });
console.log("UCI bridge listening on", PORT);

wss.on("connection", (ws) => {
  let engine = null;
  let currentFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

  let lastScore = "0.0";
  let lastPv = "";

  const engines = CONFIG.engines.map(e => ({
    name: e.split("/").at(-1), 
    path: e
  }));
  ws.send(JSON.stringify({ type: "engineList", engines }));

  ws.on("message", (msg) => {
    const line = msg.toString().trim();

    if (line.startsWith("position fen ")) {
      currentFen = line.replace("position fen ", "").split(" moves")[0];
    }

    if (line.startsWith("load ")) {
      const name = line.slice(5);
      const engineEntry = engines.find((e) => e.name == name);
      if (!engineEntry) return;

      if (engine) engine.kill();

      try {
        engine = spawn(engineEntry.path);

        engine.stdout.on("data", async (data) => {
          const lines = data.toString().split("\n");

          for (let l of lines) {
            const trimmed = l.trim();
            if (!trimmed) continue;

            if (trimmed.startsWith("info ")) {
              const scoreMatch = trimmed.match(/score cp (-?\d+)/);
              const mateMatch = trimmed.match(/score mate (-?\d+)/);
              const pvMatch = trimmed.match(/ pv (.+)/);

              if (scoreMatch) lastScore = (parseInt(scoreMatch[1]) / 100).toFixed(2);
              if (mateMatch) lastScore = `Mate in ${mateMatch[1]}`;
              if (pvMatch) lastPv = pvMatch[1];
            }

            if (trimmed.startsWith("bestmove ")) {
              const bestMove = trimmed.split(" ")[1];

              ws.send(trimmed);

              try {
                const llmResponse = await axios.post(`${CONFIG.llm.prefix}/analyze`, {
                  fen: currentFen,
                  move: bestMove,
                  score: lastScore,
                  pv: lastPv
                });

                ws.send(JSON.stringify({
                  type: "explanation",
                  move: bestMove,
                  text: llmResponse.data.explanation,
                  score: lastScore
                }));
              } catch (err) {
                console.error("LLM API Error:", err.message);
              }
              continue; // Don't send the raw bestmove twice
            }

            ws.send(trimmed);
          }
        });

        engine.on("close", () => ws.send("info string engine exited"));
        ws.send("info string engine loaded");
      } catch (e) {
        ws.send("info string failed to start engine");
      }
      return;
    }

    if (engine) engine.stdin.write(line + "\n");
  });

  ws.on("close", () => { if (engine) engine.kill(); });
});
