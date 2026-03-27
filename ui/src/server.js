const WebSocket = require("ws")
const { spawn } = require("child_process")
const fs = require('fs');
const yaml = require('js-yaml');
const path = require("path")

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

const PORT = CONFIG.server.port || parseInt(process.argv[2], 10) || process.env.PORT || 8765
const wss = new WebSocket.Server({ port: PORT })
console.log("UCI bridge listening on", PORT)

wss.on("connection", (ws) => {
  let engine = null

  const engines = CONFIG.engines.map(e => ({
    name: e.split("/").at(-1), 
    path: e
  }));
  ws.send(JSON.stringify({ type: "engineList", engines }));

  ws.on("message", (msg) => {
    const line = msg.toString().trim()

    // Load engine
    if (line.startsWith("load ")) {
      const name = line.slice(5)
      const enginePath = engines.filter((e) => e.name == name)[0].path;
      console.log(`[INFO] Loading ${name} (${enginePath})`);

      if (engine) {
        engine.kill()
        engine = null
      }

      try {
        engine = spawn(enginePath)

        engine.stdout.on("data", (data) => {
          data.toString().split("\n").forEach(l => {
            if (l.trim().length > 0) ws.send(l.trim())
          })
        })

        engine.stderr.on("data", (data) => {
          ws.send("info string " + data.toString())
        })

        engine.on("close", () => {
          ws.send("info string engine exited")
        })

        ws.send("info string engine loaded")
      } catch (e) {
        ws.send("info string failed to start engine")
      }

      return
    }

    // Send UCI command to engine
    if (engine) {
      engine.stdin.write(line + "\n")
    }
  })

  ws.on("close", () => {
    if (engine) engine.kill()
  })
})
