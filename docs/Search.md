# Chess Engine Search Architecture

# 1. Core Search Structure

```
iterative_deepening()
    └── aspiration_window()
            └── pvs_search()
                    ├── transposition_table_probe()
                    ├── null_move_pruning()
                    ├── move_ordering()
                    ├── late_move_reductions()
                    ├── extensions()
                    ├── alpha_beta recursion
                    └── transposition_table_store()
                            └── quiescence_search()
```
---

# 3. Iterative Deepening

## What It Is

Search depth increases incrementally:

```
depth = 1
depth = 2
depth = 3
...
```

## Why It Is Critical

* Improves move ordering using previous principal variation
* Allows time control
* Enables aspiration windows
* Improves TT efficiency

## Requirements

* Store best move each depth
* Track score from previous iteration
* Stop cleanly when time expires

---

# 4. Alpha-Beta (Negamax Form)

## Purpose

Eliminate branches that cannot improve the position.

## Required Behavior

```
if score >= beta:
    return beta  // cutoff

if score > alpha:
    alpha = score
```

### Must Support:

* Mate scores adjusted by ply
* Proper alpha/beta bounds
* Correct fail-hard or fail-soft semantics

Alpha-beta efficiency depends entirely on move ordering.

---

# 5. Principal Variation Search (PVS)

## Why Replace Plain Alpha-Beta?

Most moves are bad. Searching them with a full window wastes time.

## Behavior

First move:

```
full window search
```

Subsequent moves:

```
zero window search (-alpha-1, -alpha)
re-search only if improves alpha
```

## Benefits

* Large node reduction
* Standard in all modern engines

---

# 6. Quiescence Search

## Problem: Horizon Effect

If you stop at depth 0 in a tactical position, evaluation lies.

## Solution

At leaf:

* Evaluate (stand-pat)
* Search only captures/promotions (optionally checks)

## Requirements

* Stand-pat cutoff
* Capture generation only
* Avoid infinite recursion
* Consider delta pruning later

---

# 7. Transposition Table (TT)

## What It Does

Caches previously evaluated positions.

## Required Data Per Entry

* Zobrist key
* Depth searched
* Score
* Node type (EXACT / LOWERBOUND / UPPERBOUND)
* Best move

## Required Logic

### Probe Before Search

* If entry depth ≥ current depth:

  * Use bounds to cutoff

### Store After Search

* Save score with correct bound type

## Why It Matters

* Prevents re-searching same positions
* Improves move ordering (TT move first)
* Massive pruning gain

---

# 8. Move Ordering System

Move ordering determines pruning efficiency.

## Required Priority Order

1. TT move
2. Captures (MVV-LVA)
3. Promotions
4. Killer moves
5. History heuristic
6. Remaining quiet moves

---

## 8.1 MVV-LVA

Score captures by:

```
value(victim) * 10 - value(attacker)
```

Purpose:
Prefer capturing high-value pieces with low-value attackers.

---

## 8.2 Killer Moves

Per ply:

* Store 2 quiet moves that caused beta cutoff
* Search them early next time

Do NOT store captures as killers.

---

## 8.3 History Heuristic

Global table:

```
history[from][to] += depth * depth
```

Used for quiet move ordering.

Encourages moves that previously caused cutoffs.

---

# 9. Null Move Pruning

## Idea

If doing nothing still proves position ≥ beta,
the position is so good we can prune.

## Conditions

* Not in check
* Depth >= R+1
* Not likely zugzwang
* Sufficient material

## Procedure

```
make null move
score = -search(depth-R-1, -beta, -beta+1)
if score >= beta:
    return beta
```

## Benefit

Massive pruning.

## Risk

Zugzwang errors. Must handle carefully.

---

# 10. Late Move Reductions (LMR)

## Idea

Later moves are less promising.

If:

* Move index > N
* Quiet move
* Depth >= 3
* Not check

Then:
Search at reduced depth first.

If improves alpha → re-search full depth.

---

# 11. Extensions

Selective depth increases.

Common extensions:

* In-check extension
* Recapture extension
* Passed pawn push near promotion
* Mate threat extension

---

# 12. Aspiration Windows

Instead of:

```
search(-INF, INF)
```

Use:

```
search(prevScore - window, prevScore + window)
```

If fail-high or fail-low:

* Re-search with wider window

Benefits:

* Faster search
* Narrower alpha-beta window improves pruning

---

# 13. Mate Score Handling

Use:

```
MATE = 100000
score = -MATE + ply
```

This ensures:

* Faster mates are better
* Mate scores propagate correctly

Must adjust when storing in TT.

---

# 14. Time Management

Requirements:

* Check time inside search
* Abort safely
* Always return best move found so far

Time control must:

* Allocate time per move
* Consider increment
* Consider remaining time

---

# 15. Repetition & Draw Detection

Before searching:

* Threefold repetition detection
* 50-move rule
* Insufficient material

Return draw score when applicable.

---

# 18. Development Order

- [ ] 1. Stable alpha-beta + quiescence
- [ ] 2. Iterative deepening
- [ ] 3. Transposition table
- [ ] 4. TT move ordering
- [ ] 5. Killer + history
- [ ] 6. PVS
- [ ] 7. Null move pruning
- [ ] 8. LMR
- [ ] 9. Aspiration windows
- [ ] 10. Extensions
