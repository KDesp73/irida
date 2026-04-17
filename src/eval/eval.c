// www.chessprogramming.org/PeSTO's_Evaluation_Function

#include "castro.h"
#include "eval.h"
#include <stdio.h>
#include "utils.h"

#define WHITE_PAWN_ENCODING    (2*PAWN   + COLOR_WHITE)
#define BLACK_PAWN_ENCODING    (2*PAWN   + COLOR_BLACK)
#define WHITE_KNIGHT_ENCODING  (2*KNIGHT + COLOR_WHITE)
#define BLACK_KNIGHT_ENCODING  (2*KNIGHT + COLOR_BLACK)
#define WHITE_BISHOP_ENCODING  (2*BISHOP + COLOR_WHITE)
#define BLACK_BISHOP_ENCODING  (2*BISHOP + COLOR_BLACK)
#define WHITE_ROOK_ENCODING    (2*ROOK   + COLOR_WHITE)
#define BLACK_ROOK_ENCODING    (2*ROOK   + COLOR_BLACK)
#define WHITE_QUEEN_ENCODING   (2*QUEEN  + COLOR_WHITE)
#define BLACK_QUEEN_ENCODING   (2*QUEEN  + COLOR_BLACK)
#define WHITE_KING_ENCODING    (2*KING   + COLOR_WHITE)
#define BLACK_KING_ENCODING    (2*KING   + COLOR_BLACK)

#define EMPTY 12

#define THREAT_WEIGHT 3
#define SPACE_WEIGHT 2
#define MOBILITY_WEIGHT 4

static inline int piece_color(int pc) {
    return pc & 1;
}

static inline int flip_sq(int sq) {
    return sq ^ 56;
}

static inline int is_white_piece_char(char c) {
    return c >= 'A' && c <= 'Z';
}

static inline int is_black_piece_char(char c) {
    return c >= 'a' && c <= 'z';
}

static inline int same_color(char a, char b) {
    return (is_white_piece_char(a) && is_white_piece_char(b)) ||
           (is_black_piece_char(a) && is_black_piece_char(b));
}

static inline int opposite_color(char a, char b) {
    return (is_white_piece_char(a) && is_black_piece_char(b)) ||
           (is_black_piece_char(a) && is_white_piece_char(b));
}

static inline int piece_index_from_char(char ch) {
    switch (ch) {
        case 'P': case 'p': return 0;
        case 'N': case 'n': return 1;
        case 'B': case 'b': return 2;
        case 'R': case 'r': return 3;
        case 'Q': case 'q': return 4;
        case 'K': case 'k': return 5;
        default:            return -1;
    }
}

static inline int simple_piece_value_from_char(char ch) {
    static const int values[6] = {100, 320, 330, 500, 900, 0};
    int idx = piece_index_from_char(ch);
    return (idx >= 0) ? values[idx] : 0;
}

static const int mg_value[6] = { 82, 337, 365, 477, 1025, 0 };
static const int eg_value[6] = { 94, 281, 297, 512, 936, 0 };

const int mg_pawn_table[] =  {
      0,   0,   0,   0,   0,   0,  0,   0,
     98, 134,  61,  95,  68, 126, 34, -11,
     -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
      0,   0,   0,   0,   0,   0,  0,   0,
};

const int eg_pawn_table[] = {
      0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
     94, 100,  85,  67,  56,  53,  82,  84,
     32,  24,  13,   5,  -2,   4,  17,  17,
     13,   9,  -3,  -7,  -7,  -8,   3,  -1,
      4,   7,  -6,   1,   0,  -5,  -1,  -8,
     13,   8,   8,  10,  13,   0,   2,  -7,
      0,   0,   0,   0,   0,   0,   0,   0,
};

const int mg_knight_table[] = {
    -167, -89, -34, -49,  61, -97, -15, -107,
     -73, -41,  72,  36,  23,  62,   7,  -17,
     -47,  60,  37,  65,  84, 129,  73,   44,
      -9,  17,  19,  53,  37,  69,  18,   22,
     -13,   4,  16,  13,  28,  19,  21,   -8,
     -23,  -9,  12,  10,  19,  17,  25,  -16,
     -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
};

