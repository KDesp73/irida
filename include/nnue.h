#ifndef NNUE_H
#define NNUE_H

#include "castro.h"
#include <stdbool.h>

/* Load NNUE network from file. Returns true on success.
 * If no NNUE backend is linked, returns false and eval falls back to PeSTO. */
bool nnue_load(const char* path);

/* Whether an NNUE net is loaded and ready. */
bool nnue_available(void);

#endif
