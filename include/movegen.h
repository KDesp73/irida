#ifndef ENGINE_MOVEGEN_H
#define  ENGINE_MOVEGEN_H

#include "bitboard.h"
#include "board.h"
#include "move.h"

typedef enum {
    MOVE_LEGAL,        // A fully legal move after legality checks
    MOVE_PSEUDO,       // A pseudo-legal move (ignores check legality)
    MOVE_CAPTURES,     // A move that captures an opponent’s piece
    MOVE_QUIET,        // A non-capturing, non-promoting move
    MOVE_CHECK,        // A move that puts the opponent in check
    MOVE_CHECKMATE,    // A move that delivers checkmate
    MOVE_CASTLING,     // A king-side or queen-side castling move
    MOVE_EN_PASSANT,   // A special pawn capture move
    MOVE_PROMOTION,    // A pawn promotion move
    MOVE_ILLEGAL       // An invalid move (used for error handling)
} MoveType;

/*** Pseudo-Legal Moves ***/

Moves GeneratePseudoLegalMoves(const Board* board, MoveType type);
Bitboard GeneratePseudoLegalMovesBitboard(const Board* board, MoveType type);

Bitboard GeneratePseudoLegalPawnMoves(Bitboard pawns, Bitboard enemy, PieceColor color);
Bitboard GeneratePseudoLegalPawnAttacks(Bitboard pawns, Bitboard enemy, PieceColor color, bool strict);
Bitboard GeneratePseudoLegalKnightAttacks(Bitboard knights, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalBishopAttacks(Bitboard bishops, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalRookAttacks(Bitboard rooks, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalQueenAttacks(Bitboard queens, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalKingAttacks(Bitboard kings, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalAttacks(const Board* board, PieceColor color);

Bitboard GeneratePawnMoves(const Board* board, Square piece, PieceColor color, MoveType type);
Bitboard GenerateKnightMoves(const Board* board, Square piece, PieceColor color, MoveType type);
Bitboard GenerateBishopMoves(const Board* board, Square piece, PieceColor color, MoveType type);
Bitboard GenerateRookMoves(const Board* board, Square piece, PieceColor color, MoveType type);
Bitboard GenerateQueenMoves(const Board* board, Square piece, PieceColor color, MoveType type);
Bitboard GenerateKingMoves(const Board* board, Square piece, PieceColor color, MoveType type);

/*** Legal Moves ***/

bool IsLegal(const Board* board, Move move);

Moves GenerateMoves(const Board* board, MoveType type);
Moves GenerateMovesSquare(const Board* board, Square square, MoveType type);
Bitboard GenerateMovesBitboard(const Board* board, MoveType type);

Moves GenerateLegalPawnMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);
Moves GenerateLegalKnightMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);
Moves GenerateLegalBishopMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);
Moves GenerateLegalRookMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);
Moves GenerateLegalQueenMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);
Moves GenerateLegalKingMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type);

#define TYPE_ERROR(type) \
    do { \
        fprintf(stderr, "You can't generate " #type " moves from this function %s:%d\n", __FILE__, __LINE__); \
        exit(1); \
    } while(0) \

#endif //ENGINE_MOVEGEN_H
