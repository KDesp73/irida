#include "uci.h"

#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "version.h"

FILE* in_debug_file = NULL;
FILE* out_debug_file = NULL;
FILE* original_stdout = NULL;
static State state = {0};

static void sigint_handler(int sig)
{
    if(in_debug_file) fclose(in_debug_file);
    if(out_debug_file) fclose(out_debug_file);
    if(original_stdout) fclose(original_stdout);
    uci_quit(&state);
}

#define NEW_TTY

int UciMain(int argc, char** argv)
{
    signal(SIGINT, sigint_handler);
    char input[1024];
    InitState(&state);
    LoadUciConfig(&state);

#ifdef NEW_TTY
    int saved_stdout_fd = dup(fileno(stdout));
    original_stdout = fdopen(saved_stdout_fd, "w");
#endif

    UciOption input_debug_opt;
    if(GetUciOption(&state, "DebugInputLogFile", &input_debug_opt)){
        in_debug_file = fopen(input_debug_opt.value.string, "w");
    }
    UciOption output_debug_opt;
    if(GetUciOption(&state, "DebugOutputLogFile", &output_debug_opt)){
        out_debug_file = fopen(output_debug_opt.value.string, "w");
    }

#ifdef NEW_TTY
    FILE* tty = fopen("/dev/pts/2", "w");
    if (tty)
        dup2(fileno(tty), fileno(stdout));
#endif

    LogPrintf("%s v%s by %s\n", ENGINE_NAME, VERSION, ENGINE_AUTHOR);
    fflush(stdout);

    for(;;) {
        if(fgets(input, sizeof(input), stdin) == NULL) continue; 

        input[strcspn(input, "\n")] = 0;
        if(in_debug_file)
            fprintf(in_debug_file, "%s\n", input); 

        HandleCommand(&state, input);
    }
    return 0;
}

#define IS_COMMAND(command, check) \
    (strncmp(command, check, strlen(check)) == 0 && \
    (command[strlen(check)] == '\0' || command[strlen(check)] == ' '))

bool HandleCommand(State* state, const char *command)
{
    if (IS_COMMAND(command, COMMAND_UCI)) {
        uci_uci(state);
    } else if (IS_COMMAND(command, COMMAND_ISREADY)) {
        uci_isready(state);
    } else if (IS_COMMAND(command, COMMAND_UCINEWGAME)) {
        uci_ucinewgame(state);
    } else if (IS_COMMAND(command, COMMAND_POSITION)) {
        uci_position(state, command);
    } else if (IS_COMMAND(command, COMMAND_GO)) {
        uci_go(state, command);
    } else if (IS_COMMAND(command, COMMAND_STOP)) {
        uci_stop(state);
    } else if (IS_COMMAND(command, COMMAND_QUIT)) {
        if(in_debug_file) fclose(in_debug_file);
        uci_quit(state);
    } else if (IS_COMMAND(command, COMMAND_SETOPTION)) {
        uci_setoption(state, command);
    } else if (IS_COMMAND(command, COMMAND_DEBUG)) {
        uci_debug(state, command);
    } else if (IS_COMMAND(command, COMMAND_DISPLAY)) {
        uci_display(state);
    } else {
        printf("info string Unknown command: %s\n", command);
        return false;
    }
    fflush(stdout);
    return true;
}


void StatePrint(const State* state)
{
    if (!state) {
        printf("State is NULL\n");
        return;
    }

    printf("Start Position FEN: %s\n", state->startPositionFen);
    printf("UCI Mode: %s\n", state->uciMode ? "Enabled" : "Disabled");
    printf("Debug Mode: %s\n", state->debugMode ? "Enabled" : "Disabled");
    printf("Depth Limit: %d\n", state->depthLimit);
    printf("Time Left - White: %d ms, Black: %d ms\n", state->timeLeft[0], state->timeLeft[1]);
    printf("Increment - White: %d ms, Black: %d ms\n", state->increment[0], state->increment[1]);
    printf("Moves to Go: %d\n", state->movesToGo);
    printf("Ponder Mode: %s\n", state->ponderMode ? "Enabled" : "Disabled");
    printf("Infinite Mode: %s\n", state->infiniteMode ? "Enabled" : "Disabled");
    printf("Stop Requested: %s\n", state->stopRequested ? "Yes" : "No");

    printf("\nUCI Options (%zu total):\n", state->uciOptionCount);
    for (size_t i = 0; i < state->uciOptionCount; i++) {
        const UciOption* opt = &state->uciOptions[i];
        printf("  [%zu] Name: %s\n", i + 1, opt->name);

        switch (opt->type) {
            case UCI_CHECK:
                printf("      Type: CHECK\n");
                printf("      Value: %s\n", opt->value.check ? "true" : "false");
                break;
            case UCI_SPIN:
                printf("      Type: SPIN\n");
                printf("      Value: %d (Min: %d, Max: %d)\n", opt->value.spin, opt->params.min, opt->params.max);
                break;
            case UCI_COMBO:
                printf("      Type: COMBO\n");
                printf("      Value: %s\n", opt->value.combo);
                printf("      Options: ");
                for (int j = 0; j < 10 && opt->params.combos[j][0] != '\0'; j++) {
                    printf("%s%s", opt->params.combos[j], (j < 9 && opt->params.combos[j + 1][0] != '\0') ? ", " : "");
                }
                printf("\n");
                break;
            case UCI_STRING:
                printf("      Type: STRING\n");
                printf("      Value: %s\n", opt->value.string);
                break;
        }
        printf("      Default: %s\n", opt->default_value);
    }

    printf("\nLast Command: %s\n", state->lastCommand);
}
