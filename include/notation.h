#ifndef ENGINE_NOTATION_H
#define ENGINE_NOTATION_H

/*--------------------------------.
| Contains Fen Import and Export  |
| Pgn Import and Export are       |
| already implemented in libchess |
`-------------------------------*/

#include "move.h"
#include <chess/notation.h>

void FenImport(Board* board, const char* fen);
void FenExport(const Board* board, char fen[]);

void MoveToSan(Board board, Move move, san_move_t* san);
Move SanToMove(Board board, san_move_t san);


#endif // ENGINE_NOTATION_H
