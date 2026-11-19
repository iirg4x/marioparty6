#ifndef GAME_HU3D_PARTICLE_HOOK_H
#define GAME_HU3D_PARTICLE_HOOK_H

#include "dolphin/mtx.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct Hu3DModel_s HU3D_MODEL;
typedef struct Hu3DParticle_s HU3D_PARTICLE;
typedef void (*HU3D_PARTICLE_HOOK)(HU3D_MODEL *modelP, HU3D_PARTICLE *particleP, Mtx mtx);

#ifdef __cplusplus
}
#endif

#endif
