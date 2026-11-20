#ifndef REL_M611DLL_POSITIONED_PARTICLE_EFFECT_H
#define REL_M611DLL_POSITIONED_PARTICLE_EFFECT_H

extern "C" {
#include "game/hu3d.h"
}

#include "dolphin/types.h"
#include "REL/framework/list.h"

// Tracks the particle model at a scene position.
struct PositionedParticleEffect : ListNode<PositionedParticleEffect> {
    PositionedParticleEffect(Vec *initialPosition, s32 variant);
    ~PositionedParticleEffect();
    s16 particleModelId;
    s32 elapsedFrames;
};

void fn_1_B210();
void fn_1_B2E0();
void fn_1_B310(PositionedParticleEffect *effect);

// List of particle effects placed at scene positions.
extern ListRoot<PositionedParticleEffect> lbl_1_bss_194;

void fn_1_B378(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx);

void fn_1_B5C8(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx);

#endif
