#include <dolphin/mtx/GeoTypes.h>

#include "datadir_enum.h"
#include "game/memory.h"

typedef Vec HuVecF;

typedef struct MdResultVec2f {
    float x;
    float y;
} MDRESULT_VEC2F;

typedef struct MdResultObject MDBRESULT_OBJECT;
typedef void (*MDRESULT_OBJECT_FUNC)(MDBRESULT_OBJECT *obj);

struct MdResultObject {
    u16 stat;
    s16 objNext;
    s16 prio;
    s16 prev;
    s16 next;
    s16 nextNo;
    s16 grpNo;
    u16 memberNo;
    u32 mode;
    MDRESULT_OBJECT_FUNC objFunc;
    Vec trans;
    Vec rot;
    Vec scale;
    u16 modelCount;
    s16 *mdlId;
    u16 motionCount;
    s16 *mtnId;
    u32 work[4];
    void *data;
};

typedef struct AnimData_s ANIMDATA;

typedef struct MdResultSpriteInfo {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    MDRESULT_VEC2F pos;
    MDRESULT_VEC2F scale;
    float zRot;
} MDRESULT_SPRITE_INFO;

typedef struct MdResultVectorTable {
    Vec values[8];
} MDRESULT_VECTOR_TABLE;

enum {
    MDRESULT_HU3D_ATTR_DISPOFF = 1 << 0,
    MDRESULT_HUSPR_ATTR_DISPOFF = 0x4,
    MDRESULT_OM_STAT_MODELPAUSE = 1 << 8,
};

#define MDRESULT_HU3D_MOTATTR_LOOP 0x40000001

extern void *lbl_1_bss_0;
extern MDBRESULT_OBJECT *lbl_1_bss_30;
extern Vec lbl_1_bss_109C[4];
extern s16 lbl_1_bss_117C[18];
extern s16 lbl_1_bss_11A0[6];
extern ANIMDATA *lbl_1_bss_11AC[39];
extern s16 lbl_1_bss_1278[16];
extern s32 lbl_1_data_C0[39];
extern s16 lbl_1_data_15C[6];
extern MDRESULT_SPRITE_INFO lbl_1_data_168[18];

extern const float lbl_1_rodata_F4;
extern const float lbl_1_rodata_F8;
extern const float lbl_1_rodata_FC;
extern const float lbl_1_rodata_104;
extern const Vec lbl_1_rodata_16C;
extern const Vec lbl_1_rodata_178;
extern const Vec lbl_1_rodata_184;
extern const MDRESULT_VECTOR_TABLE lbl_1_rodata_190;
extern const MDRESULT_VECTOR_TABLE lbl_1_rodata_1F0;
extern const float lbl_1_rodata_250;
extern const float lbl_1_rodata_254;
extern const float lbl_1_rodata_258;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
void Hu3D2Dto3D(Vec *src, s16 cameraBit, Vec *dst);
s16 Hu3DModelCreate(void *data);
void Hu3DModelKill(s16 modelId);
void Hu3DModelPosSetV(s16 modelId, Vec *pos);
void Hu3DModelPosGet(s16 modelId, Vec *pos);
void Hu3DModelRotSet(s16 modelId, float x, float y, float z);
void Hu3DModelRotSetV(s16 modelId, Vec *rot);
void Hu3DModelRotGet(s16 modelId, Vec *rot);
void Hu3DModelScaleSet(s16 modelId, float x, float y, float z);
void Hu3DModelScaleSetV(s16 modelId, Vec *scale);
void Hu3DModelScaleGet(s16 modelId, Vec *scale);
void Hu3DModelAttrSet(s16 modelId, u32 attr);
void Hu3DModelAttrReset(s16 modelId, u32 attr);
void Hu3DModelLayerSet(s16 modelId, s16 layer);
s32 Hu3DMotionKill(s16 motionId);
void Hu3DMotionShiftSet(s16 modelId, s16 motionId, float start,
    float end, u32 attr);
s16 Hu3DMotionIDGet(s16 modelId);
void Hu3DShadowCreate(float fov, float near, float far);
void Hu3DShadowPosSet(Vec *position, Vec *up, Vec *target);
ANIMDATA *HuSprAnimRead(void *data);
s16 HuSprCreate(ANIMDATA *anim, s16 priority, s16 bank);
s16 HuSprGrpCreate(s16 count);
void HuSprGrpMemberSet(s16 group, s16 member, s16 sprite);
void HuSprPosSet(s16 group, s16 member, float x, float y);
void HuSprZRotSet(s16 group, s16 member, float rotation);
void HuSprScaleSet(s16 group, s16 member, float x, float y);
void HuSprExecLayerSet(s16 drawNo, s16 layer);
void omDelObjEx(void *manager, MDBRESULT_OBJECT *obj);
void omSetStatBit(MDBRESULT_OBJECT *obj, u16 stat);

