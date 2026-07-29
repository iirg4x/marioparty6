#include <stddef.h>

#include <dolphin/mtx/GeoTypes.h>

#define HUSPR_ATTR_DISPOFF (1 << 2)
#define MDRESULT_GROUP_SLOT 0
#define MDRESULT_GROUP_MEMBER_FIRST 0
#define MDRESULT_GROUP_MEMBER_SECOND 1
#define MDRESULT_GROUP_MEMBER_THIRD 2
#define MDRESULT_GROUP_MEMBER_COUNT 3
#define MDRESULT_GROUP_ANIM_FIRST 10
#define MDRESULT_GROUP_ANIM_SECOND 11
#define MDRESULT_GROUP_ANIM_THIRD 12
#define MDRESULT_GROUP_SPRITE_PRIORITY 0
#define MDRESULT_GROUP_SPRITE_BANK 0
#define MDRESULT_GROUP_DRAW_NO 64
#define MDRESULT_BANK_STATE_INDEX 0
#define MDRESULT_OTHER_BANK_STATE_INDEX 1
#define MDRESULT_OTHER_BANK_BASE 10
#define MDRESULT_OTHER_BANK_STEP 5

typedef s16 HUSPRID;
typedef s16 HUSPR_GROUPID;
typedef struct AnimData_s ANIMDATA;

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
};

typedef struct MdResultGroupWork_s {
    HUSPR_GROUPID group;
    HUSPRID sprites[3];
} MDRESULT_GROUP_WORK;

extern ANIMDATA *lbl_1_bss_11AC[39];
extern HUSPR_GROUPID lbl_1_bss_714;
extern s16 lbl_1_bss_1278[16];
extern const float lbl_1_rodata_2B8;
extern const float lbl_1_rodata_594;
extern const float lbl_1_rodata_598;
extern const float lbl_1_rodata_59C;

HUSPRID HuSprCreate(ANIMDATA *anim, s16 priority, s16 bank);
HUSPR_GROUPID HuSprGrpCreate(s16 memberCount);
void HuSprGrpMemberSet(HUSPR_GROUPID groupId, s16 member, HUSPRID spriteId);
void HuSprPosSet(HUSPR_GROUPID groupId, s16 member, float x, float y);
void HuSprBankSet(HUSPR_GROUPID groupId, s16 member, s16 bank);
void HuSprDrawNoSet(HUSPR_GROUPID groupId, s16 member, s32 drawNo);
void fn_1_20108(HUSPR_GROUPID groupId, s32 attr);
void fn_1_20188(HUSPR_GROUPID groupId, s32 attr);

void fn_1_17CF4(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;
    s16 bank = lbl_1_bss_1278[MDRESULT_BANK_STATE_INDEX];
    s16 otherBank =
        (lbl_1_bss_1278[MDRESULT_OTHER_BANK_STATE_INDEX]
            - MDRESULT_OTHER_BANK_BASE) / MDRESULT_OTHER_BANK_STEP;

    fn_1_20188(group[MDRESULT_GROUP_SLOT], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(group[MDRESULT_GROUP_SLOT], MDRESULT_GROUP_MEMBER_SECOND,
        bank);
    HuSprBankSet(group[MDRESULT_GROUP_SLOT], MDRESULT_GROUP_MEMBER_THIRD,
        otherBank);
}

void fn_1_17D94(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;

    fn_1_20108(group[MDRESULT_GROUP_SLOT], HUSPR_ATTR_DISPOFF);
}

void fn_1_17DCC(OMOBJ *obj)
{
    MDRESULT_GROUP_WORK *group = (MDRESULT_GROUP_WORK *)&lbl_1_bss_714;
    MDRESULT_GROUP_WORK *finalGroup;
    s16 i;

    group->group = HuSprGrpCreate(MDRESULT_GROUP_MEMBER_COUNT);
    group->sprites[MDRESULT_GROUP_MEMBER_FIRST] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GROUP_ANIM_FIRST],
        MDRESULT_GROUP_SPRITE_PRIORITY, MDRESULT_GROUP_SPRITE_BANK);
    group->sprites[MDRESULT_GROUP_MEMBER_SECOND] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GROUP_ANIM_SECOND],
        MDRESULT_GROUP_SPRITE_PRIORITY, MDRESULT_GROUP_SPRITE_BANK);
    group->sprites[MDRESULT_GROUP_MEMBER_THIRD] = HuSprCreate(
        lbl_1_bss_11AC[MDRESULT_GROUP_ANIM_THIRD],
        MDRESULT_GROUP_SPRITE_PRIORITY, MDRESULT_GROUP_SPRITE_BANK);
    for (i = 0; i < MDRESULT_GROUP_MEMBER_COUNT; i++) {
        HuSprGrpMemberSet(group->group, i, group->sprites[i]);
    }
    HuSprPosSet(group->group, MDRESULT_GROUP_MEMBER_FIRST,
        lbl_1_rodata_594, lbl_1_rodata_2B8);
    HuSprPosSet(group->group, MDRESULT_GROUP_MEMBER_SECOND,
        lbl_1_rodata_598, lbl_1_rodata_2B8);
    HuSprPosSet(group->group, MDRESULT_GROUP_MEMBER_THIRD,
        lbl_1_rodata_59C, lbl_1_rodata_2B8);
    HuSprDrawNoSet(group->group, MDRESULT_GROUP_MEMBER_FIRST,
        MDRESULT_GROUP_DRAW_NO);
    HuSprDrawNoSet(group->group, MDRESULT_GROUP_MEMBER_SECOND,
        MDRESULT_GROUP_DRAW_NO);
    HuSprDrawNoSet(group->group, MDRESULT_GROUP_MEMBER_THIRD,
        MDRESULT_GROUP_DRAW_NO);
    finalGroup = (MDRESULT_GROUP_WORK *)&lbl_1_bss_714;
    fn_1_20108(finalGroup->group, HUSPR_ATTR_DISPOFF);
    obj->objFunc = NULL;
}

void fn_1_17F60(void)
{
    HUSPR_GROUPID *group = &lbl_1_bss_714;
}
