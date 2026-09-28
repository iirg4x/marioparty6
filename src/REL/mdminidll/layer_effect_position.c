#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
extern const f32 lbl_1_rodata_288;
extern const f32 lbl_1_rodata_298;
extern const f32 lbl_1_rodata_2C4;
extern void fn_1_1F508(s16 layerNo);
extern void fn_1_1F9F4(OMOBJ *obj);

void fn_1_1FFA8(Point3d *arg0)
{
    OMOBJ *obj = lbl_1_bss_87C;

    if (obj) {
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->field_1C =
            lbl_1_rodata_288;
        ((MDMinidllLayerEffectStateView *) lbl_1_bss_8E8)->field_24 =
            lbl_1_rodata_298;
        Hu3DModelPosSet(*obj->mdlId, arg0->x, arg0->y, lbl_1_rodata_2C4);
        Hu3DModelAttrReset(*obj->mdlId, 1U);
        Hu3DLayerHookSet(1, fn_1_1F508);
        obj->objFunc = fn_1_1F9F4;
    }
}
