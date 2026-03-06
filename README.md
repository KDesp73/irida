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

**Custom NNUE training:** To train your own NNUE net (loadable via UCI option EvalFile), see the [nnue_training](nnue_training/README.md) Python module: it documents the pipeline (nnue-pytorch, data, export) and provides a script to generate FEN,score data from this engine.

---

> This engine is a work-in-progress and being developed with learning, experimentation, and future research in mind.

