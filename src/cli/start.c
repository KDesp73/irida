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
                TODO("help");
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
            default:
                goto error;
        }
    }
    
    Dispatcher dispatcher = {0};
    set_handler(&dispatcher, COMMAND_PERFT, perft_handler);
    set_handler(&dispatcher, COMMAND_EVAL, eval_handler);
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
