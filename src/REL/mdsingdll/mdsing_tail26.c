#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/types.h"
#include "game/memory.h"

typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
typedef struct AnimData_s ANIMDATA;

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

typedef struct MdsingSpriteDesc {
    s16 groupNo;
    s16 memberNo;
    s16 animNo;
    s16 priority;
    s16 bank;
    HuVec2f pos;
    HuVec2f scale;
    float zRot;
} MDSING_SPRITE_DESC;

#define HUSPR_ATTR_DISPOFF 0x4

extern u32 lbl_1_data_32C[25];
extern s16 lbl_1_data_390[9];
extern MDSING_SPRITE_DESC lbl_1_data_3A4[29];
extern HUSPRID lbl_1_bss_1258[29];
extern HUSPR_GROUPID lbl_1_bss_1292[9];
extern ANIMDATA *lbl_1_bss_12A4[25];
extern HUSPR_GROUP HuSprGrpData[256];

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
ANIMDATA *HuSprAnimRead(void *data);
HUSPR_GROUPID HuSprGrpCreate(s16 memberMax);
HUSPRID HuSprCreate(ANIMDATA *anim, s16 priority, s16 bank);
void HuSprGrpMemberSet(
    HUSPR_GROUPID groupId, s16 memberNo, HUSPRID spriteId);
void HuSprPosSet(
    HUSPR_GROUPID groupId, s16 memberNo, float x, float y);
void HuSprScaleSet(HUSPR_GROUPID groupId, s16 memberNo,
    float x, float y);
void HuSprZRotSet(
    HUSPR_GROUPID groupId, s16 memberNo, float angle);
void HuSprAttrSet(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);

void fn_1_47F0(void)
{
}

void fn_1_47F4(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_47F4(HUSPR_GROUPID groupId, s32 attr);

void fn_1_4874(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_4874(HUSPR_GROUPID groupId, s32 attr);

void fn_1_48F4(void)
{
    MDSING_SPRITE_DESC *desc;
    s16 i;

    for (i = 0; i < 25; i++) {
        lbl_1_bss_12A4[i] = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_32C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 9; i++) {
        lbl_1_bss_1292[i] = HuSprGrpCreate(lbl_1_data_390[i]);
    }
    for (i = 0, desc = lbl_1_data_3A4; i < 29; i++, desc++) {
        lbl_1_bss_1258[i] = HuSprCreate(
            lbl_1_bss_12A4[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            lbl_1_bss_1258[i]);
        HuSprPosSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_1292[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 9; i++) {
        fn_1_47F4(lbl_1_bss_1292[i], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_48F4(void);

void fn_1_4B40(void)
{
}
