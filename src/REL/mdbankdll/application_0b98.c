#include <dolphin/mtx/GeoTypes.h>

#define HUWIN_ATTR_ALIGN_CENTER (1 << 11)

typedef s16 HUWINID;

extern u32 lbl_1_data_90C;
extern s16 lbl_1_bss_198C[4];

void HuWinAttrSet(HUWINID winId, u32 attr);
void HuWinMesSpeedSet(HUWINID winId, s16 mesSpeed);
void HuWinMesSet(HUWINID winId, u32 messNum);

void fn_1_B98(s16 winNo, u32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_198C[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_198C[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_198C[winNo], speed);
    if (lbl_1_data_90C != messNum) {
        lbl_1_data_90C = -1;
    }
}
