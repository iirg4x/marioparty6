#include "dolphin.h"

#include "game/audio.h"
#include "game/board/main.h"
#include "game/board/masu.h"
#include "game/board/object.h"
#include "game/board/opening.h"
#include "game/board/pause.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "string.h"

typedef void (*VoidFunc)(void);
typedef void (*MBHook)(void);

enum {
    S03_BOARD_NO = 8,
    S03_WORK_CLEAR_SIZE = 112,
    S03_SAVE_CLEAR_SIZE = 8,
    S03_MOTION_DATA_COUNT = 16,
    S03_HOOK_NAME_SIZE = 16,
    S03_HOOK_COUNT = 10,
    S03_OBJECT_LAYER = 3,
    S03_OBJECT_PRIORITY = 8204,
    S03_MASU_ATTR_MOVE_START = 2,
    S03_MASU_ATTR_HATENA = 1,
};

typedef struct S03Work {
    s16 modelId;
    s16 pathModelId[2];
    s16 unk_06;
    s16 chainModelId;
    s16 sourceModelId;
    s16 markerModelId;
    s16 eventModelId;
    HuVecF chainPos;
    HuVecF chainEndPos;
    s32 state;
    s32 timer;
    s32 substate;
    HuVecF rotation;
    HuVecF scale;
    HuVecF targetPos;
    s32 effectState;
    void *effectWork;
    float effectAngle;
    u32 unk_64;
    u32 unk_68;
    u32 unk_6C;
    u32 unk_70;
} S03Work;

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern S03Work lbl_1_bss_4;
extern u32 *lbl_1_bss_78;
extern BOOL mbSaveNewF;

extern const float lbl_1_rodata_10;
extern const float lbl_1_rodata_14;
extern const float lbl_1_rodata_18;
extern const float lbl_1_rodata_1C;
extern const float lbl_1_rodata_20;

extern HuVecF lbl_1_data_0;
extern HuVecF lbl_1_data_C;
extern HuVecF lbl_1_data_18[2];
extern HuVecF lbl_1_data_30;
extern HuVecF lbl_1_data_3C;
extern float lbl_1_data_48;
extern char lbl_1_data_4C[];
extern char lbl_1_data_52[];
extern char lbl_1_data_5C[];
extern char lbl_1_data_62[];

void mbObjectSetup(s32 boardNo, MBHook init, MBHook close);
void mbLightFuncSet(MBHook setHook, MBHook resetHook);
void mbScrollInit(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));

int _prolog(void);
void _epilog(void);
void fn_1_A0(void);
void fn_1_F4(void);
void fn_1_5EC(void);
void fn_1_5F0(OMOBJ *obj);
void fn_1_634(void);
int fn_1_638(int playerNo, s16 id);
int fn_1_69C(int playerNo, s16 id);
int fn_1_6CC(int playerNo, s16 id);
void fn_1_728(void);
void fn_1_764(void);
void fn_1_768(BOOL enterF);
void fn_1_76C(int playerNo, s16 id);
void fn_1_1450(int playerNo, s16 id);
void fn_1_2670(void);

int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors != 0) {
        (**ctors)();
        ctors++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors != 0) {
        (**dtors)();
        dtors++;
    }
}

void fn_1_A0(void)
{
    GWPartySet(FALSE);
    mbObjectSetup(S03_BOARD_NO, fn_1_F4, fn_1_5EC);
}

