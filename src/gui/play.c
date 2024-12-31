#include "board.h"
#include "generator.h"
#include "gui/gui.h"
#include "gui/colors.h"
#include "gui/globals.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "gui/utils.h"
#include "move.h"
#include "notation.h"
#include "piece.h"
#include "utils.h"

char promotion_gui(Vector2 board_position, Texture2D textures[12], int turn)
{
    while (true) { // Loop until a valid piece is selected
        BeginDrawing();
        ClearBackground(GetColor(0x181818FF));

        // Calculate the position for the promotion GUI
        Vector2 position = place_center(
            (Rectangle) {board_position.x, board_position.y, BOARD_SIZE * SQUARE_SIZE, BOARD_SIZE * SQUARE_SIZE},
            SQUARE_SIZE * 4, 
            SQUARE_SIZE
        );

        // Draw the promotion options
        for (size_t i = 0; i < 4; i++) {
            Vector2 option_position = Vector2Add(position, (Vector2) {SQUARE_SIZE * i, 0});
            DrawRectangleV(option_position, (Vector2) {SQUARE_SIZE, SQUARE_SIZE}, RAYWHITE);
            DrawRectangleLinesEx((Rectangle){option_position.x, option_position.y, SQUARE_SIZE, SQUARE_SIZE}, 2.0, GetColor(0x000000FF));

            char piece;
            switch (i) {
                case 0: piece = 'Q'; break; // Queen
                case 1: piece = 'R'; break; // Rook
                case 2: piece = 'B'; break; // Bishop
                case 3: piece = 'N'; break; // Knight
            }
            if (turn == 0) piece = tolower(piece);

            DrawTextureV(textures[piece_to_index(piece)], option_position, RAYWHITE);
        }

        // Check if the left mouse button is pressed
        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
            Vector2 click = (Vector2) {GetMouseX(), GetMouseY()};
            Vector2 local_click = (Vector2) {click.x - position.x, click.y - position.y};

            int clicked_x = (int)(local_click.x / SQUARE_SIZE);
            int clicked_y = (int)(local_click.y / SQUARE_SIZE);

            if (clicked_y == 0 && clicked_x >= 0 && clicked_x < 4) {
                EndDrawing();
                switch (clicked_x) {
                    case 0: return 'q'; // Queen
                    case 1: return 'r'; // Rook
                    case 2: return 'b'; // Bishop
                    case 3: return 'n'; // Knight
                }
            }
        }

        EndDrawing();
    }
}

#define MoveToVector(move) \
    V(GetTo(move) % 8, 7-(GetTo(move) / 8))

void gui(const char* fen)
{
    SetTraceLogLevel(LOG_WARNING);

    // Initialize Window
    InitWindow(SQUARE_SIZE * 8, SQUARE_SIZE * 8, TITLE);
    SET_FULLSCREEN(0);
    SetTargetFPS(60);

    Texture2D textures[12];
    load_textures(textures, "wiki");

    Vector2 display_size = { GetScreenWidth(), GetScreenHeight() };

    Game game;
    GameInit(&game, NULL, TITLE, "Player 1", "Player 2", fen);
    Board board;
    BoardInitFen(&board, fen);
    Vector2 selected_square = VEMPTY;
    Vector2 move_square = VEMPTY;

    Moves moves = {0};

    Square from, to;
    char promotion = '\0';

    while (!WindowShouldClose()) {
        display_size = (Vector2){ GetScreenWidth(), GetScreenHeight() };
        Rectangle display = (Rectangle){.x = 0, .y = 0, VARGS(display_size)};
        Vector2 board_position = place_center(display, SQUARE_SIZE * BOARD_SIZE, SQUARE_SIZE * BOARD_SIZE);

        handle_exports(&board, &game);

        BeginDrawing();
        ClearBackground(GetColor(0x282828FF));

        draw_board(board_position, SQUARE_SIZE);

        // Highlight valid moves
        if (!VCMP(selected_square, VEMPTY)) {
            for (size_t i = 0; i < moves.count; i++) {
                Vector2 square = MoveToVector(moves.list[i]);
                highlight_square(board_position, square, (((int)square.x + (int)square.y) % 2) ? COLOR_HIGHLIGHT_BLACK : COLOR_HIGHLIGHT_WHITE);
            }
        }

        draw_pieces(board.grid, textures, board_position);

        if (!IsResult(&board)) {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (VCMP(selected_square, VEMPTY)) {
                    selected_square = square_clicked(board_position);
                    if (!VCMP(selected_square, VEMPTY)) {
                        Square square = selected_square.y*8 + selected_square.x;
                        moves = GenerateLegalMovesSquare(&board, square);
                    }
                } else {
                    move_square = square_clicked(board_position);
                    if (!VCMP(move_square, VEMPTY)) {
                        from = selected_square.y*8 + selected_square.x;
                        to = move_square.y*8 + move_square.x;

                        Move move = MoveEncode(from, to, PROMOTION_NONE, FLAG_NORMAL);
                        if (IS_PAWN(PieceAt(&board, from)) && IsPromotion(&board, &move)) {
                            promotion = promotion_gui(board_position, textures, board.turn);
                            MoveSetFlag(&move, FLAG_PROMOTION);
                            MoveSetPromotion(&move, CharToPromotion(promotion));
                        } else promotion = '\0';

                        SanMove san;
                        Notate(&board, move, &san);

                        if(IsLegal(&board, move)){
                            if(MakeMove(&board, move)){
                                GameAddMove(&game, san);
                                promotion = '\0';
                            }
                        }

                        selected_square = VEMPTY;
                        move_square = VEMPTY;
                    }
                }
            }
        } else {
            exit(0);
        }

        EndDrawing();
    }

    CloseWindow();
}
