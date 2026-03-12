#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

// @module cli
// @desc CLI (non-UCI): command parsing, context, handlers, perft/eval/search entry points.

// @enum Command
// @desc CLI command: PERFT, EVAL, EVAL_BATCH, EVAL_BREAKDOWN_BATCH, SEARCH, NONE.
typedef enum {
    COMMAND_PERFT,
    COMMAND_EVAL,
    COMMAND_EVAL_BATCH,
    COMMAND_EVAL_BREAKDOWN_BATCH,
    COMMAND_SEARCH,
    COMMAND_NONE,
} Command;

// @const COMMAND_COUNT
// @desc Number of commands (COMMAND_NONE).
#define COMMAND_COUNT COMMAND_NONE

// @function parse_command
// @param str Command string (e.g. "perft", "eval").
// @returns Command Parsed command or COMMAND_NONE.
Command parse_command(const char* str);

// @enum CliArgs
// @desc Short option characters for getopt (h, v, F, d, E, S, N).
typedef enum {
   ARG_HELP = 'h',
   ARG_VERSION = 'v',
   ARG_FEN = 'F',
   ARG_DEPTH = 'd',
   ARG_EVAL = 'E',
   ARG_SEARCH = 'S',
   ARG_NNUE_PATH = 'N'
} CliArgs;

// @struct Context
// @desc Parsed CLI context: fen, eval, search, nnue_path, depth, command.
typedef struct {
    char* fen;
    char* eval;
    char* search;
    char* nnue_path;
    size_t depth;
    Command command;
} Context;

// @function context_init
// @param ctx Context to initialize.
// @param command Initial command.
void context_init(Context* ctx, Command command);

// @function context_reset
// @param ctx Context to reset.
void context_reset(Context* ctx);

// @function context_free
// @param ctx Context to free.
void context_free(Context* ctx);

// @type HandlerFunc
// @desc CLI handler: Context -> bool (success).
typedef bool (*HandlerFunc)(Context);

// @struct Dispatcher
// @desc Table of handlers per command.
typedef struct {
   HandlerFunc table[COMMAND_COUNT];
} Dispatcher;

// @function set_handler
// @param this Dispatcher.
// @param command Command to bind.
// @param handler Handler function.
void set_handler(Dispatcher* this, Command command, HandlerFunc handler);

// @function get_handler
// @param this Dispatcher.
// @param command Command to look up.
// @returns HandlerFunc Handler or NULL.
HandlerFunc get_handler(Dispatcher* this, Command command);

// @function CliMain
// @param argc Argument count.
// @param argv Argument vector.
// @returns int Exit code.
int CliMain(int argc, char** argv);

// @function perft_handler
// @param context CLI context.
// @returns bool Success.
bool perft_handler(Context context);

// @function eval_handler
// @param context CLI context.
// @returns bool Success.
bool eval_handler(Context context);

// @function eval_batch_handler
// @param context CLI context.
// @returns bool Success.
bool eval_batch_handler(Context context);

// @function eval_breakdown_batch_handler
// @param context CLI context.
// @returns bool Success.
bool eval_breakdown_batch_handler(Context context);

// @function search_handler
// @param context CLI context.
// @returns bool Success.
bool search_handler(Context context);

// @function help
// @param command Command to show help for (or COMMAND_NONE for general).
void help(Command command);

#endif // CLI_CONFIG_H
