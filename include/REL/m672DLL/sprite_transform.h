#ifndef REL_M672DLL_SPRITE_TRANSFORM_H
#define REL_M672DLL_SPRITE_TRANSFORM_H

extern "C" {
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"
}

class SpriteTransform {
public:
    u8 unknown00[0x14]; /* No code names this field. */
    ANIMDATA *texture;
    f32 x;
    f32 y;
    f32 z;
    f32 scale;
    f32 alpha;
    void fn_1_8EE4(Mtx parentMatrix);
};

#endif
