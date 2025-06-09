#include "moveordering.h"
#include "square.h"
#include <string.h>

int ScoreMove(Board* board, Move move) {
    Square from = GetFrom(move);
    Square to = GetTo(move);
    uint8_t promotion = GetPromotion(move);

    char attacker = board->grid[COORDS(from)];
    char victim = board->grid[COORDS(to)];

    int score = 0;

    if (victim != EMPTY_SQUARE) {
        int attacker_index = strchr(PIECES, attacker) - PIECES;
        int victim_index   = strchr(PIECES, victim) - PIECES;
        score += 100000 + MVV_LVA_SCORES[attacker_index][victim_index];
    }

    if (IsPromotion(board, &move)) {
        score += 80000 + promotion * 1000; // Q > R > B > N
    }

    if (IsCapture(board, move)) {
        score += 50000; // generic bonus for captures
    }

    if (IsInCheckAfterMove(board, move)) {
        score += 3000;
    }

    return score;
}

typedef struct {
    Move move;
    int score;
} ScoredMove;

static int CompareScoredMoves(const void* a, const void* b)
{
    return ((ScoredMove*)b)->score - ((ScoredMove*)a)->score;
}

void SortMoves(Board* board, Moves* moves)
{
    ScoredMove scored[MOVES_CAPACITY];

    for (size_t i = 0; i < moves->count; ++i) {
        scored[i].move = moves->list[i];
        scored[i].score = ScoreMove(board, moves->list[i]);
    }

    qsort(scored, moves->count, sizeof(ScoredMove), CompareScoredMoves);

    for (size_t i = 0; i < moves->count; ++i) {
        moves->list[i] = scored[i].move;
    }
}
