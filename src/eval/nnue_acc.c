/*
 * Optional NNUE accumulator (nnue-probe incremental) for search.
 * Used only when SearchConfig.useNNUEAccumulator and eval is NNUE; search must
 * call the Begin/BeforeChild/Commit/Pop/End and null-move hooks in lockstep
 * with the board. Public eval entry remains irida_EvalNNUE(Board*).
 */
#include "castro.h"
#include "nnue-probe/nnue.h"
#include <assert.h>
#include <stdalign.h>
#include <stdbool.h>
#include <string.h>
#ifndef MAX_PLY
#define MAX_PLY 128
#endif

extern int nnue_evaluate(int player, int* pieces, int* squares);
extern int nnue_evaluate_incremental(int player, int* pieces, int* squares, NNUEdata** nnue);

/* nnue.h piece codes: wk..wp = 1..6, bk..bp = 7..12 */
enum {
  NN_WK = 1, NN_WQ, NN_WR, NN_WB, NN_WN, NN_WP,
  NN_BK = 7, NN_BQ, NN_BR, NN_BB, NN_BN, NN_BP
};

#define NN_SQ_NONE 64
#define MAX_NNUE_PIECES 32

/* Stack depth from root of subtree: height 0 = position at SearchBegin, +1 per MakeMove, etc. */
#define ACC_STACK  (MAX_PLY + 4)

static alignas(64) NNUEdata g_stack[ACC_STACK];
static int g_height; /* index of current node in g_stack: 0 after SearchBegin, ++ on Commit */
static int g_nmp;    /* >0: inside null-move subtree, use full nnue_evaluate */
static bool g_active; /* acc session active (between SearchBegin and SearchEnd) */

static int piece_char_to_nnue(char c)
{
  switch (c) {
  case 'K': return NN_WK;
  case 'Q': return NN_WQ;
  case 'R': return NN_WR;
  case 'B': return NN_WB;
  case 'N': return NN_WN;
  case 'P': return NN_WP;
  case 'k': return NN_BK;
  case 'q': return NN_BQ;
  case 'r': return NN_BR;
  case 'b': return NN_BB;
  case 'n': return NN_BN;
  case 'p': return NN_BP;
  default:  return 0;
  }
}

void irida_NNUEBuildPsq(const Board* board, int* pieces, int* squares)
{
  int out = 2;
  Square wk = SQUARE_NONE, bk = SQUARE_NONE;

  for (int sq = 0; sq < 64; sq++) {
    char c = board->grid[sq / 8][sq % 8];
    if (c == 'K')  wk = (Square)sq;
    else if (c == 'k') bk = (Square)sq;
  }

  pieces[0] = NN_WK;
  squares[0] = wk;
  pieces[1] = NN_BK;
  squares[1] = bk;

  for (int sq = 0; sq < 64; sq++) {
    char c = board->grid[sq /8][sq % 8];
    if (c == ' ' || c == EMPTY_SQUARE) continue;
    if (c == 'K' || c == 'k') continue;
    if (out >= MAX_NNUE_PIECES)
      break;
    int pc = piece_char_to_nnue(c);
    if (pc)
    {
      pieces[out]  = pc;
      squares[out] = sq;
      out++;
    }
  }
  pieces[out]  = 0;
  squares[out] = 0;
}

int irida_NNUEEvalWithPsq(const Board* board)
{
  int pieces[MAX_NNUE_PIECES + 2];
  int squares[MAX_NNUE_PIECES + 2];
  irida_NNUEBuildPsq(board, pieces, squares);
  int player = board->turn ? 0 : 1;
  return nnue_evaluate(player, pieces, squares);
}

