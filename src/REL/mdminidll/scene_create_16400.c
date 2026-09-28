#define _MATH_H
#include "game/object.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/window.h"
#include "REL/mdminidll/camera.h"
#include "REL/mdminidll/model_animation.h"
#include "REL/mdminidll/readonly_scalars.h"
#include "REL/mdminidll/window_callback.h"
#include "string.h"

/* Target-backed consumed view; this declaration does not allocate storage. */
typedef struct MDMinidllSpriteLayoutEntry {
    s16 groupIndex;
    s16 memberIndex;
    s16 animationIndex;
    s16 priority;
    s16 spriteParam;
    u8 unknown_0A[2];
    f32 positionX;
    f32 positionY;
    f32 scaleX;
    f32 scaleY;
    f32 rotation;
} MDMinidllSpriteLayoutEntry;

typedef struct MDMinidllAnimationNameTable {
    char *name[4];
} MDMinidllAnimationNameTable;

extern HUPROCESS *lbl_1_bss_4;
extern s16 lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_24;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern HU3D_LIGHTID lbl_1_bss_860[2];
extern HUWINID lbl_1_bss_858[4];
extern HUSPR_GROUPID lbl_1_bss_752[9];
extern ANIMDATA *lbl_1_bss_764[26];
extern HUSPRID lbl_1_bss_71C[27];
extern ANIMDATA *lbl_1_bss_6C8[21];
extern s32 lbl_1_data_98[26];
extern s16 lbl_1_data_100[9];
extern u8 lbl_1_data_114[];
extern s32 lbl_1_data_474[21];
extern const Point3d lbl_1_rodata_104;
extern const Point3d lbl_1_rodata_110;
extern const Point3d lbl_1_rodata_11C;
extern const MDMinidllAnimationNameTable lbl_1_rodata_238;

void fn_1_17294(OMOBJ *, MDMinidllCameraState *);
void fn_1_2FD4(OMOBJ *);
void fn_1_24D80(HUPROCESS *);
void fn_1_7048(OMOBJ *);
void fn_1_7C4C(OMOBJ *);
void fn_1_5BF4(OMOBJ *);
void fn_1_6B74(OMOBJ *);
void fn_1_8430(OMOBJ *);
void fn_1_90C8(OMOBJ *);
void fn_1_15E0C(void);

