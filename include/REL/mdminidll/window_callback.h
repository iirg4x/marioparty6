/* Message sound callback used by minigame-mode menu windows. */
#ifndef REL_MDMINIDLL_WINDOW_CALLBACK_H
#define REL_MDMINIDLL_WINDOW_CALLBACK_H

#include "game/window.h"

/* A window message invokes this to play its indexed sound once per new message. */
void fn_1_0(HUWINID unusedWindow, u32 messageId, s16 soundIndex);

#endif
