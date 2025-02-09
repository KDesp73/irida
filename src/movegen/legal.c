#include "bitboard.h"
#include "board.h"
#include "extern/bench.h"
#include "movegen.h"
#include "masks.h"
#include "move.h"
#include "result.h"
#include "square.h"
#include <stdio.h>

Moves GenerateLegalPawnMoves(const Board *board, Bitboard pieces, PieceColor color, MoveType type)
{
    if(type == MOVE_CASTLING) return NO_MOVES;

    Moves moves = {0};
    Moves pseudo = {0};
    Moves illegal = {0};
    Moves checkmate = {0};
    Moves promotionMoves = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GeneratePawnMoves(board, current, color, type);
        if(MOVE_PSEUDO)
            pseudo = MovesCombine(pseudo, BitboardToMoves(pseudoLegal, current));

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            // Handle normal moves
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            Move* mp = &move;
            if(!IsPromotion((Board*) board, mp)){
                if(MakeMove(&temp, move)){
                    if (!IsInCheckColor(&temp, !temp.turn)){
                        MovesAppend(&moves, move);
                        
                        if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                            MovesAppend(&checkmate, move);
                    } else {
                        MovesAppend(&illegal, move);
                    }
                    UnmakeMove(&temp);
                } else continue;
            } else {
                // Handle promotions
                if(PawnPromotionMask(current, color) || PawnPromotionAttackMask(current, color)) {
                    Promotion promotions[] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                    for(size_t i = 0; i < 4; i++){
                        Move promotion = MoveEncode(current, target, promotions[i], FLAG_PROMOTION);
                        if(MakeMove(&temp, promotion)){
                            if (!IsInCheckColor(&temp, !temp.turn)) {
                                MovesAppend(&moves, promotion);
                                if(type == MOVE_PSEUDO)
                                    MovesAppend(&pseudo, promotion);
                                else if(type == MOVE_PROMOTION)
                                    MovesAppend(&promotionMoves, promotion);
                                else if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                                    MovesAppend(&checkmate, promotion);
                            } else {
                                if(type == MOVE_ILLEGAL)
                                    MovesAppend(&illegal, promotion);
                                else if(type == MOVE_PSEUDO)
                                    MovesAppend(&pseudo, promotion);
                            }
                            UnmakeMove(&temp);
                        }
                    }
                }
            }

        }
    }

    switch (type) {
    case MOVE_QUIET:
    case MOVE_LEGAL:
    case MOVE_CHECK:
    case MOVE_EN_PASSANT:
    case MOVE_CAPTURES:
        return moves;
    case MOVE_PROMOTION:
        return promotionMoves;
    case MOVE_PSEUDO:
        return pseudo; 
    case MOVE_CHECKMATE:
        return checkmate;
    case MOVE_ILLEGAL:
        return illegal;
    case MOVE_CASTLING:
        return NO_MOVES;
    }

    return moves;
}

Moves GenerateLegalKnightMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type)
{
    if(
        type == MOVE_PROMOTION ||
        type == MOVE_CASTLING ||
        type == MOVE_EN_PASSANT
    ) return NO_MOVES;

    Moves moves = {0};
    Moves pseudo = {0};
    Moves illegal = {0};
    Moves checkmate = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateKnightMoves(board, current, color, type);
        if(MOVE_PSEUDO)
            pseudo = MovesCombine(pseudo, BitboardToMoves(pseudoLegal, current));

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                    if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                        MovesAppend(&checkmate, move);
                } else {
                    if(type == MOVE_ILLEGAL)
                        MovesAppend(&illegal, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    switch (type) {
    case MOVE_LEGAL:
    case MOVE_CAPTURES:
    case MOVE_QUIET:
    case MOVE_CHECK:
        return moves;
    case MOVE_PSEUDO:
        return pseudo;
    case MOVE_CHECKMATE:
        return checkmate;
    case MOVE_ILLEGAL:
        return illegal;
    case MOVE_CASTLING:
    case MOVE_EN_PASSANT:
    case MOVE_PROMOTION:
        return NO_MOVES;
    }

    return moves;
}

Moves GenerateLegalBishopMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type)
{
    if(
        type == MOVE_PROMOTION ||
        type == MOVE_CASTLING ||
        type == MOVE_EN_PASSANT
    ) return NO_MOVES;

    Moves moves = {0};
    Moves pseudo = {0};
    Moves illegal = {0};
    Moves checkmate = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateBishopMoves(board, current, color, type);
        if(MOVE_PSEUDO)
            pseudo = MovesCombine(pseudo, BitboardToMoves(pseudoLegal, current));

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                    if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                        MovesAppend(&checkmate, move);
                } else {
                    if(type == MOVE_ILLEGAL)
                        MovesAppend(&illegal, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    switch (type) {
    case MOVE_LEGAL:
    case MOVE_CAPTURES:
    case MOVE_QUIET:
    case MOVE_CHECK:
        return moves;
    case MOVE_PSEUDO:
        return pseudo;
    case MOVE_CHECKMATE:
        return checkmate;
    case MOVE_ILLEGAL:
        return illegal;
    case MOVE_CASTLING:
    case MOVE_EN_PASSANT:
    case MOVE_PROMOTION:
        return NO_MOVES;
    }


    return moves;
}

Moves GenerateLegalRookMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type)
{
    if(
        type == MOVE_PROMOTION ||
        type == MOVE_CASTLING ||
        type == MOVE_EN_PASSANT
    ) return NO_MOVES;

    Moves moves = {0};
    Moves pseudo = {0};
    Moves illegal = {0};
    Moves checkmate = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateRookMoves(board, current, color, type);
        if(MOVE_PSEUDO)
            pseudo = MovesCombine(pseudo, BitboardToMoves(pseudoLegal, current));

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                    if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                        MovesAppend(&checkmate, move);
                } else {
                    if(type == MOVE_ILLEGAL)
                        MovesAppend(&illegal, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    switch (type) {
    case MOVE_LEGAL:
    case MOVE_CAPTURES:
    case MOVE_QUIET:
    case MOVE_CHECK:
        return moves;
    case MOVE_PSEUDO:
        return pseudo;
    case MOVE_CHECKMATE:
        return checkmate;
    case MOVE_ILLEGAL:
        return illegal;
    case MOVE_CASTLING:
    case MOVE_EN_PASSANT:
    case MOVE_PROMOTION:
        return NO_MOVES;
    }


    return moves;
}

Moves GenerateLegalQueenMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type)
{
    return MovesCombine(
            GenerateLegalBishopMoves(board, pieces, color, type),
            GenerateLegalRookMoves(board, pieces, color, type));
}

