#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "castro.h"
#include "core.h"
#include "moveordering.h"

/* 12×12 MVV-LVA table: [attacker][victim] */
static int MVV_LVA[12][12];

int history_heuristic[2][64][64];

static Move killer_moves[KILLER_MAX_PLIES][2];

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

#define ORDER_SORT_STACK 256

typedef struct {
    Move m;
    int score;
    size_t orig_idx;
} MoveScore;

static int cmp_move_score_desc(const void *a, const void *b)
{
    const MoveScore *x = (const MoveScore *)a;
    const MoveScore *y = (const MoveScore *)b;
    if (x->score != y->score)
        return (x->score < y->score) ? 1 : -1;
    if (x->orig_idx < y->orig_idx)
        return -1;
    if (x->orig_idx > y->orig_idx)
        return 1;
    return 0;
}

void irida_MVVLVAInit(void)
{
    for (int attacker = 0; attacker < 12; attacker++) {
        for (int victim = 0; victim < 12; victim++) {
            MVV_LVA[attacker][victim] = (PIECE_VALUES[victim % 6] * 10)
                                        - PIECE_VALUES[attacker % 6];
        }
    }
}

static inline int piece_index(char ch)
{
    switch (ch) {
        case 'P': return 0; case 'N': return 1; case 'B': return 2;
        case 'R': return 3; case 'Q': return 4; case 'K': return 5;
        case 'p': return 6; case 'n': return 7; case 'b': return 8;
        case 'r': return 9; case 'q': return 10; case 'k': return 11;
        default:  return -1;
    }
}

int irida_MVVLVAScore(Board *board, Move m)
{
    int from = castro_GetFrom(m);
    int to   = castro_GetTo(m);

    char attacker_ch = board->grid[from / 8][from % 8];
    char victim_ch   = board->grid[to   / 8][to   % 8];

    int attacker = piece_index(attacker_ch);
    int victim   = piece_index(victim_ch);

    if (attacker == -1 || victim == -1)
        return 0;

    return MVV_LVA[attacker][victim];
}

void irida_KillerStore(Move m, int ply)
{
    if (ply < 0 || ply >= KILLER_MAX_PLIES)
        return;
    if (m == NULL_MOVE)
        return;
    if (killer_moves[ply][0] != NULL_MOVE && castro_MoveCmp(killer_moves[ply][0], m))
        return;
    killer_moves[ply][1] = killer_moves[ply][0];
    killer_moves[ply][0] = m;
}

void irida_OrderingReset(void)
{
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_heuristic, 0, sizeof(history_heuristic));
}

void irida_OrderMoves(Board *board, Move moves[], size_t count, size_t ply, Move tt_move)
{
    if (count == 0)
        return;

    MoveScore stack_row[ORDER_SORT_STACK];
    MoveScore *row = stack_row;
    bool heap = false;

    if (count > ORDER_SORT_STACK) {
        row = (MoveScore *)malloc(count * sizeof(MoveScore));
        if (!row)
            return;
        heap = true;
    }

    for (size_t i = 0; i < count; i++) {
        Move m = moves[i];
        row[i].m = m;
        row[i].orig_idx = i;
        int s = 0;

        if (m == tt_move && tt_move != NULL_MOVE) {
            s = SCORE_TT;
        } else if (castro_MoveIsCapture(board, m)) {
            if (castro_GetFlag(m) == FLAG_ENPASSANT) {
                s = SCORE_CAPTURE_BASE + (PIECE_VALUES[0] * 10) - PIECE_VALUES[0];
            } else {
                s = SCORE_CAPTURE_BASE + irida_MVVLVAScore(board, m);
            }
            Move mp = m;
            if (castro_IsPromotion(board, &mp))
                s += SCORE_PROMO_CAP_BONUS;
        } else {
            Move mp = m;
            if (castro_IsPromotion(board, &mp)) {
                uint8_t pr = castro_GetPromotion(m);
                s = (pr == PROMOTION_QUEEN) ? SCORE_PROMO_QUIET_Q : SCORE_PROMO_QUIET_MIN;
            } else if (ply < (size_t)KILLER_MAX_PLIES) {
                if (killer_moves[ply][0] != NULL_MOVE && castro_MoveCmp(m, killer_moves[ply][0]))
                    s = SCORE_KILLER_0;
                else if (killer_moves[ply][1] != NULL_MOVE && castro_MoveCmp(m, killer_moves[ply][1]))
                    s = SCORE_KILLER_1;
                else {
                    int side = board->turn;
                    int from = castro_GetFrom(m);
                    int to   = castro_GetTo(m);
                    int h    = history_heuristic[side][from][to];
                    if (h > HISTORY_MAX)
                        h = HISTORY_MAX;
                    s = h;
                }
            } else {
                int side = board->turn;
                int from = castro_GetFrom(m);
                int to   = castro_GetTo(m);
                int h    = history_heuristic[side][from][to];
                if (h > HISTORY_MAX)
                    h = HISTORY_MAX;
                s = h;
            }
        }

        row[i].score = s;
    }

    qsort(row, count, sizeof(MoveScore), cmp_move_score_desc);

    for (size_t i = 0; i < count; i++)
        moves[i] = row[i].m;

    if (heap)
        free(row);
}
