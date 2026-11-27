#ifndef REL_MINIGAME_PARTICIPANT_MOTION_H
#define REL_MINIGAME_PARTICIPANT_MOTION_H

#include "dolphin/types.h"

typedef struct MinigameParticipantMotionState {
    s16 motionIds[7];
    s16 mode;
    s16 previousMode;
    f32 speed;
    f32 previousSpeed;
    f32 time;
    f32 previousTime;
    f32 unk24; /* The game's code never reads this field. */
    u32 attributes;
    u32 previousAttributes;
    u32 attributesToReset;
    s32 motionEnded;
    s16 resetRequested;
    s16 resetMode;
    f32 transition;
    u32 resetAttributes;
    u32 resetAttributesToClear;
    s16 shiftState;
    s16 shiftMotion;
    u32 shiftFlags;
    f32 shiftStart;
    f32 shiftEnd;
    f32 shiftTime;
    f32 unk5C; /* The game's code never reads this field. */
    u8 unk60[4]; /* No code names this field. */
} MinigameParticipantMotionState;

#endif
