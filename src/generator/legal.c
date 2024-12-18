#include "board.h"
#include "generator.h"
#include "masks.h"

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




Bitboard GenerateLegalPawnMoves(const Board *board, Bitboard pieces, Color color)
{
    Bitboard moves = 0ULL;
    Bitboard king = board->bitboards[(color)?INDEX_WHITE_KING:INDEX_BLACK_KING];
    size_t pawn_index = (color) ? INDEX_WHITE_PAWN : INDEX_BLACK_PAWN;

    Board temp = BoardCopy(board);

    while (pieces) {
        Square current = poplsb(&pieces);
        Bitboard currentPawn = 1ULL << current;
        Bitboard pseudoLegal = GeneratePawnMoves(board, pieces, color);

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
        Bitboard pseudoLegal = GenerateKnightMoves(board, pieces, color);

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
