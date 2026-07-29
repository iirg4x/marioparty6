#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_ANIMID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
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

typedef struct HuVec2f_s {
    float x;
    float y;
} HuVec2f;

typedef struct HuSprGroup_s {
    s16 sprNum;
    HuVec2f pos;
    float zRot;
    HuVec2f scale;
    HuVec2f center;
    HUSPRID *sprId;
    Mtx mtx;
    s16 work[4];
} HUSPR_GROUP;

typedef struct MdsingModelEntry {
    HU3D_MODELID modelId;
    HU3D_ANIMID animId[4];
    s16 unk_A;
    Vec pos;
    Vec rot;
    Vec scale;
    s16 unk_30;
    u8 unk_32;
    u8 unk_33;
    u8 unk_34;
    u8 unk_35;
    u8 unk_36;
    u8 unk_37;
} MDSING_MODEL_ENTRY;

#define HU3D_ATTR_DISPOFF (1 << 0)
#define HUSPR_ATTR_DISPOFF 0x4
#define PAD_BUTTON_LEFT 0x1
#define PAD_BUTTON_RIGHT 0x2

extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_24;
extern float lbl_1_bss_C94[3];
extern MDSING_MODEL_ENTRY lbl_1_bss_E74[16];
extern HUSPR_GROUPID lbl_1_bss_1292[9];
extern s16 lbl_1_bss_1340[];
extern Vec lbl_1_data_7A8[];
extern s16 lbl_1_data_AC8[3];
extern u8 HuPadDStkRep[4];
extern HUSPR_GROUP HuSprGrpData[256];

extern const Vec lbl_1_rodata_1E8;
extern const float lbl_1_rodata_5C;
extern const float lbl_1_rodata_60;
extern const float lbl_1_rodata_64;
extern const float lbl_1_rodata_84;
extern const float lbl_1_rodata_B0;
extern const float lbl_1_rodata_12C;
extern const float lbl_1_rodata_148;
extern const float lbl_1_rodata_17C;
extern const float lbl_1_rodata_1C8;
extern const float lbl_1_rodata_1E4;
extern const float lbl_1_rodata_1F4;
extern const float lbl_1_rodata_284;
extern const float lbl_1_rodata_288;

