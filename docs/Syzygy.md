# Syzygy Tablebases

The engine supports optional Syzygy (Syzygy) endgame tablebase probing. When a tablebase path is set and the Fathom library is linked, the engine will probe WDL (Win-Draw-Loss) during search in positions with few pieces.

## UCI options

- **SyzygyPath** (string): Directory containing Syzygy `.rtbw` (and optionally `.rtbz`) files. Set via `setoption name SyzygyPath value <path>`.
- **SyzygyProbeDepth** (spin 1–100): Only probe when search depth is at or below this value (default 1).
- **SyzygyProbeLimit** (spin 0–7): Only probe when the number of pieces is at or below this value (default 7).
- **Syzygy50MoveRule** (check): Whether to consider the 50-move rule when probing (default true).

## Enabling Syzygy (optional Fathom build)

The engine compiles and runs without Fathom; in that case Syzygy options are stored but probing is a no-op. To use tablebases:

1. Clone and build Fathom (MIT license):
   ```bash
   git clone https://github.com/jdart1/Fathom.git extern/fathom
   cd extern/fathom && make
   ```

2. Build the engine with Fathom:
   - Add to compiler flags: `-DUSE_FATHOM -Iextern/fathom/src` (or the path to Fathom's `tbprobe.h`).
   - Link with Fathom's library (e.g. `extern/fathom/libfathom.a` or the object files from Fathom's build).

3. Download Syzygy tablebase files (e.g. 3–5 piece sets from [Syzygy tablebases](https://www.chessprogramming.org/Syzygy_Bases)) and set **SyzygyPath** to the directory containing the `.rtbw` (and optionally `.rtbz`) files.

## Behaviour

- **In search**: When `syzygy_available()` is true, depth ≤ SyzygyProbeDepth, and piece count ≤ SyzygyProbeLimit, the engine probes the WDL table. If the probe succeeds, the returned score (win/draw/loss) is used instead of the normal evaluator. Probing is only done when castling rights are zero; the 50-move rule is respected according to **Syzygy50MoveRule**.
- **At root**: Root DTZ probing (to choose a TB move directly) is stubbed; the engine continues to use normal search at the root.
