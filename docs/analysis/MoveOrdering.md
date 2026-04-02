# Move Ordering

## Role in the engine

`OrderFn` is a function pointer type: it receives the board, a **mutable** move array, the move count, the current **ply**, and an optional **TT / PV hint** move. The canonical implementation is `order_moves` in `src/moveordering.c`. The UCI binary sets `engine.order = order_moves` in `apps/irida.c` (and similarly in `apps/bench.c`).

Search calls ordering in three places:

1. **Root** (`search.c`): after legal move generation, with `iterationBestMove` as the hint (iterative deepening’s best move from the previous depth), then `prioritize_hash_move` forces that move to index `0` if it appears in the list (so PVS/LMR logic that cares about the “first” child aligns with the PV hint even if sort ties or encoding quirks matter).
2. **`negamax`**: after generating legal moves, with `tt_move` from the transposition table probe.
3. **`quiescence`**: on the capture list, with `NULL_MOVE` as the hint—so **no TT tier** in quiescence; ordering is capture-centric only.

```11:11:include/moveordering.h
typedef void (*OrderFn)(Board* board, Move moves[], size_t count, size_t ply, Move tt_move);
```

## Initialization and reset

`EngineInit` (`src/core.c`) calls `init_mvv_lva()` to fill a static `MVV_LVA[12][12]` table and `ordering_reset()` to clear killers and history. `ordering_reset` is also the right hook for `ucinewgame`-style resets (killers + `history_heuristic`).

**MVV-LVA table** (`init_mvv_lva`): for each attacker/victim piece type (12 = 6 white + 6 black in your indexing), the score is  
`PIECE_VALUES[victim % 6] * 10 - PIECE_VALUES[attacker % 6]`, so trades prefer **Most Valuable Victim / Least Valuable Attacker**, using the same `PIECE_VALUES` as in `core.h` (pawn..queen; king 0).

## Scoring tiers (single combined score, then `qsort`)

Each move gets one integer score. The design comment in code summarizes the priority:

```16:24:src/moveordering.c
/* Score tiers: TT >> captures >> quiet queen promo >> killers >> other promo >> history */
enum {
    SCORE_TT               = 30000000,
    SCORE_CAPTURE_BASE     = 25000000,
    SCORE_PROMO_QUIET_Q    = 23500000,
    SCORE_KILLER_0         = 22000000,
    SCORE_KILLER_1         = 21000000,
    SCORE_PROMO_QUIET_MIN  = 20500000,
    SCORE_PROMO_CAP_BONUS  = 80000,
};
```

Rough ordering:

1. **TT / hint move**  
   If `m == tt_move` and `tt_move != NULL_MOVE`, score is `SCORE_TT`. At interior nodes `tt_move` comes from the TT; at the root, the caller passes `iterationBestMove` so the previous ID best is treated like the hash move.

2. **Captures** (including en passant)  
   Base `SCORE_CAPTURE_BASE` plus `mvv_lva_score(board, m)` from the board grid at from/to squares. **En passant** is special-cased: victim is treated like a pawn for the formula. If the move is also a promotion, `SCORE_PROMO_CAP_BONUS` is added so capture-promotions sort ahead of plain captures in the same MVV-LVA band.

3. **Quiet promotions**  
   If `castro_IsPromotion` applies and it is not handled as a capture branch above: queen promotion gets `SCORE_PROMO_QUIET_Q`, other promotion pieces get `SCORE_PROMO_QUIET_MIN`—both sit **between** captures and killers in the total order.

4. **Killers** (quiet moves only, when `ply < KILLER_MAX_PLIES`)  
   Two slots per ply: `killer_moves[ply][0]` and `[1]`. Match uses `castro_MoveCmp`, which is **from/to equality** (ignores flags). Slot 0 gets `SCORE_KILLER_0`, slot 1 gets `SCORE_KILLER_1`.

5. **History**  
   Otherwise the score is the **raw** `history_heuristic[side][from][to]` for the side to move, clamped to `[-HISTORY_MAX, HISTORY_MAX]` with `HISTORY_MAX = 200000`. Because killer scores are ~21–22 million, killers always sort above any history value.

If `ply >= KILLER_MAX_PLIES`, the killer branch is skipped and quiets use history only (same clamping).

## Sorting and stability

For each move you build a `MoveScore { m, score, orig_idx }`, then `qsort` with `cmp_move_score_desc`: higher score first; on a tie, **lower `orig_idx` first** (deterministic, preserves original order among equals).

For up to 256 moves you use a stack buffer; above that you `malloc` a row of `MoveScore` (if allocation fails, the function returns without reordering—worth knowing for edge-case robustness).

## Killer moves: storage

`killer_store(m, ply)` is invoked from `negamax` only on a **beta cutoff** caused by a **non-capture** (`search.c`). It:

- Ignores invalid ply or `NULL_MOVE`.
- If the new move is **from/to-equal** to killer slot 0, it does nothing (avoids duplicate).
- Otherwise shifts: old slot 0 → slot 1, new move → slot 0.

So you keep the two most recent distinct quiet cutoffs at that ply (standard two-killer scheme).

## History heuristic: updates

Still in `negamax`, on a **quiet** beta cutoff (`!is_capture`):

- The **cutoff move** gets `history_heuristic[side][from][to] += depth * depth`, capped at `HISTORY_MAX`.
- Every **quiet move tried earlier** at this node (`quiets_tried` before the cutoff move) gets `-= depth * depth`, floored at `-HISTORY_MAX`.

That is a **Butterfly-style** history: reward the move that refuted the node, penalize other quiet tries that did not. Only quiet moves participate; captures do not update history on cutoff.

## Interaction with transposition table and `prioritize_hash_move`

Ordering puts the TT/hint move at the top **score tier**, and `prioritize_hash_move` in `search.c` additionally swaps that encoded move into list index `0` when found. That supports the comment there: PVS treats the first **successfully played** extension as PV-related; if the best-scoring move fails `MakeMove`, you still want the hash/PV move in front when it exists in the list.

## Quiescence-specific behavior

Quiescence generates only captures (`MOVE_CAPTURE`), then calls `order(..., NULL_MOVE)`. So you only see **capture scoring** (MVV-LVA + promo bonus + en passant rule)—no TT tier, no killers, no history for those moves in the current pipeline (history could still affect ordering if you ever passed a non-null hint or mixed move types; with pure captures, you are in the capture branch only).

---

**Summary:** This implementation of move ordering is a **single-pass score + `qsort`** pipeline: TT/hint first, then captures (MVV-LVA, with en passant and promotion tweaks), then quiet queen promotions, then other quiet promotions, then two killer slots per ply (from/to match), then side-specific **from–to history** with large fixed tiers so ordering stays unambiguous. Killers and history are **updated only in main search** on quiet beta cutoffs; MVV-LVA tables are initialized at engine startup and reused for every order call.
