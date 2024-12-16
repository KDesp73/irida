#include "bitboard.h"
#include "board.h"
#include "masks.h"
#include "move.h"
#include "square.h"
#include <chess/board.h>
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


// Helper function to check if two squares are on the same diagonal
bool isOnSameDiagonal(Square pieceSquare, Square kingSquare)
{
    int dx = abs((pieceSquare % 8) - (kingSquare % 8)); // Column difference
    int dy = abs((pieceSquare / 8) - (kingSquare / 8)); // Row difference

    return dx == dy; // True if they are on the same diagonal
}

// Helper function to check if two squares are in the same column
bool isOnSameColumn(Square pieceSquare, Square kingSquare)
{
    return (pieceSquare % 8) == (kingSquare % 8); // Same column
}

// Helper function to check if two squares are in the same row
bool isOnSameRow(Square pieceSquare, Square kingSquare)
{
    return (pieceSquare / 8) == (kingSquare / 8); // Same row
}

// Helper function to check if a square is occupied by any piece
bool isOccupied(const Board* board, Square square)
{
    return board->bitboards[INDEX_WHITE_PAWN] & (1ULL << square) ||
           board->bitboards[INDEX_BLACK_PAWN] & (1ULL << square) ||
           board->bitboards[INDEX_WHITE_KING] & (1ULL << square) ||
           board->bitboards[INDEX_BLACK_KING] & (1ULL << square) ||
           board->bitboards[INDEX_WHITE_BISHOP] & (1ULL << square) ||
           board->bitboards[INDEX_BLACK_BISHOP] & (1ULL << square) ||
           board->bitboards[INDEX_WHITE_ROOK] & (1ULL << square) ||
           board->bitboards[INDEX_BLACK_ROOK] & (1ULL << square) ||
           board->bitboards[INDEX_WHITE_QUEEN] & (1ULL << square) ||
           board->bitboards[INDEX_BLACK_QUEEN] & (1ULL << square);
}

// Helper function to check if the path between the piece and the king is clear (no pieces blocking)
bool isPathClear(const Board* board, Square pieceSquare, Square kingSquare, Color color, RayType rayType)
{
    int direction = (kingSquare > pieceSquare) ? 1 : -1; // Direction to move towards the king

    // Check based on the ray type (diagonal, vertical, or horizontal)
    if (rayType == DIAGONAL) {
        // Move along the diagonal
        int currentSquare = pieceSquare + direction * 9; // Diagonal direction (top-left to bottom-right)
        while (currentSquare != kingSquare) {
            if (isOccupied(board, currentSquare)) {
                return false; // Path is blocked by another piece
            }
            currentSquare += direction * 9; // Continue moving along the diagonal
        }
    } else if (rayType == VERTICAL) {
        // Move along the vertical line (same column)
        int currentSquare = pieceSquare + direction * 8; // Move up or down the column
        while (currentSquare != kingSquare) {
            if (isOccupied(board, currentSquare)) {
                return false; // Path is blocked by another piece
            }
            currentSquare += direction * 8;
        }
    } else if (rayType == HORIZONTAL) {
        // Move along the horizontal line (same row)
        int currentSquare = pieceSquare + direction; // Move left or right along the row
        while (currentSquare != kingSquare) {
            if (isOccupied(board, currentSquare)) {
                return false; // Path is blocked by another piece
            }
            currentSquare += direction;
        }
    }

    return true; // Path is clear
}

bool IsPiecePinned(const Board* board, Square pieceSquare, Color color)
{
    // Get the king's position
    Square kingSquare = lsb(board->bitboards[(color == COLOR_WHITE) ? INDEX_WHITE_KING : INDEX_BLACK_KING]);

    // Check if the piece is in line with the king (diagonal, horizontal, or vertical)
    
    // Check for diagonal alignment
    if (isOnSameDiagonal(pieceSquare, kingSquare)) {
        // Check if there are no pieces blocking the diagonal
        if (isPathClear(board, pieceSquare, kingSquare, color, DIAGONAL)) {
            return true; // Piece is pinned along the diagonal
        }
    }

    // Check for vertical alignment (same column)
    if (isOnSameColumn(pieceSquare, kingSquare)) {
        // Check if there are no pieces blocking the vertical path
        if (isPathClear(board, pieceSquare, kingSquare, color, VERTICAL)) {
            return true; // Piece is pinned along the vertical
        }
    }

    // Check for horizontal alignment (same row)
    if (isOnSameRow(pieceSquare, kingSquare)) {
        // Check if there are no pieces blocking the horizontal path
        if (isPathClear(board, pieceSquare, kingSquare, color, HORIZONTAL)) {
            return true; // Piece is pinned along the horizontal
        }
    }

    return false; // Piece is not pinned
}

