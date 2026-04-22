/* Parity: FEN / full PSQ vs optional incremental path (optional net: skip if no file). */
int test_nnue_accumulator(int _unused);

#define TAG "nnue_acc"
#include "IncludeOnly/test.h"
#include "castro.h"
#include "eval.h"
#include "nnue.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int eval_nnue_convention(const Board* b, int raw_mover_cp)
{
  if (!b->turn) return -raw_mover_cp;
  return raw_mover_cp;
}

int test_nnue_accumulator(int _unused)
{
  (void)_unused;
  const char* p = getenv("IRIDA_NNUE_TEST");
  if (!p || p[0] == '\0')
    p = "~/.irida/nn/nn-04cf2b4ed1da.nnue";

  if (!irida_NNUELoad(p)) {
    SUCC("skip: could not load NNUE (set IRIDA_NNUE_TEST=path to a .nnue)");
    return 1;
  }

  /* Start: incremental (primed) vs FEN/EvalNNUE */
  Board b;
  castro_BoardInitFen(&b, STARTING_FEN);
  int expect = irida_EvalNNUE(&b);

  irida_NNUEAccSearchBegin(&b);
  int got = irida_NNUEEvalIncrementalForBoard(&b);
  got = eval_nnue_convention(&b, got);
  irida_NNUEAccSearchEnd();

  if (got != expect) {
    FAIL("incremental startpos: got %d, expected %d (FEN/EvalNNUE)", got, expect);
    castro_BoardFree(&b);
    return 0;
  }

  /* one ply: e2e4 */
  Move m = castro_StringToMove("e2e4");
  if (m == NULL_MOVE) {
    FAIL("e2e4");
    castro_BoardFree(&b);
    return 0;
  }
  irida_NNUEAccSearchBegin(&b);
  irida_NNUEAccBeforeChild(&b, m);
  if (!castro_MakeMove(&b, m)) {
    irida_NNUEAccSearchEnd();
    FAIL("MakeMove e2e4");
    castro_BoardFree(&b);
    return 0;
  }
  irida_NNUEAccCommitChild();

  expect     = irida_EvalNNUE(&b);
  int inc_cp = irida_NNUEEvalIncrementalForBoard(&b);
  got = eval_nnue_convention(&b, inc_cp);

  castro_UnmakeMove(&b);
  irida_NNUEAccPop();
  irida_NNUEAccSearchEnd();
  castro_BoardFree(&b);

  if (got != expect) {
    FAIL("incremental after e2e4: got %d, expected %d", got, expect);
    return 0;
  }

  SUCC("NNUE incremental matches FEN/full for startpos and 1.e4");
  return 1;
}
