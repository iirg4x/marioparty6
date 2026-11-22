// Declares an XZ-plane effect quad at a tracked position.

#ifndef REL_M673DLL_FOLLOW_EFFECT_H
#define REL_M673DLL_FOLLOW_EFFECT_H
#include "REL/framework/game_object2.h"
#include "game/animdata.h"

// Draws a textured quad at its attached world position.
struct FollowEffect : RenderObject {
    const Vec *position; // Borrowed world position read when drawing the quad.
    float scale; // Width of the square quad in world units.
    ANIMDATA *animation; // Texture shown on the quad.
    float alpha; // Multiplier applied to the color alpha.
    GXColor color; // Tint used to draw the quad.
    unsigned char visible; // Enables drawing.
    // Attaches an animated quad to a world position.
    FollowEffect(const Vec *position, float scale, ANIMDATA *animation);
    // Releases the quad render object.
    virtual ~FollowEffect();
    // Leaves the particle quad anchored to its position source.
    virtual void update();
    // Draws the effect using its animation and captured scene texture.
    virtual void draw(Mtx *transform);
    // Sets the alpha multiplier.
    void setAlpha(float value)
    {
        alpha = value;
    }

    // Sets the effect color.
    void setColor(GXColor value)
    {
        color = value;
    }
};

// Sets the effect's visibility flag.
void fn_1_BFBC(FollowEffect *effect, unsigned char visible);

#endif
