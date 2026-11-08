#ifndef M663DLL_LIGHTING_H
#define M663DLL_LIGHTING_H

#include "game/hu3d.h"

typedef struct M663LightState {
    HU3D_LIGHTID id;
    Vec position;
    Vec orientation;
    Vec aim;
    Vec direction;
    f32 range;
    GXColor color;
} M663LightState;

extern M663LightState lbl_1_bss_13EC;
void fn_1_1CC4(void);
void fn_1_1E98(HU3D_LIGHTID lightId);

#endif
