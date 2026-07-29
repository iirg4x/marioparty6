#include <dolphin/mtx/GeoTypes.h>

typedef s16 HU3D_MODELID;

extern HU3D_MODELID lbl_1_bss_1E2A;

void Hu3DModelKill(HU3D_MODELID modelId);

void fn_1_F03C(void)
{
    Hu3DModelKill(lbl_1_bss_1E2A);
}
