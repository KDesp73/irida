#include "board.h"
#include <stdio.h>

void Uint32Print(uint32_t value)
{
    for (int i = 31; i >= 0; i--) {
        printf("%d", (value >> i) & 1);
        if (i % 4 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void Uint64Print(uint64_t value)
{
    for (int i = 63; i >= 0; i--) {
        printf("%lu", (value >> i) & 1);
        if (i % 8 == 0) {
            printf(" ");
        }
    }
    printf("\n");
}

void PiecesPrint(const Board* board, int index)
{
    BitboardPrint(board->bitboards[index]);
}

void BitboardPrint(Bitboard bitboard)
{
    const char* yellow_bg = "\033[48;5;214m"; // Background yellow color
    const char* reset = "\033[0m";             // Reset color formatting
    const char* padding = "   ";

    printf("%s┌───┬───┬───┬───┬───┬───┬───┬───┐\n", padding);

    for (int rank = 0; rank <= 7; rank++) {
        printf(" %d ", 8 - rank);  // Rank label (1-8)

        for (int file = 0; file <= 7; ++file) {
            int square_index = rank * 8 + file;  // Adjusted for a1 -> 0
            char piece = ' ';

            // Check if a piece is on this square
            if (bitboard & (1ULL << square_index)) {
                piece = ' ';
            }

            // Check if the square is highlighted in the bitboard
            int highlighted = (bitboard & (1ULL << square_index)) != 0;

            printf("│");

            if (highlighted) {
                printf("%s %c %s", yellow_bg, piece, reset);  // Highlighted piece
            } else {
                printf(" %c ", piece);  // Regular piece
            }
        }

        printf("│\n");

        if (rank != 7) {
            printf("%s├───┼───┼───┼───┼───┼───┼───┼───┤\n", padding);
        }
    }

    printf("%s└───┴───┴───┴───┴───┴───┴───┴───┘\n", padding);

    // Print the file labels (a-h)
    printf("%s", padding);
    for (int file = 0; file < 8; ++file) {
        char label = 'a' + file;
        printf("  %c ", label);
    }
    printf("\n");
    Uint64Print(bitboard);
}

