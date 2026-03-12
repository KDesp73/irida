/*
 * Theory: CLI entry and argument parsing.
 *
 * CliMain is called from main when argv has arguments. We use getopt_long to
 * parse --help, --version, --fen, --depth, --eval, --search, --nnue-path, etc.
 * The parsed command and options are stored in a context; then we dispatch to
 * the appropriate handler (perft, eval, search) or print help/version. This
 * file wires the CLI API to the engine's eval and search.
 */
#include "core.h"
#include "version.h"
#include "cli.h"
#include <stdlib.h>
#define CLI_IMPLEMENTATION
#include "IncludeOnly/cli.h"
#define LOGGING_IMPLEMENTATION
#include "IncludeOnly/logging.h"
#include <stdio.h>
#include <string.h>
#include "utils.h"

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

    if (!ctx.nnue_path)
        ctx.nnue_path = strdup("./nn/nn-04cf2b4ed1da.nnue");
    
    Dispatcher dispatcher = {0};
    set_handler(&dispatcher, COMMAND_PERFT, perft_handler);
    set_handler(&dispatcher, COMMAND_EVAL, eval_handler);
    set_handler(&dispatcher, COMMAND_EVAL_BATCH, eval_batch_handler);
    set_handler(&dispatcher, COMMAND_EVAL_BREAKDOWN_BATCH, eval_breakdown_batch_handler);
    set_handler(&dispatcher, COMMAND_SEARCH, search_handler);

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
