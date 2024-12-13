#include "board.h"
#include <chess/piece.h>

_Bool IsCheckmateColor(Board* board, uint8_t color)
{
    // uint64_t kingBitboard = board->bitboards[color == PIECE_COLOR_BLACK ? INDEX_BLACK_KING : INDEX_WHITE_KING];
    // 
    // if (kingBitboard == 0) {
    //     return 0;
    // }
    //
    // int kingSquare = __builtin_ctzll(kingBitboard);
    //
    // // 1. Check if the King is in check (Check if any opposing pieces can attack the king)
    // if (IsKingInCheck(*board, color)) {
    //     // 2. Check if the king has any legal moves to escape the check
    //     if (HasEscapeMoves(board, color, kingSquare)) {
    //         return 0; // Not checkmate, the king has escape moves
    //     }
    //
    //     // 3. Check if any piece can block or capture the attacking piece
    //     if (CanBlockOrCaptureAttacker(board, color, kingSquare)) {
    //         return 0;
    //     }
    //
    //     return 1;
    // }

    return 0;
}

_Bool IsCheckmate(Board* board)
{
    return IsCheckmateColor(board, PIECE_COLOR_WHITE)
        || IsCheckmateColor(board, PIECE_COLOR_BLACK);
}

_Bool IsStalemate(Board* board)
{

}

_Bool IsInsufficientMaterial(Board* board)
{
    int total_white = 0;
    int total_black = 0;
    int white_knights = 0, black_knights = 0;
    int white_bishops = 0, black_bishops = 0;

    for (int i = 0; i < PIECE_TYPES; i++) {
        if (board->bitboards[i]) {
            // Count the number of pieces for each type
            int count = __builtin_popcountll(board->bitboards[i]);

            if (i == INDEX_WHITE_KNIGHT) white_knights += count;
            else if (i == INDEX_BLACK_KNIGHT) black_knights += count;
            else if (i == INDEX_WHITE_BISHOP) white_bishops += count;
            else if (i == INDEX_BLACK_BISHOP) black_bishops += count;

            // Total piece count for each color
            if (i > PIECE_TYPES / 2) total_white += count;
            else total_black += count;
        }
    }

    // Insufficient material cases:
    // 1. Two lone kings
    if (total_white == 1 && total_black == 1) {
        return 1;
    }

    // 2. A lone king vs king and bishop
    if ((total_white == 1 && total_black == 2 && black_bishops == 1) ||
        (total_black == 1 && total_white == 2 && white_bishops == 1)) {
        return 1;
    }

    // 3. A lone king vs king and knight
    if ((total_white == 1 && total_black == 2 && black_knights == 1) ||
        (total_black == 1 && total_white == 2 && white_knights == 1)) {
        return 1;
    }

    // 4. King and knight vs king and bishop
    if (total_white == 2 && total_black == 2 &&
        white_knights == 1 && black_bishops == 1) {
        return 1;
    }

    if (total_black == 2 && total_white == 2 &&
        black_knights == 1 && white_bishops == 1) {
        return 1;
    }

    // 5. King and knight vs king and knight
    if (total_white == 2 && total_black == 2 &&
        white_knights == 1 && black_knights == 1) {
        return 1;
    }

    if (total_white == 2 && total_black == 2 &&
        white_bishops == 1 && black_bishops == 1) {
        return 1;
    }

    // 7. A lone king vs king and 2 knights
    if ((total_white == 1 && total_black == 3 && black_knights == 2) ||
        (total_black == 1 && total_white == 3 && white_knights == 2)) {
        return 1;
    }

    // Not insufficient material
    return 0;
}

_Bool IsThreefoldRepetition(Board* board)
{

}
