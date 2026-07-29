#include <dolphin/types.h>

typedef s16 HU3D_LIGHTID;

extern HU3D_LIGHTID lbl_1_bss_82C[2];

void Hu3DGLightKill(HU3D_LIGHTID lightId);

void fn_1_10B8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DGLightKill(lbl_1_bss_82C[i]);
    }
}
