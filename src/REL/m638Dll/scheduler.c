/* Callback queue used by M638 for timed model, prop, and result effects. */
#include "REL/m638Dll.h"

M638Jobs lbl_1_bss_A18;

M638Job lbl_1_bss_18[128];

/* Initialize the free callback slots before the scene object starts its update loop. */
void fn_1_75BC(void)
{
    M638Job *node;
    M638Jobs *jobs;
    int i;
    jobs = &lbl_1_bss_A18;
    node = lbl_1_bss_18;
    jobs->storage = node;
    jobs->freeHead = node;
    jobs->activeHead = NULL;
    for (i = 0; i < 127; i++, node++) {
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

/* Add a callback to the active list and return nonzero when a free slot was available. */
s32 fn_1_7660(s32 (*callback)(void *), void *data)
{
    M638Job *node = NULL;
    M638Jobs *jobs;
    jobs = &lbl_1_bss_A18;
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
    /* Callers use a nonzero result to tell whether the callback was scheduled. */
    return (s32)node;
}

/* Remove a completed callback from the active list and return its slot to the free list. */
void fn_1_76C8(M638Job **job)
{
    M638Job *node;
    M638Jobs *jobs;
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

/* Scene update: run active callbacks and recycle each slot that reports completion. */
void fn_1_7754(void)
{
    M638Job *node;
    M638Jobs *jobs;
    M638Job *next;
    jobs = &lbl_1_bss_A18;
    node = jobs->activeHead;
    while (node) {
        next = node->next;
        if (node->callback && node->callback(node->data) != 0) {
            fn_1_76C8(&node);
        }
        node = next;
    }
}
