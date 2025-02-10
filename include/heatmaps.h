#ifndef ENGINE_PIECE_TABLES_H
#define ENGINE_PIECE_TABLES_H

// TODO: use different tables depending on the stage of the game

#include "board.h"
#include "square.h"
#include "tuning.h"
static const int PawnTable[64] = {
    0,  0,  0,  0,  0,  0,  0,  0,
    50, 50, 50, 50, 50, 50, 50, 50,
    10, 10, 20, 30, 30, 20, 10, 10,
    5,  5,  10, 25, 25, 10, 5,  5,
    0,  0,  0,  20, 20, 0,  0,  0,
    5, -5, -10, 0,  0, -10, -5, 5,
    10, 5,  0,  0,  0,  0,  5,  10,
    0,  0,  0,  0,  0,  0,  0,  0
};
static const int PawnTableMiddlegame[64] ={}; // TODO: define the table
static const int PawnTableEndgame[64] ={}; // TODO: define the table
int PawnTableValue(const Board* board, const Tuning* tuning, Square square);

static const int KnightTable[64] = {
    -50, -40, -30, -30, -30, -30, -40, -50,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 5, 15, 20, 20, 15, 5, -30,
    -30, 5, 10, 15, 15, 10, 5, -30,
    -40, -20, 0, 5, 5, 0, -20, -40,
    -50, -40, -30, -30, -30, -30, -40, -50
};
static const int KnightTableMiddlegame[64] ={}; // TODO: define the table
static const int KnightTableEndgame[64] ={}; // TODO: define the table
int KnightTableValue(const Board* board, const Tuning* tuning, Square square);

static const int BishopTable[64] = {
    -20, -10, -10, -10, -10, -10, -10, -20,
    -10, 5, 10, 10, 10, 10, 5, -10,
    -10, 10, 20, 20, 20, 20, 10, -10,
    -10, 10, 20, 30, 30, 20, 10, -10,
    -10, 10, 20, 30, 30, 20, 10, -10,
    -10, 10, 20, 20, 20, 20, 10, -10,
    -10, 5, 10, 10, 10, 10, 5, -10,
    -20, -10, -10, -10, -10, -10, -10, -20
};
static const int BishopTableMiddlegame[64] ={}; // TODO: define the table
static const int BishopTableEndgame[64] ={}; // TODO: define the table
int BishopTableValue(const Board* board, const Tuning* tuning, Square square);

static const int RookTable[64] = {
    0, 0, 0, 5, 5, 0, 0, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 5, 10, 20, 20, 10, 5, 0,
    5, 10, 20, 30, 30, 20, 10, 5,
    5, 10, 20, 30, 30, 20, 10, 5,
    0, 5, 10, 20, 20, 10, 5, 0,
    0, 0, 5, 10, 10, 5, 0, 0,
    0, 0, 0, 5, 5, 0, 0, 0
};
static const int RookTableMiddlegame[64] ={}; // TODO: define the table
static const int RookTableEndgame[64] ={}; // TODO: define the table
int RookTableValue(const Board* board, const Tuning* tuning, Square square);

static const int QueenTable[64] = {
    -20, -10, -10, -5, -5, -10, -10, -20,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -10, 5, 10, 20, 20, 10, 5, -10,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -5, 10, 20, 30, 30, 20, 10, -5,
    -10, 5, 10, 20, 20, 10, 5, -10,
    -10, 0, 5, 10, 10, 5, 0, -10,
    -20, -10, -10, -5, -5, -10, -10, -20
};
static const int QueenTableMiddlegame[64] ={}; // TODO: define the table
static const int QueenTableEndgame[64] ={}; // TODO: define the table
int QueenTableValue(const Board* board, const Tuning* tuning, Square square);

static const int KingTable[64] = {
    -30, -40, -50, -60, -60, -50, -40, -30,
    -40, -50, -60, -70, -70, -60, -50, -40,
    -50, -60, -70, -80, -80, -70, -60, -50,
    -60, -70, -80, -90, -90, -80, -70, -60,
    -60, -70, -80, -90, -90, -80, -70, -60,
    -50, -60, -70, -80, -80, -70, -60, -50,
    -40, -50, -60, -70, -70, -60, -50, -40,
    -30, -40, -50, -60, -60, -50, -40, -30
};
static const int KingTableMiddlegame[64] ={}; // TODO: define the table
static const int KingTableEndgame[64] ={}; // TODO: define the table
int KingTableValue(const Board* board, const Tuning* tuning, Square square);

static inline bool isMiddlegame(const Board* board, const Tuning* tuning)
{
    size_t piecesCount = NumberOfPieces(board, COLOR_WHITE) + NumberOfPieces(board, COLOR_BLACK);
    size_t movesPlayed = board->halfmove;

    return (
            (piecesCount > tuning->thresholds.endgamePieces && piecesCount <= tuning->thresholds.middlegamePieces) ||
            (movesPlayed < tuning->thresholds.endgameMoves  && movesPlayed >= tuning->thresholds.middlegameMoves)
    );

}

static inline bool isEndgame(const Board* board, const Tuning* tuning)
{
    size_t piecesCount = NumberOfPieces(board, COLOR_WHITE) + NumberOfPieces(board, COLOR_BLACK);
    size_t movesPlayed = board->halfmove;

    return (
            piecesCount <= tuning->thresholds.endgamePieces ||
            movesPlayed >= tuning->thresholds.endgameMoves
    );

}

#endif // ENGINE_PIECE_TABLES_H