Moves GenerateLegalKingMoves(const Board* board, Bitboard pieces, PieceColor color, MoveType type)
{
    if(
        type == MOVE_EN_PASSANT ||
        type == MOVE_PROMOTION
    ) return NO_MOVES;

    Moves moves = {0};
    Moves illegal= {0};
    Moves pseudo = {0};
    Moves check = {0};
    Moves checkmate = {0};
    Board temp = *board;

    Square king = lsb(pieces);
    Bitboard pseudoLegal = GenerateKingMoves(&temp, king, color, type);
    if(MOVE_PSEUDO)
        pseudo = MovesCombine(pseudo, BitboardToMoves(pseudoLegal, king));

    Bitboard opponentAttacks = GeneratePseudoLegalAttacks(&temp, !color);
    pseudoLegal &= ~opponentAttacks;

    // Get the opposing king's position and generate its control zone
    Bitboard opposingKing = temp.bitboards[(!color)*6 + INDEX_BLACK_KING];
    if (opposingKing) {
        Square opposingKingSquare = lsb(opposingKing);
        Bitboard opposingKingControl = KingMoveMask(opposingKingSquare);

        // Exclude squares controlled by the opposing king
        pseudoLegal &= ~opposingKingControl;
    }

    // Process pseudo-legal moves
    while (pseudoLegal) {
        Square target = poplsb(&pseudoLegal);

        Move move = MoveEncode(king, target, PROMOTION_NONE, FLAG_NORMAL);
        if (MakeMove(&temp, move)) {
            bool isInCheck = IsInCheckColor(&temp, !temp.turn);
            if (!isInCheck){
                MovesAppend(&moves, move);
                
                if(type == MOVE_CHECK){
                    Bitboard ourAttacks = GeneratePseudoLegalAttacks(&temp, color);
                    if(IsKingInCheck(temp.bitboards[!color*6 + INDEX_KING], ourAttacks))
                        MovesAppend(&check, move);
                } else if(type == MOVE_CHECKMATE && IsCheckmate(&temp))
                    MovesAppend(&checkmate, move);
            } else {
                    if(type == MOVE_ILLEGAL)
                        MovesAppend(&illegal, move);
            }
            UnmakeMove(&temp);
        }
    }

    switch (type) {
    case MOVE_LEGAL:
    case MOVE_CAPTURES:
    case MOVE_QUIET:
    case MOVE_CHECK:
        return moves;
    case MOVE_PSEUDO:
        return pseudo;
    case MOVE_CHECKMATE:
        return checkmate;
    case MOVE_ILLEGAL:
        return illegal;
    case MOVE_CASTLING:
    case MOVE_EN_PASSANT:
    case MOVE_PROMOTION:
        return NO_MOVES;
    }


    return moves;
}

Bitboard GenerateMovesBitboard(const Board* board, MoveType type)
{
    return MovesToBitboard(GenerateMoves(board, type));
}

bool IsLegal(const Board* board, Move move)
{
    Moves moves = GenerateMoves(board, MOVE_LEGAL);
    for(size_t i = 0; i < moves.count; i++){
        if(MoveCmp(moves.list[i], move)) return true;
    }
    return false;
}

Moves GenerateMoves(const Board* board, MoveType type)
{
#ifndef RELEASE
    BENCH_START();
#endif // RELEASE
    Moves moves = {0};
    PieceColor color = board->turn;

    moves = MovesCombine(
            GenerateLegalPawnMoves(board, board->bitboards[color*6 + INDEX_BLACK_PAWN], color, type),
            MovesCombine(
            GenerateLegalKnightMoves(board, board->bitboards[color*6 + INDEX_BLACK_KNIGHT], color, type),
            MovesCombine(
            GenerateLegalBishopMoves(board, board->bitboards[color*6 + INDEX_BLACK_BISHOP], color, type),
            MovesCombine(
            GenerateLegalRookMoves(board, board->bitboards[color*6 + INDEX_BLACK_ROOK], color, type),
            MovesCombine(
            GenerateLegalQueenMoves(board, board->bitboards[color*6 + INDEX_BLACK_QUEEN], color, type),
            GenerateLegalKingMoves(board, board->bitboards[color*6 + INDEX_BLACK_KING], color, type)
            )))));

#ifndef RELEASE
    BENCH_END();
    BENCH_LOG("GenerateLegalMoves");
#endif // RELEASE
    return moves;
}

Moves GenerateMovesSquare(const Board* board, Square square, MoveType type)
{
    Moves moves = GenerateMoves(board, type);
    Moves result = {0};
    for(size_t i = 0; i < moves.count; i++){
        if(GetFrom(moves.list[i]) == square){
            MovesAppend(&result, moves.list[i]);
        }
    }

    return result;
}
