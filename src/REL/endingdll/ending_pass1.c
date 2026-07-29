#include "game/hu3d.h"

typedef struct Process_s OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);

struct omObj_s {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    OMOBJ_FUNC objFunc;
    HuVecF trans;
    HuVecF rot;
    HuVecF scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

typedef struct EndingMotionWork {
    s16 state;
    float time;
    float duration;
    HuVecF unk_0C;
    HuVecF unk_18;
    HuVecF unk_24;
    float start;
    float end;
    float unk_38;
    float unk_3C;
} EndingMotionWork;

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_18;
extern EndingMotionWork lbl_1_bss_1834[7];
extern EndingMotionWork lbl_1_bss_1A5C[2];

extern float lbl_1_rodata_C0;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_F8;
extern float lbl_1_rodata_19C;
extern float lbl_1_rodata_1E8;
extern float lbl_1_rodata_1EC;
extern float lbl_1_rodata_200;
extern float lbl_1_rodata_214;
extern float lbl_1_rodata_218;
extern float lbl_1_rodata_21C;
extern float lbl_1_rodata_220;
extern float lbl_1_rodata_224;
extern float lbl_1_rodata_228;
extern float lbl_1_rodata_22C;
extern float lbl_1_rodata_230;
extern char lbl_1_data_CB[];

void omDelObjEx(OMOBJMAN *manager, OMOBJ *object);
void omSetStatBit(OMOBJ *object, u16 bit);
void fn_1_36E0(OMOBJ *object);
float fn_1_DDF8(float start, float end, float time, float duration);

void fn_1_39AC(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF rotation;

    work->time = lbl_1_rodata_C8;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelRotGet(object->mdlId[0], &rotation);
    work->start = rotation.y;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 1;
    object->objFunc = fn_1_36E0;
}

void fn_1_3A7C(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF position;

    work->time = lbl_1_rodata_C8;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelPosGet(object->mdlId[0], &position);
    work->start = position.x;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 0;
    object->objFunc = fn_1_36E0;
}

void fn_1_3B4C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 0x100);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x22001E, HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(0x22001F + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0x40000001);
    Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_21C,
        lbl_1_rodata_C0, lbl_1_rodata_C8);
    Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_214, lbl_1_rodata_C8);
    Hu3DModelScaleSet(object->mdlId[0], lbl_1_rodata_220,
        lbl_1_rodata_220, lbl_1_rodata_220);
    object->objFunc = NULL;
}

void fn_1_3CD4(OMOBJ *object)
{
    s16 motion;

    if (object) {
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_3D5C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 0x100);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x220025, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x220026, HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(0x220027 + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelHookSet(object->mdlId[0], lbl_1_data_CB,
        object->mdlId[1]);
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0x40000001);
    Hu3DModelPosSet(object->mdlId[0], lbl_1_rodata_C0,
        lbl_1_rodata_C0, lbl_1_rodata_C8);
    Hu3DModelRotSet(object->mdlId[0], lbl_1_rodata_C8,
        lbl_1_rodata_218, lbl_1_rodata_C8);
    Hu3DModelScaleSet(object->mdlId[0], lbl_1_rodata_220,
        lbl_1_rodata_220, lbl_1_rodata_220);
    object->objFunc = NULL;
}

void fn_1_3F20(OMOBJ *object)
{
    s16 motion;

    if (object) {
        Hu3DModelHookReset(object->mdlId[0]);
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        Hu3DModelKill(object->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_3FC0(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], lbl_1_rodata_C8);
    model->motShapeWork.speed = lbl_1_rodata_1E8;
    object->work[3] = 1;
}

void fn_1_4058(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], lbl_1_rodata_C8);
    model->motShapeWork.speed = lbl_1_rodata_224;
    object->work[3] = 0;
}

void fn_1_40F0(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1834;
    s16 model;
    float alpha;

    for (model = 0; model < 7; model++, work++) {
        if (work->time < lbl_1_rodata_C8) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        } else if (work->time >= lbl_1_rodata_C8) {
            if (lbl_1_rodata_C8 == work->time) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    lbl_1_rodata_C8);
                Hu3DMotionSet(object->mdlId[model + 2],
                    object->mtnId[model + 2]);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    lbl_1_rodata_C8);
                Hu3DMotionSet(object->mdlId[model + 9],
                    object->mtnId[model + 2]);
            } else if (work->time < lbl_1_rodata_228) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8(lbl_1_rodata_C8, lbl_1_rodata_22C,
                    work->time, lbl_1_rodata_228);
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            } else if (work->time < lbl_1_rodata_1EC) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    lbl_1_rodata_22C);
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    lbl_1_rodata_22C);
            } else {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8(lbl_1_rodata_22C, lbl_1_rodata_C8,
                    work->time - lbl_1_rodata_1EC, lbl_1_rodata_F8);
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            }
        }
        if ((work->time += lbl_1_rodata_19C) > lbl_1_rodata_200) {
            work->time = lbl_1_rodata_230;
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    }
    if (object->work[3] == 0) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    } else if (object->work[3] == 1) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
        }
    }
}
