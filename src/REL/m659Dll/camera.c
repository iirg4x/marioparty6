/* Camera, sequence, lighting, and player-startup logic for Asteroad Rage. */
#define M659_STREAM_COURSE 87
#define M659_DATA_FINISH_GRID_MODEL DATANUM(DATA_m659, 20)
#define M659_OBJECT_CAPACITY 200
#define M659_OBJECT_MANAGER_PRIORITY 131072
#include "game/object.h"
#include "game/memory.h"
#include "string.h"
#include "REL/m659/typed-context.h"
#include "REL/m659/player.h"

M659Work *lbl_1_data_0 = &lbl_1_bss_4;
static int lbl_1_data_4[2] = {1, 2};
M659Viewport lbl_1_data_C[2] = {
    {0, 0, 318, 480}, {0, 322, 318, 480}
};
extern HU3D_LIGHTID lbl_1_bss_0;
typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void fn_1_804(void);
#include "game/mg/seqman.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "game/flag.h"
#include "game/wipe.h"
#include "game/data.h"

void fn_1_B2C(s16 mode, s16 frameNo);
void fn_1_BF0(s16 mode, s16 frameNo);
void fn_1_C40(s16 mode, s16 frameNo);
void fn_1_C44(s16 mode, s16 frameNo);
void fn_1_D4C(s16 mode, s16 frameNo);
void fn_1_EF8(s16 mode, s16 frameNo);
void fn_1_1408(s16 mode, s16 frameNo);
void fn_1_144C(s16 mode, s16 frameNo);
void fn_1_1450(s16 mode, s16 frameNo);
MGSEQ_PARAM lbl_1_data_2C = {
    30, MGSEQ_TIMER_TOP,
    fn_1_B2C, fn_1_BF0, fn_1_C40, fn_1_C44, fn_1_D4C,
    fn_1_EF8, fn_1_1408, fn_1_144C, fn_1_1450
};
extern HU3D_MODELID lbl_1_bss_28;
void fn_1_1454(OMOBJMAN *manager);
void fn_1_41F4(OMOBJMAN *manager);
void fn_1_4E68(OMOBJMAN *manager);
void fn_1_5944(OMOBJMAN *manager);
void fn_1_1610(s16 side, u16 camera);
void fn_1_16A8(s16 side, int result);
void fn_1_16E4(void);
void fn_1_1790(s16 side);
void fn_1_17E8(s16 side);
void fn_1_2648(OMOBJ *obj);
void fn_1_3328(void);
void fn_1_57BC(void);

/* Clears shared game state before the minigame objects are created. */
void fn_1_0(void)
{
    memset(&lbl_1_bss_4, 0, sizeof(M659Work));
    lbl_1_data_0->streamHandle = -1;
    lbl_1_data_0->resultPhase = 0;
}

void fn_1_58(void)
{
}

/* Creates the minigame object manager and initializes its game-system objects. */
void fn_1_5C(void)
{
    lbl_1_data_0->objman = omInitObjMan(M659_OBJECT_CAPACITY, M659_OBJECT_MANAGER_PRIORITY);
    omGameSysInit(lbl_1_data_0->objman);
}

/* Sets the result camera position and target when the results sequence enters its camera phase. */
void fn_1_A8(void)
{
    M659Camera *camera = lbl_1_data_0->cameraObj[0]->data;
    HuVecF initialCameraOffset = {0, 0, -800}; /* Initialized default camera offset; this
                                                * result-camera setup does not use it. */
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 90.0f, 20.0f, 100000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0, 0, 640, 480, 0, 1);
    camera->position.x = 0;
    camera->position.y = 191;
    camera->position.z = -434;
    camera->targetPosition.x = 0;
    camera->targetPosition.y = 0;
    camera->targetPosition.z = 1000;
    Hu3DCameraPosSet(camera->cameraMask,
        camera->position.x, camera->position.y, camera->position.z,
        0, 1, 0, camera->targetPosition.x, camera->targetPosition.y, camera->targetPosition.z);
    lbl_1_data_0->cameraObj[0]->objFunc = fn_1_558;
}

