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
    INDENT("eval-batch              Texel: read N, params (10 or 18 ints: mg,eg or +term weights), N FENs; output scores (white cp)");
    INDENT("eval-breakdown-batch    Read N and N FENs from stdin, output N lines of 8 term values");
    INDENT("search                  Find the best move in a position");
    INDENT("customize               Set eval/search options then start UCI (no rebuild needed)");
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
    INDENT("nnue");
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

void customize_help()
{
    PRINT("%s customize [<OPTIONS>...]", engine.name);
    PRINT("Apply the given eval and search options, then start UCI. No rebuild required.");
    PRINT("");
    HEADER("EVAL");
    INDENT("--eval <EVAL>           material | pesto | nnue (default: pesto)");
    INDENT("--evalfile <PATH>       NNUE file path (same as setoption name EvalFile value <PATH>)");
    INDENT("--nnue-path <PATH>      Alias for --evalfile");
    PRINT("");
    HEADER("SEARCH");
    INDENT("--null-move <BOOL>      true | false");
    INDENT("--lmr <BOOL>            Late move reduction");
    INDENT("--aspiration <BOOL>     Aspiration windows");
    INDENT("--tt <BOOL>             Transposition table");
    INDENT("--quiescence <BOOL>     Quiescence search");
    PRINT("");
    HEADER("SYZYGY / HASH");
    INDENT("--hash <MB>             1..2048");
    INDENT("--syzygy-path <PATH>    Tablebase path");
    INDENT("--syzygy-probe-depth <N>  1..100");
    INDENT("--syzygy-probe-limit <N>  0..7");
    INDENT("--syzygy-50-rule <BOOL> true | false");
    PRINT("");
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
    case COMMAND_CUSTOMIZE:
        customize_help();
        break;
    default:
        default_help();
        break;
    }
}
