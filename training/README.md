# Training & Tuning

Python tools for **improving this engine**: tune the PeSTO eval (Texel) or train/convert NNUE nets. One CLI, run from the repo root.

**Jump to:** [Texel tuning](#texel-tuning-pesto-piece-values) · [CLI reference](#unified-cli) · [NNUE in-repo](#training-without-nnue-pytorch-do-it-yourself) · [nnue-pytorch](#pipeline-overview-using-nnue-pytorch)

---

## Unified CLI

From the **repo root** (engine built when a command needs it):

```bash
python3 -m training                    # list commands
python3 -m training texel ...          # PeSTO piece + optional term-weight tuning
python3 -m training texel-weights ...  # PeSTO term weights only (fixed mg/eg)
python3 -m training data ...           # generate FEN,score CSV from engine
python3 -m training train ...          # NNUE-style net training
python3 -m training convert IN OUT     # .pt → .nnue
python3 -m training pgn2texel ...     # PGN → fen,result CSV for Texel
```

`python3 -m training texel -h` for all Texel options.

**Make shortcuts**

| Target | What it does |
|--------|----------------|
| `training.texel.deps` | `pip install numpy scipy` (+ optional python-chess) |
| `training.texel.data` | PGN → fen,result CSV (`TEXEL_PGN`, `TEXEL_CSV`) |
| `training.texel.tune` | Run Texel tuning (`TEXEL_CSV`, `ENGINE_PATH`) |
| `training.texel.tune_weights` | Tune 8 term weights only |
| `training.data` | Engine → FEN,score CSV for NNUE |
| `training.train` | Train .pt model |
| `training.convert` | .pt → .nnue |

---

## Texel tuning (PeSTO piece values)

Minimize **cross-entropy** between the engine’s eval (sigmoid) and game results (1 / 0.5 / 0 for white win / draw / black win). Tunes **mg_value** and **eg_value** in `src/eval/pesto.c`, and optionally the **8 PeSTO term weights**. All eval runs in the **C engine** (eval-batch)—no Python eval duplicate.

**Need:** Engine built, CSV with `fen,result`, and `pip install numpy scipy` (optional: `python-chess` for PGN).

### Quick start

```bash
# 1) Get a fen,result dataset (e.g. from PGN)
make training.texel.data TEXEL_PGN=path/to/games.pgn
# Default output: training/out/texel_positions.csv (override with TEXEL_CSV)

# 2) Run the tuner (--engine is required)
make training.texel.tune
# Or: python3 -m training texel --data training/out/texel_positions.csv --engine ./engine --output params.json

# 3) Paste the printed C snippet into src/eval/pesto.c (mg_value / eg_value), then rebuild
```

### Dataset

- **Format:** One line per position: `fen,result`. Result from White’s view: `0` (black wins), `0.5` (draw), `1` (white wins).
- **From PGN:** `make training.texel.data TEXEL_PGN=file.pgn` (optional: `pip install python-chess`). Override output with `TEXEL_CSV` (e.g. `training/out/texel-positions.csv`).

### Options and examples

| Option | Effect |
|--------|--------|
| `--engine` | **Required.** Path to engine binary (e.g. `./engine`). |
| `--tune-weights` | Also tune the 8 PeSTO term scale factors (full C eval). Output JSON includes `term_weights`. |
| `--tune-k` | Also tune the sigmoid scale K (default 1). |
| `--iter N` | Max optimization iterations (default 1000). |
| `--output FILE` | Write tuned params to JSON. |

```bash
# Piece values only
python3 -m training texel --data training/out/texel_positions.csv --engine ./engine --output params.json --iter 1000

# Piece values + 8 term weights (full C eval)
python3 -m training texel --data training/out/texel_positions.csv --engine ./engine --tune-weights --output params.json --iter 500

# Weights-only (fixed mg/eg)
make training.texel.tune_weights
# Or: python3 -m training texel-weights --data training/out/texel_positions.csv --engine ./engine --output weights.json
```

### Apply the result

- **mg_value / eg_value:** The script prints a C snippet; replace the existing `static const int mg_value[6]` and `eg_value[6]` in `src/eval/pesto.c`, then rebuild.
- **Term weights:** With `--tune-weights`, the JSON has `term_weights`. The engine uses them in eval-batch when given 18 ints (mg, eg, w0…w7). To use them in normal search you’d need to apply the scales in `pesto.c` (e.g. multiply each term by its weight).

### Tips

> **Weights stuck at 1.0?** The default may already fit your data. Try more `--iter` or a larger, more diverse CSV.  
> **Stable tuning** — Use thousands of positions when you can.

---

## Training without nnue-pytorch (do it yourself)

If you want to **train your own net without using Stockfish’s nnue-pytorch**:

1. **Generate data** with the engine:
   ```bash
   python3 -m training data --engine ./engine --depth 6 --fen-file positions.txt --output data.csv
   ```

2. **Train** with the custom trainer in this module (PyTorch only, no nnue-pytorch):
   ```bash
   pip install torch
   python3 -m training train --data data.csv --epochs 20 --output model.pt
   ```
   Use `--arch mlp` (default) for a simple 768→256→32→1 MLP, or `--arch halfkp` for a Stockfish-compatible HalfKP feature transformer + 512→32→1. HalfKP models export to a full .nnue (convert script writes the real feature transformer).

3. **Convert .pt to .nnue** so the engine can load it:
   ```bash
   python3 -m training convert model.pt model.nnue
   ```
   Or from the repo root: `make training.convert` (uses `training/out/model.pt` → `training/out/model.nnue`).  
   The script writes the Stockfish .nnue binary layout (header, LEB128 feature transformer placeholder, dense 512→32→1 from your trained weights). The feature transformer is filled with zeros, so evaluation may be weak; for stronger play, see [export_nnue.md](export_nnue.md).

4. **Load the net in the engine**: Set UCI option **EvalFile** to the path of your `.nnue` file (e.g. `training/out/model.nnue`). If the engine fails to load the file, nnue-probe may expect a different architecture/hash; see [export_nnue.md](export_nnue.md). Alternatively:
   - **Use your own inference**: Replace nnue-probe in the engine with code that loads your format (e.g. ONNX or a custom layout). See [export_nnue.md](export_nnue.md).

See **[export_nnue.md](export_nnue.md)** for more on the .nnue format and export options.

---

## Pipeline overview (using nnue-pytorch)

1. **Data** – Many positions (FEN) with target scores or WDL.
2. **Train** – Run Stockfish’s [nnue-pytorch](https://github.com/official-stockfish/nnue-pytorch) to produce checkpoints.
3. **Export** – Convert a checkpoint to Stockfish `.nnue` format.
4. **Use** – Set the engine’s UCI option `EvalFile` to the new `.nnue` path (no code changes).

---

## Step 1: Set up nnue-pytorch (Stockfish)

- **Repo:** https://github.com/official-stockfish/nnue-pytorch  
- **Docs:** https://official-stockfish.github.io/docs/nnue-pytorch-wiki/README.html  

Typical setup:

```bash
git clone https://github.com/official-stockfish/nnue-pytorch
cd nnue-pytorch
python3 -m venv venv
source venv/bin/activate   # or venv\Scripts\activate on Windows
pip install -r requirements.txt
# Build the data loader (see repo for CMake steps)
```

You need: Python 3.7+, C++ compiler, CMake, Make, PyTorch (CUDA optional), and optionally Stockfish + c-chess-cli for data generation and match evaluation.

---

## Step 2: Training data

You need a dataset of positions with target values in the format nnue-pytorch expects.

- **Use existing data:** e.g. [Stockfish training data](https://robotmoon.com/nnue-training-data/) (T80, etc.).
- **Generate from this engine:** Use the script in this module:

  ```bash
  # From the chess-engine repo root, with the engine built:
  python3 -m training data --engine ./engine --depth 6 --fen-file positions.txt --output data.csv
  ```

  This runs the engine in UCI, sends each FEN, runs `go depth N`, parses the score, and writes `fen,score` (or similar). You can then convert `data.csv` to the binary format nnue-pytorch expects (see nnue-pytorch docs).

---

## Step 3: Train

- **Easier:** `easy_train.py` in nnue-pytorch (handles many steps; may require Stockfish and c-chess-cli on PATH).
- **More control:** `train.py` with your dataset and config.

Training produces checkpoints (e.g. `.ckpt`). Use TensorBoard to monitor.

---

## Step 4: Export to .nnue

Use nnue-pytorch’s scripts to **convert a checkpoint to Stockfish .nnue format** (exact command is in the nnue-pytorch docs, e.g. a serialize/export script). The resulting file is what nnue-probe and this engine expect.

---

## Step 5: Load in the engine

- Copy the `.nnue` file to a path the engine can read (e.g. `nn/` or next to the binary).
- In UCI: `setoption name EvalFile value /path/to/your_net.nnue`
- Or set EvalFile in your GUI. No engine code changes are required.

---

## Summary

| Step | Action |
|------|--------|
| 1 | Set up nnue-pytorch (venv, deps, data loader build). |
| 2 | Obtain or generate a dataset (FEN + score or WDL); use `python3 -m training data` for engine-based data. |
| 3 | Run training (`easy_train.py` or `train.py`) to produce a checkpoint. |
| 4 | Export the checkpoint to Stockfish .nnue format. |
| 5 | Set the engine’s UCI option **EvalFile** to the new .nnue path. |

---

## Verification

After training, run a few positions with the new EvalFile and compare to an existing net (e.g. from `make nn` in the main repo) to sanity-check that the engine runs and scores look reasonable.
