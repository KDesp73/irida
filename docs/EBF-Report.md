# Search growth and EBF-style metrics (Irida builds)

This report summarizes **effective branching / incremental node growth** for each staged engine binary, measured with [`scripts/ebf.py`](../scripts/ebf.py) on **2026-04-17** (local run; your absolute numbers will vary by CPU and build flags).

## Methodology

| Setting | Value |
|--------|--------|
| Script | `./scripts/ebf.py --depth 8 --pty` |
| Position | Start position (`position startpos`) |
| Hash | 16 MB (`setoption name Hash value 16`) |
| Node source | UCI `info depth … nodes …` (cumulative nodes including quiescence) |

The script records, for each completed iterative-deepening depth \(d\), the **last** `nodes` value reported for that depth (aspiration can emit several `info` lines per depth).

From cumulative totals it computes:

- **delta\[d\]** = nodes completed through depth \(d\) minus nodes completed through depth \(d-1\) (extra work for that ID step).
- **delta_ratio\[d\]** = delta\[d\] / delta\[d-1\].
- **N^(1/D)** = \(N^{1/D}\) with \(N\) = final cumulative nodes and \(D\) = max depth — a **crude** single number (not a rigorous per-ply branching factor under alpha–beta).
- **Geom mean of ratios** = geometric mean of delta_ratio for depths \(2 \ldots D\) — typical **multiplicative growth** of incremental work from one ID step to the next.

**Important:** These are **diagnostics**, not chess-theoretic branching factors. Pruning, LMR, TT, quiescence, null move, and move ordering all change the shape of the tree. Compare builds **at the same depth and position** to see which configuration expands less for this run.

## Build feature matrix

Compile-time flags follow [`make/targets.mk`](../make/targets.mk). “Default” `irida` uses the `%` rule (full feature set **without** PVS, **with** Syzygy).

| Binary | Quiescence | TT | NMP | LMR | Aspiration | PVS | Syzygy |
|--------|:---:|:---:|:---:|:---:|:---:|:---:|:---:|
| `irida` (default) | ✓ | ✓ | ✓ | ✓ | ✓ | — | ✓ |
| `irida-plain` | — | — | — | — | — | — | — |
| `irida-q` | ✓ | — | — | — | — | — | — |
| `irida-q-tt` | ✓ | ✓ | — | — | — | — | — |
| `irida-q-tt-nmp` | ✓ | ✓ | ✓ | — | — | — | — |
| `irida-q-tt-nmp-lmr` | ✓ | ✓ | ✓ | ✓ | — | — | — |
| `irida-q-tt-nmp-lmr-aw` | ✓ | ✓ | ✓ | ✓ | ✓ | — | — |
| `irida-q-tt-nmp-lmr-aw-pvs` | ✓ | ✓ | ✓ | ✓ | ✓ | ✓ | — |

## Results at depth 8 (startpos)

| Binary | Cumulative nodes N | N^(1/D) | Geom mean Δ[d]/Δ[d−1] |
|--------|-------------------:|--------:|----------------------:|
| `irida` | 362,823 | 4.954 | 3.166 |
| `irida-plain` | 1,297,709 | 5.810 | 4.634 |
| `irida-q` | 5,731,889 | 6.995 | 5.273 |
| `irida-q-tt` | 1,639,226 | 5.982 | 4.330 |
| `irida-q-tt-nmp` | 2,261,276 | 6.227 | 4.607 |
| `irida-q-tt-nmp-lmr` | 389,612 | 4.998 | 3.513 |
| `irida-q-tt-nmp-lmr-aw` | 362,823 | 4.954 | 3.166 |
| `irida-q-tt-nmp-lmr-aw-pvs` | 335,834 | 4.906 | 3.138 |

### Observations (this run)

1. **Quiescence (`irida-q`)** explodes cumulative nodes versus `irida-plain` at the same nominal depth — expected: qsearch explores tactical continuations beyond the main `depth` horizon.
2. **Transposition table (`irida-q-tt`)** greatly reduces total nodes versus `irida-q` here — TT reuse dominates on the start position for this depth.
3. **LMR (`irida-q-tt-nmp-lmr`)** drops node count sharply versus `irida-q-tt-nmp` — reductions trim many late-order quiet moves.
4. **Aspiration (`irida-q-tt-nmp-lmr-aw`)** matches **`irida`** on this run (same totals): with Syzygy inactive on startpos at these depths, the staged build with AW aligns with the default stack’s node profile for this single test.
5. **PVS (`…-aw-pvs`)** is slightly **lower** total nodes than the same build without PVS here — consistent with PVS as a search **efficiency** aid when ordering is reasonable (not a guarantee in all positions).

## Reproducing

```bash
make libirida.a
make irida irida-plain irida-q irida-q-tt irida-q-tt-nmp \
     irida-q-tt-nmp-lmr irida-q-tt-nmp-lmr-aw irida-q-tt-nmp-lmr-aw-pvs

./scripts/ebf.py ./irida --depth 8 --pty
# repeat per binary, or loop (allow a short pause between runs if PTYs are scarce)
```

To compare **fairly** across algorithms, keep **depth**, **Hash**, **FEN**, and **binary build** fixed; only the executable should change.

## See also

- [`scripts/ebf.py`](../scripts/ebf.py) — implementation and metric definitions.
