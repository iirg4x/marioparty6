#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "REL/mdminidll/layer_effect_state.h"

extern s16 lbl_1_bss_8E4;
extern s16 lbl_1_bss_8DC[4];
extern s16 lbl_1_bss_8DA;
extern s16 lbl_1_bss_8D2[4];
extern s16 lbl_1_bss_8CE[2];
extern s16 lbl_1_bss_8CA[2];
extern s16 lbl_1_bss_88E[6][5];
extern OMOBJ *lbl_1_bss_87C;
extern void *lbl_1_bss_888;

void fn_1_257D0(void)
{
    OMOBJ *var_r31;
    s16 var_r30;
    s16 var_r29;
    s16 var_r28;
    s16 var_r27;
    s16 var_r26;
    s16 var_r25;
    HU3D_MODEL *var_r24;

    Hu3DModelKill(lbl_1_bss_8E4);
    var_r30 = 0;
    while (var_r30 < 4) {
        Hu3DModelKill(lbl_1_bss_8DC[var_r30]);
        var_r30 += 1;
    }
    Hu3DModelKill(lbl_1_bss_8DA);
    var_r29 = 0;
    while (var_r29 < 4) {
        Hu3DModelKill(lbl_1_bss_8D2[var_r29]);
        var_r29 += 1;
    }
    var_r28 = 0;
    while (var_r28 < 2) {
        Hu3DModelKill(lbl_1_bss_8CE[var_r28]);
        var_r28 += 1;
    }
    var_r27 = 0;
    while (var_r27 < 2) {
        Hu3DModelKill(lbl_1_bss_8CA[var_r27]);
        var_r27 += 1;
    }
    var_r25 = 0;
    while (var_r25 < 6) {
        var_r26 = 0;
        while (var_r26 < 5) {
            Hu3DModelKill(lbl_1_bss_88E[var_r25][var_r26]);
            var_r26 += 1;
        }
        var_r25 += 1;
    }
    Hu3DLayerHookReset(14);
    if ((OMOBJ *) lbl_1_bss_87C) {
        var_r31 = lbl_1_bss_87C;
        var_r24 = NULL;
        Hu3DLayerHookReset(1);
        if (var_r31) {
            var_r24 = &Hu3DData[*var_r31->mdlId];
            var_r24->hookData = lbl_1_bss_8E8;
            Hu3DModelKill(*var_r31->mdlId);
            if ((void *) lbl_1_bss_888) {
                HuMemDirectFree(lbl_1_bss_888);
            }
            lbl_1_bss_888 = NULL;
        }
        var_r31 = NULL;
    }
    lbl_1_bss_87C = NULL;
}
