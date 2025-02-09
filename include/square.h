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

// Rank 1
#define A1  0
#define B1  1
#define C1  2
#define D1  3
#define E1  4
#define F1  5
#define G1  6
#define H1  7

// Rank 2
#define A2  8
#define B2  9
#define C2 10
#define D2 11
#define E2 12
#define F2 13
#define G2 14
#define H2 15

// Rank 3
#define A3 16
#define B3 17
#define C3 18
#define D3 19
#define E3 20
#define F3 21
#define G3 22
#define H3 23

// Rank 4
#define A4 24
#define B4 25
#define C4 26
#define D4 27
#define E4 28
#define F4 29
#define G4 30
#define H4 31

// Rank 5
#define A5 32
#define B5 33
#define C5 34
#define D5 35
#define E5 36
#define F5 37
#define G5 38
#define H5 39

// Rank 6
#define A6 40
#define B6 41
#define C6 42
#define D6 43
#define E6 44
#define F6 45
#define G6 46
#define H6 47

// Rank 7
#define A7 48
#define B7 49
#define C7 50
#define D7 51
#define E7 52
#define F7 53
#define G7 54
#define H7 55

// Rank 8
#define A8 56
#define B8 57
#define C8 58
#define D8 59
#define E8 60
#define F8 61
#define G8 62
#define H8 63

#define SQUARE_NONE 64

#endif // ENGINE_SQUARE_H
