#include "dolphin/types.h"
#include "game/window.h"

extern s16 lbl_1_data_A98;
extern s32 lbl_1_data_A9C[];
extern HUWINID lbl_1_bss_1398[];

void fn_1_3B30(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_1398[i]);
    }
    HuWinAllKill();
}

void fn_1_3B8C(s16 winNo)
{
    if (lbl_1_data_A98 != -1 && lbl_1_data_A98 != winNo) {
        s16 activeWin = lbl_1_data_A98;

        if (activeWin == 0) {
            HuWinDispOff(lbl_1_bss_1398[activeWin]);
        } else {
            HuWinExClose(lbl_1_bss_1398[activeWin]);
        }
    }
    if (lbl_1_data_A98 == -1 || lbl_1_data_A98 != winNo) {
        s16 activeWin;

        lbl_1_data_A98 = winNo;
        lbl_1_data_A9C[0] = -1;
        lbl_1_data_A9C[1] = -1;
        activeWin = lbl_1_data_A98;
        if (activeWin == 0) {
            HuWinDispOn(lbl_1_bss_1398[activeWin]);
        } else {
            HuWinExOpen(lbl_1_bss_1398[activeWin]);
        }
    }
}
