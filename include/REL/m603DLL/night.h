#ifndef REL_M603DLL_NIGHT_H
#define REL_M603DLL_NIGHT_H

extern "C" {
#include "game/gamework.h"
}

static inline BOOL NightActive()
{
    s16 nightSetting = lbl_802C0278;
    return nightSetting != 0;
}

#endif
