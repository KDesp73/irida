#ifndef ENGINE_GENERATOR_H
#define  ENGINE_GENERATOR_H

#include "bitboard.h"
#include "board.h"
#include "move.h"

/*** Pseudo-Legal Moves ***/

Moves GeneratePseudoLegalMoves(const Board* board);
Bitboard GeneratePseudoLegalMovesBitboard(const Board* board);

Bitboard GeneratePseudoLegalPawnMoves(Bitboard pawns, Bitboard enemy, Color color);
Bitboard GeneratePseudoLegalPawnAttacks(Bitboard pawns, Bitboard enemy, Color color);
Bitboard GeneratePseudoLegalKnightAttacks(Bitboard knights, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalBishopAttacks(Bitboard bishops, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalRookAttacks(Bitboard rooks, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalQueenAttacks(Bitboard queens, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalKingAttacks(Bitboard kings, Bitboard empty, Bitboard enemy);
Bitboard GeneratePseudoLegalAttacks(const Board* board, Color color);

Bitboard GeneratePawnMoves(const Board* board, Square piece, Color color);
Bitboard GenerateKnightMoves(const Board* board, Square piece, Color color);
Bitboard GenerateBishopMoves(const Board* board, Square piece, Color color);
Bitboard GenerateRookMoves(const Board* board, Square piece, Color color);
Bitboard GenerateQueenMoves(const Board* board, Square piece, Color color);
Bitboard GenerateKingMoves(const Board* board, Square piece, Color color);

/*** Legal Moves ***/

bool IsLegal(const Board* board, Move move);

Moves GenerateLegalMoves(const Board* board);
Bitboard GenerateLegalMovesBitboard(const Board* board);

Moves GenerateLegalPawnMoves(const Board* board, Bitboard pieces, Color color);
Moves GenerateLegalKnightMoves(const Board* board, Bitboard pieces, Color color);
Moves GenerateLegalBishopMoves(const Board* board, Bitboard pieces, Color color);
Moves GenerateLegalRookMoves(const Board* board, Bitboard pieces, Color color);
Moves GenerateLegalQueenMoves(const Board* board, Bitboard pieces, Color color);
Moves GenerateLegalKingMoves(const Board* board, Bitboard pieces, Color color);

#endif //ENGINE_GENERATOR_H
