#ifndef M678_PLAYER_WORK_H
#define M678_PLAYER_WORK_H

#include "game/object.h"

typedef struct M678PlayerSetupWork {
    s32 playerIndex;
    s32 groupPlayerIndex;
    s32 characterIndex;
    s32 characterNo;
    s32 padNo;
    s32 difficulty;
    f32 gridRow;
    f32 gridColumn;
    f32 previousRow;
    f32 shiftedPosition;
    s32 state[8];
    f32 releaseSpeed;
    s32 currentMotion;
    s32 motionState[2];
    s32 activeFlag;
    s32 pendingRumble;
    s32 computerState;
    struct M678LinkEntry *computerTarget;
    s32 computerDestinationRow;
    s32 computerRowOffset;
    s32 computerWaitFrames;
} M678PlayerSetupWork;

void fn_1_1BC8(OMOBJ *obj);
void fn_1_10D7C(OMOBJ *obj);
void fn_1_11148(OMOBJ *obj);
void fn_1_4FB0(OMOBJ *obj);
void fn_1_5CC(void);

#endif
