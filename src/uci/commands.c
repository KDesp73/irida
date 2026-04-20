#include "eval.h"
#include "search.h"
#include "uci.h"
#include "core.h"
#include "tt.h"
#include "nnue.h"
#include "syzygy.h"
#include "version.h"
#include <ctype.h>
#include <stdio.h>
#include <string.h>

static void trim_tail(char* s)
{
    for (size_t i = strlen(s); i > 0 && (s[i - 1] == ' ' || s[i - 1] == '\t'); i--)
        s[i - 1] = '\0';
}

void uci_setoption(UciState* state, const char *command)
{
    char option_name[64];
    char option_value[128];

    const char* name_key = " name ";
    const char* value_key = " value ";
    const char* p = strstr(command, name_key);
    if (!p) {
        printf("info string Invalid setoption format\n");
        return;
    }
    p += strlen(name_key);
    const char* value_start = strstr(p, value_key);
    if (!value_start) {
        printf("info string Invalid setoption format\n");
        return;
    }
    size_t name_len = (size_t)(value_start - p);
    if (name_len >= sizeof(option_name))
        name_len = sizeof(option_name) - 1;
    memcpy(option_name, p, name_len);
    option_name[name_len] = '\0';
    trim_tail(option_name);

    const char* val = value_start + strlen(value_key);
    strncpy(option_value, val, sizeof(option_value) - 1);
    option_value[sizeof(option_value) - 1] = '\0';
    trim_tail(option_value);

    for (size_t i = 0; i < state->uciOptionCount; i++) {
        if (strcmp(state->uciOptions[i].name, option_name) == 0) {
            switch (state->uciOptions[i].type) {
                case UCI_CHECK:
                    state->uciOptions[i].value.check = (strcmp(option_value, "true") == 0);
                    if (strcmp(option_name, "Syzygy50MoveRule") == 0)
                        g_searchConfig.syzygy50MoveRule = state->uciOptions[i].value.check;
                    break;
                case UCI_SPIN: {
                    int v = atoi(option_value);
                    state->uciOptions[i].value.spin = v;
                    if (strcmp(option_name, "Hash") == 0) {
                        if (v < 1) v = 1;
                        if (v > 2048) v = 2048;
                        irida_TTInit((size_t)v);
                    } else if (strcmp(option_name, "SyzygyProbeDepth") == 0) {
                        g_searchConfig.syzygyProbeDepth = (v < 1) ? 1 : (v > 100) ? 100 : v;
                    } else if (strcmp(option_name, "SyzygyProbeLimit") == 0) {
                        g_searchConfig.syzygyProbeLimit = (v < 0) ? 0 : (v > 7) ? 7 : v;
                    }
                    break;
                }
                case UCI_COMBO:
                    snprintf(state->uciOptions[i].value.combo, sizeof(state->uciOptions[i].value.combo), "%s", option_value);
                    break;
                case UCI_STRING:
                    snprintf(state->uciOptions[i].value.string, sizeof(state->uciOptions[i].value.string), "%s", option_value);
                    if (strcmp(option_name, "EvalFile") == 0) {
                        if (irida_NNUELoad(option_value))
                            printf("info string EvalFile loaded: %s\n", option_value);
                        else
                            printf("info string Failed to load EvalFile '%s' (using PeSTO evaluation)\n", option_value);
                    } else if (strcmp(option_name, "SyzygyPath") == 0)
                        irida_SyzygyInit(option_value);
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

/* Parse "go depth N", "go movetime M", "go wtime W btime B", "go infinite" and set g_searchConfig. */
static void parse_go_command(const char* command)
{
    /* Defaults: use existing config; override from command */
    int maxDepth = g_searchConfig.maxDepth;
    int timeLimitMs = g_searchConfig.timeLimitMs;
    bool saw_wtime = false, saw_btime = false;

    const char* p = command + 2;  /* skip "go" */
    while (*p) {
        while (*p == ' ') p++;
        if (!*p) break;
        if (strncmp(p, "depth ", 6) == 0) {
            maxDepth = atoi(p + 6);
            if (maxDepth < 1) maxDepth = 1;
            if (maxDepth > 128) maxDepth = 128;
            p += 6;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "movetime ", 9) == 0) {
            timeLimitMs = atoi(p + 9);
            if (timeLimitMs < 0) timeLimitMs = 0;
            p += 9;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "infinite", 8) == 0 && (p[8] == '\0' || p[8] == ' ')) {
            timeLimitMs = 0;
            maxDepth = 128;
            p += 8;
            continue;
        }
        if (strncmp(p, "wtime ", 6) == 0) {
            uci_state.timeLeft[0] = atoi(p + 6);
            saw_wtime = true;
            p += 6;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "btime ", 6) == 0) {
            uci_state.timeLeft[1] = atoi(p + 6);
            saw_btime = true;
            p += 6;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "winc ", 5) == 0) {
            uci_state.increment[0] = atoi(p + 5);
            p += 5;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "binc ", 5) == 0) {
            uci_state.increment[1] = atoi(p + 5);
            p += 5;
            while (*p && *p != ' ') p++;
            continue;
        }
        if (strncmp(p, "movestogo ", 10) == 0) {
            uci_state.movesToGo = atoi(p + 10);
            p += 10;
            while (*p && *p != ' ') p++;
            continue;
        }
        /* Unknown token: skip until next space */
        while (*p && *p != ' ') p++;
    }

    /* If wtime/btime were given in this command, set time limit from remaining time.
     * Allocate (time_left / moves) + increment with a 10% reserve to avoid flagging. */
    if ((saw_wtime || saw_btime) && timeLimitMs <= 0) {
        int side = engine.board.turn ? 0 : 1;  /* white = 0, black = 1 */
        int time_left = uci_state.timeLeft[side];
        int inc = uci_state.increment[side];
        int moves = (uci_state.movesToGo > 0) ? uci_state.movesToGo : 20;
        if (moves < 1) moves = 1;
        int alloc = (time_left / moves) + inc;
        if (alloc > 0)
            timeLimitMs = (alloc * 90) / 100;  /* use 90% to keep reserve */
    }

    /* If still no time limit (plain "go" or no time params), use 10s default so we don't hang */
    // if (timeLimitMs <= 0 && !go_infinite)
    //     timeLimitMs = 10000;

    g_searchConfig.maxDepth = maxDepth;
    g_searchConfig.timeLimitMs = timeLimitMs;
}

