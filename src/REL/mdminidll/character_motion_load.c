#define _MATH_H
#include "game/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/process.h"

void fn_1_6CC(void)
{
    s16 var_r31;
    s32 var_r30;

    var_r31 = 0;
    while (var_r31 < 4) {
        if ((void *) CharMotionAMemPGet((s16) GwPlayer[var_r31].charNo) == NULL) {
            break;
        }
        var_r31 += 1;
    }
    if (var_r31 != 4) {
        CharDataClose(-1);
        var_r31 = 0;
        while (var_r31 < 4) {
            var_r30 = HuDataDirReadAsync(CharDataDirTbl[GwPlayer[var_r31].charNo][4]);
            if (var_r30 != -1) {
                while (HuDataGetAsyncStat(var_r30) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[var_r31].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[var_r31].charNo][4]);
            var_r31 += 1;
        }
    }
}