const int eg_knight_table[] = {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
};

const int mg_bishop_table[] = {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
     -4,   5,  19,  50,  37,  37,   7,  -2,
     -6,  13,  13,  26,  34,  12,  10,   4,
      0,  15,  15,  15,  14,  27,  18,  10,
      4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
};

const int eg_bishop_table[] = {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
     -8,  -4,   7, -12, -3, -13,  -4, -14,
      2,  -8,   0,  -1, -2,   6,   0,   4,
     -3,   9,  12,   9, 14,  10,   3,   2,
     -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
};

const int mg_rook_table[] = {
     32,  42,  32,  51, 63,  9,  31,  43,
     27,  32,  58,  62, 80, 67,  26,  44,
     -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
};

const int eg_rook_table[] = {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
     7,  7,  7,  5,  4,  -3,  -5,  -3,
     4,  3, 13,  1,  2,   1,  -1,   2,
     3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
};

const int mg_queen_table[] = {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
     -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
     -1, -18,  -9,  10, -15, -25, -31, -50,
};

const int eg_queen_table[] = {
     -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
      3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
};

const int mg_king_table[] = {
    -65,  23,  16, -15, -56, -34,   2,  13,
     29,  -1, -20,  -7,  -8,  -4, -38, -29,
     -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
      1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
};

const int eg_king_table[] = {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
     10,  17,  23,  15,  20,  45,  44,  13,
     -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
};

static const int* mg_pesto_table[6] = {
    mg_pawn_table,
    mg_knight_table,
    mg_bishop_table,
    mg_rook_table,
    mg_queen_table,
    mg_king_table
};

static const int* eg_pesto_table[6] = {
    eg_pawn_table,
    eg_knight_table,
    eg_bishop_table,
    eg_rook_table,
    eg_queen_table,
    eg_king_table
};

static const int gamephase_inc[12] = {
    0,0, 1,1, 1,1, 1,1, 2,2, 4,4
};

static inline int char_to_piece(char ch);

static int mg_table[12][64];
static int eg_table[12][64];

/* --------------------------------------------------------------
 *  Helper terms: pawn structure, mobility, king safety,
 *  piece activity, space, threats, endgame extras
 * -------------------------------------------------------------- */

static int evaluate_pawn_structure(Board* board)
{
    int score = 0;

    /* Passed pawns: no enemy pawn on same or adjacent files ahead */
    int max_black_rank[8];
    int min_white_rank[8];
    for (int f = 0; f < 8; f++) {
        max_black_rank[f] = -1;
        min_white_rank[f] = 8;
    }

    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int f = sq % 8;
        char ascii = board->grid[r][f];
        if (ascii == 'p' && r > max_black_rank[f]) max_black_rank[f] = r;
        if (ascii == 'P' && r < min_white_rank[f]) min_white_rank[f] = r;
    }

    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int f = sq % 8;
        char ascii = board->grid[r][f];
        if (ascii == 'P') {
            int passed = 1;
            for (int df = -1; df <= 1 && passed; df++) {
                int ff = f + df;
                if (ff >= 0 && ff <= 7 && max_black_rank[ff] >= r) passed = 0;
            }
            if (passed && r < 7) score += (6 - r) * 6;
        }
        if (ascii == 'p') {
            int passed = 1;
            for (int df = -1; df <= 1 && passed; df++) {
                int ff = f + df;
                if (ff >= 0 && ff <= 7 && min_white_rank[ff] <= r) passed = 0;
            }
            if (passed && r > 0) score -= (r - 1) * 6;
        }
    }

    /* Doubled and isolated pawns */
    int wpawns[8] = {0}, bpawns[8] = {0};
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int f = sq % 8;
        (void)r;
        char c = board->grid[r][f];
        if (c == 'P') wpawns[f]++;
        if (c == 'p') bpawns[f]++;
    }

    for (int f = 0; f < 8; f++) {
        if (wpawns[f] > 1) score -= (wpawns[f] - 1) * 12;
        if (bpawns[f] > 1) score += (bpawns[f] - 1) * 12;

        int white_adj = 0;
        int black_adj = 0;
        if (f > 0) {
            white_adj += wpawns[f - 1];
            black_adj += bpawns[f - 1];
        }
        if (f < 7) {
            white_adj += wpawns[f + 1];
            black_adj += bpawns[f + 1];
        }

        if (wpawns[f] > 0 && white_adj == 0)
            score -= wpawns[f] * 8;
        if (bpawns[f] > 0 && black_adj == 0)
            score += bpawns[f] * 8;
    }

    return score;
}

