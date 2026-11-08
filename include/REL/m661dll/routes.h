#ifndef M661DLL_ROUTES_H
#define M661DLL_ROUTES_H

#include "game/hu3d.h"

typedef struct MovementPath {
    HuVecF positions[20];
    f32 speeds[20];
    s16 waitFrames[20];
} MovementPath;

typedef struct BoardRouteGroup {
    MovementPath directions[2];
} BoardRouteGroup;

extern BoardRouteGroup lbl_1_bss_31D0[16];
void fn_1_C8D4(void);

#endif
