#include <dolphin/mtx/GeoTypes.h>

#include "datadir_enum.h"
#include "game/memory.h"

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;

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

typedef struct MdResultVectorPair_s {
    HuVecF values[2];
} MDRESULT_VECTOR_PAIR;

typedef struct MdResultEmitterWork_s {
    s16 active;
    float timer;
    float scale;
    void *data;
} MDRESULT_EMITTER_WORK;

typedef struct MdResultStateWork_s {
    s16 state;
    float time;
    float delay;
    s16 score;
} MDRESULT_STATE_WORK;

typedef struct MdResultMoveWork_s {
    s16 state;
    float time;
    float duration;
    HuVecF current;
    HuVecF middle;
    HuVecF target;
    float values[4];
} MDRESULT_MOVE_WORK;

enum {
    OM_STAT_MODELPAUSE = 1 << 8,
    HU3D_ATTR_DISPOFF = 1 << 0,
};

#define HU3D_MOTATTR_LOOP 0x40000001

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_1C;
extern OMOBJ *lbl_1_bss_24;
extern MDRESULT_EMITTER_WORK lbl_1_bss_81C[9];
extern MDRESULT_STATE_WORK lbl_1_bss_8AC[4];
extern MDRESULT_MOVE_WORK lbl_1_bss_8EC[7];
extern s16 lbl_1_bss_1278[16];
extern char lbl_1_data_67D[];
extern char lbl_1_data_682[];

extern const float lbl_1_rodata_F4;
extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_258;
extern const float lbl_1_rodata_2C0;
extern const float lbl_1_rodata_31C;
extern const float lbl_1_rodata_360;
extern const float lbl_1_rodata_380;
extern const float lbl_1_rodata_3B8;
extern const float lbl_1_rodata_3CC;
extern const MDRESULT_VECTOR_PAIR lbl_1_rodata_3D0;
extern const float lbl_1_rodata_3E8;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelPosSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelPosGet(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelScaleSet(HU3D_MODELID modelId, float x, float y, float z);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelAttrReset(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
BOOL Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void omSetStatBit(OMOBJ *obj, u16 bit);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *obj);
int HuAudFXPlay(int seId);
int rand8(void);
void OSReport(const char *message, ...);

void fn_1_1F868(HuVecF *vec, float x, float y, float z);
void fn_1_1F948(HuVecF *result, const HuVecF *start,
    const HuVecF *middle, const HuVecF *end, float time);
float fn_1_1FC94(float start, float end, float time, float duration);
void fn_1_25FF4(s16 index);
void fn_1_26070(s16 index, s16 parManId, HuVecF *velocity,
    float accelX, u8 *color);

void fn_1_A85C(OMOBJ *obj)
{
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];
    HuVecF position;

    fn_1_1F948(&position, &work->current, &work->middle, &work->target,
        fn_1_1FC94(lbl_1_rodata_104, lbl_1_rodata_110, work->time,
            work->duration));
    Hu3DModelPosSetV(obj->mdlId[obj->work[3]], &position);
    if ((work->time += lbl_1_rodata_110) > work->duration) {
        Hu3DModelAttrSet(obj->mdlId[obj->work[3]], HU3D_ATTR_DISPOFF);
        obj->objFunc = NULL;
        fn_1_25FF4((s16)(obj->work[3] + 4));
    }
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &position);
    fn_1_26070((s16)(obj->work[3] + 4), -1, &position,
        lbl_1_rodata_360, NULL);
}

void fn_1_A984(void)
{
    OMOBJ *obj = lbl_1_bss_1C;
    MDRESULT_MOVE_WORK *work = &lbl_1_bss_8EC[obj->work[3]];

    work->state = 0;
    work->time = lbl_1_rodata_104;
    work->duration = lbl_1_rodata_380;
    Hu3DModelPosGet(obj->mdlId[obj->work[3]], &work->current);
    fn_1_1F868(&work->middle, lbl_1_rodata_104, lbl_1_rodata_104,
        lbl_1_rodata_3CC);
    fn_1_1F868(&work->target, lbl_1_rodata_104, lbl_1_rodata_2C0,
        lbl_1_rodata_31C);
    HuAudFXPlay(0x496);
    obj->objFunc = fn_1_A85C;
}