float fn_1_1F8BC(float current, float target, float weight);
void fn_1_1FB50(Vec *current, const Vec *target, float weight);
void fn_1_2001C(s16 modelId, const Vec *first, const Vec *second);
void fn_1_20108(s16 groupId, s32 attr);

void fn_1_2BF0(void)
{
    Vec shadowPos = lbl_1_rodata_16C;
    Vec shadowUp = lbl_1_rodata_178;
    Vec shadowTarget = lbl_1_rodata_184;

    Hu3DShadowCreate(
        lbl_1_rodata_F4, lbl_1_rodata_F8, lbl_1_rodata_FC);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
}

inline void fn_1_2BF0(void);

void fn_1_2CA4(void)
{
}

void fn_1_2CA8(void)
{
    MDRESULT_SPRITE_INFO *desc;
    s16 i;

    for (i = 0; i < 39; i++) {
        lbl_1_bss_11AC[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_C0[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_11A0[i] = HuSprGrpCreate(lbl_1_data_15C[i]);
    }
    for (i = 0, desc = lbl_1_data_168; i < 18; i++, desc++) {
        lbl_1_bss_117C[i] = HuSprCreate(
            lbl_1_bss_11AC[desc->animNo], desc->priority + 6000,
            desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            lbl_1_bss_117C[i]);
        HuSprPosSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_11A0[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 6; i++) {
        fn_1_20108(lbl_1_bss_11A0[i], MDRESULT_HUSPR_ATTR_DISPOFF);
    }
    HuSprExecLayerSet(0x40, 2);
}

inline void fn_1_2CA8(void);

void fn_1_2ED0(void)
{
}

void fn_1_2ED4(s16 index)
{
    MDRESULT_VECTOR_TABLE positions = lbl_1_rodata_190;
    Vec world;

    Hu3D2Dto3D(&positions.values[index + (lbl_1_bss_1278[3] * 4)], 1,
        &world);
    lbl_1_bss_109C[0].x = world.x;
    lbl_1_bss_109C[0].y = world.y;
    lbl_1_bss_109C[0].z = world.z;
}

void fn_1_2F80(s16 index)
{
    MDBRESULT_OBJECT *obj = lbl_1_bss_30;
    MDRESULT_VECTOR_TABLE positions = lbl_1_rodata_1F0;
    Vec world;

    fn_1_2001C(obj->mdlId[0],
        &positions.values[index + (lbl_1_bss_1278[3] * 4)], NULL);
    Hu3DModelRotSet(obj->mdlId[0], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_250,
        lbl_1_rodata_250, lbl_1_rodata_250);
    Hu3DModelPosGet(obj->mdlId[0], &world);
    lbl_1_bss_109C[0].x = world.x;
    lbl_1_bss_109C[0].y = world.y;
    lbl_1_bss_109C[0].z = world.z;
    Hu3DModelAttrReset(obj->mdlId[0], MDRESULT_HU3D_ATTR_DISPOFF);
}

void fn_1_30C4(void)
{
    MDBRESULT_OBJECT *obj = lbl_1_bss_30;

    Hu3DModelAttrSet(obj->mdlId[0], MDRESULT_HU3D_ATTR_DISPOFF);
}

inline void fn_1_30C4(void);

void fn_1_3104(MDBRESULT_OBJECT *obj)
{
    Vec transform;

    Hu3DModelPosGet(obj->mdlId[0], &transform);
    fn_1_1FB50(&transform, &lbl_1_bss_109C[0], lbl_1_rodata_254);
    Hu3DModelPosSetV(obj->mdlId[0], &transform);
    Hu3DModelRotGet(obj->mdlId[0], &transform);
    transform.z = fn_1_1F8BC(
        transform.z, lbl_1_rodata_104, lbl_1_rodata_254);
    Hu3DModelRotSetV(obj->mdlId[0], &transform);
    Hu3DModelScaleGet(obj->mdlId[0], &transform);
    transform.x = transform.y = transform.z = fn_1_1F8BC(
        transform.x, lbl_1_rodata_250, lbl_1_rodata_254);
    Hu3DModelScaleSetV(obj->mdlId[0], &transform);
}

void fn_1_31F8(MDBRESULT_OBJECT *obj)
{
    MDBRESULT_OBJECT *activeObj;

    omSetStatBit(obj, MDRESULT_OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x60), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    Hu3DModelLayerSet(obj->mdlId[0], 3);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
        lbl_1_rodata_104, lbl_1_rodata_104, MDRESULT_HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(obj->mdlId[0], lbl_1_rodata_258,
        lbl_1_rodata_258, lbl_1_rodata_258);
    activeObj = lbl_1_bss_30;
    Hu3DModelAttrSet(activeObj->mdlId[0], MDRESULT_HU3D_ATTR_DISPOFF);
    obj->objFunc = fn_1_3104;
}

void fn_1_3304(MDBRESULT_OBJECT *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_3304(MDBRESULT_OBJECT *obj);
