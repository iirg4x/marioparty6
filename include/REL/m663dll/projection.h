#ifndef M663DLL_PROJECTION_H
#define M663DLL_PROJECTION_H

#include "game/hu3d.h"

typedef struct M663ProjectionState {
    HU3D_PROJID projectionId;
    Vec target;
    f32 horizontalAngle;
    f32 verticalAngle;
    f32 unobservedValue; /* Initialized to zero; never read. */
    f32 distance;
    f32 transparency;
} M663ProjectionState;

extern M663ProjectionState lbl_1_bss_1378;

#endif
