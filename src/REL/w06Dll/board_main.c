/* Board setup and event logic for Clockwork Castle. */
#include "math.h"
#include "datadir_enum.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "messdir_enum.h"
#include "msm_se.h"
void mbWipeSpecialFadeInCreate(int type, int durationFrames);
void mbWipeSpecialFadeOutCreate(int type, int durationFrames);
#include "REL/w06Dll/module_context.h"

#define W06_MASU_ATTR_BIT_0_MASK 0x00000001
#define W06_MASU_ATTR_BIT_1_MASK 0x00000002
#define W06_MASU_ATTR_BIT_2_MASK 0x00000004
#define W06_MASU_ATTR_BIT_3_MASK 0x00000008
#define W06_MASU_ATTR_BIT_4_MASK 0x00000010
#define W06_MASU_ATTR_BIT_5_MASK 0x00000020
#define W06_MASU_ATTR_MASK_00110001 0x00110001
#define W06_MASU_ATTR_MASK_00110004 0x00110004
#define W06_MASU_ATTR_MASK_00110007 0x00110007
#define W06_MASU_ATTR_MASK_00110011 0x00110011
#define W06_MASU_ATTR_MASK_0011000C 0x0011000C
#define W06_MASU_ATTR_MASK_00110014 0x00110014
#define W06_MASU_ATTR_MASK_00110016 0x00110016
#define W06_MASU_ATTR_MASK_0011000D 0x0011000D
#define W06_MASU_ATTR_MASK_0011001C 0x0011001C
#define W06_MASU_ATTR_MASK_0011001F 0x0011001F
#define W06_MASU_ATTR_MASK_00110022 0x00110022
#define W06_MASU_ATTR_MASK_0011002B 0x0011002B
#define W06_MASU_ATTR_MASK_00110026 0x00110026
#define W06_MASU_ATTR_MASK_0011002E 0x0011002E
#define W06_MASU_ATTR_MASK_00110030 0x00110030
#define W06_MASU_ATTR_MASK_00110027 0x00110027
#define W06_MASU_ATTR_MASK_00E1002A 0x00E1002A
#define W06_MASU_ATTR_ALL_MASK 0xFFFFFFFF
#define W06_MASU_ATTR_BIT_31_MASK 0x80000000
#define W06_MASU_ATTR_BIT_30_MASK 0x40000000
#define W06_MASU_ATTR_BIT_29_MASK 0x20000000
#define W06_MASU_ATTR_BIT_28_MASK 0x10000000
#define W06_MASU_ATTR_BIT_27_MASK 0x08000000
#define W06_MASU_ATTR_BIT_26_MASK 0x04000000
#define W06_MASU_ATTR_BIT_25_MASK 0x02000000
#define W06_MASU_ATTR_BIT_24_MASK 0x01000000
#define W06_MASU_ATTR_BIT_23_MASK 0x00800000
#define W06_MASU_ATTR_BIT_22_MASK 0x00400000
#define W06_MASU_ATTR_BIT_21_MASK 0x00200000
#define W06_MASU_ATTR_BIT_20_MASK 0x00100000
#define W06_BRANCH_MATTR_BIT_16_MASK (1 << 16)
#define W06_BRANCH_MATTR_BIT_13_MASK 0x00002000
#define W06_BRANCH_MATTR_LOW_SIX_BITS_MASK 0x0000003F
#define W06_SNPC_EXCLUDED_MASU_ATTR_MASK 0x00001000
#define W06_NPC_MASU_ATTR_MASK 0x00000100
#define W06_CAPSULE_MACHINE_EFFECT_SOUND_ID 1525
#define W06_BOARD_DATA_W06_44 14745644
#define W06_BOARD_DATA_W06N_42 14811178
#define W06_BOARD_DATA_W06N_44 14811180
#define W06_BOARD_DATA_W06N_23 14811159
#define W06_BOARD_DATA_W06N_25 14811161
#define W06_BOARD_DATA_W06_23 14745623
#define W06_BOARD_DATA_W06_24 14745624
#define W06_BOARD_DATA_W06_25 14745625
#define W06_BOARD_DATA_W06_7 14745607
#define W06_BOARD_DATA_W06N_7 14811143
#define W06_BOARD_DATA_W06_9 14745609
#define W06_BOARD_DATA_W06N_9 14811145
#define W06_BOARD_CURSOR_DATA_ID 327681
#define W06_BOARD_COIN_DATA_ID 327684
#define W06_BRANCH_MATTR_ROW_0_MASK 0x00080000
#define W06_BRANCH_MATTR_ROW_1_MASK 0x00040000
#define W06_BRANCH_MATTR_ROW_2_MASK 0x00020000
#define W06_BRANCH_MATTR_ROW_3_MASK 0x00008000
#define W06_SPACE_ATTR_SPECIAL_MASK 0xFFF00000
#define W06_HOOK_SHAPE_ATTR_FLAGS 0x40000040
#define W06_HELP_MESSAGE_ID 2490380
#define W06_REVERSE_MOTION_ATTR_FLAGS 0x40000004
#define W06_SPACE_ATTR_BRANCH_MASK 0x00010000
#define W06_BOARD_OBJECT_ID_1_OFFSET 68
#define W06_BOARD_OBJECT_ID_2_OFFSET 72
#define W06_BOARD_OBJECT_ID_3_OFFSET 76
#define W06_BOARD_OBJECT_ID_4_OFFSET 80
#define W06_BOARD_OBJECT_ID_5_OFFSET 84
#define W06_BOARD_OBJECT_ID_6_OFFSET 88
#define W06_BOARD_OBJECT_ID_7_OFFSET 92
#define W06_BOARD_MODEL_OBJECT_IDS_OFFSET 148
#define MSM_SE_W06_TILE_REVEAL 1526
#define MSM_SE_W06_WHEEL_SPIN 1524
#define MSM_SE_W06_WHEEL_STOP 1533
#define MSM_SE_W06_GUIDE_MOTION 1535
#define MSM_SE_W06_TIME_CHANGE 1538
#define MSM_SE_W06_CAPSULE_DROP 1530
#define MSM_SE_W06_TRANSPORT_MOTION 1531
#define MSM_SE_W06_TRANSPORT_LIFT 1532
#define MSM_SE_W06_GUIDE_MOTION_ALT 1534
#define MSM_SE_W06_CAPSULE_REWARD_VOICE 579
#define W06_RANDOM_U16_MASK 0x0000FFFF

OMOBJ *lbl_1_data_0[2] = {NULL, NULL};
s32 lbl_1_data_8 = -1;
Point3d lbl_1_data_C[6] = {
    {-3169.0f, 57.0f, 3710.0f},
    {0.0f, 57.0f, 4535.0f},
    {3169.0f, 57.0f, 3710.0f},
    {-2793.0f, 699.0f, -2360.0f},
    {0.0f, 913.0f, -856.0f},
    {2793.0f, 699.0f, -2360.0f}
};
Point3d lbl_1_data_54[4] = {
    {2491.0f, 317.0f, 458.0f},
    {2700.0f, 317.0f, 788.0f},
    {4010.0f, 706.0f, -378.0f},
    {4160.0f, 706.0f, -116.0f}
};
u32 lbl_1_data_84[40] = {
    W06_MASU_ATTR_BIT_0_MASK, W06_MASU_ATTR_BIT_1_MASK, W06_MASU_ATTR_BIT_2_MASK,
    W06_MASU_ATTR_BIT_3_MASK, W06_MASU_ATTR_BIT_4_MASK, W06_MASU_ATTR_BIT_5_MASK,
    W06_MASU_ATTR_MASK_00110001, W06_MASU_ATTR_MASK_00110004,
    W06_MASU_ATTR_MASK_00110007, W06_MASU_ATTR_MASK_00110011,
    W06_MASU_ATTR_MASK_0011000C, W06_MASU_ATTR_MASK_00110014,
    W06_MASU_ATTR_MASK_00110016, W06_MASU_ATTR_MASK_0011000D,
    W06_MASU_ATTR_ALL_MASK, 0,
    0, 0, 0, 0, 0, 0, W06_MASU_ATTR_MASK_0011001C, W06_MASU_ATTR_MASK_0011001F,
    W06_MASU_ATTR_MASK_00110022, W06_MASU_ATTR_MASK_0011002B,
    W06_MASU_ATTR_MASK_00110026, W06_MASU_ATTR_MASK_0011002E,
    W06_MASU_ATTR_MASK_00110030, W06_MASU_ATTR_MASK_00110027,
    W06_MASU_ATTR_ALL_MASK, 0,
    0, 0, 0, 0, 0, 0, W06_MASU_ATTR_MASK_00E1002A, W06_MASU_ATTR_ALL_MASK
};
int lbl_1_data_124[4] = {W06_BOARD_DATA_W06_44, -1, W06_BOARD_DATA_W06N_42, -1};
int lbl_1_data_134[2] = {W06_BOARD_DATA_W06N_44, -1};
u32 lbl_1_data_13C[12] = {
    W06_MASU_ATTR_BIT_31_MASK, W06_MASU_ATTR_BIT_30_MASK, W06_MASU_ATTR_BIT_29_MASK,
    W06_MASU_ATTR_BIT_28_MASK, W06_MASU_ATTR_BIT_27_MASK, W06_MASU_ATTR_BIT_26_MASK,
    W06_MASU_ATTR_BIT_25_MASK, W06_MASU_ATTR_BIT_24_MASK, W06_MASU_ATTR_BIT_23_MASK,
    W06_MASU_ATTR_BIT_22_MASK, W06_MASU_ATTR_BIT_21_MASK, W06_MASU_ATTR_BIT_20_MASK
};
Point3d lbl_1_data_16C = {-29.0f, 0.0f, 0.0f};
Point3d lbl_1_data_178 = {0.0f, 139.6999969482422f, 205.39999389648438f};
f32 lbl_1_data_184 = 16200.0f;
Point3d lbl_1_data_188 = {-78.0f, 0.0f, 0.0f};
Point3d lbl_1_data_194 = {0.0f, -106.5199966430664f, 835.760009765625f};
f32 lbl_1_data_1A0 = 20474.0f;
#include "game/gamework.h"
#include "game/object.h"

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
void fn_1_A0(void);

/* Runs module constructors, then initializes Clockwork Castle at module startup. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

extern const VoidFunc _dtors[];

void fn_1_F4(void);
void fn_1_99C(OMOBJ *obj);
void mbObjectSetup(s32 boardNo, void (*init)(void), void (*close)(OMOBJ *));

/* Runs the module's registered destructors as the board module unloads. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Enables party mode and registers Clockwork Castle's board setup and close callbacks. */
void fn_1_A0(void)
{
    GwSystem.partyF = TRUE;
    mbObjectSetup(5, fn_1_F4, fn_1_99C);
}

#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/player.h"
#include "game/board/opening.h"
#include "game/board/branch.h"

typedef struct W06BoardObjects {
    struct {
        s32 objectId;
        s32 dataId;
    } displayObjects[6];
    s32 cleanupObjectIds[4];
    s32 objectIds[10];
    s32 spriteObjectIds[3];
    s32 remainingObjectIds[4];
    s32 modelObjectIds[4];
    s32 trailingValues[6];
} W06BoardObjects;

extern W06BoardObjects lbl_1_bss_404;
typedef struct W06SaveState {
    u8 state[2];
    u8 objectStates[6];
    u8 branchRows[2];
} W06SaveState;

extern W06SaveState *lbl_1_bss_4B0;
extern s16 lbl_1_bss_4B4;
extern OMOBJ *lbl_1_bss_0;
extern BOOL mbSaveNewF;
extern OMOBJMAN *mbObjMan;
extern int lbl_1_data_124[4];
extern int lbl_1_data_134[2];

extern Point3d lbl_1_data_54[4];
extern Point3d lbl_1_data_16C;
extern Point3d lbl_1_data_178;
extern f32 lbl_1_data_184;
extern Point3d lbl_1_data_194;
extern f32 lbl_1_data_1A0;

void fn_1_B584(void);
void fn_1_1CDC(void);
void fn_1_318C(void);
void fn_1_4A40(void);
void fn_1_63C4(void);
void fn_1_6FA4(void);
void fn_1_9474(void);
void fn_1_AA48(void);
void fn_1_1160(void);
void fn_1_1228(void);
int fn_1_1364(int playerNo, s16 spaceId);
int fn_1_1398(int playerNo, s16 spaceId);
s32 fn_1_1280(s32 playerNo, s16 spaceId);
int fn_1_1444(void);
void fn_1_AD0(OMOBJ *obj);
void fn_1_124C(void);
void fn_1_127C(void);
void fn_1_C92C(void);
void fn_1_D2E4(s32 model, Mtx matrix);
void fn_1_E004(s32 enable);
s32 fn_1_E3EC(s32 playerNo, s32 linkCount, const s16 *spaceIds);
void fn_1_E104(void);
void fn_1_E224(void);
void mbev_ShopInit(int dataNum);
void mbev_NextTimeSet(void (*hook)(void));
void mbScrollInit(int dataNum);
void mbLightFuncSet(void (*setHook)(void), void (*resetHook)(void));
int mbCapThrowColCreate(int dataNum);
void mbMapCameraSet(const HuVecF *rot, const HuVecF *pos, float zoom);
void mbMapHookSet(void (*hook)(BOOL enterF));

/* Selects the board's model resource directory for the current day or night. */
static inline int W06TimeSideGet(void)
{
    return GwSystem.curTime == 0;
}

#define W06DataNumGet(fileNo) \
    ((W06TimeSideGet() != 0 ? DATA_w06 : DATA_w06n) | (u16)(fileNo))

static inline void W06BackgroundLoopSet(s16 modelId)
{
    mbObjAttrSet(modelId, HU3D_MOTATTR_LOOP);
}

/* Sets up Clockwork Castle's models, spaces, camera and callbacks when board setup enters it. */
void fn_1_F4(void)
{
    s32 boardNumbers[2];
    int objectId;
    s32 objectIndex;
    s16 drawModelId;

    boardNumbers[0] = GwSystem.boardNo;
    boardNumbers[1] = boardNumbers[0];
    HuAudSndGrpSetSet(27);
    lbl_1_bss_4B0 = (W06SaveState *)&GwSystem.boardWork[4];
    if (mbSaveNewF != 0) {
        for (objectIndex = 0; objectIndex < 6; objectIndex++) {
            lbl_1_bss_4B0->objectStates[objectIndex] = 0;
        }
        lbl_1_bss_4B0->state[0] = 0;
        lbl_1_bss_4B0->state[1] = 0;
    }
    mbMasuInit(W06DataNumGet(0));
    lbl_1_bss_4B4 = mbObjCreate(W06DataNumGet(1), NULL, 0);
    mbObjPosSet(lbl_1_bss_4B4, (0.0f), (0.0f), (0.0f));
    W06BackgroundLoopSet(lbl_1_bss_4B4);
    mbObjCullRadiusSet(lbl_1_bss_4B4, (-1.0f));
    mbObjMotionSpeedSet(lbl_1_bss_4B4, (1.0f));
    mbObjMotionTimeSet(lbl_1_bss_4B4, (0.0f));
    W06BackgroundLoopSet(lbl_1_bss_4B4);
    mbCameraNearFarSet((100.0f), (30000.0f));
    mbev_ShopInit(W06DataNumGet(2));
    mbev_NextTimeSet(fn_1_B584);
    for (objectIndex = 0; objectIndex < 2; objectIndex++) {
        lbl_1_bss_404.cleanupObjectIds[objectIndex] = objectId =
            mbObjCreate(W06DataNumGet(DATANUM(DATA_w06, 4) + objectIndex), NULL, 0);
        mbObjMotionTimeSet(objectId, (0.0f));
        mbObjMotionSpeedSet(objectId, (1.0f));
        mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    }
    if (GwSystem.curTime == 0) {
        lbl_1_bss_404.cleanupObjectIds[2] = objectId =
            mbObjCreate(W06DataNumGet(43), lbl_1_data_124, 1);
    } else {
        lbl_1_bss_404.cleanupObjectIds[2] = objectId =
            mbObjCreate(W06DataNumGet(43), lbl_1_data_134, 1);
    }
    mbObjHookSet((s16)lbl_1_bss_404.cleanupObjectIds[0], "hook00",
        (s16)lbl_1_bss_404.cleanupObjectIds[2]);
    mbObjMotionSet((s16)lbl_1_bss_404.cleanupObjectIds[2], 1, HU3D_MOTATTR_LOOP);
    if (GwSystem.curTime == 0) {
        lbl_1_bss_404.cleanupObjectIds[3] = objectId =
            mbObjCreate(W06DataNumGet(43), lbl_1_data_124, 1);
    } else {
        lbl_1_bss_404.cleanupObjectIds[3] = objectId =
            mbObjCreate(W06DataNumGet(43), lbl_1_data_134, 1);
    }
    mbObjHookSet((s16)lbl_1_bss_404.cleanupObjectIds[1], "hook01",
        (s16)lbl_1_bss_404.cleanupObjectIds[3]);
    mbObjMotionSet((s16)lbl_1_bss_404.cleanupObjectIds[3], 1, HU3D_MOTATTR_LOOP);
    fn_1_1CDC();
    fn_1_318C();
    fn_1_4A40();
    fn_1_63C4();
    fn_1_6FA4();
    fn_1_9474();
    for (objectIndex = 0; objectIndex < 4; objectIndex++) {
        lbl_1_bss_404.modelObjectIds[objectIndex] = objectId =
            mbObjCreate(W06DataNumGet(40), NULL, 1);
        mbObjPosSetV(objectId, &lbl_1_data_54[objectIndex]);
        mbObjScaleSet(objectId, (0.75f), (0.75f), (0.75f));
        mbObjMotionTimeSet(objectId, (0.0f));
        mbObjMotionSpeedSet(objectId, (1.0f));
        mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    }
    fn_1_AA48();
    mbBranchMAttrSet(W06_BRANCH_MATTR_BIT_13_MASK);
    mbBranchMAttrSet(W06_BRANCH_MATTR_BIT_16_MASK);
    mbBranchMAttrSet(W06_BRANCH_MATTR_LOW_SIX_BITS_MASK);
    mbScrollInit(W06DataNumGet(45));
    mbLightFuncSet(fn_1_1160, fn_1_1228);
    mbCapThrowColCreate(W06DataNumGet(46));
    HuDataDirClose(W06DataNumGet(0));
    HuDataDirClose(DATA_capsulechar4);
    mbev_MasuMoveStartSet(fn_1_1364);
    mbev_MasuMoveEndSet(fn_1_1398);
    mbev_MasuHatenaSet((MASUEVENTHOOK)fn_1_1280);
    mbev_MasuLinkTblHookSet((MASUPATHCHECKHOOK)fn_1_1444);
    lbl_1_bss_0 = omAddObjEx(mbObjMan, -32768, 0, 0, -1, fn_1_AD0);
    mbOpeningViewSet(&lbl_1_data_16C, &lbl_1_data_178, lbl_1_data_184);
    mbMapCameraSet(NULL, &lbl_1_data_194, lbl_1_data_1A0);
    mbPlayerTurnInitHookSet((void (*)(int))fn_1_124C);
    mbPlayerTurnCloseHookSet((void (*)(int))fn_1_127C);
    fn_1_C92C();
    drawModelId = Hu3DHookFuncCreate((HU3D_MODEL_HOOK)fn_1_D2E4);
    mbMapHookSet((void (*)(BOOL))fn_1_E004);
    mbBranchComStarHookSet((MBBRANCHCOMSTARHOOK)fn_1_E3EC);
    mbOpeningInstHookSet(fn_1_E104);
    mbOpeningStarInstHookSet(fn_1_E224);
}

#include "math.h"
#include "game/object.h"
#include "game/gamework.h"
#include "game/board/main.h"
#include "game/board/player.h"

extern OMOBJ *lbl_1_bss_0;
extern s32 lbl_1_bss_10;
extern s16 lbl_1_bss_4B4;

void fn_1_CAF0(void);
void fn_1_9C38(void);
void fn_1_CBE8(void);
void fn_1_30E4(void);
void fn_1_49AC(void);
void fn_1_6308(void);
void fn_1_6F38(void);
void fn_1_92C8(void);
void fn_1_A968(void);
void fn_1_B464(void);

/* Releases the board's models and event helpers when board object setup closes it. */
void fn_1_99C(OMOBJ *obj)
{
    s32 objectIndex;

    if (lbl_1_bss_4B4 >= 0) {
        mbObjKill(lbl_1_bss_4B4);
        lbl_1_bss_4B4 = -1;
    }
    for (objectIndex = 0; objectIndex < 2; objectIndex++) {
        mbObjKill((s16)lbl_1_bss_404.cleanupObjectIds[objectIndex]);
        lbl_1_bss_404.cleanupObjectIds[objectIndex] = -1;
    }
    mbObjKill((s16)lbl_1_bss_404.cleanupObjectIds[2]);
    lbl_1_bss_404.cleanupObjectIds[2] = -1;
    mbObjKill((s16)lbl_1_bss_404.cleanupObjectIds[3]);
    lbl_1_bss_404.cleanupObjectIds[3] = -1;
    fn_1_30E4();
    fn_1_49AC();
    fn_1_6308();
    fn_1_6F38();
    fn_1_92C8();
    fn_1_A968();
    for (objectIndex = 0; objectIndex < 4; objectIndex++) {
        mbObjKill((s16)lbl_1_bss_404.modelObjectIds[objectIndex]);
    }
    fn_1_B464();
}

/* Updates the board's roaming NPC and turn-player rotation from its object-manager callback. */
void fn_1_AD0(OMOBJ *obj)
{
    Point3d playerRotation;
    s32 playerNo;

    if (mbExitCheck() != 0 || lbl_1_bss_0 == NULL) {
        omDelObjEx(mbObjMan, obj);
        lbl_1_bss_0 = NULL;
        return;
    }

    fn_1_CAF0();
    fn_1_9C38();
    fn_1_CBE8();
    if (lbl_1_bss_10 != 0) {
        playerNo = GwSystem.turnPlayerNo;
        mbPlayerRotGet(playerNo, &playerRotation);
        if ((playerRotation.y += (3.0f)) >= (360.0f)) {
            playerRotation.y -= (360.0f);
        }
        mbPlayerRotSetV(playerNo, &playerRotation);
    }
}

#include "math.h"
#include "dolphin.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/gamework.h"
#include "game/board/object.h"

extern Point3d lbl_1_bss_68;
extern s32 lbl_1_bss_64;

extern char lbl_1_data_7F8[12];

extern s32 lbl_1_bss_30[6];
extern s32 lbl_1_bss_48[6];
extern s32 lbl_1_data_7B0[6][3];
extern HuVecF lbl_1_data_C[6];
extern OMOBJ *lbl_1_data_0[2];
extern s8 lbl_1_data_7A8[8];
extern OMOBJ *mbGuideCreate(int guideNo, HuVecF *pos, HuVecF *rot, s8 *motTbl,
                           float scale, u32 attr);
extern void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
extern s16 mbGuideModelGet(OMOBJ *obj);
extern void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
extern void mbGuideEnd(OMOBJ *obj, BOOL endF);

extern s32 lbl_1_bss_1C;
extern s32 lbl_1_bss_20;
extern f32 lbl_1_bss_24;
extern f32 lbl_1_bss_28;
extern f32 lbl_1_bss_2C;
extern GXColor lbl_1_data_1B2;

