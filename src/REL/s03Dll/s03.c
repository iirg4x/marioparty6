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

typedef struct S03Work {
    s16 modelId;
    s16 pathModelId[2];
    u8 reserved_06[2];
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
    u8 reserved_64[0x10];
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
    mbObjectSetup(8, fn_1_F4, fn_1_5EC);
}

void fn_1_F4(void)
{
    S03Work *work = &lbl_1_bss_4;
    int boardNo = MBBoardNoGet();
    int motData[16];
    Mtx matrix;
    s32 modelId;
    s32 hookModelId;
    int i;
    char name[16];

    HuAudSndGrpSetSet(0x1D);
    lbl_1_bss_78 = GwSystem.boardWork;
    mbMasuInit(0xC90000);
    memset(work, 0, 0x70);
    work->effectState = 0;
    work->effectWork = NULL;

    work->modelId = mbObjCreate(0xC90002, NULL, FALSE);
    mbObjAttrSet(work->modelId, 0x40000001);
    mbScrollInit(0xC90001);
    mbLightFuncSet(fn_1_728, fn_1_764);
    if (mbSaveNewF) {
        memset(lbl_1_bss_78, 0, 8);
    }

    modelId = mbObjCreate(0xC90004, NULL, FALSE);
    mbObjMotionSpeedSet(modelId, lbl_1_rodata_10);
    mbObjAttrSet(modelId, 0x40000001);
    modelId = mbObjCreate(0xC90005, NULL, FALSE);
    mbObjAttrSet(modelId, 0x40000041);
    modelId = mbObjCreate(0xC90006, NULL, FALSE);
    mbObjLayerSet(modelId, 3);
    hookModelId = mbObjCreate(0xC9000C, NULL, FALSE);
    mbObjAttrSet(hookModelId, 0x40000001);
    mbObjLayerSet(hookModelId, 3);
    mbObjScaleSet(hookModelId, lbl_1_rodata_14, lbl_1_rodata_14,
        lbl_1_rodata_14);
    for (i = 0; i < 10; i++) {
        sprintf(name, lbl_1_data_4C, i + 1);
        mbObjHookSet(modelId, name, hookModelId);
    }

    for (i = 0; i < 2; i++) {
        modelId = mbObjCreate(0xC90007, NULL, TRUE);
        work->pathModelId[i] = modelId;
        mbObjPosSetV(modelId, &lbl_1_data_18[i]);
        mbObjMotionSpeedSet(modelId, lbl_1_rodata_18);
    }

    motData[0] = 0xC9000E;
    motData[1] = 0xC9000F;
    motData[2] = -1;
    modelId = mbObjCreate(0xC9000D, motData, FALSE);
    work->eventModelId = modelId;
    mbObjDispSet(modelId, FALSE);

    modelId = mbObjCreate(0xC9000B, NULL, FALSE);
    work->chainModelId = modelId;
    mbObjAttrSet(modelId, 0x40000001);
    mbObjDispSet(modelId, FALSE);
    Hu3DMotionCalc(mbObjModelIDGet(modelId));
    Hu3DModelObjMtxGet(mbObjModelIDGet(modelId), lbl_1_data_52, matrix);
    work->targetPos.x = matrix[0][3];
    work->targetPos.y = matrix[1][3];
    work->targetPos.z = matrix[2][3];

    modelId = mbObjCreate(0xC90008, NULL, FALSE);
    work->markerModelId = modelId;
    mbObjPosSetV(modelId, &lbl_1_data_0);

    modelId = mbObjCreate(0xC90009, NULL, FALSE);
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
    omAddObjEx(mbObjMan, 0x200C, 0, 0, -1, fn_1_5F0);
    HuDataDirClose(0xC90000);
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

    if (mAttr & 2) {
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

    if (mAttr & 1) {
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
