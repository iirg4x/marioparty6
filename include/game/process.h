#ifndef _GAME_PROCESS_H
#define _GAME_PROCESS_H

#include "game/jmp.h"
#include "dolphin/types.h"

typedef struct Process_s HUPROCESS;

struct Process_s {
    HUPROCESS *next;
    HUPROCESS *prev;
    HUPROCESS *child;
    HUPROCESS *parent;
    HUPROCESS *nextChild;
    HUPROCESS *firstChild;
    void *heap;
    u16 exec;
    u16 stat;
    u16 prio;
    s32 sleep;
    u32 spBase;
    jmp_buf jump;
    void (*destructor)(void);
    void *property;
};

void HuPrcInit(void);
void HuPrcEnd(void);
HUPROCESS *fn_8000D818(void (*func)(void), u16 prio, u32 stackSize, s32 extraSize);
void HuPrcChildLink(HUPROCESS *parent, HUPROCESS *child);
void HuPrcChildUnlink(HUPROCESS *process);
HUPROCESS *fn_8000DAC0(void (*func)(void), u16 prio, u32 stackSize, s32 extraSize,
    HUPROCESS *parent);
void HuPrcChildWatch(void);
HUPROCESS *fn_8000DD9C(void);
s32 HuPrcKill(HUPROCESS *process);
void HuPrcChildKill(HUPROCESS *process);
void HuPrcSleep(s32 time);
void HuPrcVSleep();
void HuPrcWakeup(HUPROCESS *process);
void HuPrcDestructorSet2(HUPROCESS *process, void (*func)(void));
void HuPrcDestructorSet(void (*func)(void));
void fn_8000E37C(s32 tick);
void *HuPrcMemAlloc(s32 size);
void HuPrcMemFree(void *ptr);
void HuPrcSetStat(HUPROCESS *process, u16 value);
void HuPrcResetStat(HUPROCESS *process, u16 value);
void HuPrcAllPause(s32 flag);
void HuPrcAllUPause(s32 flag);

#endif