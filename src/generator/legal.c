#include "board.h"
#include "generator.h"
#include "masks.h"

Bitboard GenerateLegalPawnMoves(const Board *board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard king = board->bitboards[(color)?INDEX_WHITE_KING:INDEX_BLACK_KING];
    size_t pawn_index = (color) ? INDEX_WHITE_PAWN : INDEX_BLACK_PAWN;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GeneratePawnMoves(board, 1ULL << current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[pawn_index];
            DoMove(&tempPieces, move);
            temp.bitboards[pawn_index] = tempPieces;

            // Check if the move leaves the king in check
            if (!IsKingInCheck(king, GeneratePseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[pawn_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateLegalKnightMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard king = board->bitboards[(color) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t knight_index = (color) ? INDEX_WHITE_KNIGHT : INDEX_BLACK_KNIGHT;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateKnightMoves(board, 1ULL << current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[knight_index];
            DoMove(&tempPieces, move);
            temp.bitboards[knight_index] = tempPieces;

            if (!IsKingInCheck(king, GeneratePseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[knight_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateLegalBishopMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard king = board->bitboards[(color == COLOR_WHITE) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t bishop_index = (color == COLOR_WHITE) ? INDEX_WHITE_BISHOP : INDEX_BLACK_BISHOP;

    Board temp = BoardCopy(board);

    // Iterate over each piece of the given color
    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateBishopMoves(board, 1ULL << current, color);

        // Iterate over all possible pseudo-legal moves for the bishop
        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Simulate the move
            Bitboard tempPieces = temp.bitboards[bishop_index];
            DoMove(&tempPieces, move);
            temp.bitboards[bishop_index] = tempPieces;

            if (!IsKingInCheck(king, GeneratePseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }

            // Undo the move
            UndoMove(&tempPieces, move);
            temp.bitboards[bishop_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateLegalRookMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard king = board->bitboards[(color) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t rook_index = (color) ? INDEX_WHITE_ROOK : INDEX_BLACK_ROOK;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateRookMoves(board, 1ULL << current, color);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[rook_index];
            DoMove(&tempPieces, move);
            temp.bitboards[rook_index] = tempPieces;

            if (!IsKingInCheck(king, GeneratePseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[rook_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateLegalQueenMoves(const Board* board, Bitboard pieces, Color color)
{
    return GenerateLegalBishopMoves(board, pieces, color) | GenerateLegalRookMoves(board, pieces, color); 
}

Bitboard GenerateLegalKingMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    size_t king_index = (color == COLOR_WHITE) ? INDEX_WHITE_KING : INDEX_BLACK_KING;

    Bitboard kingBitboard = board->bitboards[king_index];
    Board temp = BoardCopy(board);

    // Iterate over each king (should be a single piece)
    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = GenerateKingMoves(board, 1ULL << current, color);

        // Verify each pseudo-legal move
        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Simulate the move
            Bitboard tempPieces = temp.bitboards[king_index];
            DoMove(&tempPieces, move);
            temp.bitboards[king_index] = tempPieces;

            if (!IsKingInCheck(temp.bitboards[king_index], GeneratePseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }

            // Undo the move
            UndoMove(&tempPieces, move);
            temp.bitboards[king_index] = tempPieces;
        }
    }

    return moves;
}

Moves GenerateLegalMoves(const Board* board)
{
    Moves moves = {0};
    size_t start = board->state.turn ? 6 : 0;
    Color color = board->state.turn;

    Bitboard pawnsBB = board->bitboards[start + INDEX_BLACK_PAWN];
    while(pawnsBB){
        Square current = poplsb(&pawnsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalPawnMoves(board, 1ULL << current, color), current));
    }

    Bitboard knightsBB = board->bitboards[start + INDEX_BLACK_KNIGHT];
    while(knightsBB){
        Square current = poplsb(&knightsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalKnightMoves(board, 1ULL << current, color), current));
    }

    Bitboard bishopsBB = board->bitboards[start + INDEX_BLACK_BISHOP];
    while(bishopsBB){
        Square current = poplsb(&bishopsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalBishopMoves(board, 1ULL << current, color), current));
    }

    Bitboard rooksBB = board->bitboards[start + INDEX_BLACK_ROOK];
    while(rooksBB){
        Square current = poplsb(&rooksBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalRookMoves(board, 1ULL << current, color), current));
    }

    Bitboard queensBB = board->bitboards[start + INDEX_BLACK_QUEEN];
    while(queensBB){
        Square current = poplsb(&queensBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalQueenMoves(board, 1ULL << current, color), current));
    }

    Bitboard kingsBB = board->bitboards[start + INDEX_BLACK_KING];
    while(kingsBB){
        Square current = poplsb(&kingsBB);
        MovesAppendList(&moves, BitboardToMoves(GenerateLegalKingMoves(board, 1ULL << current, color), current));
    }

    return moves;
}
