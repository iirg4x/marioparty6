#include <dolphin/mtx/GeoTypes.h>

#include "game/memory.h"

#define HU3D_ATTR_DISPOFF (1 << 0)

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
extern float lbl_1_rodata_1A0;
extern float lbl_1_rodata_22C;
extern HuVecF lbl_1_rodata_294;
extern HuVecF lbl_1_rodata_2A0;
extern char lbl_1_data_110[];

void omSetStatBit(OMOBJ *object, u16 bit);
void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float blend, u32 attr);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelHookSet(HU3D_MODELID parent, char *name,
    HU3D_MODELID child);
void Hu3DMotionSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId);
void Hu3DMotionTimeSet(HU3D_MODELID modelId, float time);
void HuPrcVSleep(void);
void fn_1_F1B8(s16 display, HuVecF *position);
void fn_1_F23C(s16 count);
void fn_1_F068(s16 index, s16 display, HuVecF *position);
void fn_1_F11C(s16 index, s16 count);

void fn_1_B244(void)
{
    OMOBJ *object;
    s16 index;
    HuVecF position0;
    HuVecF position1;

    omSetStatBit(lbl_1_bss_14, 0x100);
    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 1] = Hu3DModelCreate(HuDataSelHeapReadNum(
            0x220001 + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 1] = Hu3DMotionIDGet(
            object->mdlId[index + 1]);
        Hu3DModelAttrSet(object->mdlId[index + 1], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(object->mdlId[index + 1],
            object->mtnId[index + 1], lbl_1_rodata_C8,
            lbl_1_rodata_C8, 0x40000001);
    }

    position0 = lbl_1_rodata_294;
    position1 = lbl_1_rodata_2A0;
    fn_1_F1B8(1, &position0);
    fn_1_F23C(0);
    fn_1_F068(1, 1, NULL);
    fn_1_F11C(1, 2);

    object = lbl_1_bss_14;
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x220030, HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        0x220033, HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(0x220031 + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelScaleSet(object->mdlId[1], lbl_1_rodata_22C,
        lbl_1_rodata_22C, lbl_1_rodata_22C);
    Hu3DModelHookSet(object->mdlId[0], lbl_1_data_110,
        object->mdlId[1]);
    Hu3DMotionSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[0]);
    Hu3DMotionSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[0]);
    HuPrcVSleep();
    Hu3DMotionTimeSet(lbl_1_bss_14->mdlId[0], lbl_1_rodata_1A0);
    Hu3DMotionTimeSet(lbl_1_bss_4->mdlId[1], lbl_1_rodata_1A0);
}
