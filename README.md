# Chess Engine

A UCI chess engine with **PeSTO evaluation**, **NNUE support**, and a **training toolkit** for tuning and net training. Built for learning and experimentation.

**TL;DR** — `make build.all` then `./engine`. Use [training/README.md](training/README.md) for Texel tuning and NNUE training.

---

## Get going in 3 steps

**1. Prerequisites** — C compiler (GCC or Clang), `make`. Optional: `bear` for `compile_commands.json`.

**2. Build**

```bash
make build.all
```

Release build (optimized, no sanitizers):

```bash
make build.all type=RELEASE
```

**3. Run**

```bash
./engine
```

At the UCI prompt try:

```
uci
position startpos
go depth 10
quit
```

Or point a GUI (Arena, Scid, etc.) at `./engine`.

---

## What you can do

| Goal | Command |
|------|---------|
| **Build** | `make build.all` |
| **Run tests** | `make test` |
| **Sanity check** (tests + one search) | `make test.engine` |
| **Engine vs engine** | `make gauntlet` *(needs cutechess-cli)* · override: `REF_ENGINE=path GAMES=50 TC=10+0.1` |
| **ELO from PGN** | [docs/Testing.md](docs/Testing.md) (BayesElo / Ordo) |
| **List all targets** | `make help` |

Unit tests auto-regenerate headers from `.ctd` files.

---

## Evaluation

- **PeSTO** — Hand-crafted eval (material, PST, pawn structure, mobility, etc.). Used when no NNUE is loaded.
- **NNUE** — The engine loads a net from the **EvalFile** UCI option (default: `nn/nn-04cf2b4ed1da.nnue`). You’ll see either `info string EvalFile loaded: ...` or `info string Failed to load EvalFile '...' (using PeSTO evaluation)`.
  - **Format:** nnue-probe supports the legacy **halfkp_256x2-32-32** format (e.g. [FireFather/halfkp_256x2-32-32-nets](https://github.com/FireFather/halfkp_256x2-32-32-nets)). Current Stockfish test-server nets use a newer format and **won’t load**.
  - **Nets:** Run `make deps.fetch` to download compatible nets into `nn/`. If loading fails, run the engine from the repo root or set EvalFile to an absolute path.

---

## Training & tuning

The [**training**](training/README.md) Python module lets you:

- **Texel tuning** — Tune PeSTO piece values and term weights from position–result CSV (uses the engine’s C eval).
- **NNUE pipeline** — Generate FEN,score data, train a net, convert to .nnue for EvalFile.

From repo root: `python3 -m training` for the CLI; `python3 -m training texel -h` for Texel options. Full docs: [training/README.md](training/README.md).

---

> This engine is a work-in-progress, developed for learning, experimentation, and future research.
