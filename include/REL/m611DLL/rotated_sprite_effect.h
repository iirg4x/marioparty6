#ifndef REL_M611DLL_ROTATED_SPRITE_EFFECT_H
#define REL_M611DLL_ROTATED_SPRITE_EFFECT_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/list.h"
#include "REL/framework/model.h"
#include "dolphin/mtx.h"

// A fading ground sprite aligned with the player heading.
struct RotatedSpriteEffect : ListNode<RotatedSpriteEffect> {
    RotatedSpriteEffect(const Vec *initialPosition, f32 initialRotation);
    ~RotatedSpriteEffect();
    OwnedModel model;
    Vec position;
    f32 scale;
    f32 rotationY;
    f32 maximumScale;
    s32 elapsedFrames;
    s32 alpha;
};

void fn_1_6A24(RotatedSpriteEffect *effect);
void fn_1_6A4C(RotatedSpriteEffect *effect);

void fn_1_6B54(RotatedSpriteEffect *effect, Mtx parentMatrix);
void fn_1_68C4(HU3D_MODEL *model, Mtx *matrix);

#endif
