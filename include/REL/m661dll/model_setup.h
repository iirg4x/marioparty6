#ifndef REL_M661DLL_MODEL_SETUP_H
#define REL_M661DLL_MODEL_SETUP_H

#include "game/hu3d.h"

typedef struct ModelSetupRecord {
    HU3D_MODELID modelId;
    s16 state;
    HuVecF position;
    HuVecF shadowPosition;
    HuVecF rotation;
} ModelSetupRecord;

extern ModelSetupRecord lbl_1_bss_1E3C[113];

typedef struct BoundaryEdge { f32 x0; f32 x1; f32 z0; f32 z1; } BoundaryEdge;

#endif
