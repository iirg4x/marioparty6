#ifndef REL_FRAMEWORK_GROUND_SPRITE_H
#define REL_FRAMEWORK_GROUND_SPRITE_H

// Draws the player's ground sprite through an owned Hu3D model.
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}
extern "C" {
#include "game/animdata.h"
}
#include <new>
struct GroundSprite : GameObject, OwnedModel {
    ANIMDATA *animation;
    Vec origin;
    float radius;
    float alpha;
    GroundSprite(ANIMDATA *, const Vec *);
    virtual ~GroundSprite();
    virtual void update();
    void fn_1_8EE4(Mtx parentMatrix);
    // Set the sprite opacity.
    void setAlpha(float value) { alpha = value; }
    // Set the sprite radius.
    void setRadius(float value) { radius = value; }
};

#endif
