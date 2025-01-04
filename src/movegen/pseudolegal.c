#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "movegen.h"
#include "move.h"
#include "square.h"
#include <stdio.h>

Bitboard GeneratePseudoLegalPawnMoves(Bitboard pawns, Bitboard empty, PieceColor color)
{
    Bitboard result = 0ULL;
    while(pawns) {
        Square current = poplsb(&pawns);
        result |= PawnPushes(current, empty, color);
    }
    return result;
}

Bitboard GeneratePseudoLegalPawnAttacks(Bitboard pawns, Bitboard enemy, PieceColor color, bool strict)
{
    Bitboard result = 0ULL;
    while(pawns){
        Square current = poplsb(&pawns);
        if(strict)
            result |= PawnAttacks(current, enemy, color);
        else
            result |= PawnAttackMask(current, color);
    }
    return result;
}

Bitboard GeneratePseudoLegalKnightAttacks(Bitboard knights, Bitboard empty, Bitboard enemy)
{
    Bitboard result = 0ULL;
    while(knights){
        Square current = poplsb(&knights);
        result |= KnightAttacks(current, empty, enemy);
    }
    return result;
}

Bitboard GeneratePseudoLegalBishopAttacks(Bitboard bishops, Bitboard empty, Bitboard enemy)
{
    Bitboard result = 0ULL;
    while(bishops){
        Square current = poplsb(&bishops);
        result |= BishopAttacks(current, empty, enemy);
    }
    return result;
}

Bitboard GeneratePseudoLegalRookAttacks(Bitboard rooks, Bitboard empty, Bitboard enemy)
{
    Bitboard result = 0ULL;
    while(rooks){
        Square current = poplsb(&rooks);
        result |= RookAttacks(current, empty, enemy);
    }
    return result;
}

Bitboard GeneratePseudoLegalQueenAttacks(Bitboard queens, Bitboard empty, Bitboard enemy)
{
    Bitboard result = 0ULL;
    while(queens){
        Square current = poplsb(&queens);
        result |= QueenAttacks(current, empty, enemy);
    }
    return result;
}

Bitboard GeneratePseudoLegalKingAttacks(Bitboard kings, Bitboard empty, Bitboard enemy)
{
    Bitboard result = 0ULL;
    while(kings){
        Square current = poplsb(&kings);
        result |= KingAttacks(current, empty, enemy);
    }
    return result;
}


Bitboard GeneratePseudoLegalAttacks(const Board* board, PieceColor attackerColor)
{
    size_t start = (attackerColor) ? 6 : 0;
    Bitboard enemy = GetEnemyColor(board, attackerColor);
    Bitboard empty = GetEmpty(board);
    Square enpassantSquare = board->enpassant_square;

    Bitboard attacks = 
        GeneratePseudoLegalPawnAttacks(board->bitboards[start + INDEX_BLACK_PAWN], enemy, attackerColor, false)
        | GeneratePseudoLegalKnightAttacks(board->bitboards[start + INDEX_BLACK_KNIGHT], empty, enemy)
        | GeneratePseudoLegalBishopAttacks(board->bitboards[start + INDEX_BLACK_BISHOP], empty, enemy)
        | GeneratePseudoLegalRookAttacks(board->bitboards[start + INDEX_BLACK_ROOK], empty, enemy)
        | GeneratePseudoLegalQueenAttacks(board->bitboards[start + INDEX_BLACK_QUEEN], empty, enemy)
        | GeneratePseudoLegalKingAttacks(board->bitboards[start + INDEX_BLACK_KING], empty, enemy)
        ;

    if(enpassantSquare != 64){
        if(attackerColor == COLOR_WHITE){
            if((1ULL << enpassantSquare) & RANK_6){
                attacks |= (
                        PawnAttacks(board->bitboards[INDEX_WHITE_PAWN], ~0ULL, attackerColor) 
                        & (1ULL << enpassantSquare)
                        );
            }
        } else {
            if((1ULL << enpassantSquare) & RANK_3){
                attacks |= (
                        PawnAttacks(board->bitboards[INDEX_BLACK_PAWN], ~0ULL, attackerColor) 
                        & (1ULL << enpassantSquare)
                        );
            }
        }
    }

    return attacks;
}


Bitboard GeneratePawnMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard pseudoLegal = 0ULL;
    Square enpassantSquare = board->enpassant_square;
    Bitboard enemySquares = GetEnemy(board);
    Bitboard emptySquares = GetEmpty(board);

    Bitboard enpassantBB = BB(enpassantSquare);
    pseudoLegal = PawnAttacks(piece, enemySquares, color)
        | PawnPushes(piece, emptySquares, color);

    if((color == WHITE && enpassantBB & RANK_6) ||
        (color == BLACK && enpassantBB & RANK_3))
        pseudoLegal |= (PawnAttackMask(piece, color) & enpassantBB);

    return pseudoLegal;
}

