#ifndef M654DLL_SPRITE_PARAMETERS_H
#define M654DLL_SPRITE_PARAMETERS_H

#include "game/animdata.h"

// One image of a score display: half size, place, scale and the animation it draws.
struct SpriteParameters {
    float halfWidth;
    float halfHeight;
    float x;
    float y;
    float scaleX;
    float scaleY;
    ANIMDATA *animation;

    SpriteParameters(ANIMDATA *image);
    SpriteParameters(const SpriteParameters &source)
        : halfWidth(source.halfWidth), halfHeight(source.halfHeight),
          x(source.x), y(source.y),
          scaleX(source.scaleX), scaleY(source.scaleY),
          animation(source.animation)
    {
    }
    SpriteParameters &operator=(const SpriteParameters &source);
};

#endif
