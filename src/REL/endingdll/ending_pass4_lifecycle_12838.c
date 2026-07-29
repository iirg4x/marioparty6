#include <dolphin/types.h>

extern s16 lbl_1_bss_1DE0[6][5];
extern s16 lbl_1_bss_1E1C;
extern s16 lbl_1_bss_1E1E[2];
extern s16 lbl_1_bss_1E22[2];
extern s16 lbl_1_bss_1E26[2];
extern s16 lbl_1_bss_1E2A;
extern s16 lbl_1_bss_1E2C;

void Hu3DModelKill(s16 modelId);

void fn_1_12838(void)
{
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 model;
    s16 group;

    Hu3DModelKill(lbl_1_bss_1E2C);
    Hu3DModelKill(lbl_1_bss_1E2A);
    for (firstIndex = 0; firstIndex < 2; firstIndex++) {
        Hu3DModelKill(lbl_1_bss_1E26[firstIndex]);
    }
    Hu3DModelKill(lbl_1_bss_1E1C);
    for (secondIndex = 0; secondIndex < 2; secondIndex++) {
        Hu3DModelKill(lbl_1_bss_1E22[secondIndex]);
    }
    for (thirdIndex = 0; thirdIndex < 2; thirdIndex++) {
        Hu3DModelKill(lbl_1_bss_1E1E[thirdIndex]);
    }
    for (group = 0; group < 6; group++) {
        for (model = 0; model < 5; model++) {
            Hu3DModelKill(lbl_1_bss_1DE0[group][model]);
        }
    }
}
