/* The round timer: its countdown digits, the color pulse near the end, and its state. */
#include "REL/m679dll/module_types.h"

extern M679ScoreState lbl_1_bss_6160;
extern s32 lbl_1_data_C4;
extern s32 lbl_1_data_C8;

HUPROCESS *fn_1_A0(void);
void fn_1_3FA4(OMOBJ *obj);

void fn_1_D37C(OMOBJ *object)
{
    s32 hours;
    s32 minutes;
    s32 seconds;

    if (lbl_1_bss_6160.state != 0) {
        if (lbl_1_bss_6160.mode > 0) {
            lbl_1_bss_6160.mode--;
        }
        hours = (lbl_1_bss_6160.mode + 0x3B) / 3600;
        minutes = ((lbl_1_bss_6160.mode + 0x3B) - hours * 0xE10) / 60;
        seconds = (lbl_1_bss_6160.mode - hours * 0xE10) % 60;
        MgScoreValueSet(lbl_1_bss_6160.scoreA, hours);
        MgScoreValueSet(lbl_1_bss_6160.scoreB, minutes);
        if (hours == 0) {
            if (((minutes % 10) == 0) && (minutes != 0) && (seconds == 0)) {
                lbl_1_bss_6160.colorStep += 2;
            }
            lbl_1_bss_6160.colorCountdown--;
            if (lbl_1_bss_6160.colorCountdown == 0) {
                lbl_1_bss_6160.colorInterval--;
                if (lbl_1_bss_6160.colorInterval < 1) {
                    lbl_1_bss_6160.colorInterval = 1;
                }
                lbl_1_bss_6160.colorCountdown = lbl_1_bss_6160.colorInterval;
                if (lbl_1_bss_6160.colorIncreasing == 1) {
                    lbl_1_bss_6160.colorValue += lbl_1_bss_6160.colorStep;
                    if (lbl_1_bss_6160.colorValue > 0xFF) {
                        lbl_1_bss_6160.colorIncreasing = 0;
                        lbl_1_bss_6160.colorValue = 0xFF;
                    }
                } else {
                    lbl_1_bss_6160.colorValue -= lbl_1_bss_6160.colorStep;
                    if (lbl_1_bss_6160.colorValue < 0) {
                        lbl_1_bss_6160.colorIncreasing = 1;
                        lbl_1_bss_6160.colorValue = 0;
                    }
                }
            }
            MgScoreColorSet(lbl_1_bss_6160.scoreA, 0xFFU,
                lbl_1_bss_6160.colorValue, lbl_1_bss_6160.colorValue);
            MgScoreColorSet(lbl_1_bss_6160.scoreB, 0xFFU,
                lbl_1_bss_6160.colorValue, lbl_1_bss_6160.colorValue);
        }
    }
}

void fn_1_D6A0(void)
{
    lbl_1_bss_6160.state = 1;
}

void fn_1_D6B4(void)
{
    lbl_1_bss_6160.state = 0;
}

s32 fn_1_D6C8(void)
{
    s32 result;

    if (lbl_1_bss_6160.mode == 0) {
        result = 1;
    } else {
        result = 0;
    }
    return result;
}

void fn_1_D700(void)
{
    MgScoreBoxDispSet(lbl_1_bss_6160.scoreBox, 1);
    MgScoreDispOn(lbl_1_bss_6160.scoreA);
    MgScoreDispOn(lbl_1_bss_6160.scoreB);
}

void fn_1_D750(void)
{
    MgScoreBoxDispSet(lbl_1_bss_6160.scoreBox, 0);
    MgScoreDispOff(lbl_1_bss_6160.scoreA);
    MgScoreDispOff(lbl_1_bss_6160.scoreB);
}

void fn_1_D7A0(void)
{
    OMOBJ *obj;

    obj = fn_8003AB38(fn_1_A0(), 0x190, 0U, 0U, 0, fn_1_3FA4);
}

void fn_1_D7E0(void)
{
    s32 result;

    if (lbl_1_data_C4 == 1) {
        result = 0;
    } else {
        result = 1;
    }
    lbl_1_data_C4 = result;
}

s32 fn_1_D820(void)
{
    return lbl_1_data_C4;
}

s32 fn_1_D830(void)
{
    return lbl_1_data_C8;
}
