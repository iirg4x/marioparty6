#ifndef M604_PARTICLE_OWNER_H
#define M604_PARTICLE_OWNER_H
#include "dolphin/mtx.h"
extern "C" {
#include "game/hu3d.h"
}

// A particle model the object owns; pending until its first position is set.
struct ModelParticles {
    short modelId;
    u32 unknown_04; // Never read.
    HU3D_PARTICLE *particle;
    unsigned char pending;

    ModelParticles(int mode);
    ~ModelParticles();
};

void fn_1_153C4(ModelParticles *record, const Vec *position);
void fn_1_15440(ModelParticles *owner, HU3D_PARTICLE *particle);
void fn_1_1578C(ModelParticles *owner, Mtx matrix);
#endif
