#ifndef ENGINE_UCI_H
#define ENGINE_UCI_H

#include "board.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    UCI_CHECK,   // Checkbox option (true/false)
    UCI_SPIN,    // Integer spin (range of values)
    UCI_COMBO,   // Dropdown menu of values
    UCI_STRING   // Free-text input
} UciOptionType;

typedef enum {
    UCI_WAITING,         // Idle state, waiting for a command
    UCI_START_THINKING,  // Received "go" command, starting search
    UCI_THINKING,        // Actively searching for the best move
    UCI_STOPPED,         // Search stopped (due to "stop" or "ponderhit")
    UCI_READY,           // Ready for new commands (after "isready" response)
    UCI_QUIT,            // Engine is shutting down
    UCI_PONDERING,       // Thinking in opponent’s time (after "ponder" move)
    UCI_SETUP_POSITION,  // Processing "position" command
    UCI_BESTMOVE_SENT    // Best move has been determined and sent to GUI
} UciStatus;

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


#define MAX_UCI_OPTIONS 32
typedef struct {
    char startPositionFen[128];    // The starting position in FEN notation
    bool uciMode;                  // Whether the engine is currently running in UCI mode
    bool debugMode;                // Debug mode
    int depthLimit;                // Search depth limit for the engine
    int timeLeft[2];               // Remaining time for each player (milliseconds) [0] = white, [1] = black
    int increment[2];              // Increment per move for each player (milliseconds) [0] = white, [1] = black
    int maxBookmoves;              // Max number of bookmoves to play
    int movesToGo;                 // Moves to the next time control, if applicable
    bool ponderMode;               // Whether ponder mode is enabled
    bool infiniteMode;             // Whether infinite search mode is enabled
    bool stopRequested;            // Whether a stop command has been received
    UciStatus status;

    UciOption uciOptions[MAX_UCI_OPTIONS]; // Array of UCI options
    size_t uciOptionCount;         // Number of UCI options available
    char lastCommand[128];         // Stores the last command received 
    
    Board board;
} State;

#define UCI_CONFIG_SCRIPT "config.lua"
#define UCI_TUNING_SCRIPT "tuning.lua"

void LoadUciConfig(State* state);
void PrintUciOptions(State* state);
void StatePrint(const State* state);
bool GetUciOption(const State* state, char* name, UciOption* opt);

extern FILE* in_debug_file;
extern FILE* out_debug_file;
extern FILE* original_stdout;
void LogPrintf(const char* format, ...);


#define ENGINE_NAME   "chess-engine"
#define ENGINE_AUTHOR "KDesp73"

#define COMMAND_DEBUG      "debug"
#define COMMAND_DISPLAY    "d"
#define COMMAND_GO         "go"
#define COMMAND_ISREADY    "isready"
#define COMMAND_POSITION   "position"
#define COMMAND_QUIT       "quit"
#define COMMAND_SETOPTION  "setoption"
#define COMMAND_STOP       "stop"
#define COMMAND_UCI        "uci"
#define COMMAND_UCINEWGAME "ucinewgame"

int UciMain(int argc, char** argv);
bool HandleCommand(State* state, const char *command);

void uci_debug(State* state, const char* command);
void uci_display(State* state);
void uci_go(State* state, const char* command);
void uci_isready(State* state);
void uci_position(State* state, const char* command);
void uci_quit(State* state);
void uci_setoption(State* state, const char *command);
void uci_stop(State* state);
void uci_uci(State* state);
void uci_ucinewgame(State* state);

static inline void StateSetStartPos(State* state, const char* startpos)
{
    strncpy(state->startPositionFen, startpos, sizeof(state->startPositionFen) - 1);
    state->startPositionFen[sizeof(state->startPositionFen) - 1] = '\0'; // Null-terminate
    BoardInitFen(&state->board, state->startPositionFen);
}

static inline void InitState(State* state)
{
    StateSetStartPos(state, STARTING_FEN);
}

#endif // ENGINE_UCI_H