static int evaluate_rooks(Board* board)
{
    int score = 0;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {

            char c = board->grid[r][f];

            if (c != 'R' && c != 'r')
                continue;

            int sign = (c == 'R') ? 1 : -1;

            /* rook on 7th / 8th rank */
            if (c == 'R' && r >= 6)
                score += 80;
            if (c == 'r' && r <= 1)
                score -= 80;

            /* rook on open file */
            int file_blocked = 0;
            for (int rr = 0; rr < 8; rr++) {
                char t = board->grid[rr][f];
                if (t == 'P' || t == 'p') {
                    file_blocked = 1;
                    break;
                }
            }

            if (!file_blocked)
                score += sign * 35;

            /* rook near enemy king file */
            for (int rr = 0; rr < 8; rr++) {
                char t = board->grid[rr][f];

                if (c == 'R' && t == 'k')
                    score += 120;

                if (c == 'r' && t == 'K')
                    score -= 120;
            }
        }
    }

    return score;
}

static inline int knight_mobility(Board* board, int r, int f, char me)
{
    static const int offsets[8][2] = {
        { 2,  1}, { 1,  2}, {-1,  2}, {-2,  1},
        {-2, -1}, {-1, -2}, { 1, -2}, { 2, -1}
    };
    int mob = 0;
    for (int i = 0; i < 8; i++) {
        int nr = r + offsets[i][0];
        int nf = f + offsets[i][1];
        if (nr < 0 || nr > 7 || nf < 0 || nf > 7)
            continue;
        char t = board->grid[nr][nf];
        if (!same_color(me, t))
            mob++;
    }
    return mob;
}

static inline int king_mobility(Board* board, int r, int f, char me)
{
    int mob = 0;
    for (int dr = -1; dr <= 1; dr++) {
        for (int df = -1; df <= 1; df++) {
            if (dr == 0 && df == 0) continue;
            int nr = r + dr;
            int nf = f + df;
            if (nr < 0 || nr > 7 || nf < 0 || nf > 7)
                continue;
            char t = board->grid[nr][nf];
            if (!same_color(me, t))
                mob++;
        }
    }
    return mob;
}

static inline int sliding_mobility(Board* board,
                                   int r,
                                   int f,
                                   char me,
                                   const int directions[][2],
                                   int dir_count)
{
    int mob = 0;
    for (int d = 0; d < dir_count; d++) {
        int dr = directions[d][0];
        int df = directions[d][1];
        int nr = r + dr;
        int nf = f + df;
        while (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
            char t = board->grid[nr][nf];
            if (t == '.' || t == ' ') {
                mob++;
            } else {
                if (!same_color(me, t))
                    mob++;
                break;
            }
            nr += dr;
            nf += df;
        }
    }
    return mob;
}

