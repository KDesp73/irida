#include "core.h"
#include "version.h"
#include "cli.h"
#include "eval.h"
#include "search.h"
#include "uci.h"
#include "nnue.h"
#include "tt.h"
#include "syzygy.h"
#include <stdlib.h>
#define CLI_IMPLEMENTATION
#include "IncludeOnly/cli.h"
#define LOGGING_IMPLEMENTATION
#include "IncludeOnly/logging.h"
#include <stdio.h>
#include <string.h>
#include "utils.h"

static struct {
    int hash;
    char syzygy_path[128];
    int syzygy_probe_depth;
    int syzygy_probe_limit;
    int syzygy_50_rule;
    int null_move;
    int lmr;
    int aspiration;
    int tt;
    int quiescence;
    char evalfile[128];
} customize_opts = { .hash = -1, .syzygy_probe_depth = -1, .syzygy_probe_limit = -1,
    .syzygy_50_rule = -1, .null_move = -1, .lmr = -1, .aspiration = -1, .tt = -1, .quiescence = -1 };

static void parse_customize_opts(int argc, char** argv, Context* ctx)
{
    for (int i = 2; i < argc; i++) {
        const char* a = argv[i];
        const char* v = (i + 1 < argc) ? argv[i + 1] : NULL;
        if (strcmp(a, "--eval") == 0 && v) {
            if (ctx) { free(ctx->eval); ctx->eval = strdup(v); }
            i++; continue;
        }
        if (strcmp(a, "--evalfile") == 0 && v) {
            strncpy(customize_opts.evalfile, v, sizeof(customize_opts.evalfile) - 1);
            customize_opts.evalfile[sizeof(customize_opts.evalfile)-1] = '\0';
            if (ctx) { free(ctx->nnue_path); ctx->nnue_path = strdup(v); }
            i++; continue;
        }
        if (strcmp(a, "--nnue-path") == 0 && v) {
            strncpy(customize_opts.evalfile, v, sizeof(customize_opts.evalfile) - 1);
            customize_opts.evalfile[sizeof(customize_opts.evalfile)-1] = '\0';
            if (ctx) { free(ctx->nnue_path); ctx->nnue_path = strdup(v); }
            i++; continue;
        }
        if (strcmp(a, "--hash") == 0 && v) { customize_opts.hash = atoi(v); i++; continue; }
        if (strcmp(a, "--syzygy-path") == 0 && v) {
            strncpy(customize_opts.syzygy_path, v, sizeof(customize_opts.syzygy_path) - 1);
            customize_opts.syzygy_path[sizeof(customize_opts.syzygy_path)-1] = '\0';
            i++; continue;
        }
        if (strcmp(a, "--syzygy-probe-depth") == 0 && v) { customize_opts.syzygy_probe_depth = atoi(v); i++; continue; }
        if (strcmp(a, "--syzygy-probe-limit") == 0 && v) { customize_opts.syzygy_probe_limit = atoi(v); i++; continue; }
        if (strcmp(a, "--syzygy-50-rule") == 0 && v) {
            customize_opts.syzygy_50_rule = (strcmp(v, "true") == 0 || strcmp(v, "1") == 0) ? 1 : 0;
            i++; continue;
        }
        if (strcmp(a, "--null-move") == 0 && v) {
            customize_opts.null_move = (strcmp(v, "false") == 0 || strcmp(v, "0") == 0) ? 0 : 1;
            i++; continue;
        }
        if (strcmp(a, "--lmr") == 0 && v) {
            customize_opts.lmr = (strcmp(v, "false") == 0 || strcmp(v, "0") == 0) ? 0 : 1;
            i++; continue;
        }
        if (strcmp(a, "--aspiration") == 0 && v) {
            customize_opts.aspiration = (strcmp(v, "false") == 0 || strcmp(v, "0") == 0) ? 0 : 1;
            i++; continue;
        }
        if (strcmp(a, "--tt") == 0 && v) {
            customize_opts.tt = (strcmp(v, "false") == 0 || strcmp(v, "0") == 0) ? 0 : 1;
            i++; continue;
        }
        if (strcmp(a, "--quiescence") == 0 && v) {
            customize_opts.quiescence = (strcmp(v, "false") == 0 || strcmp(v, "0") == 0) ? 0 : 1;
            i++; continue;
        }
    }
}

