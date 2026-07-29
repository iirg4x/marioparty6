#include "dolphin/types.h"
#include "game/window.h"

extern s16 lbl_1_data_A98;
extern HUWINID lbl_1_bss_1398[];

void fn_1_3DB4(void)
{
    if (lbl_1_data_A98 != -1) {
        s16 winNo = lbl_1_data_A98;

        HuWinMesWait(lbl_1_bss_1398[winNo]);
    }
}
