#include "dolphin/types.h"

#define HUWIN_ATTR_ALIGN_CENTER (1 << 11)

typedef s16 HUWINID;

extern HUWINID lbl_1_bss_1398[];
extern s32 lbl_1_data_8EC;

void fn_1_37DC(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_1398[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1398[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_1398[winNo], speed);
    if (lbl_1_data_8EC != messNum) {
        lbl_1_data_8EC = -1;
    }
}

void fn_1_3898(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_1398[winNo]);
    HuWinInsertMesSet(lbl_1_bss_1398[winNo], messNum, insertPos);
}