static int evaluate_mobility(Board* board)
{
    int white_mob = 0;
    int black_mob = 0;

    static const int bishop_dirs[4][2] = {
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };
    static const int rook_dirs[4][2] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1}
    };
    static const int queen_dirs[8][2] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char c = board->grid[r][f];
            if (!is_white_piece_char(c) && !is_black_piece_char(c))
                continue;

            switch (c) {
                case 'N':
                    white_mob += knight_mobility(board, r, f, c);
                    break;
                case 'n':
                    black_mob += knight_mobility(board, r, f, c);
                    break;
                case 'B':
                    white_mob += sliding_mobility(board, r, f, c,
                                                  bishop_dirs, 4);
                    break;
                case 'b':
                    black_mob += sliding_mobility(board, r, f, c,
                                                  bishop_dirs, 4);
                    break;
                case 'R':
                    white_mob += sliding_mobility(board, r, f, c,
                                                  rook_dirs, 4);
                    break;
                case 'r':
                    black_mob += sliding_mobility(board, r, f, c,
                                                  rook_dirs, 4);
                    break;
                case 'Q':
                    white_mob += sliding_mobility(board, r, f, c,
                                                  queen_dirs, 8);
                    break;
                case 'q':
                    black_mob += sliding_mobility(board, r, f, c,
                                                  queen_dirs, 8);
                    break;
                case 'K':
                    white_mob += king_mobility(board, r, f, c);
                    break;
                case 'k':
                    black_mob += king_mobility(board, r, f, c);
                    break;
                case 'P': {
                    int nr = r + 1;
                    if (nr <= 7) {
                        if (board->grid[nr][f] == '.' ||
                            board->grid[nr][f] == ' ')
                            white_mob++;
                        if (f > 0 && is_black_piece_char(board->grid[nr][f - 1]))
                            white_mob++;
                        if (f < 7 && is_black_piece_char(board->grid[nr][f + 1]))
                            white_mob++;
                    }
                    break;
                }
                case 'p': {
                    int nr = r - 1;
                    if (nr >= 0) {
                        if (board->grid[nr][f] == '.' ||
                            board->grid[nr][f] == ' ')
                            black_mob++;
                        if (f > 0 && is_white_piece_char(board->grid[nr][f - 1]))
                            black_mob++;
                        if (f < 7 && is_white_piece_char(board->grid[nr][f + 1]))
                            black_mob++;
                    }
                    break;
                }
                default:
                    break;
            }
        }
    }

    return (white_mob - black_mob) * MOBILITY_WEIGHT;
}

static int evaluate_king_safety(Board* board, int game_phase)
{
    UNUSED(game_phase);
    int score = 0;

    int wk_r = -1, wk_f = -1;
    int bk_r = -1, bk_f = -1;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char c = board->grid[r][f];
            if (c == 'K') { wk_r = r; wk_f = f; }
            if (c == 'k') { bk_r = r; bk_f = f; }
        }
    }

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {

            char c = board->grid[r][f];

            if (is_white_piece_char(c)) {

                int dist = abs(r - bk_r) + abs(f - bk_f);
                if (dist <= 2)
                    score += 30;
            }

            if (is_black_piece_char(c)) {

                int dist = abs(r - wk_r) + abs(f - wk_f);
                if (dist <= 2)
                    score -= 30;
            }
        }
    }

    return score;
}

static int evaluate_piece_activity(Board* board)
{
    int score = 0;
    for (int sq = 0; sq < 64; sq++) {
        int r = sq / 8;
        int f = sq % 8;
        (void)f;
        char c = board->grid[r][f];
        if ((c == 'N' || c == 'B' || c == 'R' || c == 'Q') &&
            r >= 2 && r <= 6)
            score += 4;
        if ((c == 'n' || c == 'b' || c == 'r' || c == 'q') &&
            r >= 1 && r <= 5)
            score -= 4;
    }
    return score;
}

static int evaluate_space(Board* board)
{
    int white_space = 0;
    int black_space = 0;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char c = board->grid[r][f];
            if (c == 'P' && r >= 3)
                white_space++;
            if (c == 'p' && r <= 4)
                black_space++;

            if (c == 'N' || c == 'B' || c == 'R' || c == 'Q') {
                if (r >= 2 && r <= 5 && f >= 2 && f <= 5)
                    white_space++;
            }
            if (c == 'n' || c == 'b' || c == 'r' || c == 'q') {
                if (r >= 2 && r <= 5 && f >= 2 && f <= 5)
                    black_space++;
            }
        }
    }

    return (white_space - black_space) * SPACE_WEIGHT;
}

