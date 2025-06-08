#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "uci.h"
#include <stdio.h>

void uci_setoption(State* state, const char *command)
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

void uci_go(State* state, const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        printf("depth: %d\n", depth);
        int nodes = Perft(&state->board, depth, true);
        printf("\nNodes searched: %d\n", nodes);
    } else {
        char bestmove[16];
        int score = 0;
        Move move = FindBest(&state->board, state->depthLimit, &score);
        MoveToString(move, bestmove);
        printf("bestmove %s\n", bestmove);
    }
}

void uci_position(State* state, const char* command)
{
    char fen[128] = "";

    if (strcmp(command, "position startpos") == 0) {
        strcpy(fen, STARTING_FEN);
    } else if (strncmp(command, "position fen ", 13) == 0) {
        strncpy(fen, command + 13, sizeof(fen) - 1);
        fen[sizeof(fen) - 1] = '\0'; // Ensure null-termination
    }

    if (strlen(fen) > 0) {
        StateSetStartPos(state, fen);
    } else {
        printf("info string Invalid position command\n");
    }

}

void uci_uci(State* state)
{
    state->uciMode = true;
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", ENGINE_AUTHOR);

    printf("\n");

    PrintUciOptions(state);
    printf("uciok\n");
}

void uci_isready(State* state)
{
    printf("readyok\n");
}

void uci_ucinewgame(State* state)
{
    InitState(state);
    printf("info New game started.\n");
}

void uci_stop(State* state)
{
    // TODO: Handle stop command if a calculation is running
    state->stopRequested = true;
    printf("info Calculation stopped.\n");
}

void uci_quit(State* state)
{
    uci_stop(state);
    BoardFree(&state->board);
    exit(0);
}

void uci_debug(State* state, const char* command)
{
    state->debugMode = strcmp(command + strlen(COMMAND_DEBUG), "on") == 0;
}

void uci_display(State* state)
{
    BoardPrint(&state->board, 64);
}