/* Creates both split-screen camera objects and initializes their viewport and projection
 * settings. */
void fn_1_254(OMOBJMAN *manager)
{
    M659Camera *camera = NULL;
    OMOBJ *obj = NULL;
    M659Viewport *viewport;
    int i = 0;
    for (i = 0; i < 2; i++) {
        viewport = &lbl_1_data_C[i];
        obj = lbl_1_data_0->cameraObj[i] = omAddObjEx(manager, 32730, 0, 0, -1, fn_1_408);
        camera = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Camera), HU_MEMNUM_OVL);
        obj->data = camera;
        memset(camera, 0, sizeof(M659Camera));
        camera->cameraMask = lbl_1_data_4[i];
        Hu3DCameraCreate(camera->cameraMask);
        Hu3DCameraPerspectiveSet(camera->cameraMask, 90, 20, 50000, 0.6f);
        Hu3DCameraViewportSet(camera->cameraMask,
            viewport->x, viewport->y, viewport->width, viewport->height, 0, 1);
        Hu3DCameraScissorSet(camera->cameraMask, viewport->x, viewport->y, 318, 480);
    }
}

void fn_1_3CC(void)
{
}

/* Assigns the player object that a split-screen camera follows. */
void fn_1_3D0(s16 index, OMOBJ *target)
{
    M659Camera *camera = lbl_1_data_0->cameraObj[index]->data;
    camera->target = target;
}

/* Initializes a new camera object and hands its per-frame updates to fn_1_48C. */
void fn_1_408(OMOBJ *obj)
{
    M659Camera *camera = obj->data;
    camera->position.x = 0;
    camera->position.y = 600;
    camera->position.z = -650;
    camera->targetPosition.x = 0;
    camera->targetPosition.y = -5000;
    camera->targetPosition.z = 2000;
    obj->objFunc = fn_1_48C;
}

/* Updates the split-screen camera's horizontal position from its target ship while keeping the
 * other view coordinates fixed. */
void fn_1_48C(OMOBJ *obj)
{
    M659Camera *camera = obj->data;
    M659Player *playerData = camera->target->data;
    M659PlayerInfo *targetPlayerInfo = &playerData->info;
    M659Motion *motion = &playerData->motion;
    s16 cameraSideIndex = camera->cameraMask - 1;
    float cameraOffsetDefaults[2] = {0, 0};
    Hu3DCameraPosSet(camera->cameraMask, motion->position.x, camera->position.y,
        camera->position.z, 0, 1, 0, camera->targetPosition.x, camera->targetPosition.y, 17000);
}

void fn_1_558(OMOBJ *obj)
{
}

/* Creates the course light during minigame startup. */
void fn_1_55C(void)
{
    HU3D_LIGHTID light;
    GXColor color = {255, 255, 255, 255};
    HuVecF pos = {0, 1000, -500};
    HuVecF dir = {0.3f, -0.8f, 0.3f};
    HuVecF courseLightParameters = {20, 45, 1000};

    light = lbl_1_data_0->light = Hu3DGLightCreateV(&pos, &dir, &color);
    Hu3DGLightPointSet(light, 6000, 0.5f, GX_DA_MEDIUM);
    Hu3DGLightStaticSet(light, TRUE);
}

void fn_1_65C(void)
{
}

/* Replaces the scene lighting for the result presentation. */
void fn_1_660(void)
{
    Hu3DLightAllKill();
    {
        GXColor color = {255, 255, 255, 255};
        HuVecF pos = {0, 1000, -500};
        HuVecF dir = {0, -0.8f, 0};
        lbl_1_bss_0 = Hu3DGLightCreateV(&pos, &dir, &color);
        Hu3DGLightPointSet(lbl_1_bss_0, 1000, 1.0f, GX_DA_MEDIUM);
        Hu3DGLightStaticSet(lbl_1_bss_0, TRUE);
        Hu3DGLightColorSet(lbl_1_bss_0, color.r, color.g, color.b, color.a);
    }
}

