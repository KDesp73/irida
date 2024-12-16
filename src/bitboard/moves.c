#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"
#include "square.h"
#include <chess/ui.h>
#include <io/logging.h>

Bitboard DoMove(Bitboard* current, Move move)
{
    Square source, destination;
    uint8_t promotion, flag;
    MoveDecode(move, &source, &destination, &promotion, &flag);

    // Update the bitboard
    // Clear the source bit
    *current &= ~(1ULL << source);

    // Set the destination bit
    if(promotion == PROMOTION_NONE)
        *current |= (1ULL << destination);
    else {
        *current &= ~(1ULL << destination);
        // Returning the promotion square so it can be or'd with the appropriate bitboard 
        return 1ULL << destination; 
    }

    return 0;
}

Bitboard UndoMove(Bitboard* current, Move move)
{
    Square source, destination;
    uint8_t promotion, flag;
    MoveDecode(move, &source, &destination, &promotion, &flag);

    // Revert the move on the bitboard
    // Clear the destination bit
    *current &= ~(1ULL << destination);

    // Restore the source bit
    *current |= (1ULL << source);

    // Handle promotions
    if (promotion != PROMOTION_NONE) {
        *current &= ~(1ULL << source);
        return 1ULL << source;
    }

    return 0;
}

Bitboard GenerateWhitePawnMovesPieces(const Board *board, Bitboard pieces)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, COLOR_WHITE);
    Square enpassantSquare = board->enpassant_square;
    Bitboard king = board->bitboards[INDEX_WHITE_KING];

    while (pieces) {
        Square current = lsb(pieces);
        Bitboard currentPawn = 1ULL << current;
        Bitboard pseudoLegal = 0ULL;

        if((1ULL << enpassantSquare) & RANK_3){
            pseudoLegal = WhitePawnAttacks(currentPawn, enemySquares) 
                | WhitePawnPushes(currentPawn, emptySquares)
                | (WhitePawnAttacks(currentPawn, ~0ULL) & (1ULL << enpassantSquare));
        } else {
            pseudoLegal = WhitePawnAttacks(currentPawn, enemySquares) 
                | WhitePawnPushes(currentPawn, emptySquares);
        }

        while (pseudoLegal) {
            Square target = lsb(pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Create a temporary board to check the legality of the move
            Board temp = BoardCopy(board);
            Bitboard tempPieces = temp.bitboards[INDEX_WHITE_PAWN]; // Store the original pawn positions
            DoMove(&tempPieces, move); // Apply the move on the temporary board
            temp.bitboards[INDEX_WHITE_PAWN] = tempPieces; // Update the board with the modified state

            // Check if the move leaves the king in check
            if (!IsKingInCheck(temp.bitboards[INDEX_WHITE_KING], GetPseudoValidAttacks(&temp, COLOR_BLACK))) {
                moves |= (1ULL << target);
            }
            UndoMove(&tempPieces, move); // Undo the move on the temporary board
            temp.bitboards[INDEX_WHITE_PAWN] = tempPieces; // Restore the original pawn state

            // Remove the target from pseudoLegal moves
            off(&pseudoLegal, target);
        }
        off(&pieces, current); // Move to the next pawn
    }

    return moves;
}

Bitboard GenerateWhitePawnMoves(const Board* board)
{
    Bitboard pieces = board->bitboards[INDEX_WHITE_PAWN];
    return GenerateWhitePawnMovesPieces(board, pieces);
}

Bitboard GenerateBlackPawnMovesPieces(const Board* board, Bitboard pieces)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, COLOR_BLACK);
    Square enpassantSquare = board->enpassant_square;
    Bitboard king = board->bitboards[INDEX_BLACK_KING];

    while (pieces) {
        Square current = lsb(pieces);
        Bitboard currentPawn = 1ULL << current;
        Bitboard pseudoLegal;

        if((1ULL << enpassantSquare) & RANK_3){
            pseudoLegal = BlackPawnAttacks(currentPawn, enemySquares) 
                | BlackPawnPushes(currentPawn, emptySquares)
                | (BlackPawnAttacks(currentPawn, ~0ULL) & (1ULL << enpassantSquare));
        } else {
            pseudoLegal = BlackPawnAttacks(currentPawn, enemySquares) 
                | BlackPawnPushes(currentPawn, emptySquares);
        }

        while (pseudoLegal) {
            Square target = lsb(pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Create a temporary board to check the legality of the move
            Board temp = BoardCopy(board);
            Bitboard tempPieces = temp.bitboards[INDEX_BLACK_PAWN]; // Store the original pawn positions
            DoMove(&tempPieces, move); // Apply the move on the temporary board
            temp.bitboards[INDEX_BLACK_PAWN] = tempPieces; // Update the board with the modified state

            // Check if the move leaves the king in check
            if (!IsKingInCheck(temp.bitboards[INDEX_BLACK_KING], GetPseudoValidAttacks(&temp, COLOR_WHITE))) {
                moves |= (1ULL << target);
            }
            UndoMove(&tempPieces, move); // Undo the move on the temporary board
            temp.bitboards[INDEX_BLACK_PAWN] = tempPieces; // Restore the original pawn state

            // Remove the target from pseudoLegal moves
            off(&pseudoLegal, target);
        }
        off(&pieces, current); // Move to the next pawn
    }

    return moves;
}
Bitboard GenerateBlackPawnMoves(const Board* board)
{
    Bitboard pieces = board->bitboards[INDEX_BLACK_PAWN];
    return GenerateBlackPawnMovesPieces(board, pieces);
}

Bitboard GenerateKnightMoves(const Board* board);
Bitboard GenerateBishopMoves(const Board* board);
Bitboard GenerateRookMoves(const Board* board);
Bitboard GenerateQueenMoves(const Board* board);
Bitboard GenerateKingMoves(const Board* board);

bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks)
{
    return kingPosition & enemyAttacks;
}
