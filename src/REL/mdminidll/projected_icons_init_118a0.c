#define _MATH_H
#include "game/object.h"
#include "game/audio.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_752[];
extern OMOBJ *lbl_1_bss_24;
extern f32 lbl_1_bss_38[];
extern Point3d lbl_1_data_4C8[];
extern s16 lbl_1_data_86E[];
extern const Point3d lbl_1_rodata_1B0;
/* Target-backed scalar views; storage remains with the module data owner. */
extern const f32 lbl_1_rodata_25C;
extern const f32 lbl_1_rodata_260;
extern void fn_1_111C0(OMOBJ *obj);

void fn_1_118A0(OMOBJ *obj) {
    Point3d sp14;
    Point3d sp8;
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 6) {
        lbl_1_bss_230[var_r31 + 11].unknown_30 = 0;
        var_r31 += 1;
    }
    sp14 = lbl_1_rodata_1B0;
    if (lbl_1_data_4C8) {
        Hu3D3Dto2D((Point3d *) lbl_1_data_4C8, 1, &sp14);
    }
    HuSprPosSet(lbl_1_bss_752[2], 0, sp14.x + lbl_1_rodata_25C, sp14.y + lbl_1_rodata_74);
    HuSprScaleSet(lbl_1_bss_752[2], 0, lbl_1_rodata_70, lbl_1_rodata_70);
    HuSprAttrReset(lbl_1_bss_752[2], 0, 4);
    lbl_1_bss_38[0] = (f32) lbl_1_rodata_70;
    lbl_1_data_86E[0] = 1;
    sp8 = lbl_1_rodata_1B0;
    if (lbl_1_data_4C8) {
        Hu3D3Dto2D((Point3d *) lbl_1_data_4C8, 1, &sp8);
    }
    HuSprPosSet(lbl_1_bss_752[2], 1, sp8.x + lbl_1_rodata_260, sp8.y + lbl_1_rodata_74);
    HuSprScaleSet(lbl_1_bss_752[2], 1, lbl_1_rodata_70, lbl_1_rodata_70);
    HuSprAttrReset(lbl_1_bss_752[2], 1, 4);
    lbl_1_bss_38[1] = (f32) lbl_1_rodata_70;
    lbl_1_data_86E[1] = 1;
    obj->work[0] = 10;
    obj->objFunc = fn_1_111C0;
    *lbl_1_bss_24->mtnId = 1;
}
