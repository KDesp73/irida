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
    printf("Middlegame moves: %zu\n", tuning->thresholds.middlegameMoves);
    printf("Middlegame pieces: %zu\n", tuning->thresholds.middlegamePieces);
    printf("Endgame moves: %zu\n", tuning->thresholds.endgameMoves);
    printf("Endgame piece: %zu\n", tuning->thresholds.endgamePieces);
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
    // NOTE: Should be removed once the phase of the game is being calculated
    tuning->thresholds = (Thresholds) {
        .middlegameMoves = 20,
        .middlegamePieces = 20,
        .endgameMoves = 60,
        .endgamePieces = 12
    };
}
