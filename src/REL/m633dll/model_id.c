#include "REL/m633dll.h"

void fn_1_36BC(OMOBJ *obj)
{
    Point3d sp8;
    s16 temp_r0;
    u32 temp_r31;

    temp_r31 = obj->work[0];
    temp_r0 = lbl_1_bss_0.unk040[temp_r31]->actor->mdlId;
    Hu3DModelPosGet(temp_r0, &sp8);
    sp8.x += 24.0f * lbl_1_bss_0.unk090[temp_r31].x;
    sp8.y += lbl_1_bss_0.unk0D0[temp_r31];
    sp8.z += 24.0f * lbl_1_bss_0.unk090[temp_r31].z;
    Hu3DModelPosSetV(temp_r0, &sp8);
    lbl_1_bss_0.unk0C0[temp_r31] += 1;
    lbl_1_bss_0.unk0D0[temp_r31] -= 1.2f;
    if ((s32) lbl_1_bss_0.unk0C0[temp_r31] >= 120) {
        obj->objFunc = fn_1_3828;
        Hu3DModelAttrSet(temp_r0, 1U);
    }
}
