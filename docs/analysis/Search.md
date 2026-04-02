## Search architecture

The main chess search lives in `src/search/search.c` as **`search()`** plus a static **`negamax()`** core. Quiescence is in `src/search/quiescence.c`. Configuration and statistics types are in `include/search.h`; the UCI binary’s default flags come from `apps/shared.h` (e.g. aspiration, PVS, LMR, NMP, TT, quiescence, and Syzygy all enabled there in the current tree).

---

### Entry point and configuration

**`SearchConfig`** (`search.h`) holds `maxDepth`, `timeLimitMs`, `threads`, and boolean switches: `useNMP`, `useLMR`, `useAspiration`, `usePVS`, `useTT`, `useQuiescence`, `useSyzygy`, plus Syzygy probe depth/limit and the 50-move rule flag. The UCI **`go`** handler fills these from the GUI; tests and bench programs set their own structs or globals.

**`SearchStats`** exposes `nodes`, `qnodes`, `selDepth`, `tbHits`, and placeholders `ttHits` / `rootChildTtHits` (the latter two are not incremented anywhere in the codebase as of this review).

**`search_variants`** in `search.h` pairs names (`"random"`, `"search"`) with `random_move` and `search` for bench or alternate binaries.

---

### `search()`: root driver

**Transposition table generation:** `tt_inc_generation()` runs at the start so TT entries from prior searches are not reused incorrectly.

**Root Syzygy:** If Syzygy is enabled, piece count is within `syzygyProbeLimit` and `TB_LARGEST`, and `syzygy_probe_root` succeeds, the function returns the tablebase move immediately after `info string tablebase hit`.

**Statistics:** `g_searchStats.nodes`, `qnodes`, and `selDepth` are zeroed.

**Timer:** `search_start_timer(config->timeLimitMs)`; negamax periodically checks time and stop flags.

**Root FEN snapshot:** The position is exported to a string so the board can be **reloaded from FEN** repeatedly. Iterative deepening and aspiration re-searches deliberately **re-initialize the root** from that snapshot to avoid castro undo/metadata edge cases (comments cite side-to-move / fullmove drift when `history.count` is 0).

**Iterative deepening:** For `currentDepth` from 1 to `maxDepth`, the board is freed and reloaded from the snapshot, then:

- **Aspiration windows** (if `useAspiration` and `currentDepth >= 5`): `alpha = last_depth_score - delta`, `beta = last_depth_score + delta`, with `delta = 40 + abs(last_depth_score)/4`. Otherwise `[-INF, INF]`.
- **`iterationBestMove`** starts as the previous depth’s `best_move` and is passed into move ordering as the hash/PV hint.
- An inner loop handles aspiration **fail-low / fail-high**: widen the window (`delta *= 2`), re-search, update `iterationBestMove` from the current best when available. After more than two failures, the window expands to full `[-INF, INF]`. Fail status is judged against **original** alpha/beta for the iteration, not the updated bounds after child moves.

**Root move loop:** Before each root move, the board is again reset from the FEN snapshot. Legal moves are generated, **`order()`** is called with **`iterationBestMove`** as the TT hint, then **`prioritize_hash_move`** swaps that move into list index 0 if present (so the “first” child aligns with the PV hint even when `MakeMove` could fail for a stale encoded move—see PVS below).

Each root move: `MakeMove`, then **`-negamax(..., currentDepth - 1, 1, -beta, -alpha, true, config)`** — ply 1 inside the subtree, `tt_exact_ok == true` at the first ply below root.

After each completed depth, `best_move` and `last_depth_score` are committed and **`uci_report_search`** prints depth, score, nodes, nps, hashfull, tbhits, time, PV string. Search stops on time/stop, or when a mate score exceeds the early-exit threshold (`last_depth_score > INF - MAX_PLY`).

**Fallback:** If no move was chosen, a temporary board is loaded from the snapshot and the first legal move is taken.

---

### `negamax()`: recursive core

Parameters: `depth` (remaining full-width plies), `ply` (distance from root), `alpha`/`beta`, **`tt_exact_ok`** (prevents storing/using certain exact scores when unsafe—e.g. after null-move), and `config`.

**Order of operations:**

1. **Draw rules (non-root):** threefold repetition or 100 halfmoves → return `0`.

2. **TT probe:** If `useTT` and `tt_probe` returns a cutoff, return `tt_score` **without** incrementing `nodes` (TT cuts save work and do not count as visited nodes in this implementation).

