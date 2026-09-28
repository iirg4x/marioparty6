#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern HUPROCESS *lbl_1_bss_4;
extern void *lbl_1_bss_888;

void fn_1_1FDCC(OMOBJ *arg0)
{
    HU3D_MODEL *model;

    model = NULL;
    Hu3DLayerHookReset(1);
    if (arg0) {
        model = &Hu3DData[arg0->mdlId[0]];
        model->hookData = lbl_1_bss_8E8;
        Hu3DModelKill(arg0->mdlId[0]);
        if (lbl_1_bss_888) {
            HuMemDirectFree(lbl_1_bss_888);
        }
        lbl_1_bss_888 = NULL;
    }
    arg0 = NULL;
}