u16 mbPadDStkRepGetAll(void);

GXColor lbl_1_data_1B2 = {0, 0, 144, 192};
static const char *lbl_1_data_1C4[3] = {"start", "end", "add"};

/* Updates the fog range and step controls while the board fog panel is open. */
void fn_1_BD4(void)
{
    int row;
    int buttonRepeat;
    int panelX;
    int panelY;
    int panelWidth;
    int panelHeight;
    int directionRepeat;
    int rowCount;

    directionRepeat = mbPadDStkRepGetAll();
    buttonRepeat = HuPadBtnRep[0];
    if (buttonRepeat & 8) {
        lbl_1_bss_20 ^= 1;
    }
    if (lbl_1_bss_20 != 0) {
        rowCount = 3;
        panelX = 16;
        panelY = 160;
        panelWidth = 160;
        panelHeight = 12.0f + 1.5f * (rowCount * 8);
        printWin(panelX, panelY, panelWidth, panelHeight, &lbl_1_data_1B2);

        for (row = 0; row < 3; row++) {
            if (lbl_1_bss_1C == row) {
                fontcolor = FONT_COLOR_GREEN;
            } else {
                fontcolor = FONT_COLOR_YELLOW;
            }
            switch (row) {
            case 0:
                print8(panelX + 22, row * 12 + (panelY + 6), 1.5f,
                       "start->%3.1f", 0.01f * lbl_1_bss_2C);
                break;
            case 1:
                print8(panelX + 22, row * 12 + (panelY + 6), 1.5f,
                       "end->%3.1f", 0.01f * lbl_1_bss_28);
                break;
            case 2:
                print8(panelX + 22, row * 12 + (panelY + 6), 1.5f,
                       "add->%3.1f", 0.01f * lbl_1_bss_24);
                break;
            }
        }

        fontcolor = FONT_COLOR_WHITE;
        print8(panelX + 6, lbl_1_bss_1C * 12 + (panelY + 6), 1.5f, ">");
        if (buttonRepeat & 0x20) {
            switch (lbl_1_bss_1C) {
            case 0:
                lbl_1_bss_2C += lbl_1_bss_24;
                break;
            case 1:
                lbl_1_bss_28 += lbl_1_bss_24;
                break;
            case 2:
                lbl_1_bss_24 += 10.0f;
                break;
            }
        }
        if (buttonRepeat & 0x40) {
            switch (lbl_1_bss_1C) {
            case 0:
                lbl_1_bss_2C -= lbl_1_bss_24;
                break;
            case 1:
                lbl_1_bss_28 -= lbl_1_bss_24;
                break;
            case 2:
                lbl_1_bss_24 -= 10.0f;
                break;
            }
        }

        if (lbl_1_bss_2C <= (0.0f)) {
            lbl_1_bss_2C = (0.0f);
        }
        if (lbl_1_bss_28 <= (0.0f)) {
            lbl_1_bss_28 = (0.0f);
        }
        if (lbl_1_bss_24 <= (0.0f)) {
            lbl_1_bss_24 = (0.0f);
        }
        if (directionRepeat & 8) {
            if (--lbl_1_bss_1C < 0) {
                lbl_1_bss_1C = rowCount - 1;
            }
        }
        if (directionRepeat & 4) {
            if (++lbl_1_bss_1C >= rowCount) {
                lbl_1_bss_1C = 0;
            }
        }
        Hu3DFogClear();
        Hu3DFogSet(lbl_1_bss_2C, lbl_1_bss_28, 200, 200, 200);
    }
}
#include "math.h"
#include "game/board/masu.h"
#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

s32 mbSNpcCreate(u32 *, s32);               /* extern */
extern f32 lbl_1_bss_24;
extern f32 lbl_1_bss_28;
extern f32 lbl_1_bss_2C;
extern s16 lbl_1_bss_4B4;

/* Sets board lighting, creates the roaming NPC and initializes fog during light setup. */
void fn_1_1160(void) {
    Hu3DBGColorSet(255, 255, 255);
    Hu3DModelLightInfoSet(mbObjModelIDGet(lbl_1_bss_4B4), 1);
    mbSNpcCreate(GwSystem.boardWork, W06_SNPC_EXCLUDED_MASU_ATTR_MASK);
    lbl_1_bss_2C = (12000.0f);
    lbl_1_bss_28 = (100000.0f);
    lbl_1_bss_24 = (10.0f);
    Hu3DFogSet(lbl_1_bss_2C, lbl_1_bss_28, 200, 200, 200);
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

s32 mbSNpcKill();                               /* extern */

/* Releases the roaming NPC when the board light setup is closed. */
void fn_1_1228(void) {
    mbSNpcKill();
}

void fn_1_1248(void)
{
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

        /* extern */
s32 mbSNpcMotReset();                           /* extern */

/* Looks up the NPC's marked space but discards its ID, then resets NPC motion at turn start. */
void fn_1_124C(void) {
    s16 npcSpaceId;

    npcSpaceId = mbMasuFind_MAttrIdGet(-1, W06_NPC_MASU_ATTR_MASK);
    mbSNpcMotReset();
}

void fn_1_127C(void)
{
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

                            /* extern */
void fn_1_2AB8(s32, s16);                        /* static */
void fn_1_3380(int, s16);                        /* static */
void fn_1_4CC0(int, s16);                        /* static */
void fn_1_64D0(int, s16);                        /* static */
void fn_1_7100(int, s16);                        /* static */

/* Dispatches space-attribute events for the player from the board's special-space hook. */
s32 fn_1_1280(s32 playerNo, s16 spaceId) {
    s32 spaceAttributes;

    spaceAttributes = mbMasuMAttrGet(spaceId);
    if ((u32) (spaceAttributes & 0x4000) != 0) {
        fn_1_2AB8(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x40) != 0) {
        fn_1_3380(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x100) != 0) {
        fn_1_7100(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x80) != 0) {
        fn_1_7100(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x400) != 0) {
        fn_1_64D0(playerNo, spaceId);
    }
    if ((u32) (spaceAttributes & 0x200) != 0) {
        fn_1_4CC0(playerNo, spaceId);
    }
    return 1;
}

#include "dolphin/types.h"

s32 fn_1_135C(void)
{
    return 0;
}

#include "math.h"

extern void mbSNpcPlayerWalkSet(int playerNo, int masuId);

#include "game/board/masu.h"

extern s32 lbl_1_bss_14;
extern void fn_1_1FCC(int playerNo, s16 masuId);
extern void fn_1_A204(int playerNo, s16 masuId);
extern void mbSNpcStarExec(int playerNo, s16 masuId);

/* Starts the roaming NPC's walk when a player begins moving between spaces. */
int fn_1_1364(int playerNo, short spaceId)
{
    mbSNpcPlayerWalkSet(playerNo, spaceId);
    return 0;
}

/* Runs space effects and the roaming NPC's star event when a player's move ends. */
int fn_1_1398(int playerNo, s16 spaceId)
{
    MASU *masu;
    u32 masuAttr;

    masu = mbMasuGet(spaceId);
    masuAttr = mbMasuMAttrGet(spaceId);
    if ((masuAttr & 0x800) != 0) {
        fn_1_1FCC(playerNo, spaceId);
    }
    if (lbl_1_bss_14 >= 3 && mbMasuTypeGet(spaceId) == 0) {
        fn_1_A204(playerNo, spaceId);
    }
    mbSNpcStarExec(playerNo, (s16)spaceId);
    return 0;
}

int fn_1_1444(void)
{
    return -1;
}

#include "game/hu3d.h"
#include <string.h>

HSF_OBJECT *fn_1_1494(HU3D_MODEL *model, char *name, HSF_OBJECT *object);

/* Begins a named-object search at the root of a board model when requested. */
HSF_OBJECT *fn_1_144C(HU3D_MODEL *model, char *name)
{
    HSF_DATA *modelData = model->hsf;

    return fn_1_1494(model, name, modelData->root);
}

/* Searches the named object in a board model and its child objects. */
HSF_OBJECT *fn_1_1494(HU3D_MODEL *model, char *name, HSF_OBJECT *object)
{
    int childIndex;
    HSF_OBJECT *meshObject;
    HSF_OBJECT *replicaObject;
    HSF_OBJECT *result;

    switch (object->type) {
    case HSF_OBJ_NULL1:
    case HSF_OBJ_MESH:
    case HSF_OBJ_ROOT:
    case HSF_OBJ_JOINT:
    case HSF_OBJ_NULL2:
    case HSF_OBJ_NULL3:
    case HSF_OBJ_MAP:
        meshObject = object;
        if (!strcmp(name, meshObject->name)) {
            return meshObject;
        }
        for (childIndex = 0; childIndex < meshObject->mesh.childNum; childIndex++) {
            result = fn_1_1494(model, name, meshObject->mesh.child[childIndex]);
            if (result) {
                return result;
            }
        }
        break;
    case HSF_OBJ_REPLICA:
        replicaObject = object;
        if (!strcmp(name, replicaObject->name)) {
            return replicaObject;
        }
        for (childIndex = 0; childIndex < replicaObject->mesh.childNum; childIndex++) {
            result = fn_1_1494(model, name, replicaObject->mesh.child[childIndex]);
            if (result) {
                return result;
            }
        }
        break;
    }
    return NULL;
}

typedef struct W06RingVertex {
    Point3d position;
    Point3d velocity;
    f32 angle;
} W06RingVertex;
typedef struct W06SpaceObject {
    int objectId;
    s32 reactionObjectId;
    f32 bobAngle;
    s32 reactionActive;
    s32 bobDisabled;
    u8 usesReactionObject;
} W06SpaceObject;
typedef struct W06HookModels {
    s32 firstRootId;
    s32 secondRootId;
    s32 attachedIds[11];
} W06HookModels;

s16 lbl_1_bss_4B4;
W06SaveState *lbl_1_bss_4B0;
W06BoardObjects lbl_1_bss_404;
W06SpaceObject lbl_1_bss_2E4[4][3];
u8 lbl_1_bss_2E0[4];
W06HookModels lbl_1_bss_2AC;
W06RingVertex lbl_1_bss_7C[20];
s32 lbl_1_bss_74[2];
Point3d lbl_1_bss_68;
s32 lbl_1_bss_64;
s16 lbl_1_bss_60;
s32 lbl_1_bss_48[6];
s32 lbl_1_bss_30[6];
f32 lbl_1_bss_2C;
f32 lbl_1_bss_28;
f32 lbl_1_bss_24;
s32 lbl_1_bss_20;
s32 lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
s32 lbl_1_bss_14;
s32 lbl_1_bss_10;
u8 lbl_1_bss_C[4];
s32 lbl_1_bss_8;
u8 lbl_1_bss_4[4];
OMOBJ *lbl_1_bss_0;

#include "math.h"
#include "dolphin.h"
#include "game/board/main.h"
#include "game/board/object.h"
#include "game/gamework.h"

#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/effect.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/data.h"
#include "game/process.h"

extern s32 lbl_1_bss_30[6];
extern s32 lbl_1_bss_48[6];

extern HuVecF lbl_1_data_C[6];
extern s32 lbl_1_data_1F8[6];
extern s32 lbl_1_data_210[2][3];
extern s32 lbl_1_data_228[2][3];
extern HuVecF lbl_1_data_244[2];
extern HuVecF lbl_1_data_25C;
extern f32 lbl_1_data_268[2];
extern HuVecF lbl_1_data_270[2];
extern s32 lbl_1_data_288[6];

void fn_1_C5D0(s32 mode);
void fn_1_DACC(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix);

s32 lbl_1_data_1F8[6] = {2, 0, 1, 0, 2, 1};
s32 lbl_1_data_210[2][3] = {{1, 4, 4}, {1, 2, 4}};
s32 lbl_1_data_228[2][3] = {
    {W06_BOARD_DATA_W06N_23, W06_BOARD_DATA_W06N_25, W06_BOARD_DATA_W06N_25},
    {W06_BOARD_DATA_W06_23, W06_BOARD_DATA_W06_24, W06_BOARD_DATA_W06_25}
};

/* Creates the six time-specific entrance tiles during board setup. */
void fn_1_1CDC(void)
{
    s32 timeIndex;
    s32 objectIndex;
    s32 item;
    s32 currentObject;
    s32 rowOffset;
    s32 useEarlyDisplay;
    s32 modelPrefix;
    s32 displayObjectId;

    timeIndex = 0;
    if (GwSystem.curTime == 0) {
        timeIndex++;
    }

    if (mbSaveNewF != FALSE) {
        for (objectIndex = 0; objectIndex < 6; objectIndex++) {
            *(&lbl_1_bss_4B0->objectStates[0] + objectIndex) = (u8)lbl_1_data_1F8[objectIndex];
        }
    }

    currentObject = 0;
    for (objectIndex = 0; objectIndex < 2; objectIndex++) {
        for (item = 0; item < 3; item++) {
            lbl_1_bss_30[currentObject] = mbObjCreate(
                lbl_1_data_228[timeIndex][item], NULL, TRUE);
            lbl_1_bss_48[currentObject] = lbl_1_data_210[timeIndex][item];
            currentObject++;
        }
    }

    currentObject = 0;
    for (objectIndex = 0; objectIndex < 2; objectIndex++) {
        for (item = 0; item < 3; item++) {
            rowOffset = objectIndex * 3;
            lbl_1_bss_404.displayObjects[currentObject].objectId =
                lbl_1_bss_30[*(&lbl_1_bss_4B0->objectStates[0] + currentObject) + rowOffset];
            lbl_1_bss_404.displayObjects[currentObject].dataId =
                lbl_1_bss_48[*(&lbl_1_bss_4B0->objectStates[0] + currentObject) + rowOffset];
            currentObject++;
        }
    }

    for (objectIndex = 0; objectIndex < 6; objectIndex++) {
        mbObjPosSetV((s16)lbl_1_bss_404.displayObjects[objectIndex].objectId,
                     &lbl_1_data_C[objectIndex]);
    }

    for (objectIndex = 0; objectIndex < 6; objectIndex++) {
        useEarlyDisplay = !GwSystem.curTime;
        if (useEarlyDisplay != 0) {
modelPrefix = DATA_w06;
        } else {
modelPrefix = DATA_w06n;
        }
        displayObjectId = (s16)mbObjCreate(modelPrefix | 0x1A, NULL, TRUE);
        lbl_1_bss_404.trailingValues[objectIndex] = displayObjectId;
        mbObjPosSetV(displayObjectId, &lbl_1_data_C[objectIndex]);
        mbObjDispSet(displayObjectId, FALSE);
        mbObjMotionTimeSet(displayObjectId, (0.0f));
        mbObjMotionSpeedSet(displayObjectId, (0.0f));
    }
}

#include "game/board/masu.h"
#include "game/board/comchoice.h"
#include "game/board/status.h"

extern u32 lbl_1_data_84[40];

/* For entrances whose data ID is not 4, offers transfer to the paired entrance and plays entry and
 * exit. */
void fn_1_1FCC(int playerNo, s16 spaceId)
{
    char priceText[20];
    s16 parentLinks[MASU_LINK_MAX];
    Point3d playerPosition;
    Point3d entrancePosition;
    Point3d movement;
    Point3d heightOffset;
    Point3d exitPosition;
    HuVec2f windowPosition;
    u32 spaceAttributes;
    u32 destinationMask;
    MASU *space;
    f32 progress;
    f32 originalZoom;
    f32 expandedZoom;
    u32 entranceIndex;
    u32 entranceAttributes;
    s32 index;
    s16 entranceSpace;
    s16 exitSpace;
    s16 landingSpace;
    s16 starSpace;
    s32 distanceToStar;
    s32 distanceFromStar;
    s32 movesLeft;
    s32 objectType;
    s32 choice;
    s32 price;

    space = mbMasuGet(spaceId);
    spaceAttributes = mbMasuMAttrGet(spaceId);
    entranceSpace = mbMasuTypeFindLink(spaceId, 0);
    entranceAttributes = mbMasuMAttrGet(entranceSpace);
    for (index = 0; index < 6; index++) {
        if ((entranceAttributes & lbl_1_data_84[index]) != 0) {
            entranceIndex = index;
        }
    }
    objectType = lbl_1_bss_404.displayObjects[entranceIndex].dataId;
    if (objectType != 4) {
        if (GwSystem.curTime == 0) {
            price = 5;
        } else {
            price = 10;
        }
        if (entranceIndex == 0) {
            mbPlayerRotateStart(playerNo, -90, 15);
        } else if (entranceIndex == 2) {
            mbPlayerRotateStart(playerNo, 90, 15);
        } else {
            mbPlayerRotateStart(playerNo, 180, 15);
        }
        while (mbPlayerRotateCheck(playerNo) == FALSE) {
            HuPrcVSleep();
        }
mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        sprintf(priceText, "%d", price);
        do {
mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W06, 0), -1, 0);
            mbWinTopInsertMesSet((u32)priceText, 0);
            mbWinTopPosGet(&windowPosition);
            windowPosition.y -= (21.0f);
            mbWinTopPosSet(windowPosition.x, windowPosition.y);
            if (GwPlayer[playerNo].comF != FALSE) {
                starSpace = mbSNpcMasuGet();
                distanceToStar = mbMasuFind_IdStepGet2(spaceId, starSpace, TRUE, TRUE);
                distanceFromStar = mbMasuFind_IdStepGet2(starSpace, spaceId, TRUE, TRUE);
                movesLeft = GwPlayer[playerNo].moveNum;
                if (GwSystem.curTime == 0) {
                    if (distanceToStar <= movesLeft + 5 || distanceFromStar <= 5) {
                        mbComChoiceDownSet();
                    } else {
                        mbComChoiceUpSet();
                    }
                } else if (distanceToStar <= movesLeft || distanceFromStar <= 5) {
                    mbComChoiceUpSet();
                } else {
                    mbComChoiceDownSet();
                }
            }
            mbWinTopWait();
            choice = mbWinTopChoiceGet();
            switch (choice) {
            case -1:
                mbMoveNumDispSet(playerNo, TRUE);
                return;
            case 1:
                mbMoveNumDispSet(playerNo, TRUE);
                return;
            case 2:
                mbMoveNumDispSet(playerNo, FALSE);
                mbev_Scroll(playerNo, TRUE);
                mbStatusDispSetAll(TRUE);
                mbMoveNumDispSet(playerNo, TRUE);
                break;
            }
        } while (choice == 2);
        mbMoveNumDispSet(playerNo, FALSE);
        for (index = 0; index < 6; index++) {
            if (index != entranceIndex &&
                objectType == lbl_1_bss_404.displayObjects[index].dataId) {
                destinationMask = lbl_1_data_84[index];
                break;
            }
        }
        exitSpace = mbMasuFind_MAttrIdGet(-1, destinationMask);
        mbMasuLinkParentGet(exitSpace, parentLinks);
        landingSpace = parentLinks[0];
        mbMasuPosGet(entranceSpace, &entrancePosition);
        mbPlayerEffectSet(playerNo, FALSE);
        mbPlayerWinLoseVoicePlay(playerNo, 12, CHARVOICEID(6));
        mbPlayerMotionShiftSet(playerNo, 4, (0.0f), (8.0f), 0);
        mbPlayerPosGet(playerNo, &playerPosition);
        entrancePosition.z -= (50.0f);
        movement.x = (entrancePosition.x - playerPosition.x) / (25.0f);
        movement.z = (entrancePosition.z - playerPosition.z) / (25.0f);
        for (index = 0; (f32)index < (25.0f); index++) {
            progress = (f32)index / (25.0f);
            if (index == 10) {
                mbAudFXPlay(MSM_SE_BRD00_40);
                omVibrate(playerNo, 20, 7, 3);
            }
            playerPosition.x += movement.x;
            playerPosition.z += movement.z;
            mbPlayerPosSet(playerNo, playerPosition.x,
                playerPosition.y + (100.0) *
                    ((3.0) * sin((3.141592653589793) *
                        ((180.0f) * progress) / (180.0))),
                playerPosition.z);
            HuPrcVSleep();
        }
        mbPlayerPosGet(playerNo, &playerPosition);
        movement.y = (40.0f);
        for (index = 0; (f32)index < (5.0f); index++) {
            playerPosition.y -= movement.y;
            mbPlayerPosSetV(playerNo, &playerPosition);
            HuPrcVSleep();
        }
        heightOffset.y = fabs(entrancePosition.y - playerPosition.y);
mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        mbMasuPosGet(exitSpace, &exitPosition);
        exitPosition.y -= heightOffset.y;
        exitPosition.z -= (50.0f);
        HuPrcSleep(20);
        originalZoom = mbCameraZoomGet();
        expandedZoom = (1000.0f) + originalZoom;
        mbCameraMovePos(NULL, NULL, NULL, expandedZoom, (-1.0f), 30);
        mbCameraMoveWait();
        mbCameraFocusPlayerSet(-1);
        mbPlayerPosSetV(playerNo, &exitPosition);
        mbCameraMovePos(&exitPosition, NULL, NULL, (-1.0f), (-1.0f), 90);
        mbCameraMoveWait();
        mbCameraFocusPlayerSet(playerNo);
        mbCameraMovePos(NULL, NULL, NULL, originalZoom, (-1.0f), 30);
        mbCameraMoveWait();
        entranceAttributes = mbMasuMAttrGet(exitSpace);
        for (index = 0; index < 6; index++) {
            if ((entranceAttributes & lbl_1_data_84[index]) != 0) {
                entranceIndex = index;
            }
        }
        if (entranceIndex == 0) {
            mbPlayerRotateStart(playerNo, 90, 1);
        } else if (entranceIndex == 2) {
            mbPlayerRotateStart(playerNo, -90, 1);
        } else {
            mbPlayerRotateStart(playerNo, 0, 1);
        }
mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
        HuPrcSleep(20);
        mbMasuPosGet(landingSpace, &entrancePosition);
        mbPlayerPosGet(playerNo, &playerPosition);
        movement.y = (entrancePosition.y - playerPosition.y) / (5.0f);
        mbPlayerMotionShiftSet(playerNo, 4, (0.0f), (1.0f), 0);
        for (index = 0; (f32)index < (5.0f); index++) {
            playerPosition.y += movement.y;
            mbPlayerPosSetV(playerNo, &playerPosition);
            HuPrcVSleep();
        }
        mbAudFXPlay(MSM_SE_BRD00_40);
        omVibrate(playerNo, 20, 7, 3);
        mbPlayerPosGet(playerNo, &playerPosition);
        movement.x = (entrancePosition.x - playerPosition.x) / (25.0f);
        movement.z = (entrancePosition.z - playerPosition.z) / (25.0f);
        for (index = 0; (f32)index < (25.0f); index++) {
            progress = (f32)index / (25.0f);
            playerPosition.x += movement.x;
            playerPosition.z += movement.z;
            mbPlayerPosSet(playerNo, playerPosition.x,
                playerPosition.y + (100.0) *
                    ((3.0) * sin((3.141592653589793) *
                        ((180.0f) * progress) / (180.0))),
                playerPosition.z);
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 5, (0.0f), (0.0f), 0);
        GwPlayer[playerNo].masuId = landingSpace;
        mbPlayerEffectSet(playerNo, TRUE);
        mbMoveNumDispSet(playerNo, TRUE);
    }
}

/* Camera positions and zooms for revealing the six board tiles. */
HuVecF lbl_1_data_244[2] = {{-2760.0f, 2100.0f, 367.0f}, {-3180.0f, -1014.0f, 2197.0f}};
HuVecF lbl_1_data_25C = {-30.0f, 0.0f, 0.0f};
f32 lbl_1_data_268[2] = {2400.0f, 6000.0f};
HuVecF lbl_1_data_270[2] = {{2760.0f, 2100.0f, 367.0f}, {3180.0f, -1014.0f, 2197.0f}};
s32 lbl_1_data_288[6] = {0, 140, 300, 0, 140, 300};
Point3d lbl_1_data_2A0 = {0.0f, 2069.300048828125f, -3990.800048828125f};
s8 lbl_1_data_2AC[8] = {12, 10, 22, 7, -1, 0, 0, 0};
Point3d lbl_1_data_2B4 = {11.800000190734863f, 2004.699951171875f, -3524.300048828125f};
Point3d lbl_1_data_2C0 = {-5.0f, 0.0f, 0.0f};

/* Reveals the six tiles and their bursts when a player reaches this special space. */
void fn_1_2AB8(s32 playerNo, s16 spaceId)
{
    s32 displayDone[6];
    s32 particleIds[3];
    Point3d playerPosition;
    Point3d particlePosition;
    ANIMDATA *animations[3];
    s32 work[4];
    s32 index;
    s32 side;
    s32 vibrationPlayer;
    s32 elapsed;
    s32 completed;
    s32 soundHandle;
    s16 displayObjectId;

    work[3] = 0;
    work[2] = 0;
    work[1] = 0;
    work[0] = 0;
HuDataDirRead(DATA_effect);
    for (index = 0; index < 3; index++) {
animations[index] = HuSprAnimRead(HuDataReadNum(
    DATANUM(DATA_effect, 1), HU_MEMNUM_OVL));
    }
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == 0) {
        HuPrcVSleep();
    }
mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 1), -1);
    mbWinTopWait();
    mbWipeSpecialFadeInCreate(1, 60);
    mbPlayerPosGet(playerNo, &playerPosition);
    if (playerPosition.z >= (0.0f)) {
        fn_1_C5D0(1);
        side = 1;
    } else {
        fn_1_C5D0(0);
        side = 0;
    }
    for (index = 0; index < 6; index++) {
        mbObjDispSet((s16)lbl_1_bss_404.displayObjects[index].objectId, 0);
        mbObjDispSet((s16)lbl_1_bss_404.trailingValues[index], 1);
        mbObjMotionTimeSet((s16)lbl_1_bss_404.trailingValues[index], (0.0f));
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.trailingValues[index], (1.0f));
        displayObjectId = (s16)lbl_1_bss_404.trailingValues[index];
