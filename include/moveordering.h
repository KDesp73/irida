#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "castro.h"

typedef void (*OrderFn)(Board* board, Move moves[], size_t count, size_t ply);

void init_mvv_lva(void);
int mvv_lva_score(Board *board, Move m);
void order_moves(Board *board, Move moves[], size_t count, size_t ply);

/* Killer moves */
void update_killer(Move move, int ply);

/* History heuristic */
void update_history(Move move, int depth);

#endif // MOVE_ORDERING_H
