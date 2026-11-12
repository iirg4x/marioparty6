#ifndef M604DLL_SPRITE_PARAMETERS_H
#define M604DLL_SPRITE_PARAMETERS_H

#include "dolphin/types.h"
extern "C" {
#include "game/sprput.h"
}

struct FlareSpriteParameters {
    f32 width;
    f32 verticalScale;
    u32 color;
    s32 textureMap;
};

extern "C" const FlareSpriteParameters lbl_1_rodata_230[8];

#endif
