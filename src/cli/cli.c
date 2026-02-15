#include "cli.h"
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Command parse_command(const char* str)
{
    if(!str) return COMMAND_NONE;

    if(!strcmp("perft", str)) return COMMAND_PERFT;
    else if(!strcmp("eval", str)) return COMMAND_EVAL;
    else if(!strcmp("search", str)) return COMMAND_SEARCH;

    return COMMAND_NONE;
}

void context_init(Context* ctx, Command command)
{
    context_reset(ctx);
    ctx->command = command;
}

void context_reset(Context* ctx)
{
    context_free(ctx);
    ctx->command = COMMAND_NONE;
    ctx->fen = NULL;
}

void context_free(Context* ctx)
{
    if(!ctx) return;
    if(ctx->fen) free(ctx->fen);
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
