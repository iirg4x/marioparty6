#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/hu3d.h"

/* The target call relocates both zero float arguments to this existing
 * read-only scalar; this TU consumes it but does not allocate storage. */
extern const f32 lbl_1_rodata_74;

void fn_1_6B74(OMOBJ *arg0)
{
    s16 var_r30;

    omSetStatBit(arg0, 256U);
    var_r30 = 0;
    while (var_r30 < 11) {
        arg0->mdlId[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(var_r30 + 9830454, 268435456, HEAP_MODEL));
        arg0->mtnId[var_r30 * 2] = Hu3DJointMotion(arg0->mdlId[var_r30], HuDataSelHeapReadNum(var_r30 + 9830465, 268435456, HEAP_MODEL));
        arg0->mtnId[(var_r30 * 2) + 1] = Hu3DJointMotion(arg0->mdlId[var_r30], HuDataSelHeapReadNum(var_r30 + 9830476, 268435456, HEAP_MODEL));
        Hu3DModelAttrSet(arg0->mdlId[var_r30], 1U);
        Hu3DModelLayerSet(arg0->mdlId[var_r30], 1);
        Hu3DMotionShiftSet(arg0->mdlId[var_r30], arg0->mtnId[var_r30 * 2], lbl_1_rodata_74, lbl_1_rodata_74, 1073741825U);
        Hu3DModelShadowSet(arg0->mdlId[var_r30]);
        var_r30 += 1;
    }
    arg0->objFunc = NULL;
}
