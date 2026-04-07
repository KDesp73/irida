#include "uci.h"
#include "version.h"
#include <stdio.h>
#include <string.h>

int irida_UciMainLoop(void)
{
    char input[1024];

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
        irida_UciHandleCommand(&uci_state, input);
        fflush(stdout);
        uci_stdout_unlock();
    }

    uci_search_thread_join();
    return 0;
}

int irida_UciMain(void)
{
    irida_InitState(&uci_state);
    irida_LoadUciConfig(&uci_state);
    /* Line-buffer stdout so "info" lines appear promptly when stdout is a pipe. */
    setvbuf(stdout, NULL, _IOLBF, 0);
    return irida_UciMainLoop();
}
