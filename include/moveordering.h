#ifndef MOVE_ORDERING_H
#define MOVE_ORDERING_H

#include "castro.h"

void init_mvv_lva(void);
int mvv_lva_score(Board *board, Move m);
void order_moves(Board *board, Move moves[], int count, int ply);


#endif // MOVE_ORDERING_H
