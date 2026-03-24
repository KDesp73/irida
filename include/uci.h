#ifndef ENGINE_UCI_H
#define ENGINE_UCI_H

#include "core.h"
#define CASTRO_STRIP_PREFIX
#include "castro.h"
#include <stdbool.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

// @module uci
// @desc UCI protocol: option types, state, commands, handlers, search thread sync.

// @enum UciOptionType
// @desc Option kind: check, spin, combo, string.
typedef enum {
    UCI_CHECK,   // Checkbox option (true/false)
    UCI_SPIN,    // Integer spin (range of values)
    UCI_COMBO,   // Dropdown menu of values
    UCI_STRING   // Free-text input
} UciOptionType;

// @var UciOptionTypeString
// @desc String names for UciOptionType (for UCI output).
static char* UciOptionTypeString[] = {
    "check",
    "spin",
    "combo",
    "string"
};

// @enum UciStatus
// @desc Engine state: waiting, thinking, stopped, ready, quit, pondering, etc.
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

// @struct UciOption
// @desc Single UCI option: name, type, value (check/spin/combo/string), params, default.
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


// @const MAX_UCI_OPTIONS
// @desc Max number of UCI options.
#define MAX_UCI_OPTIONS 32

// @struct UciState
// @desc Full UCI state: start FEN, mode flags, time, options, status, last command.
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
    bool stopRequested;            // Whether a stop command has been received (tells search to stop)
    bool quitRequested;            // Whether to exit the UCI loop (set by "quit" only)
    UciStatus status;

    UciOption uciOptions[MAX_UCI_OPTIONS]; // Array of UCI options
    size_t uciOptionCount;         // Number of UCI options available
    char lastCommand[128];         // Stores the last command received 
} UciState;

// @var uci_state
// @desc Global UCI state.
extern UciState uci_state;

// @function LoadUciConfig
// @param state UciState to load into.
void LoadUciConfig(UciState* state);

// @function PrintUciOptions
// @param state UciState with options.
void PrintUciOptions(UciState* state);

// @function StatePrint
// @param state UciState to print.
void StatePrint(const UciState* state);

// @function GetUciOption
// @param state UciState.
// @param name Option name.
// @param opt Output option (if found).
// @returns bool True if option found.
bool GetUciOption(const UciState* state, char* name, UciOption* opt);

#define COMMAND_DEBUG      "debug"
#define COMMAND_GO         "go"
#define COMMAND_ISREADY    "isready"
#define COMMAND_POSITION   "position"
#define COMMAND_QUIT       "quit"
#define COMMAND_SETOPTION  "setoption"
#define COMMAND_STOP       "stop"
#define COMMAND_UCI        "uci"
#define COMMAND_UCINEWGAME "ucinewgame"

#define COMMAND_DISPLAY    "d"
#define COMMAND_SETEVAL    "seteval"
#define COMMAND_SETSEARCH  "setsearch"

int UciMain(void);

/** Run the UCI command loop only (state and config must already be initialized). */
int UciMainLoop(void);

bool UciHandleCommand(UciState* state, const char *command);

void uci_debug(UciState* state, const char* command);
void uci_go(UciState* state, const char* command);
void uci_isready(UciState* state);
void uci_position(UciState* state, const char* command);
void uci_quit(UciState* state);
void uci_setoption(UciState* state, const char *command);
void uci_stop(UciState* state);
void uci_uci(UciState* state);
void uci_ucinewgame(UciState* state);

// Custom commands
void uci_display(UciState* state);
void uci_seteval(UciState* state, const char* command);
void uci_setsearch(UciState* state, const char* command);


// @function uci_stdout_lock
// @desc Lock before writing to stdout from any thread.
void uci_stdout_lock(void);

// @function uci_stdout_unlock
void uci_stdout_unlock(void);

// @function uci_search_wait_done
// @desc Block until the search thread is idle.
void uci_search_wait_done(void);

// @function uci_search_start
// @desc Signal the search thread to run search (called from uci_go).
void uci_search_start(void);

// @function uci_search_thread_start
// @desc Start the search worker thread. Call once from UciMain before the command loop.
void uci_search_thread_start(void);

// @function uci_search_thread_join
// @desc Ask the search thread to exit and wait for it. Call before process exit.
void uci_search_thread_join(void);

// @function uci_report_search
// @desc UCI-compatible search process logging
void uci_report_search(int depth, int bestScore, uint64_t nodes, uint64_t timeMs, const char* pvBuf);

// @function StateSetStartPos
// @desc Set start position FEN and init board.
// @param state UciState.
// @param startpos FEN string (e.g. "startpos" or full FEN).
static inline void StateSetStartPos(UciState* state, const char* startpos)
{
    strncpy(state->startPositionFen, startpos, sizeof(state->startPositionFen) - 1);
    state->startPositionFen[sizeof(state->startPositionFen) - 1] = '\0'; // Null-terminate
    castro_BoardInitFen(&engine.board, state->startPositionFen);
}

// @function InitState
// @desc Initialize state with starting position.
// @param state UciState.
static inline void InitState(UciState* state)
{
    StateSetStartPos(state, STARTING_FEN);
}

static inline bool search_should_stop(void)
{
    return search_time_up() || uci_state.stopRequested;
}

#endif // ENGINE_UCI_H
