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

Moves GenerateMoves(const Board* board, MoveType type);

/*** Pseudo-Legal Moves ***/

Moves GeneratePseudoLegalMoves(const Board* board);
Bitboard GeneratePseudoLegalMovesBitboard(const Board* board);

Bitboard GeneratePseudoLegalPawnMoves(Bitboard pawns, Bitboard enemy, PieceColor color);
Bitboard GeneratePseudoLegalPawnAttacks(Bitboard pawns, Bitboard enemy, PieceColor color, bool strict);
Bitboard GeneratePseudoLegalKnightAttacks(Bitboard knights, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalBishopAttacks(Bitboard bishops, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalRookAttacks(Bitboard rooks, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalQueenAttacks(Bitboard queens, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalKingAttacks(Bitboard kings, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalAttacks(const Board* board, PieceColor color);

Bitboard GeneratePawnMoves(const Board* board, Square piece, PieceColor color);
Bitboard GenerateKnightMoves(const Board* board, Square piece, PieceColor color);
Bitboard GenerateBishopMoves(const Board* board, Square piece, PieceColor color);
Bitboard GenerateRookMoves(const Board* board, Square piece, PieceColor color);
Bitboard GenerateQueenMoves(const Board* board, Square piece, PieceColor color);
Bitboard GenerateKingMoves(const Board* board, Square piece, PieceColor color);

/*** Legal Moves ***/

bool IsLegal(const Board* board, Move move);

Moves GenerateLegalMoves(const Board* board);
Moves GenerateLegalMovesSquare(const Board* board, Square square);
Bitboard GenerateLegalMovesBitboard(const Board* board);

Moves GenerateLegalPawnMoves(const Board* board, Bitboard pieces, PieceColor color);
Moves GenerateLegalKnightMoves(const Board* board, Bitboard pieces, PieceColor color);
Moves GenerateLegalBishopMoves(const Board* board, Bitboard pieces, PieceColor color);
Moves GenerateLegalRookMoves(const Board* board, Bitboard pieces, PieceColor color);
Moves GenerateLegalQueenMoves(const Board* board, Bitboard pieces, PieceColor color);
Moves GenerateLegalKingMoves(const Board* board, Bitboard pieces, PieceColor color);

#endif //ENGINE_MOVEGEN_H
