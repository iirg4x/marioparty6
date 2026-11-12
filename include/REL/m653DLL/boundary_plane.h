#ifndef M653_BOUNDARY_PLANE_H
#define M653_BOUNDARY_PLANE_H

#include "dolphin/mtx.h"
#include "REL/framework/plane.h"

// The arena's four walls, two across x and two across z; fn_1_8D0C and fn_1_8F8C move them in.
extern "C" Plane lbl_1_bss_70[4];

// Keeps the plane's normal and moves it through (x, 0, z).
void fn_1_8C84(Plane *plane, f32 x, f32 z);
void fn_1_9660(s32 mode);
f32 fn_1_9D30(f32 x, f32 z, Vec *nearestNormal);
s32 fn_1_9EF4(f32 x, f32 z, Vec *result, f32 threshold);

extern "C" {
void fn_1_9210(void);
f32 fn_1_9568(f32 x, f32 z);
void fn_1_965C(void);
}

#endif
