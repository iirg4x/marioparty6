#include "dolphin.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/object.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

typedef void (*VoidFunc)(void);

typedef struct StaffLightVectors {
    HuVecF entries[2];
} STAFF_LIGHT_VECTORS;

typedef struct StaffModelResource {
    s32 modelData;
    s32 motionData;
} STAFF_MODEL_RESOURCE;

typedef struct StaffMotionWork {
    s16 state;
    s16 unk_02;
    f32 unk_04;
    f32 unk_08;
    HuVecF position;
    HuVecF unk_18;
    HuVecF unk_24;
    f32 unk_30;
    HuVecF unk_34;
    HuVecF unk_40;
    HuVecF unk_4C;
    HuVecF unk_58;
} STAFF_MOTION_WORK;

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_40;
extern const f32 lbl_1_rodata_44;
extern const f32 lbl_1_rodata_64;
extern const f32 lbl_1_rodata_80;
extern const f32 lbl_1_rodata_98;
extern const f32 lbl_1_rodata_B0;
extern const f32 lbl_1_rodata_B4;
extern const f32 lbl_1_rodata_B8;
extern const f32 lbl_1_rodata_BC;
extern const f32 lbl_1_rodata_C0;
extern const f32 lbl_1_rodata_C4;
extern const STAFF_LIGHT_VECTORS lbl_1_rodata_C8;
extern const STAFF_LIGHT_VECTORS lbl_1_rodata_E0;
extern const GXColor lbl_1_rodata_F8;
extern const f32 lbl_1_rodata_19C;
extern const f32 lbl_1_rodata_1A0;
extern const f32 lbl_1_rodata_1C4;
extern const f32 lbl_1_rodata_1C8;
extern const f32 lbl_1_rodata_1CC;

extern STAFF_MODEL_RESOURCE lbl_1_data_0[13];
extern char lbl_1_data_998[];
extern char lbl_1_data_9AA[];
extern char lbl_1_data_9AF[];
extern char lbl_1_data_9C1[];
extern s32 lbl_1_data_9F8[2];

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern ANIMDATA *lbl_1_bss_20;
extern f32 lbl_1_bss_2C;
extern f32 lbl_1_bss_30;
extern HU3D_MODELID lbl_1_bss_36[2];
extern HU3D_MODELID lbl_1_bss_3A[2];
extern HU3D_MODELID lbl_1_bss_3E;
extern HU3D_MODELID lbl_1_bss_40[2];
extern HU3D_MODELID lbl_1_bss_44[2];
extern ANIMDATA *lbl_1_bss_48[2];
extern f32 lbl_1_bss_124;
extern s16 lbl_1_bss_128;
extern s16 lbl_1_bss_12A;
extern HuVecF lbl_1_bss_12C[32];
extern s16 lbl_1_bss_2AC[44];
extern s16 lbl_1_bss_304[];
extern HUSPRID lbl_1_bss_828;
extern HUSPR_GROUPID lbl_1_bss_82A;
extern s32 lbl_1_bss_824;
extern HU3D_LIGHTID lbl_1_bss_82C[2];
extern STAFF_MOTION_WORK lbl_1_bss_5C[2];
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

