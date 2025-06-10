#include "uci.h"
#include <stdarg.h>

void LogPrintf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    // Print to stdout
    va_list copy1;
    va_copy(copy1, args);
    vfprintf(stdout, format, copy1);
    va_end(copy1);

    // Print to original_stdout if available
    if (original_stdout) {
        va_list copy2;
        va_copy(copy2, args);
        vfprintf(original_stdout, format, copy2);
        va_end(copy2);
    }

    // Print to debug_file if available
    // if (debug_file) {
    //     va_list copy3;
    //     va_copy(copy3, args);
    //     vfprintf(debug_file, format, copy3);
    //     va_end(copy3);
    // }

    va_end(args);

    fflush(stdout);
    if (original_stdout) fflush(original_stdout);
    if (debug_file) fflush(debug_file);
}