void fn_1_F4(void)
{
    S03Work *work = &lbl_1_bss_4;
    int boardNo = MBBoardNoGet();
    int motData[S03_MOTION_DATA_COUNT];
    Mtx matrix;
    s32 modelId;
    s32 hookModelId;
    int i;
    char name[S03_HOOK_NAME_SIZE];

    HuAudSndGrpSetSet(MSM_GRP_SBRD);
    lbl_1_bss_78 = GwSystem.boardWork;
    mbMasuInit(DATANUM(DATA_s03, 0));
    memset(work, 0, S03_WORK_CLEAR_SIZE);
    work->effectState = 0;
    work->effectWork = NULL;

    work->modelId = mbObjCreate(DATANUM(DATA_s03, 2), NULL, FALSE);
    mbObjAttrSet(work->modelId, HU3D_MOTATTR_LOOP);
    mbScrollInit(DATANUM(DATA_s03, 1));
    mbLightFuncSet(fn_1_728, fn_1_764);
    if (mbSaveNewF) {
        memset(lbl_1_bss_78, 0, S03_SAVE_CLEAR_SIZE);
    }

    modelId = mbObjCreate(DATANUM(DATA_s03, 4), NULL, FALSE);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_10);
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    modelId = mbObjCreate(DATANUM(DATA_s03, 5), NULL, FALSE);
    mbObjAttrSet(modelId, (HU3D_MOTATTR_LOOP | HU3D_MOTATTR_SHAPE_LOOP));
    modelId = mbObjCreate(DATANUM(DATA_s03, 6), NULL, FALSE);
    mbObjLayerSet(modelId, S03_OBJECT_LAYER);
    hookModelId = mbObjCreate(DATANUM(DATA_s03, 12), NULL, FALSE);
    mbObjAttrSet(hookModelId, HU3D_MOTATTR_LOOP);
    mbObjLayerSet(hookModelId, S03_OBJECT_LAYER);
    mbObjScaleSet(hookModelId, lbl_1_rodata_14, lbl_1_rodata_14,
        lbl_1_rodata_14);
    for (i = 0; i < S03_HOOK_COUNT; i++) {
        sprintf(name, lbl_1_data_4C, i + 1);
        mbObjHookSet(modelId, name, hookModelId);
    }

    for (i = 0; i < 2; i++) {
        modelId = mbObjCreate(DATANUM(DATA_s03, 7), NULL, TRUE);
        work->pathModelId[i] = modelId;
        mbObjPosSetV(modelId, &lbl_1_data_18[i]);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_18);
    }

    motData[0] = DATANUM(DATA_s03, 14);
    motData[1] = DATANUM(DATA_s03, 15);
    motData[2] = HU_DATANUM_NONE;
    modelId = mbObjCreate(DATANUM(DATA_s03, 13), motData, FALSE);
    work->eventModelId = modelId;
    mbObjDispSet(modelId, FALSE);

    modelId = mbObjCreate(DATANUM(DATA_s03, 11), NULL, FALSE);
    work->chainModelId = modelId;
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(modelId, FALSE);
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), lbl_1_data_52, matrix);
    work->targetPos.x = matrix[0][3];
    work->targetPos.y = matrix[1][3];
    work->targetPos.z = matrix[2][3];

    modelId = mbObjCreate(DATANUM(DATA_s03, 8), NULL, FALSE);
    work->markerModelId = modelId;
    mbObjPosSetV(modelId, &lbl_1_data_0);

    modelId = mbObjCreate(DATANUM(DATA_s03, 9), NULL, FALSE);
    work->sourceModelId = modelId;
    mbObjDispSet(modelId, FALSE);
    Hu3DMotionCalc(mbObjModelIDGet(work->sourceModelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId), lbl_1_data_5C,
        matrix);
    work->chainPos.x = matrix[0][3];
    work->chainPos.y = matrix[1][3];
    work->chainPos.z = matrix[2][3];
    Hu3DModelObjMtxGet(mbObjModelIDGet(work->sourceModelId), lbl_1_data_62,
        matrix);
    work->chainEndPos.x = matrix[0][3];
    work->chainEndPos.y = matrix[1][3];
    work->chainEndPos.z = matrix[2][3];

    work->state = -1;
    work->timer = 0;
    work->substate = 0;
    work->rotation.x = work->rotation.y = work->rotation.z =
        lbl_1_rodata_18;
    work->scale.x = work->scale.y = work->scale.z = lbl_1_rodata_1C;

    fn_1_2670();
    mbev_MasuMoveEndSet(fn_1_69C);
    mbev_MasuMoveStartSet(fn_1_638);
    mbev_MasuHatenaSet(fn_1_6CC);
    mbMapCameraSet(NULL, &lbl_1_data_C, lbl_1_rodata_20);
    mbMapHookSet(fn_1_768);
    mbOpeningInstHookSet(fn_1_634);
    omAddObjEx(mbObjMan, S03_OBJECT_PRIORITY, 0, 0, -1, fn_1_5F0);
    HuDataDirClose(DATANUM(DATA_s03, 0));
    mbOpeningViewSet(&lbl_1_data_30, &lbl_1_data_3C, lbl_1_data_48);
}

void fn_1_5EC(void)
{
}

void fn_1_5F0(OMOBJ *obj)
{
    if (mbExitCheck()) {
        omDelObjEx(mbObjMan, obj);
        return;
    }
}

void fn_1_634(void)
{
}

int fn_1_638(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & S03_MASU_ATTR_MOVE_START) {
        mbPauseDisableSet(TRUE);
        fn_1_1450(playerNo, id);
    }
    return 0;
}

int fn_1_69C(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    return 0;
}

int fn_1_6CC(int playerNo, s16 id)
{
    u32 mAttr = mbMasuMAttrGet(id);

    if (mAttr & S03_MASU_ATTR_HATENA) {
        fn_1_76C(playerNo, id);
    }
    return 0;
}

void fn_1_728(void)
{
    S03Work *work = &lbl_1_bss_4;

    Hu3DModelLightInfoSet(mbObjModelIDGet(work->modelId), TRUE);
}

void fn_1_764(void)
{
}

void fn_1_768(BOOL enterF)
{
}