void fn_1_884(OMOBJ *obj);
void fn_1_1110(OMOBJ *obj);
void fn_1_2BD4(OMOBJ *obj);
void fn_1_3140(OMOBJ *obj);
void fn_1_3568(OMOBJ *obj);
void fn_1_5148(OMOBJ *obj);
void fn_1_6840(void);
void fn_1_6E1C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_7670(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_7E34(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_8618(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_8FC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_96A4(void);
void fn_1_9D14(void);

void HuAudFadeOut(s32 speed);
void HuAudSStreamFadeOut(int streamNo, s32 speed);

float fn_1_0(float start, float end, float time, float duration)
{
    if (time <= lbl_1_rodata_10) {
        return start;
    }
    if (time >= duration) {
        return end;
    }
    return start + ((time / duration) * (end - start));
}

void fn_1_610(HuVecF *dst, float x, float y, float z)
{
    dst->x = x;
    dst->y = y;
    dst->z = z;
}

float fn_1_620(float a, float b, float c, float t)
{
    float inv = lbl_1_rodata_40 - t;

    return (c * (t * t))
        + ((a * (inv * inv)) + ((b * (inv * t)) * lbl_1_rodata_44));
}

inline float fn_1_620(float a, float b, float c, float t);

void fn_1_67C(
    HuVecF *dst, const HuVecF *a, const HuVecF *b, const HuVecF *c, float t)
{
    dst->x = fn_1_620(a->x, b->x, c->x, t);
    dst->y = fn_1_620(a->y, b->y, c->y, t);
    dst->z = fn_1_620(a->z, b->z, c->z, t);
}

void fn_1_E20(void)
{
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, lbl_1_rodata_B0, lbl_1_rodata_B4,
        lbl_1_rodata_B8, lbl_1_rodata_BC);
    Hu3DCameraViewportSet(1, lbl_1_rodata_10, lbl_1_rodata_10,
        lbl_1_rodata_C0, lbl_1_rodata_C4, lbl_1_rodata_10, lbl_1_rodata_40);
    Hu3DCameraPosSet(1, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_80, lbl_1_rodata_10, lbl_1_rodata_40,
        lbl_1_rodata_10, lbl_1_rodata_10, lbl_1_rodata_64,
        lbl_1_rodata_98);
}

void fn_1_F44(void)
{
    Hu3DCameraKill(1);
}

void fn_1_F68(void)
{
    STAFF_LIGHT_VECTORS positions = lbl_1_rodata_C8;
    STAFF_LIGHT_VECTORS directions = lbl_1_rodata_E0;
    GXColor color = lbl_1_rodata_F8;
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_82C[i] = Hu3DGLightCreateV(
            &positions.entries[i], &directions.entries[i], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_82C[i]);
        Hu3DGLightStaticSet(lbl_1_bss_82C[i], TRUE);
    }
}

void fn_1_10B8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DGLightKill(lbl_1_bss_82C[i]);
    }
}

void fn_1_1684(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 31; i++) {
        lbl_1_bss_2AC[i] = -1;
        lbl_1_bss_12C[i].x = lbl_1_rodata_10;
        lbl_1_bss_12C[i].y = lbl_1_rodata_10;
    }
    lbl_1_bss_12A = 0;
    lbl_1_bss_128 = 0;
    lbl_1_bss_124 = lbl_1_rodata_10;
    obj->objFunc = fn_1_1110;
}

void fn_1_2B94(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[1] = 1;
    lbl_1_bss_C->objFunc = fn_1_884;
}

void fn_1_9E30(s32 modelNo, HuVecF *pos, s32 display);

void fn_1_3140(OMOBJ *obj)
{
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    if (obj->work[0] < 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        fn_1_9E30(0, &firstWork->position, 1);
        fn_1_9E30(1, &secondWork->position, 1);
    }
    if (obj->work[0] == 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        lbl_1_bss_C->work[0] = 0;
        lbl_1_bss_C->work[1] = 180;
        lbl_1_bss_C->objFunc = fn_1_2BD4;
    }
    if (obj->work[0] == 60) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[3],
            lbl_1_rodata_10, lbl_1_rodata_B0, 0);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[3],
            lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    } else if (obj->work[0] == 180) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[4],
            lbl_1_rodata_10, lbl_1_rodata_B0, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[4],
            lbl_1_rodata_10, lbl_1_rodata_B0, HU3D_MOTATTR_LOOP);
        lbl_1_bss_8->objFunc = NULL;
    }
    obj->work[0]++;
}

void fn_1_3378(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[2],
        lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[2],
        lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    obj->work[0] = 0;
    obj->objFunc = fn_1_3140;
}

void fn_1_3460(void)
{
    HuSprAttrReset(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    lbl_1_bss_30 = lbl_1_rodata_40;
    lbl_1_bss_2C = lbl_1_rodata_10;
}

void fn_1_34E4(void)
{
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;
}

void fn_1_389C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(13434880 + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_10, lbl_1_rodata_10, HU3D_MOTATTR_LOOP);
    }
    obj->work[0] = Hu3DTexScrollCreate(obj->mdlId[1], lbl_1_data_9AA);
    Hu3DTexScrollPosMoveSet(
        obj->work[0], lbl_1_rodata_10, lbl_1_rodata_19C, lbl_1_rodata_10);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_44, lbl_1_rodata_44,
        lbl_1_rodata_44);
    obj->objFunc = fn_1_3568;
}

void fn_1_3A08(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_3A98(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(13434908);
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 13434909 + i);
    }
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_10,
        lbl_1_rodata_10, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_1A0,
        lbl_1_rodata_1A0, lbl_1_rodata_1A0);
    obj->objFunc = NULL;
}

