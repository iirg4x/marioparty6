#ifndef REL_M611DLL_EFFECT_NODE_H
#define REL_M611DLL_EFFECT_NODE_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/list.h"
#include "REL/framework/model.h"

// An expanding ground quad created below the ground plane.
struct EffectNode : ListNode<EffectNode> {
    EffectNode(const Vec *initialPosition, f32 finalSizeIncrease);
    ~EffectNode();
    OwnedModel model;
    Vec position;
    f32 sizeIncrease;
    f32 maximumSizeIncrease;
    f32 alpha;
    s32 elapsedFrames;
};

void fn_1_5734(EffectNode *effect);
void fn_1_575C(EffectNode *effect);

void fn_1_58D4(EffectNode *effect, const Mtx sourceMatrix);
void fn_1_55D4(HU3D_MODEL *model, Mtx *matrix);

#endif
