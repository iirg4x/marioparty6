#ifndef REL_MINIGAME_CAMERA_FIT_H
#define REL_MINIGAME_CAMERA_FIT_H

#include "dolphin/mtx.h"

/* Viewing direction and frame bounds used to fit a camera around points. */
typedef struct CameraFitParams {
    Vec direction;
    Vec up;
    f32 fov;
    s32 width;
    s32 height;
    s32 marginX;
    s32 marginY;
} CameraFitParams;

/* World-space sphere to fit. */
typedef struct CameraFitSphere {
    Vec center;
    f32 radius;
} CameraFitSphere;

#endif
