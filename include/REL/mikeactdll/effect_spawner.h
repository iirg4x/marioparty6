#ifndef MIKEACTDLL_EFFECT_SPAWNER_H
#define MIKEACTDLL_EFFECT_SPAWNER_H

#include "dolphin/types.h"
#include "game/hu3d.h"

typedef struct {
    s16 modelId;
    s16 active;
    f32 scale;
    f32 transparency;
} MikeActEffectParticle;

typedef struct {
    s16 modelId;
    f32 riseSpeed;
    s16 state;
    s16 cooldown;
    MikeActEffectParticle particles[0x20];
} MikeActEffectSpawner;

static inline void MikeActEffectModelSetup(HU3D_MODELID model, HuVecF *position)
{
    fn_80026A44(model, (1 << 0));
    fn_80026A44(model, 0x40000002);
    fn_800269E8(model, (1 << 27));
    fn_800269E8(model, (1 << 22));
    fn_80026B8C(model, 7);
    fn_80026790(model, position);
}

#endif
