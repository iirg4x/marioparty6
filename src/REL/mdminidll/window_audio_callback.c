#define _MATH_H
#include "REL/mdminidll/window_callback.h"
#include "game/audio.h"
#include "dolphin/os.h"

typedef struct MDMinidllAudioTable {
    s32 value[16];
} MDMinidllAudioTable;

extern const s32 lbl_1_rodata_10;
extern const MDMinidllAudioTable lbl_1_rodata_14;
extern u32 lbl_1_data_690;
extern const char lbl_1_data_694[];

void fn_1_0(HUWINID winId, u32 arg1, s16 arg2)
{
    s32 sentinel[1];
    MDMinidllAudioTable table;
    s16 var_r30;

    sentinel[0] = lbl_1_rodata_10;
    table = lbl_1_rodata_14;
    arg2 -= 1;
    OSReport(lbl_1_data_694, (s16) arg2);
    if ((u32) lbl_1_data_690 != arg1) {
        lbl_1_data_690 = arg1;
        var_r30 = 0;
loop_2:
        if (sentinel[var_r30] == -1) {
            HuAudFXPlay(table.value[arg2]);
            return;
        }
        if (arg1 == (u32) sentinel[var_r30]) {
            if (arg2 >= 8) {
                HuAudFXPlayPan(table.value[arg2], 80);
                return;
            }
            HuAudFXPlayPan(table.value[arg2], 48);
            return;
        }
        var_r30 += 1;
        goto loop_2;
    }
}
