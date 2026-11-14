#ifndef M654DLL_SPRITE_PARAMETERS_H
#define M654DLL_SPRITE_PARAMETERS_H

#include "game/animdata.h"

// One score-display image: half size, position, uniform scale, opacity and animation.
struct SpriteParameters {
    float halfWidth;
    float halfHeight;
    float x;
    float y;
    float scale;
    float opacity;
    ANIMDATA *animation;

    SpriteParameters(ANIMDATA *image);
    // Copies the sprite geometry, scale, opacity and animation.
    SpriteParameters(const SpriteParameters &source)
        : halfWidth(source.halfWidth), halfHeight(source.halfHeight),
          x(source.x), y(source.y),
          scale(source.scale), opacity(source.opacity),
          animation(source.animation)
    {
    }
    SpriteParameters &operator=(const SpriteParameters &source);
};

#endif
