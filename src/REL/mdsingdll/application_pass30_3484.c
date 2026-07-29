#include "dolphin/types.h"
#include "game/hu3d.h"

extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_F0;
extern s16 lbl_1_bss_13A0[];

void fn_1_3484(void)
{
    lbl_1_bss_13A0[0] = Hu3DGLightCreate(
        lbl_1_rodata_64, lbl_1_rodata_5C, lbl_1_rodata_5C,
        lbl_1_rodata_64, lbl_1_rodata_F0, lbl_1_rodata_F0,
        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[0]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[0], TRUE);
    lbl_1_bss_13A0[1] = Hu3DGLightCreate(
        lbl_1_rodata_F0, lbl_1_rodata_5C, lbl_1_rodata_F0,
        lbl_1_rodata_5C, lbl_1_rodata_F0, lbl_1_rodata_F0,
        255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_13A0[1]);
    Hu3DGLightStaticSet(lbl_1_bss_13A0[1], TRUE);
}

void fn_1_35B0(void)
{
    Hu3DGLightKill(lbl_1_bss_13A0[0]);
    Hu3DGLightKill(lbl_1_bss_13A0[1]);
}
