#ifndef SYZYGY_H
#define SYZYGY_H

#include "castro.h"
#include <stdbool.h>

// @module syzygy
// @desc Syzygy tablebases: init, free, probe WDL/DTZ, piece count.

// @function syzygy_init
// @desc Initialize Syzygy tablebases from path.
// @param path Path to Syzygy .rtbw/.rtbz files.
// @returns bool True on success.
bool irida_SyzygyInit(const char* path);

// @function syzygy_free
// @desc Free Syzygy resources.
void irida_SyzygyFree(void);

// @function syzygy_available
// @desc Whether tablebases are loaded and usable.
// @returns bool True if loaded.
bool irida_SyzygyAvailable(void);

// @const SYZYGY_PROBE_FAILED
// @desc Return value when probe fails; caller uses normal eval.
#define SYZYGY_PROBE_FAILED (-1000001)

// @function syzygy_probe_wdl
// @desc Probe WDL for current position. Returns score in centipawns (or draw/mate-like) if probe succeeds; otherwise SYZYGY_PROBE_FAILED. Search gates probes by piece limit and SyzygyProbeDepth (Stockfish-style); Fathom also requires halfmove == 0 and no castling.
// @param board Board position.
// @param use_rule50 Whether to apply 50-move rule for TB.
// @returns int WDL score or SYZYGY_PROBE_FAILED.
int irida_SyzygyProbeWdl(Board* board, bool use_rule50);

// @function syzygy_probe_root
// @desc At root: probe DTZ and fill in best move if position is in TB.
// @param board Board position.
// @param use_rule50 Whether to apply 50-move rule.
// @param best_move_out Output: best move if TB hit.
// @returns bool True if TB move was chosen (best_move_out set).
bool irida_SyzygyProbeRoot(Board* board, bool use_rule50, Move* best_move_out);

// @function syzygy_piece_count
// @desc Total piece count (both sides).
// @param board Board position.
// @returns unsigned Piece count.
unsigned irida_SyzygyPieceCount(Board* board);

#define SYZYGY_DEFAULT_PATH "~/.irida/tb/dtz:~/.irida/tb/wdl"

#endif
