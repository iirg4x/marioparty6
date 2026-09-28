#define _MATH_H
#include "game/object.h"

/* Existing target readonly scalars and two consumed light-ID halfwords.
 * No storage is defined here; original declarations/names remain unknown. */
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_F0;
extern HU3D_LIGHTID lbl_1_bss_860[2];

void fn_1_329C(void)
{
    lbl_1_bss_860[0] = Hu3DGLightCreate(lbl_1_rodata_74, lbl_1_rodata_6C, lbl_1_rodata_6C,
                                        lbl_1_rodata_74, lbl_1_rodata_F0, lbl_1_rodata_F0,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[0]);
    Hu3DGLightStaticSet(lbl_1_bss_860[0], TRUE);
    lbl_1_bss_860[1] = Hu3DGLightCreate(lbl_1_rodata_F0, lbl_1_rodata_6C, lbl_1_rodata_F0,
                                        lbl_1_rodata_6C, lbl_1_rodata_F0, lbl_1_rodata_F0,
                                        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[1]);
    Hu3DGLightStaticSet(lbl_1_bss_860[1], TRUE);
}
