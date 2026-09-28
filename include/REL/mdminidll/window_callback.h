#ifndef REL_MDMINIDLL_WINDOW_CALLBACK_H
#define REL_MDMINIDLL_WINDOW_CALLBACK_H

#include "game/window.h"

/* Registered with HuWinCallbackSet; the unused window argument is part of the ABI. */
void fn_1_0(HUWINID winId, u32 message, s16 c);

#endif