void fn_1_AA7C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x3A) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i], lbl_1_rodata_104,
            lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i], lbl_1_rodata_3B8,
            lbl_1_rodata_3B8, lbl_1_rodata_3B8);
    }
    for (i = 0; i < 4; i++) {
        obj->mdlId[i + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x3D), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 3] = Hu3DMotionIDGet(obj->mdlId[i + 3]);
        Hu3DModelLayerSet(obj->mdlId[i + 3], 1);
        Hu3DMotionShiftSet(obj->mdlId[i + 3], obj->mtnId[i + 3],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i + 3], HU3D_ATTR_DISPOFF);
        Hu3DModelScaleSet(obj->mdlId[i + 3], lbl_1_rodata_258,
            lbl_1_rodata_258, lbl_1_rodata_258);
    }
    obj->objFunc = NULL;
}

void fn_1_AD04(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 7; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_AD94(s16 player, s16 step)
{
    OMOBJ *obj = lbl_1_bss_24;
    MDRESULT_VECTOR_PAIR positions;
    s16 phase;
    s16 model;

    positions = lbl_1_rodata_3D0;
    step += 2;
    phase = step / 10;
    model = step % 10;
    if (phase >= 1) {
        Hu3DModelPosSet(obj->mdlId[(player * 11) + 10],
            positions.values[player].x - lbl_1_rodata_3E8,
            positions.values[player].y, positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + 10],
            HU3D_ATTR_DISPOFF);
        Hu3DModelPosSet(obj->mdlId[(player * 11) + model],
            lbl_1_rodata_3E8 + positions.values[player].x,
            positions.values[player].y, positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + model],
            HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelPosSet(obj->mdlId[(player * 11) + model],
            positions.values[player].x, positions.values[player].y,
            positions.values[player].z);
        Hu3DModelAttrReset(obj->mdlId[(player * 11) + model],
            HU3D_ATTR_DISPOFF);
    }
}

void fn_1_AFF4(void)
{
    OMOBJ *obj = lbl_1_bss_24;
    s16 i;

    for (i = 0; i < 22; i++) {
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
}

void fn_1_B05C(OMOBJ *obj)
{
    s16 i;
    s16 j;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        for (j = 0; j < 11; j++) {
            if (j == 10) {
                obj->mdlId[(i * 11) + j] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_mdpresult, 0x41), HU_MEMNUM_OVL, HEAP_MODEL));
            } else {
                obj->mdlId[(i * 11) + j] = Hu3DModelCreate(HuDataSelHeapReadNum(
                    DATANUM(DATA_mdpresult, 0x40) + j, HU_MEMNUM_OVL, HEAP_MODEL));
            }
            Hu3DModelAttrSet(obj->mdlId[(i * 11) + j], HU3D_ATTR_DISPOFF);
        }
    }
    obj->objFunc = NULL;
}

void fn_1_B178(OMOBJ *obj)
{
    s16 j;
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            for (j = 0; j < 11; j++) {
                Hu3DModelKill(obj->mdlId[j + (i * 11)]);
            }
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_B220(void)
{
    s16 shuffled[9];
    s16 values[9];
    s16 i;
    s16 pick;

    for (i = 0; i < 9; i++) {
        values[i] = i;
    }
    for (i = 0; i < 9; i++) {
        pick = rand8() % (9 - i);
        shuffled[i] = values[pick];
        values[pick] = values[8 - i];
    }
    for (i = 0; i < 4; i++) {
        lbl_1_bss_8AC[i].score = shuffled[i];
        OSReport(lbl_1_data_67D, lbl_1_bss_8AC[i].score);
    }
    OSReport(lbl_1_data_682);
    if (lbl_1_bss_1278[3] == 1
        && lbl_1_bss_8AC[0].score + lbl_1_bss_8AC[1].score
            == lbl_1_bss_8AC[2].score + lbl_1_bss_8AC[3].score) {
        if (rand8() % 2 == 0) {
            lbl_1_bss_8AC[0].score = 7;
            lbl_1_bss_8AC[1].score = 8;
            lbl_1_bss_8AC[2].score = 5;
            lbl_1_bss_8AC[3].score = 4;
        } else {
            lbl_1_bss_8AC[0].score = 4;
            lbl_1_bss_8AC[1].score = 6;
            lbl_1_bss_8AC[2].score = 5;
            lbl_1_bss_8AC[3].score = 8;
        }
    }
}

void fn_1_B454(OMOBJ *obj, s16 index, HuVecF *pos)
{
    index--;
    lbl_1_bss_81C[index].active = 1;
    lbl_1_bss_81C[index].timer = lbl_1_rodata_104;
    lbl_1_bss_81C[index].scale = lbl_1_rodata_F4;
    Hu3DModelPosSetV(obj->mdlId[index + 4], pos);
}
