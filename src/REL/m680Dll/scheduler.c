/* Strawberry Shortfuse: frame callbacks for timed character and stage actions. */
#include "REL/m680Dll/scheduler.h"

/* Initializes the callback pool before the minigame starts. */
void fn_1_BD64(void)
{
    M680CallbackNode *node;
    M680Scheduler *jobs;
    int i;

    jobs = &lbl_1_bss_1410;
    node = lbl_1_bss_10;
    jobs->storage = node;
    jobs->freeHead = node;
    jobs->activeHead = NULL;
    for (i = 0; i < 255; i++, node++) {
        node->owner = jobs;
        node->prev = NULL;
        node->next = node + 1;
        node->data = NULL;
        node->callback = NULL;
    }
    node->owner = jobs;
    node->prev = NULL;
    node->next = NULL;
    node->data = NULL;
    node->callback = NULL;
}

/* Adds an action to the active list, returning NULL when the pool is full. */
M680CallbackNode *fn_1_BE08(M680Callback callback, void *data)
{
    M680CallbackNode *node = NULL;
    M680Scheduler *jobs;
    jobs = &lbl_1_bss_1410;
    node = jobs->freeHead;
    if (node) {
        jobs->freeHead = node->next;
        node->callback = callback;
        node->data = data;
        node->next = jobs->activeHead;
        if (node->next) {
            node->next->prev = node;
        }
        jobs->activeHead = node;
    }
    return node;
}

/* Returns a completed action to the pool and clears its caller's handle. */
void fn_1_BE70(M680CallbackNode **job)
{
    M680CallbackNode *node;
    M680Scheduler *jobs;
    node = *job;
    jobs = node->owner;
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        jobs->activeHead = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    node->prev = NULL;
    node->next = jobs->freeHead;
    node->callback = NULL;
    node->data = NULL;
    jobs->freeHead = node;
    *job = NULL;
}

/* Runs active actions each frame, removing those whose callback reports completion. */
void fn_1_BEFC(void)
{
    M680CallbackNode *node;
    M680Scheduler *jobs;
    M680CallbackNode *next;

    jobs = &lbl_1_bss_1410;
    node = jobs->activeHead;
    while (node) {
        next = node->next;
        if (node->callback && node->callback(node->data) != 0) {
            fn_1_BE70(&node);
        }
        node = next;
    }
}

M680Scheduler lbl_1_bss_1410;
M680CallbackNode lbl_1_bss_10[256];
