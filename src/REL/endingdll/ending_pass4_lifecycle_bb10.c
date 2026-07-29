#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;

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

extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_14;
extern float lbl_1_rodata_C8;
extern float lbl_1_rodata_1E0;
extern HuVecF lbl_1_rodata_2B8;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelHookReset(HU3D_MODELID modelId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DModelShadowMapSet(HU3D_MODELID modelId);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelShadowSet(HU3D_MODELID modelId);
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
void HuPrcVSleep(void);
void fn_1_F1B8(s16 display, HuVecF *position);

void fn_1_BB10(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 1]);
        Hu3DModelKill(object->mdlId[model + 1]);
    }
    object = lbl_1_bss_14;
    Hu3DModelHookReset(object->mdlId[0]);
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model]);
    }
    Hu3DModelKill(object->mdlId[0]);
    Hu3DModelKill(object->mdlId[1]);
}

void fn_1_BBEC(void)
{
    OMOBJ *object;
    s16 index;

    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            0x220003 + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 3] = Hu3DMotionIDGet(
            object->mdlId[index + 3]);
        Hu3DMotionShiftSet(object->mdlId[index + 3],
            object->mtnId[index + 3], lbl_1_rodata_C8,
            lbl_1_rodata_C8, 0x40000001);
    }
    Hu3DModelShadowMapSet(object->mdlId[3]);
    {
        HuVecF position = lbl_1_rodata_2B8;

        fn_1_F1B8(1, &position);
    }

    object = lbl_1_bss_14;
    object->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x22002D, HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index + 2] = Hu3DJointMotion(object->mdlId[2],
            HuDataSelHeapReadNum(0x22002E + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelPosSet(object->mdlId[2], lbl_1_rodata_C8,
        lbl_1_rodata_C8, lbl_1_rodata_C8);
    Hu3DMotionShiftSet(object->mdlId[2], object->mtnId[3],
        lbl_1_rodata_C8, lbl_1_rodata_C8, 0);
    Hu3DModelShadowSet(object->mdlId[2]);
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[4], lbl_1_rodata_1E0);
}
