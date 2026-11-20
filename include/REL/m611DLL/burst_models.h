#ifndef REL_M611DLL_BURST_MODELS_H
#define REL_M611DLL_BURST_MODELS_H

#include "REL/framework/model.h"

extern "C" {
#include "game/hu3d.h"
}

#include "REL/framework/list.h"

// Model and per-instance storage for a particle burst.
struct BurstModel {
    HU3D_MODELID modelId;
    s32 unkC; // The game's code never reads this field.
};

// Owns the model for a radial particle burst.
struct RadialBurstModel : ListNode<RadialBurstModel>, BurstModel {
    RadialBurstModel(Vec *position);
    ~RadialBurstModel();
};

// Owns the model for a single fading particle.
struct FadingParticleModel : ListNode<FadingParticleModel>, BurstModel {
    FadingParticleModel(Vec *position);
    ~FadingParticleModel();
};

// Owns a model for a rotating particle burst.
struct RotatingBurstModel : ListNode<RotatingBurstModel>, BurstModel {
    RotatingBurstModel(Vec *position);
    ~RotatingBurstModel();
};

// Initializes and advances the particles for each burst.
void fn_1_5168(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx);
void fn_1_4BF8(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx);
void fn_1_4650(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx);

// Removes finished particle bursts.
void fn_1_45CC(RadialBurstModel *);
void fn_1_4B74(FadingParticleModel *);
void fn_1_50E4(RotatingBurstModel *);

#endif
