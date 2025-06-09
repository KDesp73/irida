#ifndef ENGINE_TUNING_H
#define ENGINE_TUNING_H

#include <stddef.h>
#define PAWN_VALUE   100
#define KNIGHT_VALUE 310
#define BISHOP_VALUE 330
#define ROOK_VALUE   500
#define QUEEN_VALUE  900
#define KING_VALUE   20000

typedef struct {
    int pawn;
    int knight;
    int bishop;
    int rook;
    int queen;
    int king;
} PieceValues;

typedef struct {
    size_t middlegameMoves;
    size_t middlegamePieces;
    size_t endgameMoves;
    size_t endgamePieces;
} Thresholds;

typedef struct {
    PieceValues pieces;
    Thresholds thresholds;
} Tuning;

void PrintTuning(const Tuning* tuning);
void LoadTuning(Tuning* tuning);
#define TUNING_SCRIPT "tuning.lua"

#define IS_SET(x) ((x) != 0)
#define THIS_OR(x, y) IS_SET(x) ? x : y

#endif // ENGINE_TUNING_H
