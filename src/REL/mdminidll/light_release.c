#define _MATH_H
#include "game/hu3d.h"

/* Creation and cleanup access both halfwords before the next block at +0x864. */
extern HU3D_LIGHTID lbl_1_bss_860[2];

void fn_1_33C8(void)
{
    Hu3DGLightKill(lbl_1_bss_860[0]);
    Hu3DGLightKill(lbl_1_bss_860[1]);
}
