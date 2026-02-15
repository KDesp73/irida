#include "uci.h"
#include <stdarg.h>
#include <stdio.h>

static void log_to_stream(FILE *stream, const char *format, va_list args)
{
    if (!stream)
        return;

    va_list copy;
    va_copy(copy, args);
    vfprintf(stream, format, copy);
    va_end(copy);

    fflush(stream);
}

void LogPrintf(const char *format, ...)
{
    if (!format)
        return;

    va_list args;
    va_start(args, format);

    log_to_stream(stdout, format, args);
    log_to_stream(original_stdout, format, args);
    log_to_stream(out_debug_file, format, args);

    va_end(args);
}
