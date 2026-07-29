#include <dolphin/mtx/GeoTypes.h>

#define HU3D_CAM0 (1 << 0)
#define HUSPR_ATTR_DISPOFF (1 << 2)
#define HUSPR_GROUP_MAX 256
#define MDRESULT_DIGIT_BLANK_BANK 10
#define MDRESULT_TENS_MEMBER_OFFSET 1
#define MDRESULT_ONES_MEMBER_OFFSET 2

typedef Vec HuVecF;

typedef struct HuVec2f_s {
    float x;
    float y;
} HuVec2f;

typedef s16 HU3D_MODELID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;

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

extern HUSPR_GROUP HuSprGrpData[HUSPR_GROUP_MAX];
extern const HuVecF lbl_1_rodata_EA8;

void Hu3D2Dto3D(HuVecF *src, s16 cameraBit, HuVecF *dst);
void Hu3DModelPosSet(HU3D_MODELID modelId, float posX, float posY, float posZ);
void HuSprAttrSet(HUSPR_GROUPID groupId, s16 member, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID groupId, s16 member, s32 attr);
void HuSprBankSet(HUSPR_GROUPID groupId, s16 member, s16 bank);

void fn_1_2001C(HU3D_MODELID modelId, const HuVecF *first,
    const HuVecF *second)
{
    HuVecF screen = lbl_1_rodata_EA8;
    HuVecF world;

    if (first) {
        screen.x += first->x;
        screen.y += first->y;
        screen.z += first->z;
    }
    if (second) {
        screen.x += second->x;
        screen.y += second->y;
        screen.z += second->z;
    }
    Hu3D2Dto3D(&screen, HU3D_CAM0, &world);
    Hu3DModelPosSet(modelId, world.x, world.y, world.z);
}

void fn_1_20108(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrSet(groupId, i, (u16)attr);
    }
}

void fn_1_20188(HUSPR_GROUPID groupId, s32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 i;

    for (i = 0; i < group->sprNum; i++) {
        HuSprAttrReset(groupId, i, (u16)attr);
    }
}

void fn_1_20208(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprBankSet(groupId, member, MDRESULT_DIGIT_BLANK_BANK);
    }
    digit = (value - (digit * 100)) / 10;
        HuSprBankSet(groupId, member + MDRESULT_TENS_MEMBER_OFFSET, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + MDRESULT_TENS_MEMBER_OFFSET,
            HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + MDRESULT_ONES_MEMBER_OFFSET, digit);
}

void fn_1_2035C(HUSPR_GROUPID groupId, s32 member, s16 value)
{
    s16 digit;

    digit = value / 100;
    HuSprBankSet(groupId, member, digit);
    if (digit == 0) {
        HuSprAttrSet(groupId, member, HUSPR_ATTR_DISPOFF);
    }
    digit = (value - (digit * 100)) / 10;
        HuSprBankSet(groupId, member + MDRESULT_TENS_MEMBER_OFFSET, digit);
    if (digit == 0 && value / 100 == 0) {
        HuSprAttrSet(groupId, member + MDRESULT_TENS_MEMBER_OFFSET,
            HUSPR_ATTR_DISPOFF);
    }
    digit = value % 10;
    HuSprBankSet(groupId, member + MDRESULT_ONES_MEMBER_OFFSET, digit);
}
