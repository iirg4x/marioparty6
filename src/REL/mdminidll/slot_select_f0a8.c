#define _MATH_H
#include "game/object.h"
#include "game/hsfex.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Consumed fields and record strides are target-backed. Unobserved bytes
 * retain unknown storage, not an assertion about the original declaration. */
typedef struct MDMinidllSlotConsumedView {
    s16 state;
    u8 unknown_02[2];
    s16 active;
    u8 unknown_06[2];
    s16 selection;
    u8 unknown_0A[4];
} MDMinidllSlotConsumedView;

typedef struct MDMinidllSelectionConsumedView {
    u8 unknown_00[16];
    s16 availability;
} MDMinidllSelectionConsumedView;

typedef struct MDMinidllPositionConsumedView {
    f32 x;
    f32 y;
    f32 z;
} MDMinidllPositionConsumedView;

typedef struct MDMinidllModelConsumedView {
    s16 model_id;
    u8 unknown_02[54];
} MDMinidllModelConsumedView;

extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_24;
extern MDMinidllSlotConsumedView lbl_1_bss_7CC[];
extern MDMinidllPositionConsumedView lbl_1_bss_50[];
extern s16 lbl_1_bss_752[];
extern f32 lbl_1_bss_38[];
extern MDMinidllModelConsumedView lbl_1_bss_230[];
extern MDMinidllSelectionConsumedView lbl_1_data_89A[];
extern Point3d lbl_1_data_4C8[];
extern s16 lbl_1_data_86E[];
extern const Point3d lbl_1_rodata_1B0;
extern const f32 lbl_1_rodata_24C;
extern const f32 lbl_1_rodata_250;
extern const f32 lbl_1_rodata_254;
void fn_1_E1C0(OMOBJ *obj);

void fn_1_F0A8(OMOBJ *arg0)
{
    Point3d sp20;
    Point3d sp14;
    Point3d sp8;
    MDMinidllSlotConsumedView *var_r31;
    s16 var_r30;
    Point3d *var_r29;
    OMOBJ *var_r28;
    s16 var_r27;

    var_r30 = 0;
    var_r31 = lbl_1_bss_7CC;
    while (var_r30 < 4) {
        if (var_r31->active == 1) {
            if (var_r31->state == 0) {
                var_r27 = 0;
                while (var_r27 < 11) {
                    if (lbl_1_data_89A[var_r27].availability == -1) {
                        var_r31->selection = var_r27;
                        lbl_1_data_89A[var_r27].availability = 1;
                    } else {
                        var_r27 += 1;
                        continue;
                    }
                    break;
                }
                var_r29 = &lbl_1_data_4C8[var_r31->selection + 14];
                var_r28 = lbl_1_bss_1C;
                Hu3DModelPosSet(var_r28->mdlId[var_r30], var_r29->x, var_r29->y, var_r29->z);
                Hu3DModelRotSet(var_r28->mdlId[var_r30], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_90);
                Hu3DModelScaleSet(var_r28->mdlId[var_r30], lbl_1_rodata_6C, lbl_1_rodata_6C, lbl_1_rodata_6C);
                lbl_1_bss_50[var_r30].x = var_r29->x - lbl_1_rodata_1C0;
                lbl_1_bss_50[var_r30].y = lbl_1_rodata_1C0 + var_r29->y;
                lbl_1_bss_50[var_r30].z = lbl_1_rodata_1C0 + var_r29->z;
                Hu3DModelAttrReset(var_r28->mdlId[var_r30], 1U);
                HuSprScaleSet(lbl_1_bss_752[3], (s16) var_r30, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
                HuSprAttrReset(lbl_1_bss_752[3], (s16) var_r30, 4);
                HuSprBankSet(lbl_1_bss_752[3], (s16) var_r30, 4);
                break;
            } else if (var_r31->state == 1) {
                Hu3DModelPosGet(lbl_1_bss_230[var_r31->selection].model_id, &sp20);
                sp14 = lbl_1_rodata_1B0;
                Hu3D3Dto2D(&sp20, 1, &sp14);
                HuSprPosSet(lbl_1_bss_752[2], 4, sp14.x + lbl_1_rodata_24C, sp14.y + lbl_1_rodata_250);
                HuSprScaleSet(lbl_1_bss_752[2], 4, lbl_1_rodata_70, lbl_1_rodata_70);
                HuSprAttrReset(lbl_1_bss_752[2], 4, 4);
                lbl_1_bss_38[4] = lbl_1_rodata_70;
                lbl_1_data_86E[4] = 1;
                sp8 = lbl_1_rodata_1B0;
                Hu3D3Dto2D(&sp20, 1, &sp8);
                HuSprPosSet(lbl_1_bss_752[2], 5, sp8.x + lbl_1_rodata_254, sp8.y + lbl_1_rodata_250);
                HuSprScaleSet(lbl_1_bss_752[2], 5, lbl_1_rodata_70, lbl_1_rodata_70);
                HuSprAttrReset(lbl_1_bss_752[2], 5, 4);
                lbl_1_bss_38[5] = lbl_1_rodata_70;
                lbl_1_data_86E[5] = 1;
                break;
            }
        }
        var_r30 += 1;
        var_r31 += 1;
    }
    arg0->work[0] = 10;
    arg0->objFunc = fn_1_E1C0;
    *lbl_1_bss_24->mtnId = 1;
}