Bitboard GenerateKnightMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return KnightAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateBishopMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return BishopAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateRookMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return RookAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateQueenMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return QueenAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateKingMoves(const Board* board, Square piece, PieceColor color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard pseudoLegal = KingAttacks(piece, emptySquares, enemySquares);

    if(IsInCheckColor(board, color)) return pseudoLegal;

    // TODO: replace the logic below with bitmasks
    Bitboard castlingMoves = 0ULL;
    if (color == COLOR_WHITE && piece == 4) {
        // White Kingside Castling: Squares e1 -> f1 -> g1 (4 -> 5 -> 6)
        if (HasCastlingRights(board, CASTLE_WHITE_KINGSIDE)
            && IsSquareEmpty(board, 5)
            && IsSquareEmpty(board, 6)
            && !IsSquareAttacked(board, 5, COLOR_BLACK)
            && !IsSquareAttacked(board, 6, COLOR_BLACK)
        ) {
            on(&castlingMoves, 6);
        }
        // White Queenside Castling: Squares e1 -> d1 -> c1 (4 -> 3 -> 2)
        if (HasCastlingRights(board, CASTLE_WHITE_QUEENSIDE)
            && IsSquareEmpty(board, 3)
            && IsSquareEmpty(board, 2) 
            && IsSquareEmpty(board, 1) 
            && !IsSquareAttacked(board, 3, COLOR_BLACK)
            && !IsSquareAttacked(board, 2, COLOR_BLACK)
        ) {
            on(&castlingMoves, 2);
        }
    } else if(color == COLOR_BLACK && piece == 60){
        // Black Kingside Castling: Squares e8 -> f8 -> g8 (60 -> 61 -> 62)
        if (HasCastlingRights(board, CASTLE_BLACK_KINGSIDE)
            && IsSquareEmpty(board, 61)
            && IsSquareEmpty(board, 62)
            && !IsSquareAttacked(board, 61, COLOR_WHITE)
            && !IsSquareAttacked(board, 62, COLOR_WHITE)
        ) {
            on(&castlingMoves, 62);
        }
        // Black Queenside Castling: Squares e8 -> d8 -> c8 (60 -> 59 -> 58)
        if (HasCastlingRights(board, CASTLE_BLACK_QUEENSIDE)
            && IsSquareEmpty(board, 59)
            && IsSquareEmpty(board, 58)
            && IsSquareEmpty(board, 57)
            && !IsSquareAttacked(board, 59, COLOR_WHITE)
            && !IsSquareAttacked(board, 58, COLOR_WHITE)
        ) {
            on(&castlingMoves, 58); // Queenside castling
        }
    }

    pseudoLegal |= castlingMoves;

    return pseudoLegal;
}

Bitboard GeneratePseudoLegalMovesBitboard(const Board* board)
{
    return MovesToBitboard(GeneratePseudoLegalMoves(board));
}

Moves GeneratePseudoLegalMoves(const Board* board)
{
    Moves moves = {0};
    PieceColor color = board->turn;

    Bitboard pawnsBB = board->bitboards[color*6 + INDEX_BLACK_PAWN];
    while(pawnsBB){
        Square current = poplsb(&pawnsBB);
        MovesAppendList(&moves, BitboardToMoves(GeneratePawnMoves(board, current, color), current));
    }

    Bitboard knightsBB = board->bitboards[color*6 + INDEX_BLACK_KNIGHT];
    while(knightsBB){
        Square current = poplsb(&knightsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKnightMoves(board, current, color), current));
    }

    Bitboard bishopsBB = board->bitboards[color*6 + INDEX_BLACK_BISHOP];
    while(bishopsBB){
        Square current = poplsb(&bishopsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateBishopMoves(board, current, color), current));
    }

    Bitboard rooksBB = board->bitboards[color*6 + INDEX_BLACK_ROOK];
    while(rooksBB){
        Square current = poplsb(&rooksBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateRookMoves(board, current, color), current));
    }

    Bitboard queensBB = board->bitboards[color*6 + INDEX_BLACK_QUEEN];
    while(queensBB){
        Square current = poplsb(&queensBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateQueenMoves(board, current, color), current));
    }

    Bitboard kingsBB = board->bitboards[color*6 + INDEX_BLACK_KING];
    while(kingsBB){
        Square current = poplsb(&kingsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKingMoves(board, current, color), current));
    }

    return moves;
}
