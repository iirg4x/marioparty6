#include <dolphin/mtx/GeoTypes.h>

typedef s16 HUWINID;

extern s16 lbl_1_data_932[3];
extern s32 lbl_1_data_938[3];
extern s16 lbl_1_bss_198C[4];

void HuWinHomeClear(HUWINID winId);
void HuWinDispOff(HUWINID winId);
void HuWinDispOn(HUWINID winId);
void HuWinExOpen(HUWINID winId);
void HuWinExClose(HUWINID winId);

static inline void fn_1_9A8(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_198C[winNo]);
    }
}

static inline void fn_1_A18(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_198C[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_198C[winNo]);
    }
}

void fn_1_F68(s16 winNo)
{
    if (lbl_1_data_932[0] != -1 && lbl_1_data_932[0] != winNo) {
        HuWinHomeClear(lbl_1_data_932[0]);
        fn_1_A18(lbl_1_data_932[0]);
    }
    if (lbl_1_data_932[0] == -1 || lbl_1_data_932[0] != winNo) {
        lbl_1_data_932[0] = winNo;
        lbl_1_data_938[0] = -1;
        lbl_1_data_938[1] = -1;
        fn_1_9A8(lbl_1_data_932[0]);
    }
}
