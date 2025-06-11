#ifndef ENGINE_TUNING_H
#define ENGINE_TUNING_H

#include <stddef.h>

// Material values
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

    // Pawn structure
    int doubledPawnPenalty;
    int isolatedPawnPenalty;
    int backwardPawnPenalty;
    int passedPawnBonus[8]; // per rank

    // Mobility
    int knightMobility[9];  // Number of squares attacked
    int bishopMobility[14];
    int rookMobility[15];
    int queenMobility[28];

    // King safety
    int pawnShieldBonus;
    int openFileKingPenalty;
    int kingRingAttackWeight;

    // Piece activity
    int rookOpenFileBonus;
    int rookSemiOpenFileBonus;
    int bishopPairBonus;

    // Threats
    int minorThreatBonus;
    int majorThreatBonus;
    int hangingPiecePenalty;

    // Space
    int spaceControlBonus;

    // Tempo
    int tempoBonus;

    // Piece-square tables
    int pstPawn[64];
    int pstKnight[64];
    int pstBishop[64];
    int pstRook[64];
    int pstQueen[64];
    int pstKingMidgame[64];
    int pstKingEndgame[64];

    // Lazy eval / pruning
    int nullMoveReduction;
    int futilityMargin;

    // Scale factor for interpolation
    int phaseWeight[6]; // PAWN to KING
} Tuning;

void PrintTuning(const Tuning* tuning);
void LoadTuning(Tuning* tuning);

#define IS_SET(x) ((x) != 0)
#define THIS_OR(x, y) IS_SET(x) ? x : y

#endif // ENGINE_TUNING_H
