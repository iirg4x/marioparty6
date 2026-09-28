#define _MATH_H
#include "game/object.h"

/* Readonly consumed views: three 0xC-byte Point3d templates and three
 * four-byte scalar parameters. Original declarations/names are unknown. */
extern const Point3d lbl_1_rodata_104;
extern const Point3d lbl_1_rodata_110;
extern const Point3d lbl_1_rodata_11C;
extern const f32 lbl_1_rodata_C8;
extern const f32 lbl_1_rodata_A0;
extern const f32 lbl_1_rodata_CC;

void fn_1_4380(void)
{
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;

    sp20 = lbl_1_rodata_104;
    sp14 = lbl_1_rodata_110;
    sp8 = lbl_1_rodata_11C;
    Hu3DShadowCreate(lbl_1_rodata_C8, lbl_1_rodata_A0, lbl_1_rodata_CC);
    Hu3DShadowPosSet(&sp20, &sp14, &sp8);
}