/* Runs the module constructors and then initializes the minigame. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_804();
    return 0;
}

/* Runs the module destructors when the minigame module is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates minigame systems and scene objects, then starts the minigame sequence. */
void fn_1_804(void)
{
    fn_1_0();
    fn_1_5C();
    fn_1_254(lbl_1_data_0->objman);
    fn_1_55C();
    fn_1_1454(lbl_1_data_0->objman);
    fn_1_41F4(lbl_1_data_0->objman);
    fn_1_4E68(lbl_1_data_0->objman);
    fn_1_5944(lbl_1_data_0->objman);
    MgSeqCreate(&lbl_1_data_2C);
}

/* Sequence callback that assigns each active side its player object before play. */
void fn_1_B2C(s16 mode, s16 frameNo)
{
    s16 player;
    s16 side;
    for (player = 0; player < 4; player++) {
        side = GwPlayerConf[player].grpNo;
        if (side == 0 || side == 1) {
            fn_1_3D0(side, lbl_1_bss_2C[side]);
        }
    }
    MgSeqModeNext();
}

/* Sequence callback that starts the course music and advances after 80 frames. */
void fn_1_BF0(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0) {
        lbl_1_data_0->streamHandle = HuAudSStreamPlay(M659_STREAM_COURSE);
    }
    if (MgSeqFrameNoGet() == 80) {
        MgSeqModeNext();
    }
}

void fn_1_C40(s16 mode, s16 frameNo)
{
}

/* Sequence callback that checks both sides for completion and records the remaining winner
 * sides. */
void fn_1_C44(s16 mode, s16 frameNo)
{
    s16 side = 0;
    s16 done = 0;
    lbl_1_data_0->winners[0] = -1;
    lbl_1_data_0->winners[1] = -1;
    lbl_1_data_0->winnerCount = 0;
    if (MgSeqTimerValueGet() == 0U) {
        MgSeqModeNext();
        return;
    }
    for (side = 0; side < 2; side++) {
        if (fn_1_1840(side)) {
            done++;
        } else {
            lbl_1_data_0->winners[lbl_1_data_0->winnerCount] = side;
            lbl_1_data_0->winnerCount++;
        }
    }
    if (done > 0) {
        MgSeqModeNext();
        return;
    }
}

/* Sequence callback that stops course music, marks player results, and sets the game winner. */
void fn_1_D4C(s16 mode, s16 frameNo)
{
    s16 side;
    u16 sequenceResult;
    if (MgSeqFrameNoGet() == 0) {
        if (lbl_1_data_0->streamHandle != -1) {
            HuAudSStreamFadeOut(lbl_1_data_0->streamHandle, 100);
            lbl_1_data_0->streamHandle = -1;
        }
        for (side = 0; side < 2; side++) {
            fn_1_16A8(side, 0);
            if (!fn_1_1840(side)) {
                fn_1_16A8(side, 1);
            } else {
                fn_1_16A8(side, 0);
                lbl_1_data_0->otherPlayer = side;
            }
        }
        switch (lbl_1_data_0->winnerCount) {
        case 0:
        case 2:
            /* The returned status is not used here. */
            sequenceResult = MgSeqWinnerSet(-1, -1, -1, -1);
            break;
        case 1:
            {
                M659Player *playerData = lbl_1_bss_2C[lbl_1_data_0->winners[0]]->data;
                int playerNo;
                /* The returned status is not used here. */
                sequenceResult = MgSeqWinnerSet(playerData->info.charNo, -1, -1, -1);
                playerNo = playerData->info.playerNo;
                if (!_CheckFlag(FLAG_MG_PRACTICE)) {
                    GwPlayer[playerNo].mgCoinBonus = 10;
                }
            }
            break;
        }
    }
}

