#ifndef CLI_CONFIG_H
#define CLI_CONFIG_H

#include <stdbool.h>
#include <stddef.h>

typedef enum {
    COMMAND_PERFT,
    COMMAND_EVAL,
    COMMAND_EVAL_BATCH,
    COMMAND_EVAL_BREAKDOWN_BATCH,
    COMMAND_SEARCH,
    COMMAND_NONE,
} Command;
#define COMMAND_COUNT COMMAND_NONE
Command parse_command(const char* str);

typedef enum {
   ARG_HELP = 'h',
   ARG_VERSION = 'v',
   ARG_FEN = 'F',
   ARG_DEPTH = 'd',
   ARG_EVAL = 'E',
   ARG_SEARCH = 'S',
   ARG_NNUE_PATH = 'N'
} CliArgs;

typedef struct {
    char* fen;
    char* eval;
    char* search;
    char* nnue_path;
    size_t depth;
    Command command;
} Context;
void context_init(Context* ctx, Command command);
void context_reset(Context* ctx);
void context_free(Context* ctx);

typedef bool (*HandlerFunc)(Context);
typedef struct {
   HandlerFunc table[COMMAND_COUNT];
} Dispatcher;
void set_handler(Dispatcher* this, Command command, HandlerFunc handler);
HandlerFunc get_handler(Dispatcher* this, Command command);

int CliMain(int argc, char** argv);

bool perft_handler(Context context);
bool eval_handler(Context context);
bool eval_batch_handler(Context context);
bool eval_breakdown_batch_handler(Context context);
bool search_handler(Context context);

void help(Command command);

#endif // CLI_CONFIG_H