static int evaluate_threats(Board* board)
{
    int white_threat = 0;
    int black_threat = 0;

    static const int bishop_dirs[4][2] = {
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };
    static const int rook_dirs[4][2] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1}
    };
    static const int queen_dirs[8][2] = {
        { 1,  0}, {-1,  0}, { 0,  1}, { 0, -1},
        { 1,  1}, { 1, -1}, {-1,  1}, {-1, -1}
    };

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char c = board->grid[r][f];
            if (!is_white_piece_char(c) && !is_black_piece_char(c))
                continue;

            int sign = is_white_piece_char(c) ? 1 : -1;

            if (c == 'P') {
                int nr = r + 1;
                if (nr <= 7) {
                    if (f > 0 && is_black_piece_char(board->grid[nr][f - 1]))
                        white_threat += simple_piece_value_from_char(board->grid[nr][f - 1]) / 10;
                    if (f < 7 && is_black_piece_char(board->grid[nr][f + 1]))
                        white_threat += simple_piece_value_from_char(board->grid[nr][f + 1]) / 10;
                }
                continue;
            }
            if (c == 'p') {
                int nr = r - 1;
                if (nr >= 0) {
                    if (f > 0 && is_white_piece_char(board->grid[nr][f - 1]))
                        black_threat += simple_piece_value_from_char(board->grid[nr][f - 1]) / 10;
                    if (f < 7 && is_white_piece_char(board->grid[nr][f + 1]))
                        black_threat += simple_piece_value_from_char(board->grid[nr][f + 1]) / 10;
                }
                continue;
            }

            if (c == 'N' || c == 'n') {
                static const int offsets[8][2] = {
                    { 2,  1}, { 1,  2}, {-1,  2}, {-2,  1},
                    {-2, -1}, {-1, -2}, { 1, -2}, { 2, -1}
                };
                for (int i = 0; i < 8; i++) {
                    int nr = r + offsets[i][0];
                    int nf = f + offsets[i][1];
                    if (nr < 0 || nr > 7 || nf < 0 || nf > 7)
                        continue;
                    char t = board->grid[nr][nf];
                    if (t != '.' && t != ' ' && opposite_color(c, t)) {
                        int val = simple_piece_value_from_char(t) / 8;
                        if (sign > 0) white_threat += val;
                        else          black_threat += val;
                    }
                }
            } else if (c == 'B' || c == 'b') {
                for (int d = 0; d < 4; d++) {
                    int dr = bishop_dirs[d][0];
                    int df = bishop_dirs[d][1];
                    int nr = r + dr;
                    int nf = f + df;
                    while (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
                        char t = board->grid[nr][nf];
                        if (t != '.' && t != ' ') {
                            if (opposite_color(c, t)) {
                                int val = simple_piece_value_from_char(t) / 8;
                                if (sign > 0) white_threat += val;
                                else          black_threat += val;
                            }
                            break;
                        }
                        nr += dr;
                        nf += df;
                    }
                }
            } else if (c == 'R' || c == 'r') {
                for (int d = 0; d < 4; d++) {
                    int dr = rook_dirs[d][0];
                    int df = rook_dirs[d][1];
                    int nr = r + dr;
                    int nf = f + df;
                    while (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
                        char t = board->grid[nr][nf];
                        if (t != '.' && t != ' ') {
                            if (opposite_color(c, t)) {
                                int val = simple_piece_value_from_char(t) / 8;
                                if (sign > 0) white_threat += val;
                                else          black_threat += val;
                            }
                            break;
                        }
                        nr += dr;
                        nf += df;
                    }
                }
            } else if (c == 'Q' || c == 'q') {
                for (int d = 0; d < 8; d++) {
                    int dr = queen_dirs[d][0];
                    int df = queen_dirs[d][1];
                    int nr = r + dr;
                    int nf = f + df;
                    while (nr >= 0 && nr <= 7 && nf >= 0 && nf <= 7) {
                        char t = board->grid[nr][nf];
                        if (t != '.' && t != ' ') {
                            if (opposite_color(c, t)) {
                                int val = simple_piece_value_from_char(t) / 8;
                                if (sign > 0) white_threat += val;
                                else          black_threat += val;
                            }
                            break;
                        }
                        nr += dr;
                        nf += df;
                    }
                }
            }
        }
    }

    return (white_threat - black_threat) * THREAT_WEIGHT;
}

