/*
 * Theory: Search time control.
 *
 * We use a monotonic clock (CLOCK_MONOTONIC) so that elapsed time is not
 * affected by system clock changes. When the search starts we record the
 * start time and the limit in milliseconds. search_time_up() returns true
 * when elapsed >= limit; the root search checks this each iteration to stop
 * iterative deepening in time.
 */
#include <time.h>
#include <stdbool.h>
#include <stdint.h>

#include "search.h"

static uint64_t g_startTimeMs = 0;
static uint64_t g_timeLimitMs = 0;

static uint64_t current_time_ms(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    return (uint64_t)ts.tv_sec * 1000ULL +
           (uint64_t)ts.tv_nsec / 1000000ULL;
}

void search_start_timer(int timeLimitMs)
{
    if (timeLimitMs <= 0) {
        g_timeLimitMs = 0;
        return;
    }

    g_timeLimitMs = (uint64_t)timeLimitMs;
    g_startTimeMs = current_time_ms();
}

bool search_time_up(void)
{
    if (g_timeLimitMs == 0)
        return false;

    uint64_t now = current_time_ms();
    return (now - g_startTimeMs) >= g_timeLimitMs;
}

uint64_t search_elapsed_ms(void)
{
    if (g_timeLimitMs == 0)
        return 0;

    uint64_t now = current_time_ms();
    return now - g_startTimeMs;
}
