let socket, board, game = new Chess();
const container = document.getElementById("main-container");
const statusLight = document.getElementById("status-light");
const llmLoader = document.getElementById("llm-loader");
const llmExp = document.getElementById("llm-explanation");

function log(text, type = '') {
    const c = document.getElementById("console");
    let color = "inherit";
    let weight = "normal";

    if (type === 'in') color = "#60a5fa";
    if (type === 'out') color = "#fbbf24";
    if (type === 'sys') color = "#4ade80";
    if (type === 'result') { 
        color = "#f43f5e";
        weight = "bold";
    }

    c.innerHTML += `<div style="color:${color}; font-weight:${weight}">${text}</div>`;
    c.scrollTop = c.scrollHeight;
}

function setThinking(isThinking) {
    if (isThinking) {
        container.classList.add("thinking");
        statusLight.innerText = "THINKING";
        llmLoader.style.display = "block";
        llmExp.style.display = "none";
    } else {
        container.classList.remove("thinking");
        statusLight.innerText = "IDLE";
        llmLoader.style.display = "none";
        llmExp.style.display = "block";
    }
}

function connect() {
    // Use the dynamic host to match FastAPI port
    socket = new WebSocket(`ws://${window.location.host}/ws`);

    socket.onopen = () => log("CONNECTED TO BACKEND", 'sys');
    socket.onclose = () => log("CONNECTION CLOSED", 'sys');

    socket.onmessage = (event) => {
        const data = event.data;

        // Handle JSON (Engine List or LLM Explanation)
        try {
            const obj = JSON.parse(data);
            if (obj.type === "engineList") {
                const sel = document.getElementById("engineSelect");
                sel.innerHTML = "";
                obj.engines.forEach(f => sel.add(new Option(f.name, f.name)));
                return;
            }
            if (obj.type === "explanation") {
                setThinking(false); // Analysis arrived
                llmExp.innerHTML = `<strong>Move: ${obj.move} (Score: ${obj.score})</strong><br>${obj.text}`;
                return;
            }
        } catch (e) {}

        // Handle raw UCI strings
        log(`< ${data}`, 'in');

        if (data.startsWith("bestmove")) {
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
            }
        }
    };
}

function send(cmd) {
    if (!socket || socket.readyState !== 1) return;
    log(`> ${cmd}`, 'out');
    socket.send(cmd);
}

function loadEngine() {
    const selectElement = document.getElementById("engineSelect");
    if (selectElement.selectedIndex === -1) return;
    
    const engineName = selectElement.options[selectElement.selectedIndex].text;
    
    // Sequence: Load -> UCI -> IsReady
    send(`load ${engineName}`);
    setTimeout(() => send("uci"), 200);
    setTimeout(() => send("isready"), 500);
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
    // Slight delay to ensure the position command is processed
    setTimeout(() => send(`go depth ${d}`), 100);
}

function checkStatus() {
    let moveColor = game.turn() === 'b' ? 'Black' : 'White';
    if (game.in_checkmate()) {
        log(`*** GAME OVER: ${moveColor} Checkmated ***`, 'result');
    } else if (game.in_draw()) {
        log(`*** GAME OVER: Draw ***`, 'result');
    }
}

function resetGame() {
    game.reset();
    board.start();
    send("ucinewgame");
    log("GAME RESET", "sys");
}

function updateOrientation() {
    board.orientation(document.getElementById("playerSide").value);
}

function sendRaw() {
    const el = document.getElementById("raw");
    send(el.value);
    el.value = "";
}

$(document).ready(() => {
    board = Chessboard('board', {
        draggable: true,
        position: 'start',
        onDrop: onDrop,
        pieceTheme: 'https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png'
    });
    connect();
});
