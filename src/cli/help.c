#include "IncludeOnly/ansi.h"
#include "cli.h"
#include "core.h"
#include <stdio.h>

#define PRINT(format, ...) \
    do { \
        fprintf(stdout, format, ##__VA_ARGS__); \
        fprintf(stdout, "\n"); \
    } while(0)

#define HEADER(format, ...) \
    do { \
        fprintf(stdout, ANSI_BOLD""format""ANSI_RESET, ##__VA_ARGS__); \
        fprintf(stdout, "\n"); \
    } while(0)

#define INDENT(format, ...) \
    do { \
        PRINT("  "format, ##__VA_ARGS__); \
    } while(0)

void default_help()
{
    PRINT("%s <COMMAND> [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("COMMANDS");
    INDENT("perft           Run PERFT on a specific fen");
    INDENT("eval            Run an evaluation on a position");
    INDENT("search          Find the best move in a position");
    PRINT("");

    HEADER("OPTIONS");
    INDENT("-h --help       Print this message and exit");
    INDENT("-v --version    Print the version and exit");
    INDENT("-F --fen        Specify the FEN string");
    INDENT("-d --depth      Specify the depth");
    PRINT("");

    PRINT("Written by %s", engine.author);
}

void perft_help()
{
    PRINT("%s perft [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help       Print this message and exit");
    INDENT("-F --fen        Specify the FEN string");
    INDENT("-d --depth      Specify the depth");
}

void eval_help()
{
    PRINT("%s eval [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help       Print this message and exit");
    INDENT("-F --fen        Specify the FEN string");
}

void search_help()
{
    PRINT("%s search [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help       Print this message and exit");
    INDENT("-F --fen        Specify the FEN string");
    INDENT("-d --depth      Specify the depth");
}

void help(Command command) {
    switch (command) {
    case COMMAND_PERFT:
        perft_help();
        break;
    case COMMAND_EVAL:
        eval_help();
        break;
    case COMMAND_SEARCH:
        search_help();
        break;
    default:
        default_help();
        break;
    }
}
