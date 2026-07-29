#include <dolphin/mtx/GeoTypes.h>

typedef s16 HUWINID;

extern s16 lbl_1_data_932[3];
extern s16 lbl_1_bss_198C[4];

void HuWinMesWait(HUWINID winId);

static inline void fn_1_A88(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_198C[winNo]);
}

void fn_1_11A0(void)
{
    if (lbl_1_data_932[0] != -1) {
        fn_1_A88(lbl_1_data_932[0]);
    }
}
