#ifndef M639_RESOURCE_PARAMETERS_H
#define M639_RESOURCE_PARAMETERS_H
#include "dolphin/types.h"
#include "game/animdata.h"
/* One textured quad: half its size, its pattern, its centre and opacity, and the textures it draws. */
struct ResourceParameters {
    float halfWidth;
    float halfHeight;
    long textureIndex;
    float centerX;
    float centerY;
    float opacity;
    ANIMDATA *firstTexture;
    ANIMDATA *secondTexture;
    u8 blendMode;
    ResourceParameters(ANIMDATA *resource, unsigned int mode);
    ResourceParameters(ANIMDATA *resource, ANIMDATA *value, unsigned int mode);
    void draw();
};
#endif