void fn_1_16400(void)
{
    Point3d sp30;
    Point3d sp24;
    Point3d sp18;
    MDMinidllAnimationNameTable animationNames;
    s16 var_r30;
    MDMinidllCameraState *var_r29;
    s16 var_r28;
    s16 var_r27;
    s16 var_r26;
    s16 var_r25;
    MDMinidllSpriteLayoutEntry *var_r31;
    HUSPR_GROUP *var_r24;
    HUSPR_GROUPID var_r23;

    lbl_1_bss_4 = omInitObjMan(27, 8192);
    omGameSysInit(lbl_1_bss_4);
    var_r29 = &lbl_1_bss_808;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_C8, lbl_1_rodata_A0, lbl_1_rodata_CC, lbl_1_rodata_D0);
    Hu3DCameraViewportSet(1, lbl_1_rodata_74, lbl_1_rodata_74, lbl_1_rodata_D4, lbl_1_rodata_D8, lbl_1_rodata_74, lbl_1_rodata_6C);
    memset(var_r29, 0, 80U);
    var_r29->callback = fn_1_17294;
    var_r29->center.x = lbl_1_rodata_74;
    var_r29->center.y = lbl_1_rodata_DC;
    var_r29->center.z = lbl_1_rodata_E0;
    var_r29->rotation.x = lbl_1_rodata_E4;
    var_r29->rotation.y = lbl_1_rodata_74;
    var_r29->rotation.z = lbl_1_rodata_74;
    if (lbl_1_bss_0 == 0) {
        var_r29->zoom = lbl_1_rodata_E8;
    } else {
        var_r29->zoom = lbl_1_rodata_EC;
    }
    var_r29->outViewObj = omAddObjEx(lbl_1_bss_4, 32730, 16U, 16U, -1, fn_1_2FD4);
    lbl_1_bss_860[0] = Hu3DGLightCreate(lbl_1_rodata_74, lbl_1_rodata_6C, lbl_1_rodata_6C, lbl_1_rodata_74, lbl_1_rodata_F0, lbl_1_rodata_F0, 255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[0]);
    Hu3DGLightStaticSet(lbl_1_bss_860[0], 1);
    lbl_1_bss_860[1] = Hu3DGLightCreate(lbl_1_rodata_F0, lbl_1_rodata_6C, lbl_1_rodata_F0, lbl_1_rodata_6C, lbl_1_rodata_F0, lbl_1_rodata_F0, 255U, 255U, 255U);
    Hu3DGLightInfinitytSet(lbl_1_bss_860[1]);
    Hu3DGLightStaticSet(lbl_1_bss_860[1], 1);
    HuWinInit(1);
    lbl_1_bss_858[0] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_F8, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_858[0]);
    HuWinBGTPLvlSet(lbl_1_bss_858[0], lbl_1_rodata_74);
    lbl_1_bss_858[1] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 0);
    HuWinDispOff(lbl_1_bss_858[1]);
    HuWinBGTPLvlSet(lbl_1_bss_858[1], lbl_1_rodata_100);
    lbl_1_bss_858[2] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_858[2]);
    HuWinBGTPLvlSet(lbl_1_bss_858[2], lbl_1_rodata_100);
    lbl_1_bss_858[3] = HuWinExCreateFrame(lbl_1_rodata_F4, lbl_1_rodata_FC, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_858[3]);
    HuWinBGTPLvlSet(lbl_1_bss_858[3], lbl_1_rodata_100);
    var_r27 = 0;
    while (var_r27 < 4) {
        winData[lbl_1_bss_858[var_r27]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_858[var_r27], fn_1_0);
        var_r27 += 1;
    }
    sp18 = lbl_1_rodata_104;
    sp24 = lbl_1_rodata_110;
    sp30 = lbl_1_rodata_11C;
    Hu3DShadowCreate(lbl_1_rodata_C8, lbl_1_rodata_A0, lbl_1_rodata_CC);
    Hu3DShadowPosSet(&sp18, &sp24, &sp30);
    var_r30 = 0;
    while (var_r30 < 26) {
        lbl_1_bss_764[var_r30] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_98[var_r30], 268435456, HEAP_MODEL));
        var_r30 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 9) {
        lbl_1_bss_752[var_r30] = HuSprGrpCreate(lbl_1_data_100[var_r30]);
        var_r30 += 1;
    }
    var_r30 = 0;
    var_r31 = (MDMinidllSpriteLayoutEntry *)&lbl_1_data_114;
    while (var_r30 < 27) {
        lbl_1_bss_71C[var_r30] = HuSprCreate(lbl_1_bss_764[var_r31->animationIndex], (s16)(var_r31->priority + 6000), var_r31->spriteParam);
        HuSprGrpMemberSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, lbl_1_bss_71C[var_r30]);
        HuSprPosSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->positionX, var_r31->positionY);
        HuSprScaleSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->scaleX, var_r31->scaleY);
        HuSprZRotSet(lbl_1_bss_752[var_r31->groupIndex], var_r31->memberIndex, var_r31->rotation);
        var_r30 += 1;
        var_r31 += 1;
    }
    var_r30 = 0;
    while (var_r30 < 9) {
        var_r23 = lbl_1_bss_752[var_r30];
        var_r24 = &HuSprGrpData[var_r23];
        var_r25 = 0;
        while (var_r25 < var_r24->sprNum) {
            HuSprAttrSet(var_r23, var_r25, 4);
            var_r25 += 1;
        }
        var_r30 += 1;
    }
    animationNames = lbl_1_rodata_238;
    var_r28 = 0;
    while (var_r28 < 21) {
        lbl_1_bss_6C8[var_r28] = HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_474[var_r28], 268435456, HEAP_MODEL));
        var_r28 += 1;
    }
    for (var_r28 = 0; var_r28 < 21; var_r28++) {
        memset(&lbl_1_bss_230[var_r28], 0, 56U);
        if (var_r28 == 0) {
            lbl_1_bss_230[var_r28].model = Hu3DModelCreate(HuDataSelHeapReadNum(9830425, 268435456, HEAP_MODEL));
        } else {
            lbl_1_bss_230[var_r28].model = Hu3DModelLink(lbl_1_bss_230->model);
        }
        var_r26 = 0;
        while (var_r26 < 4) {
            lbl_1_bss_230[var_r28].animation[var_r26] = Hu3DAnimCreate(lbl_1_bss_6C8[0], lbl_1_bss_230[var_r28].model, animationNames.name[var_r26]);
            var_r26 += 1;
        }
        Hu3DModelLayerSet(lbl_1_bss_230[var_r28].model, 1);
        Hu3DModelAttrSet(lbl_1_bss_230[var_r28].model, 1U);
    }
    lbl_1_bss_24 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, NULL);
    fn_1_24D80(lbl_1_bss_4);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_7048);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_7C4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_5BF4);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 32U, -1, fn_1_6B74);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_8430);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_4, 4096, 16U, 16U, -1, fn_1_90C8);
    HuPrcChildCreate(fn_1_15E0C, 12288U, 12288U, 0, lbl_1_bss_4);
}
