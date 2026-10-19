/* Strawberry Shortfuse: pooled frame callbacks for stage and character actions. */
#ifndef REL_M680DLL_SCHEDULER_H
#define REL_M680DLL_SCHEDULER_H

#include "dolphin/types.h"

typedef s32 (*M680Callback)(void *);
typedef struct M680CallbackNode M680CallbackNode;

typedef struct M680Scheduler {
    M680CallbackNode *freeHead;
    M680CallbackNode *activeHead;
    M680CallbackNode *storage;
} M680Scheduler;

struct M680CallbackNode {
    M680Scheduler *owner;
    M680CallbackNode *prev;
    M680CallbackNode *next;
    M680Callback callback;
    void *data;
};

extern M680Scheduler lbl_1_bss_1410;
extern M680CallbackNode lbl_1_bss_10[256];

void fn_1_BD64(void);
M680CallbackNode *fn_1_BE08(M680Callback callback, void *data);
void fn_1_BE70(M680CallbackNode **job);
void fn_1_BEFC(void);

#endif
