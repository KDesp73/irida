#include "heatmaps.h"

static inline int flipSquare(Square square) {
    return (square ^ 56); // Flip rank
}

int PawnTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return PawnTableEndgame[index];
    if (isMiddlegame(board, tuning)) return PawnTableMiddlegame[index];
    return PawnTable[index];
}

int KnightTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return KnightTableEndgame[index];
    if (isMiddlegame(board, tuning)) return KnightTableMiddlegame[index];
    return KnightTable[index];
}

int BishopTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return BishopTableEndgame[index];
    if (isMiddlegame(board, tuning)) return BishopTableMiddlegame[index];
    return BishopTable[index];
}

int RookTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return RookTableEndgame[index];
    if (isMiddlegame(board, tuning)) return RookTableMiddlegame[index];
    return RookTable[index];
}

int QueenTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return QueenTableEndgame[index];
    if (isMiddlegame(board, tuning)) return QueenTableMiddlegame[index];
    return QueenTable[index];
}

int KingTableValue(const Board* board, const Tuning* tuning, Square square, bool isWhite)
{
    int index = isWhite ? square : flipSquare(square);
    if (isEndgame(board, tuning)) return KingTableEndgame[index];
    if (isMiddlegame(board, tuning)) return KingTableMiddlegame[index];
    return KingTable[index];
}
