#include "REL/m638Dll.h"

M638Jobs lbl_1_bss_A18;

M638Job lbl_1_bss_18[128];

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
    /* The 32-bit target API exposes an opaque scalar token; consumers test it. */
    return (s32)node;
}

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
