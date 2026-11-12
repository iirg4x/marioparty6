/* Shared minigame timer state. */
#ifndef _MG_TIMER_TYPES_H
#define _MG_TIMER_TYPES_H

#include "dolphin/types.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/gamemes.h"

#include "humath.h"

typedef struct MgTimer_s {
    s32 mode;
    s32 prevMode;
    s32 type;
    s32 offType;
    BOOL dispOnF;
    BOOL stopF;
    s32 scaleDir;
    BOOL boxDispF;
    u8 flashFlag;
    s32 maxTime;
    s32 endTime;
    s32 speed;
    s32 time;
    s32 recordTime;
    s32 fadeOutTime;
    HuVec2f pos;
    s16 timeUnit;
    GAMEMESID gameMesId;
    HUSPR_GROUPID boxId;
    HUSPRID espId[15];
    float digitScale;
    HUPROCESS *proc;
    u8 digitR;
    u8 digitG;
    u8 digitB;
} MGTIMER;

#endif
