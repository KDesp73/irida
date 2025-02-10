#include "heatmaps.h"

int PawnTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return PawnTableEndgame[square];
    if(isMiddlegame(board, tuning)) return PawnTableMiddlegame[square];
    return PawnTable[square];
}

int KnightTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return KnightTableEndgame[square];
    if(isMiddlegame(board, tuning)) return KnightTableMiddlegame[square];
    return KnightTable[square];
}

int BishopTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return BishopTableEndgame[square];
    if(isMiddlegame(board, tuning)) return BishopTableMiddlegame[square];
    return BishopTable[square];
}

int RookTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return RookTableEndgame[square];
    if(isMiddlegame(board, tuning)) return RookTableMiddlegame[square];
    return RookTable[square];
}

int QueenTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return QueenTableEndgame[square];
    if(isMiddlegame(board, tuning)) return QueenTableMiddlegame[square];
    return QueenTable[square];
}

int KingTableValue(const Board* board, const Tuning* tuning, Square square)
{
    if(isEndgame(board, tuning)) return KingTableEndgame[square];
    if(isMiddlegame(board, tuning)) return KingTableMiddlegame[square];
    return KingTable[square];
}

