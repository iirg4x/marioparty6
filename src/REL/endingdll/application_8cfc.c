#include <dolphin/mtx/GeoTypes.h>

typedef Vec HuVecF;
typedef s16 HU3D_MODELID;
typedef s16 HU3D_MOTIONID;
typedef struct Process_s OMOBJMAN;
typedef struct omObj_s OMOBJ;
typedef void (*OMOBJ_FUNC)(OMOBJ *object);

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

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern s16 lbl_1_bss_1A52[2];

void Hu3DLLightKill(HU3D_MODELID modelId, s16 lightId);
void Hu3DMotionKill(HU3D_MOTIONID motionId);
void Hu3DModelKill(HU3D_MODELID modelId);
void Hu3DModelHookReset(HU3D_MODELID modelId);
void omDelObjEx(OMOBJMAN *manager, OMOBJ *object);
void fn_1_10DEC(s16 index, HuVecF *position, s16 mode);

static inline void fn_1_26D4(OMOBJ *object)
{
    s16 model;
    s16 motion;

    if (object) {
        for (model = 0; model < 10; model++) {
            for (motion = 0; motion < 6; motion++) {
                Hu3DMotionKill(object->mtnId[motion + (model * 6)]);
            }
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

static inline void fn_1_3CD4(OMOBJ *object)
{
    s16 motion;

    if (object) {
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

static inline void fn_1_3F20(OMOBJ *object)
{
    s16 motion;

    if (object) {
        Hu3DModelHookReset(object->mdlId[0]);
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        Hu3DModelKill(object->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

void fn_1_8CFC(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    fn_1_10DEC(0, NULL, 2);
    fn_1_10DEC(1, NULL, 2);
    lbl_1_bss_10->objFunc = NULL;

    fn_1_26D4(lbl_1_bss_8);
    lbl_1_bss_8->objFunc = NULL;
    lbl_1_bss_8 = NULL;

    fn_1_3CD4(lbl_1_bss_C);
    lbl_1_bss_C->objFunc = NULL;
    lbl_1_bss_C = NULL;

    fn_1_3F20(lbl_1_bss_10);
    lbl_1_bss_10->objFunc = NULL;
    lbl_1_bss_10 = NULL;
}
