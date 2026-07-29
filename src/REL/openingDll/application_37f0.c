#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;

extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_1C;
extern const float lbl_1_rodata_38;
extern const double lbl_1_rodata_98;
extern const double lbl_1_rodata_A8;
extern const double lbl_1_rodata_140;
extern const double lbl_1_rodata_148;
extern const float lbl_1_rodata_150;

extern HuVecF lbl_1_bss_14;
extern HU3D_MODELID lbl_1_bss_1CE[8];

double sin(double value);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void HuPrcVSleep(void);

void fn_1_37F0(void)
{
    HuVecF position;
    float phase;

    phase = lbl_1_rodata_14;
    while (TRUE) {
        position = lbl_1_bss_14;
        position.x += lbl_1_rodata_140
            * sin(lbl_1_rodata_98 * (phase * lbl_1_rodata_1C)
                / lbl_1_rodata_A8);
        position.y += lbl_1_rodata_148
            * sin(lbl_1_rodata_98 * phase / lbl_1_rodata_A8);
        Hu3DModelPosSetV(lbl_1_bss_1CE[5], &position);
        Hu3DModelPosSetV(lbl_1_bss_1CE[6], &position);
        phase += lbl_1_rodata_38;
        if (phase > lbl_1_rodata_150) {
            phase -= lbl_1_rodata_150;
        }
        HuPrcVSleep();
    }
}
