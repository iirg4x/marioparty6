#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

extern OMOBJ *lbl_1_bss_1C;
extern HUSPR_GROUPID lbl_1_bss_752[9];
extern s16 lbl_1_data_86E[6];
extern void fn_1_8C08(OMOBJ *arg0);

void fn_1_90C8(OMOBJ *arg0)
{
    s16 var_r30;
    s16 var_r29;
    OMOBJ *var_r28;
    s16 var_r27;

    omSetStatBit(arg0, 256U);
    var_r30 = 0;
    while (var_r30 < 4) {
        if (var_r30 == 0) {
            arg0->mdlId[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(9830420, 268435456, HEAP_MODEL));
            arg0->mtnId[var_r30] = Hu3DMotionIDGet(*arg0->mdlId);
        } else {
            arg0->mdlId[var_r30] = Hu3DModelLink(*arg0->mdlId);
        }
        Hu3DModelLayerSet(arg0->mdlId[var_r30], 1);
        Hu3DMotionShiftSet(arg0->mdlId[var_r30], *arg0->mtnId, lbl_1_rodata_74, lbl_1_rodata_74, 1073741825U);
        var_r28 = lbl_1_bss_1C;
        Hu3DModelAttrSet(var_r28->mdlId[var_r30], 1U);
        HuSprScaleSet(lbl_1_bss_752[3], (s16) var_r30, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
        HuSprAttrSet(lbl_1_bss_752[3], (s16) var_r30, 4);
        var_r30 += 1;
    }
    var_r29 = 0;
    while (var_r29 < 6) {
        var_r27 = var_r29;
        HuSprAttrSet(lbl_1_bss_752[2], var_r27, 4);
        lbl_1_data_86E[var_r27] = 0;
        lbl_1_data_86E[var_r29] = 0;
        var_r29 += 1;
    }
    arg0->objFunc = fn_1_8C08;
}
