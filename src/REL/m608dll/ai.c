#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

char lbl_1_data_4C8[40] = "com(%d) ... level : %d, interval : %d\012\000";

void fn_1_69F4(void)
{
    u32 temp_r30;
    s32 var_r31;
    u32 temp_r29;

    var_r31 = 0;
    while (var_r31 < 4) {
        if (GwPlayerConf[var_r31].type == 1) {
            lbl_1_bss_3E80.difficulty[var_r31] = GwPlayerConf[var_r31].comDif;
            temp_r29 = lbl_1_data_56C[(lbl_1_bss_3E80.difficulty[var_r31] * 2) + 1];
            temp_r30 = frand() % temp_r29;
            lbl_1_bss_3E80.base[var_r31] = (s16) lbl_1_data_56C[lbl_1_bss_3E80.difficulty[var_r31] * 2];
            lbl_1_bss_3E80.interval[var_r31] = (s16) (300U / (lbl_1_bss_3E80.base[var_r31] + temp_r30));
            lbl_1_bss_3E80.counter[var_r31] = 0;
            OSReport(lbl_1_data_4C8, var_r31, lbl_1_bss_3E80.difficulty[var_r31], lbl_1_bss_3E80.interval[var_r31]);
        }
        var_r31 += 1;
    }
}

u32 fn_1_6B90(s32 player)
{
    s16 interval;
    s16 count;
    unsigned int buttons;
    int trigger;

    interval = lbl_1_bss_3E80.interval[player];
    count = lbl_1_bss_3E80.counter[player];
    buttons = 0;
    trigger = 0;
    count++;
    if (interval <= count) {
        trigger = 1;
        count = 0;
    } else if (frandmod(1000) < 10U) {
        trigger = 1;
        count = 0;
    }
    if (trigger != 0) {
        switch (lbl_1_bss_3E80.scores[player] % 4) {
        case 0: buttons |= 256; break;
        case 1: buttons |= 512; break;
        case 2: buttons |= 2048; break;
        case 3: buttons |= 1024; break;
        }
    }
    lbl_1_bss_3E80.counter[player] = count;
    return buttons;
}
