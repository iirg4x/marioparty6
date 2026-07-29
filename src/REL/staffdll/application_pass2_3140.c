#include <dolphin/mtx/GeoTypes.h>

#define HU3D_MOTATTR_LOOP 0x40000001
#define HUSPR_ATTR_DISPOFF 0x4

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
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

typedef struct StaffMotionWork {
    s16 state;
    s16 unk_02;
    f32 unk_04;
    f32 unk_08;
    HuVecF position;
    HuVecF unk_18;
    HuVecF unk_24;
    f32 unk_30;
    HuVecF unk_34;
    HuVecF unk_40;
    HuVecF unk_4C;
    HuVecF unk_58;
} STAFF_MOTION_WORK;

extern const f32 lbl_1_rodata_10;
extern const f32 lbl_1_rodata_40;
extern const f32 lbl_1_rodata_B0;

extern char lbl_1_data_998[];

extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern f32 lbl_1_bss_2C;
extern f32 lbl_1_bss_30;
extern STAFF_MOTION_WORK lbl_1_bss_5C[2];
extern HUSPR_GROUPID lbl_1_bss_82A;

void Hu3DModelObjPosGet(HU3D_MODELID modelId, char *objName, HuVecF *pos);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motId,
    float start, float end, u32 attr);
void HuSprAttrSet(HUSPR_GROUPID grpId, s16 memberNo, s32 attr);
void HuSprAttrReset(HUSPR_GROUPID grpId, s16 memberNo, s32 attr);
void HuSprTPLvlSet(HUSPR_GROUPID grpId, s16 memberNo, float tpLvl);

void fn_1_2BD4(OMOBJ *obj);
void fn_1_3140(OMOBJ *obj);
void fn_1_9E30(s32 modelNo, HuVecF *pos, s32 display);

void fn_1_3140(OMOBJ *obj)
{
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    if (obj->work[0] < 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        fn_1_9E30(0, &firstWork->position, 1);
        fn_1_9E30(1, &secondWork->position, 1);
    }
    if (obj->work[0] == 30) {
        Hu3DModelObjPosGet(
            first->mdlId[0], lbl_1_data_998, &firstWork->position);
        Hu3DModelObjPosGet(
            second->mdlId[0], lbl_1_data_998, &secondWork->position);
        lbl_1_bss_C->work[0] = 0;
        lbl_1_bss_C->work[1] = 180;
        lbl_1_bss_C->objFunc = fn_1_2BD4;
    }
    if (obj->work[0] == 60) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[3],
            lbl_1_rodata_10, lbl_1_rodata_B0, 0);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[3],
            lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    } else if (obj->work[0] == 180) {
        Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[4],
            lbl_1_rodata_10, lbl_1_rodata_B0, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[4],
            lbl_1_rodata_10, lbl_1_rodata_B0, HU3D_MOTATTR_LOOP);
        lbl_1_bss_8->objFunc = NULL;
    }
    obj->work[0]++;
}

void fn_1_3378(void)
{
    OMOBJ *obj = lbl_1_bss_8;
    OMOBJ *first = lbl_1_bss_8;
    OMOBJ *second = lbl_1_bss_C;
    STAFF_MOTION_WORK *firstWork = &lbl_1_bss_5C[0];
    STAFF_MOTION_WORK *secondWork = &lbl_1_bss_5C[1];

    Hu3DMotionShiftSet(first->mdlId[0], first->mtnId[2],
        lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    Hu3DMotionShiftSet(second->mdlId[0], second->mtnId[2],
        lbl_1_rodata_10, lbl_1_rodata_B0, 0);
    obj->work[0] = 0;
    obj->objFunc = fn_1_3140;
}

void fn_1_3460(void)
{
    HuSprAttrReset(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    lbl_1_bss_30 = lbl_1_rodata_40;
    lbl_1_bss_2C = lbl_1_rodata_10;
}

void fn_1_34E4(void)
{
    HuSprTPLvlSet(lbl_1_bss_82A, 0, lbl_1_rodata_10);
    HuSprAttrSet(lbl_1_bss_82A, 0, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_30 = lbl_1_rodata_10;
    lbl_1_bss_2C = lbl_1_rodata_10;
}
