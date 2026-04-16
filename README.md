# Irida

A UCI compatible chess engine written in C

<img src="https://github.com/user-attachments/assets/8e2208bf-b31c-4ad8-af9f-8252af5e178d" width=500>

## Get Started

> Run `make` to see what's available

### Build

```bash
make irida
```

### Install

```bash
make install
```

## Engine Design

### Move Generation

Move generation is handled by [castro](https://github.com/KDesp73/castro)

### Evaluation

Irida scores positions in **centipawns** from the **side to move** perspective (positive = good for the player to move).

**Hand-crafted (default)** — `irida_Evaluation` implements a **PeSTO-style** core: separate midgame and endgame piece values, piece–square tables, and a tapered blend between phases (see [PeSTO on CPW](https://www.chessprogramming.org/PeSTO's_Evaluation_Function)). On top of that, `src/eval/eval.c` adds heuristics for **pawn structure**, **rooks** (open files, seventh rank, king file pressure), **mobility**, **king safety**, **piece activity**, **central space**, **threats**, and small **endgame** tweaks (e.g. bishop pair, pawn imbalance in late phase). Plain **material** from bitboard counts is also folded in (`src/eval/material.c`). For a term-by-term breakdown API, see `EvalBreakdown` in `include/eval.h` and the longer write-up in [`docs/analysis/Evaluation.md`](docs/analysis/Evaluation.md).

**NNUE (optional)** — A network in **`.nnue`** format can be loaded via the UCI option **`EvalFile`**. The code in `src/eval/nnue.c` wraps the linked **nnue-probe** library: the board is exported to FEN and scored in centipawns for the side to move. If no net is loaded, FEN validation fails, or evaluation fails, the engine **falls back to PeSTO**. The default UCI binary uses PeSTO; you can switch with **`seteval nnue`** (after loading a file), **`seteval pesto`**, or **`seteval material`** for material-only debugging (`src/uci/commands.c`).


### Search

The search is based on the negamax algorithm with plenty of optimizations.

+ Iterative Deepening
+ Alpha-Beta Pruning
+ Quiescence
+ Move Ordering (MVV-VLA)
+ Transposition Table
+ Null Move Pruning
+ Late Move Reductions
+ Aspiration Windows
+ Principal Variation Search

### Syzygy

Syzygy is enabled by default as long as the tablebase can be located


## License

[MIT](./LICENSE)

## Author

Konstantinos Despoindis ([KDesp73](https://github.com/KDesp73))

> This engine is a work-in-progress, developed with learning, experimentation, and future research in mind.
