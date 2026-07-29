#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;

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
extern OMOBJ *lbl_1_bss_8;
extern s16 lbl_1_bss_26;
extern s16 lbl_1_bss_19F4[10];
extern EndingMotionWork lbl_1_bss_1ADC[10];

extern float lbl_1_rodata_78;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_110;
extern float lbl_1_rodata_19C;
extern float lbl_1_rodata_1E0;
extern float lbl_1_rodata_1E4;
extern float lbl_1_rodata_1E8;

int rand8(void);
int HuAudFXPlay(int soundId);
void fn_1_193C(OMOBJ *object);
void fn_1_1C0C(s16 index, s16 motion, float time);

void omSetStatBit(OMOBJ *object, u16 bit);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *object);

void HuPrcSleep(s32 time);
void HuPrcVSleep(void);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);

HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);

void fn_1_1E3C(s16 motion, float time)
{
    s16 index;

    for (index = 0; index < 10; index++) {
        fn_1_1C0C(index, motion, time);
    }
}

void fn_1_1E90(s16 model, s16 motion, float blend, u32 attr)
{
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
        lbl_1_bss_8->mtnId[motion + (model * 6)], lbl_1_rodata_C8,
        blend, attr);
}

void fn_1_1F20(s16 motion, float blend, u32 attr)
{
    s16 model;

    for (model = 0; model < 10; model++) {
        Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
            lbl_1_bss_8->mtnId[motion + (model * 6)], lbl_1_rodata_C8,
            blend, attr);
    }
}

void fn_1_1FC4(void)
{
    s16 model;

    HuPrcSleep(2);
    for (model = 0; model < 10; model++) {
        Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[model], lbl_1_rodata_1E0);
    }
}

void fn_1_2034(s16 motion, float blend, u32 attr, s16 delay)
{
    s16 frame;
    s16 model;

    for (model = 0; model < 10; model++) {
        lbl_1_bss_19F4[model] = (rand8() % delay) + 1;
    }
    if (motion == 4 && lbl_1_bss_26 == 0) {
        HuAudFXPlay(0x254);
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (model = 0; model < 10; model++) {
            if (lbl_1_bss_19F4[model] == 0) {
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[motion + (model * 6)],
                    lbl_1_rodata_C8, blend, attr);
            }
            lbl_1_bss_19F4[model]--;
            if (lbl_1_bss_19F4[model] <= -10) {
                lbl_1_bss_19F4[model] = -10;
            }
        }
    }
}

void fn_1_2208(s16 motion, float blend, u32 attr, s16 unused, s16 delay)
{
    s16 frame;
    s16 model;

    for (model = 0; model < 10; model++) {
        lbl_1_bss_19F4[model] = (rand8() % delay) + 1;
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (model = 0; model < 10; model++) {
            if (lbl_1_bss_19F4[model] == 0) {
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[motion + (model * 6)],
                    lbl_1_rodata_C8, blend, attr);
                Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_rodata_1E0);
            }
            lbl_1_bss_19F4[model]--;
            if (lbl_1_bss_19F4[model] <= -10) {
                lbl_1_bss_19F4[model] = -10;
            }
        }
    }
}

void fn_1_23D8(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;

    omSetStatBit(object, 0x100);
    for (model = 0; model < 10; model++, work++) {
        object->mdlId[model] = Hu3DModelCreate(HuDataSelHeapReadNum(
            0x220035 + model, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[model * 6] = Hu3DJointMotion(object->mdlId[model],
            HuDataSelHeapReadNum(0x22004A + model, HU_MEMNUM_OVL,
                HEAP_MODEL));
        object->mtnId[(model * 6) + 1] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(0x220055 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 2] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(0x22003F + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 3] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(0x220060 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 4] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(0x22006B + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 5] = Hu3DJointMotion(
            object->mdlId[model], HuDataSelHeapReadNum(0x220076 + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelScaleSet(object->mdlId[model], lbl_1_rodata_110,
            lbl_1_rodata_110, lbl_1_rodata_110);
        Hu3DMotionShiftSet(object->mdlId[model], object->mtnId[model * 6],
            lbl_1_rodata_C8, lbl_1_rodata_C8, 0x40000001);
        work->time = lbl_1_rodata_1E4;
        work->duration = lbl_1_rodata_78;
        work->unk_38 = lbl_1_rodata_19C;
    }
    object->objFunc = fn_1_193C;
}

void fn_1_26D4(OMOBJ *object)
{
    s16 model;
    s16 motion;

    if (object) {
        for (model = 0; model < 10; model++) {
            for (motion = 0; motion < 6; motion++) {
                Hu3DMotionKill(object->mtnId[motion + (model * 6)]);
            }
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_2790(HuVecF *dest, float x, float y, float z)
{
    dest->x = x;
    dest->y = y;
    dest->z = z;
}

float fn_1_27A0(float start, float middle, float end, float time)
{
    float inverse = lbl_1_rodata_19C - time;

    return end * (time * time)
        + (start * (inverse * inverse)
        + lbl_1_rodata_1E8 * (middle * (inverse * time)));
}

void fn_1_27FC(HuVecF *dest, HuVecF *start, HuVecF *middle,
    HuVecF *end, float time)
{
    dest->x = fn_1_27A0(start->x, middle->x, end->x, time);
    dest->y = fn_1_27A0(start->y, middle->y, end->y, time);
    dest->z = fn_1_27A0(start->z, middle->z, end->z, time);
}
