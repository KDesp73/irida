#include "gui/colors.h"
#include "gui/globals.h"
#include "gui/gui.h"
#include "../lib/raylib/include/raylib.h"
#include "../lib/raylib/include/raymath.h"
#include "gui/utils.h"
#include "notation.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Vector2 place_center(Rectangle outer, float innerWidth, float innerHeight)
{
    Rectangle inner;
    inner.width = innerWidth;
    inner.height = innerHeight;
    inner.x = outer.x + (outer.width - innerWidth) / 2.0f;
    inner.y = outer.y + (outer.height - innerHeight) / 2.0f;
    return V(inner.x, inner.y);
}

void draw_board(Vector2 board_position, float square_size)
{
    for (size_t i = 0; i < BOARD_SIZE; i++) {
        for (size_t j = 0; j < BOARD_SIZE; j++) {
            Vector2 square = Vector2Add(board_position, (Vector2){i * square_size, j * square_size});
            DrawRectangleV(square, SQUARE(square_size), ((i + j) % 2 == 0 ? COLOR_WHITE_SQUARE : COLOR_BLACK_SQUARE));
        }
    }
}

void load_textures(Texture2D textures[12], const char* theme)
{
    const char* pieces = "BKNPQR";
    for(size_t j = 0; j < 2; j++){
        for(size_t i = 0; i < strlen(pieces); i++){
            char path[256];
            sprintf(path, "assets/%s/%c%c.png", theme, (j == 0) ? 'b' : 'w', pieces[i]);
            Image image = LoadImage(path);
            ImageResize(&image, SQUARE_SIZE, SQUARE_SIZE);
            textures[(j) ? i : strlen(pieces) + i] = LoadTextureFromImage(image);
            UnloadImage(image);
        }
    }
}

int piece_to_index(char piece)
{
    switch(piece){
        case 'B': return 0;
        case 'K': return 1;
        case 'N': return 2;
        case 'P': return 3;
        case 'Q': return 4;
        case 'R': return 5;
        case 'b': return 6;
        case 'k': return 7;
        case 'n': return 8;
        case 'p': return 9;
        case 'q': return 10;
        case 'r': return 11;
        default: return -1;
    }
}

void draw_pieces(char board[8][8], Texture2D textures[12], Vector2 board_position)
{
    for (int rank = BOARD_SIZE-1; rank >= 0; rank--) {  // Iterate top to bottom
        for (size_t file = 0; file < BOARD_SIZE; file++) {
            Vector2 square = Vector2Add(board_position, (Vector2){file * SQUARE_SIZE, rank * SQUARE_SIZE});

            char piece = board[7-rank][file];  // Flip rank indexing

            if (piece == ' ') {
                continue;
            }

            int texture_index = piece_to_index(piece);

            if (texture_index >= 0 && texture_index < 12) {
                DrawTextureV(textures[texture_index], square, RAYWHITE);
            } else {
                printf("Invalid texture index: %d\n", texture_index);
            }
        }
    }
}

Vector2 square_clicked(Vector2 board_position)
{
    Vector2 click = (Vector2){GetMouseX(), GetMouseY()};
    int clicked_file = (click.x - board_position.x) / SQUARE_SIZE;
    int clicked_rank = (click.y - board_position.y) / SQUARE_SIZE;
    return (Vector2){clicked_file, 7-clicked_rank};
}

void highlight_square(Vector2 board_position, Vector2 square, Color color)
{
    Vector2 square_position = Vector2Add(board_position, (Vector2){square.x * SQUARE_SIZE, square.y * SQUARE_SIZE});

    DrawRectangleV(square_position, SQUARE(SQUARE_SIZE), color);
}

Vector2 text_size(const char* text, int fontSize)
{
    int textWidth = MeasureText(text, fontSize);
    int textHeight = fontSize;

    Rectangle rect = { VARGS(VEMPTY), (float)textWidth, (float)textHeight };
    return V(rect.width, rect.height);
}


void handle_exports(Board* board, Game* game)
{
        if(IsKeyPressed(KEY_F)){
            char fen[256];
            FenExport(board, fen);
            SetClipboardText(fen);
        }

        if(IsKeyPressed(KEY_P)){
            char pgn[2048];
            PgnExport(game, pgn);
            GamePrint(*game);
            SetClipboardText(pgn);
        }
}