mbObjAttrSet(displayObjectId, HU3D_MOTATTR_LOOP);
        displayDone[index] = 0;
    }
    mbCameraFocusPlayerSet(-1);
    mbCameraMovePos(&lbl_1_data_244[side], &lbl_1_data_25C, NULL,
        lbl_1_data_268[side], (-1.0f), 0);
    mbWipeSpecialFadeOutCreate(1, 60);
    HuPrcVSleep();
    soundHandle = mbAudFXPlay(W06_CAPSULE_MACHINE_EFFECT_SOUND_ID);
    mbCameraMovePos(&lbl_1_data_244[side], &lbl_1_data_25C, NULL,
        lbl_1_data_268[side], (-1.0f), 60);
    mbCameraMoveWait();
    mbCameraMovePos(&lbl_1_data_270[side], NULL, NULL,
        (-1.0f), (-1.0f), 300);
    elapsed = 0;
    completed = 0;
    for (;;) {
        for (index = 0; index < 6; index++) {
            if (elapsed >= lbl_1_data_288[index] && displayDone[index] == 0) {
                if (playerPosition.z >= (0.0f)) {
                    if (index < 3) {
                        particleIds[index] = mbParticleCreate(animations[index], 256);
                        mbParticleHookSet((s16)particleIds[index], fn_1_DACC);
                        Hu3DModelLayerSet((s16)particleIds[index], 5);
                        mbObjPosGet((s16)lbl_1_bss_404.trailingValues[index], &particlePosition);
                        particlePosition.y += (200.0f);
                        Hu3DModelPosSetV((s16)particleIds[index], &particlePosition);
                    }
                } else if (index >= 3) {
                    particleIds[index - 3] = mbParticleCreate(animations[index - 3], 256);
                    mbParticleHookSet((s16)particleIds[index - 3], fn_1_DACC);
                    Hu3DModelLayerSet((s16)particleIds[index - 3], 5);
                    mbObjPosGet((s16)lbl_1_bss_404.trailingValues[index], &particlePosition);
                    particlePosition.y += (200.0f);
                    Hu3DModelPosSetV((s16)particleIds[index - 3], &particlePosition);
                }
                mbAudFXPlay(MSM_SE_W06_TILE_REVEAL);
                mbObjMotionSpeedSet((s16)lbl_1_bss_404.trailingValues[index], (0.0f));
                mbObjDispSet((s16)lbl_1_bss_404.displayObjects[index].objectId, 1);
                mbObjDispSet((s16)lbl_1_bss_404.trailingValues[index], 0);
                displayDone[index] = 1;
                completed++;
                for (vibrationPlayer = 0; vibrationPlayer < 4; vibrationPlayer++) {
                    omVibrate((s16)vibrationPlayer, 20, 4, 4);
                }
            }
        }
        if (completed >= 6) {
            break;
        }
        elapsed++;
        HuPrcVSleep();
    }
    mbAudFXStop(soundHandle);
    mbCameraMoveWait();
    HuPrcSleep(60);
    mbWipeSpecialFadeInCreate(1, 60);
    mbCameraPlayerViewSetFast(playerNo, 1);
    mbWipeSpecialFadeOutCreate(1, 60);
    HuPrcVSleep();
    for (index = 0; index < 3; index++) {
        if (particleIds[index] != -1) {
            mbParticleKill((s16)particleIds[index]);
        }
    }
HuDataDirClose(DATA_effect);
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

/* Releases both model sets for the six board object slots when the board closes. */
void fn_1_30E4(void) {
    s32 objectIndex;

    objectIndex = 0;
    while (objectIndex < 6) {
        if ((s32) *(s32 *) ((u8 *) &lbl_1_bss_30 + (objectIndex * 4)) != -1) {
            mbObjKill((s16) *(s32 *) ((u8 *) &lbl_1_bss_30 + (objectIndex * 4)));
        }
        if ((s32) (*(s32 *) ((s8 *) ((s32 *) ((u8 *) &lbl_1_bss_404 + (objectIndex * 4))) +
                             (W06_BOARD_MODEL_OBJECT_IDS_OFFSET))) != -1) {
            mbObjKill((s16) (*(
                s32 *) ((s8 *) ((s32 *) ((u8 *) &lbl_1_bss_404 + (objectIndex * 4))) +
                    (W06_BOARD_MODEL_OBJECT_IDS_OFFSET))));
        }
        objectIndex += 1;
    }
}

/* Creates the board's paired machinery models during board setup. */
void fn_1_318C(void)
{
    s32 objectId;
    s32 objectType1;
    s32 objectType2;
    s32 objectType3;
    s32 objectType4;
    s32 currentTimeIsClear1;
    s32 currentTimeIsClear2;
    s32 currentTimeIsClear3;
    s32 currentTimeIsClear4;

    currentTimeIsClear1 = GwSystem.curTime == 0;
    if (currentTimeIsClear1 != 0) {
        objectType1 = DATA_w06;
    } else {
        objectType1 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType1 | 0x1B, NULL, 0);
    lbl_1_bss_404.objectIds[1] = objectId;
    currentTimeIsClear2 = GwSystem.curTime == 0;
    if (currentTimeIsClear2 != 0) {
        objectType2 = DATA_w06;
    } else {
        objectType2 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType2 | 0x1C, NULL, 0);
    lbl_1_bss_404.objectIds[2] = objectId;
    mbObjMotionSpeedSet(objectId, (0.0f));
    mbObjMotionTimeSet(objectId, (0.0f));
mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    currentTimeIsClear3 = GwSystem.curTime == 0;
    if (currentTimeIsClear3 != 0) {
        objectType3 = DATA_w06;
    } else {
        objectType3 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType3 | 0x1C, NULL, 0);
    lbl_1_bss_404.objectIds[3] = objectId;
    mbObjMotionSpeedSet(objectId, (0.0f));
    mbObjMotionTimeSet(objectId, (0.0f));
mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    mbObjDispSet(objectId, 0);
    currentTimeIsClear4 = GwSystem.curTime == 0;
    if (currentTimeIsClear4 != 0) {
        objectType4 = DATA_w06;
    } else {
        objectType4 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType4 | 0x24, NULL, 0);
    lbl_1_bss_404.remainingObjectIds[3] = objectId;
    mbObjDispSet(objectId, 0);
}

#include "game/process.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/effect.h"

extern s8 lbl_1_data_2AC[];
extern Point3d lbl_1_data_2B4;
extern Point3d lbl_1_data_2C0;

extern u32 mbRandMod(u32 max);

int mbGuideNoGet(void);
void mbChangeTime(void);
void fn_1_C1CC(void);

void mbGuideKill(OMOBJ *obj);

void fn_1_D4C8(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix);

/* Lets the player stop the wheel or stops it when the timer expires, then applies the day or night
 * change. */
void fn_1_3380(int playerNo, s16 spaceId)
{
    Point3d guidePositions[2];
    Point3d savedCenter;
    Point3d savedRotation;
    Point3d playerPosition;
    Point3d wheelPosition;
    Point3d particlePosition;
    f32 speed;
    f32 secondSpeed;
    f32 maximumSpeed;
    f32 alpha;
    f32 acceleration;
    f32 deceleration;
    f32 fadeProgress;
    f32 maximumTime;
    f32 firstMotionTime;
    f32 secondMotionTime;
    f32 firstAngle;
    f32 savedZoom;
    int index;
    s32 particleId;
    s32 vibratingPlayer;
    s32 helpWindow;
    s16 timerMessage;
    s32 state;
    s32 computerDelay;
    s32 fadeFrames;
    s32 speaker;
    s32 timeLeft;
    s32 messageOffset;
    s32 stopFrames;
    s16 entranceSpace;
    int result;
    s16 guideModels[2];
    f32 secondAngle;
    BOOL wheelDone;
    BOOL fadeDone;
    BOOL forcedStop;
    ANIMDATA *animation;
    int sound;
    s16 previousSpace;

    computerDelay = 0;
    maximumSpeed = (0.0f);
    speed = (0.0f);
    secondSpeed = (0.0f);
    deceleration = (0.0f);
    wheelDone = 0;
    fadeDone = 0;
    forcedStop = 0;
    if (GwSystem.curTime == 0) {
        speaker = 6;
        messageOffset = 0;
    } else {
        speaker = 7;
        messageOffset = 1;
    }
    entranceSpace = mbMasuTypeFindLink(spaceId, 0);
    previousSpace = spaceId;
    GwPlayer[playerNo].masuIdPrev = previousSpace;
    mbPlayerMasuMoveTo(playerNo, entranceSpace, TRUE);
    GwPlayer[playerNo].masuId = entranceSpace;
    mbPlayerPosGet(playerNo, &playerPosition);
    mbPlayerRotateStart(playerNo, 180, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    mbCameraCenterGet(&savedCenter);
    mbCameraRotGet(&savedRotation);
    savedZoom = mbCameraZoomGet();
    mbCameraFocusPlayerSet(-1);
    mbCameraMovePos(&lbl_1_data_2B4, &lbl_1_data_2C0, NULL,
        (2000.0f), (-1.0f), 30);
    mbCameraMoveWait();
    for (index = 0; index < 2; index++) {
        guidePositions[index] = playerPosition;
        guidePositions[index].x = (200.0) *
            cos((3.141592653589793) * ((180.0f) * (f32)index) / (180.0));
        guidePositions[index].x *= (-1.0f);
        guidePositions[index].y += (200.0f);
        guidePositions[index].z += (300.0f);
    }
    if (GwSystem.curTime == 0) {
        for (index = 0; index < 2; index++) {
            lbl_1_data_0[index] = mbGuideCreate(index, &guidePositions[index], NULL,
                lbl_1_data_2AC, (1.0f), 2);
            mbGuideMotionNextSet(lbl_1_data_0[index], 1);
            guideModels[index] = mbGuideModelGet(lbl_1_data_0[index]);
        }
    } else {
        for (index = 1; index > -1; index--) {
            lbl_1_data_0[index] = mbGuideCreate(index, &guidePositions[index], NULL,
                lbl_1_data_2AC, (1.0f), 2);
            mbGuideMotionNextSet(lbl_1_data_0[index], 1);
            guideModels[index] = mbGuideModelGet(lbl_1_data_0[index]);
        }
    }
    HuPrcSleep(20);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbGuideMotionShiftSet(lbl_1_data_0[mbGuideNoGet()], 12, TRUE);
mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 7), speaker);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    mbGuideMotionShiftSet(lbl_1_data_0[mbGuideNoGet()], 12, TRUE);
mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 9), speaker);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbGuideMotionShiftSet(lbl_1_data_0[mbGuideNoGet()], 12, TRUE);
mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 11), speaker);
    mbWinTopWait();
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[2], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[3], (0.0f));
    maximumSpeed = (4.0f) + (f32)mbRandMod(2);
    state = 1;
    acceleration = (0.15000000596046448f);
    sound = mbAudFXPlay(MSM_SE_W06_WHEEL_SPIN);
    for (;;) {
        speed += acceleration;
        secondSpeed += acceleration;
        if (speed >= maximumSpeed && secondSpeed >= maximumSpeed) {
            speed = maximumSpeed;
            secondSpeed = maximumSpeed;
            state = 1;
            break;
        }
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[2], speed);
        HuPrcVSleep();
    }
animation = HuSprAnimRead(HuDataReadNum(
    DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
    mbPlayerPosGet(playerNo, &playerPosition);
    wheelPosition = playerPosition;
    wheelPosition.y += (240.00001525878906f);
    mbObjDispSet((s16)lbl_1_bss_404.remainingObjectIds[3], TRUE);
    mbObjPosSetV((s16)lbl_1_bss_404.remainingObjectIds[3], &wheelPosition);
    mbObjAlphaSet((s16)lbl_1_bss_404.remainingObjectIds[3], 0);
    particleId = mbParticleCreate(animation, 64);
    mbParticleHookSet(particleId, fn_1_D4C8);
    Hu3DModelLayerSet(particleId, 5);
    particlePosition = wheelPosition;
    particlePosition.y += (30.0);
    Hu3DModelPosSetV(particleId, &particlePosition);
    for (index = 0; (f32)index < (35.0f); index++) {
        fadeProgress = (f32)index / (35.0f);
        alpha = (1.5f) * ((255.0f) * fadeProgress);
        if (alpha >= (255.0f)) {
            alpha = (255.0f);
        }
        mbObjAlphaSet((s16)lbl_1_bss_404.remainingObjectIds[3], (s32)alpha);
        HuPrcVSleep();
    }
    HuPrcSleep(20);
    mbParticleKill(particleId);
    particleId = -1;
    helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W06, 19));
    mbWinPosSet(helpWindow, 240, 376);
    mbObjPosGet((s16)lbl_1_bss_404.remainingObjectIds[3], &wheelPosition);
    computerDelay = mbRandMod(60);
    timeLeft = 600;
    timerMessage = GameMesCreate(1, 10, -1, -1);
    HuSprGrpDrawNoSet(GameMesGet(timerMessage)->grpId[0], 32);
    if (GwPlayer[playerNo].comF != FALSE) {
        computerDelay = (s32)((30.0f) + (f32)mbRandMod(90));
    }
    while (wheelDone == 0 || fadeDone == 0) {
        if (timerMessage == -1 && forcedStop == 0) {
            if (helpWindow != -1) {
                mbWinKill(helpWindow);
                helpWindow = -1;
            }
            deceleration = (0.009999999776482582f) * (f32)(mbRandMod(5) + 4);
            state = 2;
            forcedStop = 1;
            stopFrames = 0;
            fadeFrames = 0;
        }
        switch (state) {
        case 0:
            speed += acceleration;
            secondSpeed += acceleration;
            if (speed >= maximumSpeed && secondSpeed >= maximumSpeed) {
                speed = maximumSpeed;
                secondSpeed = maximumSpeed;
                state = 1;
                helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W06, 19));
                mbWinPosSet(helpWindow, 240, 376);
                if (GwPlayer[playerNo].comF != FALSE) {
                    computerDelay = mbRandMod(60);
                }
            }
            break;
        case 1:
            if (GwPlayer[playerNo].comF != FALSE) {
                computerDelay--;
                if ((f32)computerDelay <= (0.0f)) {
                    HuPadBtnDown[GwPlayer[playerNo].padNo] |= 0x100;
                }
            }
            if ((HuPadBtnDown[GwPlayer[playerNo].padNo] & 0x100) != 0) {
                mbPlayerMotionShiftSet(playerNo, 11, (10.0f), (5.0f), 0);
                stopFrames = 0;
                fadeFrames = 0;
                deceleration = (0.019999999552965164f) * (f32)(mbRandMod(5) + 2);
                state = 2;
                if (helpWindow != -1) {
                    mbWinKill(helpWindow);
                    helpWindow = -1;
                }
            }
            break;
        case 2:
            speed -= deceleration;
            secondSpeed -= deceleration;
            if (speed <= (0.0f) && secondSpeed <= (0.0f)) {
                speed = (0.0f);
                secondSpeed = (0.0f);
                state = 3;
            }
            break;
        case 3:
            mbAudFXStop(sound);
            mbAudFXPlay(MSM_SE_W06_WHEEL_STOP);
            wheelDone = 1;
            break;
        }
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[2], speed);
        if (state >= 2) {
            if ((f32)stopFrames++ > (14.0f)) {
                if (particleId == -1) {
                    omVibrate(playerNo, 20, 7, 3);
                    mbAudFXPlay(MSM_SE_BRD00_56);
                    animation =
                        HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
                    particleId = mbParticleCreate(animation, 32);
                    mbParticleHookSet(particleId, fn_1_D4C8);
                    Hu3DModelLayerSet(particleId, 5);
                    particlePosition = wheelPosition;
                    particlePosition.y += (30.0);
                    Hu3DModelPosSetV(particleId, &particlePosition);
                }
                if ((f32)fadeFrames >= (20.0f)) {
                    fadeDone = 1;
                } else {
                    fadeFrames++;
                    fadeProgress = (f32)fadeFrames / (20.0f);
                    alpha = (255.0f) - (255.0f) * fadeProgress;
                    mbObjAlphaSet((s16)lbl_1_bss_404.remainingObjectIds[3],
                        alpha);
                    if (forcedStop == 0) {
                        mbObjPosSet((s16)lbl_1_bss_404.remainingObjectIds[3], wheelPosition.x,
                            wheelPosition.y + (10.0) *
                                sin((3.141592653589793) * ((180.0f) * fadeProgress) /
                                    (180.0)), wheelPosition.z);
                    }
                }
            }
        }
        if (timeLeft >= 0) {
            timeLeft--;
            if (timeLeft >= 0) {
                GameMesDispSet(timerMessage, 1, (s16)((timeLeft + 59) / 60));
            } else if (timerMessage >= 0) {
                GameMesDispSet(timerMessage, 2, -1);
                timerMessage = -1;
            }
        }
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[2], (0.0f));
    if (timerMessage >= 0) {
        GameMesDispSet(timerMessage, 2, -1);
        timerMessage = -1;
    }
    mbParticleKill(particleId);
    mbObjDispSet((s16)lbl_1_bss_404.remainingObjectIds[3], FALSE);
    maximumTime = mbObjMotionMaxTimeGet((s16)lbl_1_bss_404.objectIds[2]);
    firstMotionTime = mbObjMotionTimeGet((s16)lbl_1_bss_404.objectIds[2]);
    firstAngle = (360.0f / maximumTime) * firstMotionTime;
    maximumTime = mbObjMotionMaxTimeGet((s16)lbl_1_bss_404.objectIds[3]);
    secondMotionTime = mbObjMotionTimeGet((s16)lbl_1_bss_404.objectIds[3]);
    secondAngle = (360.0f / maximumTime) * secondMotionTime;
    if (firstAngle < (180.0f)) {
        result = 1;
    } else {
        result = 0;
    }
    switch (result) {
    case 0:
        mbAudFXPlay(MSM_SE_GUIDE_25);
        mbGuideMotionShiftSet(lbl_1_data_0[0], 12, TRUE);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 13), 6);
        mbWinTopWait();
        for (index = 0; index < 2; index++) {
            mbAudFXPlay(index + MSM_SE_W06_GUIDE_MOTION);
            mbObjMotionShiftSet(guideModels[index], 10, (0.0f), (8.0f), 0);
        }
        if (GwSystem.curTime == 0) {
            HuPrcSleep(120);
            for (index = 0; index < 2; index++) {
                mbGuideMotionNextSet(lbl_1_data_0[index], 1);
            }
            mbAudFXPlay(MSM_SE_GUIDE_26);
            mbGuideMotionShiftSet(lbl_1_data_0[0], 12, TRUE);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 14), 6);
            mbWinTopWait();
        } else {
            HuPrcSleep(60);
            for (vibratingPlayer = 0; vibratingPlayer < GW_PLAYER_MAX; vibratingPlayer++) {
                omVibrate(vibratingPlayer, 20, 7, 3);
            }
            mbAudFXPlay(MSM_SE_W06_TIME_CHANGE);
            WipeCreate(2, 130, 60);
            WipeColorSet(255, 255, 255);
            HuPrcSleep(90);
            GwPlayer[playerNo].masuId = spaceId;
            for (index = 0; index < 2; index++) {
                mbGuideKill(lbl_1_data_0[index]);
            }
            mbChangeTime();
            fn_1_C1CC();
        }
        break;
    case 1:
        mbAudFXPlay(MSM_SE_GUIDE_17);
        mbGuideMotionShiftSet(lbl_1_data_0[1], 12, TRUE);
        mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 15), 7);
        mbWinTopWait();
        for (index = 0; index < 2; index++) {
            mbAudFXPlay(index + MSM_SE_W06_GUIDE_MOTION);
            mbObjMotionShiftSet(guideModels[index], 10, (0.0f), (8.0f), 0);
        }
        if (GwSystem.curTime != 0) {
            HuPrcSleep(120);
            for (index = 0; index < 2; index++) {
                mbGuideMotionNextSet(lbl_1_data_0[index], 1);
            }
            mbAudFXPlay(MSM_SE_GUIDE_18);
            mbGuideMotionShiftSet(lbl_1_data_0[1], 12, TRUE);
            mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 16), 7);
            mbWinTopWait();
        } else {
            HuPrcSleep(60);
            for (vibratingPlayer = 0; vibratingPlayer < GW_PLAYER_MAX; vibratingPlayer++) {
                omVibrate(vibratingPlayer, 20, 7, 3);
            }
            mbAudFXPlay(MSM_SE_W06_TIME_CHANGE);
            WipeCreate(2, 131, 60);
            WipeColorSet(0, 0, 0);
            HuPrcSleep(90);
            GwPlayer[playerNo].masuId = spaceId;
            for (index = 0; index < 2; index++) {
                mbGuideKill(lbl_1_data_0[index]);
            }
            mbChangeTime();
            fn_1_C1CC();
        }
        break;
    }
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    mbGuideMotionShiftSet(lbl_1_data_0[mbGuideNoGet()], 7, TRUE);
    mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 17), speaker);
    mbWinTopWait();
    if (GwSystem.curTime == 0) {
        for (index = 1; index > -1; index--) {
            mbGuideEnd(lbl_1_data_0[index], TRUE);
        }
    } else {
        for (index = 0; index < 2; index++) {
            mbGuideEnd(lbl_1_data_0[index], TRUE);
        }
    }
    mbCameraMovePos(&savedCenter, &savedRotation, NULL, savedZoom, (-1.0f), 30);
    mbCameraMoveWait();
    mbCameraFocusPlayerSet(playerNo);
    previousSpace = entranceSpace;
    GwPlayer[playerNo].masuIdPrev = previousSpace;
    mbPlayerMasuMoveTo(playerNo, spaceId, TRUE);
    GwPlayer[playerNo].masuId = spaceId;
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[2], (0.0f));
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