void fn_1_3BB0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 5; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_3C38(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreateData(13434914);
    obj->mdlId[1] = Hu3DModelCreateData(13434915);
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 13434916 + i);
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_9AF, obj->mdlId[1]);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_10,
        lbl_1_rodata_10, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_1A0,
        lbl_1_rodata_1A0, lbl_1_rodata_1A0);
    obj->objFunc = NULL;
}

void fn_1_3D8C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[0]);
        for (i = 0; i < 5; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4AF4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 13; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(lbl_1_data_0[i].modelData);
        obj->mtnId[i] = Hu3DJointMotionData(
            obj->mdlId[i], lbl_1_data_0[i].motionData);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_10, lbl_1_rodata_10, HU3D_MOTATTR_LOOP);
    }
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->objFunc = NULL;
}

void fn_1_4C40(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 13; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4CD0(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    first = lbl_1_bss_4;
    if (first) {
        for (firstIndex = 0; firstIndex < 2; firstIndex++) {
            Hu3DMotionKill(first->mtnId[firstIndex]);
            Hu3DModelKill(first->mdlId[firstIndex]);
        }
        omDelObjEx(lbl_1_bss_0, first);
    }
    first = NULL;
    second = lbl_1_bss_8;
    if (second) {
        for (secondIndex = 0; secondIndex < 5; secondIndex++) {
            Hu3DMotionKill(second->mtnId[secondIndex]);
        }
        Hu3DModelKill(second->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, second);
    }
    second = NULL;
    third = lbl_1_bss_C;
    if (third) {
        Hu3DModelHookReset(third->mdlId[0]);
        for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
            Hu3DMotionKill(third->mtnId[thirdIndex]);
        }
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, third);
    }
    third = NULL;
    fourth = lbl_1_bss_10;
    if (fourth) {
        for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
            Hu3DMotionKill(fourth->mtnId[fourthIndex]);
            Hu3DModelKill(fourth->mdlId[fourthIndex]);
        }
        omDelObjEx(lbl_1_bss_0, fourth);
    }
    fourth = NULL;
    fn_1_9D14();
    for (lightIndex = 0; lightIndex < 2; lightIndex++) {
        Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
    }
    Hu3DCameraKill(1);
}

void fn_1_4EF4(OMOBJ *obj)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    if (WipeCheck() == 0) {
        HuAudFadeOut(1000);
        first = lbl_1_bss_4;
        if (first) {
            for (firstIndex = 0; firstIndex < 2; firstIndex++) {
                Hu3DMotionKill(first->mtnId[firstIndex]);
                Hu3DModelKill(first->mdlId[firstIndex]);
            }
            omDelObjEx(lbl_1_bss_0, first);
        }
        first = NULL;
        second = lbl_1_bss_8;
        if (second) {
            for (secondIndex = 0; secondIndex < 5; secondIndex++) {
                Hu3DMotionKill(second->mtnId[secondIndex]);
            }
            Hu3DModelKill(second->mdlId[0]);
            omDelObjEx(lbl_1_bss_0, second);
        }
        second = NULL;
        third = lbl_1_bss_C;
        if (third) {
            Hu3DModelHookReset(third->mdlId[0]);
            for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
                Hu3DMotionKill(third->mtnId[thirdIndex]);
            }
            Hu3DModelKill(third->mdlId[0]);
            Hu3DModelKill(third->mdlId[1]);
            omDelObjEx(lbl_1_bss_0, third);
        }
        third = NULL;
        fourth = lbl_1_bss_10;
        if (fourth) {
            for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
                Hu3DMotionKill(fourth->mtnId[fourthIndex]);
                Hu3DModelKill(fourth->mdlId[fourthIndex]);
            }
            omDelObjEx(lbl_1_bss_0, fourth);
        }
        fourth = NULL;
        fn_1_9D14();
        for (lightIndex = 0; lightIndex < 2; lightIndex++) {
            Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
        }
        Hu3DCameraKill(1);
        omOvlReturnEx(1, 1);
        obj->objFunc = NULL;
    }
}

