#ifndef REL_M611DLL_GROUND_SPRITE_EFFECT_H
#define REL_M611DLL_GROUND_SPRITE_EFFECT_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/list.h"
#include "REL/framework/model.h"
#include "dolphin/mtx.h"

// An expanding, fading sprite on the ground.
struct GroundSpriteEffect : ListNode<GroundSpriteEffect> {
    GroundSpriteEffect(const Vec *initialPosition);
    ~GroundSpriteEffect();
    OwnedModel model;
    Vec position;
    f32 scale;
    f32 maximumScale;
    s32 elapsedFrames;
    s32 alpha;
};

void fn_1_7354(GroundSpriteEffect *effect);
void fn_1_737C(GroundSpriteEffect *effect);

void fn_1_7484(GroundSpriteEffect *effect, Mtx parentMatrix);
void fn_1_71F4(HU3D_MODEL *model, Mtx *matrix);

#endif
