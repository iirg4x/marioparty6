#ifndef MIKEACT_EVENT_CALLBACK_H
#define MIKEACT_EVENT_CALLBACK_H

#include "dolphin/types.h"

typedef s32 (*MikeActEventCallback)(void *playerContext, void *eventTarget);

extern MikeActEventCallback lbl_1_bss_C0;
void fn_1_4518(MikeActEventCallback callback);
s32 fn_1_120E0(void *playerContext, void *eventTarget);
s32 fn_1_15B9C(void *playerContext, void *eventTarget);

#endif
