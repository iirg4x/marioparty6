#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;
/* Existing readonly scalar; original source declaration/name is unknown. */
extern const f32 lbl_1_rodata_288;
extern void fn_1_20068(s16 layerNo);

void fn_1_26394(void)
{
    HU3D_PARTICLE_DATA *var_r31;
    HU3D_PARTICLE *var_r30;
    s16 var_r28;
    HU3D_MODEL *var_r29;

    Hu3DLayerHookSet(14, fn_1_20068);
    var_r29 = &Hu3DData[lbl_1_bss_8DA];
    var_r30 = var_r29->hookData;
    var_r28 = 0;
    var_r31 = var_r30->data;
    while (var_r28 < var_r30->maxCnt) {
        var_r31->time = 0;
        var_r31->scale = lbl_1_rodata_288;
        var_r28 += 1;
        var_r31 += 1;
    }
    var_r30->dataCnt = 1;
    var_r29->attr &= 4294967294;
}
