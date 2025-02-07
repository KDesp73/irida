#include "uci.h"
#include "move.h"
#include "search.h"
#include <io/ansi.h>
#include <io/logging.h>

int UciMain(int argc, char** argv)
{
    char input[1024];

    printf("Welcome to %s%s%s by %s%s%s\n", ANSI_RED, ENGINE_NAME, ANSI_RESET, ANSI_BLUE, ENGINE_AUTHOR, ANSI_RESET);
    FLUSH;
    
    for(;;) {
        if(fgets(input, sizeof(input), stdin) == NULL) continue; 

        input[strcspn(input, "\n")] = 0;

        HandleCommand(input);
    }

    return 0;
}

void setoption(const char *command)
{
    char option_name[64];
    char option_value[128];

    if (sscanf(command, "setoption name %63s value %127[^\n]", option_name, option_value) != 2) {
        printf("info string Invalid setoption format\n");
        return;
    }

    for (size_t i = 0; i < state.uciOptionCount; i++) {
        if (strcmp(state.uciOptions[i].name, option_name) == 0) {
            switch (state.uciOptions[i].type) {
                case UCI_CHECK:
                    state.uciOptions[i].value.check = (strcmp(option_value, "true") == 0);
                    break;
                case UCI_SPIN:
                    state.uciOptions[i].value.spin = atoi(option_value);
                    break;
                case UCI_COMBO:
                    strncpy(state.uciOptions[i].value.combo, option_value, sizeof(state.uciOptions[i].value.combo) - 1);
                    break;
                case UCI_STRING:
                    strncpy(state.uciOptions[i].value.string, option_value, sizeof(state.uciOptions[i].value.string) - 1);
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

void go(const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        printf("depth: %d\n", depth);
        int nodes = Perft(&state.board, depth, true);
        printf("\nNodes searched: %d\n", nodes);
    } else {
        char bestmove[16];
        Move move = FindBest(&state.board, state.depthLimit);
        MoveToString(move, bestmove);
        printf("bestmove %s\n", bestmove);
    }
}

void position(const char* command)
{
    char fen[128] = "";

    if (strcmp(command, "position startpos") == 0) {
        strcpy(fen, STARTING_FEN);
    } else if (strncmp(command, "position fen ", 13) == 0) {
        strncpy(fen, command + 13, sizeof(fen) - 1);
        fen[sizeof(fen) - 1] = '\0'; // Ensure null-termination
    }

    if (strlen(fen) > 0) {
        StateSetStartPos(fen);
    } else {
        printf("info string Invalid position command\n");
    }

}

