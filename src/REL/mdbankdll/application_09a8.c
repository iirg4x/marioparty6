#include <dolphin/mtx/GeoTypes.h>

typedef s16 HUWINID;

extern s16 lbl_1_bss_198C[4];

void HuWinMesWait(HUWINID winId);
void HuWinDispOff(HUWINID winId);
void HuWinDispOn(HUWINID winId);
void HuWinExOpen(HUWINID winId);
void HuWinExClose(HUWINID winId);

void fn_1_9A8(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_198C[winNo]);
    }
}

void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

void fn_1_A88(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_198C[winNo]);
}
