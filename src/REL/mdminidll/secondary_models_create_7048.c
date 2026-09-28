#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_0;
extern s16 lbl_1_bss_752[];

void fn_1_7048(OMOBJ *arg0)
{
    s16 var_r30;
    s16 var_r29;
    HUSPR_GROUP *var_r28;
    s16 var_r27;

    omSetStatBit(arg0, 256U);
    var_r30 = 0;
    while (var_r30 < 3) {
        arg0->mdlId[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r30 + 9830400, 268435456, HEAP_MODEL));
        arg0->mtnId[var_r30] = Hu3DMotionIDGet(arg0->mdlId[var_r30]);
        Hu3DModelLayerSet(arg0->mdlId[var_r30], 1);
        Hu3DMotionShiftSet(arg0->mdlId[var_r30], arg0->mtnId[var_r30], lbl_1_rodata_74, lbl_1_rodata_74, 1073741825U);
        Hu3DModelShadowMapSet(arg0->mdlId[var_r30]);
        var_r30 += 1;
    }
    if (lbl_1_bss_0 == 0) {
        HuSprGrpPosSet(lbl_1_bss_752[0], lbl_1_rodata_18C, lbl_1_rodata_190);
        var_r27 = lbl_1_bss_752[0];
        var_r28 = &HuSprGrpData[var_r27];
        var_r29 = 0;
        while (var_r29 < var_r28->sprNum) {
            HuSprAttrReset(var_r27, var_r29, 4);
            var_r29 += 1;
        }
    }
    arg0->objFunc = NULL;
}
