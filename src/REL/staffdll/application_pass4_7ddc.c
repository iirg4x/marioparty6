#include <dolphin/types.h>

typedef s16 HU3D_MODELID;

extern HU3D_MODELID lbl_1_bss_40[2];

void Hu3DModelKill(HU3D_MODELID modelId);

void fn_1_7DDC(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_40[i]);
    }
}
