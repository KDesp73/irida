#include "board.h"
#include "square.h"
#include <stdio.h>
#include <inttypes.h>
#include "zobrist.h"
#include <stdarg.h>


void TuiBoardPrintSquares(const Board* board, ui_config_t config, Square* squares, size_t count)
{
    const char* yellow_bg = "\033[48;5;214m"; // Background yellow color
    const char* reset = "\033[0m";             // Reset color formatting

    const char* padding = "   ";

    if (config.errors && board->state.error > 0) {
        fprintf(stderr, "Error: %s\n", error_messages[board->state.error]);
    }

    printf("%s┌───┬───┬───┬───┬───┬───┬───┬───┐\n", (config.coords) ? padding : "");

    for (int rank = (config.perspective ? 0 : 7);
         config.perspective ? rank <= 7 : rank >= 0;
         config.perspective ? rank++ : rank--) {

        if (config.coords) {
            printf(" %d ", !config.perspective ? rank + 1 : 8 - rank);
        }

        for (int file = 0; file <= 7; ++file) {
            int highlighted = 0;

            if (squares != NULL) {
                for (size_t i = 0; i < count; ++i) {
                    int square_rank = squares[i] / 8;
                    int square_file = squares[i] % 8;

                    if (!config.perspective) {
                        square_rank = 7 - square_rank;
                    }

                    if (rank == square_rank && file == square_file) {
                        highlighted = 1;
                        break;
                    }
                }
            }

            printf("│");

            char piece = ' '; // Default empty square
            for (int i = 0; i < PIECE_TYPES; ++i) {
                if (board->bitboards[i] & (1ULL << (rank * 8 + file))) {
                    piece = PIECES[i];
                    break;
                }
            }

            if (config.highlights && highlighted) {
                printf("%s %c %s", yellow_bg, piece, reset);
            } else {
                printf(" %c ", piece);
            }
        }
        printf("│\n");

        if (rank != (!config.perspective ? 0 : 7)) {
            printf("%s├───┼───┼───┼───┼───┼───┼───┼───┤\n", (config.coords) ? padding : "");
        }
    }
    printf("%s└───┴───┴───┴───┴───┴───┴───┴───┘\n", (config.coords) ? padding : "");

    if (config.coords) {
        printf("%s", padding);
        for (int file = 0; file < 8; ++file) {
            char label = config.perspective ? 'a' + file : 'h' - file;
            printf("  %c ", label);
        }
        printf("\n");
    }

    printf("\n");
    if (config.castling) {
        tui_print_castling_rights(board->state);
    }

    if (config.enpassant && board->enpassant_square != 64) {
        char enpassant[3];
        SquareToName(enpassant, board->enpassant_square);
        printf("En passant square: %s\n", enpassant);
    }

    if (config.halfmove) {
        printf("Halfmove clock: %zu\n", board->state.halfmove);
    }

    if (config.fullmove) {
        printf("Fullmove number: %zu\n", board->state.fullmove);
    }

    if (config.checks) {

    }

    if (config.turn) {
        printf("%s's turn\n", board->state.turn ? "White" : "Black");
    }

    if (config.hash) {
        printf("Hash : 0x%" PRIx64 "\n", CalculateZobristHash(board));
    }

    printf("\n");
}

void TuiBoardPrint(const Board* board, ui_config_t config, Square first, ...)
{
    Square* squares = NULL;
    int count = 0;

    if (first != 64) { // Use 64 as the terminator value
        va_list args;
        va_start(args, first);

        squares = (Square*)malloc(100 * sizeof(Square)); // Allocate memory for squares
        if (!squares) {
            perror("malloc failed");
            return;
        }

        squares[count++] = first;

        Square next_square;
        while ((next_square = va_arg(args, int)) != 64) { // Fetch arguments as int
            squares[count++] = next_square;
            if (count % 100 == 0) {
                squares = (Square*)realloc(squares, (count + 100) * sizeof(Square));
                if (!squares) {
                    perror("realloc failed");
                    va_end(args);
                    return;
                }
            }
        }
        va_end(args);

        // Resize to exact size
        squares = (Square*)realloc(squares, count * sizeof(Square));
        if (!squares) {
            perror("realloc failed");
            return;
        }
    }

    // Print the board with the collected squares
    TuiBoardPrintSquares(board, config, squares, count);

    // Free allocated memory
    if (squares) {
        free(squares);
        squares = NULL;
    }
}

