#ifndef M625DLL_H
#define M625DLL_H

void fn_1_1284C(void);
void fn_1_12C80(void);
void fn_1_12D7C(s16 player);
void fn_1_12EA8(void);
s32 fn_1_13318(Point3d point, f32 radius_a, Point3d center, f32 radius_b);
s32 fn_1_133A4(Point3d point, Point3d *direction, f32 distance, Point3d origin, Point3d normal,
               f32 amount);
s32 fn_1_137DC(Point3d point, Point3d unusedPointA, Point3d *direction, f32 firstRadius,
               f32 firstDiameter, Point3d otherPoint, Point3d unusedPointB,
               Point3d *otherDirection, f32 secondRadius, f32 secondDiameter);
s32 fn_1_13AA8(f32 top, f32 bottom, f32 left, f32 right, Point3d position);

#endif
