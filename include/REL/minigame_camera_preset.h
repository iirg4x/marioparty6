#ifndef REL_MINIGAME_CAMERA_PRESET_H
#define REL_MINIGAME_CAMERA_PRESET_H

#include "dolphin/types.h"

/* Camera preset with perspective and viewport settings. */
typedef struct MinigameCameraPreset {
    s32 unk00; /* The game's code never reads this field. */
    u8 unk04[12]; /* No code names this field. */
    f32 unk10[3]; /* The game's code never reads this field. */
    f32 unk1C[3]; /* The game's code never reads this field. */
    u8 unk28[12]; /* No code names this field. */
    f32 unk34; /* The game's code never reads this field. */
    f32 aspectRatio;
    f32 viewport[4]; /* X, Y, width and height. */
    f32 fieldOfView;
} MinigameCameraPreset;

#endif
