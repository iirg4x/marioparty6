#include "REL/m635dll.h"
#include "game/charman.h"

void fn_1_31BC(s16 charNo)
{
    int i;

    for (i = 0; i < 2; i++) {
        CharModelCryCreate(charNo, 30.0f, 0.8f);
    }
}

void fn_1_3218(s16 player, s16 motion, float time)
{
    s16 charNo;

    if (motion < 0 || motion >= 6) {
        return;
    }
    charNo = lbl_1_bss_BC[player].charNo;
    CharMotionShiftSet(charNo, lbl_1_bss_BC[player].motions[motion],
        0.0f, time, lbl_1_data_158[motion].attr);
}
