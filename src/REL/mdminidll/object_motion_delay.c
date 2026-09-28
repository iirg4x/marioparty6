#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"

extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_70;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_BC;

void fn_1_4D98(OMOBJ *arg0)
{
    Hu3DMotionSpeedSet(*arg0->mdlId, lbl_1_rodata_70);
    if (arg0->work[3]++ > 30U) {
        arg0->objFunc = NULL;
        Hu3DMotionSpeedSet(*arg0->mdlId, lbl_1_rodata_6C);
        Hu3DMotionShiftSet(*arg0->mdlId, *arg0->mtnId, lbl_1_rodata_74,
                           lbl_1_rodata_BC, 1073741825U);
    }
}
