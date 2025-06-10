#include "move.h"
#include "movegen.h"
#include "perft.h"
#include "search.h"
#include "uci.h"
#include <ctype.h>
#include <stdio.h>

void uci_setoption(State* state, const char *command)
{
    char option_name[64];
    char option_value[128];

    if (sscanf(command, "setoption name %63s value %127[^\n]", option_name, option_value) != 2) {
        LogPrintf("info string Invalid setoption format\n");
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
                    LogPrintf("info string Unknown option type\n");
                    return;
            }
            LogPrintf("info string Option %s set to %s\n", option_name, option_value);
            return;
        }
    }

    LogPrintf("info string Unknown option: %s\n", option_name);
}

void uci_go(State* state, const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        LogPrintf("depth: %d\n", depth);
        int nodes = Perft(&state->board, depth, true);
        LogPrintf("\nNodes searched: %d\n", nodes);
    } else {
        char bestmove[16];
        int score = 0;
        Move move = FindBestIterative(&state->board, state->depthLimit, &score);
        MoveToString(move, bestmove);
        LogPrintf("bestmove %s\n", bestmove);
    }
}

void uci_position(State* state, const char* command)
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
        LogPrintf("info string Invalid position command\n");
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

            Move move = StringToMove(move_str);
            if (!MakeMove(&state->board, move)) {
                LogPrintf("info string Illegal move in position: %s\n", move_str);
                break;
            }
        }
    }
}

void uci_uci(State* state)
{
    state->uciMode = true;
    LogPrintf("id name %s\n", ENGINE_NAME);
    LogPrintf("id author %s\n", ENGINE_AUTHOR);

    LogPrintf("\n");

    PrintUciOptions(state);
    LogPrintf("uciok\n");
}

void uci_isready(State* state)
{
    LogPrintf("readyok\n");
}

void uci_ucinewgame(State* state)
{
    InitState(state);
    LogPrintf("info New game started.\n");
}

void uci_stop(State* state)
{
    // TODO: Handle stop command if a calculation is running
    state->stopRequested = true;
    LogPrintf("info Calculation stopped.\n");
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

