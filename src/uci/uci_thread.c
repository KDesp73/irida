#include "uci_thread.h"
#include "uci.h"
#include "core.h"
#include "search.h"
#include "tt.h"
#include "castro.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>

static pthread_mutex_t g_mutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t g_go_cond = PTHREAD_COND_INITIALIZER;
static pthread_cond_t g_search_done_cond = PTHREAD_COND_INITIALIZER;
static pthread_mutex_t g_stdout_mutex = PTHREAD_MUTEX_INITIALIZER;

static volatile bool g_go_pending = false;
static volatile bool g_search_running = false;
static volatile bool g_quit = false;
static pthread_t g_search_tid;

void uci_stdout_lock(void)
{
    pthread_mutex_lock(&g_stdout_mutex);
}

void uci_stdout_unlock(void)
{
    pthread_mutex_unlock(&g_stdout_mutex);
}

void uci_search_wait_done(void)
{
    pthread_mutex_lock(&g_mutex);
    while (g_search_running)
        pthread_cond_wait(&g_search_done_cond, &g_mutex);
    pthread_mutex_unlock(&g_mutex);
}

void uci_search_start(void)
{
    pthread_mutex_lock(&g_mutex);
    g_go_pending = true;
    pthread_cond_signal(&g_go_cond);
    pthread_mutex_unlock(&g_mutex);
}

static void* search_thread_main(void* arg)
{
    (void)arg;
    pthread_mutex_lock(&g_mutex);
    for (;;) {
        while (!g_go_pending && !g_quit)
            pthread_cond_wait(&g_go_cond, &g_mutex);
        if (g_quit) {
            pthread_mutex_unlock(&g_mutex);
            return NULL;
        }
        g_go_pending = false;
        g_search_running = true;
        pthread_mutex_unlock(&g_mutex);

        Move move = engine.search(&engine.board, engine.eval, engine.order, &g_searchConfig);

        pthread_mutex_lock(&g_mutex);
        g_search_running = false;
        pthread_cond_broadcast(&g_search_done_cond);
        pthread_mutex_unlock(&g_mutex);

        char bestmove[16];
        castro_MoveToString(move, bestmove);
        uci_stdout_lock();
        printf("bestmove %s\n", bestmove);
        fflush(stdout);
        uci_stdout_unlock();
    }
}

void uci_search_thread_start(void)
{
    g_quit = false;
    g_go_pending = false;
    g_search_running = false;
    if (pthread_create(&g_search_tid, NULL, search_thread_main, NULL) != 0) {
        uci_stdout_lock();
        fprintf(stderr, "info string Failed to create search thread\n");
        uci_stdout_unlock();
    }
}

void uci_search_thread_join(void)
{
    pthread_mutex_lock(&g_mutex);
    g_quit = true;
    pthread_cond_signal(&g_go_cond);
    pthread_mutex_unlock(&g_mutex);
    pthread_join(g_search_tid, NULL);
}
