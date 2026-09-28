#define _MATH_H
#include "game/hu3d.h"

extern s16 lbl_1_bss_8DA;
/* Recovered view of the existing four-byte readonly scalar at rodata+0x288.
 * Its original declaration/name is unknown; this introduces no new storage. */
extern const f32 lbl_1_rodata_288;

void fn_1_217BC(void)
{
    HU3D_PARTICLE *var_r31;
    HU3D_PARTICLE_DATA *var_r30;
    s16 var_r28;
    HU3D_MODEL *var_r29;

    var_r29 = &Hu3DData[lbl_1_bss_8DA];
    var_r31 = var_r29->hookData;
    var_r28 = 0;
    var_r30 = var_r31->data;
    while (var_r28 < var_r31->maxCnt) {
        var_r30->time = 0;
        var_r30->scale = lbl_1_rodata_288;
        var_r28 += 1;
        var_r30 += 1;
    }
    var_r31->dataCnt = 1;
    var_r29->attr &= 4294967294;
}