static int evaluate_endgame_terms(Board* board, int game_phase)
{
    int score = 0;

    int white_bishops = 0;
    int black_bishops = 0;
    int white_pawns = 0;
    int black_pawns = 0;

    for (int r = 0; r < 8; r++) {
        for (int f = 0; f < 8; f++) {
            char c = board->grid[r][f];
            if (c == 'B') white_bishops++;
            else if (c == 'b') black_bishops++;
            else if (c == 'P') white_pawns++;
            else if (c == 'p') black_pawns++;
        }
    }

    if (white_bishops >= 2)
        score += 15;
    if (black_bishops >= 2)
        score -= 15;

    if (game_phase <= 8) {
        int pawn_diff = white_pawns - black_pawns;
        score += pawn_diff * 4;
    }

    return score;
}

static void rebuild_tables(const int *mg_val, const int *eg_val)
{
    for (int p = 0; p < 6; p++) {
        int pc_white = 2 * p + COLOR_WHITE;
        int pc_black = 2 * p + COLOR_BLACK;
        for (int sq = 0; sq < 64; sq++) {
            mg_table[pc_white][sq] = mg_val[p] + mg_pesto_table[p][sq];
            eg_table[pc_white][sq] = eg_val[p] + eg_pesto_table[p][sq];
            int fsq = flip_sq(sq);
            mg_table[pc_black][sq] = mg_val[p] + mg_pesto_table[p][fsq];
            eg_table[pc_black][sq] = eg_val[p] + eg_pesto_table[p][fsq];
        }
    }
}

void irida_EvalPestoInit(void)
{
    rebuild_tables(mg_value, eg_value);
}

void irida_EvalPestoSetTuneValues(const int mg_val[6], const int eg_val[6])
{
    rebuild_tables(mg_val, eg_val);
}

int irida_EvalPestoMaterialPstEvalWhite(Board* board)
{
    int mg_acc[2] = {0, 0};
    int eg_acc[2] = {0, 0};
    int game_phase = 0;

    for (int sq = 0; sq < 64; sq++) {
        int rank = sq / 8;
        int file = sq % 8;
        char ascii = board->grid[rank][file];
        int pc = char_to_piece(ascii);
        if (pc == EMPTY)
            continue;
        int col = piece_color(pc);
        mg_acc[col] += mg_table[pc][sq];
        eg_acc[col] += eg_table[pc][sq];
        game_phase += gamephase_inc[pc];
    }

    int mg_score = mg_acc[COLOR_WHITE] - mg_acc[COLOR_BLACK];
    int eg_score = eg_acc[COLOR_WHITE] - eg_acc[COLOR_BLACK];
    if (game_phase > 24)
        game_phase = 24;
    int eg_phase = 24 - game_phase;
    return (mg_score * game_phase + eg_score * eg_phase) / 24;
}

static inline int char_to_piece(char ch)
{
    switch (ch) {
        case 'P': return WHITE_PAWN_ENCODING;
        case 'p': return BLACK_PAWN_ENCODING;
        case 'N': return WHITE_KNIGHT_ENCODING;
        case 'n': return BLACK_KNIGHT_ENCODING;
        case 'B': return WHITE_BISHOP_ENCODING;
        case 'b': return BLACK_BISHOP_ENCODING;
        case 'R': return WHITE_ROOK_ENCODING;
        case 'r': return BLACK_ROOK_ENCODING;
        case 'Q': return WHITE_QUEEN_ENCODING;
        case 'q': return BLACK_QUEEN_ENCODING;
        case 'K': return WHITE_KING_ENCODING;
        case 'k': return BLACK_KING_ENCODING;
        default:  return EMPTY;
    }
}

