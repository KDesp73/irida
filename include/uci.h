#ifndef ENGINE_UCI_H
#define ENGINE_UCI_H

#include "board.h"
#define INLINE static inline 

#define ANSI_IMPLEMENTATION
#include <io/ansi.h>
#include <stdio.h>
#include <string.h>

#define ENGINE_NAME "chess-engine"
#define ENGINE_AUTHOR "KDesp73"

#define COMMAND_UCI "uci"
#define COMMAND_ISREADY "isready"
#define COMMAND_SETOPTION "setoption"
#define COMMAND_UCINEWGAME "ucinewgame"
#define COMMAND_POSITION "position"
#define COMMAND_GO "go"
#define COMMAND_STOP "stop"
#define COMMAND_QUIT "quit"

#define FLUSH fflush(stdout)

int UciMain(int argc, char** argv);

INLINE void uci()
{
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", ENGINE_AUTHOR);
    printf("uciok\n");
    FLUSH;
}


INLINE void isready()
{
    printf("readyok\n");
    FLUSH;
}

INLINE void ucinewgame()
{
    // TODO: reset engine state for a new game
    printf("info New game started.\n");
    FLUSH;
}

INLINE void position(const char* command)
{
    if(!strcmp(command, COMMAND_POSITION)) goto quit;
    
    char fen[128];
    if (!strcmp(command, "startpos")){
        strcpy(fen, STARTING_FEN);
    } else if(!strcmp(command, "fen")){
        strcpy(fen, command);
    }

quit:
    FLUSH;
}

INLINE void go()
{
    // TODO: Calculate best move
    printf("bestmove e2e4\n");
    FLUSH;
}

INLINE void stop()
{
    // TODO: handle stop command in case calculation is running
    printf("info Calculation stopped.\n");
    FLUSH;
}

INLINE void quit()
{
    printf("info Quitting...\n");
    FLUSH;
    exit(0);
}

INLINE void HandleCommand(const char *command)
{
    if (strcmp(command, COMMAND_UCI) == 0) {
        uci();
    } else if (strcmp(command, COMMAND_ISREADY) == 0) {
        isready();
    } else if (strcmp(command, COMMAND_UCINEWGAME) == 0) {
        ucinewgame();
    } else if (strncmp(command, COMMAND_POSITION, 8) == 0) {
        position(command);
    } else if (strcmp(command, COMMAND_GO) == 0) {
        go();
    } else if (strcmp(command, COMMAND_STOP) == 0) {
        stop();
    } else if (strcmp(command, COMMAND_QUIT) == 0) {
        quit();
    } else {
        printf("Unknown command: %s\n", command);
        fflush(stdout);
    }
}

#endif // ENGINE_UCI_H