Bitboard GetPinPath(const Board* board, Square pieceSquare, Square kingSquare, RayType rayType)
{
    Bitboard path = 0ULL;  // Initialize path with 0

    // Determine the direction of movement based on the ray type
    int direction;
    if (rayType == DIAGONAL) {
        // Ensure both squares are on the same diagonal
        if (abs((pieceSquare % 8) - (kingSquare % 8)) != abs((pieceSquare / 8) - (kingSquare / 8))) {
            return 0ULL; // Not on the same diagonal
        }
        direction = ((kingSquare > pieceSquare) ? 1 : -1) * ((kingSquare % 8 > pieceSquare % 8) ? 9 : 7);
    } else if (rayType == VERTICAL) {
        // Ensure both squares are in the same column
        if ((pieceSquare % 8) != (kingSquare % 8)) {
            return 0ULL; // Not on the same column
        }
        direction = (kingSquare > pieceSquare) ? 8 : -8;
    } else if (rayType == HORIZONTAL) {
        // Ensure both squares are in the same row
        if ((pieceSquare / 8) != (kingSquare / 8)) {
            return 0ULL; // Not on the same row
        }
        direction = (kingSquare > pieceSquare) ? 1 : -1;
    } else {
        return 0ULL; // Invalid ray type
    }

    int currentSquare = pieceSquare + direction;

    // Trace the path until we reach the king square
    while (currentSquare != kingSquare) {
        // Ensure the current square is still on the board
        if (currentSquare < 0 || currentSquare >= 64) {
            return 0ULL; // Invalid path
        }

        // Add the current square to the path
        path |= (1ULL << currentSquare);

        // Move to the next square
        currentSquare += direction;
    }

    // Add the king's square to the path
    path |= (1ULL << kingSquare);

    return path;
}