/* Releases the three auxiliary models when the board object closes. */
void fn_1_49AC(void) {
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_1_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_1_OFFSET))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_2_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_2_OFFSET))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_3_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_3_OFFSET))));
    }
}

/* Creates the capsule machine models during board setup. */
void fn_1_4A40(void)
{
    s32 objectId;
    s32 objectType1;
    s32 objectType2;
    s32 objectType3;
    s32 objectType4;
    s32 currentTimeIsClear1;
    s32 currentTimeIsClear2;
    s32 currentTimeIsClear3;
    s32 currentTimeIsClear4;

    currentTimeIsClear1 = GwSystem.curTime == 0;
    if (currentTimeIsClear1 != 0) {
        objectType1 = DATA_w06;
    } else {
        objectType1 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType1 | 0x1D, NULL, 0);
    lbl_1_bss_404.objectIds[4] = objectId;
    mbObjMotionSpeedSet(objectId, (0.0f));
    mbObjMotionTimeSet(objectId, (0.0f));
    mbObjMotionStartEndSet(objectId, 0, 240);
    currentTimeIsClear2 = GwSystem.curTime == 0;
    if (currentTimeIsClear2 != 0) {
        objectType2 = DATA_w06;
    } else {
        objectType2 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType2 | 0x21, NULL, 0);
    lbl_1_bss_404.objectIds[5] = objectId;
    mbObjDispSet(objectId, 0);
    mbObjMotionTimeSet(objectId, (0.0f));
    mbObjMotionSpeedSet(objectId, (0.0f));
    mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    currentTimeIsClear3 = GwSystem.curTime == 0;
    if (currentTimeIsClear3 != 0) {
        objectType3 = DATA_w06;
    } else {
        objectType3 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType3 | 0x23, NULL, 0);
    lbl_1_bss_404.objectIds[6] = objectId;
    mbObjDispSet(objectId, 0);
    mbObjMotionTimeSet(objectId, (0.0f));
    mbObjMotionSpeedSet(objectId, (0.0f));
    mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
    mbObjHookSet((s16)lbl_1_bss_404.objectIds[5], "itemhook_c", objectId);
    currentTimeIsClear4 = GwSystem.curTime == 0;
    if (currentTimeIsClear4 != 0) {
        objectType4 = DATA_w06;
    } else {
        objectType4 = DATA_w06n;
    }
    objectId = mbObjCreate(objectType4 | 0x22, NULL, 0);
    lbl_1_bss_404.objectIds[7] = objectId;
    mbObjDispSet(objectId, 0);
    mbObjMotionTimeSet(objectId, (0.0f));
    mbObjMotionSpeedSet(objectId, (0.0f));
}

#include "game/process.h"
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/window.h"
#include "game/board/audio.h"
#include "game/board/capsule.h"
#include "game/board/comchoice.h"

extern f32 lbl_1_data_2D8[8];

int mbCapObjColorCreate(int capsuleNo, BOOL createF);
void mbCapObjColorAlphaSet(int id, u8 alpha);
void mbCapObjColorPosSetV(int id, HuVecF *pos);
void mbCapObjColorKill(int id);
int mbCapObjCreate(int capsuleNo, BOOL flag);
void mbCapObjKill(int objId);
int mbCapUseMesGet(int capsuleNo);
int mbCapDelete(int capsuleNo, BOOL repeatF);
void mbev_ScrollCapsule(int playerNo);

/* Rank-indexed chances used to choose the capsule machine's outcome. */
f32 lbl_1_data_2D8[8] = {
    75.0f, 80.0f, 85.0f, 90.0f,
    2.802596928649634e-45f, 4.203895392974451e-45f, 3.0828566215145976e-44f, 3.2229864679470793e-44f
};

/* Runs the capsule machine when a player lands there, with a timed choice and prize animation. */
void fn_1_4CC0(int playerNo, s16 spaceId)
{
    Point3d capsulePositions[5];
    Point3d capsuleVelocities[5];
    int motionIds[5];
    int capsuleModels[5];
    int capsuleAlpha[5];
    Point3d floorPosition;
    Point3d playerPosition;
    Point3d capsulePosition;
    Point3d playerVelocity;
    Point3d landingPosition;
    Point3d returnPosition;
    int playerRank;
    int randomCapsule;
    f32 elevation;
    f32 gravity;
    f32 azimuth;
    f32 stepHeight;
    f32 motionFactor;
    f32 capsuleScale;
    f32 originalZoom;
    f32 closeZoom;
    int index;
    int awardedCapsule;
    int removeIndex;
    int discardedCapsule;
    int capsuleCount;
    int helpWindow;
    int timeLeft;
    BOOL scatterInventory;
    s16 timerMessage;
    s32 currentSpace;
    s32 nextSpace;
    int awardObject;
    s32 previousSpace;

    awardObject = -1;
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 2), -1);
    mbWinTopWait();
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 3), -1);
    mbWinTopWait();
    mbWinCreateChoice(2, MESSNUM(MESS_BOARD_W06, 4), -1, 0);
    if (GwPlayer[playerNo].comF != FALSE) {
        mbComChoiceUpSet();
    }
    mbWinTopWait();
    if (mbWinTopChoiceGet() == 1 || mbWinTopChoiceGet() == -1) {
        return;
    }
    for (index = 0; index < 5; index++) {
        capsuleModels[index] = -1;
        capsuleAlpha[index] = 255;
    }
    capsuleCount = mbPlayerCapsuleNumGet(playerNo);
    for (index = 0; index < capsuleCount; index++) {
        capsuleModels[index] = mbCapObjColorCreate(mbPlayerCapsuleGet(playerNo, index), TRUE);
    }
    playerRank = GwPlayer[playerNo].rank;
    if ((f32)mbRandMod(100) < lbl_1_data_2D8[playerRank]) {
        scatterInventory = FALSE;
    } else {
        scatterInventory = TRUE;
    }
    if (scatterInventory != FALSE) {
        lbl_1_bss_64 = lbl_1_bss_404.objectIds[5];
        mbObjDispSet((s16)lbl_1_bss_404.objectIds[5], TRUE);
    } else {
        mbCapRandomListGet(&randomCapsule, 1);
        awardedCapsule = randomCapsule;
        lbl_1_bss_64 = awardObject = mbCapObjCreate(awardedCapsule, FALSE);
    }
    lbl_1_bss_8 = 1;
    currentSpace = spaceId;
    for (;;) {
        if (mbMasuLinkNumGet(currentSpace) == 0) {
            break;
        }
        nextSpace = mbMasuTypeFindLink(currentSpace, 0);
        previousSpace = currentSpace;
        GwPlayer[playerNo].masuIdPrev = previousSpace;
        mbPlayerMasuMoveTo(playerNo, nextSpace, TRUE);
        currentSpace = nextSpace;
    }
    mbMasuPosGet(nextSpace, &floorPosition);
    floorPosition.y -= (250.0);
    motionIds[0] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 8));
    motionIds[1] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 9));
    motionIds[2] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 25));
    motionIds[3] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 70));
    motionIds[4] = mbPlayerMotionCreate(playerNo, DATANUM(DATA_mariomot, 71));
    mbPlayerRotateStart(playerNo, -135, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    timeLeft = 600;
    timerMessage = GameMesCreate(1, 10, -1, -1);
    HuSprGrpDrawNoSet(GameMesGet(timerMessage)->grpId[0], 32);
    helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W06, 5));
    mbWinPosSet(helpWindow, 148, 376);
    for (;;) {
        if (timerMessage == -1) {
            if (helpWindow != -1) {
                mbWinKill(helpWindow);
                helpWindow = -1;
            }
            HuPadBtnDown[GwPlayer[playerNo].padNo] |= 0x100;
        }
        if (GwPlayer[playerNo].comF != FALSE) {
            HuPadBtnDown[GwPlayer[playerNo].padNo] |= 0x100;
        }
        if ((HuPadBtnDown[GwPlayer[playerNo].padNo] & 0x100) != 0) {
            break;
        }
        if (timeLeft >= 0) {
            timeLeft--;
            if (timeLeft >= 0) {
                GameMesDispSet(timerMessage, 1, (s16)((timeLeft + 59) / 60));
            } else if (timerMessage >= 0) {
                GameMesDispSet(timerMessage, 2, -1);
                timerMessage = -1;
            }
        }
        HuPrcVSleep();
    }
    if (helpWindow != -1) {
        mbWinKill(helpWindow);
        helpWindow = -1;
    }
    if (timerMessage >= 0) {
        GameMesDispSet(timerMessage, 2, -1);
        timerMessage = -1;
    }
    mbPlayerMotionShiftSet(playerNo, 4, (0.0f), (8.0f), 0);
    stepHeight = (33.33333206176758f);
    mbPlayerPosGet(playerNo, &playerPosition);
    for (index = 0; index < 6U; index++) {
        playerPosition.y += stepHeight;
        mbPlayerPosSetV(playerNo, &playerPosition);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, motionIds[0], (0.0f), (8.0f), 0);
    while (mbPlayerMotionEndCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, motionIds[1], (0.0f), (8.0f), 0);
    mbPlayerPosGet(playerNo, &playerPosition);
    stepHeight = (playerPosition.y - floorPosition.y) / (9.0f);
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[4], (1.5f));
    mbAudFXPlay(MSM_SE_W06_CAPSULE_DROP);
    omVibrate(playerNo, 20, 7, 3);
    for (index = 0; index < 9U; index++) {
        playerPosition.y -= stepHeight;
        mbPlayerPosSetV(playerNo, &playerPosition);
        HuPrcVSleep();
    }
    CharEffectHipDropCreate(playerNo, &playerPosition);
    HuPrcSleep(10);
    lbl_1_bss_8 = 0;
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (16.0f), 0);
    HuPrcSleep(30);
    mbPlayerMasuMoveTo(playerNo, previousSpace, TRUE);
    mbPlayerRotateStart(playerNo, 0, 15);
    HuPrcSleep(10);
    if (scatterInventory != FALSE) {
        mbPlayerMotionShiftSet(playerNo, motionIds[2], (0.0f), (8.0f), 0);
    }
    HuPrcSleep(20);
    mbPlayerPosGet(playerNo, &playerPosition);
    if (scatterInventory != FALSE) {
        mbPlayerMotionShiftSet(playerNo, motionIds[3], (0.0f), (8.0f), 0);
        mbObjDispSet((s16)lbl_1_bss_404.objectIds[5], TRUE);
        mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[5], (0.0f));
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[5], (1.0f));
        mbObjDispSet((s16)lbl_1_bss_404.objectIds[6], TRUE);
        mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[6], (0.0f));
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[6], (1.0f));
    }
    if (scatterInventory != FALSE) {
        for (index = 0; index < 40; index++) {
            motionFactor = (f32)index / (40.0f);
            capsulePosition.x = playerPosition.x;
            capsulePosition.y = (100.0f) + playerPosition.y + (1000.0) *
                cos((3.141592653589793) * ((90.0f) * motionFactor) / (180.0));
            capsulePosition.z = (50.0f) + playerPosition.z;
            mbObjPosSetV((s16)lbl_1_bss_64, &capsulePosition);
            HuPrcVSleep();
        }
        mbAudFXPlay(MSM_SE_GUIDE_05);
        mbObjPosSet((s16)lbl_1_bss_64, (0.0f), (0.0f), (0.0f));
        mbObjHookSet(mbPlayerObjIDGet(playerNo),
            CharModelItemHookGet(GwPlayer[playerNo].charNo, 2, 0), (s16)lbl_1_bss_64);
        mbPlayerMotionShiftSet(playerNo, motionIds[4], (0.0f), (8.0f), 0);
        originalZoom = mbCameraZoomGet();
        closeZoom = originalZoom - (800.0f);
        mbCameraMovePos(NULL, NULL, NULL, closeZoom,
            (-1.0f), 90);
        while (mbPlayerMotionEndCheck(playerNo) == FALSE) {
            HuPrcVSleep();
        }
        mbCameraShakeSet(50, (100.0f));
        omVibrate(playerNo, 20, 7, 3);
        mbAudFXPlay(MSM_SE_BRD00_68);
        mbObjHookReset(mbPlayerObjIDGet(playerNo));
        mbObjHookReset((s16)lbl_1_bss_64);
        mbObjDispSet((s16)lbl_1_bss_64, FALSE);
        mbObjDispSet((s16)lbl_1_bss_404.objectIds[6], FALSE);
        mbObjDispSet((s16)lbl_1_bss_404.objectIds[7], TRUE);
        mbObjPosSetV((s16)lbl_1_bss_404.objectIds[7], &capsulePosition);
        mbObjScaleSet((s16)lbl_1_bss_404.objectIds[7], (1.2000000476837158f),
            (1.2000000476837158f), (1.2000000476837158f));
        mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[7], (5.0f));
        mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[7], (1.0f));
        mbPlayerPosGet(playerNo, &playerPosition);
        mbPlayerPosGet(playerNo, &landingPosition);
        gravity = (7.840000152587891f);
        playerVelocity.x = playerVelocity.z = (0.0f);
        playerVelocity.y = (72.00000762939453f);
        mbPlayerMotionSet(playerNo, 9, 0);
        for (index = 0; index < capsuleCount; index++) {
            mbPlayerCapsuleRemove(playerNo, 0);
        }
        azimuth = (360.0f) * ((1.52587890625e-05f) * (f32)(u16)frand());
        for (index = 0; index < capsuleCount; index++) {
            azimuth += (360.0f) / (f32)capsuleCount +
                (10.0f) * ((1.52587890625e-05f) * (f32)(u16)frand());
            capsulePositions[index] = playerPosition;
            capsulePositions[index].y += (100.0f);
            elevation = (70.0f) + (15.0f) *
                ((1.52587890625e-05f) * (f32)(u16)frand());
            motionFactor =
                (1.2000000476837158f) *
                ((65.0f) * ((0.800000011920929f) +
                            (0.30000001192092896f) * ((1.52587890625e-05f) * (f32) (u16) frand())));
            capsuleVelocities[index].x = motionFactor *
                (sin((3.141592653589793) * azimuth / (180.0)) *
                    cos((3.141592653589793) * elevation / (180.0)));
            capsuleVelocities[index].z = motionFactor *
                (cos((3.141592653589793) * azimuth / (180.0)) *
                    cos((3.141592653589793) * elevation / (180.0)));
            capsuleVelocities[index].y = motionFactor *
                sin((3.141592653589793) * elevation / (180.0));
        }
        for (;;) {
            PSVECAdd(&playerPosition, &playerVelocity, &playerPosition);
            playerVelocity.y -= gravity;
            for (index = 0; index < capsuleCount; index++) {
                PSVECAdd(&capsulePositions[index], &capsuleVelocities[index],
                    &capsulePositions[index]);
                capsuleVelocities[index].y -= gravity;
                capsuleAlpha[index] -= 15;
                if (capsuleAlpha[index] <= 0) {
                    capsuleAlpha[index] = 0;
                }
                mbCapObjColorAlphaSet(capsuleModels[index], capsuleAlpha[index]);
                mbCapObjColorPosSetV(capsuleModels[index], &capsulePositions[index]);
            }
            if (playerPosition.y <= landingPosition.y) {
                playerPosition.y = landingPosition.y;
                if ((u32)mbPlayerMotionGet(playerNo) != HU3D_MOTATTR_LOOP) {
                    mbPlayerMotionShiftSet(playerNo, 6, (0.0f),
                        (8.0f), HU3D_MOTATTR_LOOP);
                }
                break;
            }
            mbPlayerPosSetV(playerNo, &playerPosition);
            HuPrcVSleep();
        }
        mbCameraShakeReset();
        mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 6), -1);
        mbWinTopWait();
    } else {
        capsuleScale = (1.0f);
        for (index = 0; index < 40; index++) {
            motionFactor = (f32)index / (40.0f);
            capsulePosition.x = playerPosition.x;
            capsulePosition.y = (100.0f) + playerPosition.y + (1000.0) *
                cos((3.141592653589793) * ((90.0f) * motionFactor) / (180.0));
            capsulePosition.z = (50.0f) + playerPosition.z;
            if (index >= 35) {
                capsuleScale -= (0.10000000149011612f);
            }
            mbObjPosSetV((s16)lbl_1_bss_64, &capsulePosition);
            mbObjScaleSet((s16)lbl_1_bss_64, capsuleScale, capsuleScale, capsuleScale);
            HuPrcVSleep();
        }
        mbObjDispSet((s16)lbl_1_bss_64, FALSE);
        discardedCapsule = -1;
        removeIndex = -1;
        if (mbPlayerCapsuleNumGet(playerNo) >= mbPlayerCapsuleMaxGet()) {
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 0), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(awardedCapsule), 0);
            mbWinTopWait();
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 1), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(awardedCapsule), 0);
            mbWinTopWait();
            do {
                discardedCapsule = mbCapDelete(awardedCapsule, TRUE);
                switch (discardedCapsule) {
                default:
                    if (discardedCapsule != awardedCapsule) {
                        for (index = 0; index < mbPlayerCapsuleMaxGet(); index++) {
                            if (discardedCapsule == mbPlayerCapsuleGet(playerNo, index)) {
                                removeIndex = index;
                            }
                        }
                        if (removeIndex != -1) {
                            mbPlayerCapsuleRemove(playerNo, removeIndex);
                        }
                    } else {
                        discardedCapsule = -1;
                        removeIndex = -2;
                    }
                    break;
                case -3:
                    mbev_ScrollCapsule(playerNo);
                    removeIndex = -1;
                    break;
                case -7:
                    removeIndex = -2;
                    break;
                }
            } while (removeIndex == -1);
            if (removeIndex >= 0) {
                mbPlayerCapsuleAdd(playerNo, awardedCapsule);
                mbPlayerWinLoseVoicePlay(playerNo, 12, MSM_SE_W06_CAPSULE_REWARD_VOICE);
                mbPlayerMotionShiftSet(playerNo, 12, (0.0f), (8.0f), 0);
                mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 3), -1);
                mbWinTopInsertMesSet(mbCapUseMesGet(discardedCapsule), 0);
                mbWinTopInsertMesSet(mbCapUseMesGet(awardedCapsule), 1);
                mbWinTopWait();
            } else {
                mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 4), -1);
                mbWinTopInsertMesSet(mbCapUseMesGet(awardedCapsule), 0);
                mbWinTopWait();
            }
        } else {
            mbPlayerCapsuleAdd(playerNo, awardedCapsule);
            mbPlayerWinLoseVoicePlay(playerNo, 12, MSM_SE_W06_CAPSULE_REWARD_VOICE);
            mbPlayerMotionShiftSet(playerNo, 12, (0.0f), (8.0f), 0);
            mbWinCreate(2, MESSNUM(MESS_CAPSULE_MASU, 0), -1);
            mbWinTopInsertMesSet(mbCapUseMesGet(awardedCapsule), 0);
            mbWinTopWait();
        }
    }
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (16.0f), HU3D_MOTATTR_LOOP);
    HuPrcSleep(30);
    mbMasuPosGet(spaceId, &returnPosition);
    mbWipeSpecialFadeInCreate(1, 60);
    mbPlayerPosSetV(playerNo, &returnPosition);
    GwPlayer[playerNo].masuId = spaceId;
    mbev_PlayerColMasu(playerNo, spaceId, TRUE);
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[4], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[4], (0.0f));
    mbCameraPlayerViewSetFast(playerNo, 1);
    mbWipeSpecialFadeOutCreate(1, 60);
    HuPrcVSleep();
    for (index = 0; index < 5; index++) {
        if (motionIds[index] != -1) {
            mbPlayerMotionKill(playerNo, motionIds[index]);
        }
    }
    for (index = 0; index < capsuleCount; index++) {
        mbCapObjColorKill(capsuleModels[index]);
    }
    if (awardObject != -1) {
        mbCapObjKill(awardObject);
    }
    lbl_1_bss_64 = -1;
}

#include "math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

/* Releases the capsule machine models when the board object closes. */
void fn_1_6308(void) {
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_4_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_4_OFFSET))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_5_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_5_OFFSET))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_6_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_6_OFFSET))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_7_OFFSET))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (W06_BOARD_OBJECT_ID_7_OFFSET))));
    }
}

/* Creates two hidden event models during board setup. */
void fn_1_63C4(void) {
    s32 objectId;
    s32 firstObjectType;
    s32 secondObjectType;
    s32 firstTime;
    s32 secondTime;

    firstTime = GwSystem.curTime == 0;
    if (firstTime != 0) {
        firstObjectType = DATA_w06;
    } else {
        firstObjectType = DATA_w06n;
    }
    objectId = mbObjCreate(firstObjectType | 0x1E, NULL, 0);
    lbl_1_bss_404.objectIds[8] = objectId;
    mbObjDispSet(objectId, 0);
    mbObjMotionTimeSet(objectId, (0.0f));
    mbObjMotionSpeedSet(objectId, (0.0f));

    secondTime = GwSystem.curTime == 0;
    if (secondTime != 0) {
        secondObjectType = DATA_w06;
    } else {
        secondObjectType = DATA_w06n;
    }
    objectId = mbObjCreate(secondObjectType | 0x1F, NULL, 0);
    lbl_1_bss_404.objectIds[9] = objectId;
    mbObjDispSet(objectId, 0);
}

#include "game/process.h"
#include "game/board/player.h"
#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/board/window.h"

int lbl_1_data_2F8[3] = {
    DATANUM(DATA_mario, 28), DATANUM(DATA_mariomot, 23), DATANUM(DATA_mariomot, 34)
};

