/*
 * Theory: UCI command dispatch.
 *
 * The engine receives text lines from stdin. This module parses the command
 * name (uci, isready, ucinewgame, position, go, stop, quit, setoption) and
 * dispatches to the appropriate handler. Commands are matched by prefix; extra
 * arguments (e.g. "position fen ..." or "go depth 10") are passed to the
 * handler. No state machine beyond "quit requested"; the GUI drives the flow.
 */
#include "uci.h"
#include "core.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "version.h"

#define IS_COMMAND(command, check) \
    (strncmp(command, check, strlen(check)) == 0 && \
    (command[strlen(check)] == '\0' || command[strlen(check)] == ' '))

bool HandleCommand(UciState* uci_state, const char *command)
{
    if (IS_COMMAND(command, COMMAND_UCI)) {
        uci_uci(uci_state);
    } else if (IS_COMMAND(command, COMMAND_ISREADY)) {
        uci_isready(uci_state);
    } else if (IS_COMMAND(command, COMMAND_UCINEWGAME)) {
        uci_ucinewgame(uci_state);
    } else if (IS_COMMAND(command, COMMAND_POSITION)) {
        uci_position(uci_state, command);
    } else if (IS_COMMAND(command, COMMAND_GO)) {
        uci_go(uci_state, command);
    } else if (IS_COMMAND(command, COMMAND_STOP)) {
        uci_stop(uci_state);
    } else if (IS_COMMAND(command, COMMAND_QUIT)) {
        uci_quit(uci_state);
    } else if (IS_COMMAND(command, COMMAND_SETOPTION)) {
        uci_setoption(uci_state, command);
    } else if (IS_COMMAND(command, COMMAND_DEBUG)) {
        uci_debug(uci_state, command);
    } else if (IS_COMMAND(command, COMMAND_DISPLAY)) {
        uci_display(uci_state);
    } else {
        printf("info string Unknown command: %s\n", command);
        return false;
    }
    fflush(stdout);
    return true;
}


void StatePrint(const UciState* uci_state)
{
    if (!uci_state) {
        printf("State is NULL\n");
        return;
    }

    printf("Start Position FEN: %s\n", uci_state->startPositionFen);
    printf("UCI Mode: %s\n", uci_state->uciMode ? "Enabled" : "Disabled");
    printf("Debug Mode: %s\n", uci_state->debugMode ? "Enabled" : "Disabled");
    printf("Depth Limit: %d\n", uci_state->depthLimit);
    printf("Time Left - White: %d ms, Black: %d ms\n", uci_state->timeLeft[0], uci_state->timeLeft[1]);
    printf("Increment - White: %d ms, Black: %d ms\n", uci_state->increment[0], uci_state->increment[1]);
    printf("Moves to Go: %d\n", uci_state->movesToGo);
    printf("Ponder Mode: %s\n", uci_state->ponderMode ? "Enabled" : "Disabled");
    printf("Infinite Mode: %s\n", uci_state->infiniteMode ? "Enabled" : "Disabled");
    printf("Stop Requested: %s\n", uci_state->stopRequested ? "Yes" : "No");

    printf("\nUCI Options (%zu total):\n", uci_state->uciOptionCount);
    for (size_t i = 0; i < uci_state->uciOptionCount; i++) {
        const UciOption* opt = &uci_state->uciOptions[i];
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

    printf("\nLast Command: %s\n", uci_state->lastCommand);
}

