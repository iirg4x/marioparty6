#define _MATH_H
#include "game/process.h"
#include "game/audio.h"
#include "game/wipe.h"
extern s16 lbl_1_bss_0;
extern s32 lbl_1_data_94;
void fn_1_17894(void);

s32 fn_1_17E78(void)
{
    HuPrcSleep(5);
    if (lbl_1_bss_0 == 1) fn_1_17894();
    lbl_1_data_94 = HuAudSStreamPlay(6);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) HuPrcVSleep();
    return 1;
}
