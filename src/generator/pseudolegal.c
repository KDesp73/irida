#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "generator.h"
#include "square.h"
#include <stdio.h>

Bitboard GeneratePseudoLegalPawnAttacks(Bitboard pawns, Bitboard enemy, Color color)
{
    Bitboard result = 0ULL;
    while(pawns){
        Square current = poplsb(&pawns);
        result |= PawnAttacks(current, enemy, color);
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


Bitboard GeneratePseudoLegalAttacks(const Board* board, Color attackerColor)
{
    size_t start = (attackerColor) ? 6 : 0;
    Bitboard enemy = GetEnemyColor(board, attackerColor);
    Bitboard empty = GetEmpty(board);
    Square enpassantSquare = board->enpassant_square;

    Bitboard attacks = 
        GeneratePseudoLegalPawnAttacks(board->bitboards[start + INDEX_BLACK_PAWN], enemy, attackerColor)
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


Bitboard GeneratePawnMoves(const Board* board, Square piece, Color color)
{
    Bitboard pseudoLegal = 0ULL;
    Square enpassantSquare = board->enpassant_square;
    Bitboard enemySquares = GetEnemy(board);
    Bitboard emptySquares = GetEmpty(board);

    if(color == COLOR_WHITE){
        if((1ULL << enpassantSquare) & RANK_6){
            pseudoLegal = PawnAttacks(piece, enemySquares, color) 
                | PawnPushes(piece, emptySquares, color)
                | (PawnAttacks(piece, ~0ULL, color) & (1ULL << enpassantSquare))
                ;
        } else {
            pseudoLegal = PawnAttacks(piece, enemySquares, color) 
                | PawnPushes(piece, emptySquares, color)
                ;
        }
    } else {
        if((1ULL << enpassantSquare) & RANK_3){
            pseudoLegal = PawnAttacks(piece, enemySquares, color) 
                | PawnPushes(piece, emptySquares, color)
                | (PawnAttacks(piece, ~0ULL, color) & (1ULL << enpassantSquare))
                ;
        } else {
            pseudoLegal = PawnAttacks(piece, enemySquares, color) 
                | PawnPushes(piece, emptySquares, color)
                ;
        }
    }

    return pseudoLegal;
}

Bitboard GenerateKnightMoves(const Board* board, Square piece, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return KnightAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateBishopMoves(const Board* board, Square piece, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return BishopAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateRookMoves(const Board* board, Square piece, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return RookAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateQueenMoves(const Board* board, Square piece, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return QueenAttacks(piece, emptySquares, enemySquares);
}

Bitboard GenerateKingMoves(const Board* board, Square piece, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard pseudoLegal = KingAttacks(piece, emptySquares, enemySquares);

    // Castling Logic
    if (!IsInCheck(board)) {
        if (color == COLOR_WHITE) {
            // White Kingside Castling: Squares e1 -> f1 -> g1 (4 -> 5 -> 6)
            if (HasCastlingRights(board, CASTLE_WHITE_KINGSIDE)) {
                if (IsSquareEmpty(board, 5) && IsSquareEmpty(board, 6)) {
                    if (!IsSquareAttacked(board, 5, COLOR_BLACK) && !IsSquareAttacked(board, 6, COLOR_BLACK)) {
                        on(&pseudoLegal, 6); // Add kingside castling move
                    }
                }
            }
            // White Queenside Castling: Squares e1 -> d1 -> c1 (4 -> 3 -> 2)
            if (HasCastlingRights(board, CASTLE_WHITE_QUEENSIDE)) {
                if (IsSquareEmpty(board, 3) && IsSquareEmpty(board, 2) && IsSquareEmpty(board, 1)) {
                    if (!IsSquareAttacked(board, 3, COLOR_BLACK) && !IsSquareAttacked(board, 2, COLOR_BLACK)) {
                        on(&pseudoLegal, 2); // Add queenside castling move
                    }
                }
            }
        } else {
            // Black Kingside Castling: Squares e8 -> f8 -> g8 (60 -> 61 -> 62)
            if (HasCastlingRights(board, CASTLE_BLACK_KINGSIDE)) {
                if (IsSquareEmpty(board, 61) && IsSquareEmpty(board, 62)) {
                    if (!IsSquareAttacked(board, 61, COLOR_WHITE) && !IsSquareAttacked(board, 62, COLOR_WHITE)) {
                        on(&pseudoLegal, 62); // Add kingside castling move
                    }
                }
            }
            // Black Queenside Castling: Squares e8 -> d8 -> c8 (60 -> 59 -> 58)
            if (HasCastlingRights(board, CASTLE_BLACK_QUEENSIDE)) {
                if (IsSquareEmpty(board, 59) && IsSquareEmpty(board, 58) && IsSquareEmpty(board, 57)) {
                    if (!IsSquareAttacked(board, 59, COLOR_WHITE) && !IsSquareAttacked(board, 58, COLOR_WHITE)) {
                        on(&pseudoLegal, 58); // Add queenside castling move
                    }
                }
            }
        }
    }

    return pseudoLegal;
}

Moves GeneratePseudoLegalMoves(const Board* board)
{
    Moves moves = {0};
    size_t start = board->turn ? 6 : 0;
    Color color = board->turn;

    Bitboard pawnsBB = board->bitboards[start + INDEX_BLACK_PAWN];
    while(pawnsBB){
        Square current = poplsb(&pawnsBB);
        MovesAppendList(&moves, BitboardToMoves(GeneratePawnMoves(board, current, color), current));
    }

    Bitboard knightsBB = board->bitboards[start + INDEX_BLACK_KNIGHT];
    while(knightsBB){
        Square current = poplsb(&knightsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKnightMoves(board, current, color), current));
    }

    Bitboard bishopsBB = board->bitboards[start + INDEX_BLACK_BISHOP];
    while(bishopsBB){
        Square current = poplsb(&bishopsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateBishopMoves(board, current, color), current));
    }

    Bitboard rooksBB = board->bitboards[start + INDEX_BLACK_ROOK];
    while(rooksBB){
        Square current = poplsb(&rooksBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateRookMoves(board, current, color), current));
    }

    Bitboard queensBB = board->bitboards[start + INDEX_BLACK_QUEEN];
    while(queensBB){
        Square current = poplsb(&queensBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateQueenMoves(board, current, color), current));
    }

    Bitboard kingsBB = board->bitboards[start + INDEX_BLACK_KING];
    while(kingsBB){
        Square current = poplsb(&kingsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKingMoves(board, current, color), current));
    }

    return moves;
}
