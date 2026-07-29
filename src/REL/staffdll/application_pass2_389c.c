#include <dolphin/mtx/GeoTypes.h>

#define HU_MEMNUM_OVL 0x10000000
#define HU3D_MOTATTR_LOOP 0x40000001
#define OM_STAT_MODELPAUSE (1 << 8)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_TEXSCRID;

typedef enum HeapID_s {
    HEAP_HEAP,
    HEAP_SOUND,
    HEAP_MODEL,
    HEAP_DVD,
    HEAP_SPACE,
    HEAP_MAX
} HEAPID;

typedef struct omObj_s OMOBJ;
typedef struct Process OMOBJMAN;
typedef void (*OMOBJ_FUNC)(OMOBJ *obj);

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
    Vec trans;
    Vec rot;
    Vec scale;
    u16 mdlcnt;
    HU3D_MODELID *mdlId;
    u16 mtncnt;
    HU3D_MOTIONID *mtnId;
    u32 work[4];
    void *data;
};

#define Hu3DModelCreateData(dataNum) \
    (Hu3DModelCreate( \
        HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HEAP_MODEL)))
#define Hu3DJointMotionData(model, dataNum) \
    (Hu3DJointMotion((model), \
        HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HEAP_MODEL)))

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_44;
extern const f32 lbl_1_rodata_19C;
extern const f32 lbl_1_rodata_1A0;

extern char lbl_1_data_9AA[];
extern char lbl_1_data_9AF[];

extern OMOBJMAN *lbl_1_bss_0;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float scaleX, float scaleY, float scaleZ);
void Hu3DModelHookSet(
    HU3D_MODELID modelId, char *objName, HU3D_MODELID hookMdlId);
void Hu3DModelHookReset(HU3D_MODELID modelId);
BOOL Hu3DMotionKill(HU3D_MOTIONID motId);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motId,
    float start, float end, u32 attr);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
HU3D_TEXSCRID Hu3DTexScrollCreate(HU3D_MODELID modelId, char *bmpName);
void Hu3DTexScrollPosMoveSet(
    HU3D_TEXSCRID texScrId, float posX, float posY, float posZ);
void omDelObjEx(OMOBJMAN *objMan, OMOBJ *obj);
void omSetStatBit(OMOBJ *obj, u16 bit);

void fn_1_3568(OMOBJ *obj);

void fn_1_389C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(0xCD0000 + i);
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
    obj->mdlId[0] = Hu3DModelCreateData(0xCD001C);
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 0xCD001D + i);
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
    obj->mdlId[0] = Hu3DModelCreateData(0xCD0022);
    obj->mdlId[1] = Hu3DModelCreateData(0xCD0023);
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotionData(obj->mdlId[0], 0xCD0024 + i);
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
