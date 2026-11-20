#ifndef REL_M611DLL_MOTION_PARTICLE_OWNER_H
#define REL_M611DLL_MOTION_PARTICLE_OWNER_H

extern "C" {
#include "game/hu3d.h"
}

#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/list.h"

// Position and velocity of a falling particle.
struct FallingParticle {
    Vec position;
    Vec velocity;
};

// Owns the particles, ring vertices, and their draw model.
struct MotionParticleOwner : ListNode<MotionParticleOwner> {
    OwnedModel model;
    FallingParticle particles[6000];
    Vec ring[9];
    MotionParticleOwner();
    ~MotionParticleOwner();
};

void fn_1_7E44(MotionParticleOwner *pool);
void fn_1_7E6C(MotionParticleOwner *pool);

void fn_1_806C(MotionParticleOwner *pool, Mtx sourceMatrix);

void fn_1_7CF0(HU3D_MODEL *model, Mtx *matrix);

#endif
