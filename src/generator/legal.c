#include "bitboard.h"
#include "board.h"
#include "generator.h"
#include "masks.h"
#include "move.h"
#include "square.h"
#include <io/logging.h>
#include <stdio.h>

Moves GenerateLegalPawnMoves(const Board *board, Bitboard pieces, Color color)
{
    Moves moves = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GeneratePawnMoves(board, current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            // Handle normal moves
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                }
                UnmakeMove(&temp);
            } else continue;

            // Handle promotions
            if(PawnPromotionMask(current, color) || PawnPromotionAttackMask(current, color)) {
                Promotion promotions[] = {PROMOTION_QUEEN, PROMOTION_ROOK, PROMOTION_BISHOP, PROMOTION_KNIGHT};
                for(size_t i = 0; i < 4; i++){
                    Move promotion = MoveEncode(current, target, promotions[i], FLAG_PROMOTION);
                    MakeMove(&temp, promotion);
                    if (!IsInCheckColor(&temp, !temp.turn)) {
                        MovesAppend(&moves, promotion);
                    }
                    UnmakeMove(&temp);
                }
            }
        }
    }

    return moves;
}

Moves GenerateLegalKnightMoves(const Board* board, Bitboard pieces, Color color)
{
    Moves moves = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateKnightMoves(board, current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    return moves;
}

Moves GenerateLegalBishopMoves(const Board* board, Bitboard pieces, Color color)
{
    Moves moves = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateBishopMoves(board, current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    return moves;
}

Moves GenerateLegalRookMoves(const Board* board, Bitboard pieces, Color color)
{
    Moves moves = {0};
    Board temp = *board;

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateRookMoves(board, current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);

            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);
            if(MakeMove(&temp, move)){
                if (!IsInCheckColor(&temp, !temp.turn)){
                    MovesAppend(&moves, move);
                }
                UnmakeMove(&temp);
            }
        }
    }

    return moves;
}

Moves GenerateLegalQueenMoves(const Board* board, Bitboard pieces, Color color)
{
    return MovesCombine(
            GenerateLegalBishopMoves(board, pieces, color),
            GenerateLegalRookMoves(board, pieces, color));
}

Moves GenerateLegalKingMoves(const Board* board, Bitboard pieces, Color color)
{
    Moves moves = {0};
    Board temp = *board;

    Square king = lsb(pieces);
    Bitboard pseudoLegal = GenerateKingMoves(board, king, color);
    size_t move_count = popcount(pseudoLegal);

    // Process pseudo-legal moves
    while (pseudoLegal && move_count > 0) {
        Square target = poplsb(&pseudoLegal);

        Move move = MoveEncode(king, target, PROMOTION_NONE, FLAG_NORMAL);
        if(MakeMove(&temp, move)){
            if (!IsInCheckColor(&temp, !temp.turn)) {
                MovesAppend(&moves, move);
            }
            UnmakeMove(&temp);
        }
        move_count--;
    }

    return moves;
}

Bitboard GenerateLegalMovesBitboard(const Board* board)
{
    return MovesToBitboard(GenerateLegalMoves(board));
}

bool IsLegal(const Board* board, Move move)
{
    Moves moves = GenerateLegalMoves(board);
    for(size_t i = 0; i < moves.count; i++){
        if(MoveCmp(moves.list[i], move)) return true;
    }
    return false;
}

Moves GenerateLegalMoves(const Board* board)
{
    Moves moves = {0};
    Color color = board->turn;

    moves = MovesCombine(
                GenerateLegalPawnMoves(board, board->bitboards[color*6 + INDEX_BLACK_PAWN], color),
                MovesCombine(
                    GenerateLegalKnightMoves(board, board->bitboards[color*6 + INDEX_BLACK_KNIGHT], color),
                    MovesCombine(
                        GenerateLegalBishopMoves(board, board->bitboards[color*6 + INDEX_BLACK_BISHOP], color),
                        MovesCombine(
                            GenerateLegalRookMoves(board, board->bitboards[color*6 + INDEX_BLACK_ROOK], color),
                            MovesCombine(
                                GenerateLegalQueenMoves(board, board->bitboards[color*6 + INDEX_BLACK_QUEEN], color),
                                GenerateLegalKingMoves(board, board->bitboards[color*6 + INDEX_BLACK_KING], color)
                            )
                        )
                    )
                )
            );

    return moves;
}
