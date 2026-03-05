#ifndef UCI_THREAD_H
#define UCI_THREAD_H

#include <stdbool.h>

/** Lock before writing to stdout from any thread. */
void uci_stdout_lock(void);
void uci_stdout_unlock(void);

/** Block until the search thread is idle (so e.g. position can safely update the board). */
void uci_search_wait_done(void);

/** Signal the search thread to run search (called from uci_go). */
void uci_search_start(void);

/** Start the search worker thread. Call once from UciMain before the command loop. */
void uci_search_thread_start(void);

/** Ask the search thread to exit and wait for it. Call before process exit. */
void uci_search_thread_join(void);

#endif
