# NNUE Training for This Engine

This Python module provides **scripts and documentation** for training a custom NNUE network that the engine can load via the **EvalFile** UCI option. The engine uses [nnue-probe](https://github.com/dshawul/nnue-probe) and expects a **Stockfish-format .nnue file**.

---

## Unified CLI

All entrypoints live under a single CLI. From the repo root:

```bash
python3 -m training                    # list commands
python3 -m training train ...          # NNUE-style net training
python3 -m training texel ...          # Texel piece-value tuning
python3 -m training texel-weights ...  # PeSTO term-weight tuning
python3 -m training data ...           # generate FEN,score CSV from engine
python3 -m training convert IN OUT     # .pt → .nnue
python3 -m training pgn2texel ...      # PGN → fen,result CSV
```

Add `-h` after any command for its options (e.g. `python3 -m training texel -h`).

---

## Texel tuning (PeSTO piece values)

**Texel tuning** minimizes the cross-entropy between the engine’s evaluation (via a sigmoid) and the game result (1 = white wins, 0.5 = draw, 0 = black wins) on a dataset of positions. This tunes the **mg_value** and **eg_value** arrays in `src/eval/pesto.c` (piece values for material + PST).

The tuner uses the **engine’s eval-batch** by default (no duplicate eval logic in Python). Build the engine first.

1. **Dataset**: CSV with columns `fen,result`. Create it by hand or from PGN:
   ```bash
   pip install python-chess   # optional, for PGN extraction
   make training.texel.data TEXEL_PGN=/path/to/games.pgn
   ```
   Or build a CSV yourself: one line per position, `fen,result` (result 0, 0.5, or 1 from White’s perspective).

2. **Run the tuner** (from repo root, engine built):
   ```bash
   pip install numpy scipy   # scipy recommended; numpy-only fallback available
   make training.texel.tune TEXEL_CSV=training/out/texel_positions.csv
   ```
   Or: `python3 -m training texel --data positions.csv --engine ./engine --output params.json --iter 1000`  
   Omit `--engine` to use the Python eval clone (slower, for when the engine is not built).

3. **Apply the result**: The script prints a C snippet. Paste the `mg_value` and `eg_value` lines into `src/eval/pesto.c` (replace the existing `static const int mg_value[6]` and `eg_value[6]`), then rebuild.

- **Make targets**: `training.texel.deps`, `training.texel.data` (from PGN), `training.texel.tune`.
- **Optional**: `--tune-k` tunes the sigmoid scale K as well (default K=1).

### Tuning PeSTO term weights

You can also tune the **weights** of the eight PeSTO terms (material+PST, pawn structure, mobility, king safety, piece activity, space, threats, endgame) from the same CSV. The engine outputs a per-term breakdown per position; the script finds scale factors that minimize cross-entropy.

```bash
make training.texel.tune_weights TEXEL_CSV=training/out/texel_positions.csv
# Or: python3 -m training texel-weights --data positions.csv --engine ./engine --output weights.json
```

Result is JSON with one weight per term. You can then apply these scales in `src/eval/pesto.c` (e.g. multiply each term by its weight) if you want the engine to use them.

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
| 2 | Obtain or generate a dataset (FEN + score or WDL); use `generate_data` for engine-based data. |
| 3 | Run training (`easy_train.py` or `train.py`) to produce a checkpoint. |
| 4 | Export the checkpoint to Stockfish .nnue format. |
| 5 | Set the engine’s UCI option **EvalFile** to the new .nnue path. |

---

## Verification

After training, run a few positions with the new EvalFile and compare to an existing net (e.g. from `make nn` in the main repo) to sanity-check that the engine runs and scores look reasonable.