3. **Mate distance pruning:** `alpha` is raised to at least `-INF + ply`; if `beta <= alpha`, return.

4. **Time / stop:** Every 1024 nodes (`nodes & 1023 == 0`), check `search_time_up()` and may set `stopRequested`. If search should stop, return `0`.

5. **`g_searchStats.nodes++`** and update **`selDepth`** from `ply`.

6. **Syzygy WDL (non-root):** Eligibility follows piece count vs `syzygyProbeLimit` and, at the cap, `depth >= syzygyProbeDepth`. On success, optionally store in TT, increment `tbHits`, return the score.

7. **Leaf:** If `depth <= 0`, return **`quiescence`** if `useQuiescence`, else **`0`** (no static eval in the zero-quiescence path).

8. **Null-move pruning:** If `useNMP`, `depth >= 3`, not in check, and the side has non-pawn material: perform null move, recursive call with depth `depth - 1 - 3` and a **null-window** `[-beta, -beta+1]`, unmake. If `score >= beta`, return (mate scores are clamped to `beta` to avoid storing absurd values in some paths).

9. **Move generation:** No moves → checkmate (`-INF + ply`) or stalemate (`0`).

10. **Ordering:** `order(..., tt_move)` then **`prioritize_hash_move`** for the TT best move.

11. **Move loop:** For each move, `MakeMove` may fail (skipped). The **first successful** child defines **`pv_node`**; later siblings are non-PV. This is independent of list index 0 so PVS/LMR stay correct when the TT move fails.

    - **LMR** (`useLMR`): `depth >= 3`, index `i >= 4`, not PV node, parent not in check, not a capture, does not give check, move is not the TT move. Reduction `R = 1 + depth/6 + i/10`; `newDepth = depth - 1 - R`, floored at 1.
    - **PVS** (`usePVS`): non-PV nodes use a **zero-window** scout `-(alpha+1), -alpha` with `tt_exact_ok` forced `false` on that scout. Re-search at full depth `depth - 1` when `score > alpha` and either the move was reduced (`newDepth < depth - 1`) or `score < beta` (avoids full re-search on every beta cutoff from the scout).
    - Non-PVS path: full window at `newDepth`; if LMR was applied and `score > alpha`, re-search at `depth - 1`.

12. On **beta cutoff** with a **quiet** move: **`killer_store`**, history bonuses/penalties on `history_heuristic` (same scheme as described in the move-ordering overview).

13. **TT store:** If `useTT`, classify bound type (exact / lower / upper); if the node would be exact but `!tt_exact_ok`, store as lower bound only. Store `bestMove` for future ordering.

Return `bestScore` (which equals `alpha` at the end of a normal search of the move list).

---

### Quiescence

`quiescence()` increments **`qnodes`**, updates **`selDepth`**, polls time every 1024 qnodes, and returns **`alpha`** if stopped. At `ply >= MAX_PLY` it returns **`eval(board)`**. Otherwise it uses **stand-pat**: static eval; cutoff if `>= beta`; raise `alpha` if better. Then **capture-only** move generation (`MOVE_CAPTURE`), **`order(..., NULL_MOVE)`** (no TT hint in quiescence), and recursive negamax-style minimax on captures with alpha-beta cutoffs.

---

### Interaction with evaluation and move ordering

Search does not implement ordering itself; it calls the injected **`OrderFn`**. On cutoffs it updates **killers and history** in `moveordering.c`. Evaluation is invoked at quiescence nodes and (via `depth <= 0`) at the horizon; the `eval` pointer comes from the engine.

---

### Observability

**`uci_report_search`** (`src/uci/log.c`) prints **`nodes + qnodes`** as total nodes, **`selDepth`**, **`tbHits`**, **`hashfull`**, nps, and PV. TT cutoffs do not increment `nodes`, so large TT hit rates reduce reported nodes relative to a TT-off run. **`ttHits`** in `SearchStats` is unused in the current implementation.

---

### Summary

The Irida search is **iterative deepening negamax** at the root with **aspiration windows** from depth 5, **alpha-beta**, **transposition table** probe/store, **null-move pruning**, **late move reductions** with conditional re-search, optional **PVS** zero-window scouts with guarded full re-searches, **Syzygy** at root and in the tree, and **quiescence** on capture moves when enabled. The root aggressively **rebuilds the board from a FEN snapshot** for reliability across depths and aspiration iterations. Statistics and UCI reporting expose total nodes, selective depth, and tablebase hits; fine-grained per-optimization counters are not present unless added separately.
