#ifndef ENGINE_TUNING_H
#define ENGINE_TUNING_H

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
    PieceValues pieces;
} Tuning;

void PrintTuning(const Tuning* tuning);
void LoadTuning(Tuning* tuning);
#define TUNING_SCRIPT "tuning.lua"

#endif // ENGINE_TUNING_H
