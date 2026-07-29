#include <dolphin/mtx/GeoTypes.h>

#define HU3D_ATTR_DISPOFF (1 << 0)

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;

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

extern HUSPR_GROUP HuSprGrpData[256];
extern HU3D_MODELID lbl_1_bss_1E2C;

void HuSprAttrSet(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);

void fn_1_E0EC(s16 groupId, u32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 member;

    for (member = 0; member < group->sprNum; member++) {
        HuSprAttrSet(groupId, member, (u16)attr);
    }
}

void fn_1_E16C(s16 groupId, u32 attr)
{
    HUSPR_GROUP *group = &HuSprGrpData[groupId];
    s16 member;

    for (member = 0; member < group->sprNum; member++) {
        HuSprAttrReset(groupId, member, (u16)attr);
    }
}

void fn_1_E1EC(s16 display, HuVecF *pos)
{
    if (pos != NULL) {
        Hu3DModelPosSetV(lbl_1_bss_1E2C, pos);
    }
    if (display == 0) {
        Hu3DModelAttrSet(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_1E2C, HU3D_ATTR_DISPOFF);
    }
}
