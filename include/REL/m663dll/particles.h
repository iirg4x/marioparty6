#ifndef M663DLL_PARTICLES_H
#define M663DLL_PARTICLES_H

#include "game/hu3d.h"

typedef struct M663ParticleState {
    HU3D_MODELID modelIds[2];
    s16 maximumCounts[2];
    void *allocation[2];
} M663ParticleState;

extern M663ParticleState lbl_1_bss_1368;

#endif
