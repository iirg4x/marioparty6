#ifndef REL_M613DLL_GEOMETRY_H
#define REL_M613DLL_GEOMETRY_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"

extern s16 lbl_1_bss_0;
s32 fn_1_0(Point3d point, f32 radius, Point3d center, f32 centerRadius);
s32 fn_1_28C(Point3d firstPosition, Point3d *firstDirection, f32 firstRadius,
             Point3d secondPosition, Point3d *secondDirection, f32 secondRadius);
s32 fn_1_70C(Point3d firstPosition, f32 angle, f32 firstRadius,
             Point3d firstDirection, Point3d secondPosition, f32 secondRadius,
             Point3d *secondDirection);

#endif
