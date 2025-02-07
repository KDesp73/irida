#ifndef ENGINE_UCI_H
#define ENGINE_UCI_H

#include "board.h"
#include "perft.h"
#include "extern/logging.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define INLINE static inline 

typedef enum {
    UCI_CHECK,   // Checkbox option (true/false)
    UCI_SPIN,    // Integer spin (range of values)
    UCI_COMBO,   // Dropdown menu of values
    UCI_STRING   // Free-text input
} UciOptionType;

typedef struct {
    char name[64];                 // Name of the option
    UciOptionType type;            // Type of the option
    union {
        bool check;                // Value for UCI_CHECK
        int spin;                  // Value for UCI_SPIN
        char combo[128];           // Value for UCI_COMBO
        char string[128];          // Value for UCI_STRING
    } value;                       // Current value of the option
    union {
        struct { int min, max; };  // Range for UCI_SPIN
        char combos[10][64];       // List of values for UCI_COMBO
    } params;                      // Parameters for the option (if applicable)
    char default_value[128];       // Default value (for resetting)
} UciOption;

void PrintUciOptions();
void InitUciOptions();

typedef struct {
    char startPositionFen[128];    // The starting position in FEN notation
    bool uciMode;                  // Whether the engine is currently running in UCI mode
    bool debugMode;                // Debug mode
    int depthLimit;                // Search depth limit for the engine
    int timeLeft[2];               // Remaining time for each player (milliseconds) [0] = white, [1] = black
    int increment[2];              // Increment per move for each player (milliseconds) [0] = white, [1] = black
    int movesToGo;                 // Moves to the next time control, if applicable
    bool ponderMode;               // Whether ponder mode is enabled
    bool infiniteMode;             // Whether infinite search mode is enabled
    bool stopRequested;            // Whether a stop command has been received

    UciOption uciOptions[10];      // Array of UCI options
    size_t uciOptionCount;         // Number of UCI options available
    char lastCommand[128];         // Stores the last command received 
    
    Board board;
} State;

static State state = {0};

INLINE void StateSetStartPos(const char* startpos)
{
    strncpy(state.startPositionFen, startpos, sizeof(state.startPositionFen) - 1);
    state.startPositionFen[sizeof(state.startPositionFen) - 1] = '\0'; // Null-terminate
    BoardInitFen(&state.board, state.startPositionFen);
}

INLINE void InitState()
{
    InitUciOptions();
    memset(&state, 0, sizeof(state)); // Zero out state
    StateSetStartPos(STARTING_FEN);
    state.depthLimit = 10;
}

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
#define COMMAND_DEBUG "debug"
#define COMMAND_DISPLAY "d"

#define FLUSH fflush(stdout)

int UciMain(int argc, char** argv);

void setoption(const char *command);
void go(const char* command);
void position(const char* command);

INLINE void uci()
{
    state.uciMode = true;
    printf("id name %s\n", ENGINE_NAME);
    printf("id author %s\n", ENGINE_AUTHOR);

    printf("\n");

    PrintUciOptions();
    printf("uciok\n");
}

INLINE void isready()
{
    printf("readyok\n");
}

INLINE void ucinewgame()
{
    InitState();
    printf("info New game started.\n");
}

INLINE void stop()
{
    // TODO: Handle stop command if a calculation is running
    state.stopRequested = true;
    printf("info Calculation stopped.\n");
}

INLINE void quit()
{
    BoardFree(&state.board);
    exit(0);
}

INLINE void debug(const char* command)
{
    state.debugMode = strcmp(command + strlen(COMMAND_DEBUG), "on") == 0;
}

INLINE void display()
{
    BoardPrint(&state.board, 64);
}

#define IS_COMMAND(command, check) \
    (strncmp(command, check, strlen(check)) == 0 && \
    (command[strlen(check)] == '\0' || command[strlen(check)] == ' '))

INLINE void HandleCommand(const char *command)
{
    if (IS_COMMAND(command, COMMAND_UCI)) {
        uci();
    } else if (IS_COMMAND(command, COMMAND_ISREADY)) {
        isready();
    } else if (IS_COMMAND(command, COMMAND_UCINEWGAME)) {
        ucinewgame();
    } else if (IS_COMMAND(command, COMMAND_POSITION)) {
        position(command);
    } else if (IS_COMMAND(command, COMMAND_GO)) {
        go(command);
    } else if (IS_COMMAND(command, COMMAND_STOP)) {
        stop();
    } else if (IS_COMMAND(command, COMMAND_QUIT)) {
        quit();
    } else if (IS_COMMAND(command, COMMAND_SETOPTION)) {
        setoption(command);
    } else if (IS_COMMAND(command, COMMAND_DEBUG)) {
        debug(command);
    } else if (IS_COMMAND(command, COMMAND_DISPLAY)) {
        display();
    } else {
        printf("info string Unknown command: %s\n", command);
    }
    FLUSH;
}

#endif // ENGINE_UCI_H
