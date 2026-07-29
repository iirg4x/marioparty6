#include <dolphin/types.h>

typedef s16 HU3D_MODELID;

void Hu3DModelKill(HU3D_MODELID modelId);

extern HU3D_MODELID lbl_1_bss_1DE0[6][5];

void fn_1_11EB0(void)
{
    s16 group;
    s16 model;

    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelKill(lbl_1_bss_1DE0[group][model]);
        }
    }
}
