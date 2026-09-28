#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern OMOBJ *lbl_1_bss_87C;
extern void *lbl_1_bss_888;

void fn_1_1FECC(void)
{
    OMOBJ *obj;
    HU3D_MODEL *model;
    if (lbl_1_bss_87C) {
        obj = lbl_1_bss_87C;
        model = NULL;
        Hu3DLayerHookReset(1);
        if (obj) {
            model = &Hu3DData[*obj->mdlId];
            model->hookData = lbl_1_bss_8E8;
            Hu3DModelKill(*obj->mdlId);
            if (lbl_1_bss_888) {
                HuMemDirectFree(lbl_1_bss_888);
            }
            lbl_1_bss_888 = NULL;
        }
        /* The local clear and following global clear are distinct in retail. */
        obj = NULL;
    }
    lbl_1_bss_87C = NULL;
}
