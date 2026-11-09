#ifndef REL_M672DLL_EFFECT_BF64_H
#define REL_M672DLL_EFFECT_BF64_H

// Draws the player's ground sprite through an owned Hu3D model.
#include "REL/framework/game_object.h"
#include "REL/m672DLL/model_bf64.h"
extern "C" {
#include "game/animdata.h"
}
void fn_1_8EAC(HU3D_MODEL *, Mtx *);
void operator delete(void *);
struct GroundSprite : GameObject, OwnedModel {
    ANIMDATA *animation;
    Vec origin;
    float radius;
    float alpha;
    GroundSprite(ANIMDATA *, const Vec *);
    virtual ~GroundSprite();
    virtual void update();
    void setAlpha(float value) { alpha = value; }
    void setRadius(float value) { radius = value; }
};

#endif
