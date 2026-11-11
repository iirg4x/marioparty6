#ifndef M639_RESOURCE_PARAMETERS_H
#define M639_RESOURCE_PARAMETERS_H
#include "dolphin/types.h"
#include "game/animdata.h"
struct ParameterResource : ANIMDATA {};
/* One textured quad: half its size, its pattern, its centre and opacity, and the textures it draws. */
struct ResourceParameters {
    float halfWidth;
    float halfHeight;
    long textureIndex;
    float centerX;
    float centerY;
    float opacity;
    ParameterResource *firstTexture;
    ParameterResource *secondTexture;
    u8 blendMode;
    ResourceParameters(ParameterResource *resource, unsigned int mode);
    ResourceParameters(ParameterResource *resource, ParameterResource *value, unsigned int mode);
    void draw();
};
#endif
