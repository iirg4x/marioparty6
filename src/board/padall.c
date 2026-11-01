#include "game/pad.h"

u16 fn_80184DD4(void)
{
    u16 btn = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (!HuPadStatGet(i)) {
            btn |= lbl_802BFFE0[i];
        }
    }
    return btn;
}

u16 mbPadBtnDownGetAll(void)
{
    u16 btn = 0;
    int i;
    for (i = 0; i < 4; i++) {
        if (!HuPadStatGet(i)) {
            btn |= lbl_802C0008[i];
        }
    }
    return btn;
}
