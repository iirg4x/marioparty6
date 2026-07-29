#include <dolphin/types.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef s16 HU3D_MODELID;

void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);

extern HU3D_MODELID lbl_1_bss_1E1C;
extern HU3D_MODELID lbl_1_bss_1DE0[6][5];

void fn_1_1144C(void)
{
    Hu3DModelKill(lbl_1_bss_1E1C);
}

void fn_1_11478(s16 group, s16 display)
{
    s16 model;

    for (model = 0; model < 5; model++) {
        if (display != 0) {
            Hu3DModelAttrReset(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_1DE0[group][model],
                HU3D_ATTR_DISPOFF);
        }
    }
}
