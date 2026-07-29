#include <dolphin/types.h>

typedef s16 HU3D_MODELID;

extern HU3D_MODELID lbl_1_bss_3E;

void Hu3DModelKill(HU3D_MODELID modelId);

void fn_1_83E4(void)
{
    Hu3DModelKill(lbl_1_bss_3E);
}
