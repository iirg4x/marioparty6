#ifndef MICQUIZISHIDLL_MESSAGE_SETUP_VIEWS_V2_H
#define MICQUIZISHIDLL_MESSAGE_SETUP_VIEWS_V2_H

#include "REL/micquizishidll/module_base.h"
#include "game/window.h"

void MicQuizWinMesSet(u32 messageNumber, s16 messageSpeed, u8 windowIndex);
void MicQuizWinOpen(s32 windowIndex);
void MicQuizWinMesWait(void);
void MicQuizWaitTwoSeconds(void);
void fn_1_32724(void);
void fn_1_3275C(void);
void fn_1_C78(void);
void fn_1_12F4(void);
void fn_1_192C(void);
void fn_1_1D24(void);
void fn_1_2160(void);
void fn_1_235C(void);
void MicQuizTimeoutFollowupPresent(void);
void MicQuizResponseFollowupPresent(void);
void MicQuizReactionLeadIn(void);
void fn_1_A38(u32 message);
void fn_1_E74(u32 message);
void fn_1_10B4(u32 message);
void fn_1_14F0(u32 message);
void fn_1_1F20(u32 message);
void MicQuizPresentationDone(void);
inline void MicQuizWinInsertMesSet(s32 windowNo, u32 messageId, s16 insertNo);
void MicQuizAnswerMesPresent(s32 messageType, s32 alternate);

#endif
