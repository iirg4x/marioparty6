/* Owns the Hyper Sniper stage models, shadow camera, and introduction and main-stage motions. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

#define M646_STAGE_WORK_BYTES 860
#define M646_INTRO_MODEL_FILE 2
#define M646_INTRO_MOTION_FILE 3
#define M646_MAIN_STAGE_MODEL_FILE 4
#define M646_MAIN_STAGE_MOTION_FILE 5
#define M646_STAGE_MODEL_FILE 6
#define M646_SHADOW_RECEIVER_MODEL_FILE 8
#define M646_SECONDARY_STAGE_MODEL_FILE 9
#define M646_STAGE_FINISH_SE_ID 1990
#define M646_STAGE_INTRO_SE_ID 1991

/* State and shadow camera values used while the stage scene is active. */
typedef struct M646StageWork_s {
    u8 stageWorkBeforeSequence[820]; /* Stage work preceding the motion sequence state. */
    s16 sequenceState;             /* Stage-motion sequence state, 0 through 3. */
    u8 statePadding[2]; /* Unused bytes between the sequence state and camera position. */
    Vec shadowCameraPosition;       /* Shadow camera position in world coordinates. */
    Vec shadowCameraTarget;         /* World point the shadow camera looks toward. */
    Vec shadowCameraUp;             /* Up direction used to orient the shadow camera. */
} M646StageWork;

extern OMOBJ *lbl_1_bss_68;

extern s32 lbl_1_data_140[2];
void fn_1_4ED4(OMOBJ *obj);
void fn_1_8158(OMOBJMAN *objman);
void fn_1_8980(void);
void fn_1_B4BC(s32 effect);
void fn_1_52F0(OMOBJ *obj);
void fn_1_52B4(OMOBJ *obj);

void fn_1_4E28(OMOBJMAN *objman);
void fn_1_4ED0(void);
void fn_1_4ED4(OMOBJ *obj);
void fn_1_51D0(void);
void fn_1_521C(void);
void fn_1_5268(void);
void fn_1_52B4(OMOBJ *obj);
void fn_1_52F0(OMOBJ *obj);

OMOBJ *lbl_1_bss_68;

s32 lbl_1_data_140[2] = {
    DATANUM(DATA_m646, M646_INTRO_MOTION_FILE),
    DATANUM(DATA_m646, M646_MAIN_STAGE_MOTION_FILE)
};
/* Joint labels and numeric values used by the stage model data. */
char lbl_1_data_148[72] =
    "col"
    "\000\000\000\000\000\000\000"
    "post_R"
    "\000\000\000\000"
    "post_L"
    "\000\000\000\000"
    "post_R_C"
    "\000\000"
    "post_L_C"
    "\000\000\000\000\000\000\000\014"
    "\000\000\000\010\000\000\000\010"
    "\000\000\000\004\000\000\000\004";

/* Stage setup: creates the scene object and allocates state for its shadow and motions. */
void fn_1_4E28(OMOBJMAN *objman)
{
    OMOBJ *obj = omAddObjEx(objman, 30, 6, 2, OM_GRP_NONE, fn_1_52B4);
    void *work;
    lbl_1_bss_68 = obj;
    obj->stat |= OM_STAT_MODELPAUSE;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, M646_STAGE_WORK_BYTES, HU_MEMNUM_OVL);
    memset(work, 0, M646_STAGE_WORK_BYTES);
    fn_1_8158(objman);
}

void fn_1_4ED0(void)
{
}

/* Scene startup callback: loads stage models and motions and configures the shadow camera. */
void fn_1_4ED4(OMOBJ *obj)
{
    M646StageWork *work = obj->data;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;

    model = obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m646, M646_STAGE_MODEL_FILE), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(model, 1);
    model = obj->mdlId[5] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_m646, M646_SECONDARY_STAGE_MODEL_FILE), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(model, 6);
    model = obj->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_m646, M646_SHADOW_RECEIVER_MODEL_FILE), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(model, 6);

    work->shadowCameraPosition.x = (0.0f);
    work->shadowCameraPosition.y = (10000.0f);
    work->shadowCameraPosition.z = (0.0f);
    work->shadowCameraUp.x = (0.0f);
    work->shadowCameraUp.y = (0.0f);
    work->shadowCameraUp.z = (1.0f);
    work->shadowCameraTarget.x = (0.0f);
    work->shadowCameraTarget.y = (-100.0f);
    work->shadowCameraTarget.z = (0.0f);
    Hu3DShadowCreate((30.0f), (1.0f), (13000.0f));
    Hu3DShadowPosSet(&work->shadowCameraPosition, &work->shadowCameraUp, &work->shadowCameraTarget);
    Hu3DShadowColSet(50, 50, 50);
    Hu3DShadowSizeSet(192);
    Hu3DShadowTPLvlSet(0.699999988f);
    /* The map is resized again to 240 pixels, replacing the earlier 192-pixel buffer. */
    Hu3DShadowColSet(50, 50, 50);
    Hu3DShadowSizeSet(240);
    Hu3DModelShadowMapSet(model);

    model = obj->mdlId[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m646, M646_INTRO_MODEL_FILE), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[0] =
        Hu3DJointMotion(model, HuDataSelHeapReadNum(lbl_1_data_140[0], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (0.0f));
    Hu3DModelLayerSet(model, 1);
    model = obj->mdlId[3] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_m646, M646_MAIN_STAGE_MODEL_FILE), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[1] =
        Hu3DJointMotion(model, HuDataSelHeapReadNum(lbl_1_data_140[1], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (0.0f));
    Hu3DModelLayerSet(model, 1);
    Hu3DZClearLayerSet(1);
    Hu3DBGColorSet(178, 178, 178);
}

/* Starts the first stage introduction motion when the scene callback begins. */
void fn_1_51D0(void)
{
    HU3D_MODELID stageModel = lbl_1_bss_68->mdlId[2];
    Hu3DMotionSpeedSet(stageModel, (1.0f));
}

/* The scene updater starts the second stage motion when the start phase begins. */
void fn_1_521C(void)
{
    HU3D_MODELID stageModel = lbl_1_bss_68->mdlId[3];
    Hu3DMotionSpeedSet(stageModel, (1.0f));
}

/* Pauses the main-stage motion when this stage-control helper is invoked. */
void fn_1_5268(void)
{
    HU3D_MODELID stageModel = lbl_1_bss_68->mdlId[3];
    Hu3DMotionSpeedSet(stageModel, (0.0f));
}

/* The scene startup callback loads its models and installs the animation updater. */
void fn_1_52B4(OMOBJ *obj)
{
    fn_1_4ED4(obj);
    obj->objFunc = fn_1_52F0;
}

/* Per-frame scene callback starts the introduction and main motions and plays their stage cues. */
void fn_1_52F0(OMOBJ *obj)
{
    M646StageWork *work = obj->data;
    switch (work->sequenceState) {
        case 0:
            fn_1_51D0();
            work->sequenceState++;
            break;
        case 1: {
            HU3D_MODELID model = obj->mdlId[2];
            if ((30.0f) == Hu3DMotionTimeGet(model)) {
                fn_1_B4BC(M646_STAGE_INTRO_SE_ID);
            }
            if (Hu3DMotionEndCheck(model)) {
                work->sequenceState++;
            }
            break;
        }
        case 2:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                fn_1_8980();
                fn_1_521C();
                work->sequenceState++;
            }
            break;
        case 3:
            if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
                fn_1_B4BC(M646_STAGE_FINISH_SE_ID);
                work->sequenceState++;
            }
            break;
    }
}
