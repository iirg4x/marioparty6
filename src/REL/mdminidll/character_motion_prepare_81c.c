#define _MATH_H
#include "game/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/process.h"
#include "game/armem.h"
#include "REL/mdminidll/player_config.h"

/* Consumed view only: retail copies six 32-bit data-directory IDs (24 bytes)
 * from lbl_1_rodata_54 to its local stack record before indexed use. */
typedef struct MDMinidllMotionDirectoryIds {
    s32 value[6];
} MDMinidllMotionDirectoryIds;

extern const MDMinidllMotionDirectoryIds lbl_1_rodata_54;
extern s16 lbl_1_bss_2C;

void fn_1_81C(void)
{
    MDMinidllMotionDirectoryIds motionDirectoryIds;
    s16 var_r31;
    s32 var_r30;
    s32 var_r29;

    motionDirectoryIds = lbl_1_rodata_54;
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
            var_r29 = HuDataDirReadAsync(CharDataDirTbl[GwPlayer[var_r31].charNo][4]);
            if (var_r29 != -1) {
                while (HuDataGetAsyncStat(var_r29) == 0) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit((s16) GwPlayer[var_r31].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[var_r31].charNo][4]);
            var_r31 += 1;
        }
    }
    if (lbl_1_bss_804[1] <= 4) {
        var_r30 = HuDataDirReadAsync(motionDirectoryIds.value[lbl_1_bss_804[1]]);
        if (var_r30 != -1) {
            while (HuDataGetAsyncStat(var_r30) == 0) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(motionDirectoryIds.value[lbl_1_bss_804[1]]);
        while (HuARDMACheck() != 0) {
            HuPrcVSleep();
        }
        HuDataDirClose(motionDirectoryIds.value[lbl_1_bss_804[1]]);
    }
    lbl_1_bss_2C = 1;
    HuPrcEnd();
loop_22:
    HuPrcVSleep();
    goto loop_22;
}
