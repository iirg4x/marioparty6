#ifndef _MG_SEQMAN_H
#define _MG_SEQMAN_H

#include "dolphin/types.h"

typedef void (*MGSEQ_FUNC)(s16 mode, s16 frameNo);

typedef struct MgSeqParam_s {
    s16 maxTime;
    u8 timerPos;
    MGSEQ_FUNC initHook;
    MGSEQ_FUNC fadeInHook;
    MGSEQ_FUNC startHook;
    MGSEQ_FUNC mainHook;
    MGSEQ_FUNC finishHook;
    MGSEQ_FUNC preWinnerHook;
    MGSEQ_FUNC winnerHook;
    MGSEQ_FUNC fadeOutHook;
    MGSEQ_FUNC closeHook;
} MGSEQ_PARAM;

typedef struct {
    u16 mode;
    MGSEQ_FUNC hook;
} MGSEQ_MODEHOOK;

void fn_80070788(MGSEQ_PARAM* param);
void fn_800707B4(MGSEQ_PARAM* param, s32 prio);
void fn_80070994();
u32 fn_80071A8C(void);
u32 fn_80071A9C(void);
u32 fn_80071AC4(u32 mode);
u16 fn_80071B08(void);
u16 fn_80071B30(void);
s32 fn_80071B58();
void fn_80071BAC(int recordVal);
u16 fn_80071C08();
u16 fn_80071C18(s16 charNo1, s16 charNo2, s16 charNo3, s16 charNo4);
u16 fn_80071C70(void);
void fn_80071CE8(s16 mode, s16 delay);
u32 fn_80071D20(s16 mode, MGSEQ_FUNC func);
void fn_80071DC0(s16 mode);
u32 fn_80071DE4();
void fn_80071DEC(u32 frameNo);
s16 fn_80071DF4();
void fn_80071DFC(s16 maxTime);
void fn_80071E60(u16 bit);
void fn_80071E78(u16 bit);

#endif
