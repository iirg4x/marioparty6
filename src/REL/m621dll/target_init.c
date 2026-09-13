#include "REL/m621dll.h"
#include "game/data.h"
#include "game/memory.h"
#include "datadir_enum.h"

u32 fn_1_26A4(void)
{
    u32 mask;

    for (mask = 1U << 31; mask != 1; mask >>= 1) {
        if ((mask & lbl_1_data_2AC) != 0) {
            lbl_1_data_2AC &= ~mask;
            return mask;
        }
    }
    return 0;
}

void fn_1_270C(u32 mask)
{
    lbl_1_data_2AC |= mask;
}

s16 fn_1_272C(void)
{
    s16 count;
    u32 mask;

    count = 0;
    for (mask = 1U << 31; mask != 1; mask >>= 1) {
        if ((mask & lbl_1_data_2AC) != 0) {
            count++;
        }
    }
    return count > 23 ? 23 : count;
}

void fn_1_27A0(void)
{
    s32 i;

    lbl_1_bss_3E = Hu3DModelCreateData(DATANUM(DATA_m621, 12));
    lbl_1_bss_3C = Hu3DModelCreateData(DATANUM(DATA_m621, 13));
    lbl_1_bss_3A = Hu3DModelCreateData(DATANUM(DATA_m621, 14));
    lbl_1_bss_38 = Hu3DModelCreateData(lbl_1_data_74[fn_1_5340()][0]);
    lbl_1_bss_36 = Hu3DModelCreateData(lbl_1_data_74[fn_1_5340()][1]);
    lbl_1_bss_34 = Hu3DJointMotionData(lbl_1_bss_36, lbl_1_data_74[fn_1_5340()][2]);
    Hu3DModelDispOff(lbl_1_bss_3E);
    Hu3DModelDispOff(lbl_1_bss_3C);
    Hu3DModelDispOff(lbl_1_bss_38);
    Hu3DModelDispOff(lbl_1_bss_36);
    Hu3DModelDispOff(lbl_1_bss_3A);
    for (i = 0; i < 3; i++) {
        lbl_1_bss_40[i] = fn_1_A0(30, sizeof(M621Target), fn_1_29E8);
        lbl_1_bss_40[i]->targetNo = i;
        lbl_1_bss_40[i]->initialF = 1;
        lbl_1_bss_40[i]->model = Hu3DModelLink(lbl_1_bss_38);
    }
}

void fn_1_29E8(OMOBJ *obj)
{
    M621Target *target = obj->data;

    if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
        target->state = 0;
        target->timer = 150;
        target->positionNo = -1;
        fn_1_140(obj, fn_1_3678);
        return;
    }
}
