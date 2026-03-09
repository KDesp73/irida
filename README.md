# Chess Engine Project

## Quick Start

### Prerequisites

- `gcc` with support for `-fsanitize`
- `make`
- `bear` (optional, for generating `compile_commands.json`)
- `linux`

### Building the Project

```bash
make build.all
```

For a release build (optimized, no sanitizers):

```bash
make build.all type=RELEASE
```

---

## Recipes (common tasks)

| Task | Command |
|------|---------|
| Build everything | `make build.all` |
| Run unit tests | `make test` |
| Run gauntlet (engine-vs-engine) | `make gauntlet` (optional: `REF_ENGINE=path GAMES=50 TC=10+0.1`) |
| Engine sanity check (tests + one search) | `make test.engine` |
| Compute ELO from PGN | See [docs/Testing.md](docs/Testing.md) (BayesElo / Ordo) |
| List all make targets | `make help` |

Gauntlet requires `cutechess-cli`. Unit tests regenerate headers from `.ctd` files automatically.

**License:** This project is licensed under the MIT License. See [LICENSE](LICENSE).  
Dependencies (castro, nnue-probe, Fathom, IncludeOnly) have their own licenses; see their repositories if you distribute binaries.

**Engine name:** The UCI engine name defaults to `chess-engine`. To override, build with `make CFLAGS="-DENGINE_NAME=\\\"MyEngine\\\"\" build.all` (or set `ENGINE_NAME` in [include/version.h](include/version.h)).

**NNUE evaluation:** The engine loads an NNUE net at startup from the **EvalFile** UCI option (default: `nn/nn-04cf2b4ed1da.nnue`). You will see either `info string EvalFile loaded: ...` or `info string Failed to load EvalFile '...' (using PeSTO evaluation)`. **nnue-probe** only supports the legacy **halfkp_256x2-32-32** format (e.g. nets from [FireFather/halfkp_256x2-32-32-nets](https://github.com/FireFather/halfkp_256x2-32-32-nets)); current Stockfish test-server nets use a newer format and will not load. Fetch compatible nets with `make deps.fetch` (downloads into `nn/`) and ensure nnue-probe is built (`make build.all`). If the path is wrong, run the engine from the repo root or set EvalFile to an absolute path.

**Custom NNUE training:** To train your own NNUE net (loadable via UCI option EvalFile), see the [training](training/README.md) Python module: it documents the pipeline (nnue-pytorch, data, export) and provides a script to generate FEN,score data from this engine.

---

> This engine is a work-in-progress and being developed with learning, experimentation, and future research in mind.

