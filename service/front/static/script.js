let socket, board, game = new Chess();
const container = document.getElementById("main-container");
const statusLight = document.getElementById("status-light");
const llmLoader = document.getElementById("llm-loader");
const llmExp = document.getElementById("llm-explanation");

function syncFenFromBoard() {
    const el = document.getElementById("fenInput");
    if (el) el.value = game.fen();
}

/** PGN result token for headers */
function pgnResultToken() {
    if (game.in_checkmate()) return game.turn() === "w" ? "0-1" : "1-0";
    if (game.in_draw()) return "1/2-1/2";
    return "*";
}

function buildPgnDocument() {
    const date = new Date().toISOString().slice(0, 10).replace(/-/g, ".");
    const result = pgnResultToken();
    const body = game.pgn({ max_width: 79 }).trim();
    const movesLine = body ? `${body} ${result}` : result;
    return [
        `[Event "Irida"]`,
        `[Site "?"]`,
        `[Date "${date}"]`,
        `[Round "?"]`,
        `[White "?"]`,
        `[Black "?"]`,
        `[Result "${result}"]`,
        ``,
        movesLine
    ].join("\n");
}

function downloadPgn() {
    const text = buildPgnDocument();
    const blob = new Blob([text], { type: "text/plain;charset=utf-8" });
    const url = URL.createObjectURL(blob);
    const a = document.createElement("a");
    a.href = url;
    a.download = `irida-${new Date().toISOString().slice(0, 10)}.pgn`;
    a.click();
    URL.revokeObjectURL(url);
    log("PGN downloaded.", "sys");
}

function copyPgn() {
    const text = buildPgnDocument();
    navigator.clipboard.writeText(text).then(
        () => log("PGN copied to clipboard.", "sys"),
        () => log("Could not copy PGN (clipboard permission?).", "result")
    );
}

function loadFenFromField() {
    const el = document.getElementById("fenInput");
    if (!el) return;
    const raw = el.value.trim();
    if (!raw) {
        log("FEN field is empty.", "result");
        return;
    }
    const ok = game.load(raw);
    if (!ok) {
        log("Invalid FEN — check the string (6 fields).", "result");
        return;
    }
    board.position(game.fen());
    refreshMoveList();
    syncFenFromBoard();
    send(`position fen ${game.fen()}`);
    log("Position loaded from FEN.", "sys");
}

function refreshMoveList() {
    const el = document.getElementById("move-list");
    if (!el) return;
    const history = game.history();
    if (history.length === 0) {
        el.textContent = "";
        return;
    }
    const rows = [];
    for (let i = 0; i < history.length; i += 2) {
        const n = Math.floor(i / 2) + 1;
        const w = history[i];
        const b = history[i + 1];
        rows.push(b ? `${n}. ${w} ${b}` : `${n}. ${w}`);
    }
    el.textContent = rows.join("  ");
}

function log(text, type = '') {
    const c = document.getElementById("console");
    const cls = ["log-line"];
    if (type === 'in') cls.push("log-line--in");
    if (type === 'out') cls.push("log-line--out");
    if (type === 'sys') cls.push("log-line--sys");
    if (type === 'result') cls.push("log-line--result");

    const row = document.createElement("div");
    row.className = cls.join(" ");
    row.innerHTML = text;
    c.appendChild(row);
    c.scrollTop = c.scrollHeight;
}

function setThinking(isThinking) {
    if (isThinking) {
        container.classList.add("thinking");
        statusLight.innerText = "Thinking";
        llmLoader.hidden = false;
        llmExp.hidden = true;
    } else {
        container.classList.remove("thinking");
        statusLight.innerText = "Idle";
        llmLoader.hidden = true;
        llmExp.hidden = false;
    }
}

function connect() {
    const proto = window.location.protocol === "https:" ? "wss:" : "ws:";
    socket = new WebSocket(`${proto}//${window.location.host}/ws`);

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
                    refreshMoveList();
                    syncFenFromBoard();
                    checkStatus();
                }
            }
            setThinking(false);
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
    refreshMoveList();
    syncFenFromBoard();
    if (game.game_over()) {
        checkStatus();
        return;
    }

    // Full FEN keeps server/engine in sync (UCI "position startpos moves ..." was not tracked server-side).
    send(`position fen ${game.fen()}`);

    const d = document.getElementById("depth").value;
    setThinking(true);
    setTimeout(() => send(`go depth ${d}`), 50);
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
    refreshMoveList();
    syncFenFromBoard();
    send("ucinewgame");
    send(`position fen ${game.fen()}`);
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
    const setBoardWidth = () => Math.min(window.innerWidth * 0.92, 480);
    const bw = setBoardWidth();
    $("#board").css("width", bw + "px");
    board = Chessboard("board", {
        draggable: true,
        position: "start",
        width: bw,
        onDrop: onDrop,
        pieceTheme: "https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png"
    });
    $(window).on("resize", () => {
        const w = setBoardWidth();
        $("#board").css("width", w + "px");
        board.resize();
    });
    syncFenFromBoard();
    connect();
});