/* Runs the UFO lift and landing animation when the player uses the board transport. */
void fn_1_64D0(int playerNo, s16 spaceId)
{
    Mtx hookMatrix;
    int motionIds[3];
    Point3d startPosition;
    Point3d landingPosition;
    Point3d rotation;
    f32 originalZoom;
    f32 expandedZoom;
    f32 motionTime;
    f32 progress;
    int frame;
    int motionSound;
    int liftSound;
    s32 transportModel;
    s16 carriedObject;

    mbev_PlayerColMasuSet(playerNo, 3, TRUE);
    transportModel = mbObjModelIDGet((s16)lbl_1_bss_404.objectIds[8]);
    for (frame = 0; frame < 3; frame++) {
        motionIds[frame] = mbPlayerMotionCreate(playerNo, lbl_1_data_2F8[frame]);
    }
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    HuPrcSleep(10);
    originalZoom = mbCameraZoomGet();
    expandedZoom = (1000.0f) + originalZoom;
    mbCameraMovePos(NULL, NULL, NULL, expandedZoom, (-1.0f), 90);
    mbObjDispSet((s16)lbl_1_bss_404.objectIds[8], TRUE);
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (2.0f));
    motionSound = mbAudFXPlay(MSM_SE_W06_TRANSPORT_MOTION);
    for (;;) {
        motionTime = mbObjMotionTimeGet((s16)lbl_1_bss_404.objectIds[8]);
        if (motionTime >= (418.0f)) {
            break;
        }
        mbPlayerMotionShiftSet(playerNo, motionIds[1], (0.0f),
            (8.0f), HU3D_MOTATTR_LOOP);
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    mbAudFXStop(motionSound);
    mbPlayerMotionShiftSet(playerNo, 9, (0.0f), (8.0f), 0);
    while (mbPlayerMotionEndCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    Hu3DModelObjMtxGet(transportModel, "ufo_fook", hookMatrix);
    mbObjDispSet((s16)lbl_1_bss_404.objectIds[9], TRUE);
    mbObjPosSet((s16)lbl_1_bss_404.objectIds[9], hookMatrix[0][3], hookMatrix[1][3],
        hookMatrix[2][3]);
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[9], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[9], (1.0f));
    carriedObject = (s16)lbl_1_bss_404.objectIds[9];
    mbObjAttrSet(carriedObject, HU3D_MOTATTR_LOOP);
    liftSound = mbAudFXPlay(MSM_SE_W06_TRANSPORT_LIFT);
    HuPrcSleep(10);
    omVibrate(playerNo, 450, 4, 4);
    mbPlayerMotionShiftSet(playerNo, motionIds[2], (0.0f),
        (8.0f), HU3D_MOTATTR_LOOP);
    lbl_1_bss_10 = 1;
    mbPlayerPosGet(playerNo, &startPosition);
    for (frame = 0; (f32)frame < (75.0f); frame++) {
        progress = (f32)frame / (75.0f);
        mbPlayerPosSet(playerNo, startPosition.x,
            startPosition.y + (300.0) *
                sin((3.141592653589793) * ((90.0f) * progress) / (180.0)),
            startPosition.z);
        HuPrcVSleep();
    }
    mbAudFXStop(liftSound);
    motionSound = mbAudFXPlay(MSM_SE_W06_TRANSPORT_MOTION);
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (2.0f));
    for (;;) {
        motionTime = mbObjMotionTimeGet((s16)lbl_1_bss_404.objectIds[8]);
        if (motionTime >= (726.0f)) {
            break;
        }
        Hu3DModelObjMtxGet(transportModel, "ufo_fook", hookMatrix);
        mbPlayerPosSet(playerNo, hookMatrix[0][3], hookMatrix[1][3] - (300.0),
            hookMatrix[2][3]);
        mbObjPosSet((s16)lbl_1_bss_404.objectIds[9], hookMatrix[0][3], hookMatrix[1][3],
            hookMatrix[2][3]);
        HuPrcVSleep();
    }
    mbAudFXStop(motionSound);
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    lbl_1_bss_10 = 0;
    mbPlayerRotGet(playerNo, &rotation);
    for (;;) {
        if (rotation.y >= (360.0f)) {
            break;
        }
        rotation.y += (3.0f);
        mbPlayerRotSetV(playerNo, &rotation);
        HuPrcVSleep();
    }
    rotation.y = (0.0f);
    mbPlayerRotSetV(playerNo, &rotation);
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[9], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[9], (1.0f));
    mbObjDispSet((s16)lbl_1_bss_404.objectIds[9], FALSE);
    mbPlayerPosGet(playerNo, &landingPosition);
    omVibrate(playerNo, 20, 7, 3);
    for (frame = 0; (f32)frame < (15.000000953674316f); frame++) {
        progress = (f32)frame / (15.000000953674316f);
        mbPlayerPosSet(playerNo, landingPosition.x,
            landingPosition.y - (300.0) *
                sin((3.141592653589793) * ((90.0f) * progress) / (180.0)),
            landingPosition.z);
        HuPrcVSleep();
    }
    CharEffectHipDropCreate(playerNo, &startPosition);
    mbPlayerPosGet(playerNo, &landingPosition);
    for (frame = 0; (f32)frame < (15.000000953674316f); frame++) {
        progress = (f32)frame / (15.000000953674316f);
        mbPlayerPosSet(playerNo, landingPosition.x,
            landingPosition.y + (100.0) *
                sin((3.141592653589793) * ((180.0f) * progress) / (180.0)),
            landingPosition.z);
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 6, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    motionSound = mbAudFXPlay(MSM_SE_W06_TRANSPORT_MOTION);
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (2.0f));
    while (mbObjMotionEndCheck((s16)lbl_1_bss_404.objectIds[8]) == FALSE) {
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    mbObjDispSet((s16)lbl_1_bss_404.objectIds[8], FALSE);
    mbAudFXStop(motionSound);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 37), -1);
    mbWinTopWait();
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    mbCameraMovePos(NULL, NULL, NULL, originalZoom, (-1.0f), 30);
    mbCameraMoveWait();
    for (frame = 0; frame < 3; frame++) {
        mbPlayerMotionKill(playerNo, motionIds[frame]);
    }
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[8], (0.0f));
    mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[9], (0.0f));
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[9], (0.0f));
    GwPlayer[playerNo].masuId = 3;
}

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

s8 lbl_1_data_30D[7] = {12, 6, 7, -1, 0, 0, 0};

/* Releases the two transport models when the board object closes. */
void fn_1_6F38(void) {
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (96))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (96))));
    }
    if ((s32) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (100))) != -1) {
        mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_404) + (100))));
    }
}

/* Creates the hidden sprite models used by later board events during setup. */
void fn_1_6FA4(void) {
    s32 objectIndex;
    s32 loopObjectType;
    s32 finalObjectType;
    s32 loopUsesFirstResources;
    s32 finalUsesFirstResources;
    s32 objectId;

    objectIndex = 0;
    while (objectIndex < 2) {
        loopUsesFirstResources = GwSystem.curTime == 0;
        if (loopUsesFirstResources != 0) {
            loopObjectType = DATA_w06;
        } else {
            loopObjectType = DATA_w06n;
        }
        objectId =
            mbObjCreate((u16) (objectIndex + DATANUM(DATA_w06, 38)) | loopObjectType, NULL, 0);
        lbl_1_bss_404.spriteObjectIds[objectIndex] = objectId;
        mbObjDispSet(objectId, 0);
        mbObjMotionTimeSet(objectId, 0.0f);
        mbObjMotionSpeedSet(objectId, 0.0f);
        objectIndex++;
    }
    finalUsesFirstResources = GwSystem.curTime == 0;
    if (finalUsesFirstResources != 0) {
        finalObjectType = DATA_w06;
    } else {
        finalObjectType = DATA_w06n;
    }
    objectId = mbObjCreate(finalObjectType | 0x25, NULL, 0);
    lbl_1_bss_404.spriteObjectIds[2] = objectId;
    mbObjDispSet(objectId, 0);
    mbObjMotionTimeSet(objectId, 0.0f);
    mbObjMotionSpeedSet(objectId, 0.0f);
}

#include "game/process.h"
#include "game/board/main.h"
#include "game/board/player.h"
#include "game/board/masu.h"
#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/board/window.h"
#include "game/board/effect.h"
#include "game/board/coin.h"
#include "game/esprite.h"

extern s8 lbl_1_data_30D[7];
extern Point3d lbl_1_data_314[4];
extern Point3d lbl_1_data_344[2];
extern Point3d lbl_1_data_35C[2];

u32 mbRandMod(u32 max);
void mbGuideMotionSet(OMOBJ *obj, s16 motionNo, BOOL shiftF);
BOOL mbGuideMotionCheck(OMOBJ *obj);
void mbGuideFadeIn(OMOBJ *obj);
int mbStarObjCreate(void);
void mbStarObjPosSet(int objNo, float x, float y, float z);
void mbStarObjPosGet(int objNo, HuVecF *pos);
void mbStarObjRotSet(int objNo, float x, float y, float z);
void mbStarObjScaleSet(int objNo, float x, float y, float z);
void mbStarChestCreate(int objNo, int playerNo);
s16 mbCoinDispCapsuleCreate(HuVecF *pos, int coinNum);
s8 mbPadStkXGet(s32 padNo);
s8 mbPadStkYGet(s32 padNo);
float mbCosDeg(float angle);
float mbSinDeg(float angle);
void mbWipeFadeOut(void);
void mbWipeFadeIn(void);
void fn_1_D4C8(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix);

/* Offers two chests on the active side of the board, then awards a star or ten coins. */
void fn_1_7100(int playerNo, s16 spaceId)
{
    Point3d coinVelocities[10];
    Point3d coinPositions[10];
    s32 coinModels[10];
    s32 coinDelays[10];
    BOOL coinLanded[10];
    f32 coinRotations[10];
    Point3d chestPositions[2];
    char coinText[16];
    Point3d playerPosition;
    Point3d cursorDestination;
    Point3d cursorWorldPosition;
    Point3d cursorPosition;
    Point3d starPosition;
    Point3d coinDisplayPosition;
    Point3d particlePosition;
    Point3d guidePosition;
    Point3d chestVelocity;
    Point3d cameraPosition;
    Point3d coinOrigin;
    Point3d coinEffectPosition;
    Point3d savedCameraRotation;
    s32 stickY;
    s32 buttons;
    s32 computerDelay;
    s32 computerMoves;
    s32 timeLeft;
    s32 modelId;
    s32 starObject;
    s32 coinCount;
    u32 prizeRoll;
    s32 coinDisplay;
    s32 helpWindow;
    s16 particles[2];
    ANIMDATA *animation;
    u32 linkedAttributes;
    HU3D_MODEL *guideModelData;
    HSF_DATA *guideHsf;
    s32 starSound;
    s16 previousSpace;
    s16 chestSpace;
    s16 guideModel;
    s16 retiredCoinModel;
    s16 createdCoinModel;
    s32 cursorSprite;
    OMOBJ *guide;
    s32 repeatDelay;
    int stickX;
    HSF_MATERIAL *material;
    f32 progress;
    f32 alpha;
    f32 scale;
    f32 guideRotation;
    f32 progressStep;
    s32 index;
    s32 coinIndex;
    s32 selectedChest;
    s32 prize;
    MASU *space;
    s32 speaker;
    s32 messageOffset;
    s16 timerMessage;
    u32 attributes;

    attributes = mbMasuMAttrGet(spaceId);
    if (GwSystem.curTime == 0) {
        speaker = 6;
        messageOffset = 0;
    } else {
        speaker = 7;
        messageOffset = 1;
    }
    if ((attributes & 0x100) != 0) {
        mbPlayerRotateStart(playerNo, 125, 15);
        while (mbPlayerRotateCheck(playerNo) == FALSE) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        if (GwSystem.curTime != 0) {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 20), -1);
            mbWinTopWait();
            mbPlayerWinLoseVoicePlay(playerNo, 13, 585);
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
            while (mbPlayerMotionEndCheck(playerNo) == FALSE) {
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            HuPrcSleep(10);
        } else {
            goto openChests;
        }
    } else {
        mbPlayerRotateStart(playerNo, -125, 15);
        while (mbPlayerRotateCheck(playerNo) == FALSE) {
            HuPrcVSleep();
        }
        mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        if (GwSystem.curTime == 0) {
            mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 22), -1);
            mbWinTopWait();
            mbPlayerWinLoseVoicePlay(playerNo, 13, 585);
            mbPlayerMotionShiftSet(playerNo, 13, 0.0f, 8.0f, 0);
            while (mbPlayerMotionEndCheck(playerNo) == FALSE) {
                HuPrcVSleep();
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            HuPrcSleep(10);
        } else {
openChests:
            prizeRoll = mbRandMod(10);
            if ((s32)prizeRoll <= 2) {
                prize = 0;
            } else {
                prize = 1;
            }
            if ((attributes & 0x100) != 0) {
                guidePosition = lbl_1_data_344[0];
            } else {
                guidePosition = lbl_1_data_344[1];
            }
                cursorSprite = espEntry(mbBoardDataNumGet(W06_BOARD_CURSOR_DATA_ID), 1024, 0);
            espDispOff(cursorSprite);
            espDrawNoSet(cursorSprite, 32);
            espAttrSet(cursorSprite, 1);
            espBankSet(cursorSprite, 0);
            space = mbMasuGet(spaceId);
            for (index = 0; index < space->linkNum; index++) {
                linkedAttributes = mbMasuMAttrGet(space->linkTbl[index]);
                if ((linkedAttributes & 0x2000) != 0) {
                    chestSpace = space->linkTbl[index];
                }
            }
            mbMasuPosGet(chestSpace, &playerPosition);
            cameraPosition.x = (guidePosition.x + playerPosition.x) / 2.0f;
            cameraPosition.y = (guidePosition.y + playerPosition.y) / 2.0f;
            cameraPosition.z = (guidePosition.z + playerPosition.z) / 2.0f;
            cameraPosition.y += 100.0f;
            mbCameraFocusPlayerSet(-1);
            mbCameraMovePos(&cameraPosition, NULL, NULL, 1600.0f, -1.0f, 30);
            previousSpace = spaceId;
            GwPlayer[playerNo].masuIdPrev = previousSpace;
            mbPlayerMasuMoveTo(playerNo, chestSpace, TRUE);
            GwPlayer[playerNo].masuId = chestSpace;
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            HuPrcSleep(30);
            if ((attributes & 0x100) != 0) {
                guide = mbGuideCreate(0, &guidePosition, &lbl_1_data_35C[0],
                    lbl_1_data_30D, 1.0f, 2);
                mbGuideMotionNextSet(guide, 1);
                guideModel = mbGuideModelGet(guide);
                mbAudGuidePlay(MSM_SE_GUIDE_25);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 21), speaker);
                mbWinTopWait();
            } else {
                guide = mbGuideCreate(1, &guidePosition, &lbl_1_data_35C[1],
                    lbl_1_data_30D, 1.0f, 0);
                guideModel = mbGuideModelGet(guide);
                modelId = mbObjModelIDGet(guideModel);
                guideModelData = &Hu3DData[modelId];
                guideHsf = guideModelData->hsf;
                material = guideHsf->material;
                guideModelData->hiliteIdx = 0;
                for (coinIndex = 0; coinIndex < guideHsf->materialNum; coinIndex++, material++) {
                    material->litColor[0] = 192;
                    material->litColor[1] = 192;
                    material->litColor[2] = 192;
                }
                mbGuideFadeIn(guide);
                mbGuideMotionNextSet(guide, 1);
                mbAudGuidePlay(MSM_SE_GUIDE_25);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 23), speaker);
                mbWinTopWait();
            }
            mbAudGuidePlay(MSM_SE_GUIDE_26);
            mbGuideMotionShiftSet(guide, 12, TRUE);
            mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 24), speaker);
            mbWinTopWait();
            while (mbGuideMotionCheck(guide) == FALSE) {
                HuPrcVSleep();
            }
            mbObjMotionShiftSet(guideModel, 6, 0.0f, 8.0f, 0);
            HuPrcSleep(60);
            animation = HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
            mbPlayerPosGet(playerNo, &playerPosition);
            for (index = 0; index < 2; index++) {
                if ((attributes & 0x100) != 0) {
                    chestPositions[index] = lbl_1_data_314[index];
                    guideRotation = -60.0f;
                } else {
                    chestPositions[index] = lbl_1_data_314[index + 2];
                    guideRotation = 60.0f;
                }
                mbObjDispSet(lbl_1_bss_404.spriteObjectIds[index], TRUE);
                mbObjPosSetV(lbl_1_bss_404.spriteObjectIds[index], &chestPositions[index]);
                mbObjRotSet(lbl_1_bss_404.spriteObjectIds[index],
                    0.0f, guideRotation, 0.0f);
                mbObjScaleSet(lbl_1_bss_404.spriteObjectIds[index],
                    0.5f, 0.5f, 0.5f);
                mbObjAlphaSet(lbl_1_bss_404.spriteObjectIds[index], 255);
                mbObjMotionTimeSet(lbl_1_bss_404.spriteObjectIds[index], 0.0f);
                mbObjMotionSpeedSet(lbl_1_bss_404.spriteObjectIds[index], 0.0f);
                particles[index] = mbParticleCreate(animation, 128);
                mbParticleHookSet(particles[index], fn_1_D4C8);
                Hu3DModelLayerSet(particles[index], 5);
                particlePosition = chestPositions[index];
                particlePosition.y += 30.0;
                Hu3DModelPosSetV(particles[index], &particlePosition);
                mbAudFXPlay(MSM_SE_BRD00_45);
            }
            HuPrcSleep(30);
            mbAudGuidePlay(MSM_SE_GUIDE_25);
            mbGuideMotionShiftSet(guide, 12, TRUE);
            mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 26), speaker);
            mbWinTopWait();
            for (index = 0; index < 2; index++) {
                mbParticleKill(particles[index]);
                particles[index] = -1;
            }
            computerDelay = mbRandMod(20) + 30;
            computerMoves = 0;
            selectedChest = 0;
            repeatDelay = 0;
            timeLeft = 600;
            timerMessage = GameMesCreate(1, 10, -1, -1);
            HuSprGrpDrawNoSet(GameMesGet(timerMessage)->grpId[0], 32);
            espDispOn(cursorSprite);
            cursorWorldPosition = chestPositions[selectedChest];
            cursorWorldPosition.x -= 50.0;
            cursorWorldPosition.y += 50.0;
            Hu3D3Dto2D(&cursorWorldPosition, 1, &cursorPosition);
            espPosSet(cursorSprite, cursorPosition.x, cursorPosition.y);
            helpWindow = mbWinCreateHelp(MESSNUM(MESS_BOARD_W06, 36));
            mbWinPosSet(helpWindow, 228, 360);
            for (;;) {
                if (timerMessage == -1) {
                    break;
                }
                if (GwPlayer[playerNo].comF == FALSE) {
                    if (repeatDelay <= 0) {
                        stickX = mbPadStkXGet(GwPlayer[playerNo].padNo);
                        stickY = mbPadStkYGet(GwPlayer[playerNo].padNo);
                        buttons = HuPadBtnDown[GwPlayer[playerNo].padNo];
                    } else {
                        stickX = stickY = buttons = 0;
                        repeatDelay--;
                    }
                    if ((buttons & 0x100) != 0) {
                        mbAudFXPlay(2);
                        break;
                    }
                } else if (repeatDelay <= 0 && computerDelay-- <= 0) {
                    if (0.000015258789f * (f32)(u16)frand() >= 0.5f &&
                        computerMoves < 3) {
                        computerDelay = mbRandMod(20) + 20;
                        computerMoves++;
                        if (selectedChest <= 0) {
                            stickX = 10;
                        } else if (selectedChest >= 1) {
                            stickX = -10;
                        } else if (0.000015258789f * (f32)(u16)frand() >= 0.3f) {
                            stickX = 10;
                        } else {
                            stickX = -10;
                        }
                    } else {
                        /* The confirm mask is evaluated without storing it before leaving. */
                        buttons | 0x100;
                        mbAudFXPlay(2);
                        break;
                    }
                } else {
                    stickX = stickY = buttons = 0;
                    repeatDelay--;
                }
                if (stickX < -8 && selectedChest > 0) {
                    selectedChest--;
                    repeatDelay = 16;
                    mbAudFXPlay(0);
                } else if (stickX > 8 && selectedChest < 1) {
                    selectedChest++;
                    repeatDelay = 16;
                    mbAudFXPlay(0);
                }
                if (selectedChest < 0) {
                    selectedChest = 0;
                } else if (selectedChest > 1) {
                    selectedChest = 1;
                }
                cursorWorldPosition = chestPositions[selectedChest];
                cursorWorldPosition.x -= 50.0;
                cursorWorldPosition.y += 50.0;
                Hu3D3Dto2D(&cursorWorldPosition, 1, &cursorDestination);
                cursorPosition.x += 0.1f *
                    (cursorDestination.x - cursorPosition.x);
                cursorPosition.y += 0.1f *
                    (cursorDestination.y - cursorPosition.y);
                espPosSet(cursorSprite, cursorPosition.x, cursorPosition.y);
                if (timeLeft >= 0) {
                    timeLeft--;
                    if (timeLeft >= 0) {
                        GameMesDispSet(timerMessage, 1, (s16)((timeLeft + 59) / 60));
                    } else if (timerMessage >= 0) {
                        GameMesDispSet(timerMessage, 2, -1);
                        timerMessage = -1;
                    }
                }
                HuPrcVSleep();
            }
            if (timerMessage >= 0) {
                GameMesDispSet(timerMessage, 2, -1);
                timerMessage = -1;
            }
            espDispOff(cursorSprite);
            mbWinKill(helpWindow);
            HuPrcSleep(30);
            for (index = 0; index < 2; index++) {
                if (selectedChest != index) {
                    animation =
                        HuSprAnimRead(HuDataReadNum(DATANUM(DATA_effect, 3), HU_MEMNUM_OVL));
                    particles[index] = mbParticleCreate(animation, 128);
                    mbParticleHookSet(particles[index], fn_1_D4C8);
                    Hu3DModelLayerSet(particles[index], 5);
                    particlePosition = chestPositions[index];
                    particlePosition.y += 30.0;
                    Hu3DModelPosSetV(particles[index], &particlePosition);
                }
            }
            chestVelocity.x = (playerPosition.x - chestPositions[selectedChest].x) /
                35.0f;
            chestVelocity.y = (240.00002f + playerPosition.y -
                chestPositions[selectedChest].y) / 35.0f;
            chestVelocity.z = (playerPosition.z - chestPositions[selectedChest].z) /
                35.0f;
            mbAudFXPlay(MSM_SE_BRD00_45);
            for (index = 0; (f32)index < 35.0f; index++) {
                progress = (f32)index / 35.0f;
                alpha = 255.0f - 2.0f * (255.0f * progress);
                if (alpha <= 0.0f) {
                    alpha = 0.0f;
                }
                for (coinIndex = 0; coinIndex < 2; coinIndex++) {
                    if (coinIndex != selectedChest) {
                        mbObjAlphaSet(lbl_1_bss_404.spriteObjectIds[coinIndex], (s32)alpha);
                    } else {
                        chestPositions[coinIndex].x += chestVelocity.x;
                        chestPositions[coinIndex].y += chestVelocity.y;
                        chestPositions[coinIndex].z += chestVelocity.z;
                        mbObjPosSetV(lbl_1_bss_404.spriteObjectIds[coinIndex],
                            &chestPositions[coinIndex]);
                    }
                }
                HuPrcVSleep();
            }
            for (index = 0; index < 2; index++) {
                if (selectedChest != index) {
                    mbObjDispSet(lbl_1_bss_404.spriteObjectIds[index], FALSE);
                }
            }
            HuPrcSleep(20);
            switch (prize) {
            case 0:
                starObject = mbStarObjCreate();
                starPosition = chestPositions[selectedChest];
                break;
            case 1:
                for (index = 0; index < 10; index++) {
                    createdCoinModel =
                        mbObjCreate(mbBoardDataNumGet(W06_BOARD_COIN_DATA_ID), NULL, TRUE);
                    coinModels[index] = createdCoinModel;
                    coinPositions[index] = chestPositions[selectedChest];
                    coinDelays[index] = index * 2;
                    coinVelocities[index].x = 100.0f *
                        mbCosDeg((f32)mbRandMod(180)) / 45.0f;
                    coinVelocities[index].y = 0.15f * (100.0f +
                        100.0f * mbSinDeg((f32)mbRandMod(90)));
                    coinVelocities[index].z = 0.0f;
                    coinRotations[index] = 360.0f * mbCosDeg((f32)mbRandMod(90));
                    coinLanded[index] = FALSE;
                    mbObjDispSet(coinModels[index], FALSE);
                    mbObjScaleSet(coinModels[index],
                        0.75f, 0.75f, 0.75f);
                    mbObjRotYSet(coinModels[index], coinRotations[index]);
                }
                break;
            }
            mbPlayerMotionSet(playerNo, 11, 0);
            HuPrcSleep(24);
            mbAudFXPlay(MSM_SE_BRD00_12);
            omVibrate(playerNo, 20, 7, 3);
            mbObjMotionTimeSet(lbl_1_bss_404.spriteObjectIds[selectedChest], 0.0f);
            mbObjMotionSpeedSet(lbl_1_bss_404.spriteObjectIds[selectedChest], 3.0f);
            switch (prize) {
            case 0:
                starSound = mbAudFXPlay(MSM_SE_BRD00_91);
                mbAudFXPlay(MSM_SE_BRD00_92);
                for (index = 0; (f32)index <= 50.0f; index++) {
                    progress = (f32)index / 50.0f;
                    scale = 0.5f + 0.5f * progress;
                    mbStarObjPosSet(starObject, starPosition.x,
                        starPosition.y + 100.0 * (3.0 *
                            sin(3.141592653589793 * (170.0f * progress) /
                                180.0)), starPosition.z);
                    mbStarObjRotSet(starObject, 0.0f,
                        720.0f * progress, 0.0f);
                    mbStarObjScaleSet(starObject, scale, scale, scale);
                    if (progress <= 0.5f) {
                        mbObjPosSet(lbl_1_bss_404.spriteObjectIds[selectedChest],
                            chestPositions[selectedChest].x,
                            chestPositions[selectedChest].y + 100.0 *
                                sin(3.141592653589793 *
                                    (180.0f * (2.0f * progress)) /
                                    180.0), chestPositions[selectedChest].z);
                    } else {
                        alpha = 255.0f - 255.0f * (2.0f * progress);
                        mbObjAlphaSet(lbl_1_bss_404.spriteObjectIds[selectedChest],
                            (s32)alpha);
                    }
                    HuPrcVSleep();
                }
                mbAudFXStop(starSound);
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                break;
            case 1:
                mbPlayerPosGet(playerNo, &coinOrigin);
                progressStep = 0.02f;
                index = 0;
                progress = 0.0f;
                alpha = 255.0f;
                while (index < 10) {
                    progress += progressStep;
                    if (progress >= 1.0f) {
                        /* This comparison leaves the animation progress unchanged. */
                        progress == 1.0f;
                    }
                    scale = 0.5f + 0.5f * progress;
                    for (coinIndex = 0; coinIndex < 10; coinIndex++) {
                        if (--coinDelays[coinIndex] <= 0 && coinLanded[coinIndex] == FALSE) {
                            if (coinDelays[coinIndex] == 0) {
                                mbObjPosGet(lbl_1_bss_404.spriteObjectIds[selectedChest],
                                    &coinPositions[coinIndex]);
                            }
                            mbObjDispSet(coinModels[coinIndex], TRUE);
                            coinVelocities[coinIndex].y += -1.6333334f;
                            coinPositions[coinIndex].x += coinVelocities[coinIndex].x;
                            coinPositions[coinIndex].y += coinVelocities[coinIndex].y;
                            coinPositions[coinIndex].z += coinVelocities[coinIndex].z;
                            coinRotations[coinIndex] += 5.0f;
                            if (coinPositions[coinIndex].y <= 150.0f + coinOrigin.y) {
                                mbAudFXPlay(7);
                                coinEffectPosition = coinOrigin;
                                coinEffectPosition.y += 150.0f;
                                mbCoinEffCreate(&coinEffectPosition);
                                mbObjDispSet(coinModels[coinIndex], FALSE);
                                coinLanded[coinIndex] = TRUE;
                                mbPlayerCoinAdd(playerNo, 1);
                                index++;
                            }
                            mbObjPosSetV(coinModels[coinIndex], &coinPositions[coinIndex]);
                            mbObjRotYSet(coinModels[coinIndex], coinRotations[coinIndex]);
                        }
                    }
                    if (progress <= 0.5f) {
                        mbObjPosSet(lbl_1_bss_404.spriteObjectIds[selectedChest],
                            chestPositions[selectedChest].x,
                            chestPositions[selectedChest].y + 100.0 *
                                sin(3.141592653589793 *
                                    (180.0f * (2.0f * progress)) /
                                    180.0), chestPositions[selectedChest].z);
                    }
                    alpha = 255.0f -
                        255.0f * (2.0f * progress);
                    if (alpha < 0.0f) {
                        alpha = 0.0f;
                    }
                    mbObjAlphaSet(lbl_1_bss_404.spriteObjectIds[selectedChest], (s32)alpha);
                    HuPrcVSleep();
                }
                mbAudFXPlay(15);
                coinCount = 10;
                mbPlayerPosGet(playerNo, &coinDisplayPosition);
                coinDisplayPosition.y += 250.0f;
                coinDisplay = mbCoinDispCapsuleCreate(&coinDisplayPosition, coinCount);
                mbPlayerWinLoseVoicePlay(playerNo, 12, MSM_SE_W06_CAPSULE_REWARD_VOICE);
                mbPlayerMotionShiftSet(playerNo, 12, 0.0f, 8.0f, 0);
                while (mbPlayerMotionEndCheck(playerNo) == FALSE &&
                    mbCoinDispKillCheck(coinDisplay) == FALSE) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                break;
            }
            mbObjAlphaSet(lbl_1_bss_404.spriteObjectIds[selectedChest], 0);
            HuPrcSleep(10);
            mbAudGuidePlay(MSM_SE_GUIDE_25);
            mbGuideMotionShiftSet(guide, 12, TRUE);
            mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 28), speaker);
            mbWinTopWait();
            mbAudGuidePlay(MSM_SE_GUIDE_26);
            switch (prize) {
            case 0:
                mbStarObjPosGet(starObject, &starPosition);
                mbGuideMotionShiftSet(guide, 12, TRUE);
                mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 32), speaker);
                mbWinTopWait();
                break;
            case 1:
                sprintf(coinText, "%d", coinCount);
                mbGuideMotionShiftSet(guide, 12, TRUE);
            mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 30), speaker);
                mbWinTopInsertMesSet((u32)coinText, 0);
                mbWinTopWait();
                break;
            }
            HuPrcSleep(30);
            switch (prize) {
            case 0:
                mbCameraRotGet(&savedCameraRotation);
                mbCameraMovePlayer(playerNo, NULL, NULL, -1.0f, -1.0f, 30);
                mbPlayerRotateStart(playerNo, 0, 15);
                while (mbPlayerRotateCheck(playerNo) == FALSE) {
                    HuPrcVSleep();
                }
                mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                mbCameraMoveWait();
                mbStarChestCreate(starObject, playerNo);
                mbWipeFadeOut();
                mbPlayerMotionSet(playerNo, 1, HU3D_MOTATTR_LOOP);
                if ((attributes & 0x100) != 0) {
                    mbPlayerRotYSet(playerNo, 125.0f);
                } else {
                    mbPlayerRotYSet(playerNo, -125.0f);
                }
                mbCameraFocusPlayerSet(-1);
                mbCameraMovePos(&cameraPosition, &savedCameraRotation, NULL,
                    1600.0f, -1.0f, 1);
                mbWipeFadeIn();
                mbMusBoardPlay();
                break;
            }
            mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            mbAudGuidePlay(MSM_SE_GUIDE_25);
            mbGuideMotionSet(guide, 7, TRUE);
            mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 34), speaker);
            mbWinTopWait();
            if ((attributes & 0x100) != 0) {
                mbGuideEnd(guide, TRUE);
            } else {
                mbGuideEnd(guide, TRUE);
            }
            mbCameraFocusPlayerSet(playerNo);
            previousSpace = chestSpace;
            GwPlayer[playerNo].masuIdPrev = previousSpace;
            mbPlayerMasuMoveTo(playerNo, spaceId, TRUE);
            GwPlayer[playerNo].masuId = spaceId;
            espKill(cursorSprite);
            for (index = 0; index < 2; index++) {
                if (particles[index] != -1) {
                    mbParticleKill(particles[index]);
                }
                particles[index] = -1;
            }
            switch (prize) {
            case 0:
                break;
            case 1:
                for (index = 0; index < 10; index++) {
                    retiredCoinModel = coinModels[index];
                    mbObjKill((MBMODELID)retiredCoinModel);
                }
                break;
            }
