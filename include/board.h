#ifndef ENGINE_BOARD_H
#define ENGINE_BOARD_H

#include "history.h"
#include "square.h"
#include <stddef.h>
#include <stdint.h>
#include "bitboard.h"

#define BLACK_ROOK 'r'
#define BLACK_KNIGHT 'n'
#define BLACK_BISHOP 'b'
#define BLACK_KING 'k'
#define BLACK_QUEEN 'q'
#define BLACK_PAWN 'p'

#define WHITE_ROOK 'R'
#define WHITE_KNIGHT 'N'
#define WHITE_BISHOP 'B'
#define WHITE_KING 'K'
#define WHITE_QUEEN 'Q'
#define WHITE_PAWN 'P'

#define EMPTY_SQUARE ' '

#define STARTING_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"

#define BOARD_SIZE 8
#define PIECE_TYPES 12

/*---------------------------.
| Hybrid Representation      |
| `grid` is used for fast    |
| piece lookup               |
`--------------------------*/

typedef struct {
    Bitboard bitboards[PIECE_TYPES];
    char grid[8][8];
    Bitboard empty;

    // State
    Square enpassant_square;
    bool turn;
    uint8_t castling_rights;
    size_t halfmove;
    size_t fullmove;

    History history;
} Board;

bool AddUndo(Board* board, uint32_t move);
Undo LoadLastUndo(Board* board);

#define PIECES "pnbrqkPNBRQK"
enum {
    INDEX_BLACK_PAWN,
    INDEX_BLACK_KNIGHT,
    INDEX_BLACK_BISHOP,
    INDEX_BLACK_ROOK,
    INDEX_BLACK_QUEEN,
    INDEX_BLACK_KING,
    INDEX_WHITE_PAWN,
    INDEX_WHITE_KNIGHT,
    INDEX_WHITE_BISHOP,
    INDEX_WHITE_ROOK,
    INDEX_WHITE_QUEEN,
    INDEX_WHITE_KING,
};
#define INDEX_PAWN INDEX_BLACK_PAWN
#define INDEX_KNIGHT INDEX_BLACK_KNIGHT
#define INDEX_BISHOP INDEX_BLACK_BISHOP
#define INDEX_ROOK INDEX_BLACK_ROOK
#define INDEX_QUEEN INDEX_BLACK_QUEEN
#define INDEX_KING INDEX_BLACK_KING

typedef enum {
    COLOR_NONE = -1,
    COLOR_BLACK = 0,
    COLOR_WHITE,
} PieceColor;

char PromotionToChar(uint8_t promotion);
uint8_t CharToPromotion(char promotion);

void BoardInitFen(Board* board, const char* fen);
Board* BoardInitFenHeap(const char* fen);
void BoardFree(Board* board);

// TODO: store in seperate bitboard instead of calculating
Bitboard GetWhite(const Board* board);
Bitboard GetBlack(const Board* board);
Bitboard GetEnemyColor(const Board *board, PieceColor us);
Bitboard GetEnemy(const Board* board);
Bitboard GetEmpty(const Board* board);

int HasCastlingRights(const Board* board, uint8_t castling_rights);
void RevokeCastlingRights(Board* board, uint8_t castling_rights);

bool IsSquareAttacked(const Board* board, Square square, PieceColor color);
bool IsSquareEmpty(const Board* board, Square square);
bool IsSquareOccupiedBy(const Board* board, Square square, PieceColor color);

size_t NumberOfPieces(const Board* board, PieceColor color);

bool IsInCheckColor(const Board* board, PieceColor color);
bool IsInCheck(const Board* board);

void BoardPrintSquares(const Board* board, Square* squares, size_t count);
void BoardPrintBitboard(const Board* board, Bitboard highlight);
void BoardPrint(const Board* board, Square first, ...);
void BoardPrintBitboards(Board board);
void BoardPrintGrid(const Board* board);

Board BoardCopy(const Board* board);

#endif // ENGINE_BOARD_H
