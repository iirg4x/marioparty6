#include <dolphin/mtx/GeoTypes.h>

#define HU_MEMNUM_OVL 0x10000000
#define HU3D_MOTATTR_LOOP 0x40000001
#define HU3D_ATTR_DISPOFF (1 << 0)
#define OM_STAT_MODELPAUSE (1 << 8)

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_LIGHTID;

typedef enum HeapID_s {
    HEAP_HEAP,
    HEAP_SOUND,
    HEAP_MODEL,
    HEAP_DVD,
    HEAP_SPACE,
    HEAP_MAX
} HEAPID;

typedef struct Process OMOBJMAN;
typedef struct omObj_s OMOBJ;
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

typedef struct StaffModelResource {
    s32 modelData;
    s32 motionData;
} STAFF_MODEL_RESOURCE;

#define Hu3DModelCreateData(dataNum) \
    (Hu3DModelCreate( \
        HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HEAP_MODEL)))
#define Hu3DJointMotionData(model, dataNum) \
    (Hu3DJointMotion((model), \
        HuDataSelHeapReadNum((dataNum), HU_MEMNUM_OVL, HEAP_MODEL)))

extern const f32 lbl_1_rodata_10;

extern STAFF_MODEL_RESOURCE lbl_1_data_0[13];

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern HU3D_LIGHTID lbl_1_bss_82C[2];

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelHookReset(HU3D_MODELID modelId);
BOOL Hu3DMotionKill(HU3D_MOTIONID motId);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motId,
    float start, float end, u32 attr);
void Hu3DGLightKill(HU3D_LIGHTID lightId);
void Hu3DCameraKill(u32 cameraBit);
void omDelObjEx(OMOBJMAN *objMan, OMOBJ *obj);
void omSetStatBit(OMOBJ *obj, u16 bit);
void omOvlReturnEx(s16 hisOfs, s16 unlinkF);
u8 WipeCheck(void);
void HuAudFadeOut(s32 speed);
void fn_1_9D14(void);

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
