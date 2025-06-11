#include "tuning.h"
#include <limits.h>
#include <stdio.h>

void PrintTuning(const Tuning* tuning)
{
    printf("Pawn value: %d\n", tuning->pieces.pawn);
    printf("Knight value: %d\n", tuning->pieces.knight);
    printf("Bishop value: %d\n", tuning->pieces.bishop);
    printf("Rook value: %d\n", tuning->pieces.rook);
    printf("Queen value: %d\n", tuning->pieces.queen);
    printf("King value: %d\n", tuning->pieces.king);
    printf("\n");
}

void LoadTuning(Tuning* tuning)
{
    tuning->pieces = (PieceValues){
        .pawn = PAWN_VALUE,
        .knight = KNIGHT_VALUE,
        .bishop = BISHOP_VALUE,
        .rook = ROOK_VALUE,
        .queen = QUEEN_VALUE,
        .king = KING_VALUE
    };

    // Pawn structure
    tuning->doubledPawnPenalty = -10;
    tuning->isolatedPawnPenalty = -15;
    tuning->backwardPawnPenalty = -8;
    for (int i = 0; i < 8; ++i)
        tuning->passedPawnBonus[i] = i * 10; // more bonus for advanced passed pawns

    for (int i = 0; i < 9; ++i)
        tuning->knightMobility[i] = i * 4;
    for (int i = 0; i < 14; ++i)
        tuning->bishopMobility[i] = i * 3;
    for (int i = 0; i < 15; ++i)
        tuning->rookMobility[i] = i * 2;
    for (int i = 0; i < 28; ++i)
        tuning->queenMobility[i] = i * 1;

    // King safety
    tuning->pawnShieldBonus = 10;
    tuning->openFileKingPenalty = -20;
    tuning->kingRingAttackWeight = 5;

    // Piece activity
    tuning->rookOpenFileBonus = 15;
    tuning->rookSemiOpenFileBonus = 8;
    tuning->bishopPairBonus = 30;

    // Threats
    tuning->minorThreatBonus = 20;
    tuning->majorThreatBonus = 40;
    tuning->hangingPiecePenalty = -50;

    // Space
    tuning->spaceControlBonus = 1; // per square in center

    // Tempo
    tuning->tempoBonus = 10;

    // Piece-square tables (centralized, aggressive early-game examples)
    for (int i = 0; i < 64; ++i) {
        // Use simplified PSTs (you may want to load these from files or external data)
        tuning->pstPawn[i] = 0;
        tuning->pstKnight[i] = 0;
        tuning->pstBishop[i] = 0;
        tuning->pstRook[i] = 0;
        tuning->pstQueen[i] = 0;
        tuning->pstKingMidgame[i] = 0;
        tuning->pstKingEndgame[i] = 0;
    }

    // Lazy eval / pruning
    tuning->nullMoveReduction = 3;
    tuning->futilityMargin = 200;

    // Game phase weights (relative importance in phase interpolation)
    tuning->phaseWeight[0] = 0;    // PAWN
    tuning->phaseWeight[1] = 1;    // KNIGHT
    tuning->phaseWeight[2] = 1;    // BISHOP
    tuning->phaseWeight[3] = 2;    // ROOK
    tuning->phaseWeight[4] = 4;    // QUEEN
    tuning->phaseWeight[5] = 0;    // KING
}
