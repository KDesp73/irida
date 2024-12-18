#include "board.h"
#include "generator.h"
#include <chess/board.h>

Bitboard GeneratePseudoLegalAttacks(const Board* board, Color color)
{
    size_t start = (color) ? 6 : 0;
    Bitboard enemy = GetEnemyColor(board, color);
    Bitboard empty = GetEmpty(board);
    Square enpassantSquare = board->enpassant_square;

    Bitboard attacks =  (color) 
        ? WhitePawnAttacks(board->bitboards[INDEX_WHITE_PAWN], enemy)
        : BlackPawnAttacks(board->bitboards[INDEX_BLACK_PAWN], enemy)
        | KnightAttacks(board->bitboards[start + 1], empty, enemy)
        | BishopAttacks(board->bitboards[start + 2], empty, enemy)
        | RookAttacks(board->bitboards[start + 3], empty, enemy)
        | QueenAttacks(board->bitboards[start + 4], empty, enemy)
        | KingAttacks(board->bitboards[start + 5], empty, enemy)
        ;

    if(color == COLOR_WHITE){
        if((1ULL << enpassantSquare) & RANK_6){
            attacks |= (
                    WhitePawnAttacks(board->bitboards[INDEX_WHITE_PAWN], ~0ULL) 
                    & (1ULL << enpassantSquare)
                    );
        }
    } else {
        if((1ULL << enpassantSquare) & RANK_3){
            attacks |= (
                    BlackPawnAttacks(board->bitboards[INDEX_BLACK_PAWN], ~0ULL) 
                    & (1ULL << enpassantSquare)
                    );
        }
    }


    return attacks;
}


Bitboard GeneratePawnMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard pseudoLegal = 0ULL;
    Square enpassantSquare = board->enpassant_square;
    Bitboard enemySquares = GetEnemy(board);
    Bitboard emptySquares = GetEmpty(board);

    if(color == COLOR_WHITE){
        if((1ULL << enpassantSquare) & RANK_6){
            pseudoLegal = WhitePawnAttacks(pieces, enemySquares) 
                | WhitePawnPushes(pieces, emptySquares)
                | (WhitePawnAttacks(pieces, ~0ULL) & (1ULL << enpassantSquare))
                ;
        } else {
            pseudoLegal = WhitePawnAttacks(pieces, enemySquares) 
                | WhitePawnPushes(pieces, emptySquares)
                ;
        }
    } else {
        if((1ULL << enpassantSquare) & RANK_3){
            pseudoLegal = BlackPawnAttacks(pieces, enemySquares) 
                | BlackPawnPushes(pieces, emptySquares)
                | (BlackPawnAttacks(pieces, ~0ULL) & (1ULL << enpassantSquare))
                ;
        } else {
            pseudoLegal = BlackPawnAttacks(pieces, enemySquares) 
                | BlackPawnPushes(pieces, emptySquares)
                ;
        }
    }

    return pseudoLegal;
}

Bitboard GenerateKnightMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return KnightAttacks(pieces, emptySquares, enemySquares);
}

Bitboard GenerateBishopMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return BishopAttacks(pieces, emptySquares, enemySquares);
}

Bitboard GenerateRookMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return RookAttacks(pieces, emptySquares, enemySquares);
}

Bitboard GenerateQueenMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    return QueenAttacks(pieces, emptySquares, enemySquares);
}

Bitboard GenerateKingMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard pseudoLegal = KingAttacks(pieces, emptySquares, enemySquares);

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
        MovesAppendList(&moves, BitboardToMoves(GeneratePawnMoves(board, 1ULL << current, color), current));
    }

    Bitboard knightsBB = board->bitboards[start + INDEX_BLACK_KNIGHT];
    while(knightsBB){
        Square current = poplsb(&knightsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKnightMoves(board, 1ULL << current, color), current));
    }

    Bitboard bishopsBB = board->bitboards[start + INDEX_BLACK_BISHOP];
    while(bishopsBB){
        Square current = poplsb(&bishopsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateBishopMoves(board, 1ULL << current, color), current));
    }

    Bitboard rooksBB = board->bitboards[start + INDEX_BLACK_ROOK];
    while(rooksBB){
        Square current = poplsb(&rooksBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateRookMoves(board, 1ULL << current, color), current));
    }

    Bitboard queensBB = board->bitboards[start + INDEX_BLACK_QUEEN];
    while(queensBB){
        Square current = poplsb(&queensBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateQueenMoves(board, 1ULL << current, color), current));
    }

    Bitboard kingsBB = board->bitboards[start + INDEX_BLACK_KING];
    while(kingsBB){
        Square current = poplsb(&kingsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateKingMoves(board, 1ULL << current, color), current));
    }

    return moves;
}
