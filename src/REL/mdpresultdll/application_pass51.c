#include <dolphin/mtx/GeoTypes.h>

#include "datadir_enum.h"
#include "game/memory.h"

typedef Vec HuVecF;

typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef s16 HU3D_TEXSCRID;

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

typedef struct MdResultCameraWork_s MDRESULT_CAMERA_WORK;
typedef void (*MDRESULT_CAMERA_CALLBACK)(OMOBJ *obj, MDRESULT_CAMERA_WORK *camera);

typedef struct MdResultParticleWork_s {
    HuVecF position;
    HuVecF rotation;
    HuVecF scale;
    float phase;
    float verticalOffset;
    float speed;
    HuVecF target;
    float stateTime;
} MDRESULT_PARTICLE_WORK;

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

struct MdResultCameraWork_s {
    OMOBJ *obj;
    HuVecF center;
    HuVecF targetCenter;
    HuVecF rot;
    HuVecF targetRot;
    float zoom;
    float targetZoom;
    MDRESULT_CAMERA_CALLBACK callback;
    s16 unk_40;
    s16 mode;
    float unk_44;
};

enum {
    HU3D_ATTR_DISPOFF = 1 << 0,
};

#define HU3D_MOTATTR_LOOP 0x40000001

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_14;
extern OMOBJ *lbl_1_bss_28;
extern float lbl_1_bss_44;
extern MDRESULT_EMITTER_WORK lbl_1_bss_81C[9];
extern MDRESULT_STATE_WORK lbl_1_bss_8AC[4];
extern MDRESULT_PARTICLE_WORK lbl_1_bss_F9C[4];
extern MDRESULT_CAMERA_WORK lbl_1_bss_12BC;
extern s16 lbl_1_bss_1278[16];

extern const float lbl_1_rodata_104;
extern const float lbl_1_rodata_110;
extern const float lbl_1_rodata_258;
extern const float lbl_1_rodata_29C;
extern const float lbl_1_rodata_2B8;
extern const float lbl_1_rodata_404;
extern const float lbl_1_rodata_408;
extern const float lbl_1_rodata_40C;
extern const float lbl_1_rodata_410;
extern const float lbl_1_rodata_414;

void *HuDataSelHeapReadNum(int dataNum, s32 num, HEAPID heap);
HU3D_MODELID Hu3DModelCreate(void *data);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelPosSetV(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelPosGet(HU3D_MODELID modelId, HuVecF *position);
void Hu3DModelAttrSet(HU3D_MODELID modelId, u32 attr);
void Hu3DModelLayerSet(HU3D_MODELID modelId, s16 layer);
BOOL Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DMotionShiftSet(HU3D_MODELID modelId, HU3D_MOTIONID motionId,
    float start, float end, u32 attr);
HU3D_MOTIONID Hu3DMotionIDGet(HU3D_MODELID modelId);
void Hu3DMotionTimeSet(HU3D_MODELID modelId, float time);
void Hu3DMotionSpeedSet(HU3D_MODELID modelId, float speed);
void Hu3DTexScrollPosMoveSet(HU3D_TEXSCRID texScrollId,
    float x, float y, float z);
void HuPrcSleep(s32 time);
void HuPrcVSleep(void);
void omSetStatBit(OMOBJ *obj, u16 bit);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *obj);

void fn_1_B8E8(OMOBJ *obj);
void fn_1_C414(void);
void fn_1_25D0C(float value);
void fn_1_26EAC(float value);
void fn_1_26F74(void);

s32 fn_1_C9A0(void)
{
    s16 bestScore = 0;
    s32 result = 0;
    s32 i;
    MDRESULT_STATE_WORK *work;

    for (;;) {
        HuPrcVSleep();
        i = 0;
        work = lbl_1_bss_8AC;
        for (; (s16)i < 4; i++, work++) {
            if (work->state != 0 && work->state != 6) {
                break;
            }
        }
        if ((s16)i == 4) {
            break;
        }
    }
    HuPrcSleep(60);
    if (lbl_1_bss_1278[3] == 1) {
        fn_1_C414();
        if (lbl_1_bss_8AC[0].score + lbl_1_bss_8AC[1].score
            > lbl_1_bss_8AC[2].score + lbl_1_bss_8AC[3].score) {
            result = 0;
        } else {
            result = 2;
        }
    } else {
        i = 0;
        work = lbl_1_bss_8AC;
        for (; (s16)i < 4; i++, work++) {
            if (work->state == 6 && bestScore < work->score) {
                bestScore = work->score;
                result = i;
            }
        }
    }
    return result;
}