static void apply_customize(Context* ctx)
{
    if (ctx->eval) {
        if (strcmp(ctx->eval, "material") == 0) engine.eval = material_eval;
        else if (strcmp(ctx->eval, "pesto") == 0) engine.eval = pesto_eval;
        else if (strcmp(ctx->eval, "nnue") == 0) engine.eval = nnue_eval;
        else engine.eval = pesto_eval;
    }
    const char* nnue_path = customize_opts.evalfile[0] ? customize_opts.evalfile : ctx->nnue_path;
    if (nnue_path && nnue_path[0] && engine.eval == nnue_eval) {
        for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
            if (strcmp(uci_state.uciOptions[i].name, "EvalFile") == 0) {
                snprintf(uci_state.uciOptions[i].value.string, sizeof(uci_state.uciOptions[i].value.string), "%s", nnue_path);
                nnue_load(nnue_path);
                break;
            }
        }
    } else if (engine.eval != nnue_eval) {
        for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
            if (strcmp(uci_state.uciOptions[i].name, "EvalFile") == 0) {
                uci_state.uciOptions[i].value.string[0] = '\0';
                break;
            }
        }
    }
    if (customize_opts.null_move >= 0) g_searchConfig.useNullMove = (bool)customize_opts.null_move;
    if (customize_opts.lmr >= 0)       g_searchConfig.useLMR = (bool)customize_opts.lmr;
    if (customize_opts.aspiration >= 0) g_searchConfig.useAspiration = (bool)customize_opts.aspiration;
    if (customize_opts.tt >= 0)        g_searchConfig.useTT = (bool)customize_opts.tt;
    if (customize_opts.quiescence >= 0) g_searchConfig.useQuiescence = (bool)customize_opts.quiescence;
    if (customize_opts.syzygy_probe_depth >= 0)
        g_searchConfig.syzygyProbeDepth = customize_opts.syzygy_probe_depth < 1 ? 1 : (customize_opts.syzygy_probe_depth > 100 ? 100 : customize_opts.syzygy_probe_depth);
    if (customize_opts.syzygy_probe_limit >= 0)
        g_searchConfig.syzygyProbeLimit = customize_opts.syzygy_probe_limit < 0 ? 0 : (customize_opts.syzygy_probe_limit > 7 ? 7 : customize_opts.syzygy_probe_limit);
    if (customize_opts.syzygy_50_rule >= 0)
        g_searchConfig.syzygy50MoveRule = (bool)customize_opts.syzygy_50_rule;
    for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
        if (strcmp(uci_state.uciOptions[i].name, "SyzygyProbeDepth") == 0)
            uci_state.uciOptions[i].value.spin = g_searchConfig.syzygyProbeDepth;
        else if (strcmp(uci_state.uciOptions[i].name, "SyzygyProbeLimit") == 0)
            uci_state.uciOptions[i].value.spin = g_searchConfig.syzygyProbeLimit;
        else if (strcmp(uci_state.uciOptions[i].name, "Syzygy50MoveRule") == 0)
            uci_state.uciOptions[i].value.check = g_searchConfig.syzygy50MoveRule;
    }
    if (customize_opts.syzygy_path[0]) {
        for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
            if (strcmp(uci_state.uciOptions[i].name, "SyzygyPath") == 0) {
                snprintf(uci_state.uciOptions[i].value.string, sizeof(uci_state.uciOptions[i].value.string), "%s", customize_opts.syzygy_path);
                syzygy_init(customize_opts.syzygy_path);
                break;
            }
        }
    }
    if (customize_opts.hash >= 0) {
        int v = customize_opts.hash < 1 ? 1 : (customize_opts.hash > 2048 ? 2048 : customize_opts.hash);
        tt_init((size_t)v);
        for (size_t i = 0; i < uci_state.uciOptionCount; i++) {
            if (strcmp(uci_state.uciOptions[i].name, "Hash") == 0) {
                uci_state.uciOptions[i].value.spin = v;
                break;
            }
        }
    }
}

bool customize_handler(Context context)
{
    InitState(&uci_state);
    LoadUciConfig(&uci_state);
    apply_customize(&context);
    UciMainLoop();
    return true;
}

int CliMain(int argc, char** argv)
{
    cli_args_t args = cli_args_make(
        cli_arg_new(ARG_HELP, "help", "", no_argument),
        cli_arg_new(ARG_VERSION, "version", "", no_argument),
        cli_arg_new(ARG_FEN, "fen", "", required_argument),
        cli_arg_new(ARG_DEPTH, "depth", "", required_argument),
        cli_arg_new(ARG_EVAL, "eval", "", required_argument),
        cli_arg_new(ARG_SEARCH, "search", "", required_argument),
        cli_arg_new(ARG_NNUE_PATH, "nnue-path", "", required_argument),
        NULL
    );
    char* command_str = argc == 1 ? NULL : argv[1];
    Command command = parse_command(command_str);

    Context ctx = {0};
    context_init(&ctx, command);

    if (command != COMMAND_CUSTOMIZE) {
        int opt;
        LOOP_ARGS(opt, args) {
            switch (opt) {
                case ARG_HELP:
                    help(command);
                    goto cleanup;
                case ARG_VERSION:
                    printf("%s v%s\n", engine.name, VERSION_STRING);
                    goto cleanup;
                case ARG_FEN:
                    ctx.fen = strdup(optarg);
                    break;
                case ARG_DEPTH:
                    ctx.depth = (size_t) atoi(optarg);
                    break;
                case ARG_EVAL:
                    ctx.eval = strdup(optarg);
                    break;
                case ARG_SEARCH:
                    ctx.search = strdup(optarg);
                    break;
                case ARG_NNUE_PATH:
                    ctx.nnue_path = strdup(optarg);
                    break;
                default:
                    goto error;
            }
        }
    } else {
        for (int i = 2; i < argc; i++)
            if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
                help(COMMAND_CUSTOMIZE);
                goto cleanup;
            }
        parse_customize_opts(argc, argv, &ctx);
    }

    if (!ctx.nnue_path)
        ctx.nnue_path = strdup("./nn/nn-04cf2b4ed1da.nnue");
    
    Dispatcher dispatcher = {0};
    set_handler(&dispatcher, COMMAND_PERFT, perft_handler);
    set_handler(&dispatcher, COMMAND_EVAL, eval_handler);
    set_handler(&dispatcher, COMMAND_EVAL_BATCH, eval_batch_handler);
    set_handler(&dispatcher, COMMAND_EVAL_BREAKDOWN_BATCH, eval_breakdown_batch_handler);
    set_handler(&dispatcher, COMMAND_SEARCH, search_handler);
    set_handler(&dispatcher, COMMAND_CUSTOMIZE, customize_handler);

    HandlerFunc handler = get_handler(&dispatcher, command);
    if(!handler) {
        if(!command_str) ERRO("Please provide a command");
        else ERRO("Unknown command: `%s`", command_str);
        goto error;
    }
    if(!handler(ctx)) goto error;
    
cleanup:
    cli_args_free(&args);
    context_free(&ctx);
    return 0;

error:
    cli_args_free(&args);
    context_free(&ctx);
    return 1;

}
