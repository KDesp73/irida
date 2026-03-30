let socket, board, game = new Chess();
let engineLoaded = false;

const container = document.querySelector(".shell");
const llmLoader = document.getElementById("llm-loader");
const llmExp = document.getElementById("llm-explanation");

function humanColor() {
    return document.getElementById("playerSide").value === "white" ? "w" : "b";
}

function setBoardWidth() {
    return Math.min(window.innerWidth * 0.92, 480);
}

function rebuildBoard() {
    const bw = setBoardWidth();
    $("#board").css("width", bw + "px");
    if (board) {
        try {
            board.destroy();
        } catch (e) {
            $("#board").empty();
        }
        board = null;
    }
    board = Chessboard("board", {
        position: game.fen(),
        width: bw,
        draggable: engineLoaded,
        orientation: document.getElementById("playerSide").value,
        onDrop: onDrop,
        pieceTheme: "https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png"
    });
    setBoardLockedState();
    updateStatusTurn();
    updateEngineTurnButton();
}

function setBoardLockedState() {
    const wrap = document.getElementById("board-wrap");
    if (!wrap) return;
    wrap.classList.toggle("board-wrap--locked", !engineLoaded);
}

function updateStatusTurn() {
    const el = document.getElementById("status-turn");
    if (!el) return;
    if (!engineLoaded) {
        el.textContent = " · load engine";
        return;
    }
    if (game.game_over()) {
        el.textContent = "";
        return;
    }
    const human = humanColor();
    el.textContent = game.turn() === human ? " · you" : " · engine";
}

function updateEngineTurnButton() {
    const btn = document.getElementById("btnEngineTurn");
    if (!btn) return;
    const human = humanColor();
    const thinking = container.classList.contains("thinking");
    const needEngine =
        engineLoaded && !game.game_over() && !thinking && game.turn() !== human;
    btn.hidden = !needEngine;
}

function syncFenFromBoard() {
    const el = document.getElementById("fenInput");
    if (el) el.value = game.fen();
}

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
    updateStatusTurn();
    updateEngineTurnButton();
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

function log(text, type = "") {
    const c = document.getElementById("console");
    const cls = ["log-line"];
    if (type === "in") cls.push("log-line--in");
    if (type === "out") cls.push("log-line--out");
    if (type === "sys") cls.push("log-line--sys");
    if (type === "result") cls.push("log-line--result");

    const row = document.createElement("div");
    row.className = cls.join(" ");
    row.innerHTML = text;
    c.appendChild(row);
    c.scrollTop = c.scrollHeight;
}

function setThinking(isThinking) {
    const phase = document.getElementById("status-phase");
    if (isThinking) {
        container.classList.add("thinking");
        if (phase) phase.textContent = "Thinking";
        llmLoader.hidden = false;
        llmExp.hidden = true;
    } else {
        container.classList.remove("thinking");
        if (phase) phase.textContent = "Idle";
        llmLoader.hidden = true;
        llmExp.hidden = false;
    }
    updateEngineTurnButton();
    updateStatusTurn();
}

function handleEngineStatusLine(data) {
    if (typeof data !== "string") return;
    const lower = data.toLowerCase();
    if (lower.includes("engine loaded:")) {
        engineLoaded = true;
        rebuildBoard();
    }
    if (lower.includes("error loading engine")) {
        engineLoaded = false;
        rebuildBoard();
    }
}

function connect() {
    const proto = window.location.protocol === "https:" ? "wss:" : "ws:";
    socket = new WebSocket(`${proto}//${window.location.host}/ws`);

    socket.onopen = () => log("CONNECTED TO BACKEND", "sys");
    socket.onclose = () => log("CONNECTION CLOSED", "sys");

    socket.onmessage = (event) => {
        const data = event.data;

        try {
            const obj = JSON.parse(data);
            if (obj.type === "engineList") {
                const sel = document.getElementById("engineSelect");
                sel.innerHTML = "";
                obj.engines.forEach((f) => sel.add(new Option(f.name, f.name)));
                return;
            }
            if (obj.type === "explanation") {
                setThinking(false);
                llmExp.innerHTML = `<strong>Move: ${obj.move} (Score: ${obj.score})</strong><br>${obj.text}`;
                return;
            }
        } catch (e) {}

        handleEngineStatusLine(data);

        log(`< ${data}`, "in");

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
    log(`> ${cmd}`, "out");
    socket.send(cmd);
}

function loadEngine() {
    const selectElement = document.getElementById("engineSelect");
    if (selectElement.selectedIndex < 0) return;

    const engineName = selectElement.options[selectElement.selectedIndex].text;

    send(`load ${engineName}`);
    setTimeout(() => send("uci"), 200);
    setTimeout(() => send("isready"), 500);
}

function requestEngineMove() {
    if (!engineLoaded || game.game_over()) return;
    const human = humanColor();
    if (game.turn() === human) return;
    send(`position fen ${game.fen()}`);
    const d = document.getElementById("depth").value;
    setThinking(true);
    setTimeout(() => send(`go depth ${d}`), 50);
}

function onDrop(source, target) {
    if (!engineLoaded) {
        log("Load an engine first.", "result");
        return "snapback";
    }

    const human = humanColor();
    const piece = game.get(source);
    if (!piece || piece.color !== human) {
        return "snapback";
    }

    const move = game.move({ from: source, to: target, promotion: "q" });
    if (!move) return "snapback";

    board.position(game.fen());
    refreshMoveList();
    syncFenFromBoard();
    if (game.game_over()) {
        checkStatus();
        updateStatusTurn();
        updateEngineTurnButton();
        return;
    }

    send(`position fen ${game.fen()}`);

    const d = document.getElementById("depth").value;
    setThinking(true);
    setTimeout(() => send(`go depth ${d}`), 50);
}

function checkStatus() {
    const moveColor = game.turn() === "b" ? "Black" : "White";
    if (game.in_checkmate()) {
        log(`*** GAME OVER: ${moveColor} Checkmated ***`, "result");
    } else if (game.in_draw()) {
        log(`*** GAME OVER: Draw ***`, "result");
    }
}

function resetGame() {
    game.reset();
    refreshMoveList();
    syncFenFromBoard();
    rebuildBoard();
    send("ucinewgame");
    send(`position fen ${game.fen()}`);
    log("GAME RESET", "sys");
}

function updateOrientation() {
    if (board) board.orientation(document.getElementById("playerSide").value);
    updateStatusTurn();
    updateEngineTurnButton();
}

function sendRaw() {
    const el = document.getElementById("raw");
    send(el.value);
    el.value = "";
}

$(document).ready(() => {
    const bw = setBoardWidth();
    $("#board").css("width", bw + "px");
    board = Chessboard("board", {
        draggable: false,
        position: "start",
        width: bw,
        onDrop: onDrop,
        pieceTheme: "https://chessboardjs.com/img/chesspieces/wikipedia/{piece}.png"
    });
    $(window).on("resize", () => {
        const w = setBoardWidth();
        $("#board").css("width", w + "px");
        if (board) board.resize();
    });
    syncFenFromBoard();
    setBoardLockedState();
    updateStatusTurn();
    updateEngineTurnButton();
    connect();
});
