#include <dolphin/mtx/GeoTypes.h>

extern s16 lbl_1_data_932[3];
extern s32 lbl_1_data_938[3];
extern s16 lbl_1_bss_198C[4];

void HuWinDispOff(s16 windowId);
void HuWinExClose(s16 windowId);

static inline void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

void fn_1_10E4(void)
{
    if (lbl_1_data_932[0] != -1) {
        fn_1_A18(lbl_1_data_932[0]);
    }
    lbl_1_data_932[0] = -1;
    lbl_1_data_938[0] = -1;
    lbl_1_data_938[1] = -1;
}
