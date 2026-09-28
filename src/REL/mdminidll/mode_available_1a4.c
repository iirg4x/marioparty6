#include "game/gamework.h"

/* Target-backed halfword tables, declared as consumed views only. */
extern s16 lbl_1_data_0[];
extern s16 lbl_1_data_2E[];
extern s16 lbl_1_data_44[];
extern s16 lbl_1_data_5C[];
extern s16 lbl_1_data_7C[];

s16 fn_1_1A4(s16 selector)
{
    s16 index;
    s16 result;

    result = 0;
    switch (selector) {
    case 0:
        result = 1;
        break;
    default:
        break;
    case 1:
        index = 0;
loop_11:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto loop_11;
        }
        index = 0;
loop_16:
        if (lbl_1_data_2E[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_2E[index]) != 1) {
            index += 1;
            goto loop_16;
        }
        index = 0;
loop_21:
        if (lbl_1_data_44[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_44[index]) != 1) {
            index += 1;
            goto loop_21;
        }
        result = 1;
        break;
    case 2:
        index = 0;
loop_27:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto loop_27;
        }
        result = 1;
        break;
    case 3:
        index = 0;
loop_33:
        if (lbl_1_data_5C[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_5C[index]) != 1) {
            index += 1;
            goto loop_33;
        }
        result = 1;
        break;
    case 4:
        index = 0;
loop_39:
        if (lbl_1_data_7C[index] == -1) {
            return 1;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_7C[index]) != 0) {
            index += 1;
            goto loop_39;
        }
        result = 0;
        break;
    case 5:
        index = 0;
loop_45:
        if (lbl_1_data_0[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_0[index]) != 1) {
            index += 1;
            goto loop_45;
        }
        index = 0;
loop_50:
        if (lbl_1_data_2E[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_2E[index]) != 1) {
            index += 1;
            goto loop_50;
        }
        index = 0;
loop_55:
        if (lbl_1_data_5C[index] == -1) {
            return 0;
        }
        if (GWMgUnlockGet((s32) lbl_1_data_5C[index]) != 1) {
            index += 1;
            goto loop_55;
        }
        result = 1;
        break;
    }
    return result;
}