void uci_go(UciState* state, const char* command)
{
    if (strncmp(command, "go perft ", 9) == 0) {
        int depth = atoi(command + 9);
        printf("depth: %d\n", depth);
        int nodes = castro_Perft(&engine.board, depth, true);
        printf("\nNodes searched: %d\n", nodes);
    } else {
        parse_go_command(command);
        state->stopRequested = false;  /* clear so search runs until time or explicit stop */
        irida_UciSearchStart();  /* run search in worker thread; bestmove printed there */
    }
}

void uci_position(UciState* state, const char* command)
{
    irida_UciSearchWaitDone();  /* don't touch board while search is running */

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
    irida_StateSetStartPos(state, fen);

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

    castro_FenExport(&engine.board, state->gameFen);
    state->gameFen[sizeof(state->gameFen) - 1] = '\0';
}

void uci_uci(UciState* state)
{
    state->uciMode = true;
    printf("id name %s\n", engine.name);
    printf("id author %s\n", engine.author);
    printf("id version %s\n", VERSION_STRING);

    printf("\n");

    irida_PrintUciOptions(state);

    for(size_t i = 0; i < state->uciOptionCount; i++) {
        UciOption option = state->uciOptions[i];

        if(!strcmp(option.name, "EvalFile")) {
            // if(!nnue_load(option.value.string)){
            //     printf("error Could not load nnue %s\n", option.value.string);
            // }
        } else if(!strcmp(option.name, "SyzygyPath")) {
            if(!irida_SyzygyInit(option.value.string)) {
                printf("error Could not load syzygy tablebase from %s\n", option.value.string);
            }
        } else if(!strcmp(option.name, "Hash")) {
            size_t hash_size = option.value.spin;
            irida_TTInit(hash_size);
        }
    }

    printf("uciok\n");
}

void uci_isready(UciState* state)
{
    UNUSED(state);
    printf("readyok\n");
}

void uci_ucinewgame(UciState* state)
{
    irida_TTClear();
    irida_OrderingReset();
    irida_InitState(state);
    printf("info New game started.\n");
}

void uci_stop(UciState* state)
{
    state->stopRequested = true;  /* search thread will see this and return bestmove */
    printf("info Calculation stopped.\n");
}

void uci_quit(UciState* state)
{
    state->stopRequested = true;  /* stop any running search first */
    state->quitRequested = true;  /* then exit the main loop */
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

void uci_seteval(UciState* state, const char* command)
{
    UNUSED(state);

    if(strlen(command) <= strlen(COMMAND_SETEVAL) + 1) {
        fprintf(stderr, "error Provide a valid evaluation function name: ");
        fprintf(stderr, "pesto, nnue, material\n");
        return;
    }

    const char* eval_name = command + strlen(COMMAND_SETEVAL) + 1; // +1 for the space

    if(!strcmp(eval_name, "pesto")) {
        engine.eval = irida_Evaluation;
    } else if (!strcmp(eval_name, "nnue")) {
        for (size_t i = 0; i < state->uciOptionCount; i++) {
            if (strcmp(state->uciOptions[i].name, "EvalFile") == 0
                && state->uciOptions[i].value.string[0] != '\0') {
                const char* path = state->uciOptions[i].value.string;
                if (irida_NNUELoad(path))
                    printf("info string EvalFile loaded: %s\n", path);
                else
                    printf("info string Failed to load EvalFile '%s' (using PeSTO evaluation)\n", path);
                break;
            }
        }
        engine.eval = irida_EvalNNUE;
    } else if (!strcmp(eval_name, "material")) {
        engine.eval = irida_EvalMaterial;
    } else {
        fprintf(stderr, "error Invalid evaluation function name\n");
        return;
    }

    printf("info Set %s as the evaluation function\n", eval_name);
}

void uci_setsearch(UciState* state, const char* command)
{
    UNUSED(state);

    if(strlen(command) <= strlen(COMMAND_SETSEARCH) + 1) {
        fprintf(stderr, "error Provide a valid search function name: ");
        fprintf(stderr, "id_ab, id_ab_q_mo\n");
        return;
    }

    const char* search_name = command + strlen(COMMAND_SETSEARCH) + 1; // +1 for the space

    SearchFn searchfn = NULL;
    for(size_t i = 0; i < sizeof(search_variants) / sizeof(search_variants[0]); ++i) {
        if(strcmp(search_name, search_variants[i].name))
            searchfn = search_variants[i].fn;
    }
    if(!searchfn) {
        fprintf(stderr, "error Invalid search function name\n");
        return;
    }
    engine.search = searchfn;
    printf("info Set %s as the search function\n", search_name);
}
