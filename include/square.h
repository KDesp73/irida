#ifndef ENGINE_SQUARE_H
#define ENGINE_SQUARE_H

/*------------------------------------.
| Bitboard representation of squares. |
| Square is an 8-bit integer ranging  |
| from 0-63.                          |
|                                     |
| We start counting from a8 and       |
| finish with h1                      |
`------------------------------------*/

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t Square;

#define SQUARE_PRINT(square)\
    do{\
        char __square_name__[3];\
        SquareToName(__square_name__, square);\
        printf("%s = %s %d\n", #square, __square_name__, square);\
    } while(0)

#define COORDS(square) (square) / 8][(square) % 8

void SquareToName(char buffer[3], Square square);
Square NameToSquare(const char buffer[3]);

int Rank(Square square);
int File(Square square);
bool IsSquareValid(Square square);

Square SquareFromCoords(size_t y, size_t x);
Square SquareFromName(const char* name);

#define SR(s)\
    (7 - (s/ 8)) * 8 + (s% 8)


#endif // ENGINE_SQUARE_H