/* Sequence callback that runs the result transition, camera and lighting changes, and end delay. */
void fn_1_EF8(s16 mode, s16 frameNo)
{
    s16 initialZeroA = 0, initialZeroB = 0; /* Both are initialized to zero but not read by this
                                             * sequence callback. */
    int i;
    if (lbl_1_data_0->winnerCount == 0 && lbl_1_data_0->resultPhase < 5) {
        lbl_1_data_0->resultTimer = 30;
        lbl_1_data_0->resultPhase = 5;
    }
    switch (lbl_1_data_0->resultPhase) {
    case 0:
        lbl_1_data_0->resultPhase++;
        break;
    case 1:
        WipeCreate(2, 0, 60);
        for (i = 0; i < 60; i++) {
            HuPrcVSleep();
        }
        Hu3DCameraKill(2);
        lbl_1_data_0->resultPhase++;
        break;
    case 2:
        fn_1_A8();
        fn_1_3328();
        fn_1_57BC();
        fn_1_660();
        for (i = 0; i < 2; i++) {
            fn_1_1610(i, 1);
        }
        if (lbl_1_data_0->winnerCount == 1) {
            fn_1_1790(lbl_1_data_0->winners[0]);
            fn_1_17E8(lbl_1_data_0->otherPlayer);
        }
        lbl_1_data_0->resultPhase++;
        break;
    case 3:
        WipeCreate(1, 5, 60);
        for (i = 0; i < 60; i++) {
            HuPrcVSleep();
        }
        lbl_1_data_0->resultPhase++;
        break;
    case 4:
        lbl_1_data_0->resultTimer = 0;
        lbl_1_data_0->resultPhase++;
        /* fallthrough */
    case 5:
        if ((float)lbl_1_data_0->resultTimer > 60.0f) {
            lbl_1_data_0->resultPhase++;
        }
        lbl_1_data_0->resultTimer++;
        break;
    case 6:
        MgSeqModeNext();
        break;
    }
}

/* Sequence callback that starts the winner animation at the beginning of its result mode. */
void fn_1_1408(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0 && lbl_1_data_0->winnerCount != 0) {
        fn_1_16E4();
    }
}

void fn_1_144C(s16 mode, s16 frameNo)
{
}

void fn_1_1450(s16 mode, s16 frameNo)
{
}

/* Creates player objects and the shared finish model during minigame startup. */
void fn_1_1454(OMOBJMAN *manager)
{
    OMOBJ *obj = NULL;
    M659Player *playerData = NULL;
    int player = 0, side = 0;
    for (player = 0; player < 4; player++) {
        side = GwPlayerConf[player].grpNo;
        if (side == 0 || side == 1) {
            obj = lbl_1_bss_2C[side] = omAddObjEx(manager, 110, 5, 10, -1, fn_1_2648);
    playerData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Player), HU_MEMNUM_OVL);
            obj->data = playerData;
            memset(playerData, 0, sizeof(M659Player));
            playerData->info.playerNo = player;
            playerData->info.side = side;
            playerData->info.engineSoundHandle = -1;
            playerData->com.isCom = GwPlayerConf[player].type;
            playerData->com.difficulty = -1;
            if (playerData->com.isCom) {
                playerData->com.difficulty = GwPlayerConf[player].comDif;
            }
        }
    }
    lbl_1_bss_28 = Hu3DModelCreate(
        HuDataSelHeapReadNum(M659_DATA_FINISH_GRID_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_28, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrReset(lbl_1_bss_28, HU3D_MOTATTR_LOOP);
}

/* Removes character models when the minigame character resources are no longer needed. */
void fn_1_15EC(void)
{
    CharModelKill(-1);
}

/* Moves one side player model set onto the requested camera bit. */
void fn_1_1610(s16 side, u16 camera)
{
    OMOBJ *obj = lbl_1_bss_2C[side];
    M659Player *playerData = obj->data;
    M659PlayerInfo *info = &playerData->info;
    s16 i;
    info->cameraBit = camera;
    for (i = 0; i < 5; i++) {
        HU3D_MODELID model = obj->mdlId[i];
        Hu3DModelCameraSet(model, info->cameraBit);
    }
}
