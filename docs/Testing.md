# Engine Testing and ELO Measurement

This document describes how to measure engine strength and run regression tests.

## Unit tests (test.h)

The engine uses the [IncludeOnly/test.h](https://github.com/IncludeOnly/test.h) library with the bundled loader script. Tests are data-driven: each test has a `.c` file (implementation) and a `.ctd` file (one line per case; each line is the C argument list for the test function).

### Running tests

```bash
make test
```

This regenerates headers from `.ctd` files, builds the test binary (`check`), and runs all batches. For a release build:

```bash
make test type=RELEASE
```

### Running a single batch

```bash
make test.batch n=1   # batch 1: test_draws
make test.batch n=2   # batch 2: test_eval
make test.batch n=3   # batch 3: test_search
```

### Regenerating test headers

After editing a `.ctd` file, headers are regenerated automatically when you run `make test`. To regenerate without building:

```bash
make test.generate
```

This runs `scripts/loader -d test -H "extern/IncludeOnly/test.h" -L "castro.h"`.

### Test batches

| Batch | Tests | Description |
|-------|--------|-------------|
| 1 | test_draws | `is_draw()`: 50-move, insufficient material (K vs K, K+B, etc.) |
| 2 | test_eval | `pesto_eval()`: score in expected range for start position, e4, and K vs K |
| 3 | test_search | `search_root()`: completes without crashing at depth 2–3 |

## Prerequisites

- **cutechess-cli**: run engine-vs-engine games. Install e.g. via:
  - macOS: `brew install cutechess` (or build from [cutechess/cutechess-cli](https://github.com/cutechess/cutechess-cli))
  - Linux: package `cutechess-cli` or build from source
- **Ordo** or **BayesElo**: compute ELO from game results (PGN).
  - Ordo: [fhub/bayeselo](https://www.remi-coulom.fr/BayesElo/) or [michiguel/BayesElo](https://github.com/michiguel/BayesElo)
  - [Ordo](https://github.com/michiguel/Ordo) is another common tool

## Quick gauntlet (cutechess-cli)

From the project root, build the engine in release mode then run:

```bash
./scripts/run-gauntlet.sh
```

Or manually, example with a reference engine (adjust paths):

```bash
cutechess-cli \
  -engine name=engine cmd=./engine \
  -engine name=reference cmd=/path/to/other_engine \
  -each tc=10+0.1 \
  -games 100 \
  -pgnout gauntlet.pgn
```

## Time controls

- **10+0.1**: 10 seconds per game + 0.1s increment per move. Good for quick regression.
- **60+0.6** or **60+0**: longer TC for more reliable ELO (e.g. 2500 target).

## Computing ELO

After running games, save the PGN (e.g. `gauntlet.pgn`). Then:

**BayesElo** (typical usage):

```bash
bayeselo < gauntlet.pgn
# or
echo "gauntlet.pgn" | bayeselo
```

**Ordo**:

```bash
ordo -p gauntlet.pgn -o ordo.txt
```

Establish a baseline (e.g. after Phase 1 with PeSTO), then re-run after adding NNUE and Syzygy to measure gain.

## Regression testing

Before/after changes:

1. Run a small gauntlet (e.g. 50 games at 10+0.1) vs a fixed reference.
2. Compute ELO; note the delta.
3. Fix any regressions before merging.

## Target: 2500 ELO

Rough targets (approximate):

- Depth ~12–14 at 10+0.1 on modern hardware is a reasonable goal.
- NNUE usually gives a large ELO jump over hand-crafted eval.
- Syzygy improves endgame play; measure with endgame-heavy tests or long games.

### Tuning notes

- **Time management**: Time allocation uses 90% of `(time_left / moves) + increment` to keep a reserve; adjust in [src/uci/commands.c](src/uci/commands.c) (`parse_go_command`) if the engine flags or leaves too much time.
- **Search parameters**: LMR, null-move (R=3), and aspiration window are in [src/search/search.c](src/search/search.c) and [include/search.h](include/search.h). Tune only after establishing a baseline; change one thing at a time and re-run gauntlets.
- **Hash**: Increase the **Hash** UCI option (e.g. 128–256 MB) for longer time controls so the transposition table is effective.
- **ELO measurement**: Run 100+ games at fixed TC (e.g. 60+0.6) vs a known engine, then compute ELO with BayesElo or Ordo. Compare before/after NNUE and Syzygy to confirm gains.