void fn_1_528C(void)
{
    OMOBJ *first;
    OMOBJ *second;
    OMOBJ *third;
    OMOBJ *fourth;
    s16 firstIndex;
    s16 secondIndex;
    s16 thirdIndex;
    s16 fourthIndex;
    s16 lightIndex;

    fn_1_6840();
    HuAudSStreamFadeOut(lbl_1_bss_824, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    first = lbl_1_bss_4;
    if (first) {
        for (firstIndex = 0; firstIndex < 2; firstIndex++) {
            Hu3DMotionKill(first->mtnId[firstIndex]);
            Hu3DModelKill(first->mdlId[firstIndex]);
        }
        omDelObjEx(lbl_1_bss_0, first);
    }
    first = NULL;
    second = lbl_1_bss_8;
    if (second) {
        for (secondIndex = 0; secondIndex < 5; secondIndex++) {
            Hu3DMotionKill(second->mtnId[secondIndex]);
        }
        Hu3DModelKill(second->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, second);
    }
    second = NULL;
    third = lbl_1_bss_C;
    if (third) {
        Hu3DModelHookReset(third->mdlId[0]);
        for (thirdIndex = 0; thirdIndex < 5; thirdIndex++) {
            Hu3DMotionKill(third->mtnId[thirdIndex]);
        }
        Hu3DModelKill(third->mdlId[0]);
        Hu3DModelKill(third->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, third);
    }
    third = NULL;
    fourth = lbl_1_bss_10;
    if (fourth) {
        for (fourthIndex = 0; fourthIndex < 13; fourthIndex++) {
            Hu3DMotionKill(fourth->mtnId[fourthIndex]);
            Hu3DModelKill(fourth->mdlId[fourthIndex]);
        }
        omDelObjEx(lbl_1_bss_0, fourth);
    }
    fourth = NULL;
    fn_1_9D14();
    for (lightIndex = 0; lightIndex < 2; lightIndex++) {
        Hu3DGLightKill(lbl_1_bss_82C[lightIndex]);
    }
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

void fn_1_54F0(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 2, OM_GRP_NONE, fn_1_389C);
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(13434923, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, lbl_1_rodata_1C4, lbl_1_rodata_1C8);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(lbl_1_rodata_1CC,
            lbl_1_rodata_1C8, 544, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], lbl_1_rodata_10);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 12288, 12288, 0, lbl_1_bss_0);
}

void fn_1_5A7C(void)
{
    OSReport(lbl_1_data_9C1);
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    HuWinInit(1);
    fn_1_E20();
    fn_1_F68();
    fn_1_96A4();

    lbl_1_bss_4 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 2, OM_GRP_NONE, fn_1_389C);
    lbl_1_bss_8 = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3A98);
    lbl_1_bss_C = omAddObjEx(
        lbl_1_bss_0, 4096, 2, 4, OM_GRP_NONE, fn_1_3C38);
    lbl_1_bss_18 = omAddObjEx(
        lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_1684);

    lbl_1_bss_20 = HuSprAnimRead(
        HuDataSelHeapReadNum(13434923, HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_82A = HuSprGrpCreate(1);
    lbl_1_bss_828 = HuSprCreate(lbl_1_bss_20, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_82A, 0, lbl_1_bss_828);
    HuSprPosSet(
        lbl_1_bss_82A, 0, lbl_1_rodata_1C4, lbl_1_rodata_1C8);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;

    {
        s16 modelCount = 13;

        lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096,
            modelCount, modelCount, OM_GRP_NONE, fn_1_4AF4);
    }
    if (GwCommon.viewEnding) {
        lbl_1_bss_304[4] = HuWinExCreateFrame(lbl_1_rodata_1CC,
            lbl_1_rodata_1C8, 544, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_304[4]);
        HuWinBGTPLvlSet(lbl_1_bss_304[4], lbl_1_rodata_10);
        lbl_1_bss_1C = omAddObjEx(
            lbl_1_bss_0, 4096, 0, 0, OM_GRP_NONE, fn_1_5148);
    }
    GwCommon.viewEnding = TRUE;
    HuPrcChildCreate(fn_1_528C, 12288, 12288, 0, lbl_1_bss_0);
}

inline void fn_1_5A7C(void);

s32 _prolog(void)
{
    const VoidFunc *ctor = _ctors;

    while (*ctor) {
        (*ctor)();
        ctor++;
    }
    fn_1_5A7C();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;

    while (*dtor) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_6C14(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_6C90(s16 modelNo, HuVecF *pos, GXColor *color)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_44[modelNo]];
    particle = model->hookData;
    for (i = 0, entry = particle->data; i < particle->maxCnt; i++, entry++) {
        entry->time = 1;
        if (color != NULL) {
            entry->color.r = color->r;
            entry->color.g = color->g;
            entry->color.b = color->b;
        }
    }
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_44[modelNo], pos);
    }
    Hu3DModelAttrReset(lbl_1_bss_44[modelNo], HU3D_ATTR_DISPOFF);
}

