#include "uci.h"
#include <io/ansi.h>

int UciMain(int argc, char** argv)
{
    char input[1024];

    printf("Welcome to %s%s%s by %s%s%s\n", ANSI_RED, ENGINE_NAME, ANSI_RESET, ANSI_BLUE, ENGINE_AUTHOR, ANSI_RESET);
    FLUSH;
    
    for(;;) {
        if(fgets(input, sizeof(input), stdin) == NULL) continue; 

        input[strcspn(input, "\n")] = 0;

        HandleCommand(input);
    }

    return 0;
}
