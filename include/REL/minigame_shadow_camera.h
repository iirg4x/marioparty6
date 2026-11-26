#ifndef REL_MINIGAME_SHADOW_CAMERA_H
#define REL_MINIGAME_SHADOW_CAMERA_H

#include "dolphin/mtx.h"

/* Shadow camera transform and orbit settings. */
typedef struct MinigameShadowCamera {
    Vec cameraPosition;
    f32 elevation;
    f32 azimuth;
    f32 unk14; /* The game's code never reads this field. */
    Vec target;
    u8 unk24[12]; /* No code names this field. */
    Vec cameraUp;
    u8 unk3C[12]; /* No code names this field. */
    f32 orbitDistance;
    f32 shadowOpacity;
} MinigameShadowCamera;

#endif
