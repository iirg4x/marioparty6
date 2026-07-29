#include "dolphin/types.h"

#define HUWIN_ATTR_NOCANCEL (1 << 4)

typedef s16 HUWINID;

extern HUWINID lbl_1_bss_1398[];

s16 HuWinChoiceGet(HUWINID winId, s16 choiceNo);

void fn_1_35EC(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_365C(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_1398[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_1398[winNo]);
    }
}

void fn_1_36CC(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_1398[winNo]);
}

s16 fn_1_3708(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_1398[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_1398[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}
