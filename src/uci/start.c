#include "uci.h"
#include "version.h"

int UciMain()
{
    char input[1024];

    InitState(&uci_state);
    LoadUciConfig(&uci_state);

    printf("%s v%s by %s\n", engine.name, VERSION_STRING, engine.author);
    fflush(stdout);

    while (!uci_state.stopRequested) {

        if (!fgets(input, sizeof(input), stdin))
            break;  // Exit on EOF

        input[strcspn(input, "\n")] = '\0';

        HandleCommand(&uci_state, input);

        fflush(stdout);
    }

    return 0;
}