void fn_1_6D94(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    s16 i;
    HU3D_PARTICLE_DATA *entry;

    model = &Hu3DData[lbl_1_bss_44[modelNo]];
    particle = model->hookData;
    for (i = 0, entry = particle->data; i < particle->maxCnt; i++, entry++) {
        entry->time = 2;
    }
}

void fn_1_72A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_44[i] = Hu3DParticleCreate(lbl_1_bss_48[0], 10);
        Hu3DModelPosSet(lbl_1_bss_44[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_44[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_44[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_44[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_44[i], fn_1_6E1C);
        Hu3DParticleBlendModeSet(lbl_1_bss_44[i], 1);
    }
}

void fn_1_7410(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_44[i]);
    }
}

void fn_1_7468(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_40[modelNo], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_761C(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_40[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

void fn_1_7C70(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_40[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_40[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_40[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_40[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_40[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_40[i], fn_1_7670);
        Hu3DParticleBlendModeSet(lbl_1_bss_40[i], 1);
    }
}

void fn_1_7DDC(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_40[i]);
    }
}

void fn_1_8300(void)
{
    lbl_1_bss_3E = Hu3DParticleCreate(lbl_1_bss_48[0], 256);
    Hu3DModelPosSet(lbl_1_bss_3E, lbl_1_rodata_10, lbl_1_rodata_10,
        lbl_1_rodata_10);
    Hu3DModelScaleSet(lbl_1_bss_3E, lbl_1_rodata_40, lbl_1_rodata_40,
        lbl_1_rodata_40);
    Hu3DModelLayerSet(lbl_1_bss_3E, 2);
    Hu3DParticleHookSet(lbl_1_bss_3E, fn_1_7E34);
    Hu3DParticleBlendModeSet(lbl_1_bss_3E, 1);
}

void fn_1_83E4(void)
{
    Hu3DModelKill(lbl_1_bss_3E);
}

void fn_1_8410(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_3A[modelNo], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_85C4(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_3A[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

void fn_1_8BF8(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_3A[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_3A[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_3A[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_3A[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_3A[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_3A[i], fn_1_8618);
        Hu3DParticleBlendModeSet(lbl_1_bss_3A[i], 1);
    }
}

void fn_1_8D64(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_3A[i]);
    }
}

void fn_1_8DBC(s16 modelNo, s16 display)
{
    if (display != 0) {
        Hu3DModelAttrReset(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(lbl_1_bss_36[modelNo], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_8F70(s16 modelNo)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;

    model = &Hu3DData[lbl_1_bss_36[modelNo]];
    particle = model->hookData;
    particle->dataCnt = 0;
}

void fn_1_94E0(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_36[i] = Hu3DParticleCreate(lbl_1_bss_48[1], 128);
        Hu3DModelPosSet(lbl_1_bss_36[i], lbl_1_rodata_10,
            lbl_1_rodata_10, lbl_1_rodata_10);
        Hu3DModelScaleSet(lbl_1_bss_36[i], lbl_1_rodata_40,
            lbl_1_rodata_40, lbl_1_rodata_40);
        Hu3DModelAttrSet(lbl_1_bss_36[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_36[i], 2);
        Hu3DParticleHookSet(lbl_1_bss_36[i], fn_1_8FC4);
        Hu3DParticleBlendModeSet(lbl_1_bss_36[i], 1);
    }
}

void fn_1_964C(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_36[i]);
    }
}

inline void fn_1_72A4(void);
inline void fn_1_7C70(void);
inline void fn_1_8300(void);
inline void fn_1_8BF8(void);
inline void fn_1_94E0(void);

void fn_1_96A4(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_48[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_9F8[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    fn_1_72A4();
    fn_1_7C70();
    fn_1_8300();
    fn_1_8BF8();
    fn_1_94E0();
}

void fn_1_9D14(void)
{
    s16 i;
    s16 j;
    s16 k;
    s16 l;

    for (i = 0; i < 2; i++) {
        Hu3DModelKill(lbl_1_bss_44[i]);
    }
    for (j = 0; j < 2; j++) {
        Hu3DModelKill(lbl_1_bss_40[j]);
    }
    Hu3DModelKill(lbl_1_bss_3E);
    for (k = 0; k < 2; k++) {
        Hu3DModelKill(lbl_1_bss_3A[k]);
    }
    for (l = 0; l < 2; l++) {
        Hu3DModelKill(lbl_1_bss_36[l]);
    }
}