static int pesto_eval_impl(Board* board, EvalBreakdown* out)
{
    int mg_acc[2] = {0, 0};
    int eg_acc[2] = {0, 0};
    int game_phase = 0;

    for (int sq = 0; sq < 64; sq++) {

        int rank = sq / 8;
        int file = sq % 8;

        char ascii = board->grid[rank][file];
        int pc = char_to_piece(ascii);

        if (pc == EMPTY)
            continue;

        int col = piece_color(pc);

        mg_acc[col] += mg_table[pc][sq];
        eg_acc[col] += eg_table[pc][sq];
        game_phase  += gamephase_inc[pc];
    }

    int mg_score = mg_acc[COLOR_WHITE] - mg_acc[COLOR_BLACK];
    int eg_score = eg_acc[COLOR_WHITE] - eg_acc[COLOR_BLACK];

    if (game_phase > 24)
        game_phase = 24;

    int eg_phase = 24 - game_phase;

    int material_pst = (mg_score * game_phase + eg_score * eg_phase) / 24;
    int pawn_structure = evaluate_pawn_structure(board);
    int material = irida_EvalMaterialWhiteMinusBlack(board);
    int mobility = evaluate_mobility(board);
    int king_safety = evaluate_king_safety(board, game_phase);
    int piece_activity = evaluate_piece_activity(board);
    int space = evaluate_space(board);
    int threats = evaluate_threats(board);
    int endgame = evaluate_endgame_terms(board, game_phase);
    int rook_activity = evaluate_rooks(board);

    int score_white = material 
                    + material_pst 
                    + pawn_structure 
                    + rook_activity 
                    + mobility 
                    + king_safety
                    + piece_activity
                    + space 
                    + threats 
                    + endgame;
    int score = board->turn ? score_white : -score_white;

    if (out) {
        out->game_phase = game_phase;
        if (board->turn) {
            out->material = material;
            out->material_pst = material_pst;
            out->pawn_structure = pawn_structure;
            out->mobility = mobility;
            out->king_safety = king_safety;
            out->piece_activity = piece_activity;
            out->space = space;
            out->threats = threats;
            out->endgame = endgame;
            out->total = score_white;
        } else {
            out->material = -material;
            out->material_pst = -material_pst;
            out->pawn_structure = -pawn_structure;
            out->mobility = -mobility;
            out->king_safety = -king_safety;
            out->piece_activity = -piece_activity;
            out->space = -space;
            out->threats = -threats;
            out->endgame = -endgame;
            out->total = -score_white;
        }
    }

    return score;
}

int irida_Evaluation(Board* board)
{
    return pesto_eval_impl(board, NULL);
}

int irida_EvalBreakdown(Board* board, EvalBreakdown* out)
{
    return pesto_eval_impl(board, out);
}

void irida_EvalBreakdownLog(EvalBreakdown b)
{
    fprintf(stderr, "Evaluation Breakdown\n");
    fprintf(stderr, "\t| Material: %d\n", b.material);
    fprintf(stderr, "\t| Material PST: %d\n", b.material_pst);
    fprintf(stderr, "\t| Mobility: %d\n", b.mobility);
    fprintf(stderr, "\t| Pawn Structure: %d\n", b.pawn_structure);
    fprintf(stderr, "\t| Space: %d\n", b.space);
    fprintf(stderr, "\t| Piece Activity: %d\n", b.piece_activity);
    fprintf(stderr, "\t| Threats: %d\n", b.threats);
    fprintf(stderr, "\t| Endgame: %d\n", b.endgame);
    fprintf(stderr, "\t| Game Phase: %d\n", b.game_phase);
    fprintf(stderr, "\t| Total: %d\n", b.total);
}

