
#include "IncludeOnly/logging.h"
#include "castro.h"
#include "core.h"
#include "eval.h"
#include "moveordering.h"
#include "search.h"
#include "shared.h"
#include <stdio.h>
#include <string.h>
#include "IncludeOnly/kv.h"
#include "IncludeOnly/ansi.h"

bool user_move(Board* board, Move* move)
{
    printf("> ");
    char move_in[12];
    scanf("%s", move_in);

    INFO("Entered %s\n", move_in);

    *move = castro_StringToMove(move_in);
    if (*move == NULL_MOVE) {
        fprintf(stderr, "error Invalid input\n");
        return false;
    }
    
    if (!castro_MoveIsValid(board, *move, board->turn)) {
        fprintf(stderr, "error Illegal move\n");
        return false;
    }

    return true;
}


int main(int argc, char** argv)
{
    g_searchConfig.maxDepth = 12;
    g_searchConfig.timeLimitMs = 100;

    kv_parse(argc, argv);
    PieceColor playas = strcmp(kv_get("playas", "white"), "black") == 0 ? COLOR_BLACK : COLOR_WHITE;

    irida_EngineInit(&engine);
    engine.eval = irida_Evaluation;
    engine.search = irida_Search;
    engine.order = irida_OrderMoves;

    ansi_clear_screen();
    castro_BoardInitFen(&engine.board, STARTING_FEN);

    castro_BoardPrintGrid(&engine.board);
    while(!castro_IsResult(&engine.board)) {
        Move move;
        if (engine.board.turn == playas) {
            while(!user_move(&engine.board, &move));

            castro_MakeMove(&engine.board, move);
        } else {
            char fen[256];
            castro_FenExport(&engine.board, fen);
            move = engine.search(&engine.board, engine.eval, engine.order, &g_searchConfig);
            castro_BoardInitFen(&engine.board, fen);
            if (!castro_MakeMove(&engine.board, move)) {
                fprintf(stderr, "error Engine failed to play a legal move\n");
                break;
            }
        }
        ansi_clear_screen();
        castro_BoardPrintMove(&engine.board, move);
    }

    if(castro_IsCheckmate(&engine.board)) {
        printf("info Checkmate\n");
    } else if (castro_IsInsufficientMaterial(&engine.board)) {
        printf("info Draw by Insufficient Material\n");
    } else if (castro_IsThreefoldRepetition(&engine.board)) {
        printf("info Draw by repetition\n");
    } else if (engine.board.halfmove >= 100) {
        printf("info Draw by 50 move rule\n");
    }

    castro_BoardFree(&engine.board);
    return 0;
}
