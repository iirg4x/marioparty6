#include <dolphin/mtx/GeoTypes.h>

#define HUWIN_ATTR_NOCANCEL 0x10

typedef s16 HUWINID;

extern s16 lbl_1_data_932[3];
extern HUWINID lbl_1_bss_198C[4];

void HuWinAttrSet(HUWINID windowId, u32 attr);
void HuWinAttrReset(HUWINID windowId, u32 attr);
s16 HuWinChoiceGet(HUWINID windowId, s16 choice);

static inline s16 fn_1_AC4(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_198C[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_198C[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

s16 fn_1_1200(s16 mode)
{
    if (lbl_1_data_932[0] != -1) {
        return fn_1_AC4(lbl_1_data_932[0], mode);
    }
    return 0;
}