/* Stockfish-style DirtyPiece: see nnue-probe half_kp_append_changed_indices. */
static void irida_fill_dirty(const Board* parent, Move move, DirtyPiece* dp)
{
  memset(dp, 0, sizeof(*dp));
  Square from, to;
  uint8_t promotion = 0;
  uint8_t flag = 0;
  castro_MoveDecode(move, &from, &to, &promotion, &flag);
  (void)promotion;
  char pc_char = parent->grid[from / 8][from % 8];
  int pc = piece_char_to_nnue(pc_char);
  if (!pc) {
    dp->dirtyNum  = 1;
    dp->pc[0]     = 0;
    dp->from[0]   = 0;
    dp->to[0]     = 0;
    return;
  }

  Move m2 = move;
  if (castro_IsCastle(parent, &m2) || flag == FLAG_CASTLING) {
    int d = (int)to - (int)from;
    int rfrom, rto;
    if (d == 2) { /* O-O: rook from file h */
      rfrom = (int)to + 1;
      rto   = (int)to - 1;
    } else { /* O-O-O */
      rfrom = (int)to - 2;
      rto   = (int)to + 1;
    }
    int rk = parent->turn ? NN_WR : NN_BR;
    dp->dirtyNum = 2;
    dp->pc[0]  = parent->turn ? NN_WK : NN_BK;
    dp->from[0] = (int)from;
    dp->to[0]   = (int)to;
    dp->pc[1]  = rk;
    dp->from[1] = rfrom;
    dp->to[1]   = rto;
    return;
  }

  Undo u = castro_MakeUndo(parent, move);

  bool is_ep = (flag == FLAG_ENPASSANT) || castro_IsEnpassant(parent, &m2);
  if (u.captured != 0) {
    int cap = piece_char_to_nnue((char)u.captured);
    if (!cap) cap = NN_BP; /* should not happen */
    Square capsq = to;
    if (is_ep) {
      /* Captured pawn on square behind 'to' */
      if (parent->turn)
        capsq = (Square)(to - 8);
      else
        capsq = (Square)(to + 8);
    }
    dp->dirtyNum  = 2;
    dp->pc[0]  = pc;
    dp->from[0] = (int)from;
    dp->to[0]   = (int)to;
    dp->pc[1]  = cap;
    dp->from[1] = (int)capsq;
    dp->to[1]   = NN_SQ_NONE;
  } else {
    dp->dirtyNum  = 1;
    dp->pc[0]  = pc;
    dp->from[0] = (int)from;
    dp->to[0]   = (int)to;
  }

  if (flag == FLAG_PROMOTION || flag == FLAG_PROMOTION_WITH_CAPTURE) {
    int prom;
    uint8_t pt = castro_GetPromotion(move);
    if (parent->turn) {
      if (pt == PROMOTION_KNIGHT) prom = NN_WN;
      else if (pt == PROMOTION_BISHOP) prom = NN_WB;
      else if (pt == PROMOTION_ROOK) prom = NN_WR;
      else  prom = NN_WQ;
    } else {
      if (pt == PROMOTION_KNIGHT) prom = NN_BN;
      else if (pt == PROMOTION_BISHOP) prom = NN_BB;
      else if (pt == PROMOTION_ROOK) prom = NN_BR;
      else  prom = NN_BQ;
    }
    if (u.captured != 0) {
      dp->to[0]    = NN_SQ_NONE;
      dp->pc[2]   = prom;
      dp->from[2] = NN_SQ_NONE;
      dp->to[2]   = (int)to;
      dp->dirtyNum = 3;
    } else {
      dp->to[0]    = NN_SQ_NONE;
      dp->pc[1]   = prom;
      dp->from[1] = NN_SQ_NONE;
      dp->to[1]   = (int)to;
      dp->dirtyNum = 2;
    }
  }
}

void irida_NNUEAccSearchBegin(Board* board)
{
  g_active  = true;
  g_height  = 0;
  g_nmp     = 0;
  memset(g_stack, 0, sizeof(g_stack));
  g_stack[0].accumulator.computedAccumulation = 0;
  memset(&g_stack[0].dirtyPiece, 0, sizeof(g_stack[0].dirtyPiece));

  int ps[MAX_NNUE_PIECES + 2];
  int sqa[MAX_NNUE_PIECES + 2];
  irida_NNUEBuildPsq(board, ps, sqa);
  int player = board->turn ? 0 : 1;
  NNUEdata* nn[3] = { &g_stack[0], NULL, NULL };
  (void)nnue_evaluate_incremental(player, ps, sqa, nn);
}

void irida_NNUEAccSearchEnd(void)
{
  g_active = false;
  g_height = 0;
  g_nmp    = 0;
}

void irida_NNUEAccBeforeChild(const Board* parent, Move move)
{
  int ch = g_height + 1;
  if (ch >= ACC_STACK) {
    assert(0 && "NNUE acc stack overflow");
    return;
  }
  memset(&g_stack[ch], 0, sizeof(g_stack[ch]));
  g_stack[ch].accumulator.computedAccumulation = 0;
  irida_fill_dirty(parent, move, &g_stack[ch].dirtyPiece);
}

void irida_NNUEAccCommitChild(void) { g_height++; }

void irida_NNUEAccPop(void)
{
  if (g_height > 0) g_height--;
}

void irida_NNUENullMoveEnter(void) { g_nmp++; }

void irida_NNUENullMoveExit(void) { if (g_nmp > 0) g_nmp--; }

bool irida_NNUEAccShouldUseIncremental(void) { return g_active && (g_nmp == 0); }

bool irida_NNUEAccSessionActive(void) { return g_active; }

bool irida_NNUEAccInNullMove(void) { return g_nmp > 0; }

int irida_NNUEEvalIncrementalForBoard(const Board* board)
{
  int ps[MAX_NNUE_PIECES + 2];
  int sqp[MAX_NNUE_PIECES + 2];
  irida_NNUEBuildPsq(board, ps, sqp);
  int player = board->turn ? 0 : 1;
  NNUEdata* nn[3] = { NULL, NULL, NULL };
  if (g_height < 0 || g_height >= ACC_STACK)
    return nnue_evaluate(player, ps, sqp);
  nn[0] = &g_stack[g_height];
  nn[1] = (g_height >= 1) ? &g_stack[g_height - 1] : NULL;
  nn[2] = (g_height >= 2) ? &g_stack[g_height - 2] : NULL;
  return nnue_evaluate_incremental(player, ps, sqp, nn);
}
