/*
 * Theory: UCI main loop.
 *
 * After initialization we print the engine name and start the search thread.
 * We then read lines from stdin in a loop. Each line is passed to the command
 * handler (uci, isready, position, go, stop, etc.). The loop exits on EOF or
 * when the quit command sets quitRequested. All output is locked so the search
 * thread and main thread do not interleave prints.
 */
#include "uci.h"
#include "version.h"
#include <stdio.h>

int UciMain()
{
    char input[1024];

    InitState(&uci_state);
    LoadUciConfig(&uci_state);

    uci_stdout_lock();
    printf("%s v%s by %s\n", engine.name, VERSION_STRING, engine.author);
    fflush(stdout);
    uci_stdout_unlock();

    uci_search_thread_start();

    while (!uci_state.quitRequested) {

        if (!fgets(input, sizeof(input), stdin))
            break;  /* Exit on EOF */

        input[strcspn(input, "\n")] = '\0';

        uci_stdout_lock();
        HandleCommand(&uci_state, input);
        fflush(stdout);
        uci_stdout_unlock();
    }

    uci_search_thread_join();
    return 0;
}
