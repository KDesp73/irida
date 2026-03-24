#include <stddef.h>
#include <stdint.h>
#include "castro.h"
#include "core.h"

// 12×12 MVV-LVA table: [attacker][victim]
static int MVV_LVA[12][12];

void init_mvv_lva(void)
{
    for (int attacker = 0; attacker < 12; attacker++) {
        for (int victim = 0; victim < 12; victim++) {
            // Victim value is prioritized (x10), attacker value is a tie-breaker
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

int mvv_lva_score(Board *board, Move m)
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

/* * Simplified Move Ordering:
 * 1. TT Move (Highest Priority)
 * 2. Captures (Ordered by MVV-LVA)
 */
void order_moves(Board *board, Move moves[], size_t count, size_t ply, Move tt_move)
{
    int scores[MAX_MOVES];

    // 1. Assign scores based on TT status and MVV-LVA
    for (size_t i = 0; i < count; i++) {
        if (moves[i] == tt_move && tt_move != NULL_MOVE) {
            // Give the TT move a score higher than any possible MVV-LVA result
            scores[i] = 1000000; 
        } 
        else {
            scores[i] = mvv_lva_score(board, moves[i]);
        }
    }

    // 2. Selection Sort (Descending)
    for (size_t i = 0; i < count; i++) {
        size_t best_idx = i;
        for (size_t j = i + 1; j < count; j++) {
            if (scores[j] > scores[best_idx]) {
                best_idx = j;
            }
        }

        // Swap moves
        Move tmp_move = moves[i];
        moves[i] = moves[best_idx];
        moves[best_idx] = tmp_move;

        // Swap scores
        int tmp_score = scores[i];
        scores[i] = scores[best_idx];
        scores[best_idx] = tmp_score;
    }
}
