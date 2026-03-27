let socket, board, game = new Chess();
const container = document.getElementById("main-container");
const statusLight = document.getElementById("status-light");

function log(text, type = '') {
    const c = document.getElementById("console");
    let color = "inherit";
    let weight = "normal";

    if (type === 'in') color = "#60a5fa";
    if (type === 'out') color = "#fbbf24"; // Amber
    if (type === 'sys') color = "#4ade80"; // Emerald
    if (type === 'result') { 
        color = "#f43f5e"; // Rose
        weight = "bold";
    }

    c.innerHTML += `<div style="color:${color}; font-weight:${weight}">${text}</div>`;
    c.scrollTop = c.scrollHeight;
}

function setThinking(isThinking) {
    if (isThinking) {
        container.classList.add("thinking");
        statusLight.innerText = "THINKING";
    } else {
        container.classList.remove("thinking");
        statusLight.innerText = "IDLE";
    }
}

function checkStatus() {
    let status = "";
    let moveColor = game.turn() === 'b' ? 'Black' : 'White';

    if (game.in_checkmate()) {
        status = `GAME OVER: ${moveColor} is in checkmate.`;
    } else if (game.in_draw()) {
        status = "GAME OVER: Draw.";
    } else {
        if (game.in_check()) {
            log(`${moveColor} is in check!`, 'sys');
        }
        return;
    }

    log(`*** ${status} ***`, 'result');
    setThinking(false);
}

function connect() {
    socket = new WebSocket("ws://127.0.0.1:8765");

    socket.onopen = () => log("CONNECTED TO BACKEND", 'sys');
    socket.onclose = () => log("CONNECTION CLOSED", 'sys');

    socket.onmessage = (event) => {
        const data = event.data;

        try {
            const obj = JSON.parse(data);
            if (obj.type === "engineList") {
                const sel = document.getElementById("engineSelect");
                sel.innerHTML = "";
                obj.engines.forEach(f => sel.add(new Option(f.name, f.path)));
                return;
            }
        } catch (e) {}

        try {
          const obj = JSON.parse(data);
          if(obj.type === "explanation") {
            const el = document.getElementById("llm-explanation");
            el.innerHTML = obj.text;
            return;
          }
        } catch (e) {}

        log(`< ${data}`, 'in');

        if (data.startsWith("bestmove")) {
            setThinking(false);
            const moveStr = data.split(" ")[1];
            if (moveStr && moveStr !== "(none)") {
                const move = game.move({
                    from: moveStr.substring(0, 2),
                    to: moveStr.substring(2, 4),
                    promotion: moveStr.length === 5 ? moveStr.substring(4, 5) : "q"
                });
                if (move) {
                    board.position(game.fen());
                    checkStatus();
                }
            } else {
                checkStatus();
            }
        }
    };
}

function send(cmd) {
    if (!socket || socket.readyState !== 1) return;
    log(`> ${cmd}`, 'out');
    socket.send(cmd);
}

function updateOrientation() {
    board.orientation(document.getElementById("playerSide").value);
}

function loadEngine() {
    const selectElement = document.getElementById("engineSelect");
    const engineName = selectElement.options[selectElement.selectedIndex].text;
    send(`load ${engineName}`);
    send("uci");
    send("isready");
}

function resetGame() {
    game.reset();
    board.start();
    send("ucinewgame");
    updateOrientation();
    log("GAME RESET", "sys");
}

function onDrop(source, target) {
    const move = game.move({ from: source, to: target, promotion: "q" });
    if (!move) return "snapback";

    board.position(game.fen());

    if (game.game_over()) {
        checkStatus();
        return;
    }

    const history = game.history({ verbose: true }).map(m => m.from + m.to).join(" ");
    send(`position startpos moves ${history}`);

    const d = document.getElementById("depth").value;
    setThinking(true);
    setTimeout(() => send(`go depth ${d}`), 50);
}

function sendRaw() {
    const el = document.getElementById("raw");
    send(el.value);
    el.value = "";
}

// Ensure DOM is ready before initializing Chessboard
$(document).ready(function () {
    board = Chessboard('board', {
        draggable: true,
        position: 'start',
        onDrop: onDrop,
        pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png'
    });
    updateOrientation();
    connect();
});
