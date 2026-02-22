#include "search.h"
#include "uci.h"
#include "core.h"
#include <ctype.h>
#include <stdio.h>

void uci_setoption(UciState* state, const char *command)
{
    char option_name[64];
    char option_value[128];

    if (sscanf(command, "setoption name %63s value %127[^\n]", option_name, option_value) != 2) {
        printf("info string Invalid setoption format\n");
        return;
    }

    for (size_t i = 0; i < state->uciOptionCount; i++) {
        if (strcmp(state->uciOptions[i].name, option_name) == 0) {
            switch (state->uciOptions[i].type) {
                case UCI_CHECK:
                    state->uciOptions[i].value.check = (strcmp(option_value, "true") == 0);
                    break;
                case UCI_SPIN:
                    state->uciOptions[i].value.spin = atoi(option_value);
                    break;
                case UCI_COMBO:
                    snprintf(state->uciOptions[i].value.combo, sizeof(state->uciOptions[i].value.combo), "%s", option_value);
                    break;
                case UCI_STRING:
                    snprintf(state->uciOptions[i].value.string, sizeof(state->uciOptions[i].value.string), "%s", option_value);
                    break;
                default:
                    printf("info string Unknown option type\n");
                    return;
            }
            printf("info string Option %s set to %s\n", option_name, option_value);
            return;
        }
    }

    printf("info string Unknown option: %s\n", option_name);
}

#include "utils.h"
void uci_go(UciState* state, const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        printf("depth: %d\n", depth);
        int nodes = castro_Perft(&engine.board, depth, true);
        printf("\nNodes searched: %d\n", nodes);
    } else {
        char bestmove[16];
        Move move = engine.search(&engine.board, engine.eval, engine.order, &g_searchConfig);
        castro_MoveToString(move, bestmove);
        printf("bestmove %s\n", bestmove);
    }
}

void uci_position(UciState* state, const char* command)
{
    char fen[128] = "";
    const char* moves_str = NULL;

    if (strncmp(command, "position startpos", 17) == 0) {
        strcpy(fen, STARTING_FEN);
        moves_str = strstr(command, "moves");
        if (moves_str) {
            moves_str += 6; // skip "moves "
        }
    } else if (strncmp(command, "position fen ", 13) == 0) {
        const char* fen_start = command + 13;
        const char* moves_keyword = strstr(command, " moves ");
        size_t fen_len = moves_keyword ? (size_t)(moves_keyword - fen_start) : strlen(fen_start);
        if (fen_len >= sizeof(fen))
            fen_len = sizeof(fen) - 1;
        strncpy(fen, fen_start, fen_len);
        fen[fen_len] = '\0';

        if (moves_keyword)
            moves_str = moves_keyword + 7; // skip " moves "
    } else {
        printf("info string Invalid position command\n");
        return;
    }

    // Set up the position from FEN or startpos
    StateSetStartPos(state, fen);

    // Play any moves listed
    if (moves_str && strlen(moves_str) > 0) {
        char move_str[8];
        const char* ptr = moves_str;

        while (*ptr) {
            int len = 0;
            while (*ptr && !isspace(*ptr) && len < (int)sizeof(move_str) - 1) {
                move_str[len++] = *ptr++;
            }
            move_str[len] = '\0';

            while (*ptr && isspace(*ptr)) ptr++; // Skip whitespace

            Move move = castro_StringToMove(move_str);
            if (!castro_MakeMove(&engine.board, move)) {
                printf("info string Illegal move in position: %s\n", move_str);
                break;
            }
        }
    }
}

void uci_uci(UciState* state)
{
    state->uciMode = true;
    printf("id name %s\n", engine.name);
    printf("id author %s\n", engine.author);

    printf("\n");

    PrintUciOptions(state);
    printf("uciok\n");
}

void uci_isready(UciState* state)
{
    UNUSED(state);
    printf("readyok\n");
}

void uci_ucinewgame(UciState* state)
{
    InitState(state);
    printf("info New game started.\n");
}

void uci_stop(UciState* state)
{
    // TODO: Handle stop command if a calculation is running
    state->stopRequested = true;
    printf("info Calculation stopped.\n");
}

void uci_quit(UciState* state)
{
    uci_stop(state);
}

void uci_debug(UciState* state, const char* command)
{
    state->debugMode = strcmp(command + strlen(COMMAND_DEBUG), "on") == 0;
}

void uci_display(UciState* state)
{
    UNUSED(state);
    castro_BoardPrint(&engine.board, 64);
}
