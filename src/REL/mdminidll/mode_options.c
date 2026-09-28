#define _MATH_H
#include "game/gamework.h"
extern s16 lbl_1_bss_28;
extern s16 lbl_1_bss_2A;
extern s16 lbl_1_data_6B6[6];
s16 fn_1_1A4(s16 mode);

void fn_1_558(void)
{
    lbl_1_bss_28 = 0;
    lbl_1_bss_2A = 0;
    if (GWBankFlagGet(2) != 0) lbl_1_bss_28 = 1;
    if (GWBankFlagGet(3) != 0) lbl_1_bss_2A = 1;
    lbl_1_data_6B6[0] = 1;
    lbl_1_data_6B6[1] = 1;
    lbl_1_data_6B6[2] = 1;
    lbl_1_data_6B6[3] = 1;
    lbl_1_data_6B6[4] = 1;
    lbl_1_data_6B6[5] = 0;
    if (GWBankFlagGet(7) != 0) lbl_1_data_6B6[5] = 1;
    lbl_1_data_6B6[1] = fn_1_1A4(1);
    lbl_1_data_6B6[2] = fn_1_1A4(2);
    lbl_1_data_6B6[3] = fn_1_1A4(3);
    lbl_1_data_6B6[4] = fn_1_1A4(4);
    if (lbl_1_data_6B6[5] == 1) lbl_1_data_6B6[5] = fn_1_1A4(5);
}
