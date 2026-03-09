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
    INDENT("perft                   Run PERFT on a specific fen");
    INDENT("eval                    Run an evaluation on a position");
    INDENT("eval-batch              Texel: read params + FENs from stdin, output material+PST scores");
    INDENT("eval-breakdown-batch    Read N and N FENs from stdin, output N lines of 8 term values");
    INDENT("search                  Find the best move in a position");
    PRINT("");

    HEADER("OPTIONS");
    INDENT("-h --help               Print this message and exit");
    INDENT("-F --fen <FEN>          Specify the FEN string");
    INDENT("-d --depth <DEPTH>      Specify the depth");
    INDENT("-E --eval <EVAL>        Specify the evaluation function to use");
    INDENT("-S --search <SEARCH>    Specify the search function to use");
    PRINT("");

    HEADER("EVALUATION FUNCTIONS");
    INDENT("material");
    INDENT("pesto");
    PRINT("");

    HEADER("SEARCH FUNCTIONS");
    INDENT("alpha-beta");
    INDENT("quiescence-iterative-deepening");
    PRINT("");

    PRINT("Written by %s", engine.author);
}

void perft_help()
{
    PRINT("%s perft [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help               Print this message and exit");
    INDENT("-F --fen <FEN>          Specify the FEN string");
    INDENT("-d --depth <DEPTH>      Specify the depth");
    INDENT("-E --eval <EVAL>        Specify the evaluation function to use");
}

void eval_help()
{
    PRINT("%s eval [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help               Print this message and exit");
    INDENT("-F --fen <FEN>          Specify the FEN string");
    INDENT("-E --eval <EVAL>        Specify the evaluation function to use");
}

void eval_batch_help()
{
    PRINT("%s eval-batch", engine.name);
    PRINT("Read from stdin: repeat [ N ; mg0 mg1 mg2 mg3 mg4 eg0 eg1 eg2 eg3 eg4 ; N FEN lines ].");
    PRINT("Output: N centipawn scores (white perspective) per batch, one per line.");
}

void eval_breakdown_batch_help()
{
    PRINT("%s eval-breakdown-batch", engine.name);
    PRINT("Read from stdin: N (count), then N FEN lines.");
    PRINT("Output: N lines of 8 integers (material_pst pawn_structure mobility king_safety piece_activity space threats endgame), white perspective.");
}

void search_help()
{
    PRINT("%s search [<OPTIONS>...]", engine.name);
    PRINT("");
    HEADER("OPTIONS");
    INDENT("-h --help               Print this message and exit");
    INDENT("-F --fen <FEN>          Specify the FEN string");
    INDENT("-d --depth <DEPTH>      Specify the depth");
    INDENT("-E --eval <EVAL>        Specify the evaluation function to use");
}

void help(Command command) {
    switch (command) {
    case COMMAND_PERFT:
        perft_help();
        break;
    case COMMAND_EVAL:
        eval_help();
        break;
    case COMMAND_EVAL_BATCH:
        eval_batch_help();
        break;
    case COMMAND_EVAL_BREAKDOWN_BATCH:
        eval_breakdown_batch_help();
        break;
    case COMMAND_SEARCH:
        search_help();
        break;
    default:
        default_help();
        break;
    }
}
