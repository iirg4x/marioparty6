/* Keep this unit's verified legacy call visibility for the MWCC build.
 * The fade result is discarded; host builds use the shared typed audio API. */
#ifndef __MWERKS__
#include "game/audio.h"
#endif


#pragma section code_type ".text.scalar_root"
#define _MATH_H
#define _MATH_H
#include "game/hu3d.h"

/* Consumed view of an existing four-byte readonly scalar; the original source
 * declaration and name are unknown. This declaration defines no storage. */
extern const f32 lbl_1_rodata_6C;

#define _MATH_H
#include "game/hu3d.h"

/* Existing four-byte readonly scalar; original declaration/name unknown.
 * No storage is introduced by this consumed view. */
extern const f32 lbl_1_rodata_74;

#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"

extern OMOBJ *lbl_1_bss_10;
/* Existing target readonly storage; original source declaration/name unknown. */
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_A0;

void fn_1_4D98(OMOBJ *obj);

#define _MATH_H
#include "REL/mdminidll/camera.h"
#include "string.h"

extern s16 lbl_1_bss_0;
extern OMOBJMAN *lbl_1_bss_4;

extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_A0;
extern const f32 lbl_1_rodata_C8;
extern const f32 lbl_1_rodata_CC;
extern const f32 lbl_1_rodata_D0;
extern const f32 lbl_1_rodata_D4;
extern const f32 lbl_1_rodata_D8;
extern const f32 lbl_1_rodata_DC;
extern const f32 lbl_1_rodata_E0;
extern const f32 lbl_1_rodata_E4;
extern const f32 lbl_1_rodata_E8;
extern const f32 lbl_1_rodata_EC;

void fn_1_2FD4(OMOBJ *obj);

#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "math.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common_Embedded/Math/fdlibm.h"

typedef struct {
    Point3d point[2];
} EndpointPair;

extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_74;
extern const f64 lbl_1_rodata_78;
extern const f64 lbl_1_rodata_88;
extern const f32 lbl_1_rodata_90;
extern const f32 lbl_1_rodata_94;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_9C;
extern const f32 lbl_1_rodata_A0;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C8;
extern const f32 lbl_1_rodata_168;
extern const f32 lbl_1_rodata_16C;
extern const EndpointPair lbl_1_rodata_150;

#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "string.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/readonly_scalars.h"

u32 fn_1_267B0(double value);

/* Existing interpolation provider, made visible at its observed call site. */

#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/readonly_scalars.h"

extern s16 lbl_1_bss_752[];
extern const f32 lbl_1_rodata_194;

/* The existing interpolation body is visible to its nested callers. */

#pragma section code_type ".text"
#pragma section const_type ".rodata.opacity_one"
f32 fn_1_1310(f32 farg0, f32 farg1, f32 farg2)
{
    if (farg0 == farg1) {
        return farg1;
    }
    return (farg1 + (farg0 * (farg2 - 1.0f))) / farg2;
}

#pragma section code_type ".text.opacity_156c"
#pragma section const_type ".rodata.opacity_zero"
f32 fn_1_156C(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    if (farg2 <= 0.0f) {
        return farg0;
    }
    if (farg2 >= farg3) {
        return farg1;
    }
    return farg0 + ((farg2 / farg3) * (farg1 - farg0));
}

#pragma section code_type ".text.opacity_4e40"
#pragma section const_type ".rodata.opacity_ten"
void fn_1_4E40(void)
{
    OMOBJ *obj;

    obj = lbl_1_bss_10;
    Hu3DMotionShiftSet(*obj->mdlId, obj->mtnId[3], lbl_1_rodata_74, 10.0f, 0U);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4D98;
}

#pragma section code_type ".text.opacity_30a4"
#pragma section const_type ".rodata.opacity_thirty"
void fn_1_30A4(MDMinidllCameraCallback callback)
{
    MDMinidllCameraState *var_r31;

    var_r31 = &lbl_1_bss_808;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, lbl_1_rodata_A0, lbl_1_rodata_CC, lbl_1_rodata_D0);
    Hu3DCameraViewportSet(1, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_D4, lbl_1_rodata_D8, lbl_1_rodata_74, lbl_1_rodata_6C);
    memset(var_r31, 0, 80U);
    var_r31->callback = callback;
    var_r31->center.x = lbl_1_rodata_74;
    var_r31->center.y = lbl_1_rodata_DC;
    var_r31->center.z = lbl_1_rodata_E0;
    var_r31->rotation.x = lbl_1_rodata_E4;
    var_r31->rotation.y = lbl_1_rodata_74;
    var_r31->rotation.z = lbl_1_rodata_74;
    if (lbl_1_bss_0 == 0) {
        var_r31->zoom = lbl_1_rodata_E8;
    } else {
        var_r31->zoom = lbl_1_rodata_EC;
    }
    var_r31->outViewObj = omAddObjEx(lbl_1_bss_4, 32730, 16U, 16U, -1, fn_1_2FD4);
}

