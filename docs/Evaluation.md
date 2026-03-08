# Evaluation (PeSTO and NNUE)

## Current PeSTO terms

The hand-crafted eval (`pesto_eval`) combines:

- **Material + piece-square tables** (phase-interpolated MG/EG)
- **Pawn structure**: passed pawns, doubled pawns, isolated pawns
- **Mobility**: pseudo-legal moves per piece
- **King safety**: penalty for king in centre in middlegame
- **Piece activity**: bonus for pieces in the “active” ranks
- **Space**: pawns and pieces in the centre / advanced ranks
- **Threats**: bonus for attacking enemy pieces (by piece value)
- **Endgame**: bishop pair, extra pawn weight in late endgame

Use `pesto_eval_breakdown()` / `pesto_log_breakdown()` to see per-term values.

## Possible improvements (symmetric under board mirror)

Any new term must keep **eval(A, side) = eval(mirror(A), opposite side)** so symmetry tests pass.

- **Rook on open file** (no pawns) or **semi-open file** (no friendly pawns): small bonus (~8–15 cp). Implement with per-file pawn counts and same bonus for both colors so mirror gives the same value.
- **Tempo**: small bonus for side to move in middlegame (~5 cp). Symmetric (white +5 ↔ black +5 after mirror).
- **Knight outpost**: knight in enemy half that cannot be attacked by enemy pawns (~12 cp). Ensure the “can enemy pawn attack?” check is symmetric under mirror (use mirrored coordinates).
- **Backward pawns**: pawn that cannot be supported by a friendly pawn on an adjacent file and is behind the enemy pawns on that file; penalty.
- **Pawn islands**: count contiguous pawn groups per file; fewer islands is often better.
- **Rook behind passed pawn**: bonus when a rook supports an own passed pawn from behind.
- **King shelter**: bonus for pawns near the king in middlegame; more in endgame, encourage centralisation.

Tuning: keep weights small and re-run symmetry tests (`make test.batch n=2`) and Stockfish comparison (`test_eval_stockfish`) after adding terms.

## NNUE

The engine can use an NNUE net (UCI option **EvalFile**). At startup it tries to load the default path (`nn/nn-37f18f62d772.nnue`). You should see either:

- `info string EvalFile loaded: ...`  
- `info string Failed to load EvalFile '...' (using PeSTO evaluation)`

If loading fails:

1. Run the engine from the **repository root** so the relative path `nn/` is valid, or  
2. Set **EvalFile** to an **absolute path**:  
   `setoption name EvalFile value /path/to/net.nnue`

Fetch nets with `make deps.fetch` (downloads into `nn/`). The **nnue-probe** library must be built (`extern/nnue-probe`); `make build.all` does this. The `.nnue` file must match the architecture nnue-probe expects (Stockfish-format nets).