s32 HuAudFXPlay(s32 soundId);
void Hu3D3Dto2D(Vec *pos3D, s16 cameraNo, Vec *pos2D);
void Hu3DModelPosGet(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelPosSetV(HU3D_MODELID modelId, Vec *pos);
void Hu3DModelRotGet(HU3D_MODELID modelId, Vec *rot);
void Hu3DModelRotSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelRotSetV(HU3D_MODELID modelId, Vec *rot);
void Hu3DModelScaleGet(HU3D_MODELID modelId, Vec *scale);
void Hu3DModelScaleSet(
    HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelScaleSetV(HU3D_MODELID modelId, Vec *scale);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layerNo);
void HuSprGrpPosSet(HUSPR_GROUPID groupId, float x, float y);
void HuSprGrpTPLvlSet(HUSPR_GROUPID groupId, float level);
void HuSprPosSet(HUSPR_GROUPID groupId, s16 memberNo, float x, float y);
void HuSprScaleSet(
    HUSPR_GROUPID groupId, s16 memberNo, float x, float y);
void HuSprAttrSet(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);

static inline float blend_value(float current, float target)
{
    if (current == target) {
        return target;
    }
    return (target + (current * lbl_1_rodata_1F4)) /
        lbl_1_rodata_B0;
}

static inline void blend_vector(Vec *current, const Vec *target)
{
    current->x = blend_value(current->x, target->x);
    current->y = blend_value(current->y, target->y);
    current->z = blend_value(current->z, target->z);
}

static inline void update_selected_model(s16 modelNo)
{
    Vec value;
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosGet(entry->modelId, &value);
    blend_vector(&value, &lbl_1_data_7A8[modelNo + 6]);
    Hu3DModelPosSetV(entry->modelId, &value);
    if (entry->unk_30++ > 30) {
        entry->unk_30 = 35;
        Hu3DModelRotGet(entry->modelId, &value);
        value.y += lbl_1_rodata_1E4;
        if (value.y >= lbl_1_rodata_84) {
            value.y -= lbl_1_rodata_84;
        }
        Hu3DModelRotSetV(entry->modelId, &value);
    }
    Hu3DModelScaleGet(entry->modelId, &value);
    value.y = value.x =
        blend_value(value.x, lbl_1_rodata_284);
    Hu3DModelScaleSetV(entry->modelId, &value);
    Hu3DModelLayerSet(entry->modelId, 2);
}

static inline void reset_model(s16 modelNo)
{
    MDSING_MODEL_ENTRY *entry = &lbl_1_bss_E74[modelNo + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_7A8[modelNo + 3]);
    Hu3DModelRotSet(entry->modelId,
        lbl_1_rodata_64, lbl_1_rodata_64, lbl_1_rodata_64);
    Hu3DModelScaleSet(entry->modelId,
        lbl_1_rodata_17C, lbl_1_rodata_17C, lbl_1_rodata_5C);
    entry->unk_30 = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

static inline void sprite_attr_set(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

static inline void show_member(
    s16 memberNo, Vec *pos3D, float xOffset, float yOffset)
{
    Vec pos2D = lbl_1_rodata_1E8;

    if (pos3D) {
        Hu3D3Dto2D(pos3D, 1, &pos2D);
    }
    HuSprPosSet(lbl_1_bss_1292[2], memberNo,
        pos2D.x + xOffset, pos2D.y + yOffset);
    HuSprScaleSet(lbl_1_bss_1292[2], memberNo,
        lbl_1_rodata_60, lbl_1_rodata_60);
    HuSprAttrReset(
        lbl_1_bss_1292[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_C94[memberNo] = lbl_1_rodata_60;
    lbl_1_data_AC8[memberNo] = 1;
}

void fn_1_EA7C(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340[1]--;
            if (lbl_1_bss_1340[1] < 0) {
                lbl_1_bss_1340[1] += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            memberNo = lbl_1_bss_1340[1];
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
            sprite_attr_set(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[0] = lbl_1_rodata_60;
            if (lbl_1_data_AC8[0] == 1) {
                HuAudFXPlay(0);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            OMOBJ *workObj;
            s16 memberNo;

            lbl_1_bss_1340[1]++;
            if (lbl_1_bss_1340[1] >= 3) {
                lbl_1_bss_1340[1] -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            memberNo = lbl_1_bss_1340[1];
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(
                lbl_1_bss_1292[1], lbl_1_rodata_1C8, lbl_1_rodata_12C);
            HuSprGrpTPLvlSet(lbl_1_bss_1292[1], lbl_1_rodata_64);
            sprite_attr_set(lbl_1_bss_1292[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_1292[1], memberNo, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_C94[1] = lbl_1_rodata_60;
            if (lbl_1_data_AC8[1] == 1) {
                HuAudFXPlay(0);
            }
        }
    }
    for (i = 0; i < 3; i++) {
        if (i == lbl_1_bss_1340[1]) {
            update_selected_model(i);
        } else {
            reset_model(i);
        }
    }
}

void fn_1_F15C(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 3; i++) {
        lbl_1_bss_E74[i + 11].unk_30 = 0;
    }
    show_member(0, &lbl_1_data_7A8[0],
        lbl_1_rodata_288, lbl_1_rodata_64);
    show_member(1, &lbl_1_data_7A8[0],
        lbl_1_rodata_148, lbl_1_rodata_64);
    obj->work[0] = 10;
    obj->objFunc = fn_1_EA7C;
    lbl_1_bss_24->mtnId[0] = 1;
}