HuDataDirClose(DATA_effect);
        }
    }
    mbPlayerRotateStart(playerNo, 0, 15);
    while (mbPlayerRotateCheck(playerNo) == FALSE) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerNo, 1, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
}

Point3d lbl_1_data_314[4] = { { 2375.0f, 125.0f, 607.0f },
                              { 2521.0f, 120.0f, 863.0f },
                              { -2405.0f, 459.0f, 809.0f },
                              { -2224.0f, 459.0f, 488.0f } };
Point3d lbl_1_data_344[2] = {{2486.0f, 126.0f, 712.0f}, {-2444.0f, 508.0f, 570.0f}};
Point3d lbl_1_data_35C[2] = {{0.0f, -60.0f, 0.0f}, {0.0f, 30.0f, 0.0f}};

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"

#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

/* Releases the board's three sprite models when its object-manager callback closes the board. */
void fn_1_92C8(void) {
    s32 spriteIndex;

    spriteIndex = 0;
    while (spriteIndex < 2) {
        if (lbl_1_bss_404.spriteObjectIds[spriteIndex] != -1) {
            mbObjKill((s16)lbl_1_bss_404.spriteObjectIds[spriteIndex]);
        }
        spriteIndex += 1;
    }
    if (lbl_1_bss_404.spriteObjectIds[2] != -1) {
        mbObjKill((s16)lbl_1_bss_404.spriteObjectIds[2]);
    }
}

static const GXColor lbl_1_rodata_190 = {255, 255, 255, 255};
static const Vec lbl_1_rodata_194 = {0.0f, 0.0f, 0.0f};

/* Sets the light and material state used to draw the star guide object. */
void fn_1_9360(HU3D_DRAW_OBJ *drawObj, HSF_MATERIAL *material)
{
    Vec configuredLightDirection;
    Vec lightPosition;
    Vec lightDirection;
    GXColor configuredLightColor;
    int tevStage;
    int tevColor;
    GXColor lightColor;

    configuredLightColor = lbl_1_rodata_190;
    configuredLightDirection = lbl_1_rodata_194;
    Hu3DGLightParamGet(0, &lightPosition, &lightDirection, &lightColor);
    Hu3DGlobalLight[0].color = configuredLightColor;
    Hu3DGlobalLight[0].dir = configuredLightDirection;
    mbObjStarTevStageSet(drawObj, material, &tevStage, &tevColor);
    Hu3DGLightColorSet(0, lightColor.r, lightColor.g, lightColor.b, lightColor.a);
    Hu3DGLightPosSetV(0, &lightPosition, &lightDirection);
    OSReport("guideLightSet\n");
}

/* Animate the board objects attached to spaces and their player reactions. */
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/board/object.h"
#include "game/board/masu.h"
#include "game/board/audio.h"
#include "game/board/branch.h"
#include "game/board/camera.h"
#include "game/board/player.h"
#include "game/process.h"

extern W06SpaceObject lbl_1_bss_2E4[4][3];
extern u32 lbl_1_data_13C[12];
extern s32 lbl_1_bss_14;

extern W06SaveState *lbl_1_bss_4B0;
extern f32 lbl_1_data_3AC;
extern Point3d lbl_1_data_3B0[4];
int lbl_1_data_384[2] = {W06_BOARD_DATA_W06_7, -1};
int lbl_1_data_38C[6] = {W06_BOARD_DATA_W06N_7, -1, W06_BOARD_DATA_W06_9, -1,
                         W06_BOARD_DATA_W06N_9, -1};
extern BOOL mbSaveNewF;

#define W06ObjectDataGet(fileNo) \
    ((W06TimeSideGet() != 0 ? DATA_w06 : DATA_w06n) | (fileNo))

/* Creates the space objects for the current time and hides the saved inactive branch. */
void fn_1_9474(void)
{
    Point3d spacePosition;
    s32 objectId;
    s32 row;
    s32 column;
    s32 spaceIndex;
    s16 objectSpace;

    spaceIndex = 0;
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 3; column++) {
            if (GwSystem.curTime == 0) {
                if (spaceIndex < 6) {
                    objectId = mbObjCreate(W06ObjectDataGet(8), NULL, 1);
                    lbl_1_bss_2E4[row][column].objectId = objectId;
                    lbl_1_bss_2E4[row][column].reactionObjectId =
                        mbObjCreate(W06ObjectDataGet(9), NULL, 1);
                    mbObjDispSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId, 0);
                    mbObjMotionTimeSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                        (0.0f));
                    mbObjMotionSpeedSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                        (0.0f));
                    lbl_1_bss_2E4[row][column].usesReactionObject = 1;
                } else {
                    objectId = mbObjCreate(W06ObjectDataGet(6), lbl_1_data_384, 1);
                    lbl_1_bss_2E4[row][column].objectId = objectId;
                    lbl_1_bss_2E4[row][column].reactionObjectId = -1;
                    mbObjMotionTimeSet(objectId, (0.0f));
                    mbObjMotionSpeedSet(objectId, (1.0f));
                    mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
                    lbl_1_bss_2E4[row][column].usesReactionObject = 0;
                }
            } else if (spaceIndex < 6) {
                objectId = mbObjCreate(W06ObjectDataGet(6), lbl_1_data_38C, 1);
                lbl_1_bss_2E4[row][column].objectId = objectId;
                lbl_1_bss_2E4[row][column].reactionObjectId = -1;
                mbObjMotionTimeSet(objectId, (0.0f));
                mbObjMotionSpeedSet(objectId, (1.0f));
                mbObjAttrSet(objectId, HU3D_MOTATTR_LOOP);
                lbl_1_bss_2E4[row][column].usesReactionObject = 0;
            } else {
                objectId = mbObjCreate(W06ObjectDataGet(8), NULL, 1);
                lbl_1_bss_2E4[row][column].objectId = objectId;
                lbl_1_bss_2E4[row][column].reactionObjectId =
                    mbObjCreate(W06ObjectDataGet(9), NULL, 1);
                mbObjDispSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId, 0);
                mbObjMotionTimeSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                    (0.0f));
                mbObjMotionSpeedSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                    (0.0f));
                lbl_1_bss_2E4[row][column].usesReactionObject = 1;
            }
            mbObjDispSet(objectId, 1);
            objectSpace = mbMasuFind_MAttrIdGet(-1, lbl_1_data_13C[spaceIndex]);
            mbMasuPosGet(objectSpace, &spacePosition);
            mbObjPosSetV(objectId, &spacePosition);
            if (lbl_1_bss_2E4[row][column].usesReactionObject == 1) {
                mbObjPosSetV((s16)lbl_1_bss_2E4[row][column].reactionObjectId, &spacePosition);
            }
            spaceIndex++;
            lbl_1_bss_2E4[row][column].bobAngle = (0.0f);
            lbl_1_bss_2E4[row][column].reactionActive = 0;
            lbl_1_bss_2E4[row][column].bobDisabled = 0;
        }
    }
    if (mbSaveNewF != 0) {
        lbl_1_bss_4B0->branchRows[1] = 1;
        lbl_1_bss_4B0->branchRows[0] = 2;
    }
    switch (lbl_1_bss_4B0->branchRows[W06TimeSideGet()]) {
    case 0:
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_0_MASK);
        break;
    case 1:
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_1_MASK);
        break;
    case 2:
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_2_MASK);
        break;
    case 3:
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_3_MASK);
        break;
    }
    for (row = 0; row < 3; row++) {
        mbObjDispSet((s16)lbl_1_bss_2E4[lbl_1_bss_4B0->branchRows[W06TimeSideGet()]][row].objectId,
            0);
        mbObjDispSet(
            (s16) lbl_1_bss_2E4[lbl_1_bss_4B0->branchRows[W06TimeSideGet()]][row].reactionObjectId,
            0);
    }
}

/* Called each board update to move the space objects and react to the turn player. */

void fn_1_9C38(void)
{
    Mtx spaceMatrix;
    Point3d objectPosition;
    f32 bobOffset;
    s32 row;
    s32 column;
    s32 spaceIndex;
    s32 playerNo;
    u32 spaceAttribute;
    s32 reactionStarted;
    s16 currentSpace;
    HU3D_MODELID modelId;
    s16 spaceId;

    reactionStarted = 0;
    spaceIndex = 0;
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 3; column++) {
            if (lbl_1_bss_2E4[row][column].usesReactionObject == 0 &&
                lbl_1_bss_2E4[row][column].reactionActive != 0 &&
                mbObjMotionEndCheck((s16)lbl_1_bss_2E4[row][column].objectId) != 0) {
                mbObjMotionShiftSet((s16)lbl_1_bss_2E4[row][column].objectId,
                                    0, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                lbl_1_bss_2E4[row][column].reactionActive = 0;
            }
            if (lbl_1_bss_2E4[row][column].bobDisabled == 0) {
                bobOffset = 0.4f * sin((M_PI * lbl_1_bss_2E4[row][column].bobAngle) / 180.0);
                lbl_1_bss_2E4[row][column].bobAngle += 2.0f;
                if (lbl_1_bss_2E4[row][column].bobAngle >= 360.0f) {
                    lbl_1_bss_2E4[row][column].bobAngle -= 360.0f;
                }
                mbObjPosGet((s16)lbl_1_bss_2E4[row][column].objectId, &objectPosition);
                mbObjPosSet((s16)lbl_1_bss_2E4[row][column].objectId,
                            objectPosition.x, objectPosition.y - bobOffset, objectPosition.z);
                if (lbl_1_bss_2E4[row][column].usesReactionObject == 1) {
                    mbObjPosGet((s16)lbl_1_bss_2E4[row][column].reactionObjectId, &objectPosition);
                    mbObjPosSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                                objectPosition.x, objectPosition.y - bobOffset, objectPosition.z);
                }
            }
            modelId = mbObjModelIDGet((s16)lbl_1_bss_2E4[row][column].objectId);
            Hu3DModelObjMtxGet(modelId, "hook", spaceMatrix);
            spaceId = mbMasuFind_MAttrIdGet(-1, lbl_1_data_13C[spaceIndex]);
            mbMasuMtxSet(spaceId, spaceMatrix);
            spaceIndex++;
        }
    }

    playerNo = GwSystem.turnPlayerNo;
    if (playerNo >= 0) {
        currentSpace = GwPlayer[playerNo].masuId;
        spaceAttribute = mbMasuMAttrGet(currentSpace);
        if ((spaceAttribute & W06_SPACE_ATTR_SPECIAL_MASK) != 0) {
            spaceIndex = 0;
            for (row = 0; row < 4; row++) {
                for (column = 0; column < 3; column++) {
                    if (reactionStarted == 0) {
                        if ((spaceAttribute & lbl_1_data_13C[spaceIndex]) != 0 &&
                            lbl_1_bss_2E4[row][column].reactionActive == 0) {
                            lbl_1_bss_2E4[row][column].reactionActive = 1;
                            if (lbl_1_bss_2E4[row][column].usesReactionObject == 1) {
                                mbObjDispSet((s16)lbl_1_bss_2E4[row][column].objectId, 0);
                                mbObjDispSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId, 1);
                                mbObjMotionSpeedSet(
                                    (s16) lbl_1_bss_2E4[row][column].reactionObjectId, 1.0f);
                                mbObjMotionTimeSet((s16)lbl_1_bss_2E4[row][column].reactionObjectId,
                                                   0.0f);
                                mbAudFXPlay(1536);
                            } else {
                                mbObjMotionSet((s16)lbl_1_bss_2E4[row][column].objectId, 1, 0);
                            }
                            reactionStarted = 1;
                            if (lbl_1_bss_2E4[row][column].usesReactionObject == 1) {
                                lbl_1_bss_14++;
                            }
                        }
                        spaceIndex++;
                    }
                }
            }
        }
    }
}

