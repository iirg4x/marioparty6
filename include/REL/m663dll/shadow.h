#ifndef M663DLL_SHADOW_H
#define M663DLL_SHADOW_H

#include "game/hu3d.h"

typedef struct M663ShadowState {
    Vec cameraPosition;
    f32 azimuth;
    f32 elevation;
    f32 unobservedValue; /* Initialized to zero; never read. */
    Vec target;
    u8 unobserved24[12];
    Vec cameraUp;
    u8 unobserved3C[12];
    f32 distance;
    f32 transparency;
} M663ShadowState;

extern M663ShadowState lbl_1_bss_139C;
void fn_1_2104(void);
void fn_1_2214(void);

#endif