#pragma section code_type ".text.opacity_55b0"
#pragma section const_type ".rodata.opacity_bias"
void fn_1_55B0(OMOBJ *arg0)
{
    EndpointPair sp58;
    Point3d sp4C;
    Point3d sp40;
    Point3d sp34;
    Point3d sp28;
    s16 var_r30;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;
    f32 var_f25;
    f32 var_f24;
    f32 var_f22;
    f32 var_f21;
    f32 var_f20;
    f32 var_f19;
    f32 var_f18;

    sp58 = lbl_1_rodata_150;
    var_f31 = (f32) arg0->work[0];
    var_r30 = *arg0->mdlId;
    if (var_f31 <= lbl_1_rodata_C8) {
        Hu3DModelPosGet(var_r30, &sp28);
        Hu3DModelRotGet(var_r30, &sp34);
        var_f22 = sp58.point[1].x;
        var_f29 = sp58.point[0].x;
        if (var_f31 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if (var_f31 >= lbl_1_rodata_C8) {
            var_f30 = var_f22;
        } else {
            var_f30 = var_f29 + ((var_f31 / lbl_1_rodata_C8) * (var_f22 - var_f29));
        }
        sp40.x = var_f30;
        var_f21 = sp58.point[1].y;
        var_f27 = sp58.point[0].y;
        if (var_f31 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if (var_f31 >= lbl_1_rodata_C8) {
            var_f28 = var_f21;
        } else {
            var_f28 = var_f27 + ((var_f31 / lbl_1_rodata_C8) * (var_f21 - var_f27));
        }
        sp40.y = var_f28;
        var_f20 = sp58.point[1].z;
        var_f25 = sp58.point[0].z;
        if (var_f31 <= lbl_1_rodata_74) {
            var_f26 = var_f25;
        } else if (var_f31 >= lbl_1_rodata_C8) {
            var_f26 = var_f20;
        } else {
            var_f26 = var_f25 + ((var_f31 / lbl_1_rodata_C8) * (var_f20 - var_f25));
        }
        sp40.z = var_f26;
        sp28.x -= sp40.x;
        sp28.z -= sp40.z;
        sp4C.y = (f32) -(lbl_1_rodata_88 * (atan2((f64) sp28.x, -(f64) sp28.z) / lbl_1_rodata_78));
        if ((sp34.y - sp4C.y) > lbl_1_rodata_90) {
            sp34.y -= lbl_1_rodata_94;
        } else if ((sp34.y - sp4C.y) < lbl_1_rodata_98) {
            sp34.y += lbl_1_rodata_94;
        }
        sp4C.x = sp34.x;
        var_f24 = sp4C.y;
        var_f18 = sp34.y;
        if (var_f18 == var_f24) {
            var_f19 = var_f24;
        } else {
            var_f19 = (var_f24 + (var_f18 * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
        }
        sp4C.y = var_f19;
        sp4C.z = sp34.z;
        Hu3DModelPosSetV(var_r30, &sp40);
        Hu3DModelRotSetV(var_r30, &sp4C);
    } else {
        Point3d sp1C;
        Point3d sp10;
        f32 spC;
        f32 sp8;
        f32 var_f23;

        Hu3DModelRotGet(var_r30, &sp10);
        sp1C.y = lbl_1_rodata_168;
        if ((sp10.y - sp1C.y) > lbl_1_rodata_90) {
            sp10.y -= lbl_1_rodata_94;
        } else if ((sp10.y - sp1C.y) < lbl_1_rodata_98) {
            sp10.y += lbl_1_rodata_94;
        }
        sp1C.x = sp10.x;
        var_f23 = sp1C.y;
        sp8 = sp10.y;
        if (sp8 == var_f23) {
            spC = var_f23;
        } else {
            spC = (var_f23 + (sp8 * lbl_1_rodata_9C)) / lbl_1_rodata_A0;
        }
        sp1C.y = spC;
        sp1C.z = sp10.z;
        Hu3DModelRotSetV(var_r30, &sp1C);
    }
    Hu3DMotionSpeedSet(*arg0->mdlId, lbl_1_rodata_16C);
    if (++arg0->work[0] > 60U) {
        Hu3DMotionSpeedSet(*arg0->mdlId, lbl_1_rodata_6C);
        Hu3DMotionShiftSet(*arg0->mdlId, *arg0->mtnId, lbl_1_rodata_74, lbl_1_rodata_BC, 1073741825U);
        arg0->objFunc = NULL;
    }
}

#pragma section code_type ".text.opacity_66f4"
#pragma section const_type ".rodata.opacity_bias"
void fn_1_66F4(OMOBJ *arg0)
{
    s16 var_r30;
    MDMinidllPlayerConfig *var_r29;
    s16 var_r28;
    f32 scale;

    var_r28 = 4;
    if (lbl_1_bss_804[1] == 5) {
        var_r28 = 1;
    } else {
        var_r28 = 4;
    }
    var_r30 = 0;
    var_r29 = &lbl_1_bss_7CC[var_r30];
    while (var_r30 < var_r28) {
        scale = fn_1_156C(0.0f, 1.0f, (f32)arg0->work[0], (f32)arg0->work[1]);
        Hu3DModelScaleSet(arg0->mdlId[var_r29->charNo], scale, scale, scale);
        var_r30 += 1;
        var_r29 += 1;
    }
    if ((arg0->work[0] = (u32)((f32)arg0->work[0] + lbl_1_rodata_6C)) >
        (u32)arg0->work[1]) {
        arg0->objFunc = NULL;
    }
}

#pragma section code_type ".text.opacity_7414"
#pragma section const_type ".rodata.opacity_inverse"
void fn_1_7414(OMOBJ *obj)
{
    f32 opacity;

    switch ((s32)obj->work[2]) {
    case 1:
        opacity = fn_1_156C(0.0f, 1.0f, (f32)obj->work[3], 10.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_752[1], opacity);
        if (++obj->work[3] > 10U) {
            obj->work[2] = 0;
        }
        break;
    case 2:
        opacity = fn_1_156C(1.0f, 0.0f, (f32)obj->work[3], 10.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_752[1], opacity);
        if (++obj->work[3] > 10U) {
            obj->work[2] = 0;
        }
        break;
    }
}

#pragma section code_type ".text.opacity_7754"
#pragma section const_type ".rodata.opacity_inverse"
void fn_1_7754(OMOBJ *obj)
{
    s16 i;
    f32 opacity;

    for (i = 0; i < 2; i++) {
        Point3d rotation;
        Hu3DModelRotGet(obj->mdlId[i], &rotation);
        rotation.z -= lbl_1_rodata_194;
        if (rotation.z < lbl_1_rodata_74) {
            rotation.z += lbl_1_rodata_94;
        }
        Hu3DModelRotSetV(obj->mdlId[i], &rotation);
    }
    switch ((s32)obj->work[0]) {
    case 1:
        opacity = fn_1_156C(1.0f, 0.0f, (f32)obj->work[1], 30.0f);
        Hu3DModelTPLvlSet(*obj->mdlId, opacity);
        if (++obj->work[1] > 30U) {
            obj->work[0] = 0;
        }
        break;
    case 2:
        opacity = fn_1_156C(0.0f, 1.0f, (f32)obj->work[1], 30.0f);
        Hu3DModelTPLvlSet(*obj->mdlId, opacity);
        if (++obj->work[1] > 30U) {
            obj->work[0] = 0;
        }
        break;
    }
    fn_1_7414(obj);
}

#pragma section code_type ".text.model_transition_root"
#pragma section const_type ".rodata.model_signed_bias"
#define _MATH_H
#include "game/main.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/player_config.h"
#include "REL/mdminidll/row_state.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Target-backed consumed views; the original declarations/purpose are unknown. */
extern s16 lbl_1_bss_34;
extern OMOBJ *lbl_1_bss_24;
extern const f32 lbl_1_rodata_234;

void fn_1_109D4(OMOBJ *arg0)
{
    MDMinidllModelAnimationRecord *var_r31;
    s16 var_r30;
    s16 var_r29;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    if ((u32) arg0->work[0] == 0) {
        var_r30 = 0;
        while (var_r30 < 4) {
            lbl_1_bss_80[var_r30].x = lbl_1_rodata_74;
            var_r30 += 1;
        }
    }
    var_r30 = 0;
    while (var_r30 < 11) {
        var_r29 = (s16) arg0->work[0];
        var_r31 = &lbl_1_bss_230[var_r30];
        if (var_r29 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[2]);
            Hu3DModelRotGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[14]);
            Hu3DModelScaleGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[26]);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f26 = ((HuVecF *) &var_r31->unknown_0A[14])->y - lbl_1_rodata_234;
        var_f29 = ((HuVecF *) &var_r31->unknown_0A[14])->y;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f30 = var_f26;
        } else {
            var_f30 = var_f29 + (((f32) var_r29 / lbl_1_rodata_A0) * (var_f26 - var_f29));
        }
        var_f31 = var_f30;
        Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f27 = ((HuVecF *) &var_r31->unknown_0A[26])->x;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = var_f27 + (((f32) var_r29 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f27));
        }
        var_f31 = var_f28;
        Hu3DModelScaleSet(var_r31->model, var_f31, var_f31, lbl_1_rodata_6C);
        if (var_r29 <= 10) {
            Hu3DModelAttrReset(var_r31->model, 1U);
        }
        var_r30 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

/* Keep the recovered animation helper visible to its reset caller. */
extern ANIMDATA *lbl_1_bss_6C8[];

#pragma section code_type ".text.model_animation_set"
void fn_1_9D58(s16 arg0, s16 arg1, s16 arg2, s16 arg3)
{
    MDMinidllModelAnimationRecord *entry;

    entry = &lbl_1_bss_230[arg0];
    Hu3DAnimAnimSet(entry->animation[arg1], lbl_1_bss_6C8[arg2]);
    if (arg3 != -1) {
        Hu3DAnimBankSet(entry->animation[arg1], (u16)arg3);
    }
}

void fn_1_9E00(s16 arg0, s16 arg1)
{
    s16 i;
    for (i = 0; i < 4; i++) {
        fn_1_9D58(arg0, i, arg1, -1);
    }
}

#pragma section code_type ".text.player_animation_reset"
void fn_1_E0B0(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *config;

    config = lbl_1_bss_7CC;
    config->unknown_00 = 0;
    lbl_1_bss_80[0].x = lbl_1_rodata_74;
    fn_1_9D58((s16)config->charNo, 0, (s16)config->charNo, 0);
    fn_1_9D58((s16)config->charNo, 2, (s16)config->charNo, 0);
    arg0->objFunc = NULL;
    *lbl_1_bss_24->mtnId = 1;
}

/* These recovered functions share the compiler's signed-conversion constant.
 * Named code sections preserve their retail order without duplicating storage;
 * this grouping does not claim the original translation-unit boundaries. */
#pragma section code_type ".text.sprite_projection"
extern const f32 lbl_1_rodata_70;
extern const f32 lbl_1_rodata_1A4;
extern const f32 lbl_1_rodata_1A8;
extern const f32 lbl_1_rodata_1AC;
extern HUSPR_GROUPID lbl_1_bss_752[];

void fn_1_81FC(OMOBJ *arg0)
{
    Point3d sp14;
    Point3d sp8;
    MDMinidllPlayerConfig *var_r30;
    s16 var_r31;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;

    var_r31 = 0;
    var_r30 = lbl_1_bss_7CC;
    while (var_r31 < 4) {
        Hu3DModelPosGet(lbl_1_bss_230[lbl_1_bss_7CC[var_r31].charNo].model, &sp14);
        Hu3D3Dto2D(&sp14, 1, &sp8);
        HuSprGrpPosSet(lbl_1_bss_752[var_r31 + 4], sp8.x - lbl_1_rodata_1A4,
                       (f32) (s16) (sp8.y - lbl_1_rodata_1A8));
        HuSprAttrReset(lbl_1_bss_752[var_r31 + 4], 0, 8);
        var_f31 = lbl_1_bss_80[var_r31].x;
        var_f29 = lbl_1_bss_80[var_r31].y;
        if (var_f29 == var_f31) {
            var_f30 = var_f31;
        } else {
            var_f30 = (var_f31 + (var_f29 * lbl_1_rodata_70)) / lbl_1_rodata_1AC;
        }
        lbl_1_bss_80[var_r31].y = var_f30;
        HuSprGrpScaleSet(lbl_1_bss_752[var_r31 + 4], lbl_1_bss_80[var_r31].y,
                         lbl_1_bss_80[var_r31].y);
        var_r31 += 1;
        var_r30 += 1;
    }
}


#pragma section code_type ".text.sprite_setup"
#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"

extern s16 lbl_1_bss_752[9];
extern ANIMDATA *lbl_1_bss_764[26];
extern char lbl_1_data_860[7];
extern char lbl_1_data_867[7];
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_198;
extern const f32 lbl_1_rodata_19C;
extern const f32 lbl_1_rodata_18C;
extern const f32 lbl_1_rodata_128;
void fn_1_7754(OMOBJ *arg0);

void fn_1_7C4C(OMOBJ *arg0)
{
    s16 var_r30;

    omSetStatBit(arg0, 256U);
    var_r30 = 0;
    while (var_r30 < 2) {
        if (var_r30 == 0) {
            arg0->mdlId[var_r30] = Hu3DModelCreate(HuDataSelHeapReadNum(9830403, 268435456, HEAP_MODEL));
        } else {
            arg0->mdlId[var_r30] = Hu3DModelLink(*arg0->mdlId);
        }
        Hu3DModelPosSet(arg0->mdlId[var_r30], lbl_1_rodata_74, lbl_1_rodata_198,
                        lbl_1_rodata_19C - (f32) var_r30);
        Hu3DModelRotSet(arg0->mdlId[var_r30], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
        Hu3DModelAttrSet(arg0->mdlId[var_r30], 1073741826U);
        Hu3DModelLayerSet(arg0->mdlId[var_r30], 0);
        arg0->mtnId[var_r30 * 2] = Hu3DAnimCreate(lbl_1_bss_764[1], arg0->mdlId[var_r30], lbl_1_data_860);
        arg0->mtnId[(var_r30 * 2) + 1] = Hu3DAnimCreate(lbl_1_bss_764[2], arg0->mdlId[var_r30], lbl_1_data_867);
        var_r30 += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], lbl_1_rodata_74);
    arg0->objFunc = fn_1_7754;
}

#pragma section code_type ".text.model_yaw"
#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/readonly_scalars.h"

/* Existing target-owned scalar at .rodata+0x98. */
extern const f32 lbl_1_rodata_98;

void fn_1_B078(s16 arg0, f32 farg0, s16 arg1, s16 arg2)
{
    MDMinidllModelAnimationRecord *var_r31;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;

    var_r31 = &lbl_1_bss_230[arg0];
    if (arg1 == 0) {
        Hu3DModelRotGet(var_r31->model, (HuVecF *) ((u8 *) var_r31 + 24));
        if ((((HuVecF *) ((u8 *) var_r31 + 24))->y - farg0) > lbl_1_rodata_90) {
            ((HuVecF *) ((u8 *) var_r31 + 24))->y -= lbl_1_rodata_94;
        } else if ((((HuVecF *) ((u8 *) var_r31 + 24))->y - farg0) < lbl_1_rodata_98) {
            ((HuVecF *) ((u8 *) var_r31 + 24))->y += lbl_1_rodata_94;
        }
    }
    var_f30 = ((HuVecF *) ((u8 *) var_r31 + 24))->y;
    if ((f32) arg1 <= lbl_1_rodata_74) {
        var_f31 = var_f30;
    } else if ((f32) arg1 >= (f32) arg2) {
        var_f31 = lbl_1_rodata_74;
    } else {
        var_f31 = var_f30 + (((f32) arg1 / (f32) arg2) * (lbl_1_rodata_74 - var_f30));
    }
    var_f29 = var_f31;
    Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, var_f29, lbl_1_rodata_74);
}

#pragma section code_type ".text.model_rotation_transition"
/* This unit uses no math inlines; keep the compiler from emitting their weak pools. */
#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/readonly_scalars.h"

extern OMOBJ *lbl_1_bss_24;
extern HUSPR_GROUPID lbl_1_bss_752[9];
extern s16 lbl_1_data_86E[6];
/* Target-backed consumed readonly f32 view; bytes are 0xC3340000 (-180.0f). */
extern const f32 lbl_1_rodata_98;

/* Hu3DModelRotGet consumes a HuVecF at record offset 0x18; the rest of that
 * record interval remains represented by the existing unknown-byte view. */
typedef struct MDMinidllRotationView {
    u8 unknown_00[24];
    HuVecF rotation;
} MDMinidllRotationView;

void fn_1_11B0C(OMOBJ *arg0)
{
    MDMinidllModelAnimationRecord *var_r31;
    s16 var_r29;
    s16 var_r28;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;

    if ((u32) arg0->work[0] == 0) {
        HuSprAttrSet(lbl_1_bss_752[2], 0, 4);
        lbl_1_data_86E[0] = 0;
        HuSprAttrSet(lbl_1_bss_752[2], 1, 4);
        lbl_1_data_86E[1] = 0;
    }
    var_r29 = 0;
    while (var_r29 < 6) {
        lbl_1_bss_230[var_r29 + 11].unknown_30 = 0;
        var_r28 = (s16) arg0->work[0];
        var_r31 = &lbl_1_bss_230[(s16) (var_r29 + 11)];
        if (var_r28 == 0) {
            Hu3DModelRotGet(var_r31->model,
                            &((MDMinidllRotationView *) var_r31)->rotation);
            if ((((MDMinidllRotationView *) var_r31)->rotation.y - lbl_1_rodata_74) > lbl_1_rodata_90) {
                ((MDMinidllRotationView *) var_r31)->rotation.y -= lbl_1_rodata_94;
            } else if ((((MDMinidllRotationView *) var_r31)->rotation.y - lbl_1_rodata_74) < lbl_1_rodata_98) {
                ((MDMinidllRotationView *) var_r31)->rotation.y += lbl_1_rodata_94;
            }
        }
        var_f30 = ((MDMinidllRotationView *) var_r31)->rotation.y;
        if ((f32) var_r28 <= lbl_1_rodata_74) {
            var_f31 = var_f30;
        } else if ((f32) var_r28 >= lbl_1_rodata_A0) {
            var_f31 = lbl_1_rodata_74;
        } else {
            var_f31 = var_f30 + (((f32) var_r28 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f30));
        }
        var_f29 = var_f31;
        Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, var_f29, lbl_1_rodata_74);
        var_r29 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_position_scale"

/* Consumed view only: the target uses the model at 0, then Point3d values at
 * 0xC, 0x18, and 0x24. Other bytes remain opaque; this is not an original
 * record declaration. The canonical animation-record header owns the array. */
typedef struct A700ModelStateView {
    s16 unk0;
    u8 unknown_02[10];
    Point3d unkC;
    Point3d unk18;
    Point3d unk24;
    u8 unknown_30[8];
} A700ModelStateView;

void fn_1_A700(s16 arg0, s16 arg1, s16 arg2, Point3d *arg3, f32 farg0)
{
    Point3d position;
    A700ModelStateView *model;
    f32 scale;

    model = &((A700ModelStateView *)lbl_1_bss_230)[arg0];
    if (arg1 == 0) {
        Hu3DModelPosGet(model->unk0, &model->unkC);
        Hu3DModelRotGet(model->unk0, &model->unk18);
        Hu3DModelScaleGet(model->unk0, &model->unk24);
    }
    position.x = fn_1_156C(model->unkC.x, arg3->x, (f32) arg1, (f32) arg2);
    position.y = fn_1_156C(model->unkC.y, arg3->y, (f32) arg1, (f32) arg2);
    position.z = fn_1_156C(model->unkC.z, arg3->z, (f32) arg1, (f32) arg2);
    Hu3DModelPosSet(model->unk0, position.x, position.y, position.z);
    scale = fn_1_156C(model->unk24.x, farg0, (f32) arg1, (f32) arg2);
    Hu3DModelScaleSet(model->unk0, scale, scale, 1.0f);
}

#pragma section code_type ".text.model_exit_transition"

/* Target-backed globals with existing module consumers. */
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_24;
extern s16 lbl_1_bss_34;
extern const f32 lbl_1_rodata_234;

void fn_1_12E88(OMOBJ *arg0)
{
    MDMinidllModelAnimationRecord *var_r31;
    s16 var_r30;
    s16 var_r28;
    OMOBJ *var_r27;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    if ((u32) arg0->work[0] == 0) {
        var_r27 = lbl_1_bss_C;
        var_r27->work[2] = 2;
        var_r27->work[3] = 0;
    }
    var_r28 = 0;
    while (var_r28 < 6) {
        var_r30 = (s16) arg0->work[0];
        var_r31 = &lbl_1_bss_230[(s16) (var_r28 + 11)];
        if (var_r30 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[2]);
            Hu3DModelRotGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[14]);
            Hu3DModelScaleGet(var_r31->model, (HuVecF *) &var_r31->unknown_0A[26]);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f26 = ((HuVecF *) &var_r31->unknown_0A[14])->y - lbl_1_rodata_234;
        var_f29 = ((HuVecF *) &var_r31->unknown_0A[14])->y;
        if ((f32) var_r30 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if ((f32) var_r30 >= lbl_1_rodata_A0) {
            var_f30 = var_f26;
        } else {
            var_f30 = var_f29 + (((f32) var_r30 / lbl_1_rodata_A0) * (var_f26 - var_f29));
        }
        var_f31 = var_f30;
        Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f27 = ((HuVecF *) &var_r31->unknown_0A[26])->x;
        if ((f32) var_r30 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if ((f32) var_r30 >= lbl_1_rodata_A0) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = var_f27 + (((f32) var_r30 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f27));
        }
        var_f31 = var_f28;
        Hu3DModelScaleSet(var_r31->model, var_f31, var_f31, lbl_1_rodata_6C);
        if (var_r30 <= 10) {
            Hu3DModelAttrReset(var_r31->model, 1U);
        }
        var_r28 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        Hu3DLayerHookReset(2);
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}


/* Player transitions use the same observed model-record view and signed
 * conversion pool as the selected transition functions above. */
void fn_1_68F0(void);

#pragma section code_type ".text.model_player_transition"
void fn_1_105DC(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *var_r27;
    A700ModelStateView *var_r31;
    s16 var_r30;
    s16 var_r29;
    s16 var_r26;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    if ((u32) arg0->work[0] == 0) {
        var_r30 = 0;
        while (var_r30 < 4) {
            lbl_1_bss_80[var_r30].x = lbl_1_rodata_74;
            var_r30 += 1;
        }
    }
    var_r30 = 0;
    var_r27 = lbl_1_bss_7CC;
    while (var_r30 < 4) {
        var_r29 = (s16) arg0->work[0];
        var_r26 = (s16) var_r27->charNo;
        var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[var_r26];
        if (var_r29 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
            Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
            Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f26 = var_r31->unk18.y - lbl_1_rodata_234;
        var_f29 = var_r31->unk18.y;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f30 = var_f26;
        } else {
            var_f30 = var_f29 + (((f32) var_r29 / lbl_1_rodata_A0) * (var_f26 - var_f29));
        }
        var_f31 = var_f30;
        Hu3DModelRotSet(var_r31->unk0, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f27 = var_r31->unk24.x;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = var_f27 + (((f32) var_r29 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f27));
        }
        var_f31 = var_f28;
        Hu3DModelScaleSet(var_r31->unk0, var_f31, var_f31, lbl_1_rodata_6C);
        if (var_r29 <= 10) {
            Hu3DModelAttrReset(var_r31->unk0, 1U);
        }
        var_r30 += 1;
        var_r27 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_player_exit"
void fn_1_1406C(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *var_r27;
    A700ModelStateView *var_r31;
    s16 var_r30;
    s16 var_r29;
    OMOBJ *var_r26;
    s16 var_r25;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    if ((u32) arg0->work[0] == 0) {
        var_r26 = lbl_1_bss_C;
        var_r26->work[2] = 2;
        var_r26->work[3] = 0;
        var_r30 = 0;
        while (var_r30 < 4) {
            lbl_1_bss_80[var_r30].x = lbl_1_rodata_74;
            var_r30 += 1;
        }
    }
    var_r30 = 0;
    var_r27 = lbl_1_bss_7CC;
    while (var_r30 < 4) {
        var_r29 = (s16) arg0->work[0];
        var_r25 = (s16) var_r27->charNo;
        var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[var_r25];
        if (var_r29 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
            Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
            Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f26 = var_r31->unk18.y - lbl_1_rodata_234;
        var_f29 = var_r31->unk18.y;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f30 = var_f26;
        } else {
            var_f30 = var_f29 + (((f32) var_r29 / lbl_1_rodata_A0) * (var_f26 - var_f29));
        }
        var_f31 = var_f30;
        Hu3DModelRotSet(var_r31->unk0, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f27 = var_r31->unk24.x;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = var_f27 + (((f32) var_r29 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f27));
        }
        var_f31 = var_f28;
        Hu3DModelScaleSet(var_r31->unk0, var_f31, var_f31, lbl_1_rodata_6C);
        if (var_r29 <= 10) {
            Hu3DModelAttrReset(var_r31->unk0, 1U);
        }
        var_r30 += 1;
        var_r27 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        fn_1_68F0();
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_player_exit_reset"
void fn_1_14484(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *var_r27;
    A700ModelStateView *var_r31;
    s16 var_r30;
    s16 var_r29;
    OMOBJ *var_r26;
    OMOBJ *var_r25;
    s16 var_r24;
    f32 var_f31;
    f32 var_f30;
    f32 var_f29;
    f32 var_f28;
    f32 var_f27;
    f32 var_f26;

    if ((u32) arg0->work[0] == 0) {
        var_r26 = lbl_1_bss_C;
        var_r26->work[0] = 2;
        var_r26->work[1] = 0;
        var_r25 = lbl_1_bss_C;
        var_r25->work[2] = 2;
        var_r25->work[3] = 0;
        var_r30 = 0;
        while (var_r30 < 4) {
            lbl_1_bss_80[var_r30].x = lbl_1_rodata_74;
            var_r30 += 1;
        }
    }
    var_r30 = 0;
    var_r27 = lbl_1_bss_7CC;
    while (var_r30 < 4) {
        var_r29 = (s16) arg0->work[0];
        var_r24 = (s16) var_r27->charNo;
        var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[var_r24];
        if (var_r29 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
            Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
            Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f26 = var_r31->unk18.y - lbl_1_rodata_234;
        var_f29 = var_r31->unk18.y;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f30 = var_f29;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f30 = var_f26;
        } else {
            var_f30 = var_f29 + (((f32) var_r29 / lbl_1_rodata_A0) * (var_f26 - var_f29));
        }
        var_f31 = var_f30;
        Hu3DModelRotSet(var_r31->unk0, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f27 = var_r31->unk24.x;
        if ((f32) var_r29 <= lbl_1_rodata_74) {
            var_f28 = var_f27;
        } else if ((f32) var_r29 >= lbl_1_rodata_A0) {
            var_f28 = lbl_1_rodata_74;
        } else {
            var_f28 = var_f27 + (((f32) var_r29 / lbl_1_rodata_A0) * (lbl_1_rodata_74 - var_f27));
        }
        var_f31 = var_f28;
        Hu3DModelScaleSet(var_r31->unk0, var_f31, var_f31, lbl_1_rodata_6C);
        if (var_r29 <= 10) {
            Hu3DModelAttrReset(var_r31->unk0, 1U);
        }
        var_r30 += 1;
        var_r27 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_transition_a330"
void fn_1_A330(s16 arg0, s16 arg1, s16 arg2)
{
    A700ModelStateView *var_r31;
    f32 result;

    var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[arg0];
    if (arg1 == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(53);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
        Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
        Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
    } else {
        lbl_1_bss_34 = 0;
    }
    result = fn_1_156C(var_r31->unk18.y, var_r31->unk18.y - lbl_1_rodata_234, (f32) arg1, (f32) arg2);
    Hu3DModelRotSet(var_r31->unk0, 0.0f, result, 0.0f);
    result = fn_1_156C(var_r31->unk24.x, 0.0f, (f32) arg1, (f32) arg2);
    Hu3DModelScaleSet(var_r31->unk0, result, result, 1.0f);
    if (arg1 <= arg2) {
        Hu3DModelAttrReset(var_r31->unk0, 1U);
    }
}

#pragma section code_type ".text.sprite_group_attributes"
#pragma section const_type ".rodata.opacity_inverse"
#define _MATH_H
#include "game/sprite.h"

void fn_1_4438(s16 groupId, s32 attr)
{
    s16 member;
    HUSPR_GROUP *group;
    group = &HuSprGrpData[groupId];
    member = 0;
    while (member < group->sprNum) {
        HuSprAttrSet(groupId, member, (u16)attr);
        member++;
    }
}

void fn_1_44B8(s16 groupId, s32 attr)
{
    s16 member;
    HUSPR_GROUP *group;
    group = &HuSprGrpData[groupId];
    member = 0;
    while (member < group->sprNum) {
        HuSprAttrReset(groupId, member, (u16)attr);
        member++;
    }
}

#pragma section code_type ".text.sprite_opacity_hide_6dc4"
void fn_1_6DC4(OMOBJ *arg0)
{
    f32 opacity;

    opacity = fn_1_156C(1.0f, 0.0f, (f32) arg0->work[0], 10.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_752[0], opacity);
    if (++arg0->work[0] > 10U) {
        fn_1_4438(lbl_1_bss_752[0], 4);
        arg0->objFunc = NULL;
    }
}


#pragma section code_type ".text.window_visibility"
#define _MATH_H
#include "game/window.h"

extern s16 lbl_1_bss_858[4];

void fn_1_3404(s16 windowNo)
{
    if (windowNo == 0) {
        HuWinDispOn(lbl_1_bss_858[windowNo]);
        return;
    }
    HuWinExOpen(lbl_1_bss_858[windowNo]);
}

void fn_1_3474(s16 windowNo)
{
    if (windowNo == 0) {
        HuWinDispOff(lbl_1_bss_858[windowNo]);
        return;
    }
    HuWinExClose(lbl_1_bss_858[windowNo]);
}



#pragma section code_type ".text.window_messages"
#define _MATH_H
#include "game/window.h"
extern s16 lbl_1_bss_858[4];
extern u32 lbl_1_data_690;

s16 fn_1_3520(s16 index, s16 mode)
{
    s16 choice;
    choice = 0;
    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_858[index], 16U);
    } else {
        HuWinAttrReset(lbl_1_bss_858[index], 16U);
    }
    choice = HuWinChoiceGet(lbl_1_bss_858[index], -1);
    if ((mode == 2) && (choice == -1)) {
        choice = 1;
    }
    return choice;
}

void fn_1_35F4(s16 index, u32 message, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_858[index], 2048U);
    HuWinMesSet(lbl_1_bss_858[index], message);
    HuWinMesSpeedSet(lbl_1_bss_858[index], speed);
    if ((s32)lbl_1_data_690 != (s32)message) {
        lbl_1_data_690 = -1U;
    }
}

void fn_1_36B0(s16 index, u32 message, s16 insert)
{
    HuWinHomeClear(lbl_1_bss_858[index]);
    HuWinInsertMesSet(lbl_1_bss_858[index], message, insert);
}


#pragma section code_type ".text.window_select"
#define _MATH_H
#include "game/main.h"
#include "game/window.h"

extern s16 lbl_1_data_82C[];
extern s32 lbl_1_data_830[];
extern s16 lbl_1_bss_858[4];

void fn_1_39A4(s16 arg0)
{
    if ((lbl_1_data_82C[0] != -1) && (lbl_1_data_82C[0] != arg0)) {
        fn_1_3474(lbl_1_data_82C[0]);
    }
    if ((lbl_1_data_82C[0] == -1) || (lbl_1_data_82C[0] != arg0)) {
        lbl_1_data_82C[0] = arg0;
        lbl_1_data_830[0] = -1;
        lbl_1_data_830[1] = -1;
        fn_1_3404(lbl_1_data_82C[0]);
    }
}


#pragma section code_type ".text.active_window_messages"
#define _MATH_H
#include "game/main.h"
#include "game/window.h"
#include "REL/mdminidll/window_state.h"

extern u32 lbl_1_data_690;
extern s16 lbl_1_bss_858[4];

void fn_1_3D24(s16 arg0, s32 arg1, s16 arg2)
{
    fn_1_39A4(arg0);
    if (lbl_1_data_830[0] != arg1) {
        lbl_1_data_830[0] = arg1;
        lbl_1_data_830[1] = -1;
        fn_1_35F4(lbl_1_data_82C[0], (u32)lbl_1_data_830[0], arg2);
    }
}

void fn_1_3F5C(s16 arg0, s32 arg1, s16 arg2)
{
    fn_1_39A4(arg0);
    if (lbl_1_data_830[1] != arg1) {
        lbl_1_data_830[0] = -1;
        lbl_1_data_830[1] = arg1;
        fn_1_36B0(lbl_1_data_82C[0], (u32)lbl_1_data_830[1], arg2);
    }
}


#pragma section code_type ".text.active_window_choice"
#define _MATH_H
#include "game/window.h"
#include "REL/mdminidll/window_state.h"
extern s16 lbl_1_bss_858[4];

s16 fn_1_3C2C(s16 mode)
{
    if (lbl_1_data_82C[0] != -1) {
        return fn_1_3520(lbl_1_data_82C[0], mode);
    }
    return 0;
}


#pragma section code_type ".text.window_choice_181cc"
s16 fn_1_181CC(void)
{
    fn_1_4E40();
    fn_1_3D24(2, 1114114, 1);
    return fn_1_3C2C(2);
}

#pragma section code_type ".text.direction_choice_97d8"
#include "game/main.h"
#include "game/pad.h"
#include "REL/mdminidll/readonly_scalars.h"
/* Restore the real math helper after the earlier provider declarations.
 * Its weak local statics reproduce the retail readonly prefix at 0..16. */
#undef _MATH_H
#pragma section const_type ".rodata"
#include "math.h"
#pragma section const_type ".rodata.choice_math"

/* Consumed 18-byte record stride and halfword at +16. The preceding
 * interval's fields are unknown here; this view does not allocate storage. */
struct _struct_lbl_1_data_89A_0x12 {
    unsigned char unknown_00[16];
    s16 unk10;
};
extern const Point3d lbl_1_rodata_214;
extern const f32 lbl_1_rodata_B0;
extern s16 fn_1_9730(s16, struct _struct_lbl_1_data_89A_0x12 (*)[], s16, s16);

s16 fn_1_97D8(s16 arg0, s16 *arg1, struct _struct_lbl_1_data_89A_0x12 (*arg2)[], s16 arg3)
{
    Point3d sp18;
    s16 var_r31;
    s16 var_r27;

    var_r27 = 0;
    var_r31 = 0;
    sp18 = lbl_1_rodata_214;
    if ((s32) (HuPadDStkRep[arg0] & 8) != 0) {
        sp18.y = lbl_1_rodata_6C;
    } else if ((s32) (HuPadDStkRep[arg0] & 4) != 0) {
        sp18.y = lbl_1_rodata_F0;
    }
    if ((s32) (HuPadDStkRep[arg0] & 1) != 0) {
        sp18.x = lbl_1_rodata_F0;
    } else if ((s32) (HuPadDStkRep[arg0] & 2) != 0) {
        sp18.x = lbl_1_rodata_6C;
    }
    if ((sp18.x < lbl_1_rodata_74 ? -sp18.x : sp18.x) < lbl_1_rodata_B0) {
        if ((sp18.y < lbl_1_rodata_74 ? -sp18.y : sp18.y) < lbl_1_rodata_B0) {
            return 0;
        }
    }
    sp18.z = sqrtf((sp18.x * sp18.x) + (sp18.y * sp18.y));
    sp18.x /= sp18.z;
    sp18.y /= sp18.z;
    if ((sp18.y < lbl_1_rodata_74 ? -sp18.y : sp18.y) < lbl_1_rodata_B0) {
        if (sp18.x > lbl_1_rodata_74) {
            var_r31 = 0;
        } else {
            var_r31 = 1;
        }
    } else {
        if ((sp18.x < lbl_1_rodata_74 ? -sp18.x : sp18.x) < lbl_1_rodata_B0) {
            if (sp18.y > lbl_1_rodata_74) {
                var_r31 = 2;
            } else {
                var_r31 = 3;
            }
        } else {
            if ((sp18.x < lbl_1_rodata_74 ? -sp18.x : sp18.x) > (sp18.y < lbl_1_rodata_74 ? -sp18.y : sp18.y)) {
                if (sp18.x > lbl_1_rodata_74) {
                    if (sp18.y > lbl_1_rodata_74) {
                        var_r31 = 4;
                    } else {
                        var_r31 = 5;
                    }
                } else if (sp18.y > lbl_1_rodata_74) {
                    var_r31 = 6;
                } else {
                    var_r31 = 7;
                }
            } else if (sp18.y > lbl_1_rodata_74) {
                if (sp18.x > lbl_1_rodata_74) {
                    var_r31 = 8;
                } else {
                    var_r31 = 9;
                }
            } else if (sp18.x > lbl_1_rodata_74) {
                var_r31 = 10;
            } else {
                var_r31 = 11;
            }
        }
    }
    if ((var_r27 = fn_1_9730(*arg1, arg2, var_r31, arg3)) != -1) {
        (*arg2)[*arg1].unk10 = -1;
        *arg1 = var_r27;
        (*arg2)[*arg1].unk10 = 1;
        HuAudFXPlay(0);
        return 1;
    }
    return 0;
}


extern s16 lbl_1_data_6B6[];
extern Point3d lbl_1_data_4C8[];

/* Shared native literal pool: retail .rodata+0x178 is 1.25f. */
#pragma section code_type ".text.model_row_setup"
#pragma section const_type ".rodata.model_scale_178"
void fn_1_110EC(s16 arg0)
{
    MDMinidllModelAnimationRecord *entry;

    entry = &lbl_1_bss_230[arg0 + 11];
    Hu3DModelPosSetV(entry->model, &lbl_1_data_4C8[arg0 + 1]);
    Hu3DModelRotSet(entry->model, lbl_1_rodata_74, lbl_1_rodata_74,
                    lbl_1_rodata_74);
    Hu3DModelScaleSet(entry->model, 1.25f, 1.25f,
                      lbl_1_rodata_6C);
    entry->unknown_30 = 0;
    Hu3DModelLayerSet(entry->model, 1);
}

#pragma section code_type ".text.sprite_group_select"
void fn_1_7280(s16 arg0)
{
    OMOBJ *object;
    object = lbl_1_bss_C;
    HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128);
    HuSprGrpTPLvlSet(lbl_1_bss_752[1], lbl_1_rodata_74);
    fn_1_4438(lbl_1_bss_752[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_752[1], arg0, HUSPR_ATTR_DISPOFF);
    object->work[2] = 1;
    object->work[3] = 0;
}

#pragma section code_type ".text.model_selection_11e14"
#pragma section const_type ".rodata.model_selection_11e14"
void fn_1_11E14(OMOBJ *arg0)
{
    s16 var_r31;

    lbl_1_bss_804[1] = 0;
    if ((u32) arg0->work[0] == 0) {
        var_r31 = 0;
        while (var_r31 < 6) {
            fn_1_9E00((s16)(var_r31 + 11), (s16)(var_r31 + 12));
            if (lbl_1_data_6B6[var_r31] == 0) {
                fn_1_9D58((s16)(var_r31 + 11), 2, 20, -1);
                fn_1_9D58((s16)(var_r31 + 11), 3, 20, -1);
            }
            var_r31 += 1;
        }
    }
    var_r31 = 0;
    while (var_r31 < 6) {
        s16 var_r29;
        MDMinidllModelAnimationRecord *var_r30;
        f32 var_f31;

        var_r29 = (s16) arg0->work[0];
        var_r30 = &lbl_1_bss_230[(s16) (var_r31 + 11)];
        if (var_r29 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(52);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(var_r30->model, lbl_1_data_4C8[var_r31 + 1].x, lbl_1_data_4C8[var_r31 + 1].y, lbl_1_data_4C8[var_r31 + 1].z);
            Hu3DModelRotSet(var_r30->model, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
            Hu3DModelScaleSet(var_r30->model, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
            Hu3DModelAttrReset(var_r30->model, 1U);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f31 = fn_1_156C(-540.0f, 0.0f, (f32) var_r29, 10.0f);
        Hu3DModelRotSet(var_r30->model, lbl_1_rodata_74, var_f31, lbl_1_rodata_74);
        var_f31 = fn_1_156C(0.0f, 1.25f, (f32) var_r29, 10.0f);
        Hu3DModelScaleSet(var_r30->model, var_f31, var_f31, lbl_1_rodata_6C);
        var_r31 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        Hu3DZClearLayerSet(2);
        fn_1_7280(lbl_1_bss_804[1]);
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}


#pragma section code_type ".text.model_spin_shrink_d09c"
void fn_1_D09C(OMOBJ *arg0)
{
    A700ModelStateView *var_r31;
    s16 var_r30;
    s16 var_r28;
    f32 result;

    var_r28 = 0;
    while (var_r28 < 4) {
        var_r30 = (s16)arg0->work[0];
        var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[(s16)(var_r28 + 17)];
        if (var_r30 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
            Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
            Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
        } else {
            lbl_1_bss_34 = 0;
        }
        result = fn_1_156C(var_r31->unk18.y, var_r31->unk18.y - 540.0f, (f32)var_r30, 10.0f);
        Hu3DModelRotSet(var_r31->unk0, 0.0f, result, 0.0f);
        result = fn_1_156C(var_r31->unk24.x, 0.0f, (f32)var_r30, 10.0f);
        Hu3DModelScaleSet(var_r31->unk0, result, result, 1.0f);
        if (var_r30 <= 10) {
            Hu3DModelAttrReset(var_r31->unk0, 1U);
        }
        var_r28 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}


/* Existing vector/scalar helper composition, shared with the camera update. */
#pragma section code_type ".text.vector_approach"
#pragma section const_type ".rodata.opacity_one"
void fn_1_1340(f32 *arg0, f32 *arg1, f32 farg0)
{
    arg0[0] = fn_1_1310(arg0[0], arg1[0], farg0);
    arg0[1] = fn_1_1310(arg0[1], arg1[1], farg0);
    arg0[2] = fn_1_1310(arg0[2], arg1[2], farg0);
}

#pragma section code_type ".text.camera_25b4"
void fn_1_25B4(MDMinidllCameraState *state, f32 factor)
{
    fn_1_1340(&state->center.x, &state->centerTarget.x, factor);
    fn_1_1340(&state->rotation.x, &state->rotationTarget.x, factor);
    state->zoom = fn_1_1310(state->zoom, state->zoomTarget, factor);
}

#pragma section code_type ".text.model_animation_turn_acf0"
#pragma section const_type ".rodata.scalar_arc_1ed8"
void fn_1_ACF0(s16 arg0, s32 arg1, s32 arg2, s16 arg3, s16 arg4, s16 arg5)
{
    Point3d sp8;
    MDMinidllModelAnimationRecord *var_r31;
    f32 var_f30;

    var_r31 = &lbl_1_bss_230[(s16)arg0];
    if (((s16)arg3 == 0) || ((s16)arg3 >= (s16)arg4)) {
        Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_74);
    }
    if ((s16)arg3 >= (s16)arg4) {
        fn_1_9E00((s16)arg0, (s16)(arg2 + arg1));
        return;
    }
    Hu3DModelRotGet(var_r31->model, &sp8);
    sp8.y = fn_1_156C(0.0f, 180.0f * (f32)(s16)arg5, (f32)(s16)arg3, (f32)(s16)arg4);
    if (sp8.y < 0.0f) {
        var_f30 = -sp8.y;
    } else {
        var_f30 = sp8.y;
    }
    if (var_f30 >= lbl_1_rodata_80) {
        fn_1_9E00((s16)arg0, (s16)(arg2 + arg1));
    }
    Hu3DModelRotSetV(var_r31->model, &sp8);
}

#pragma section code_type ".text.model_player_select_c770"
#pragma section const_type ".rodata.model_selection_11e14"
void fn_1_C770(OMOBJ *arg0)
{
    s16 var_r31;
    s16 var_r21;

    var_r21 = 1;
    var_r31 = 0;
    while (var_r31 < 4) {
        s16 var_r27;
        MDMinidllModelAnimationRecord *var_r30;
        f32 var_f31;
        var_r27 = (s16) arg0->work[0];
        var_r30 = &lbl_1_bss_230[(s16) (var_r31 + 17)];
        if (var_r27 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(52);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(var_r30->model, lbl_1_data_4C8[var_r31 + 33].x, lbl_1_data_4C8[var_r31 + 33].y, lbl_1_data_4C8[var_r31 + 33].z);
            Hu3DModelRotSet(var_r30->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(var_r30->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(var_r30->model, 1U);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f31 = fn_1_156C(-540.0f, 0.0f, (f32)var_r27, 10.0f);
        Hu3DModelRotSet(var_r30->model, 0.0f, var_f31, 0.0f);
        var_f31 = fn_1_156C(0.0f, 1.25f, (f32)var_r27, 10.0f);
        Hu3DModelScaleSet(var_r30->model, var_f31, var_f31, 1.0f);
        var_r31 += 1;
    }
    if ((u32) arg0->work[0] == 0) {
        var_r31 = 0;
        lbl_1_bss_804[0] = 0;
        while (var_r31 < 4) {
            if (HuPadStatGet(var_r31) == 0) {
                lbl_1_bss_804[0] += 1;
            }
            var_r31 += 1;
        }
        if (lbl_1_bss_804[0] < var_r21) {
            lbl_1_bss_804[0] = var_r21;
        }
        var_r31 = 0;
        while (var_r31 < 4) {
            if (var_r31 < lbl_1_bss_804[0]) {
                fn_1_9E00((s16)(var_r31 + 17), 19);
            } else {
                fn_1_9E00((s16)(var_r31 + 17), 18);
            }
            var_r31 += 1;
        }
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        s16 var_r26;
        MDMinidllModelAnimationRecord *var_r29;
        f32 var_f30;
        var_r26 = (s16) arg0->work[0];
        var_r29 = &lbl_1_bss_230[(s16) (var_r31 + 17)];
        if (var_r26 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(52);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(var_r29->model, lbl_1_data_4C8[var_r31 + 33].x, lbl_1_data_4C8[var_r31 + 33].y, lbl_1_data_4C8[var_r31 + 33].z);
            Hu3DModelRotSet(var_r29->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(var_r29->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(var_r29->model, 1U);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f30 = fn_1_156C(-540.0f, 0.0f, (f32)var_r26, 10.0f);
        Hu3DModelRotSet(var_r29->model, 0.0f, var_f30, 0.0f);
        var_f30 = fn_1_156C(0.0f, 1.25f, (f32)var_r26, 10.0f);
        Hu3DModelScaleSet(var_r29->model, var_f30, var_f30, 1.0f);
        var_r31 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.camera_target_17294"
#pragma section const_type ".rodata.camera_target_17294"
void fn_1_17294(OMOBJ *obj, MDMinidllCameraState *state)
{
    state->centerTarget.x = lbl_1_rodata_74;
    state->centerTarget.y = lbl_1_rodata_DC;
    state->centerTarget.z = lbl_1_rodata_E0;
    state->rotationTarget.x = lbl_1_rodata_E4;
    state->rotationTarget.y = lbl_1_rodata_74;
    state->rotationTarget.z = lbl_1_rodata_74;
    state->zoomTarget = lbl_1_rodata_EC;
    fn_1_1340(&state->center.x, &state->centerTarget.x, 15.0f);
    fn_1_1340(&state->rotation.x, &state->rotationTarget.x, 15.0f);
    state->zoom = fn_1_1310(state->zoom, state->zoomTarget, 15.0f);
}

extern struct _struct_lbl_1_data_89A_0x12 lbl_1_data_89A[];
extern s16 lbl_1_bss_28;
#pragma section code_type ".text.model_config_enter_f6f8"
#pragma section const_type ".rodata.model_selection_11e14"
void fn_1_F6F8(OMOBJ *arg0)
{
    s16 var_r31;
    s16 var_r28;
    s16 var_r26;
    f32 var_f31;

    if ((u32) arg0->work[0] == 0) {
        MDMinidllPlayerConfig *var_r30;
        var_r31 = 0;
        while (var_r31 < 11) {
            fn_1_9E00(var_r31, var_r31);
            fn_1_9D58(var_r31, 1, var_r31, 2);
            fn_1_9D58(var_r31, 3, var_r31, 2);
            lbl_1_data_89A[var_r31].unk10 = -1;
            var_r31 += 1;
        }
        if (lbl_1_bss_28 == 0) {
            fn_1_9E00(10, 11);
        }
        var_r26 = 0;
        var_r30 = lbl_1_bss_7CC;
        while (var_r26 < 4) {
            var_r30->unknown_00 = 0;
            var_r30->unknown_02 = 0;
            var_r30->comDif = 0;
            var_r30->charNo = var_r26;
            var_r30->padNo = var_r26;
            var_r26 += 1;
            var_r30 += 1;
        }
    }
    var_r31 = 0;
    while (var_r31 < 11) {
        MDMinidllModelAnimationRecord *var_r29;
        var_r28 = (s16) arg0->work[0];
        var_r29 = &lbl_1_bss_230[var_r31];
        if (var_r28 == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(52);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(var_r29->model, lbl_1_data_4C8[var_r31 + 14].x, lbl_1_data_4C8[var_r31 + 14].y, lbl_1_data_4C8[var_r31 + 14].z);
            Hu3DModelRotSet(var_r29->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(var_r29->model, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(var_r29->model, 1U);
        } else {
            lbl_1_bss_34 = 0;
        }
        var_f31 = fn_1_156C(-540.0f, 0.0f, (f32) var_r28, 10.0f);
        Hu3DModelRotSet(var_r29->model, 0.0f, var_f31, 0.0f);
        var_f31 = fn_1_156C(0.0f, 1.25f, (f32) var_r28, 10.0f);
        Hu3DModelScaleSet(var_r29->model, var_f31, var_f31, 1.0f);
        var_r31 += 1;
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_config_exit_fcc0"
#pragma section const_type ".rodata.model_selection_11e14"
void fn_1_FCC0(OMOBJ *arg0)
{
    Point3d position;
    s16 var_r29;
    s16 var_r28;
    s16 var_r26;
    s16 var_r25;
    s16 var_r24;
    f32 var_f31;
    f32 var_f18;

    var_r29 = 0;
    while (var_r29 < 11) {
        var_r25 = 0;
        while (var_r25 < 4) {
            if (var_r29 != lbl_1_bss_7CC[var_r25].charNo) {
                var_r25 += 1;
                continue;
            }
            break;
        }
        if (var_r25 == 4) {
            A700ModelStateView *var_r31;
            var_r26 = (s16) arg0->work[0];
            var_r31 = &((A700ModelStateView *)lbl_1_bss_230)[var_r29];
            if (var_r26 == 0) {
                if (lbl_1_bss_34 == 0) {
                    HuAudFXPlay(53);
                    lbl_1_bss_34 = 1;
                }
                Hu3DModelPosGet(var_r31->unk0, &var_r31->unkC);
                Hu3DModelRotGet(var_r31->unk0, &var_r31->unk18);
                Hu3DModelScaleGet(var_r31->unk0, &var_r31->unk24);
            } else {
                lbl_1_bss_34 = 0;
            }
            var_f31 = fn_1_156C(var_r31->unk18.y, var_r31->unk18.y - 540.0f, (f32) var_r26, 10.0f);
            Hu3DModelRotSet(var_r31->unk0, 0.0f, var_f31, 0.0f);
            var_f31 = fn_1_156C(var_r31->unk24.x, 0.0f, (f32) var_r26, 10.0f);
            Hu3DModelScaleSet(var_r31->unk0, var_f31, var_f31, 1.0f);
            if (var_r26 <= 10) {
                Hu3DModelAttrReset(var_r31->unk0, 1U);
            }
        }
        var_r29 += 1;
    }
    if ((u32) arg0->work[0] >= 10U) {
        var_r29 = 0;
        while (var_r29 < 4) {
            A700ModelStateView *var_r30;
            var_r28 = arg0->work[0] - 10;
            var_r24 = (s16) lbl_1_bss_7CC[var_r29].charNo;
            var_r30 = &((A700ModelStateView *)lbl_1_bss_230)[var_r24];
            if (var_r28 == 0) {
                Hu3DModelPosGet(var_r30->unk0, &var_r30->unkC);
                Hu3DModelRotGet(var_r30->unk0, &var_r30->unk18);
                Hu3DModelScaleGet(var_r30->unk0, &var_r30->unk24);
            }
            position.x = fn_1_156C(var_r30->unkC.x, lbl_1_data_4C8[var_r29 + 25].x, (f32) var_r28, 10.0f);
            position.y = fn_1_156C(var_r30->unkC.y, lbl_1_data_4C8[var_r29 + 25].y, (f32) var_r28, 10.0f);
            position.z = fn_1_156C(var_r30->unkC.z, lbl_1_data_4C8[var_r29 + 25].z, (f32) var_r28, 10.0f);
            Hu3DModelPosSet(var_r30->unk0, position.x, position.y, position.z);
            var_f18 = fn_1_156C(var_r30->unk24.x, 1.25f, (f32) var_r28, 10.0f);
            Hu3DModelScaleSet(var_r30->unk0, var_f18, var_f18, 1.0f);
            var_r29 += 1;
        }
    }
    if (arg0->work[0]++ > 20U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

extern void fn_1_259FC(Point3d *point);
extern const f32 lbl_1_rodata_264;
#pragma section code_type ".text.model_mode_exit_124a0"
#pragma section const_type ".rodata.model_scale_16c"
void fn_1_124A0(OMOBJ *arg0)
{
    Point3d rotation;
    Point3d position;
    MDMinidllModelAnimationRecord *var_r30;
    MDMinidllModelAnimationRecord *var_r31;
    s16 var_r29;
    s16 var_r27;
    OMOBJ *var_r26;
    s16 var_r25;
    s16 var_r24;
    s16 var_r23;
    f32 var_f31;
    f32 var_f18;

    var_r25 = 0;
    while (var_r25 < 6) {
        if (var_r25 != lbl_1_bss_804[1]) {
            var_r27 = (s16)arg0->work[0];
            var_r31 = &lbl_1_bss_230[(s16)(var_r25 + 11)];
            if (var_r27 == 0) {
                if (lbl_1_bss_34 == 0) {
                    HuAudFXPlay(53);
                    lbl_1_bss_34 = 1;
                }
                Hu3DModelPosGet(var_r31->model,
                                (HuVecF *)&var_r31->unknown_0A[2]);
                Hu3DModelRotGet(var_r31->model,
                                (HuVecF *)&var_r31->unknown_0A[14]);
                Hu3DModelScaleGet(var_r31->model,
                                  (HuVecF *)&var_r31->unknown_0A[26]);
            } else {
                lbl_1_bss_34 = 0;
            }
            var_f31 = fn_1_156C(((HuVecF *)&var_r31->unknown_0A[14])->y,
                ((HuVecF *)&var_r31->unknown_0A[14])->y - 540.0f, (f32)var_r27, 10.0f);
            Hu3DModelRotSet(var_r31->model, lbl_1_rodata_74, var_f31,
                            lbl_1_rodata_74);
            var_f31 = fn_1_156C(((HuVecF *)&var_r31->unknown_0A[26])->x,
                0.0f, (f32)var_r27, 10.0f);
            Hu3DModelScaleSet(var_r31->model, var_f31, var_f31,
                              lbl_1_rodata_6C);
            if (var_r27 <= 10) {
                Hu3DModelAttrReset(var_r31->model, 1U);
            }
        }
        var_r25 += 1;
    }

    if ((u32)arg0->work[0] >= 10U) {
        var_r29 = arg0->work[0] - 10;
        var_r23 = lbl_1_bss_804[1] + 11;
        var_r30 = &lbl_1_bss_230[var_r23];
        if (var_r29 == 0) {
            Hu3DModelPosGet(var_r30->model,
                            (HuVecF *)&var_r30->unknown_0A[2]);
            Hu3DModelRotGet(var_r30->model,
                            (HuVecF *)&var_r30->unknown_0A[14]);
            Hu3DModelScaleGet(var_r30->model,
                              (HuVecF *)&var_r30->unknown_0A[26]);
        }
        position.x = fn_1_156C(((HuVecF *)&var_r30->unknown_0A[2])->x,
            lbl_1_data_4C8[13].x, (f32)var_r29, 10.0f);
        position.y = fn_1_156C(((HuVecF *)&var_r30->unknown_0A[2])->y,
            lbl_1_data_4C8[13].y, (f32)var_r29, 10.0f);
        position.z = fn_1_156C(((HuVecF *)&var_r30->unknown_0A[2])->z,
            lbl_1_data_4C8[13].z, (f32)var_r29, 10.0f);
        Hu3DModelPosSet(var_r30->model, position.x, position.y, position.z);
        var_f18 = fn_1_156C(((HuVecF *)&var_r30->unknown_0A[26])->x,
            1.5f, (f32)var_r29, 10.0f);
        Hu3DModelScaleSet(var_r30->model, var_f18, var_f18, lbl_1_rodata_6C);
        Hu3DModelRotGet(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model,
                        &rotation);
        rotation.y += lbl_1_rodata_264;
        Hu3DModelRotSetV(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model,
                         &rotation);
    }

    if ((u32)arg0->work[0] == 20U) {
        Hu3DLayerHookReset(2);
        fn_1_259FC(&lbl_1_data_4C8[13]);
        HuAudFXPlay(1182);
        var_r24 = lbl_1_bss_804[1];
        var_r26 = lbl_1_bss_C;
        Hu3DAnimAnimSet(var_r26->mtnId[2], lbl_1_bss_764[(var_r24 * 2) + 3]);
        Hu3DAnimAnimSet(var_r26->mtnId[3], lbl_1_bss_764[(var_r24 * 2) + 4]);
        var_r26->work[0] = 1;
        var_r26->work[1] = 0;
        Hu3DModelAttrSet(lbl_1_bss_230[lbl_1_bss_804[1] + 11].model, 1U);
    }

    if (arg0->work[0]++ > 90U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.model_spin_grow_9f24"
#pragma section const_type ".rodata.model_selection_11e14"
void fn_1_9F24(s16 arg0, s16 arg1, s16 arg2, Point3d *arg3, f32 farg0)
{
    MDMinidllModelAnimationRecord *var_r31;
    f32 var_f31;

    var_r31 = &lbl_1_bss_230[arg0];
    if (arg1 == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(52);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosSet(var_r31->model, arg3->x, arg3->y, arg3->z);
        Hu3DModelRotSet(var_r31->model, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(var_r31->model, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(var_r31->model, 1U);
    } else {
        lbl_1_bss_34 = 0;
    }
    var_f31 = fn_1_156C(-540.0f, 0.0f, (f32) arg1, (f32) arg2);
    Hu3DModelRotSet(var_r31->model, 0.0f, var_f31, 0.0f);
    var_f31 = fn_1_156C(0.0f, farg0, (f32) arg1, (f32) arg2);
    Hu3DModelScaleSet(var_r31->model, var_f31, var_f31, 1.0f);
}

#pragma section code_type ".text.sprite_row_reveal"
extern const f32 lbl_1_rodata_1A0;
void fn_1_7F8C(s16 arg0, s16 arg1) {
    s16 var_r31;
    HUSPR_GROUP *var_r29;
    HUSPR_GROUPID var_r28;

    lbl_1_bss_80[arg0].x = lbl_1_rodata_1A0;
    lbl_1_bss_80[arg0].y = lbl_1_rodata_1A0;
    var_r28 = lbl_1_bss_752[arg0 + 4];
    var_r29 = &HuSprGrpData[var_r28];
    var_r31 = 0;
    while (var_r31 < var_r29->sprNum) {
        HuSprAttrReset(var_r28, var_r31, 4);
        var_r31 += 1;
    }
    HuSprBankSet(lbl_1_bss_752[arg0 + 4], 0, arg1);
}
void fn_1_809C(s16 arg0, s16 arg1)
{
    lbl_1_bss_80[arg0].x = lbl_1_rodata_1A0;
    fn_1_44B8(lbl_1_bss_752[arg0 + 4], 4);
    HuSprBankSet(lbl_1_bss_752[arg0 + 4], 0, arg1);
}
#pragma section code_type ".text.model_mode_enter_148b4"
#pragma section const_type ".rodata.model_scale_16c"
void fn_1_148B4(OMOBJ *arg0)
{
    MDMinidllPlayerConfig *player;
    s16 index;

    if (lbl_1_bss_804[1] == 5) {
        index = 0;
        player = lbl_1_bss_7CC;
        while (index < 4) {
            if (index == 0) {
                fn_1_A700((s16) player->charNo, (s16) arg0->work[0], 10, &lbl_1_data_4C8[index + 25], 1.25f);
            } else {
                fn_1_809C(index, player->comDif + 5);
                fn_1_9F24((s16) player->charNo, (s16) arg0->work[0], 10, &lbl_1_data_4C8[index + 25], 1.25f);
            }
            index += 1;
            player += 1;
        }
    } else {
        index = 0;
        player = lbl_1_bss_7CC;
        while (index < 4) {
            fn_1_A700((s16) player->charNo, (s16) arg0->work[0], 10, &lbl_1_data_4C8[index + 25], 1.25f);
            index += 1;
            player += 1;
        }
    }
    if (arg0->work[0]++ > 10U) {
        arg0->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.scalar_ease"
extern const f32 lbl_1_rodata_70;
f32 fn_1_10AC(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    f32 var_f31;

    var_f31 = lbl_1_rodata_6C - farg3;
    return (farg2 * (farg3 * farg3)) +
        ((farg0 * (var_f31 * var_f31)) +
         (lbl_1_rodata_70 * (farg1 * (var_f31 * farg3))));
}

void fn_1_1108(Point3d *arg0, const Point3d *arg1, const Point3d *arg2,
               const Point3d *arg3, f32 farg0)
{
    arg0->x = fn_1_10AC(arg1->x, arg2->x, arg3->x, farg0);
    arg0->y = fn_1_10AC(arg1->y, arg2->y, arg3->y, farg0);
    arg0->z = fn_1_10AC(arg1->z, arg2->z, arg3->z, farg0);
}

/* Consumed trajectory layout: state, three vectors, elapsed and limit.
 * Existing BSS storage is defined by its retained owner, not here. */
typedef struct MDMinTrajectory {
    s16 state;
    HuVecF current;
    HuVecF from;
    HuVecF to;
    f32 elapsed;
    f32 limit;
} MDMinTrajectory;
extern MDMinTrajectory lbl_1_bss_B0[];
void fn_1_26454(s16 player, HuVecF *position, s16 kind, s16 pad);

#pragma section code_type ".text.model_trajectory_update_5f6c"
#pragma section const_type ".rodata.model_trajectory_update_5f6c"
void fn_1_5F6C(OMOBJ *arg0)
{
    Point3d sp68;
    Point3d sp5C;
    Point3d sp50;
    Point3d position;
    MDMinidllPlayerConfig *var_r29;
    MDMinTrajectory *var_r31;
    s16 var_r30;
    s16 var_r27;
    s16 var_r26;
    f32 var_f31;
    f32 progress;
    f32 var_f30;
    f32 var_f24;

    var_r27 = 4;
    if (lbl_1_bss_804[1] == 5) {
        var_r27 = 1;
    } else {
        var_r27 = 4;
    }
    var_r30 = 0;
    var_r29 = &lbl_1_bss_7CC[var_r30];
    while (var_r30 < var_r27) {
        var_r31 = &lbl_1_bss_B0[var_r30];
        if (var_r31->state == 1) {
            var_f24 = var_r31->limit;
            var_f30 = var_r31->elapsed;
            var_r26 = arg0->mdlId[var_r29->charNo];
            progress = fn_1_156C(0.0f, 1.0f, var_f30, var_f24);
            var_f31 = progress;
            fn_1_1108(&position, &var_r31->current, &var_r31->from, &var_r31->to, var_f31);
            Hu3DModelPosGet(var_r26, &sp50);
            Hu3DModelRotGet(var_r26, &sp5C);
            sp50.x -= position.x;
            sp50.z -= position.z;
            sp50.y = (f32) -(lbl_1_rodata_88 * (atan2((f64) sp50.x, -sp50.z) / lbl_1_rodata_78));
            if ((sp5C.y - sp50.y) > lbl_1_rodata_90) {
                sp5C.y -= lbl_1_rodata_94;
            } else if ((sp5C.y - sp50.y) < lbl_1_rodata_98) {
                sp5C.y += lbl_1_rodata_94;
            }
            sp5C.y = fn_1_1310(sp5C.y, sp50.y, 10.0f);
            Hu3DModelPosSet(var_r26, position.x, position.y, position.z);
            Hu3DModelRotSet(var_r26, 0.0f, sp5C.y, 0.0f);
            if ((var_r31->elapsed += 1.0f) > var_r31->limit) {
                var_r31->state = 2;
                Hu3DModelAttrSet(arg0->mdlId[var_r29->charNo], 1U);
            }
            Hu3DModelPosGet(arg0->mdlId[var_r29->charNo], &sp68);
            if (var_r29->comF == 0) {
                fn_1_26454(var_r30, &sp68, 1, var_r29->padNo);
            } else {
                fn_1_26454(var_r30, &sp68, 1, 4);
            }
        }
        var_r30 += 1;
        var_r29 += 1;
    }
    var_r30 = 0;
    while (var_r30 < var_r27) {
        var_r31 = &lbl_1_bss_B0[var_r30];
        if (var_r31->state == 2) {
            var_r30 += 1;
            continue;
        }
        break;
    }
    if (var_r30 == var_r27) {
        arg0->objFunc = NULL;
    }
}

#include "game/wipe.h"
#include "game/process.h"
extern OMOBJ *lbl_1_bss_8;
extern s32 lbl_1_data_94;
void fn_1_17894(void);
#pragma section code_type ".text.mode_selection_18a5c"
s16 fn_1_18A5C(s16 value);
#pragma section code_type ".text.model_trajectory_update_5f6c"
#pragma section code_type ".text.mode_selection_19940"
s16 fn_1_19940(s16 value);
#pragma section code_type ".text.model_trajectory_update_5f6c"
#pragma section code_type ".text.mode_selection_1a6c0"
s16 fn_1_1A6C0(s16 value);
#pragma section code_type ".text.model_trajectory_update_5f6c"
#pragma section code_type ".text.mode_selection_1bf60"
s16 fn_1_1BF60(s16 value);
#pragma section code_type ".text.model_trajectory_update_5f6c"
#pragma section code_type ".text.mode_choice_1d640"
s16 fn_1_1D640(void);
#pragma section code_type ".text.mode_exit_1e1ac"
void fn_1_1E1AC(void);

#pragma section code_type ".text.mode_driver_34e4"
void fn_1_34E4(s16 index)
{
    HuWinMesWait(lbl_1_bss_858[index]);
}

#pragma section code_type ".text.mode_driver_3b10"
void fn_1_3B10(void)
{
    if (lbl_1_data_82C[0] != -1) {
        fn_1_3474(lbl_1_data_82C[0]);
    }
    lbl_1_data_82C[0] = -1;
    lbl_1_data_830[0] = -1;
    lbl_1_data_830[1] = -1;
}

#pragma section code_type ".text.mode_driver_3bcc"
void fn_1_3BCC(void)
{
    if (lbl_1_data_82C[0] != -1) {
        fn_1_34E4(lbl_1_data_82C[0]);
    }
}

#pragma section code_type ".text.mode_driver_5b48"
s32 fn_1_5B48(void)
{
    Point3d sp8;
    OMOBJ *var_r31;

    var_r31 = lbl_1_bss_10;
    Hu3DModelPosGet(*var_r31->mdlId, &sp8);
    if (sp8.x > lbl_1_rodata_140) {
        return 0;
    }
    var_r31->work[0] = 0;
    var_r31->objFunc = fn_1_55B0;
    Hu3DMotionShiftSet(*var_r31->mdlId, var_r31->mtnId[1], lbl_1_rodata_74, lbl_1_rodata_74, 1073741825U);
    return 1;
}

#pragma section code_type ".text.mode_driver_7014"
void fn_1_7014(void)
{
    OMOBJ *var_r31;

    var_r31 = lbl_1_bss_8;
    var_r31->work[0] = 0;
    var_r31->objFunc = fn_1_6DC4;
}

#pragma section code_type ".text.mode_driver_b348"
void fn_1_B348(void (*arg0)(OMOBJ *))
{
    OMOBJ *var_r31;

    var_r31 = lbl_1_bss_24;
    *lbl_1_bss_24->mtnId = 0;
    var_r31->work[0] = 0;
    var_r31->work[1] = 0;
    var_r31->work[2] = 0;
    var_r31->work[3] = 0;
    var_r31->objFunc = arg0;
    while (*lbl_1_bss_24->mtnId == 0) {
        HuPrcVSleep();
    }
}

#pragma section code_type ".text.mode_driver_17ef4"
s32 fn_1_17EF4(void)
{
    fn_1_4E40();
    fn_1_3D24(2, 1114112, 1);
    fn_1_3BCC();
    fn_1_7014();
    return 1;
}

#pragma section code_type ".text.mode_driver_184d4"
/* No meaningful return value is produced. The unused integer-return
 * contract reproduces retail; its original spelling is unknown. All known
 * callers discard this result. Do not replace the fallthrough with a value. */
s32 fn_1_184D4(void)
{
    Point3d position;
    Hu3DModelPosGet(*lbl_1_bss_10->mdlId, &position);
    if (position.x < lbl_1_rodata_144) {
        fn_1_5B48();
        fn_1_3B10();
        HuPrcSleep(60);
    }
}

#pragma section code_type ".text.mode_driver_18650"
s32 fn_1_18650(void)
{
    fn_1_184D4();
    fn_1_4E40();
    fn_1_3D24(2, 1114113, 1);
    fn_1_3BCC();
    return 1;
}

#pragma section code_type ".text.mode_driver_1bcac"
s16 fn_1_1BCAC(s16 arg0)
{
    s16 result;
    s16 retry;
    result = 0;
    retry = 0;
    if (arg0 != 0) {
        fn_1_B348(fn_1_105DC);
    }
    fn_1_B348(fn_1_F6F8);
    switch (lbl_1_bss_804[0]) {
    case 0:
        result = fn_1_1A6C0(0);
        break;
    case 4:
        result = fn_1_19940(0);
        break;
    default:
        while (1) {
            HuPrcVSleep();
            result = fn_1_19940(retry);
            if (result != 0) {
                break;
            }
            result = fn_1_1A6C0(0);
            if (result != 1) {
                break;
            }
            retry = 1;
        }
        break;
    }
    if (result == 0) {
        fn_1_B348(fn_1_FCC0);
    } else if (result == 1) {
        fn_1_B348(fn_1_109D4);
    }
    return result;
}

#pragma section code_type ".text.mode_driver_1e86c"
s16 fn_1_1E86C(s32 unused)
{
    s16 retry;
    s16 result;
    s16 state;

    state = 0;
    result = 0;
    retry = 0;
    HuPrcSleep(5);
    if (lbl_1_bss_0 == 1) {
        fn_1_17894();
    }
    lbl_1_data_94 = HuAudSStreamPlay(6);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_0 == 0) {
        fn_1_17EF4();
        state = 0;
        retry = 0;
    } else {
        state = 3;
        retry = 0;
    }
    while (1) {
        HuPrcVSleep();
        switch (state) {
        case 0:
            fn_1_18650();
            state = 1;
            retry = 0;
            break;
        case 1:
            result = fn_1_18A5C(retry);
            if (result == 0) {
                state = 2;
                retry = 0;
            }
            break;
        case 2:
            result = fn_1_1BCAC(retry);
            if (result == 0) {
                state = 3;
                retry = 0;
            } else if (result == 1) {
                state = 1;
                retry = 1;
            }
            break;
        case 3:
            result = fn_1_1BF60(retry);
            if (result == 0) {
                state = 4;
                retry = 0;
            } else if (result == 1) {
                state = 2;
                retry = 1;
            }
            break;
        case 4:
            result = fn_1_1D640();
            if (result == 0) {
                state = 5;
                retry = 0;
            } else if (result == 1) {
                state = 0;
                retry = 0;
            } else if (result == 2) {
                state = 3;
                retry = 1;
            }
            break;
        case 5:
            fn_1_1E1AC();
            return 2;
        }
        if (result == -1) {
            return 0;
        }
    }
}

/* Only the audio handle at +4 is consumed here; prefix meaning is unknown. */
typedef struct MDMiniAudioHandleView {
    u8 unknown_00[4];
    s32 handle;
} MDMiniAudioHandleView;
extern MDMiniAudioHandleView lbl_1_bss_864;
extern MDMinTrajectory lbl_1_bss_170[2];
#pragma section code_type ".text.model_trajectory_update_4eb4"
#pragma section const_type ".rodata.model_trajectory_update_4eb4"
void fn_1_4EB4(OMOBJ *arg0)
{
    Point3d sp68;
    MDMinTrajectory *trajectory = &lbl_1_bss_170[1];

    if (trajectory->state == 0) {
        Hu3DModelPosGet(*arg0->mdlId, &sp68);
        sp68.y += lbl_1_rodata_B4;
        Hu3DModelPosSetV(*arg0->mdlId, &sp68);
        if ((trajectory->elapsed += lbl_1_rodata_6C) > trajectory->limit) {
            Hu3DModelPosGet(*arg0->mdlId, &trajectory->current);
            trajectory->state = 1;
            trajectory->elapsed = lbl_1_rodata_74;
            trajectory->limit = lbl_1_rodata_80;
        }
    }
    if (trajectory->state == 1) {
        f32 limit = trajectory->limit;
        f32 elapsed = trajectory->elapsed;
        f32 interpolation;
        f32 curveT;
        s16 modelId = *arg0->mdlId;
        Point3d rotation;
        Point3d currentPosition;
        Point3d position;

        interpolation = fn_1_156C(0.0f, 1.0f, elapsed, limit);
        curveT = interpolation;
        fn_1_1108(&position, &trajectory->current, &trajectory->from, &trajectory->to, curveT);
        Hu3DModelPosGet(modelId, &currentPosition);
        Hu3DModelRotGet(modelId, &rotation);
        currentPosition.x -= position.x;
        currentPosition.z -= position.z;
        currentPosition.y = (f32) -(lbl_1_rodata_88 * (atan2((f64) currentPosition.x, -(f64) currentPosition.z) / lbl_1_rodata_78));
        if ((rotation.y - currentPosition.y) > lbl_1_rodata_90) {
            rotation.y -= lbl_1_rodata_94;
        } else if ((rotation.y - currentPosition.y) < lbl_1_rodata_98) {
            rotation.y += lbl_1_rodata_94;
        }
        rotation.y = fn_1_1310(rotation.y, currentPosition.y, 10.0f);
        Hu3DModelPosSet(modelId, position.x, position.y, position.z);
        Hu3DModelRotSet(modelId, lbl_1_rodata_74, rotation.y, lbl_1_rodata_74);
        if ((trajectory->elapsed += lbl_1_rodata_6C) > trajectory->limit) {
            HuAudFXStop(lbl_1_bss_864.handle);
            trajectory->state = 2;
        }
    }
    Hu3DModelPosGet(*arg0->mdlId, &sp68);
    fn_1_26454(5, &sp68, 1, 6);
    if (trajectory->state == 2) {
        arg0->objFunc = NULL;
    }
}

#pragma section code_type ".text.model_trajectory_update_1fac"
#pragma section const_type ".rodata.model_trajectory_update_1fac"
void fn_1_1FAC(s16 modelId, const Point3d *start, const Point3d *control,
               const Point3d *end, f32 elapsed, f32 limit)
{
    f32 curveT;
    Point3d curve;
    Point3d position;
    Point3d rotation;

    curveT = fn_1_156C(0.0f, 1.0f, elapsed, limit);
    fn_1_1108(&curve, start, control, end, curveT);
    Hu3DModelPosGet(modelId, &position);
    Hu3DModelRotGet(modelId, &rotation);
    position.x -= curve.x;
    position.z -= curve.z;
    position.y = (f32) -(lbl_1_rodata_88 *
        (atan2((f64) position.x, -(f64) position.z) / lbl_1_rodata_78));
    if ((rotation.y - position.y) > lbl_1_rodata_90) {
        rotation.y -= lbl_1_rodata_94;
    } else if ((rotation.y - position.y) < lbl_1_rodata_98) {
        rotation.y += lbl_1_rodata_94;
    }
    rotation.y = fn_1_1310(rotation.y, position.y, 10.0f);
    Hu3DModelPosSet(modelId, curve.x, curve.y, curve.z);
    Hu3DModelRotSet(modelId, lbl_1_rodata_74, rotation.y, lbl_1_rodata_74);
}

extern OMOBJ *lbl_1_bss_30;
#pragma section code_type ".text.model_mode_exit_1325c"
void fn_1_1325C(OMOBJ *obj);
#pragma section code_type ".text.model_trajectory_update_1fac"
void fn_1_1406C(OMOBJ *obj);
void fn_1_14484(OMOBJ *obj);
void fn_1_4BE0(OMOBJ *obj);

#pragma section code_type ".text.mode_choice_1d640"
#pragma section const_type ".rodata.mode_choice_1d640"
s16 fn_1_1D640(void)
{
    OMOBJ *entryModel;
    OMOBJ *retryModel;
    s16 result = 0;

    fn_1_B348(fn_1_1325C);
    HuPrcVSleep();
    entryModel = lbl_1_bss_10;
    Hu3DMotionShiftSet(*entryModel->mdlId, entryModel->mtnId[3], lbl_1_rodata_74, 10.0f, 0U);
    entryModel->work[3] = 0;
    entryModel->objFunc = fn_1_4D98;
    fn_1_3D24(2, 1114131, 1);
    result = fn_1_3C2C(2);
    if (result == 0) {
        fn_1_3D24(2, 1114133, 1);
        HuPrcSleep(60);
        Hu3DMotionShiftSet(*lbl_1_bss_10->mdlId, lbl_1_bss_10->mtnId[4],
                           0.0f, lbl_1_rodata_B4, 0U);
        lbl_1_bss_30 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_4BE0);
        lbl_1_bss_30->work[0] = 0;
        HuAudFXPlay(1184);
        HuPrcSleep(60);
        fn_1_3B10();
        fn_1_B348(fn_1_1406C);
        result = 0;
    } else {
        retryModel = lbl_1_bss_10;
        Hu3DMotionShiftSet(*retryModel->mdlId, retryModel->mtnId[3], lbl_1_rodata_74, 10.0f, 0U);
        retryModel->work[3] = 0;
        retryModel->objFunc = fn_1_4D98;
        fn_1_3D24(2, 1114132, 1);
        result = fn_1_3C2C(2);
        if (result == 0) {
            fn_1_B348(fn_1_14484);
            result = 1;
        } else {
            fn_1_B348(fn_1_148B4);
            result = 2;
        }
    }
    return result;
}

#include "game/printfunc.h"
/* Target-backed readonly consumed views; original declarations are unknown. */
extern const f32 lbl_1_rodata_A4, lbl_1_rodata_A8, lbl_1_rodata_AC, lbl_1_rodata_B0;
extern char lbl_1_data_7C8[], lbl_1_data_7EA[], lbl_1_data_804[], lbl_1_data_81E[];
#pragma section code_type ".text.camera_debug_2878"
#pragma section const_type ".rodata.camera_target_17294"
void fn_1_2878(OMOBJ *obj, MDMinidllCameraState *state)
{
    f32 subStickY;
    f32 subStickX;
    f32 worldX;
    f32 worldZ;
    f32 stickX;
    f32 stickY;

    stickX = (f32)HuPadStkX[0];
    stickY = (f32)-HuPadStkY[0];
    subStickY = (f32)-HuPadSubStkY[0];
    subStickX = (f32)HuPadSubStkX[0];
    state->rotationTarget.x += lbl_1_rodata_A4 * subStickY;
    if (state->rotationTarget.x > lbl_1_rodata_A8) {
        state->rotationTarget.x = lbl_1_rodata_A8;
    }
    if (state->rotationTarget.x < lbl_1_rodata_AC) {
        state->rotationTarget.x = lbl_1_rodata_AC;
    }
    state->rotationTarget.y += lbl_1_rodata_A4 * subStickX;
    if ((HuPadBtn[0] & 64) != 0) {
        state->centerTarget.y -= lbl_1_rodata_B0 * stickY;
    } else if ((HuPadBtn[0] & 32) != 0) {
        state->zoomTarget += lbl_1_rodata_B0 * stickY;
        if (state->zoomTarget < lbl_1_rodata_B4) {
            state->zoomTarget = lbl_1_rodata_B4;
        }
    } else {
        worldX = (f32)(((f64)stickX * sin((lbl_1_rodata_78 * (f64)(lbl_1_rodata_80 + state->rotation.y)) / lbl_1_rodata_88)) +
            ((f64)stickY * sin((lbl_1_rodata_78 * (f64)state->rotation.y) / lbl_1_rodata_88)));
        worldZ = (f32)(((f64)stickX * cos((lbl_1_rodata_78 * (f64)(lbl_1_rodata_80 + state->rotation.y)) / lbl_1_rodata_88)) +
            ((f64)stickY * cos((lbl_1_rodata_78 * (f64)state->rotation.y) / lbl_1_rodata_88)));
        state->centerTarget.x += lbl_1_rodata_B0 * worldX;
        state->centerTarget.z += lbl_1_rodata_B0 * worldZ;
    }
    fn_1_25B4(state, 15.0f);
    print8(16, 110, lbl_1_rodata_6C, lbl_1_data_7C8);
    print8(16, 120, lbl_1_rodata_6C, lbl_1_data_7EA, state->center.x, state->center.y, state->center.z);
    print8(16, 130, lbl_1_rodata_6C, lbl_1_data_804, state->rotation.x, state->rotation.y, state->rotation.z);
    print8(16, 140, lbl_1_rodata_6C, lbl_1_data_81E, state->zoom);
}

#pragma section code_type ".text.scalar_sine_1484"
#pragma section const_type ".rodata.scalar_sine_1484"
f32 fn_1_1484(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    if (farg2 <= lbl_1_rodata_74) {
        return farg0;
    }
    if (farg2 >= farg3) {
        return farg1;
    }
    return (f32) ((f64) farg0 +
        ((f64) (farg1 - farg0) *
         sin((lbl_1_rodata_78 *
              (f64) ((90.0f / farg3) * farg2)) /
             lbl_1_rodata_88)));
}

#pragma section code_type ".text.scalar_arc_1ed8"
#pragma section const_type ".rodata.scalar_arc_1ed8"
f32 fn_1_1ED8(f32 farg0, f32 farg1, f32 farg2, f32 farg3)
{
    if ((farg2 <= lbl_1_rodata_74) || (farg2 >= farg3)) {
        return farg0;
    }
    return (f32) ((f64) farg0 +
        ((f64) (farg1 - farg0) *
         sin((lbl_1_rodata_78 *
              (f64) ((180.0f / farg3) * farg2)) /
             lbl_1_rodata_88)));
}

extern MDMinTrajectory lbl_1_bss_1D0[2];
extern const f32 lbl_1_rodata_12C, lbl_1_rodata_134;
void fn_1_23EC4(s16 index, HuVecF *position, s16 kind);
#pragma section code_type ".text.object_trajectory_4790"
#pragma section const_type ".rodata.object_trajectory_4790"
void fn_1_4790(OMOBJ *arg0)
{
    Point3d position;
    MDMinTrajectory *first;
    MDMinTrajectory *second;
    s16 completed;
    OMOBJ *firstObj;
    OMOBJ *secondObj;
    f32 duration;

    completed = 0;
    duration = 60.0f;
    /* Retail retains this null object and its guarded trajectory path. */
    firstObj = NULL;
    secondObj = lbl_1_bss_10;
    first = lbl_1_bss_1D0;
    second = &lbl_1_bss_1D0[1];
    if (firstObj != NULL) {
        position.x = fn_1_1484(first->current.x, lbl_1_rodata_12C, first->elapsed, duration);
        position.y = fn_1_1ED8(first->current.y, lbl_1_rodata_130, first->elapsed, duration);
        position.z = first->current.z;
        if ((first->elapsed += lbl_1_rodata_6C) > duration) {
            completed += 1;
            fn_1_23EC4(0, NULL, 0);
        } else {
            fn_1_23EC4(0, &position, 1);
        }
    }
    if (secondObj != NULL) {
        position.x = fn_1_1484(second->current.x, lbl_1_rodata_134, second->elapsed, duration);
        position.y = fn_1_1ED8(second->current.y, lbl_1_rodata_130, second->elapsed, duration);
        position.z = second->current.z;
        if ((second->elapsed += lbl_1_rodata_6C) > duration) {
            completed += 1;
            fn_1_23EC4(1, NULL, 0);
        } else {
            fn_1_23EC4(1, &position, 1);
        }
    }
    if (completed >= 2) {
        arg0->objFunc = NULL;
    }
}


#pragma section code_type ".text.scalar_row_reset_8188"
#define _MATH_H
#include "game/main.h"
#include "REL/mdminidll/row_state.h"

/* Target-backed readonly f32 scalar; original declaration/name unknown. */
extern const f32 lbl_1_rodata_74;

void fn_1_8188(s16 arg0)
{
    lbl_1_bss_80[arg0].x = lbl_1_rodata_74;
}

#pragma section code_type ".text.position_offset_8914"
#define _MATH_H
#include "dolphin/mtx.h"

extern Point3d lbl_1_bss_50[];
/* Existing target readonly storage; original declaration/name unknown. */
extern const f32 lbl_1_rodata_1C0;

void fn_1_8914(s16 index, const Point3d *position)
{
    lbl_1_bss_50[index].x = position->x - lbl_1_rodata_1C0;
    lbl_1_bss_50[index].y = lbl_1_rodata_1C0 + position->y;
    lbl_1_bss_50[index].z = lbl_1_rodata_1C0 + position->z;
}

#pragma section code_type ".text.model_sprite_reset_8b70"
#define _MATH_H
#include "game/object.h"
#include "game/hu3d.h"
#include "game/sprite.h"

extern OMOBJ *lbl_1_bss_1C;
extern s16 lbl_1_bss_752[];
extern const f32 lbl_1_rodata_1C4;

void fn_1_8B70(s32 arg0)
{
    OMOBJ *var_r31;

    var_r31 = lbl_1_bss_1C;
    Hu3DModelAttrSet(var_r31->mdlId[(s16)arg0], 1U);
    HuSprScaleSet(lbl_1_bss_752[3], (s16)arg0, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
    HuSprAttrSet(lbl_1_bss_752[3], (s16)arg0, 4);
}

#pragma section code_type ".text.model_sprite_layout_899c"
#define _MATH_H
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"

/*
 * Target-backed consumed view only: lbl_1_bss_50 is a 0x30-byte BSS owner.
 * fn_1_8914, fn_1_899C, fn_1_8C08, fn_1_D554, fn_1_DD00, fn_1_E1C0, and
 * fn_1_F0A8 access four records at stride 0xC, reading/writing three floats
 * at +0/+4/+8. Point3d provides that observed ABI layout; this defines no
 * storage and asserts no higher-level meaning.
 */
extern Point3d lbl_1_bss_50[4];
extern OMOBJ *lbl_1_bss_1C;
extern HUSPR_GROUPID lbl_1_bss_752[];

/* Retail relocations in fn_1_899C consume these existing four-byte scalars. */
extern const f32 lbl_1_rodata_6C;
extern const f32 lbl_1_rodata_74;
extern const f32 lbl_1_rodata_90;
extern const f32 lbl_1_rodata_1C0;
extern const f32 lbl_1_rodata_1C4;

void fn_1_899C(s32 index, Point3d *position, s16 bank)
{
    OMOBJ *object = lbl_1_bss_1C;

    Hu3DModelPosSet(object->mdlId[(s16)index], position->x, position->y, position->z);
    Hu3DModelRotSet(object->mdlId[(s16)index], lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_90);
    Hu3DModelScaleSet(object->mdlId[(s16)index], lbl_1_rodata_6C, lbl_1_rodata_6C, lbl_1_rodata_6C);

    lbl_1_bss_50[(s16)index].x = position->x - lbl_1_rodata_1C0;
    lbl_1_bss_50[(s16)index].y = lbl_1_rodata_1C0 + position->y;
    lbl_1_bss_50[(s16)index].z = lbl_1_rodata_1C0 + position->z;

    Hu3DModelAttrReset(object->mdlId[(s16)index], 1U);
    HuSprScaleSet(lbl_1_bss_752[3], (s16)index, lbl_1_rodata_1C4, lbl_1_rodata_1C4);
    HuSprAttrReset(lbl_1_bss_752[3], (s16)index, 4);
    HuSprBankSet(lbl_1_bss_752[3], (s16)index, bank);
}

#pragma section code_type ".text.model_rotation_d450"
#define _MATH_H
#include "game/hu3d.h"
#include "REL/mdminidll/model_animation.h"

/* Consumed views of existing readonly scalars: 6.0f, 360.0f and +0.0f.
 * Original declarations/names are unknown; these define no new storage. */
extern const f32 lbl_1_rodata_248;
extern const f32 lbl_1_rodata_94;
extern const f32 lbl_1_rodata_74;

void fn_1_D450(s16 arg0)
{
    Point3d spC;
    MDMinidllModelAnimationRecord *entry;

    entry = &lbl_1_bss_230[arg0];
    Hu3DModelRotGet(entry->model, &spC);
    spC.y += lbl_1_rodata_248;
    if (spC.y > lbl_1_rodata_94) {
        spC.y -= lbl_1_rodata_94;
    }
    Hu3DModelRotSetV(entry->model, &spC);
}

void fn_1_D4EC(s16 arg0)
{
    MDMinidllModelAnimationRecord *entry;

    entry = &lbl_1_bss_230[arg0];
    Hu3DModelRotSet(entry->model, lbl_1_rodata_74,
                    lbl_1_rodata_74, lbl_1_rodata_74);
}

#include "game/charman.h"
void fn_1_25BF4(s16, Point3d *, s16);
#pragma section code_type ".text.player_selection_d554"
void fn_1_D554(OMOBJ *var_r19)
{
    MDMinidllPlayerConfig *var_r31;
    s16 var_r30;
    s16 var_r26;

    var_r30 = 0;
    var_r31 = lbl_1_bss_7CC;
    while (var_r30 < 4) {
        if (var_r31->comF == 0) {
            if (var_r31->unknown_00 == 0) {
                if (var_r19->work[var_r30]++ > 10U) {
                    if (fn_1_97D8(var_r31->padNo, &var_r31->charNo, (struct _struct_lbl_1_data_89A_0x12 (*)[]) lbl_1_data_89A, 11) != 0) {
                        fn_1_8914(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14]);
                        var_r19->work[var_r30] = 0;
                    } else if ((s32) (HuPadBtnDown[var_r31->padNo] & 256) != 0) {
                        if ((lbl_1_bss_28 == 0) && (var_r31->charNo == 10)) {
                            HuAudFXPlay(4);
                        } else {
                            CharFXPlay(var_r31->charNo, 587);
                            var_r31->unknown_00 = 1;
                            fn_1_8B70(var_r30);
                            fn_1_809C(var_r30, var_r31->padNo);
                            fn_1_9D58((s16)var_r31->charNo, 0, (s16)var_r31->charNo, 1);
                            fn_1_9D58((s16)var_r31->charNo, 2, (s16)var_r31->charNo, 1);
                            fn_1_25BF4(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14], var_r31->padNo);
                        }
                    }
                }
            } else if ((s32) (HuPadBtnDown[var_r31->padNo] & 512) != 0) {
                var_r31->unknown_00 = 0;
                fn_1_899C(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14], var_r31->padNo);
                fn_1_8188(var_r30);
                fn_1_9D58((s16)var_r31->charNo, 0, (s16)var_r31->charNo, 0);
                fn_1_9D58((s16)var_r31->charNo, 2, (s16)var_r31->charNo, 0);
            }
        }
        var_r30 += 1;
        var_r31 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 11) {
        var_r26 = 0;
        var_r31 = lbl_1_bss_7CC;
        while (var_r26 < 4) {
            if ((var_r31->comF != 0) || (var_r31->charNo != var_r30) || (var_r31->unknown_00 != 0)) {
                var_r26 += 1;
                var_r31 += 1;
                continue;
            }
            break;
        }
        if (var_r26 == 4) {
            fn_1_D4EC(var_r30);
        } else {
            fn_1_D450(var_r30);
        }
        var_r30 += 1;
    }
}


#define _MATH_H
#include "game/hu3d.h"
#include "game/sprite.h"

/* Target-backed owners from the original application translation unit. */
extern const Point3d lbl_1_rodata_1B0;
extern const f32 lbl_1_rodata_70;
extern HUSPR_GROUPID lbl_1_bss_752[];
extern f32 lbl_1_bss_38[];
extern s16 lbl_1_data_86E[];

#pragma section code_type ".text.sprite_project_position"
void fn_1_84D8(s16 arg0, Point3d *arg1, f32 farg0, f32 farg1)
{
    Point3d position;

    position = lbl_1_rodata_1B0;
    if (arg1) {
        Hu3D3Dto2D(arg1, 1, &position);
    }
    HuSprPosSet(lbl_1_bss_752[2], arg0, position.x + farg0, position.y + farg1);
}

void fn_1_8570(s16 arg0, Point3d *arg1, f32 farg0, f32 farg1)
{
    fn_1_84D8(arg0, arg1, farg0, farg1);
    HuSprScaleSet(lbl_1_bss_752[2], arg0, lbl_1_rodata_70, lbl_1_rodata_70);
    HuSprAttrReset(lbl_1_bss_752[2], arg0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_38[arg0] = lbl_1_rodata_70;
    lbl_1_data_86E[arg0] = 1;
}

#pragma section code_type ".text.sprite_bank_81b0"
#define _MATH_H
#include "game/sprite.h"
extern s16 lbl_1_bss_752[9];

void fn_1_81B0(s16 player, s16 bank)
{
    HuSprBankSet(lbl_1_bss_752[player + 4], 0, bank);
}

extern const f32 lbl_1_rodata_24C, lbl_1_rodata_250, lbl_1_rodata_254;
void fn_1_F0A8(OMOBJ *);

extern s16 lbl_1_bss_2A;
#pragma section code_type ".text.computer_player_selection_e1c0"
void fn_1_E1C0(OMOBJ *arg0)
{
    Point3d sp5C;
    MDMinidllPlayerConfig *var_r31;
    s16 var_r30;
    s16 var_r29;

    for (var_r30 = 0, var_r31 = lbl_1_bss_7CC; var_r30 < 4; var_r30 += 1, var_r31 += 1) {
        if ((var_r31->comF == 1) && (var_r31->unknown_00 != 2)) {
            if (var_r31->unknown_00 == 0) {
                if (arg0->work[0]++ > 10U) {
                    if (fn_1_97D8(0, &var_r31->charNo, (struct _struct_lbl_1_data_89A_0x12 (*)[])lbl_1_data_89A, 11) != 0) {
                        fn_1_8914(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14]);
                        arg0->work[0] = 0;
                    } else if ((s32)(HuPadBtnDown[0] & 256) != 0) {
                        if ((lbl_1_bss_28 == 0) && (var_r31->charNo == 10)) {
                            HuAudFXPlay(4);
                        } else {
                            CharFXPlay(var_r31->charNo, 587);
                            var_r31->unknown_00 = 1;
                            var_r31->comDif = 1;
                            fn_1_8B70(var_r30);
                            fn_1_809C(var_r30, var_r31->comDif + 5);
                            fn_1_9D58((s16)var_r31->charNo, 0, (s16)var_r31->charNo, 1);
                            fn_1_9D58((s16)var_r31->charNo, 2, (s16)var_r31->charNo, 1);
                            Hu3DModelPosGet(lbl_1_bss_230[var_r31->charNo].model, &sp5C);
                            fn_1_8570(4, &sp5C, lbl_1_rodata_24C, lbl_1_rodata_250);
                            fn_1_8570(5, &sp5C, lbl_1_rodata_254, lbl_1_rodata_250);
                            fn_1_25BF4(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14], 4);
                        }
                    } else if ((s32)(HuPadBtnDown[0] & 512) != 0) {
                        HuAudFXPlay(3);
                        lbl_1_data_89A[var_r31->charNo].unk10 = -1;
                        fn_1_8B70(var_r30);
                        var_r29 = 3;
                        while (var_r29 >= 0) {
                            if ((lbl_1_bss_7CC[var_r29].comF == 1) && (lbl_1_bss_7CC[var_r29].unknown_00 == 2)) {
                                lbl_1_bss_7CC[var_r29].unknown_00 = 0;
                                lbl_1_data_89A[lbl_1_bss_7CC[var_r29].charNo].unk10 = -1;
                                fn_1_8188(var_r29);
                                fn_1_9D58((s16)lbl_1_bss_7CC[var_r29].charNo, 0, (s16)lbl_1_bss_7CC[var_r29].charNo, 0);
                                fn_1_9D58((s16)lbl_1_bss_7CC[var_r29].charNo, 2, (s16)lbl_1_bss_7CC[var_r29].charNo, 0);
                            } else {
                                var_r29 -= 1;
                                continue;
                            }
                            break;
                        }
                        if (var_r29 != -1) {
                            arg0->objFunc = fn_1_F0A8;
                        } else {
                            arg0->objFunc = NULL;
                        }
                    }
                }
            } else if (var_r31->unknown_00 == 1) {
                if (arg0->work[0]++ > 10U) {
                    if ((s32)(HuPadDStkRep[0] & 1) != 0) {
                        var_r31->comDif -= 1;
                        if (lbl_1_bss_2A == 0) {
                            if (var_r31->comDif < 0) {
                                var_r31->comDif += 3;
                            }
                        } else if (var_r31->comDif < 0) {
                            var_r31->comDif += 4;
                        }
                        arg0->work[0] = 0;
                        lbl_1_bss_38[4] = lbl_1_rodata_70;
                        if (lbl_1_data_86E[4] == 1) {
                            HuAudFXPlay(0);
                        }
                    } else if ((s32)(HuPadDStkRep[0] & 2) != 0) {
                        var_r31->comDif += 1;
                        if (lbl_1_bss_2A == 0) {
                            if (var_r31->comDif > 2) {
                                var_r31->comDif -= 3;
                            }
                        } else if (var_r31->comDif > 3) {
                            var_r31->comDif -= 4;
                        }
                        arg0->work[0] = 0;
                        lbl_1_bss_38[5] = lbl_1_rodata_70;
                        if (lbl_1_data_86E[5] == 1) {
                            HuAudFXPlay(0);
                        }
                    } else if ((s32)(HuPadBtnDown[0] & 256) != 0) {
                        HuAudFXPlay(1);
                        var_r31->unknown_00 = 2;
                        HuSprAttrSet(lbl_1_bss_752[2], 4, 4);
                        lbl_1_data_86E[4] = 0;
                        HuSprAttrSet(lbl_1_bss_752[2], 5, 4);
                        lbl_1_data_86E[5] = 0;
                        var_r29 = 0;
                        while (var_r29 < 4) {
                            if ((lbl_1_bss_7CC[var_r29].comF != 1) || (lbl_1_bss_7CC[var_r29].unknown_00 != 0)) {
                                var_r29 += 1;
                                continue;
                            }
                            break;
                        }
                        if (var_r29 != 4) {
                            arg0->objFunc = fn_1_F0A8;
                        } else {
                            arg0->objFunc = NULL;
                        }
                    } else if ((s32)(HuPadBtnDown[0] & 512) != 0) {
                        HuAudFXPlay(3);
                        var_r31->unknown_00 = 0;
                        HuSprAttrSet(lbl_1_bss_752[2], 4, 4);
                        lbl_1_data_86E[4] = 0;
                        HuSprAttrSet(lbl_1_bss_752[2], 5, 4);
                        lbl_1_data_86E[5] = 0;
                        fn_1_9D58((s16)var_r31->charNo, 0, (s16)var_r31->charNo, 0);
                        fn_1_9D58((s16)var_r31->charNo, 2, (s16)var_r31->charNo, 0);
                        fn_1_899C(var_r30, &lbl_1_data_4C8[var_r31->charNo + 14], 4);
                        fn_1_8188(var_r30);
                    }
                }
                fn_1_81B0(var_r30, var_r31->comDif + 5);
            }
            break;
        }
    }
    var_r30 = 0;
    while (var_r30 < 11) {
        if ((var_r31->charNo == var_r30) && (var_r31->unknown_00 == 0)) {
            fn_1_D450(var_r30);
        } else {
            fn_1_D4EC(var_r30);
        }
        var_r30 += 1;
    }
}

#pragma section code_type ".text.player_count_selection_b6ec"
#pragma section const_type ".rodata.player_count_selection_b6ec"
void fn_1_B6EC(OMOBJ *var_r31)
{
    s16 var_r30;
    s16 var_r29;
    s16 var_r20;

    var_r20 = 1;
    var_r29 = 0;
    var_r30 = 0;
    while (var_r30 < 4) {
        if (HuPadStatGet(var_r30) == 0) {
            var_r29 += 1;
        }
        var_r30 += 1;
    }
    if (var_r29 < var_r20) {
        var_r29 = var_r20;
    }
    if (var_r29 >= lbl_1_bss_804[0]) {
        if (var_r31->work[0]++ > 10U) {
            if ((s32) (HuPadDStkRep[0] & 1) != 0) {
                lbl_1_bss_804[0] -= 1;
                if (lbl_1_bss_804[0] < var_r20) {
                    lbl_1_bss_804[0] = var_r29;
                }
                var_r31->work[0] = 0;
                var_r31->work[1] = 0;
                lbl_1_bss_38[4] = lbl_1_rodata_70;
                if (lbl_1_data_86E[4] == 1) {
                    HuAudFXPlay(0);
                }
            } else if ((s32) (HuPadDStkRep[0] & 2) != 0) {
                lbl_1_bss_804[0] += 1;
                if (lbl_1_bss_804[0] >= (s32) (var_r29 + 1)) {
                    lbl_1_bss_804[0] = var_r20;
                }
                var_r31->work[0] = 0;
                var_r31->work[1] = 1;
                lbl_1_bss_38[5] = lbl_1_rodata_70;
                if (lbl_1_data_86E[5] == 1) {
                    HuAudFXPlay(0);
                }
            }
            if (var_r29 == var_r20) {
                var_r31->work[0] = 12;
                if ((u32) var_r31->work[3] == 1) {
                    var_r31->work[3] = 0;
                    HuSprAttrSet(lbl_1_bss_752[2], 4, 4);
                    lbl_1_data_86E[4] = 0;
                    HuSprAttrSet(lbl_1_bss_752[2], 5, 4);
                    lbl_1_data_86E[5] = 0;
                }
            } else if ((u32) var_r31->work[3] == 0) {
                var_r31->work[3] = 1;
                fn_1_8570(4, lbl_1_data_4C8, -180.0f, 0.0f);
                fn_1_8570(5, lbl_1_data_4C8, 180.0f, 0.0f);
            }
        }
    } else {
        lbl_1_bss_804[0] = var_r29;
        var_r31->work[0] = 0;
        var_r31->work[1] = 0;
    }
    if ((u32) var_r31->work[0] <= 10U) {
        var_r30 = 0;
        while (var_r30 < 4) {
            if ((u32) var_r31->work[1] != 0) {
                if (var_r30 < lbl_1_bss_804[0]) {
                    fn_1_ACF0((s16) (var_r30 + 17), 1, 18, (s16) var_r31->work[0], 10, 1);
                } else {
                    fn_1_ACF0((s16) (var_r30 + 17), 0, 18, (s16) var_r31->work[0], 10, 1);
                }
            } else if (var_r30 < lbl_1_bss_804[0]) {
                fn_1_ACF0((s16) (var_r30 + 17), 1, 18, (s16) var_r31->work[0], 10, -1);
            } else {
                fn_1_ACF0((s16) (var_r30 + 17), 0, 18, (s16) var_r31->work[0], 10, -1);
            }
            var_r30 += 1;
        }
    }
}

#pragma section code_type ".text.model_mode_exit_1325c"
#pragma section const_type ".rodata.model_mode_exit_1325c"
void fn_1_1325C(OMOBJ *obj)
{
    MDMinidllPlayerConfig *player;
    s16 index;

    HuSprGrpPosSet(lbl_1_bss_752[1], lbl_1_rodata_18C, lbl_1_rodata_128 + (f32)(obj->work[0] * 5));
    if (lbl_1_bss_804[1] == 5) {
        index = 0;
        player = lbl_1_bss_7CC;
        while (index < 4) {
            if (index == 0) {
                fn_1_A700((s16)player->charNo, (s16)obj->work[0], 10,
                            &lbl_1_data_4C8[37], 1.25f);
            } else {
                lbl_1_bss_80[index].x = 0.0f;
                fn_1_A330((s16)player->charNo, (s16)obj->work[0], 10);
            }
            index++;
            player++;
        }
    } else {
        index = 0;
        player = lbl_1_bss_7CC;
        while (index < 4) {
            fn_1_A700((s16)player->charNo, (s16)obj->work[0], 10,
                        &lbl_1_data_4C8[index + 29], 1.25f);
            index++;
            player++;
        }
    }
    if (obj->work[0]++ > 10U) {
        obj->objFunc = NULL;
        *lbl_1_bss_24->mtnId = 1;
    }
}

#pragma section code_type ".text.mode_player_config"
void fn_1_2400(void)
{
    MDMinidllPlayerConfig *entry;
    s16 i;

    i = 0;
    entry = lbl_1_bss_7CC;
    while (i < 4) {
        entry->unknown_00 = 0;
        entry->unknown_02 = 0;
        entry->comDif = 0;
        entry->charNo = i;
        entry->padNo = i;
        i += 1;
        entry += 1;
    }
}
void fn_1_2460(void)
{
    MDMinidllPlayerConfig *entry;
    s16 remaining;
    s16 i;

    remaining = lbl_1_bss_804[0];
    lbl_1_bss_7CC->comF = 0;
    remaining -= 1;
    i = 1;
    entry = &lbl_1_bss_7CC[1];
    while (i < 4) {
        if (remaining > 0 && HuPadStatGet(i) == 0) {
            entry->comF = 0;
            remaining -= 1;
        } else {
            entry->comF = 1;
        }
        i += 1;
        entry += 1;
    }
}

#pragma section code_type ".text.mode_secondary_window"
void fn_1_4150(s32 arg0)
{
    if (lbl_1_data_82C[1] == -1) {
        lbl_1_data_82C[1] = 0;
        lbl_1_data_830[2] = -1;
        fn_1_3404(lbl_1_data_82C[1]);
    }
    if (lbl_1_data_830[2] != arg0) {
        lbl_1_data_830[2] = arg0;
        fn_1_35F4(lbl_1_data_82C[1], lbl_1_data_830[2], 0);
    }
}
void fn_1_42D4(void)
{
    if (lbl_1_data_82C[1] != -1) {
        fn_1_3474(lbl_1_data_82C[1]);
    }
    lbl_1_data_82C[1] = -1;
    lbl_1_data_830[2] = -1;
}

#pragma section code_type ".text.mode_ready_b310"
s16 fn_1_B310(void)
{
    OMOBJ *obj;
    obj = lbl_1_bss_24;
    if (obj->work[0] >= 10U) {
        return 1;
    }
    return 0;
}

#pragma section code_type ".text.mode_player_selection"
s16 fn_1_1A58C(s16 *arg0, s16 *arg1, s16 *arg2)
{
    s16 var_r31;
    s16 var_r30;

    var_r30 = 0;
    *arg0 = 0;
    *arg1 = 0;
    *arg2 = 3;
    var_r31 = 0;
    while (var_r31 < 4) {
        if (lbl_1_bss_7CC[var_r31].comF == 1) {
            *arg1 = var_r31;
        } else {
            var_r31 += 1;
            continue;
        }
        break;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_7CC[var_r31].comF != 1) ||
            (lbl_1_bss_7CC[var_r31].unknown_00 == 2)) {
            var_r31 += 1;
            continue;
        }
        break;
    }
    *arg0 = var_r31;
    var_r31 = 0;
    while (var_r31 < 4) {
        if ((lbl_1_bss_7CC[var_r31].comF == 1) &&
            (lbl_1_bss_7CC[var_r31].unknown_00 != 2)) {
            var_r30 += 1;
        }
        var_r31 += 1;
    }
    return var_r30;
}

/* Existing six-word readonly message table; consumed layout, not a claim
 * about the unknown original declaration. No storage is introduced. */
typedef struct { s32 message[6]; } ModeMessages;
extern const ModeMessages lbl_1_rodata_270;
void fn_1_C6A8(OMOBJ *obj);
void fn_1_C6D8(OMOBJ *obj);
void fn_1_F0A8(OMOBJ *obj);
void fn_1_F52C(OMOBJ *obj);
void fn_1_1240C(OMOBJ *obj);
void fn_1_118A0(OMOBJ *obj);

#pragma section code_type ".text.mode_selection_18a5c"
s16 fn_1_18A5C(s16 value)
{
    OMOBJ *entryModel;
    s16 result = 0;

    fn_1_B348(fn_1_C770);
    while (1) {
        HuPrcVSleep();
        fn_1_B348(fn_1_C6A8);
        fn_1_4150(65546);
        entryModel = lbl_1_bss_10;
        Hu3DMotionShiftSet(*entryModel->mdlId, entryModel->mtnId[3], lbl_1_rodata_74, lbl_1_rodata_A0, 0U);
        entryModel->work[3] = 0;
        entryModel->objFunc = fn_1_4D98;
        fn_1_3D24(2, lbl_1_bss_804[0] + 851977, 0);
        while (1) {
            HuPrcVSleep();
            fn_1_4150(65546);
            fn_1_3D24(2, lbl_1_bss_804[0] + 851977, 0);
            if (fn_1_B310() == 0) {
                continue;
            }
            if (HuPadBtnDown[0] & 256) {
                HuAudFXPlay(2);
                fn_1_2460();
                result = 0;
                break;
            } else if (HuPadBtnDown[0] & 512) {
                HuAudFXPlay(3);
                result = -1;
                break;
            } else if (HuPadBtnDown[0] & 1024) {
                HuAudFXPlay(3);
                result = -1;
                break;
            }
        }
        fn_1_42D4();
        fn_1_B348(fn_1_C6D8);
        if (result == -1) {
            if (fn_1_181CC()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result != -1) {
        fn_1_B348(fn_1_D09C);
    }
    return result;
}

#pragma section code_type ".text.mode_selection_1a6c0"
s16 fn_1_1A6C0(s16 unused)
{
    s16 result;
    s16 first;
    s16 active;
    s16 last;
    s16 count;

    result = 0;
    first = 0;
    active = 0;
    last = 0;
    count = 0;
    count = fn_1_1A58C(&first, &active, &last);
again:
    HuPrcVSleep();
    fn_1_4150(65546);
    fn_1_4E40();
    if (lbl_1_bss_7CC[first].unknown_00 == 0) {
        fn_1_3F5C(2, 851987 - (count - 1), 0);
        fn_1_3D24(2, 1114120, 0);
    } else {
        fn_1_3D24(2, 1114124, 0);
    }
    fn_1_B348(fn_1_F0A8);
poll:
    HuPrcVSleep();
    if (lbl_1_bss_7CC[first].unknown_00 == 0) {
        fn_1_3F5C(2, 851987 - (count - 1), 0);
        fn_1_3D24(2, 1114120, 0);
    } else {
        fn_1_3D24(2, 1114124, 0);
    }
    if (fn_1_B310() == 0) {
        goto poll;
    }
    count = fn_1_1A58C(&first, &active, &last);
    if (first > last) {
        result = 0;
    } else if ((active == first) && (lbl_1_bss_7CC[first].unknown_00 == 0) && ((HuPadBtnDown[0] & 512) != 0)) {
        HuAudFXPlay(3);
        result = 1;
    } else if ((HuPadBtnDown[0] & 1024) != 0) {
        HuAudFXPlay(3);
        result = -1;
    } else {
        goto poll;
    }
    fn_1_42D4();
    fn_1_B348(fn_1_F52C);
    if (result == -1) {
        if (!fn_1_181CC()) {
            result = -1;
        } else {
            goto again;
        }
    }
    return result;
}

#pragma section code_type ".text.mode_selection_1bf60"
s16 fn_1_1BF60(s16 arg0)
{
    ModeMessages messages;
    s16 result;

    result = 0;
    messages = lbl_1_rodata_270;
    if (arg0 != 0) {
        fn_1_B348(fn_1_1240C);
    }
    fn_1_B348(fn_1_11E14);
restart:
    HuPrcVSleep();
    fn_1_B348(fn_1_118A0);
    fn_1_4150(65546);
    fn_1_4E40();
    if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 1) {
        fn_1_3D24(2, messages.message[lbl_1_bss_804[1]], 0);
    } else if ((lbl_1_bss_804[1] == 5) && (GWBankFlagGet(7) == 0)) {
        fn_1_3D24(2, 1114135, 0);
    } else {
        fn_1_3D24(2, 1114134, 0);
    }
    while (1) {
        HuPrcVSleep();
        if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 1) {
            fn_1_3D24(2, messages.message[lbl_1_bss_804[1]], 0);
        } else if ((lbl_1_bss_804[1] == 5) && (GWBankFlagGet(7) == 0)) {
            fn_1_3D24(2, 1114135, 0);
        } else {
            fn_1_3D24(2, 1114134, 0);
        }
        if (fn_1_B310() == 0) {
            continue;
        }
        if (HuPadBtnDown[0] & 256) {
            if (lbl_1_data_6B6[lbl_1_bss_804[1]] == 0) {
                HuAudFXPlay(4);
                continue;
            }
            HuAudFXPlay(2);
            result = 0;
        } else if (HuPadBtnDown[0] & 512) {
            HuAudFXPlay(3);
            result = 1;
        } else if (HuPadBtnDown[0] & 1024) {
            HuAudFXPlay(3);
            result = -1;
        } else {
            continue;
        }
        break;
    }
    fn_1_42D4();
    fn_1_B348(fn_1_11B0C);
    if (result == -1) {
        if (fn_1_181CC()) {
            goto restart;
        }
        result = -1;
    }
    if (result == 0) {
        fn_1_B348(fn_1_124A0);
    } else if (result == 1) {
        fn_1_B348(fn_1_12E88);
    }
    return result;
}

void fn_1_DD00(OMOBJ *obj);
void fn_1_DF64(OMOBJ *obj);
void fn_1_E0B0(OMOBJ *obj);
#pragma section code_type ".text.mode_selection_19940"
s16 fn_1_19940(s16 arg0)
{
    OMOBJ *sp30;
    u32 sp2C;
    u32 sp28;
    u32 sp24;
    u32 sp20;
    s16 sp1E;
    s16 sp1C;
    s16 sp1A;
    s16 sp18;
    s16 sp16;
    s16 sp14;
    s16 sp12;
    s16 sp10;
    s16 spE;
    s16 spC;
    s16 spA;
    s16 sp8;
    u32 temp_r3;
    OMOBJ *var_r31;
    OMOBJ *var_r30;
    u32 temp_r3_2;
    s16 temp_r3_3;
    OMOBJ *var_r29;
    s16 var_r28;
    OMOBJ *var_r27;
    OMOBJ *var_r26;
    s16 var_r25;
    s16 var_r24;
    s16 var_r23;
    s16 var_r22;
    s16 var_r21;
    s16 var_r20;
    s16 var_r19;
    s16 var_r18;
    s16 var_r17;
    u32 temp_r0;

    var_r28 = 0;
    if (arg0 != 0) {
        var_r31 = lbl_1_bss_24;
        *lbl_1_bss_24->mtnId = 0;
        var_r31->work[0] = 0;
        var_r31->work[1] = 0;
        var_r31->work[2] = 0;
        var_r31->work[3] = 0;
        var_r31->objFunc = fn_1_E0B0;
        while (*lbl_1_bss_24->mtnId == 0) {
            HuPrcVSleep();
        }
    }
loop_4:
    HuPrcVSleep();
    fn_1_4150(65546);
    var_r27 = lbl_1_bss_10;
    Hu3DMotionShiftSet(*var_r27->mdlId, var_r27->mtnId[3], lbl_1_rodata_74, lbl_1_rodata_A0, 0U);
    var_r27->work[3] = 0;
    var_r27->objFunc = fn_1_4D98;
    fn_1_3D24(2, 1114119, 0);
    var_r30 = lbl_1_bss_24;
    *lbl_1_bss_24->mtnId = 0;
    var_r30->work[0] = 0;
    var_r30->work[1] = 0;
    var_r30->work[2] = 0;
    var_r30->work[3] = 0;
    var_r30->objFunc = fn_1_DD00;
    while (*lbl_1_bss_24->mtnId == 0) {
        HuPrcVSleep();
    }
loop_27:
    HuPrcVSleep();
    fn_1_3D24(2, 1114119, 0);
    var_r25 = 0;
    var_r22 = 0;
    while (var_r25 < 4) {
        if (lbl_1_bss_7CC[var_r25].unknown_00 == 1) {
            var_r22 += 1;
        }
        var_r25 += 1;
    }
    if (lbl_1_bss_804[0] <= var_r22) {
        var_r23 = 0;
    } else if (lbl_1_bss_7CC->unknown_00 == 0) {
        var_r23 = 1;
    } else {
        var_r23 = -1;
    }
    var_r28 = var_r23;
    sp30 = lbl_1_bss_24;
    if ((u32) sp30->work[0] >= 10U) {
        sp1E = 1;
    } else {
        sp1E = 0;
    }
    if (sp1E == 0) {
        goto loop_27;
    }
    if (var_r28 == 0) {
        var_r28 = 0;
    } else if ((var_r28 == 1) && ((s32) (HuPadBtnDown[0] & 512) != 0)) {
        HuAudFXPlay(3);
        var_r28 = 1;
    } else if ((s32) (HuPadBtnDown[0] & 1024) != 0) {
        HuAudFXPlay(3);
        var_r28 = -1;
    } else {
        goto loop_27;
    }
    fn_1_42D4();
    var_r29 = lbl_1_bss_24;
    *lbl_1_bss_24->mtnId = 0;
    var_r29->work[0] = 0;
    var_r29->work[1] = 0;
    var_r29->work[2] = 0;
    var_r29->work[3] = 0;
    var_r29->objFunc = fn_1_DF64;
    while (*lbl_1_bss_24->mtnId == 0) {
        HuPrcVSleep();
    }
    if (var_r28 == -1) {
        if (fn_1_181CC()) {
            goto loop_4;
        }
        var_r28 = -1;
    }
    return var_r28;
}

#include "game/armem.h"
#include "game/gamework.h"
extern OMOBJ *lbl_1_bss_14;
extern MDMinTrajectory lbl_1_bss_B0[4];
extern s16 lbl_1_bss_2C;
extern char lbl_1_data_6C2[], lbl_1_data_6F3[], lbl_1_data_704[];
extern char lbl_1_data_716[], lbl_1_data_728[], lbl_1_data_736[];
void fn_1_81C(void);
void fn_1_26394(void);
void fn_1_175F8(OMOBJ *obj, MDMinidllCameraState *state);
void fn_1_4EB4(OMOBJ *object);
void fn_1_5F6C(OMOBJ *object);
extern const f32 lbl_1_rodata_17C, lbl_1_rodata_180, lbl_1_rodata_140;
extern const f32 lbl_1_rodata_184, lbl_1_rodata_E0, lbl_1_rodata_138;
extern const f32 lbl_1_rodata_13C, lbl_1_rodata_148, lbl_1_rodata_14C;

#pragma section code_type ".text.object_trajectory_start_6520"
void fn_1_6520(void)
{
    MDMinidllPlayerConfig *config;
    MDMinTrajectory *state;
    OMOBJ *object;
    s16 count;
    s16 i;

    count = 4;
    object = lbl_1_bss_14;
    if (lbl_1_bss_804[1] == 5) {
        count = 1;
    } else {
        count = 4;
    }
    i = 0;
    config = &lbl_1_bss_7CC[i];
    while (i < count) {
        state = &lbl_1_bss_B0[i];
        state->state = 1;
        state->elapsed = lbl_1_rodata_74;
        state->limit = lbl_1_rodata_80;
        Hu3DModelPosGet(object->mdlId[config->charNo], (Point3d *) &state->current.x);
        Hu3DModelPosGet(object->mdlId[config->charNo], (Point3d *) &state->from.x);
        state->from.x += lbl_1_rodata_17C * state->from.x;
        state->from.y = lbl_1_rodata_180;
        state->from.z = lbl_1_rodata_140;
        Hu3DModelPosGet(object->mdlId[config->charNo], (Point3d *) &state->to.x);
        state->to.y = lbl_1_rodata_184;
        state->to.z = lbl_1_rodata_E0;
        Hu3DMotionShiftSet(object->mdlId[config->charNo], object->mtnId[(config->charNo * 2) + 1], lbl_1_rodata_74, lbl_1_rodata_A0, 1073741825U);
        i += 1;
        config += 1;
    }
    HuAudFXPlay(1186);
    object->objFunc = fn_1_5F6C;
}

#pragma section code_type ".text.object_trajectory_5434"
void fn_1_5434(void)
{
    MDMinTrajectory *trajectory = &lbl_1_bss_170[1];
    OMOBJ *object = lbl_1_bss_10;

    trajectory->state = 0;
    trajectory->elapsed = lbl_1_rodata_74;
    trajectory->limit = lbl_1_rodata_138;

    Hu3DModelPosGet(object->mdlId[0], &trajectory->current);
    trajectory->current.x = lbl_1_rodata_13C;
    trajectory->current.y = lbl_1_rodata_74;
    trajectory->current.z = lbl_1_rodata_140;

    Hu3DModelPosGet(object->mdlId[0], &trajectory->from);
    trajectory->from.x = lbl_1_rodata_74;
    trajectory->from.y = lbl_1_rodata_130;
    trajectory->from.z = lbl_1_rodata_144;

    Hu3DModelPosGet(object->mdlId[0], &trajectory->to);
    trajectory->to.x = lbl_1_rodata_74;
    trajectory->to.y = lbl_1_rodata_148;
    trajectory->to.z = lbl_1_rodata_14C;

    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[2],
        lbl_1_rodata_74, lbl_1_rodata_74, 0U);
    lbl_1_bss_864.handle = HuAudFXPlay(1148);
    object->objFunc = fn_1_4EB4;
}

#pragma section code_type ".text.camera_callback_2868"
void fn_1_2868(MDMinidllCameraCallback callback)
{
    lbl_1_bss_808.callback = callback;
}

#pragma section code_type ".text.player_config_apply_c90"
void fn_1_C90(void)
{
    s16 var_r31;

    var_r31 = 0;
    while (var_r31 < 4) {
        GwPlayer[var_r31].comF = lbl_1_bss_7CC[var_r31].comF;
        GwPlayer[var_r31].comDif = lbl_1_bss_7CC[var_r31].comDif;
        GwPlayer[var_r31].charNo = lbl_1_bss_7CC[var_r31].charNo;
        GwPlayer[var_r31].padNo = (s8) lbl_1_bss_7CC[var_r31].padNo;
        GwPlayer[var_r31].team = 0;
        var_r31 += 1;
    }
    var_r31 = 0;
    while (var_r31 < 4) {
        GwPlayerConf[var_r31].grpNo = 0;
        GwPlayerConf[var_r31].type = GwPlayer[var_r31].comF;
        GwPlayerConf[var_r31].comDif = GwPlayer[var_r31].comDif;
        GwPlayerConf[var_r31].charNo = GwPlayer[var_r31].charNo;
        GwPlayerConf[var_r31].padNo = GwPlayer[var_r31].padNo;
        GwPlayerConf[var_r31].grpNo = GwPlayer[var_r31].team;
        var_r31 += 1;
    }
}

#pragma section code_type ".text.resources_a4c"
void fn_1_A4C(void)
{
    lbl_1_bss_2C = 0;
    OSReport(lbl_1_data_6C2);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
    HuDataDirClose(9830400);
    HuPrcChildCreate(fn_1_81C, 256, 16384, 0, lbl_1_bss_4);
}

#pragma section code_type ".text.mode_exit_1e1ac"
void fn_1_1E1AC(void)
{
    fn_1_C90();
    fn_1_A4C();
    HuPrcSleep(10);
    fn_1_6520();
    fn_1_5434();
    HuPrcSleep(10);
    fn_1_26394();
    if (lbl_1_data_94 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_94, 3000);
        lbl_1_data_94 = -1;
    }
    HuAudFXPlay(1187);
    HuPrcSleep(90);
    fn_1_2868(fn_1_175F8);
    HuPrcSleep(60);
}

/* Keep the other two resource routines with their existing target code member. */
#include "game/charman.h"
extern char lbl_1_data_738[], lbl_1_data_767[];
#pragma section code_type ".text.resources_a4c"
void fn_1_B18(void)
{
    HuARDirFree(10420224);
    HuARDirFree(10223616);
    HuARDirFree(10289152);
    HuARDirFree(10551296);
    HuARDirFree(10354688);
    OSReport(lbl_1_data_738);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
}

void fn_1_BD0(void)
{
    CharDataClose(-1);
    HuARDirFree(10420224);
    HuARDirFree(10223616);
    HuARDirFree(10289152);
    HuARDirFree(10551296);
    HuARDirFree(10354688);
    OSReport(lbl_1_data_767);
    OSReport(lbl_1_data_6F3, 33);
    OSReport(lbl_1_data_704, 36);
    OSReport(lbl_1_data_716, 155);
    OSReport(lbl_1_data_728, 242);
    HuAMemDump();
    OSReport(lbl_1_data_736);
}
