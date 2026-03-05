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
            // value of victim * 10 minus attacker value
            MVV_LVA[attacker][victim] = PIECE_VALUES[victim % 6] * 10
                                        - PIECE_VALUES[attacker % 6];
        }
    }
}

// Helper to convert char on board to 0–11 piece index
static inline int piece_index(char ch)
{
    switch (ch) {
        case 'P': return 0;
        case 'N': return 1;
        case 'B': return 2;
        case 'R': return 3;
        case 'Q': return 4;
        case 'K': return 5;
        case 'p': return 6;
        case 'n': return 7;
        case 'b': return 8;
        case 'r': return 9;
        case 'q': return 10;
        case 'k': return 11;
        default:  return -1; // empty square
    }
}

// Compute MVV-LVA score for a capture move
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

#define MAX_PLY 128

// Killer moves per ply
static Move killer1[MAX_PLY];
static Move killer2[MAX_PLY];

// History heuristic table
static int history[64][64]; // from -> to squares

static Move tt_move;

// Score a single move
static inline int score_move(Board *board, Move m, int ply)
{
    if (m == tt_move) {
        return 1000000;
    }

    if (castro_IsCapture(board, m)) {
        return 500000 + mvv_lva_score(board, m); // MVV-LVA
    }

    if (m == killer1[ply]) return 400000;
    if (m == killer2[ply]) return 390000;

    int from = castro_GetFrom(m);
    int to   = castro_GetTo(m);
    return history[from][to];
}

// Partial selection sort for move picking
void order_moves(Board *board, Move moves[], size_t count, size_t ply)
{
    for (size_t i = 0; i < count; i++) {
        size_t best_idx = i;
        int best_score = score_move(board, moves[i], ply);

        for (size_t j = i + 1; j < count; j++) {
            int s = score_move(board, moves[j], ply);
            if (s > best_score) {
                best_score = s;
                best_idx = j;
            }
        }

        // swap moves[i] <-> moves[best_idx]
        if (best_idx != i) {
            Move tmp = moves[i];
            moves[i] = moves[best_idx];
            moves[best_idx] = tmp;
        }
    }
}

void update_history(Move m, int depth)
{
    int from = castro_GetFrom(m);
    int to   = castro_GetTo(m);
    history[from][to] += depth * depth;
}

void update_killer(Move m, int ply)
{
    if (killer1[ply] != m) {
        killer2[ply] = killer1[ply];
        killer1[ply] = m;
    }
}

void set_tt_move(Move m)
{
    tt_move = m;
}

