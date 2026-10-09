/* Owns Hyper Sniper setup, shared sequence state, camera motion, and phase hooks. */
#include "math.h"
#include "REL/m646Dll/module_types.h"

#define M646_OPENING_CAMERA_MOTION_FILE 0
#define M646_ALTERNATE_CAMERA_MOTION_FILE 1
#define M646_CAMERA_OBJECT_PRIORITY 32730
#define M646_MUSIC_FADE_SPEED 100
#define M646_DECA_RESULT_FRAMES 90
#define M646_PLAYER_WINNER_BIT 0x1

extern M646MainData *lbl_1_data_0;
extern M646MainData lbl_1_bss_0;
extern s32 lbl_1_data_4[2];

void fn_1_310(OMOBJ *obj);
void fn_1_543C(OMOBJMAN *objman);
void fn_1_2B50(OMOBJMAN *objman);
void fn_1_4E28(OMOBJMAN *objman);
void fn_1_AFC(OMOBJMAN *objman, s16 player);
void fn_1_8E78(OMOBJMAN *objman);
void fn_1_98E0(OMOBJMAN *objman);
extern MGSEQ_PARAM lbl_1_data_C;
void fn_1_5564(s16 player, s32 state);
void fn_1_5964(s16 player, s32 command);
void fn_1_9024(void);
u16 fn_1_8FD4(void);
void fn_1_9118(void);
s32 fn_1_A0D4(void);
s32 fn_1_A0FC(void);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_4D4(void);

void fn_1_0(void);
void fn_1_90(void);
void fn_1_94(void);
void fn_1_E0(OMOBJMAN *objman);
void fn_1_1E0(void);
void fn_1_1E4(s32 camera);
s32 fn_1_2CC(void);
void fn_1_310(OMOBJ *obj);
void fn_1_314(OMOBJ *obj);
void fn_1_318(void);
void fn_1_430(void);
int _prolog(void);
void _epilog(void);
void fn_1_4D4(void);
void fn_1_858(s16 mode, s16 frame);
void fn_1_878(s16 mode, s16 frame);
void fn_1_8A4(s16 mode, s16 frame);
void fn_1_908(s16 mode, s16 frame);
void fn_1_934(s16 mode, s16 frame);
void fn_1_9A0(s16 mode, s16 frame);
void fn_1_AB4(s16 mode, s16 frame);

M646MainData *lbl_1_data_0 = &lbl_1_bss_0;

s32 lbl_1_data_4[2] = {
    DATANUM(DATA_m646, M646_OPENING_CAMERA_MOTION_FILE),
    DATANUM(DATA_m646, M646_ALTERNATE_CAMERA_MOTION_FILE)
};

void fn_1_AF4(s16 mode, s16 frame);

void fn_1_AF8(s16 mode, s16 frame);

MGSEQ_PARAM lbl_1_data_C = {30, 0,
     fn_1_858, fn_1_878, fn_1_8A4, fn_1_908, fn_1_934, fn_1_9A0, fn_1_AB4, fn_1_AF4, fn_1_AF8};

M646MainData lbl_1_bss_0;

/* Resets the sequence record and snapshots the current night setting before setup. */
void fn_1_0(void)
{
    s16 nightMode;
    memset(&lbl_1_bss_0, 0, sizeof(M646MainData));
    lbl_1_data_0->streamId = -1;
    nightMode = GwMgNightF;
    lbl_1_data_0->nightFlag = nightMode;
    lbl_1_data_0->resetValue = 0;
    lbl_1_data_0->unknown20 = 0;
}

void fn_1_90(void)
{
}

/* Sequence setup creates the object manager and initializes the game system. */
void fn_1_94(void)
{
    lbl_1_data_0->objectManager = omInitObjMan(160, 1000);
    omGameSysInit(lbl_1_data_0->objectManager);
}

