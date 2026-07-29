#include <dolphin/mtx/GeoTypes.h>

#include "datadir_enum.h"
#include "game/memory.h"

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_TEXSCRID;

typedef struct Process_s OMOBJMAN;
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

typedef struct MdResultCharacterWork_s {
    s32 unk_00;
    s32 unk_04;
    s16 character;
    s16 unk_0A;
} MDRESULT_CHARACTER_WORK;

enum {
    OM_STAT_MODELPAUSE = 1 << 8,
};

#define HU3D_MOTATTR_LOOP 0x40000001

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern MDRESULT_CHARACTER_WORK lbl_1_bss_1248[4];
extern s16 lbl_1_bss_1278[16];
extern HuVecF lbl_1_data_0[16];
extern char lbl_1_data_666[];
extern char lbl_1_data_678[];

extern const float lbl_1_rodata_F8;
extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_260;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_298;
extern const float lbl_1_rodata_2C4;
extern const float lbl_1_rodata_2C8;
extern const float lbl_1_rodata_2CC;
extern const float lbl_1_rodata_2D0;
extern const float lbl_1_rodata_2D4;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID CharModelCreate(s16 character, s16 model);
HU3D_MOTIONID CharMotionCreate(s16 character, unsigned int dataNum);
void CharModelKill(s16 character);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelRotSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
void Hu3DModelShadowSet(HU3D_MODELID modelId);
void Hu3DModelShadowMapSet(HU3D_MODELID modelId);
void Hu3DModelHookSet(HU3D_MODELID modelId, char *name,
    HU3D_MODELID hookModelId);
void Hu3DModelHookReset(HU3D_MODELID modelId);
HU3D_MOTIONID Hu3DJointMotion(HU3D_MODELID modelId, void *data);
BOOL Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
HU3D_TEXSCRID Hu3DTexScrollCreate(HU3D_MODELID modelId, char *name);
void Hu3DTexScrollKill(HU3D_TEXSCRID texScrollId);
void omSetStatBit(OMOBJ *obj, u16 bit);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *obj);

void fn_1_4A9C(OMOBJ *obj);
void fn_1_4BB8(OMOBJ *obj);

void fn_1_4694(OMOBJ *obj)
{
    MDRESULT_CHARACTER_WORK *characterWork;
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    i = 0;
    characterWork = &lbl_1_bss_1248[i];
    for (; i < 4; i++, characterWork++) {
        obj->mdlId[i] = CharModelCreate(characterWork->character, 2);
        obj->mtnId[i] = CharMotionCreate(characterWork->character, 0x930000);
        obj->mtnId[i + 4] = CharMotionCreate(characterWork->character, 0x93000B);
        obj->mtnId[i + 8] = Hu3DJointMotion(obj->mdlId[i],
            HuDataSelHeapReadNum(characterWork->character + 0x980010,
                HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 12] = Hu3DJointMotion(obj->mdlId[i],
            HuDataSelHeapReadNum(characterWork->character + 0x98001B,
                HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 16] = CharMotionCreate(characterWork->character, 0x930022);
        obj->mtnId[i + 20] = CharMotionCreate(characterWork->character, 0x930024);
        obj->mtnId[i + 24] = CharMotionCreate(characterWork->character, 0x930025);
        obj->mtnId[i + 28] = CharMotionCreate(characterWork->character, 0x930029);
        obj->mtnId[i + 32] = CharMotionCreate(characterWork->character, 0x930057);
        obj->mtnId[i + 36] = CharMotionCreate(characterWork->character, 0x930007);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
    }
    if (lbl_1_bss_1278[3] == 0) {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 8]);
        }
    } else {
        for (i = 0; i < 4; i++) {
            Hu3DModelPosSetV(obj->mdlId[i], &lbl_1_data_0[i + 12]);
        }
    }
    obj->objFunc = NULL;
}

void fn_1_49C8(OMOBJ *obj)
{
    s16 i;
    s16 j;

    if (obj) {
        CharModelKill(-1);
        for (i = 0; i < 4; i++) {
            Hu3DMotionKill(obj->mtnId[i + 8]);
            Hu3DMotionKill(obj->mtnId[i + 12]);
            obj->mdlId[i] = -1;
            for (j = 0; j < 8; j++) {
            }
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4A9C(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_298);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_110);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_104, lbl_1_rodata_260,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4B44(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4A9C;
}

void fn_1_4BB8(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_298);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], lbl_1_rodata_110);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            lbl_1_rodata_104, lbl_1_rodata_260,
            HU3D_MOTATTR_LOOP);
    }
}

void fn_1_4C60(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1],
        lbl_1_rodata_104, lbl_1_rodata_F8, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_4BB8;
}

void fn_1_4CD4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x26), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotion(obj->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 0x27) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_104,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_2C4,
        lbl_1_rodata_104, lbl_1_rodata_284);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104,
        lbl_1_rodata_260, lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_2C8,
        lbl_1_rodata_2C8, lbl_1_rodata_2C8);
    obj->objFunc = NULL;
}

void fn_1_4E68(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_4EF0(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x2C), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x2D), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 5; i++) {
        obj->mtnId[i] = Hu3DJointMotion(obj->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_mdpresult, 0x2E) + i,
                HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_666, obj->mdlId[1]);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], lbl_1_rodata_104,
        lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    Hu3DModelPosSet(obj->mdlId[0], lbl_1_rodata_2CC,
        lbl_1_rodata_104, lbl_1_rodata_284);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104, lbl_1_rodata_2D0,
        lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_2C8,
        lbl_1_rodata_2C8, lbl_1_rodata_2C8);
    obj->objFunc = NULL;
}

void fn_1_50C0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
        }
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_5160(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104,
            HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[i]);
    }
    Hu3DModelPosSet(obj->mdlId[1], lbl_1_rodata_104, lbl_1_rodata_2D4,
        lbl_1_rodata_104);
    obj->work[1] = Hu3DTexScrollCreate(obj->mdlId[1], lbl_1_data_678);
    obj->objFunc = NULL;
}

void fn_1_52C4(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DTexScrollKill(obj->work[1]);
        for (i = 0; i < 2; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}
