#include "uci.h"
#include "uci_thread.h"
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