/* Creates the minigame camera and its camera-motion object during setup. */
void fn_1_E0(OMOBJMAN *objman)
{
    s32 motionIndex = 0;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, (90.0f), (20.0f),
                            (10000.0f), (1.2f));
    lbl_1_data_0->camera.object =
        omAddObjEx(objman, M646_CAMERA_OBJECT_PRIORITY, 0, 0, OM_GRP_NONE, fn_1_310);
    for (motionIndex = 0; motionIndex < 2; motionIndex++) {
        lbl_1_data_0->camera.motions[motionIndex] = Hu3DMotionCreate(
            HuDataSelHeapReadNum(lbl_1_data_4[motionIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_data_0->camera.model = -1;
}

void fn_1_1E0(void)
{
}

/* The opening coordinator replaces and starts the camera motion before staging the players. */
void fn_1_1E4(s32 cameraMotionIndex)
{
    if (lbl_1_data_0->camera.model != -1) {
        Hu3DModelKill(lbl_1_data_0->camera.model);
    }
    lbl_1_data_0->camera.model =
        Hu3DModelCameraCreate(lbl_1_data_0->camera.motions[cameraMotionIndex], 1);
    Hu3DCameraMotionStart(lbl_1_data_0->camera.model, 1);
    if (cameraMotionIndex == 0) {
        Hu3DMotionSpeedSet(lbl_1_data_0->camera.model, (1.5f));
    } else {
        Hu3DMotionSpeedSet(lbl_1_data_0->camera.model, (1.0f));
    }
}

/* Target selection polls this to detect when the active camera motion ends. */
s32 fn_1_2CC(void)
{
    if (Hu3DMotionEndCheck(lbl_1_data_0->camera.model) != 0) {
        return 1;
    }
    return 0;
}

void fn_1_310(OMOBJ *obj)
{
}

void fn_1_314(OMOBJ *obj)
{
}

/* Creates the stage light and sets its position, direction, and color. */
void fn_1_318(void)
{
    HU3D_LIGHTID light;
    GXColor color = {255, 255, 255, 255};
    HuVecF position = {0.0f, 1010.0f, -100.0f};
    HuVecF direction = {0.3f, -0.8f, 0.3f};
    HuVecF initialColorSource = {20.0f, 45.0f, 1000.0f};

    /* Creation reads the vector's first four bytes as color; the color is then set to white. */
    light = lbl_1_data_0->lightId =
        Hu3DGLightCreateV(&position, &direction, (GXColor *) &initialColorSource);
    Hu3DGLightPointSet(light, (1000.0f), (1.0f), GX_DA_MEDIUM);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightColorSet(light, color.r, color.g, color.b, color.a);
}

void fn_1_430(void)
{
}

/* Runs the module's registered constructors before the minigame setup callbacks. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_4D4();
    return 0;
}

/* Runs the module's registered destructors when the minigame module shuts down. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Initializes the camera, stage, player, collision, score, and targeting objects. */
void fn_1_4D4(void)
{
    OMOBJMAN *cameraManager;
    fn_1_0();
    fn_1_94();
    cameraManager = lbl_1_data_0->objectManager;
    fn_1_E0(cameraManager);
    fn_1_318();
    fn_1_543C(lbl_1_data_0->objectManager);
    fn_1_2B50(lbl_1_data_0->objectManager);
    fn_1_4E28(lbl_1_data_0->objectManager);
    fn_1_AFC(lbl_1_data_0->objectManager, 0);
    fn_1_AFC(lbl_1_data_0->objectManager, 1);
    fn_1_AFC(lbl_1_data_0->objectManager, 2);
    fn_1_AFC(lbl_1_data_0->objectManager, 3);
    fn_1_8E78(lbl_1_data_0->objectManager);
    fn_1_98E0(lbl_1_data_0->objectManager);
    MgSeqCreate(&lbl_1_data_C);
}

/* Sequence init hook: lets the manager advance immediately after initialization. */
void fn_1_858(s16 sequenceMode, s16 phaseFrame)
{
    MgSeqModeNext();
}

/* Waits for the opening camera and four launch events to complete their staged sequence. */
void fn_1_878(s16 sequenceMode, s16 phaseFrame)
{
    if (fn_1_A0D4() != 0) {
        MgSeqModeNext();
    }
}

/* Start hook: begins the minigame stream once the start message audio is ready. */
void fn_1_8A4(s16 sequenceMode, s16 phaseFrame)
{
    if ((lbl_1_data_0->streamId == -1) &&
        ((s32)(GameMesStatGet(MgSeqGameMesIdGet()) & GAMEMES_STAT_FXPLAY) != 0)) {
        lbl_1_data_0->streamId = HuAudSStreamPlay(MSM_STREAM_MGMUS_26);
    }
}

/* Main-play hook: advances the sequence when its countdown reaches zero. */
void fn_1_908(s16 sequenceMode, s16 phaseFrame)
{
    if (MgSeqTimerValueGet() == 0U) {
        MgSeqModeNext();
    }
}

/* Finish hook: fades the minigame stream on the first frame after play ends. */
void fn_1_934(s16 sequenceMode, s16 phaseFrame)
{
    if ((MgSeqFrameNoGet() == 0) && (lbl_1_data_0->streamId != -1)) {
        HuAudSStreamFadeOut(lbl_1_data_0->streamId, M646_MUSIC_FADE_SPEED);
        lbl_1_data_0->streamId = -1;
    }
}

/* Sets winner-state flags and rankings, then waits for all players' fixed-angle turns to finish. */
void fn_1_9A0(s16 sequenceMode, s16 phaseFrame)
{
    s16 playerIndex;
    s16 winnerCount = 0;
    s16 winningCharacters[4];
    s16 winnerMask;

    if (MgSeqFrameNoGet() == 0) {
        fn_1_9024();
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            winningCharacters[playerIndex] = -1;
            fn_1_5564(playerIndex, 0);
            fn_1_5964(playerIndex, 0);
        }
        winnerMask = fn_1_8FD4();
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            if ((winnerMask & (M646_PLAYER_WINNER_BIT << playerIndex)) != 0) {
                winningCharacters[winnerCount] = GwPlayerConf[playerIndex].charNo;
                fn_1_5564(playerIndex, 1);
                winnerCount++;
            }
        }
        fn_1_9118();
    }
    if (fn_1_A0FC() != 0) {
        MgSeqModeNext();
    }
}

/* Winner hook: advances after 90 frames when Decathlon mode is active. */
void fn_1_AB4(s16 sequenceMode, s16 phaseFrame)
{
    if ((_CheckFlag(FLAG_INST_DECA) != 0) && (MgSeqFrameNoGet() == M646_DECA_RESULT_FRAMES)) {
        MgSeqModeNext();
    }
}
