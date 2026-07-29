#define _MATH_H

#include "dolphin.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/process.h"

extern HUPROCESS *lbl_1_bss_838;
extern s16 lbl_1_bss_66E[];
extern char lbl_1_data_472[];
extern char lbl_1_data_479[];
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_6C;
extern float lbl_1_rodata_70;

void fn_1_18D8(void)
{
    float rot;

    Hu3DAnimCreate(HuDataSelHeapReadNum(0xBE0022, HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_472);
    Hu3DAnimCreate(HuDataSelHeapReadNum(0xBE0023, HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_479);
    rot = lbl_1_rodata_20;
    while (TRUE) {
        Hu3DModelRotSet(lbl_1_bss_66E[1], lbl_1_rodata_20,
            lbl_1_rodata_20, rot);
        rot -= lbl_1_rodata_6C;
        if (rot < lbl_1_rodata_20) {
            rot += lbl_1_rodata_70;
        }
        HuPrcVSleep();
    }
}

void fn_1_19B8(s32 value)
{
    void **property = &lbl_1_bss_838->property;

    *property = (void *)value;
}
