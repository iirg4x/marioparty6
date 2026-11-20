#ifndef REL_M611DLL_EFFECT_MODEL_H
#define REL_M611DLL_EFFECT_MODEL_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/list.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"

// A pulsing sprite with two offset layers.
struct EffectModel : ListNode<EffectModel> {
    EffectModel(const Vec *initialPosition, f32 finalScale);
    ~EffectModel();
    OwnedModel model;
    Vec position;
    f32 scale;
    f32 offsetScale;
    f32 maximumScale;
    s32 phase;
};

void fn_1_61C0(EffectModel *effect);
void fn_1_61E8(EffectModel *effect);

void fn_1_6294(EffectModel *effect, Mtx parentMatrix, f32 scale, f32 opacity);
void fn_1_5FCC(HU3D_MODEL *model, Mtx *matrix);

#endif