/* Swaps a group of branch objects and drops the newly revealed group onto its spaces. */
void fn_1_A204(int playerNo, s16 spaceId)
{
    Point3d spacePositions[3];
    Point3d objectPosition;
    s32 work[1];
    f32 fallSpeed;
    s32 column;
    s32 shownRow;
    s32 hiddenRow;
    s32 landed;
    s16 objectSpace;
    s32 currentTimeSide;
    s32 firstBranchTimeSide;
    s32 secondBranchTimeSide;
    s32 thirdBranchTimeSide;
    s32 fourthBranchTimeSide;

    work[0] = 0;
    mbMoveNumDispSet(playerNo, 0);
    mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
    HuPrcSleep(20);
    mbCameraMovePos(NULL, NULL, NULL, lbl_1_data_3AC, (-1.0f), 30);
    mbCameraMoveWait();
    currentTimeSide = GwSystem.curTime == 0;
    switch (lbl_1_bss_4B0->branchRows[currentTimeSide]) {
    case 0:
        shownRow = 0;
        hiddenRow = 1;
        mbBranchMAttrReset(W06_BRANCH_MATTR_ROW_0_MASK);
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_1_MASK);
        firstBranchTimeSide = GwSystem.curTime == 0;
        lbl_1_bss_4B0->branchRows[firstBranchTimeSide] = 1;
        break;
    case 1:
        shownRow = 1;
        hiddenRow = 0;
        mbBranchMAttrReset(W06_BRANCH_MATTR_ROW_1_MASK);
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_0_MASK);
        secondBranchTimeSide = GwSystem.curTime == 0;
        lbl_1_bss_4B0->branchRows[secondBranchTimeSide] = 0;
        break;
    case 2:
        shownRow = 2;
        hiddenRow = 3;
        mbBranchMAttrReset(W06_BRANCH_MATTR_ROW_2_MASK);
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_3_MASK);
        thirdBranchTimeSide = GwSystem.curTime == 0;
        lbl_1_bss_4B0->branchRows[thirdBranchTimeSide] = 3;
        break;
    case 3:
        shownRow = 3;
        hiddenRow = 2;
        mbBranchMAttrReset(W06_BRANCH_MATTR_ROW_3_MASK);
        mbBranchMAttrSet(W06_BRANCH_MATTR_ROW_2_MASK);
        fourthBranchTimeSide = GwSystem.curTime == 0;
        lbl_1_bss_4B0->branchRows[fourthBranchTimeSide] = 2;
        break;
    }
    for (column = 0; column < 3; column++) {
        mbObjDispSet((s16)lbl_1_bss_2E4[hiddenRow][column].objectId, 0);
        mbObjDispSet((s16)lbl_1_bss_2E4[hiddenRow][column].reactionObjectId, 0);
        mbObjMotionTimeSet((s16)lbl_1_bss_2E4[hiddenRow][column].reactionObjectId,
            (0.0f));
        mbObjMotionSpeedSet((s16)lbl_1_bss_2E4[hiddenRow][column].reactionObjectId,
            (0.0f));
        lbl_1_bss_2E4[hiddenRow][column].reactionActive = 0;
    }
    mbCameraMovePos(&lbl_1_data_3B0[shownRow], NULL, NULL,
        (-1.0f), (-1.0f), 60);
    mbCameraMoveWait();
    for (column = 0; column < 3; column++) {
        mbObjDispSet((s16)lbl_1_bss_2E4[shownRow][column].objectId, 1);
        mbObjDispSet((s16)lbl_1_bss_2E4[shownRow][column].reactionObjectId, 0);
        objectSpace = mbMasuFind_MAttrIdGet(-1, lbl_1_data_13C[column + shownRow * 3]);
        mbMasuPosGet(objectSpace, &spacePositions[column]);
        mbObjPosSet((s16)lbl_1_bss_2E4[shownRow][column].objectId,
            spacePositions[column].x,
            (100.0f) * ((10.0f) + (2.0f) * column) +
                spacePositions[column].y,
            spacePositions[column].z);
        mbObjPosSet((s16)lbl_1_bss_2E4[shownRow][column].reactionObjectId,
            spacePositions[column].x,
            (100.0f) * ((10.0f) + (2.0f) * column) +
                spacePositions[column].y,
            spacePositions[column].z);
        lbl_1_bss_2E4[shownRow][column].bobDisabled = 1;
    }
    fallSpeed = (16.66666603088379f);
    landed = 0;
    for (;;) {
        for (column = 0; column < 3; column++) {
            if (lbl_1_bss_2E4[shownRow][column].bobDisabled != 0) {
                mbObjPosGet((s16)lbl_1_bss_2E4[shownRow][column].objectId, &objectPosition);
                objectPosition.y -= fallSpeed;
                if (objectPosition.y <= spacePositions[column].y) {
                    mbObjPosSetV((s16)lbl_1_bss_2E4[shownRow][column].objectId,
                        &spacePositions[column]);
                    mbObjPosSetV((s16)lbl_1_bss_2E4[shownRow][column].reactionObjectId,
                        &spacePositions[column]);
                    lbl_1_bss_2E4[shownRow][column].bobDisabled = 0;
                    landed++;
                } else {
                    mbObjPosSetV((s16)lbl_1_bss_2E4[shownRow][column].objectId, &objectPosition);
                    mbObjPosSetV((s16)lbl_1_bss_2E4[shownRow][column].reactionObjectId,
                        &objectPosition);
                }
            }
        }
        if (landed >= 3) {
            break;
        }
        HuPrcVSleep();
    }
    lbl_1_bss_14 = 0;
    HuPrcSleep(30);
    mbCameraMovePlayer((s16)playerNo, NULL, NULL, (-1.0f), (-1.0f), 60);
    mbCameraMoveWait();
    HuPrcSleep(10);
    mbMoveNumDispSet(playerNo, 1);
}

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"

#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

f32 lbl_1_data_3AC = 3000.0f;
Point3d lbl_1_data_3B0[4] = {
    {-3700.0f, -100.0f, 1425.0f},
    {-1927.0f, -298.0f, 1564.0f},
    {1964.0f, -140.0f, 1863.0f},
    {3677.0f, -77.0f, 1819.0f}
};

/* Releases the first two reaction-space objects and their companion models as the board closes. */
void fn_1_A968(void) {
    s32 spaceIndex;
    s32 row;
    s32 column;
    spaceIndex = 0;
    row = 0;
    while (row < 4)
    {
        column = 0;
        while (column < 3)
        {
            if ((spaceIndex == 0) || (spaceIndex == 1))
            {
                mbObjKill((s16)lbl_1_bss_2E4[row][column].objectId);
                if (lbl_1_bss_2E4[row][column].reactionObjectId != -1) {
                    mbObjKill((s16)lbl_1_bss_2E4[row][column].reactionObjectId);
                }
            }
            spaceIndex += 1;
            column += 1;
        }
        row += 1;
    }
}

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"

#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/process.h"
#include "game/pad.h"
#include "game/board/player.h"
#include "game/board/main.h"
#include "game/board/window.h"
#include "game/wipe.h"

extern W06HookModels lbl_1_bss_2AC;
extern OMOBJ *lbl_1_bss_18;
extern s16 lbl_1_bss_60;
extern s32 lbl_1_bss_74[2];

void fn_1_B820(void);
void mbGuideKill(OMOBJ *obj);
void mbWipeFadeOut(void);

#include "game/board/camera.h"
#include "game/board/audio.h"
#include "game/board/masu.h"

typedef struct W06CameraRotations {
    Point3d start;
    Point3d end;
} W06CameraRotations;

extern s8 lbl_1_data_7A4[];
extern Point3d lbl_1_data_75C[2];
extern W06CameraRotations lbl_1_data_774[2];

OMOBJ *mbGuideCreateFlag(HuVecF *pos, s8 *motTbl, BOOL screenF, BOOL altMtxF, BOOL layerF);
void mbGuideMotionNextSet(OMOBJ *obj, s16 motNo);
MBMODELID mbGuideModelGet(OMOBJ *obj);
void mbGuideFadeIn(OMOBJ *obj);
void mbGuideMotionShiftSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
BOOL mbGuideMotionCheck(OMOBJ *obj);
void mbGuideMotionSet(OMOBJ *obj, s16 motNo, BOOL shiftF);
int mbSNpcMasuGet(void);
void mbSNpcMotWinSet(void);
void mbSNpcMotIdleSet(void);
void mbWipeFadeIn(void);
void mbTelopTimeChangeCreate(void);
BOOL mbTelopTimeChangeCheck(void);

/* Presents the day or night change, including the board feature and star guide messages. */

char *lbl_1_data_578[37] = {
    "b06_hook00", "b06_hook01", "b06_hook02", "b06_hook03",
    "b06_hook04", "b06_hook05", "b06_hook06", "b06_hook07",
    "b06_hook08", "b06_hook09", "b06_hook10", "b06_hook11",
    "b06_hook12", "b06_hook13", "b06_hook14", "b06_hook15",
    "b06_hook16", "b06_hook17", "b06_hook18", "b06_hook19",
    "b06_hook20", "b06_hook21", "b06_hook22", "b06_hook23",
    "b06_hook24", "b06_hook25", "b06_hook26", "b06_hook27",
    "b06_hook28", "b06_hook29", "b06_hook30", "b06_hook31",
    "b06_hook32", "b06_hook33", "b06_hook34", "b06_hook35",
    "b06_hook36"
};
char *lbl_1_data_6FC[24] = {
    "ya_hook00", "ya_hook01", "ya_hook02", "ya_hook03",
    "ya_hook04", "ya_hook05", "ya_hook06", "ya_hook07",
    "ya_hook08", "ya_hook09", "ya_hook10", "ya_hook11",
    "ya_hook12", "ya_hook13", "ya_hook14", "ya_hook15",
    "ya_hook18", "ya_hook19", "ya_hook20", "ya_hook21",
    "ya_hook22", "ya_hook23", "ya_hook24", "ya_hook25"
};
Point3d lbl_1_data_75C[2] = {{2486.0f, 126.0f, 712.0f}, {-2444.0f, 508.0f, 570.0f}};
W06CameraRotations lbl_1_data_774[2] = { { { -50.0f, 20.0f, 0.0f }, { -30.0f, 0.0f, 0.0f } },
                                         { { -50.0f, -20.0f, 0.0f }, { -30.0f, 0.0f, 0.0f } } };
s8 lbl_1_data_7A4[4] = {12, 6, 7, -1};

#define W06ObjectDataGet(fileNo) \
    ((W06TimeSideGet() != 0 ? DATA_w06 : DATA_w06n) | (fileNo))

/* Attaches the board decoration models to both roots and sets their initial animation. */

void fn_1_AA48(void);
void fn_1_B464(void);
void fn_1_B584(void);
void fn_1_B820(void);
/* Attaches the board decoration models to both roots during board setup. */
void fn_1_AA48(void)
{
    s32 hookObjectId;
    s32 rootObjectId;
    s32 attachment;
    s32 nextHook;

    nextHook = 0;
    rootObjectId = mbObjCreate(W06ObjectDataGet(21), NULL, 0);
    lbl_1_bss_2AC.firstRootId = rootObjectId;

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(10), NULL, 0);
        lbl_1_bss_2AC.attachedIds[0] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 4; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(11), NULL, 0);
        lbl_1_bss_2AC.attachedIds[1] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 2; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(12), NULL, 0);
        lbl_1_bss_2AC.attachedIds[2] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 4; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(13), NULL, 0);
        lbl_1_bss_2AC.attachedIds[3] = hookObjectId;
        for (attachment = 0; attachment < 5; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(14), NULL, 0);
        lbl_1_bss_2AC.attachedIds[4] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 2; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(15), NULL, 0);
        lbl_1_bss_2AC.attachedIds[5] = hookObjectId;
        for (attachment = 0; attachment < 2; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(16), NULL, 0);
        lbl_1_bss_2AC.attachedIds[6] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 2; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(17), NULL, 0);
        lbl_1_bss_2AC.attachedIds[7] = hookObjectId;
        for (attachment = 0; attachment < 4; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(18), NULL, 0);
        lbl_1_bss_2AC.attachedIds[8] = hookObjectId;
        for (attachment = 0; attachment < 6; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(19), NULL, 0);
        lbl_1_bss_2AC.attachedIds[9] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 2; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(20), NULL, 0);
        lbl_1_bss_2AC.attachedIds[10] = hookObjectId;
        mbObjAttrSet(hookObjectId, HU3D_MOTATTR_LOOP);
        mbObjMotionShapeSet(hookObjectId, 0, W06_HOOK_SHAPE_ATTR_FLAGS);
        mbObjMotionTimeSet(hookObjectId, (0.0f));
        mbObjMotionSpeedSet(hookObjectId, (1.0f));
        mbObjMotionShapeTimeSet(hookObjectId, (0.0f));
        mbObjMotionShapeSpeedSet(hookObjectId, (1.0f));
        for (attachment = 0; attachment < 4; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_578[nextHook++], hookObjectId);
        }
    }

    rootObjectId = mbObjCreate(W06ObjectDataGet(22), NULL, 0);
    lbl_1_bss_2AC.secondRootId = rootObjectId;
    {
        hookObjectId = mbObjCreate(W06ObjectDataGet(3), NULL, 0);
        lbl_1_bss_404.objectIds[0] = hookObjectId;
        mbObjAttrReset(hookObjectId, HU3D_MOTATTR_LOOP);
        if (GwSystem.curTime == 0) {
            mbObjMotionTimeSet(hookObjectId, (0.0f));
        } else {
            mbObjMotionTimeSet(hookObjectId, (60.0f));
        }
        mbObjMotionSpeedSet(hookObjectId, (0.0f));
        for (attachment = 0; attachment < 24; attachment++) {
            mbObjHookSet(rootObjectId, lbl_1_data_6FC[attachment], hookObjectId);
        }
    }
}

/* Releases the decoration roots and their attached models when the board closes. */
void fn_1_B464(void) {
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (0))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (8))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (12))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (16))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (20))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (24))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (28))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (32))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (36))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (40))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (44))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (48))));
    mbObjKill((s16) (*(s32 *)((s8 *)(&lbl_1_bss_2AC) + (4))));
}

/* Runs the next-time board transition and waits for a player to dismiss its guide. */
void fn_1_B584(void)
{
    HUPROCESS *promptProcess;
    s16 helpWindowId;
    s32 *workState;
    s32 playerNo;

    workState = lbl_1_bss_74;
    for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
        mbPlayerDispSet(playerNo, FALSE);
    }
    memset(workState, 0, 8);
    promptProcess = HuPrcChildCreate(fn_1_B820, 8199, 32768, 0, mbMainProc);
    helpWindowId = -1;
    do {
        if (workState[0] != 0) {
            if (helpWindowId < 0) {
                helpWindowId = mbWinCreateHelp(W06_HELP_MESSAGE_ID);
                mbWinPosSet(helpWindowId, 228, 408);
            }
            for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
                if (GwPlayer[playerNo].comF == FALSE &&
                    (HuPadBtnDown[(s8)GwPlayer[playerNo].padNo] & 0x1000) != 0) {
                    if (lbl_1_bss_60 != -1) {
                        mbWinKill(lbl_1_bss_60);
                        lbl_1_bss_60 = -1;
                    }
                    HuPrcKill(promptProcess);
                    workState[1] = 1;
                    break;
                }
            }
        }
        HuPrcVSleep();
    } while (workState[1] == 0);
    mbWipeWait();
    if (WipeCheckIn() == FALSE) {
        mbWipeFadeOut();
    }
    if (GwSystem.curTime != FALSE) {
        mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[0], (60.0f));
    } else {
        mbObjMotionTimeSet((s16)lbl_1_bss_404.objectIds[0], (0.0f));
    }
    mbObjMotionSpeedSet((s16)lbl_1_bss_404.objectIds[0], (0.0f));
    if (lbl_1_bss_18 != NULL) {
        mbGuideKill(lbl_1_bss_18);
        lbl_1_bss_18 = 0;
    }
    mbWinKill(helpWindowId);
    for (playerNo = 0; playerNo < GW_PLAYER_MAX; playerNo++) {
        mbPlayerDispSet(playerNo, TRUE);
    }
}

/* Shows the new-time opening, introduces the star guide, then returns to board play. */
void fn_1_B820(void)
{
    Point3d openingPosition = { 0.0f, 0.0f, 0.0f };
    Point3d openingRotation = { 329.0f, 0.0f, 0.0f };
    Point3d cameraPosition;
    Point3d featurePosition = { 0.0f, 75.0f, 3700.0f };
    W06CameraRotations cameraRotations = { { - 50.0f, 0.0f, 0.0f }, { - 30.0f, 0.0f, 0.0f } };
    Point3d starPosition;
    HuVec2f windowPosition;
    f32 startingZoom = 6000.0f;
    f32 featureZoom = 3000.0f;
    f32 finalZoom = (1600.0f);
    Point3d guidePosition = { 0.0f, 820.0f, 5000.0f };
    s32 speaker;
    s32 messageOffset;
    s32 side;
    s32 * state;
    s16 starSpace;
    s16 reverseMotionObjectId;
    s16 forwardMotionObjectId;
    s16 guideObjectId;
    state = lbl_1_bss_74;
    lbl_1_bss_60 = - 1;
    lbl_1_bss_18 = NULL;
    lbl_1_bss_18 = mbGuideCreateFlag(& guidePosition, lbl_1_data_7A4, 0, 0, 0);
    mbGuideMotionNextSet(lbl_1_bss_18, 1);
    guideObjectId = mbGuideModelGet(lbl_1_bss_18);
    mbObjDispSet(guideObjectId, 0);
    if (GwSystem.curTime == 0)
    {
        mbObjMotionTimeSet((s16) lbl_1_bss_404.objectIds[0], (60.0f));
    }
    else
    {
        mbObjMotionTimeSet((s16) lbl_1_bss_404.objectIds[0], (0.0f));
    }
    mbObjMotionSpeedSet((s16) lbl_1_bss_404.objectIds[0], (0.0f));
    mbCameraMovePos(& openingPosition, & openingRotation, NULL, 13500.0f, (30.0f), 0);
    mbWipeFadeIn();
    mbTelopTimeChangeCreate();
    while (mbTelopTimeChangeCheck() != 0)
    {
        HuPrcVSleep();
    }
    mbWipeFadeOut();
    if (GwSystem.curTime == 0)
    {
        speaker = 6;
        messageOffset = 0;
    }
    else
    {
        speaker = 7;
        messageOffset = 1;
    }
    state[0] = 1;
    cameraPosition = featurePosition;
    cameraPosition.y += (1000.0f);
    mbCameraMovePos(& cameraPosition, & cameraRotations.start, NULL, startingZoom, (-1.0f), 1);
    HuPrcVSleep();
    mbCameraMovePos(& featurePosition, & cameraRotations.end, NULL, featureZoom, (-1.0f), 120);
    mbCameraCurveTypeSet(1);
    mbWipeFadeIn();
    mbCameraMoveWait();
    mbObjDispSet(guideObjectId, 1);
    mbGuideFadeIn(lbl_1_bss_18);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbGuideMotionShiftSet(lbl_1_bss_18, 12, 1);
    lbl_1_bss_60 = mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 47), speaker);
    mbWinPlayerDisable(lbl_1_bss_60, - 1);
    mbWinTopPosGet(& windowPosition);
    windowPosition.y -= (40.0f);
    mbWinTopPosSet((s16) windowPosition.x, (s16) windowPosition.y);
    mbWinTopWait();
    lbl_1_bss_60 = - 1;
    while (mbGuideMotionCheck(lbl_1_bss_18) == 0)
    {
        HuPrcVSleep();
    }
    if (GwSystem.curTime == 0)
    {
        mbAudFXPlay(MSM_SE_W06_GUIDE_MOTION);
    }
    else
    {
        mbAudFXPlay(MSM_SE_W06_GUIDE_MOTION_ALT);
    }
    mbObjMotionShiftSet(guideObjectId, 6, (0.0f), (8.0f), 0);
    HuPrcSleep(60);
    if (GwSystem.curTime == 0)
    {
        reverseMotionObjectId = (s16) lbl_1_bss_404.objectIds[0];
        mbObjAttrSet(reverseMotionObjectId, W06_REVERSE_MOTION_ATTR_FLAGS);
        mbObjMotionSpeedSet((s16) lbl_1_bss_404.objectIds[0], (1.0f));
    }
    else
    {
        forwardMotionObjectId = (s16) lbl_1_bss_404.objectIds[0];
        mbObjAttrReset(forwardMotionObjectId, W06_REVERSE_MOTION_ATTR_FLAGS);
        mbObjMotionSpeedSet((s16) lbl_1_bss_404.objectIds[0], (1.0f));
    }
    mbAudFXPlay(1537);
    while (mbObjMotionEndCheck((s16) lbl_1_bss_404.objectIds[0]) == 0)
    {
        HuPrcVSleep();
    }
    mbObjMotionSpeedSet((s16) lbl_1_bss_404.objectIds[0], (0.0f));
    HuPrcSleep(30);
    mbGuideMotionNextSet(lbl_1_bss_18, 1);
    mbWipeFadeOut();
    mbObjDispSet(guideObjectId, 0);
    mbCameraRotSet(- 35.0f, (0.0f), (0.0f));
    starSpace = mbSNpcMasuGet();
    mbMasuPosGet(starSpace, & starPosition);
    mbCameraMovePos(& starPosition, NULL, NULL, 3200.0f, (-1.0f), 1);
    mbCameraMoveWait();
    mbWipeFadeIn();
    HuPrcSleep(20);
    mbCameraMovePos(NULL, NULL, NULL, 2400.0f, (-1.0f), 30);
    mbCameraMoveWait();
    mbSNpcMotWinSet();
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    lbl_1_bss_60 = mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 49), speaker);
    mbWinPlayerDisable(lbl_1_bss_60, - 1);
    mbWinTopPosGet(& windowPosition);
    windowPosition.y -= (40.0f);
    mbWinTopPosSet((s16) windowPosition.x, (s16) windowPosition.y);
    mbWinTopWait();
    lbl_1_bss_60 = - 1;
    mbAudGuidePlay(MSM_SE_GUIDE_26);
    lbl_1_bss_60 = mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 51), speaker);
    mbWinPlayerDisable(lbl_1_bss_60, - 1);
    mbWinTopPosGet(& windowPosition);
    windowPosition.y -= (40.0f);
    mbWinTopPosSet((s16) windowPosition.x, (s16) windowPosition.y);
    mbWinTopWait();
    lbl_1_bss_60 = - 1;
    mbWipeFadeOut();
    mbSNpcMotIdleSet();
    if (! (! (GwSystem.curTime == 0)))
    {
        side = 0;
    }
    else
    {
        side = 1;
    }
    cameraPosition = lbl_1_data_75C[side];
    cameraPosition.y += (1000.0f);
    mbCameraMovePos(& cameraPosition, & lbl_1_data_774[side].start, NULL, startingZoom, (-1.0f), 1);
    HuPrcVSleep();
    mbCameraMovePos(&lbl_1_data_75C[side], &lbl_1_data_774[side].end, NULL, finalZoom, (-1.0f),
                    180);
    mbCameraCurveTypeSet(1);
    mbWipeFadeIn();
    mbCameraMoveWait();
    mbGuideMotionSet(lbl_1_bss_18, 1, 1);
    mbObjPosSetV(guideObjectId, & lbl_1_data_75C[side]);
    mbObjDispSet(guideObjectId, 1);
    mbGuideFadeIn(lbl_1_bss_18);
    mbAudGuidePlay(MSM_SE_GUIDE_25);
    mbGuideMotionShiftSet(lbl_1_bss_18, 12, 1);
    lbl_1_bss_60 = mbWinCreate(2, messageOffset + MESSNUM(MESS_BOARD_W06, 53), speaker);
    mbWinPlayerDisable(lbl_1_bss_60, - 1);
    mbWinTopPosGet(& windowPosition);
    windowPosition.y -= (40.0f);
    mbWinTopPosSet((s16) windowPosition.x, (s16) windowPosition.y);
    mbWinTopWait();
    lbl_1_bss_60 = - 1;
    mbWipeFadeOut();
    mbGuideKill(lbl_1_bss_18);
    lbl_1_bss_18 = NULL;
    state[1] = 1;
    HuPrcEnd();
}

#include "dolphin.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/gamework.h"

#include "game/board/object.h"

extern OMOBJ *mbGuideCreate(int guideNo, HuVecF *pos, HuVecF *rot, s8 *motTbl, float scale,
                            u32 attr);

typedef struct C1CCStack {
    s16 guideIds[2];
    Point3d playerPosition;
    s16 previousSpace;
    u32 workValues[2];
} C1CCStack;

extern W06RingVertex lbl_1_bss_7C[20];
extern Point3d lbl_1_bss_68;
extern s32 lbl_1_bss_64;
extern s32 lbl_1_bss_30[6];
extern s32 lbl_1_bss_48[6];
extern s32 lbl_1_data_7B0[6][3];
extern HuVecF lbl_1_data_C[6];
extern OMOBJ *lbl_1_data_0[2];
s8 lbl_1_data_7A8[8] = {12, 10, 22, 7, -1, 0, 0, 0};
s32 lbl_1_data_7B0[6][3] = {
    {0, 1, 2},
    {0, 2, 1},
    {1, 0, 2},
    {1, 2, 0},
    {2, 0, 1},
    {2, 1, 0}
};
extern s32 lbl_1_bss_1C;
extern s32 lbl_1_bss_20;
extern f32 lbl_1_bss_24;
extern f32 lbl_1_bss_28;
extern f32 lbl_1_bss_2C;
extern GXColor lbl_1_data_1B2;

