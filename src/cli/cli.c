/*
 * Theory: CLI command parsing and context.
 *
 * The CLI accepts a command name (perft, eval, eval-batch, search, etc.) and
 * optional arguments. parse_command maps a string to a Command enum. context_init
 * and context_reset prepare a Context that holds the command, FEN, depth, eval
 * type, and search limits. The rest of the CLI uses this context to run the
 * requested operation (e.g. perft_handler, eval_handler, search_handler).
 */
#include "cli.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Command parse_command(const char* str)
{
    if(!str) return COMMAND_NONE;

    if(!strcmp("perft", str)) return COMMAND_PERFT;
    else if(!strcmp("eval", str)) return COMMAND_EVAL;
    else if(!strcmp("eval-batch", str)) return COMMAND_EVAL_BATCH;
    else if(!strcmp("eval-breakdown-batch", str)) return COMMAND_EVAL_BREAKDOWN_BATCH;
    else if(!strcmp("search", str)) return COMMAND_SEARCH;

    return COMMAND_NONE;
}

void context_init(Context* ctx, Command command)
{
    context_reset(ctx);
    ctx->command = command;
    ctx->fen = NULL;
    ctx->eval = NULL;
    ctx->search = NULL;
    ctx->nnue_path = NULL;
}

void context_reset(Context* ctx)
{
    context_free(ctx);
    ctx->command = COMMAND_NONE;
    ctx->fen = NULL;
    ctx->eval = NULL;
    ctx->search = NULL;
    ctx->nnue_path = NULL;
}

void context_free(Context* ctx)
{
    if(!ctx) return;
    if(ctx->fen) free(ctx->fen);
    if(ctx->eval) free(ctx->eval);
    if(ctx->search) free(ctx->search);
    if(ctx->nnue_path) free(ctx->nnue_path);
}

void set_handler(Dispatcher* this, Command command, HandlerFunc handler)
{
    this->table[command] = handler;
}

HandlerFunc get_handler(Dispatcher* this, Command command)
{
    if(command == COMMAND_NONE) return NULL;
    return this->table[command];
}