Bitboard GeneratePawnMoves(const Board *board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Square enpassantSquare = board->enpassant_square;
    Bitboard king = board->bitboards[(color)?INDEX_WHITE_KING:INDEX_BLACK_KING];
    size_t pawn_index = (color) ? INDEX_WHITE_PAWN : INDEX_BLACK_PAWN;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard currentPawn = 1ULL << current;
        Bitboard pseudoLegal = 0ULL;

        if(color == COLOR_WHITE){
            if((1ULL << enpassantSquare) & RANK_6){
                pseudoLegal = WhitePawnAttacks(currentPawn, enemySquares) 
                    | WhitePawnPushes(currentPawn, emptySquares)
                    | (WhitePawnAttacks(currentPawn, ~0ULL) & (1ULL << enpassantSquare))
                    ;
            } else {
                pseudoLegal = WhitePawnAttacks(currentPawn, enemySquares) 
                    | WhitePawnPushes(currentPawn, emptySquares)
                    ;
            }
        } else {
            if((1ULL << enpassantSquare) & RANK_3){
                pseudoLegal = BlackPawnAttacks(currentPawn, enemySquares) 
                    | BlackPawnPushes(currentPawn, emptySquares)
                    | (BlackPawnAttacks(currentPawn, ~0ULL) & (1ULL << enpassantSquare))
                    ;
            } else {
                pseudoLegal = BlackPawnAttacks(currentPawn, enemySquares) 
                    | BlackPawnPushes(currentPawn, emptySquares)
                    ;
            }
        }

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[pawn_index];
            DoMove(&tempPieces, move);
            temp.bitboards[pawn_index] = tempPieces;

            // Check if the move leaves the king in check
            if (!IsKingInCheck(king, GetPseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[pawn_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateKnightMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard king = board->bitboards[(color) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t knight_index = (color) ? INDEX_WHITE_KNIGHT : INDEX_BLACK_KNIGHT;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = KnightAttacks(pieces, emptySquares, enemySquares);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[knight_index];
            DoMove(&tempPieces, move);
            temp.bitboards[knight_index] = tempPieces;

            if (!IsKingInCheck(king, GetPseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[knight_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateBishopMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard king = board->bitboards[(color == COLOR_WHITE) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t bishop_index = (color == COLOR_WHITE) ? INDEX_WHITE_BISHOP : INDEX_BLACK_BISHOP;

    Board temp = BoardCopy(board);

    // Iterate over each piece of the given color
    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard currentBishop = 1ULL << current; // Current bishop's bitboard
        Bitboard pseudoLegal = BishopAttacks(pieces, emptySquares, enemySquares);

        // Iterate over all possible pseudo-legal moves for the bishop
        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Simulate the move
            Bitboard tempPieces = temp.bitboards[bishop_index];
            DoMove(&tempPieces, move);
            temp.bitboards[bishop_index] = tempPieces;

            if (!IsKingInCheck(king, GetPseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }

            // Undo the move
            UndoMove(&tempPieces, move);
            temp.bitboards[bishop_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateRookMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    Bitboard king = board->bitboards[(color) ? INDEX_WHITE_KING : INDEX_BLACK_KING];
    size_t rook_index = (color) ? INDEX_WHITE_ROOK : INDEX_BLACK_ROOK;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = RookAttacks(1ULL << current, emptySquares, enemySquares);

        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            Bitboard tempPieces = temp.bitboards[rook_index];
            DoMove(&tempPieces, move);
            temp.bitboards[rook_index] = tempPieces;

            if (!IsKingInCheck(king, GetPseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }
            UndoMove(&tempPieces, move);
            temp.bitboards[rook_index] = tempPieces;
        }
    }

    return moves;
}

Bitboard GenerateQueenMoves(const Board* board, Bitboard pieces, Color color)
{
    return GenerateBishopMoves(board, pieces, color) | GenerateRookMoves(board, pieces, color); 
}

Bitboard GenerateKingMoves(const Board* board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard emptySquares = GetEmpty(board);
    Bitboard enemySquares = GetEnemyColor(board, color);
    size_t king_index = (color == COLOR_WHITE) ? INDEX_WHITE_KING : INDEX_BLACK_KING;

    Bitboard kingBitboard = board->bitboards[king_index];
    Board temp = BoardCopy(board);

    // Iterate over each king (should be a single piece)
    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard pseudoLegal = KingAttacks(1ULL << current, emptySquares, enemySquares);

        // Castling Logic
        if (!IsKingInCheck(kingBitboard, GetPseudoLegalAttacks(board, !color))) {
            if (color == COLOR_WHITE) {
                // White Kingside Castling: Squares e1 -> f1 -> g1 (4 -> 5 -> 6)
                if (has_castling_rights(board->state, CASTLE_WHITE_KINGSIDE)) {
                    if (IsSquareEmpty(board, 5) && IsSquareEmpty(board, 6)) {
                        if (!IsSquareAttacked(board, 5, COLOR_BLACK) && !IsSquareAttacked(board, 6, COLOR_BLACK)) {
                            on(&pseudoLegal, 6); // Add kingside castling move
                        }
                    }
                }
                // White Queenside Castling: Squares e1 -> d1 -> c1 (4 -> 3 -> 2)
                if (has_castling_rights(board->state, CASTLE_WHITE_QUEENSIDE)) {
                    if (IsSquareEmpty(board, 3) && IsSquareEmpty(board, 2) && IsSquareEmpty(board, 1)) {
                        if (!IsSquareAttacked(board, 3, COLOR_BLACK) && !IsSquareAttacked(board, 2, COLOR_BLACK)) {
                            on(&pseudoLegal, 2); // Add queenside castling move
                        }
                    }
                }
            } else {
                // Black Kingside Castling: Squares e8 -> f8 -> g8 (60 -> 61 -> 62)
                if (has_castling_rights(board->state, CASTLE_BLACK_KINGSIDE)) {
                    if (IsSquareEmpty(board, 61) && IsSquareEmpty(board, 62)) {
                        if (!IsSquareAttacked(board, 61, COLOR_WHITE) && !IsSquareAttacked(board, 62, COLOR_WHITE)) {
                            on(&pseudoLegal, 62); // Add kingside castling move
                        }
                    }
                }
                // Black Queenside Castling: Squares e8 -> d8 -> c8 (60 -> 59 -> 58)
                if (has_castling_rights(board->state, CASTLE_BLACK_QUEENSIDE)) {
                    if (IsSquareEmpty(board, 59) && IsSquareEmpty(board, 58) && IsSquareEmpty(board, 57)) {
                        if (!IsSquareAttacked(board, 59, COLOR_WHITE) && !IsSquareAttacked(board, 58, COLOR_WHITE)) {
                            on(&pseudoLegal, 58); // Add queenside castling move
                        }
                    }
                }
            }
        }

        // Verify each pseudo-legal move
        while (pseudoLegal) {
            Square target = poplsb(&pseudoLegal);
            Move move = MoveEncode(current, target, PROMOTION_NONE, FLAG_NORMAL);

            // Simulate the move
            Bitboard tempPieces = temp.bitboards[king_index];
            DoMove(&tempPieces, move);
            temp.bitboards[king_index] = tempPieces;

            if (!IsKingInCheck(temp.bitboards[king_index], GetPseudoLegalAttacks(&temp, !color))) {
                on(&moves, target);
            }

            // Undo the move
            UndoMove(&tempPieces, move);
            temp.bitboards[king_index] = tempPieces;
        }
    }

    return moves;
}

bool IsKingInCheck(Bitboard kingPosition, Bitboard enemyAttacks)
{
    return kingPosition & enemyAttacks;
}

bool IsSquareAttacked(const Board* board, Square square, Color attackerColor)
{
    Bitboard attacks = GetPseudoLegalAttacks(board, attackerColor);
    return attacks & (1ULL << square);
}
