#ifndef GAME_MIC_H
#define GAME_MIC_H

#include "dolphin/types.h"

typedef struct MCResponseEntry_s {
    u16 score; /* Recognition score for this candidate. */
    s16 count; /* Number of recognized words in result. */
    s16 *result; /* Word indices in shared speech storage, or the pad window process's selected
                  * value. */
} MCResponseEntry_s;

typedef struct MCResponse_s {
    s16 status; /* Zero for success; negative values report errors, cancellation, timeout or no
                 * result. */
    u16 score; /* Recognition score of the first candidate. */
    s16 count; /* Number of recognized words in the first candidate. */
    s16 *result; /* Word indices of the first candidate. */
    s32 word; /* First speech-word index, a one-based pad-choice value, or -1 when unavailable. */
    MCResponseEntry_s entry[10]; /* Candidates in recognition rank order. */
} MCResponse_s;

typedef void (*MCResponseCallback)(MCResponse_s *response);

s32 HuMCInit(s16 mountResult);
void HuMCThresholdSet(u32 threshold);
s32 HuMCMount(s32 chan);
s32 HuMCProbe(s32 channel);
s32 HuMCMicGet(void);
void HuMCMicSet(s32 flag);
void HuMCClose(void);
s16 HuMCContextCreate(char *path);
void HuMCContextKill(s16 context);
void HuMCListenerCreate(s16 context, MCResponseCallback callback, u8 flags);
void HuMCListenerKill(void);
int HuMCResponseGet(void);
MCResponse_s *HuMCCurResponseGet(void);
s32 HuMCUnkResponseCheck(void);
s32 HuMCNewResponseGet(s32 messageBase);
void HuMCSelWinCreate(f32 x, f32 y);
s32 HuMCSelWinItemSet(s32 messageBase,s16 itemCount,s16 padNo);
void HuMCSelWinContextSet(s16 contextId, MCResponseCallback responseCallback, u8 padNo);
int HuMCSelWinChoiceGet(void);
BOOL HuMCSelWinCheck(void);
void HuMCSelWinContextKill(void);
void HuMCSessionSet(s16 playerIndex);
void HuMCSessionClose(void);
s32 HuMCButtonGet(void);
s32 HuMCButtonDownGet(void);
void HuMCSysInit(void);
void HuMCPeriodicProc(void);

#endif
