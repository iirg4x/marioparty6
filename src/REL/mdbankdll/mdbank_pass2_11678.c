#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
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

extern const float lbl_1_rodata_268;
extern const float lbl_1_rodata_274;
extern HUSPR_GROUP HuSprGrpData[256];

void HuSprAttrSet(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID groupId, s16 memberNo, s32 attr);

static inline float fn_1_1161C(
    float start, float control, float end, float weight)
{
    float inverse = lbl_1_rodata_268 - weight;

    return (weight * weight * end)
        + ((inverse * inverse * start)
            + (lbl_1_rodata_274 * (control * (inverse * weight))));
}

void fn_1_11678(HuVecF *out, const HuVecF *start,
    const HuVecF *control, const HuVecF *end, float weight)
{
    out->x = fn_1_1161C(start->x, control->x, end->x, weight);
    out->y = fn_1_1161C(start->y, control->y, end->y, weight);
    out->z = fn_1_1161C(start->z, control->z, end->z, weight);
}

void fn_1_11880(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

void fn_1_11900(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}