u16 mbPadDStkRepGetAll(void);

/* Shows the guide message after a time change and moves the player to its parent space if on a
 * branch. */

void fn_1_C1CC(void)
{
    Point3d guidePositions[2];
    C1CCStack stack;
    s32 guideIndex;
    s32 windowMessage;
    s32 messageId;
    u32 spaceAttribute;
    s32 oldSpace;
    s32 currentSpace;
    s32 parentSpace;
    s32 playerIndex;

    playerIndex = GwSystem.turnPlayerNo;
    currentSpace = GwPlayer[playerIndex].masuId;
    spaceAttribute = mbMasuMAttrGet((s16)currentSpace);
    mbPlayerColSnapSet(0);
    mbStatusDispForceSetAll(1);
    mbev_PlayerColMasu(playerIndex, currentSpace, 1);
    mbPlayerRotSet(playerIndex, (0.0f), (180.0f),
                   (0.0f));
    mbCameraMovePlayer((s16)playerIndex, NULL, NULL, (-1.0f),
                       (-1.0f), 6);
    mbPlayerColSnapSet(1);
    mbPlayerPosGet(playerIndex, &stack.playerPosition);

    for (guideIndex = 0; guideIndex < 2; guideIndex++) {
        guidePositions[guideIndex] = stack.playerPosition;
        guidePositions[guideIndex].x = (f32)((200.0) *
            cos(((3.141592653589793) * (f64)((180.0f) * guideIndex)) /
                (180.0)));
        guidePositions[guideIndex].x *= (-1.0f);
        guidePositions[guideIndex].y += (200.0f);
        guidePositions[guideIndex].z -= (200.0f);
        lbl_1_data_0[guideIndex] = mbGuideCreate(
            guideIndex, &guidePositions[guideIndex], NULL, lbl_1_data_7A8,
            (1.0f), 0);
        mbGuideMotionNextSet(lbl_1_data_0[guideIndex], 1);
        stack.guideIds[guideIndex] =
            mbGuideModelGet(lbl_1_data_0[guideIndex]);
    }

    if (GwSystem.curTime == (u32)0) {
        windowMessage = 6;
        messageId = 0;
        WipeCreate(1, 130, 60);
        HuPrcSleep(60);
    } else {
        windowMessage = 7;
        messageId = 1;
        WipeCreate(1, 131, 60);
        HuPrcSleep(60);
    }
    mbGuideMotionShiftSet(lbl_1_data_0[mbGuideNoGet()], 7, 1);
    mbWinCreate(2, messageId + MESSNUM(MESS_BOARD_W06, 17), windowMessage);
    mbWinTopWait();
    for (guideIndex = 0; guideIndex < 2; guideIndex++) {
        mbGuideEnd(lbl_1_data_0[guideIndex], 1);
    }
    mbPlayerRotateStart(playerIndex, 0, 15);
    while (mbPlayerRotateCheck(playerIndex) == 0) {
        HuPrcVSleep();
    }
    mbPlayerMotionShiftSet(playerIndex, 1, (0.0f), (8.0f),
                           HU3D_MOTATTR_LOOP);
    if ((spaceAttribute & W06_SPACE_ATTR_BRANCH_MASK) != 0) {
        mbMasuLinkParentGet((s16)currentSpace, &stack.previousSpace);
        parentSpace = stack.previousSpace;
        oldSpace = currentSpace;
        GwPlayer[playerIndex].masuIdPrev = oldSpace;
        mbPlayerMasuMoveTo(playerIndex, parentSpace, 1);
        GwPlayer[playerIndex].masuId = parentSpace;
    }
}

/* Selects the six tile objects used by the display and places them at their slots. */
void fn_1_C5D0(s32 mode)
{
    u32 randomRow;
    s32 row;
    s32 item;
    s32 displayIndex;
    s32 rowOffset;

    switch (mode) {
    case 2:
        displayIndex = 0;
        row = 0;
        while (row < 2) {
            randomRow = mbRandMod(6);
            item = 0;
            while (item < 3) {
                rowOffset = row * 3;
                lbl_1_bss_4B0->objectStates[displayIndex] =
                    (u8)lbl_1_data_7B0[randomRow][item];
                lbl_1_bss_404.displayObjects[displayIndex].dataId =
                    lbl_1_bss_48[lbl_1_bss_4B0->objectStates[displayIndex] + rowOffset];
                lbl_1_bss_404.displayObjects[displayIndex].objectId =
                    lbl_1_bss_30[lbl_1_data_7B0[randomRow][item] + rowOffset];
                displayIndex++;
                item++;
            }
            row++;
        }
        break;
    case 0:
        randomRow = mbRandMod(6);
        item = 0;
        displayIndex = 3;
        while (item < 3) {
            lbl_1_bss_4B0->objectStates[displayIndex] =
                (u8)lbl_1_data_7B0[randomRow][item];
            lbl_1_bss_404.displayObjects[displayIndex].dataId =
                lbl_1_bss_48[lbl_1_bss_4B0->objectStates[displayIndex] + 3];
            lbl_1_bss_404.displayObjects[displayIndex].objectId =
                lbl_1_bss_30[lbl_1_data_7B0[randomRow][item] + 3];
            item++;
            displayIndex++;
        }
        break;
    case 1:
        randomRow = mbRandMod(6);
        item = 0;
        displayIndex = 0;
        while (item < 3) {
            lbl_1_bss_4B0->objectStates[displayIndex] =
                (u8)lbl_1_data_7B0[randomRow][item];
            lbl_1_bss_404.displayObjects[displayIndex].dataId =
                lbl_1_bss_48[lbl_1_bss_4B0->objectStates[displayIndex]];
            lbl_1_bss_404.displayObjects[displayIndex].objectId =
                lbl_1_bss_30[lbl_1_data_7B0[randomRow][item]];
            item++;
            displayIndex++;
        }
        break;
    }

    for (row = 0; row < 6; row++) {
        mbObjPosSetV((s16)lbl_1_bss_404.displayObjects[row].objectId,
                     &lbl_1_data_C[row]);
    }
}

/* Seeds the hanging chain's 20 segments below its model during board setup. */
void fn_1_C92C(void)
{
    Mtx modelMatrix;
    s32 modelId;
    s32 vertexIndex;

    modelId = mbObjModelIDGet((s16)lbl_1_bss_404.objectIds[4]);
    Hu3DModelObjMtxGet(modelId, "z_sao_ito", modelMatrix);
    for (vertexIndex = 0; vertexIndex < 20; vertexIndex++) {
        lbl_1_bss_7C[vertexIndex].position.x = modelMatrix[0][3];
        lbl_1_bss_7C[vertexIndex].position.y =
            (f32)((f64)modelMatrix[1][3] - (60.0) * (f64)vertexIndex);
        lbl_1_bss_7C[vertexIndex].position.z = modelMatrix[2][3];
        lbl_1_bss_7C[vertexIndex].velocity.x = (0.0f);
        lbl_1_bss_7C[vertexIndex].velocity.y = (0.0f);
        lbl_1_bss_7C[vertexIndex].velocity.z = (0.0f);
        lbl_1_bss_7C[vertexIndex].angle = (0.0f);
    }
    lbl_1_bss_68.x = modelMatrix[0][3];
    lbl_1_bss_68.y = (f32)((f64)modelMatrix[1][3] - (1140.0));
    lbl_1_bss_68.z = modelMatrix[2][3];
    lbl_1_bss_64 = -1;
}

/* Advances the linked vertices under gravity and keeps each segment at its fixed length. */
#include "game/hu3d.h"
extern s32 lbl_1_bss_8;
void fn_1_CAF0(void)
{
    Mtx modelMatrix;
    s16 modelId;

    modelId = (s16)mbObjModelIDGet((s16)lbl_1_bss_404.objectIds[4]);
    Hu3DModelObjMtxGet(modelId, "z_sao_ito", modelMatrix);
    lbl_1_bss_7C[0].position.x = modelMatrix[0][3];
    lbl_1_bss_7C[0].position.y = modelMatrix[1][3];
    lbl_1_bss_7C[0].position.z = modelMatrix[2][3];
    if (lbl_1_bss_8 != 0) {
        lbl_1_bss_68.x = lbl_1_bss_7C[18].position.x;
        lbl_1_bss_68.y = lbl_1_bss_7C[18].position.y;
        lbl_1_bss_68.z = lbl_1_bss_7C[18].position.z;
        mbObjPosSetV((s16)lbl_1_bss_64, &lbl_1_bss_68);
    }
}

/* Advances the hanging chain's segment positions and velocities on each update. */
void fn_1_CBE8(void)
{
    Point3d previousPosition;
    Point3d difference;
    f32 distance;
    f32 extension;
    f32 springForce;
    s32 vertexIndex;

    previousPosition = lbl_1_bss_7C[0].position;
    for (vertexIndex = 1; vertexIndex < 20; vertexIndex++) {
        lbl_1_bss_7C[vertexIndex].velocity.y -= (65.33333674073221);
        PSVECSubtract(&lbl_1_bss_7C[vertexIndex].position, &previousPosition, &difference);
        distance = sqrt(difference.x * difference.x + difference.y * difference.y +
                        difference.z * difference.z);
        extension = distance - (60.0);
        springForce = (300.0f + vertexIndex) * extension;
        difference.x /= distance;
        difference.y /= distance;
        difference.z /= distance;
        lbl_1_bss_7C[vertexIndex].velocity.x +=
            (0.01666666753590107f) * (-difference.x * springForce);
        lbl_1_bss_7C[vertexIndex].velocity.y +=
            (0.01666666753590107f) * (-difference.y * springForce);
        lbl_1_bss_7C[vertexIndex].velocity.z +=
            (0.01666666753590107f) * (-difference.z * springForce);
        lbl_1_bss_7C[vertexIndex].position.x +=
            (0.01666666753590107f) * lbl_1_bss_7C[vertexIndex].velocity.x;
        lbl_1_bss_7C[vertexIndex].position.y +=
            (0.01666666753590107f) * lbl_1_bss_7C[vertexIndex].velocity.y;
        lbl_1_bss_7C[vertexIndex].position.z +=
            (0.01666666753590107f) * lbl_1_bss_7C[vertexIndex].velocity.z;
        PSVECSubtract(&lbl_1_bss_7C[vertexIndex].position, &previousPosition, &difference);
        /* This second distance is measured but not used to constrain the segment. */
        distance = PSVECMag(&difference);
        PSVECNormalize(&difference, &difference);
        lbl_1_bss_7C[vertexIndex].position.x =
            previousPosition.x + (60.0) * difference.x;
        lbl_1_bss_7C[vertexIndex].position.y =
            previousPosition.y + (60.0) * difference.y;
        lbl_1_bss_7C[vertexIndex].position.z =
            previousPosition.z + (60.0) * difference.z;
        lbl_1_bss_7C[vertexIndex].velocity.x *= (0.9200000166893005f);
        lbl_1_bss_7C[vertexIndex].velocity.y *= (0.9200000166893005f);
        lbl_1_bss_7C[vertexIndex].velocity.z *= (0.9200000166893005f);
        previousPosition = lbl_1_bss_7C[vertexIndex].position;
    }
}

#include "dolphin.h"
#include "game/board/effect.h"

typedef struct W06ParticlePalette {
    GXColor colors[5];
} W06ParticlePalette;

extern W06RingVertex lbl_1_bss_7C[20];
extern Point3d lbl_1_bss_68;

extern u32 mbRandMod(u32 max);

/* Recomputes the hanging chain's segment angles as its endpoints move. */
void fn_1_D184(void)
{
    Point3d topDelta;
    Point3d centerDelta;
    W06RingVertex *vertex;
    f32 topAngle;
    f32 centerAngle;
    f32 angleDelta;
    s32 ringIndex;
    s32 vertexIndex;
    W06RingVertex *ringVertex;

    ringVertex = &lbl_1_bss_7C[18];
    ringIndex = 18;
    while (ringIndex >= 0) {
        vertex = &lbl_1_bss_7C[1];
        vertexIndex = 1;
        while (vertexIndex < 20) {
            vertex->position.x = (f32)((f64)vertex[-1].position.x +
                                        (60.0) * cos((f64)vertex[-1].angle));
            vertex->position.y = (f32)((f64)vertex[-1].position.y +
                                        (60.0) * sin((f64)vertex[-1].angle));
            vertexIndex++;
            vertex++;
        }
        PSVECSubtract(&lbl_1_bss_7C[19].position, &ringVertex->position, &topDelta);
        PSVECSubtract(&lbl_1_bss_68, &ringVertex->position, &centerDelta);
        topAngle = (f32)atan2((f64)topDelta.y, (f64)topDelta.x);
        centerAngle = (f32)atan2((f64)centerDelta.y, (f64)centerDelta.x);
        angleDelta = centerAngle - topAngle;
        ringVertex->angle += angleDelta;
        ringIndex--;
        ringVertex--;
    }
}

/* Hu3D calls this render hook to draw the hanging chain as a white line strip. */
void fn_1_D2E4(s32 unused, f32 (*matrix)[4])
{
    s32 vertexIndex;

    GXLoadPosMtxImm(matrix, 0);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetNumTevStages(1);
    GXSetNumTexGens(0);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetZMode(1, GX_ALWAYS, 1);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, 0, GX_SRC_VTX, GX_SRC_VTX, 1, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetZCompLoc(0);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ONE, GX_LO_NOOP);
    GXSetAlphaUpdate(1);
    GXBegin(GX_LINESTRIP, GX_VTXFMT0, 20);
    for (vertexIndex = 0; vertexIndex < 20; vertexIndex++) {
        GXPosition3f32(lbl_1_bss_7C[vertexIndex].position.x,
                       lbl_1_bss_7C[vertexIndex].position.y,
                       lbl_1_bss_7C[vertexIndex].position.z);
        GXColor4u8(255, 255, 255, 255);
    }
}

/* Particle hooks installed by the board's white-burst effects call this each active frame. */

void fn_1_D4C8(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix)
{
    Point3d direction;
    f32 angle;
    f32 heightScale;
    s32 particleIndex;
    MBPARTICLEDATA *particle;

    if (effect->count == 0) {
        particle = effect->data;
        for (particleIndex = 0; particleIndex < effect->num; particleIndex++, particle++) {
            particle->time = 1;
            particle->activeF = (50.0f) *
                ((0.4000000059604645f) + (0.30000001192092896f) *
                    ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK)));
            angle = (360.0f) *
                ((3.0517578125e-05f) * (frand() & W06_RANDOM_U16_MASK) - (1.0f));
            direction.x = sin((3.141592653589793) * angle / (180.0));
            direction.z = cos((3.141592653589793) * angle / (180.0));
            direction.y = (1.0) +
                sin((3.141592653589793) *
                    ((360.0f) * ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK))) /
                    (180.0));
            if (direction.y > (1.0f)) {
                direction.y -= (2.0f);
            }
            heightScale = (0.5f) + (0.5f) * direction.y;
            heightScale *= (0.699999988079071f);
            PSVECScale(&direction, &particle->pos,
                (100.0f) * ((0.800000011920929f) *
                    ((0.5f) + (0.5f) *
                        ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK)))));
            PSVECScale(&direction, &particle->vel,
                (2.0f) * ((0.699999988079071f) + (0.30000001192092896f) *
                    ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK))));
            particle->rot.z = (360.0f) *
                ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK));
            particle->scale = (60.0f) + (60.0f) *
                ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK));
            particle->color.a = (20.0f) + (80.0f) *
                ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK));
            /* The extra random sample and its derived value are discarded. */
            angle = (0.699999988079071f) * heightScale + (0.30000001192092896f) *
                ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK));
            particle->color.r = 255;
            particle->color.g = 255;
            particle->color.b = 255;
        }
        effect->count = 1;
    }

    particle = effect->data;
    for (particleIndex = 0; particleIndex < effect->num; particleIndex++, particle++) {
        if (particle->time != 0) {
            PSVECAdd(&particle->pos, &particle->vel, &particle->pos);
            particle->vel.y += (0.3919999897480011f);
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = (0.0f);
                particle->color.a = 0;
            } else if (particle->activeF < 10 && particle->color.a >= 10) {
                particle->color.a -= 10;
            }
            if (particle->color.a >= 10) {
                particle->color.a -= 2;
            }
            particle->scale += (4.0f);
        }
    }
}

/* Initializes a colored burst, then advances and fades its particles each frame. */
static const W06ParticlePalette lbl_1_rodata_25C = {
    {
        {255, 255, 255, 255},
        {255, 255, 0, 255},
        {128, 255, 0, 255},
        {128, 255, 255, 255},
        {255, 128, 255, 255}
    }
};

/* Particle hooks installed on the colored board effects call this each active frame. */
void fn_1_DACC(HU3D_MODEL *model, MBPARTICLE *effect, Mtx matrix)
{
    W06ParticlePalette palette = lbl_1_rodata_25C;
    MBPARTICLEDATA *particle;
    s32 particleIndex;
    s32 activeCount;
    f32 angle;
    f32 radius;
    f32 elevation;

    if (effect->mode == 0) {
        particle = effect->data;
        for (particleIndex = 0; particleIndex < effect->num; particleIndex++, particle++) {
            particle->activeF = (50.0f) *
                ((0.30000001192092896f) + (0.4000000059604645f) *
                    ((1.52587890625e-05f) * (frand() & W06_RANDOM_U16_MASK)));
            angle = (360.0f) * frandf();
            elevation = (45.0f) * frandf();
            radius = (100.0f) * ((0.5f) * frandf());
            particle->pos.x = radius * cos((3.141592653589793) * angle / (180.0));
            particle->pos.y = (0.0f);
            particle->pos.z = radius * sin((3.141592653589793) * angle / (180.0));
            particle->vel.x = cos((3.141592653589793) * angle / (180.0)) *
                ((3000.0) *
                    ((0.5) + sin((3.141592653589793) * elevation / (180.0))));
            particle->vel.y = sin((3.141592653589793) * angle / (180.0)) *
                ((3000.0) *
                    ((0.5) + sin((3.141592653589793) * elevation / (180.0))));
            particle->vel.z = (0.0f);
            particle->scale = (30.0f) + (30.0f) * frandf();
            particle->color = palette.colors[mbRandMod(5)];
        }
        effect->mode = 1;
    }

    activeCount = 0;
    particle = effect->data;
    for (particleIndex = 0; particleIndex < effect->num; particleIndex++, particle++) {
        if (particle->activeF != 0) {
            particle->pos.x += (0.01666666753590107f) * particle->vel.x;
            particle->pos.y += (0.01666666753590107f) * particle->vel.y;
            particle->pos.z += (0.01666666753590107f) * particle->vel.z;
            particle->vel.x *= (0.8999999761581421f);
            particle->vel.y *= (0.8999999761581421f);
            particle->vel.z *= (0.8999999761581421f);
            particle->activeF--;
            if (particle->activeF <= 0) {
                particle->time = 0;
                particle->scale = (0.0f);
                particle->color.a = 0;
            } else if (particle->activeF < 10 && particle->color.a >= 10) {
                particle->color.a -= 5;
            }
            if (particle->color.a >= 10) {
                particle->color.a -= 2;
            }
            activeCount++;
        }
    }
}

#include "game/board/masu.h"
#include "game/gamework.h"

extern u32 lbl_1_data_84[];
typedef struct {
    s32 modelId;
    s32 spriteType;
} W06MapSpriteWork;

int mbSNpcMasuGet(void);
void mbMapSprAdd(s32 spriteNo, s16 masuId);

/* Adds the star and special entrance markers while the board map is being drawn. */
void fn_1_E004(s32 enable)
{
    s32 entryIndex;
    s16 masuId;
    s32 spriteType;

    if (enable != 0) {
        if (GwSystem.curTime == 0) {
            mbMapSprAdd(14, mbSNpcMasuGet());
        } else {
            mbMapSprAdd(15, mbSNpcMasuGet());
        }
        entryIndex = 0;
        while (entryIndex < 6) {
            masuId = mbMasuFind_MAttrIdGet(-1, lbl_1_data_84[entryIndex]);
            spriteType = lbl_1_bss_404.displayObjects[entryIndex].dataId;
            switch (spriteType) {
            case 1:
                mbMapSprAdd(16, masuId);
                break;
            case 2:
                mbMapSprAdd(17, masuId);
                break;
            }
            entryIndex++;
        }
    }
}

#include "dolphin.h"
#include "game/board/audio.h"
#include "game/board/camera.h"
#include "game/board/opening.h"
#include "game/board/player.h"
#include "game/board/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

HuVecF lbl_1_data_804 = {0.0f, 380.0f, 4681.0f};
HuVecF lbl_1_data_810 = {-35.0f, 0.0f, 0.0f};
float lbl_1_data_81C = 2400.0f;

/* Delivers Clockwork Castle's opening board instructions through the opening hook. */
void fn_1_E104(void)
{
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 38), 6);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 39), 6);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 40), 6);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 41), 6);
    mbWinTopWait();
    mbWipeDissolveFadeOutTime(1);
    mbCameraZoomSet(lbl_1_data_81C);
    mbCameraRotSetV(&lbl_1_data_810);
    mbCameraCenterSetV(&lbl_1_data_804);
    mbWipeDissolveFadeIn();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 42), 6);
    mbWinTopWait();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 43), 6);
    mbWinTopWait();
    mbWipeDissolveFadeOutTime(1);
    mbOpeningCameraPosRestore();
    mbWipeDissolveFadeIn();
}

/* Introduces the star guide and restores the opening camera before the first turn. */
void fn_1_E224(void)
{
    Point3d openingMasuPos;
    Point3d npcMasuPos;
    s16 npcMasuId;
    s16 openingMasuId;
    s32 playerNo;
    s32 winId;

    mbWipeFadeOut();
    for (playerNo = 0; playerNo < 4; playerNo++) {
        mbPlayerMotionShiftSet(playerNo, 1, (0.0f), (1.0f), HU3D_MOTATTR_LOOP);
    }
    mbCameraZoomSet((2400.0f));
    mbCameraRotSet((-35.0f), (0.0f), (0.0f));
    openingMasuId = mbMasuFind_AttrIdGet(-1, (1 << 15));
    mbMasuPosGet(openingMasuId, &openingMasuPos);
    npcMasuId = mbSNpcMasuGet();
    mbMasuPosGet(npcMasuId, &npcMasuPos);
    mbWipeFadeIn();
    HuPrcSleep(10);
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    winId = mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 44), 6);
    mbWinPause(winId);
    mbCameraMovePos(&npcMasuPos, NULL, NULL, (3200.0f), (-1.0f), 150);
    mbCameraMoveWait();
    mbWinKill(winId);
    HuPrcSleep(20);
    mbCameraMovePos(NULL, NULL, NULL, (2400.0f), (-1.0f), 30);
    mbCameraMoveWait();
    mbSNpcMotWinSet();
    mbAudGuidePlay(MSM_SE_GUIDE_28);
    mbWinCreate(2, MESSNUM(MESS_BOARD_W06, 45), 6);
    mbWinTopWait();
    mbWipeFadeOut();
    mbSNpcMotIdleSet();
    mbOpeningGuidePosRestore();
    mbOpeningCameraPosRestore();
    mbWipeFadeIn();
}