void fn_1_CAEC(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, 0x100);
    for (i = 0; i < 4; i++) {
        obj->mdlId[i] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x3F), HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            lbl_1_rodata_104, lbl_1_rodata_104, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
    }
    for (i = 0; i < 9; i++) {
        obj->mdlId[i + 4] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_mdpresult, 0x41) + i, HU_MEMNUM_OVL, HEAP_MODEL));
        obj->mtnId[i + 4] = Hu3DMotionIDGet(obj->mdlId[i + 4]);
        Hu3DModelLayerSet(obj->mdlId[i + 4], 1);
        Hu3DMotionSpeedSet(obj->mdlId[i + 4], lbl_1_rodata_104);
        Hu3DMotionTimeSet(obj->mdlId[i + 4], lbl_1_rodata_258);
        Hu3DModelAttrSet(obj->mdlId[i + 4], HU3D_ATTR_DISPOFF);
    }
    fn_1_B8E8(obj);
    obj->objFunc = NULL;
}

void fn_1_CD04(OMOBJ *obj)
{
    s16 i;
    s16 j;

    if (obj) {
        for (i = 0; i < 9; i++) {
            if (lbl_1_bss_81C[i].data) {
                HuMemDirectFree(lbl_1_bss_81C[i].data);
            }
            lbl_1_bss_81C[i].data = NULL;
        }
        for (j = 0; j < 13; j++) {
            Hu3DMotionKill(obj->mtnId[j]);
            Hu3DModelKill(obj->mdlId[j]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_CE0C(OMOBJ *obj)
{
    obj->objFunc = NULL;
}

void fn_1_CE18(OMOBJ *obj)
{
    if (obj) {
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

void fn_1_CE60(void)
{
    Hu3DModelAttrSet(lbl_1_bss_14->mdlId[1], HU3D_ATTR_DISPOFF);
    fn_1_26F74();
}

void fn_1_CE9C(void)
{
    OMOBJ *obj;
    HuVecF pos;
    s16 i;
    s16 j;
    MDRESULT_PARTICLE_WORK *work;
    MDRESULT_CAMERA_WORK *camera;

    obj = lbl_1_bss_10;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_DISPOFF);
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);
    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_408, lbl_1_rodata_104);

    obj = lbl_1_bss_14;
    obj->work[0] = 1;
    for (i = 2; i < 5; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }
    for (i = 0; i < 3; i++) {
        Hu3DModelPosGet(obj->mdlId[i + 8], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i + 8], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i + 8], &pos);
    }

    obj = lbl_1_bss_28;
    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &pos);
        pos.y -= lbl_1_rodata_2B8;
        if (pos.y < lbl_1_rodata_404) {
            Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
            pos.y = lbl_1_rodata_404;
        }
        Hu3DModelPosSetV(obj->mdlId[i], &pos);
    }

    obj = lbl_1_bss_4;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    obj = lbl_1_bss_8;
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    pos.y -= lbl_1_rodata_2B8;
    if (pos.y < lbl_1_rodata_404) {
        pos.y = lbl_1_rodata_404;
    }
    Hu3DModelPosSetV(obj->mdlId[0], &pos);

    fn_1_26EAC(lbl_1_rodata_40C);
    lbl_1_bss_44 = lbl_1_rodata_40C;
    for (j = 0; j < 4; j++) {
        work = &lbl_1_bss_F9C[j];
        work->verticalOffset -= lbl_1_rodata_110;
        if (work->verticalOffset < lbl_1_rodata_29C) {
            work->verticalOffset = lbl_1_rodata_29C;
        }
    }
    fn_1_25D0C(lbl_1_rodata_410);
    camera = &lbl_1_bss_12BC;
    camera->mode = 6;
}

void fn_1_D30C(float value)
{
    OMOBJ *obj = lbl_1_bss_10;
    MDRESULT_CAMERA_WORK *camera;
    float weight = lbl_1_rodata_110 - value;

    fn_1_26EAC(lbl_1_rodata_40C * weight);
    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_408 * weight, lbl_1_rodata_104);
    lbl_1_bss_44 = lbl_1_rodata_40C * weight;
    fn_1_25D0C(lbl_1_rodata_410 * weight);
    camera = &lbl_1_bss_12BC;
    camera->mode = lbl_1_rodata_414 * weight;
}

void fn_1_D40C(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DTexScrollPosMoveSet(obj->work[1], lbl_1_rodata_104,
        lbl_1_rodata_104, lbl_1_rodata_104);
    fn_1_26EAC(lbl_1_rodata_104);
    fn_1_25D0C(lbl_1_rodata_104);
}
