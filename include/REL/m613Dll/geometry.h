#ifndef REL_M613DLL_GEOMETRY_H
#define REL_M613DLL_GEOMETRY_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"

extern s16 lbl_1_bss_0;
s32 fn_1_0(Vec point, f32 radius, Vec center, f32 centerRadius);
s32 fn_1_28C(Vec firstPosition, Vec *firstDirection, f32 firstRadius, Vec secondPosition,
             Vec *secondDirection, f32 secondRadius);
s32 fn_1_70C(Vec firstPosition, f32 angle, f32 firstRadius, Vec firstDirection, Vec secondPosition,
             f32 secondRadius, Vec *secondDirection);

#endif
