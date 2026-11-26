#ifndef REL_MINIGAME_CHARACTER_MOTION_H
#define REL_MINIGAME_CHARACTER_MOTION_H

#include "dolphin/types.h"

typedef struct MinigameCharacterMotion {
    s16 motionIndex; /* Character motion slot. -1 keeps the current motion. */
    f32 duration; /* Blend duration in frames. */
    u32 modelAttributes; /* Motion playback flags. */
    s32 voiceFlag; /* Character voice setting. */
    s32 motionOption; /* Selects between two branches that set the same motion flag. */
} MinigameCharacterMotion;

#endif
