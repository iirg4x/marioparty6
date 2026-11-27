/* Declares the minigame timer operations. */
#ifndef REL_M661DLL_TIMER_H
#define REL_M661DLL_TIMER_H

#include "game/mg/timer.h"

/* Timer creation, updates and cleanup. */
MGTIMER *fn_1_2FC4(int type, int duration, int end, float x, float y);
void fn_1_3038(MGTIMER *timer);
void fn_1_308C(MGTIMER *timer);

#endif
