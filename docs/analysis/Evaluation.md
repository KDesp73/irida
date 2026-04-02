## Hand-crafted evaluation

### Meaning of “hand-crafted” in this codebase

The default UCI path sets `engine.eval = ` `evaluation` (`apps/irida.c`). That entry point is `pesto_eval_impl` with no breakdown output: a **PeSTO-style** core (tuned MG/EG piece values plus piece–square tables, tapered by game phase) plus several extra terms (pawns, rooks, mobility, king zone, activity, space, threats, endgame tweaks). The source file cites the [PeSTO article](https://www.chessprogramming.org/PeSTO's_Evaluation_Function).

`docs/Evaluation.md` lists the same term buckets; the following matches the implementation in `src/eval/eval.c`.

### Score convention

The implementation builds **`score_white`**: terms are expressed as **white minus black** (or white bonus / black penalty in helpers).

The returned value is **side-to-move relative**:

```894:894:src/eval/eval.c
    int score = board->turn == COLOR_WHITE ? score_white : -score_white;
```

So `evaluation(board)` follows the usual engine rule: **positive favors the side to move**.

`eval_breakdown()` fills `EvalBreakdown` with terms **also flipped by side to move** when `board->turn` is black (`out->total` is set to `±score_white` to match `evaluation()`). **Rook activity** is included in `score_white` / `out->total` but **has no field in `EvalBreakdown`**, and `log_breakdown()` omits king safety and rooks, so printed lines need not sum to `total` unless rook (and missing log lines) are accounted for.

### Layer 1: PeSTO material + PST (tapered)

**Piece values** (pawn..queen; king 0) are separate midgame and endgame arrays:

```71:72:src/eval/eval.c
static const int mg_value[6] = { 82, 337, 365, 477, 1025, 0 };
static const int eg_value[6] = { 94, 281, 297, 512, 936, 0 };
```

**PSTs**: `mg_*_table` / `eg_*_table` per piece type (pawn through king), 64 squares each, in **white’s** board orientation.

**Phase**: `gamephase_inc[12]` adds weight per occupied piece encoding (pawns 0; N/B 1; R 2; Q 4; both colors). `game_phase` is accumulated, capped at 24, then `eg_phase = 24 - game_phase`.

**Runtime tables**: `pesto_init()` calls `rebuild_tables()`, which fills `mg_table[12][64]` and `eg_table[12][64]`: white uses `mg_val[p] + mg_pesto_table[p][sq]`; black uses the same values with PST index **`flip_sq(sq) = sq ^ 56`**.

**Tapered PST+material score** (white − black):

\[
\texttt{material\_pst} = \frac{mg\_score \cdot game\_phase + eg\_score \cdot eg\_phase}{24}
\]

`pesto_material_pst_eval_white()` returns **only** that tapered sum, **from White’s perspective** (no side-to-move flip).

### Layer 2: Plain material (`material_eval`)

`material_eval(board)` is **added** on top. It (`src/eval/material.c`) counts **bitboards** with **100, 320, 330, 500, 900** per pawn..queen (king 0). The full eval therefore **overlaps conceptually** with material already embedded in the PeSTO MG/EG tables; the two scales are not identical.

### Layer 3: Heuristic terms

Each term below feeds **`score_white`** (then the sign flip for black to move).

**Pawn structure** (`evaluate_pawn_structure`): passed pawns (cone on adjacent files), doubled pawns per file, isolated pawns (no friendly pawns on adjacent files).

**Rooks** (`evaluate_rooks`): bonus on 7th/8th rank (white) / equivalent for black; open-file bonus; large bonus for rook on same file as enemy king. **Not exposed as its own `EvalBreakdown` field.**

**Mobility** (`evaluate_mobility`): grid-based pseudo-mobility for N/B/R/Q/K and simple pawn steps/captures; scaled by **`MOBILITY_WEIGHT` (4)**.

**King safety** (`evaluate_king_safety`): `game_phase` is unused. The code rewards **friendly pieces within Manhattan distance ≤ 2 of the enemy king** (±30 per such piece in the loop structure described in the source).

**Piece activity** (`evaluate_piece_activity`): small bonuses for white N/B/R/Q on ranks `r ∈ [2,6]` and black equivalents on `r ∈ [1,5]`.

**Space** (`evaluate_space`): advanced pawns and pieces in the central 4×4; difference scaled by **`SPACE_WEIGHT` (2)**.

**Threats** (`evaluate_threats`): pawn and sliding/jumping attacks on enemies, weighted by victim value (`/10` for pawns, `/8` for others); difference scaled by **`THREAT_WEIGHT` (3)**.

**Endgame** (`evaluate_endgame_terms`): bishop pair ±15; if `game_phase <= 8`, extra **`(white_pawns - black_pawns) * 4`**.

### Final sum

```884:893:src/eval/eval.c
    int score_white = material 
                    + material_pst 
                    + pawn_structure 
                    + rook_activity 
                    + mobility 
                    + king_safety
                    + piece_activity
                    + space 
                    + threats 
                    + endgame;
```

### Initialization and tuning

`pesto_init()` runs from `EngineInit` (`src/core.c`). `pesto_set_tune_values()` rebuilds tables from new `mg_val[6]` / `eg_val[6]` arrays for Texel-style workflows (documentation may still refer to `pesto.c`; the implementation lives in **`eval.c`** here).

### NNUE

When NNUE is loaded, `nnue_eval` may be used for play; on failure the implementation falls back to **`evaluation`** (`src/eval/nnue.c`).

### Summary

The hand-crafted eval is **PeSTO (tapered MG/EG PST and piece values)** plus **explicit material**, **pawn structure**, **rook heuristics**, **mobility**, **king-zone proximity**, **activity**, **space**, **threats**, and **late endgame / bishop-pair** terms, summed as white minus black and **negated when black is to move**. The main body is in `src/eval/eval.c`; `material_eval` is in `src/eval/material.c`.
