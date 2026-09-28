#define _MATH_H
#include "game/audio.h"

/* Extern views of observed target storage; original declarations/names unknown. */
extern s16 lbl_1_data_86E[6];
extern f32 lbl_1_bss_38[6];
extern const f32 lbl_1_rodata_70;

void fn_1_86F0(s16 arg0)
{
    lbl_1_bss_38[arg0] = lbl_1_rodata_70;
    if (lbl_1_data_86E[arg0] == 1) {
        HuAudFXPlay(0);
    }
}
