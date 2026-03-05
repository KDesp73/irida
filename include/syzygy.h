#ifndef SYZYGY_H
#define SYZYGY_H

#include "castro.h"
#include "search.h"
#include <stdbool.h>

/* Initialize Syzygy tablebases from path. Returns true on success. */
bool syzygy_init(const char* path);

/* Free Syzygy resources. */
void syzygy_free(void);

/* Whether tablebases are loaded and usable. */
bool syzygy_available(void);

/* Probe WDL for current position. Returns score in centipawns (or draw/mate-like)
 * if probe succeeds; otherwise returns SYZYGY_PROBE_FAILED so caller uses normal eval.
 * Call only when piece count <= limit, depth <= probe_depth, and (if 50-move rule)
 * halfmove == 0 and castling == 0. */
#define SYZYGY_PROBE_FAILED (-1000001)
int syzygy_probe_wdl(Board* board, bool use_rule50);

/* At root: probe DTZ and fill in best move if position is in TB. Returns true if
 * a TB move was chosen (and best_move_out is set), false otherwise. */
bool syzygy_probe_root(Board* board, bool use_rule50, Move* best_move_out);

/* Total piece count (both sides). */
unsigned syzygy_piece_count(Board* board);

#endif
