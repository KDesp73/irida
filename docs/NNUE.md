# NNUE Evaluation

The engine supports an optional NNUE (Efficiently Updatable Neural Network) evaluator. When no NNUE network is loaded, evaluation falls back to the built-in PeSTO evaluator.

## UCI option

- **EvalFile** (string): Path to a Stockfish-format `.nnue` network file. Set via UCI `setoption name EvalFile value <path>`. After setting, the engine uses NNUE for evaluation when the backend is linked (see below).

## Using NNUE (optional backend)

To use NNUE evaluation you need a compatible probing library. One option is [nnue-probe](https://github.com/dshawul/nnue-probe) (GPL-3.0):

1. Clone and build nnue-probe:
   ```bash
   git clone https://github.com/dshawul/nnue-probe.git extern/nnue-probe
   cd extern/nnue-probe && make clean && make COMP=gcc
   ```

2. Build the engine with NNUE support:
   ```bash
   make clean
   make build.static type=RELEASE CFLAGS_EXTRA="-DUSE_NNUE_PROBE -Iextern/nnue-probe"
   # Then link the engine with -Lextern/nnue-probe -lnnueprobe (or add to build.mk)
   ```

3. Download a Stockfish NNUE file (e.g. from [Stockfish](https://stockfishchess.org/download/) or [nnue-pytorch](https://github.com/official-stockfish/nnue-pytorch)) and set the path in your GUI or via `setoption name EvalFile value <path>`.

Without linking nnue-probe, the engine still compiles and runs using PeSTO; the EvalFile option is stored but evaluation uses PeSTO until you build with an NNUE backend.
