#include <dolphin/mtx/GeoTypes.h>

#include "datadir_enum.h"
#include "game/memory.h"

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HUSPR_GROUPID;
typedef s16 HUWINID;

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

typedef struct MdResultPlayerWork_s {
    HU3D_MODELID models[3];
    HUSPR_GROUPID group;
    float values[6];
    HUSPR_GROUPID secondGroup;
    s16 state[2];
    HUWINID winId;
} MDRESULT_PLAYER_WORK;

enum {
    HU3D_ATTR_DISPOFF = 1 << 0,
    HUSPR_ATTR_DISPOFF = 0x4,
};

extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_38;
extern s16 lbl_1_bss_48;
extern MDRESULT_PLAYER_WORK lbl_1_bss_66C[4];
extern s16 lbl_1_bss_1278[16];

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
BOOL Hu3DMotionKill(HU3D_MOTIONID motionId);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void HuWinDispOff(HUWINID winId);
void HuWinExKill(HUWINID winId);

void fn_1_17F78(OMOBJ *obj);
void fn_1_181C0(void);
void fn_1_18F08(OMOBJ *obj);
void fn_1_192BC(OMOBJ *obj);
void fn_1_19504(void);
void fn_1_1A570(OMOBJ *obj);
void fn_1_20108(HUSPR_GROUPID groupId, s32 attr);

void fn_1_1AA10(OMOBJ *obj)
{
    s16 i;
    s16 j;
    MDRESULT_PLAYER_WORK *work;

    i = 0;
    work = lbl_1_bss_66C;
    for (; i < 2; i++, work++) {
        HuWinExKill(work->winId);
        for (j = 0; j < 3; j++) {
            Hu3DModelKill(obj->mdlId[j]);
        }
    }
}

void fn_1_1AAA8(OMOBJ *obj)
{
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_17F78(obj);
    } else {
        fn_1_192BC(obj);
    }
}

void fn_1_1AAF8(void)
{
    lbl_1_bss_48 = 0;
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_181C0();
    } else {
        fn_1_19504();
    }
    lbl_1_bss_38->objFunc = fn_1_1AAA8;
}

void fn_1_1AB5C(void)
{
    if (lbl_1_bss_1278[3] == 0) {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 4; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    } else {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 2; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
            fn_1_20108(work->secondGroup, HUSPR_ATTR_DISPOFF);
            HuWinDispOff(work->winId);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_38->objFunc = NULL;
}

void fn_1_1AD68(OMOBJ *obj)
{
    obj->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x4F), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[0] = Hu3DMotionIDGet(obj->mdlId[0]);
    obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x50), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[1] = Hu3DMotionIDGet(obj->mdlId[1]);
    obj->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mdpresult, 0x51), HU_MEMNUM_OVL, HEAP_MODEL));
    obj->mtnId[2] = Hu3DMotionIDGet(obj->mdlId[2]);
    if (lbl_1_bss_1278[3] == 0) {
        fn_1_18F08(obj);
    } else {
        fn_1_1A570(obj);
    }
    Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[1], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_ATTR_DISPOFF);
    if (lbl_1_bss_1278[3] == 0) {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 4; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    } else {
        s16 i = 0;
        MDRESULT_PLAYER_WORK *work;

        work = &lbl_1_bss_66C[0];
        for (; i < 2; i++, work++) {
            Hu3DModelAttrSet(work->models[0], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[1], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(work->models[2], HU3D_ATTR_DISPOFF);
            fn_1_20108(work->group, HUSPR_ATTR_DISPOFF);
            fn_1_20108(work->secondGroup, HUSPR_ATTR_DISPOFF);
            HuWinDispOff(work->winId);
        }
        for (i = 0; i < 4; i++) {
            Hu3DModelAttrSet(lbl_1_bss_C->mdlId[i], HU3D_ATTR_DISPOFF);
        }
        Hu3DModelAttrSet(lbl_1_bss_4->mdlId[0], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_8->mdlId[0], HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_38->objFunc = NULL;
    obj->objFunc = NULL;
}

void fn_1_1B064(OMOBJ *obj)
{
    s16 i;

    if (lbl_1_bss_1278[3] == 0) {
        s16 player;
        s16 model;
        MDRESULT_PLAYER_WORK *work;

        player = 0;
        work = lbl_1_bss_66C;
        for (; player < 4; player++, work++) {
            for (model = 0; model < 3; model++) {
                Hu3DModelKill(obj->mdlId[model]);
            }
        }
    } else {
        s16 player;
        MDRESULT_PLAYER_WORK *work;
        s16 model;

        player = 0;
        work = lbl_1_bss_66C;
        for (; player < 2; player++, work++) {
            HuWinExKill(work->winId);
            for (model = 0; model < 3; model++) {
                Hu3DModelKill(obj->mdlId[model]);
            }
        }
    }
    for (i = 0; i < 3; i++) {
        Hu3DMotionKill(obj->mtnId[i]);
        Hu3DModelKill(obj->mdlId[i]);
    }
}
