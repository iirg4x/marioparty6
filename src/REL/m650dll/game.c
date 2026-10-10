/* Implements player setup, movement, throwing, scoring, and game updates. */
#include "REL/m650/m650.h"

/* Resource numbers are identified by the model or motion slot that uses them. */
#define M650_DATA_PLAYER_AUX_MODEL 7143428
#define M650_DATA_PLAYER_JOINT_MODEL 7143441
#define M650_DATA_PLAYER_EXTRA_MODEL 7143440
#define M650_DATA_PLAYER_EFFECT_MODEL 7143435
#define M650_DATA_PLAYER_JOINT_RESOURCE_0 7143442
#define M650_DATA_PLAYER_JOINT_RESOURCE_1 7143443
#define M650_DATA_PLAYER_JOINT_RESOURCE_2 7143444
#define M650_DATA_PLAYER_JOINT_RESOURCE_3 7143445
#define M650_DATA_DAY_COURSE_SHADOW_MODEL 7143424
#define M650_DATA_NIGHT_COURSE_SHADOW_MODEL 7143430
#define M650_DATA_DAY_COURSE_BACKGROUND_MODEL 7143425
#define M650_DATA_NIGHT_COURSE_BACKGROUND_MODEL 7143431
#define M650_DATA_DAY_PLAYER_SHADOW_MODEL 7143429
#define M650_DATA_NIGHT_PLAYER_SHADOW_MODEL 7143434
#define M650_DATA_DAY_PLAYER_EFFECT_A 7143436
#define M650_DATA_NIGHT_PLAYER_EFFECT_A 7143437
#define M650_DATA_DAY_PLAYER_EFFECT_B 7143438
#define M650_DATA_NIGHT_PLAYER_EFFECT_B 7143439
#define M650_DATA_PLAYER_JOINT_MOTION_0 9306277
#define M650_DATA_PLAYER_JOINT_MOTION_1 9633814
#define M650_DATA_PLAYER_JOINT_MOTION_2 9306278
#define M650_DATA_PLAYER_JOINT_MOTION_3 9633792
#define M650_DATA_PLAYER_JOINT_MOTION_4 9633798

void fn_1_A0(void);
void fn_1_F0(s16 mode, s16 frame);
void fn_1_168(s16 mode, s16 frame);
void fn_1_188(s16 mode, s16 frame);
void fn_1_1C8(s16 mode, s16 frame);
void fn_1_200(s16 mode, s16 frame);
void fn_1_304(s16 mode, s16 frame);
void fn_1_3AC(s16 mode, s16 frame);
void fn_1_3B0(s16 mode, s16 frame);
void fn_1_3B4(s16 mode, s16 frame);
void fn_1_3B8(void);
void fn_1_484(void);
void fn_1_550(void);
void fn_1_8EC(void);
void fn_1_F90(OMOBJ *obj);
void fn_1_1228(void);
void fn_1_14FC(void);
void fn_1_1830(void);
void fn_1_1AE4(void);
void fn_1_2010(s16 player);
void fn_1_21CC(OMOBJ *obj);
void fn_1_22A8(s16 player, s16 motion);
void fn_1_2320(s16 player, s16 motion);
void fn_1_23C4(s16 player, s16 motion, float blend);
void fn_1_2464(s16 player);
void fn_1_2558(void);
void fn_1_25B0(void);
void fn_1_26D0(void);
void fn_1_273C(s16 player);
void fn_1_289C(s16 player);
s16 fn_1_2CAC(s16 player);
void fn_1_2F1C(s16 player);
/* Called each frame in states 2 and 3 to move the thrown bone, resolve impacts, and apply
 * gravity. */
void fn_1_33F8(s16 player);
/* During state 4, guides the player toward the thrown bone and handles the finish or contact. */
void fn_1_3F28(s16 player);
/* During state 5, rebounds the runner along the reflected direction, then starts the spin penalty
 * or stops for results. */
void fn_1_4430(s16 player);
void fn_1_473C(s16 player);
void fn_1_48C0(s16 player);
void fn_1_49C8(s16 player);
void fn_1_4BD4(s16 player);
s16 fn_1_4D58(s16 frame);
/* Before non-Decathlon play, loads the record and creates the shared elapsed-time display. */
void fn_1_5258(void);
void fn_1_5308(void);
/* Called by finish detection outside Decathlon to stop the shared elapsed-time display. */
void fn_1_5354(void);
/* Before Decathlon play, creates and positions an elapsed-time display for each player. */
void fn_1_5394(void);
void fn_1_54D4(void);
void fn_1_5544(s16 player);
void fn_1_5600(s16 unusedPlayer, f32 value, f32 *out0, f32 *out1);
s16 fn_1_5728(void);
void fn_1_5A24(s16 player);
/* While pad 0 camera adjustment is enabled, adjusts all camera poses and prints camera 0. */
void fn_1_5E40(void);
void fn_1_62E0(void);
void fn_1_6478(HU3D_MODEL *modelP, Mtx *mtx);
s16 fn_1_69B0(s16 player);
s16 fn_1_6D88(s16 player, HuVecF *position);
s16 fn_1_6EE4(HuVecF *firstPosition, HuVecF *secondPosition, float radius);
void fn_1_6F54(void);
s16 fn_1_7000(s16 unusedPlayer, HuVecF *position, float angle, s16 obstacleIndex);

extern GXColor lbl_1_data_40[2];
extern Point3d lbl_1_data_28;
extern Point3d lbl_1_data_34;
extern s16 lbl_1_bss_4AC[4];
extern s16 lbl_1_bss_4B4;
extern s16 lbl_1_bss_4B6;
extern s32 lbl_1_bss_2A8;
extern s32 lbl_1_data_48[4];
extern s32 lbl_1_data_80[2];
extern s32 lbl_1_data_88[2];
extern s32 lbl_1_data_90[2];
extern s32 lbl_1_data_98[2];
extern s32 lbl_1_data_A0[2];

Point3d lbl_1_data_28 = { -2000.0f, 10000.0f, -400.0f };
Point3d lbl_1_data_34 = { -0.4f, -0.8f, -0.8f };
GXColor lbl_1_data_40[2] = { {128,128,128,128}, {144,144,144,144} };
s32 lbl_1_data_48[4] = {
    M650_DATA_PLAYER_JOINT_RESOURCE_0, M650_DATA_PLAYER_JOINT_RESOURCE_1,
    M650_DATA_PLAYER_JOINT_RESOURCE_2, M650_DATA_PLAYER_JOINT_RESOURCE_3
};
M650Motion lbl_1_data_58[5] = {
    { M650_DATA_PLAYER_JOINT_MOTION_0, HU3D_MOTATTR_LOOP },
    { M650_DATA_PLAYER_JOINT_MOTION_1, HU3D_MOTATTR_LOOP },
    { M650_DATA_PLAYER_JOINT_MOTION_2, 0U },
    { M650_DATA_PLAYER_JOINT_MOTION_3, HU3D_MOTATTR_LOOP },
    { M650_DATA_PLAYER_JOINT_MOTION_4, 0U },
};
s32 lbl_1_data_80[2] = { M650_DATA_DAY_COURSE_SHADOW_MODEL, M650_DATA_NIGHT_COURSE_SHADOW_MODEL };
s32 lbl_1_data_88[2] = {
    M650_DATA_DAY_COURSE_BACKGROUND_MODEL, M650_DATA_NIGHT_COURSE_BACKGROUND_MODEL
};
s32 lbl_1_data_90[2] = { M650_DATA_DAY_PLAYER_SHADOW_MODEL, M650_DATA_NIGHT_PLAYER_SHADOW_MODEL };
s32 lbl_1_data_98[2] = { M650_DATA_DAY_PLAYER_EFFECT_A, M650_DATA_NIGHT_PLAYER_EFFECT_A };
s32 lbl_1_data_A0[2] = { M650_DATA_DAY_PLAYER_EFFECT_B, M650_DATA_NIGHT_PLAYER_EFFECT_B };
u8 lbl_1_data_A8[168] = {
    63, 12, 204, 205, 63, 128, 0, 0, 63, 25, 153, 154, 63, 12, 204, 205,
    63, 128, 0, 0, 63, 0, 0, 0, 63, 64, 0, 0, 63, 128, 0, 0,
    63, 64, 0, 0, 63, 25, 153, 154, 63, 128, 0, 0, 63, 25, 153, 154,
    63, 64, 0, 0, 63, 128, 0, 0, 63, 51, 51, 51, 63, 64, 0, 0,
    63, 128, 0, 0, 63, 64, 0, 0, 63, 76, 204, 205, 63, 128, 0, 0,
    63, 51, 51, 51, 63, 0, 0, 0, 63, 128, 0, 0, 63, 0, 0, 0,
    63, 25, 153, 154, 63, 128, 0, 0, 63, 25, 153, 154, 63, 76, 204, 205,
    63, 128, 0, 0, 63, 51, 51, 51, 63, 0, 0, 0, 63, 128, 0, 0,
    63, 0, 0, 0, 63, 76, 204, 205, 63, 128, 0, 0, 63, 51, 51, 51,
    63, 76, 204, 205, 63, 128, 0, 0, 63, 51, 51, 51, 63, 76, 204, 205,
    63, 128, 0, 0, 63, 51, 51, 51,
};
u16 lbl_1_data_150[4] = { 1, 2, 4, 8 };

M650Player lbl_1_bss_28[4];

/* Called during scene setup to create four viewports and their starting camera poses. */
void fn_1_14FC(void)
{
    u16 cameras[4] = { 1, 2, 4, 8 };
    s32 cameraIndex;
    OMOBJ *viewObj;
    f32 viewportHeight;
    f32 viewportWidth;
    f32 scissorHeight;
    f32 scissorWidth;

    cameraIndex = 0;
    while (cameraIndex < 4) {
        Hu3DCameraCreate(cameras[cameraIndex]);
        if (cameraIndex != 0) {
            viewportHeight = 0.0f;
        } else {
            viewportHeight = 480.0f;
        }
        if (cameraIndex != 0) {
            viewportWidth = 0.0f;
        } else {
            viewportWidth = 640.0f;
        }
        Hu3DCameraViewportSet(cameras[cameraIndex], 0.0f, 0.0f, viewportWidth, viewportHeight, 0.0f,
                              1.0f);
        Hu3DCameraPerspectiveSet(cameras[cameraIndex], 40.0f, 60.0f, 25000.0f, 1.2f);
        if (cameraIndex != 0) {
            scissorHeight = 0.0f;
        } else {
            scissorHeight = 480.0f;
        }
        if (cameraIndex != 0) {
            scissorWidth = 0.0f;
        } else {
            scissorWidth = 640.0f;
        }
        Hu3DCameraScissorSet(cameras[cameraIndex], 0U, 0U, (u32) scissorWidth, (u32) scissorHeight);
        cameraIndex += 1;
    }
    viewObj = omAddObjEx(lbl_1_bss_1C, 32730, 0U, 0U, -1, omOutViewMulti);
    viewObj->work[0] = 4;
    cameraIndex = 0;
    while (cameraIndex < 4) {
        CenterM[cameraIndex].x = 0.0f;
        CenterM[cameraIndex].y = 200.0f;
        CenterM[cameraIndex].z = 0.0f;
        CRotM[cameraIndex].x = -10.0f;
        CRotM[cameraIndex].y = 0.0f;
        CRotM[cameraIndex].z = 0.0f;
        CZoomM[cameraIndex] = 1000.0f;
        cameraIndex += 1;
    }
}

/* Called during scene setup to create lighting, shadows, and background models. */
void fn_1_1830(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    s16 lightID;
    s16 modelID;

    lightID = Hu3DGLightCreateV(&lbl_1_data_28, &lbl_1_data_34, &lbl_1_data_40[lbl_1_bss_0.night]);
    Hu3DGLightStaticSet(lightID, 0);
    Hu3DGLightInfinitytSet(lightID);
    if (lbl_1_bss_0.night == 0) {
        Hu3DShineSet(1);
        Hu3DAmbColorSet(0.5f, 0.5f, 0.5f);
    }
    Hu3DShadowMultiCreate(30.0f, 500.0f, 80000.0f, 15);
    shadowPos.x = -750.0f;
    shadowPos.y = 4500.0f;
    shadowPos.z = -1000.0f;
    shadowUp.x = 0.0f;
    shadowUp.y = 1.0f;
    shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = 0.0f;
    shadowTarget.z = 0.0f;
    Hu3DShadowMultiPosSet(&shadowPos, &shadowUp, &shadowTarget, 15);
    if (lbl_1_bss_0.night == 0) {
        Hu3DShadowMultiColSet(130U, 30U, 0U, 15);
        Hu3DShadowMultiTPLvlSet(0.8f, 15);
    } else {
        Hu3DShadowMultiColSet(0U, 0U, 0U, 15);
        Hu3DShadowMultiTPLvlSet(0.5f, 15);
    }
    modelID = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_80[lbl_1_bss_0.night], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(modelID, 65535U);
    Hu3DModelShadowMapSet(modelID);
    if (lbl_1_bss_0.night == 0) {
        Hu3DModelShadowMapTPLvlSet(modelID, 1.0f);
    } else {
        Hu3DModelShadowMapTPLvlSet(modelID, 0.5f);
    }
    modelID = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_88[lbl_1_bss_0.night], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(modelID, 65535U);
}

/* Called after cameras and lighting exist; builds each player's models and motions. */
void fn_1_1AE4(void)
{
    OMOBJ *playerUpdateObj;
    s16 charNo;
    s16 modelID;
    M650Player *playerWork;
    s32 playerNo;
    s32 motionIndex;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        playerWork = &lbl_1_bss_28[playerNo];
        playerWork->charNo = charNo = GwPlayerConf[playerNo].charNo;
        playerWork->padNo = GwPlayerConf[playerNo].padNo;
        if (GwPlayerConf[playerNo].type == 0) {
            playerWork->computerDifficulty = -1;
        } else {
            playerWork->computerDifficulty = GwPlayerConf[playerNo].comDif;
        }
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(M650_DATA_PLAYER_AUX_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        Hu3DModelAttrSet(modelID, 1U);
        playerWork->model1E = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(M650_DATA_PLAYER_JOINT_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        motionIndex = 0;
        while (motionIndex < 4) {
            playerWork->jointMotionIDs[motionIndex] =
                Hu3DJointMotion(modelID, HuDataSelHeapReadNum(lbl_1_data_48[motionIndex],
                                                              HU_MEMNUM_OVL, HEAP_MODEL));
            motionIndex += 1;
        }
        Hu3DMotionSet(modelID, playerWork->jointMotionIDs[0]);
        Hu3DModelAttrSet(modelID, HU3D_MOTATTR_LOOP);
        Hu3DModelAmbSet(modelID, 1.0f, 1.0f, 1.0f);
        Hu3DModelShadowSet(modelID);
        playerWork->model20 = modelID;
        modelID = CharModelCreate(playerWork->charNo, 8);
        Hu3DModelShadowSet(modelID);
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        playerWork->model = modelID;
        motionIndex = 0;
        while (motionIndex < 5) {
            playerWork->motionIDs[motionIndex] =
                CharMotionCreate(playerWork->charNo, (u32) lbl_1_data_58[motionIndex].file);
            motionIndex += 1;
        }
        CharMotionShiftSet(playerWork->charNo, playerWork->motionIDs[0], 0.0f, 0.0f,
                           lbl_1_data_58->flags);
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(M650_DATA_PLAYER_EXTRA_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        Hu3DModelAttrSet(modelID, 1U);
        playerWork->model52 = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_90[lbl_1_bss_0.night], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        Hu3DModelScaleSet(modelID, 0.3f, 1.0f, 0.3f);
        Hu3DModelPosSet(modelID, 0.0f, 0.2f, 0.0f);
        Hu3DModelAttrSet(modelID, 1U);
        Hu3DModelTPLvlSet(modelID, 0.5f);
        playerWork->model54 = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_98[lbl_1_bss_0.night], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSpeedSet(modelID, 0.0f);
        Hu3DModelAttrSet(modelID, 1U);
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        playerWork->model7A = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_A0[lbl_1_bss_0.night], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSpeedSet(modelID, 0.0f);
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        Hu3DModelAttrSet(modelID, 1U);
        playerWork->model7C = modelID;
        modelID = Hu3DModelCreate(
            HuDataSelHeapReadNum(M650_DATA_PLAYER_EFFECT_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DMotionSpeedSet(modelID, 0.0f);
        Hu3DModelCameraSet(modelID, lbl_1_data_150[playerNo]);
        Hu3DModelAttrSet(modelID, 1U);
        playerWork->model78 = modelID;
        playerWork->timerValue = -1;
    }
    CharEffectLayerSet(3);
    /* The callback is used through the object manager; its returned handle is ignored. */
    playerUpdateObj = omAddObjEx(lbl_1_bss_1C, 80, 0U, 0U, -1, fn_1_21CC);
}

/* Updates one player's action, positions its attached model, and selects the live camera view. */
void fn_1_2010(s16 player)
{
    Mtx characterMatrix;
    Point3d playerPosition;
    s16 sequenceMode;
    M650Player *playerWork;

    playerWork = &lbl_1_bss_28[player];
    /* The current sequence mode is read here but does not select a player action. */
    sequenceMode = MgSeqModeGet();
    switch (playerWork->state) {
        case 1:
            fn_1_273C(player);
            break;
        case 2:
            fn_1_2F1C(player);
            fn_1_33F8(player);
            break;
        case 3:
            fn_1_33F8(player);
            break;
        case 4:
            fn_1_3F28(player);
            break;
        case 5:
            fn_1_4430(player);
            break;
        case 6:
            fn_1_473C(player);
            break;
        case 7:
            fn_1_48C0(player);
            break;
        case 8:
            fn_1_49C8(player);
            break;
        case 10:
            fn_1_4BD4(player);
            break;
        }
    Hu3DModelPosGet(playerWork->model20, &playerPosition);
    Hu3DModelPosSetV(playerWork->model1E, &playerPosition);
    /* This offset is deliberately not fed back to either model position. */
    playerPosition.y += 0.2;
    if (lbl_1_bss_0.resultsScene == 0) {
        Hu3DModelObjMtxGet(playerWork->model20, "itemhook_C", characterMatrix);
        Hu3DModelMtxSet(playerWork->model, &characterMatrix);
    } else {
        mtxRot(characterMatrix, 0.0f, 180.0f, 0.0f);
        mtxTransCat(characterMatrix, 0.0f, 0.0f, 900.0f);
        Hu3DModelMtxSet(playerWork->model, &characterMatrix);
    }
    fn_1_5A24(player);
}

/* Object-manager callback that advances all four players and checks for round transitions each
 * frame. */
void fn_1_21CC(OMOBJ *obj)
{
    s32 player;
    u32 sequenceMode;

    sequenceMode = MgSeqModeGet();
    player = 0;
    while (player < 4) {
        fn_1_2010((s16) player);
        player += 1;
    }
    if (fn_1_5728() != 0) {
        MgSeqModeNext();
    }
    if (_CheckFlag(FLAG_INST_DECA) != 0) {
        for (player = 0; player < 4; player++) {
            if (lbl_1_bss_28[player].timer && MgTimerDoneCheck(lbl_1_bss_28[player].timer) == 1) {
                break;
            }
        }
        if ((player < 4) && (sequenceMode == 5)) {
            MgSeqModeNext();
        }
    }
}

/* Blends the shared player model to one of its joint motions during gameplay. */
void fn_1_22A8(s16 player, s16 motion)
{
    M650Player *work;

    work = &lbl_1_bss_28[player];
    Hu3DMotionShiftSet(work->model20, work->jointMotionIDs[motion], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
}

/* Starts a character animation using the standard eight-frame blend. */
void fn_1_2320(s16 player, s16 motion)
{
    CharMotionShiftSet(lbl_1_bss_28[player].charNo, lbl_1_bss_28[player].motionIDs[motion], 0.0f,
                       8.0f, lbl_1_data_58[motion].flags);
}

/* Starts a character animation with the caller-selected blend duration. */
void fn_1_23C4(s16 player, s16 motion, float blend)
{
    CharMotionShiftSet(lbl_1_bss_28[player].charNo,
        lbl_1_bss_28[player].motionIDs[motion], 0.0f, blend, lbl_1_data_58[motion].flags);
}

/* After the runner's rebound ends, starts a 120-frame spin penalty before returning to aiming. */
void fn_1_2464(s16 player)
{
    M650Player *work;
    work = &lbl_1_bss_28[player];
    work->throwWait = 120;
    work->state = 6;
    fn_1_22A8(player, 2);
    fn_1_23C4(player, 1, 8.0f);
}

/* The gameplay-entry callback calls this at frame zero to ready all players and refresh CPU
 * obstacle choices. */
void fn_1_2558(void)
{
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_28[playerIndex].state = 1;
        fn_1_289C((s16) playerIndex);
        playerIndex += 1;
    }
}

/* At finish frame zero, marks the round ending and stops aiming players and nonwinning chasers;
 * other action states continue until they settle. */
void fn_1_25B0(void)
{
    int playerIndex;

    lbl_1_bss_0.roundEnding = 1;
    playerIndex = 0;
    while (playerIndex < 4) {
        switch (lbl_1_bss_28[playerIndex].state) {
        case 1:
            lbl_1_bss_28[playerIndex].state = 9;
            Hu3DModelAttrSet(lbl_1_bss_28[playerIndex].model1E, 1U);
            break;
        case 4:
            if (lbl_1_bss_0.winner != playerIndex) {
                lbl_1_bss_28[playerIndex].state = 9;
                fn_1_22A8(playerIndex, 0);
            }
            break;
        }
        playerIndex += 1;
    }
}

/* During results-scene setup, stops all players and hides their auxiliary models. */
void fn_1_26D0(void)
{
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_28[playerIndex].state = 9;
        Hu3DModelAttrSet(lbl_1_bss_28[playerIndex].model1E, 1U);
        playerIndex += 1;
    }
}

/* Oscillates a player between the two aiming limits until a throw is chosen. */
void fn_1_273C(s16 player)
{
    M650Player *playerWork;

    playerWork = &lbl_1_bss_28[player];
    Hu3DModelAttrReset(playerWork->model1E, 1U);
    Hu3DModelRotSet(playerWork->model1E, 0.0f, playerWork->turnAngle, 0.0f);
    if (playerWork->turnTowardPositiveAngle != 0) {
        playerWork->turnAngle += 1.7142857f;
        if (playerWork->turnAngle >= 30.0f) {
            playerWork->turnAngle = 30.0f;
            playerWork->turnTowardPositiveAngle = 0;
        }
    } else {
        playerWork->turnAngle -= 1.7142857f;
        if (playerWork->turnAngle <= -30.0f) {
            playerWork->turnAngle = -30.0f;
            playerWork->turnTowardPositiveAngle = 1;
        }
    }
    if (fn_1_2CAC(player) != 0) {
        playerWork->state = 2;
        playerWork->movementSpeed = 0.0f;
        playerWork->actionFrame = 0;
        playerWork->horizontalDistance = 0.0f;
    }
}

/* Chooses a computer player's throw delay and caches nearby obstacles for aiming. */
void fn_1_289C(s16 player)
{
    HuVecF playerPosition;
    HuVecF obstacleOffset;
    s16 delay[4] = { 210, 140, 50, 0 };
    s16 delayRange[4] = { 105, 70, 35, 70 };
    s16 chance[4] = { 90, 45, 15, 5 };
    M650Player *work;
    int i;

    work = &lbl_1_bss_28[player];
    if (work->computerDifficulty != -1) {
        work->throwDelay =
            delay[work->computerDifficulty] + frandmod(delayRange[work->computerDifficulty]);
        if (frandmod(100) < chance[work->computerDifficulty]) {
            work->riskyThrowMode = 1;
        } else {
            work->riskyThrowMode = 0;
        }
        Hu3DModelPosGet(work->model20, &playerPosition);
        work->candidateCount = 0;
        work->decisionFrameCount = 0;
        for (i = 0; i < 32; i++) {
            if (lbl_1_bss_2AC[i][player].state != 0) continue;
            if (lbl_1_data_248[i].pos.z < playerPosition.z - 50.0f) continue;
            if (lbl_1_data_248[i].pos.z > 900.0f + playerPosition.z) continue;
            if (lbl_1_data_248[i].pos.x > 500.0f + playerPosition.x) continue;
            if (lbl_1_data_248[i].pos.x < playerPosition.x - 500.0f) continue;
            obstacleOffset.x = lbl_1_data_248[i].pos.x - playerPosition.x;
            obstacleOffset.y = 0.0f;
            obstacleOffset.z = lbl_1_data_248[i].pos.z - playerPosition.z;
            if (sqrtf(PSVECSquareMag(&obstacleOffset)) > 1000.0f) continue;
            work->candidateIDs[work->candidateCount] = i;
            if (++work->candidateCount == 12) {
                OSReport("WARNING! obst cnt\n");
                print8(100, player * 16 + 100, 1.8f, "OBST CNT OVER %d", player);
                return;
            }
        }
    }
}

/* Decides whether a human or computer player throws on this frame. */
s16 fn_1_2CAC(s16 player)
{
    Point3d playerPosition;
    f32 projectedForwardDistance;
    M650Player *playerWork;
    s32 candidateIndex;
    s16 blockedByBoundary;
    s16 pathBlocked;
    f32 projectedSideDistance;

    playerWork = &lbl_1_bss_28[player];
    Hu3DModelPosGet(playerWork->model20, &playerPosition);
    if (playerWork->computerDifficulty == -1) {
        return (s16) (HuPadBtnDown[playerWork->padNo] & 0x0100);
    }
    if (playerWork->throwDelay != 0) {
        playerWork->throwDelay -= 1;
        return 0;
    }
    projectedSideDistance =
        (f32) (800.0 * sin((3.141592653589793 * (f64) playerWork->turnAngle) / 180.0));
    /* The forward projection is retained from the original logic but does not affect this
     * decision. */
    projectedForwardDistance =
        (f32) (800.0 * cos((3.141592653589793 * (f64) playerWork->turnAngle) / 180.0));
    if (((playerPosition.x + projectedSideDistance) >= 1150.0f) ||
        ((playerPosition.x + projectedSideDistance) <= -1150.0f)) {
        blockedByBoundary = 1;
    } else {
        blockedByBoundary = 0;
    }
    pathBlocked = 0;
    candidateIndex = 0;
    while (candidateIndex < playerWork->candidateCount) {
        if (fn_1_7000(player, &playerPosition, playerWork->turnAngle,
                      playerWork->candidateIDs[candidateIndex]) != 0) {
            pathBlocked = 1;
        } else {
            candidateIndex += 1;
            continue;
        }
        break;
    }
    if (playerWork->decisionFrameCount++ >= 70) {
        if (playerWork->riskyThrowMode != 0) {
            return 1;
        }
        if (blockedByBoundary != 0) {
            return 0;
        }
        return 1;
    }
    if (playerWork->riskyThrowMode != 0) {
        if ((pathBlocked != 0) || (blockedByBoundary != 0) || (playerWork->candidateCount == 0)) {
            return 1;
        }
        return 0;
    }
    if ((pathBlocked == 0) && (blockedByBoundary == 0)) {
        return 1;
    }
    return 0;
}

/* The player update calls this during a throw to release the bone at frame 44. */
void fn_1_2F1C(s16 playerIndex)
{
    M650Player *playerWork = &lbl_1_bss_28[playerIndex];
    Point3d bodyRotation;
    Point3d boneHookPosition;
    HuVecF offset;
    s32 sounds[4] = { M650_EFFECT_2028, M650_EFFECT_2029, M650_EFFECT_2030, M650_EFFECT_2031 };
    f32 throwElevation;
    f32 launchSpeed;
    f32 motionTime;
    f32 motionEndTime;

    Hu3DModelRotGet(playerWork->model20, &bodyRotation);
    if (bodyRotation.y > playerWork->turnAngle) {
        bodyRotation.y -= 2.0f;
        if (bodyRotation.y < playerWork->turnAngle) {
            bodyRotation.y = playerWork->turnAngle;
        }
    } else if (bodyRotation.y < playerWork->turnAngle) {
        bodyRotation.y += 2.0f;
        if (bodyRotation.y > playerWork->turnAngle) {
            bodyRotation.y = playerWork->turnAngle;
        }
    }
    Hu3DModelRotSetV(playerWork->model20, &bodyRotation);
    if (playerWork->actionFrame == 30) {
        fn_1_23C4(playerIndex, 2, 8.0f);
        Hu3DModelPosSet(playerWork->model52, 0.0f, 0.0f, 0.0f);
        Hu3DModelHookSet(playerWork->model, "f-itemhook-r", playerWork->model52);
        Hu3DModelRotSet(playerWork->model52, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(playerWork->model52, 1U);
    }
    if (playerWork->actionFrame > 30) {
        motionTime = Hu3DMotionTimeGet(playerWork->model);
        motionEndTime = Hu3DMotionMaxTimeGet(playerWork->model);
        if ((motionTime >= motionEndTime) && (playerWork->actionFrame > 44)) {
            fn_1_23C4(playerIndex, 0, 8.0f);
            playerWork->state = 3;
            playerWork->actionFrame = 0;
            return;
        }
        if (playerWork->actionFrame == 44) {
            HuAudFXPlay(sounds[playerIndex]);
            Hu3DModelObjPosGet(playerWork->model, "f-itemhook-r", &boneHookPosition);
            offset.x = offset.y = offset.z = 0.0f;
            Hu3DModelHookReset(playerWork->model);
            Hu3DModelPosSet(playerWork->model52, boneHookPosition.x + offset.x,
                            boneHookPosition.y + offset.y, boneHookPosition.z + offset.z);
            Hu3DModelAttrReset(playerWork->model54, 1U);
            Hu3DModelPosSet(playerWork->model54, boneHookPosition.x + offset.x, 0.2f,
                            boneHookPosition.z + offset.z);
            Hu3DModelRotSet(playerWork->model52, 0.0f, 45.0f, 0.0f);
            fn_1_5600(playerIndex, boneHookPosition.y + offset.y, &launchSpeed, &throwElevation);
            playerWork->boneVelocity.x =
                (launchSpeed * cos((3.141592653589793 * throwElevation) / 180.0)) *
                sin((3.141592653589793 * playerWork->turnAngle) / 180.0);
            playerWork->boneVelocity.y =
                (f32) ((f64) launchSpeed * sin((3.141592653589793 * (f64) throwElevation) / 180.0));
            playerWork->boneVelocity.z =
                (launchSpeed * cos((3.141592653589793 * throwElevation) / 180.0)) *
                cos((3.141592653589793 * playerWork->turnAngle) / 180.0);
            Hu3DModelAttrSet(playerWork->model1E, 1U);
        }
    }
    playerWork->actionFrame += 1;
}

/* During states 2 and 3 of the per-frame player update, advances the thrown bone and resolves
 * its ground, wall, and obstacle bounces. */
void fn_1_33F8(s16 player)
{
    M650Player *work = &lbl_1_bss_28[player];
    HuVecF pos;
    HuVecF horizontal;
    HuVecF reflection;
    HuVecF normal;
    s32 hitSounds[4] = { M650_EFFECT_2032, M650_EFFECT_2033, M650_EFFECT_2034, M650_EFFECT_2035 };
    s32 groundSounds[4] = {
        M650_EFFECT_2036, M650_EFFECT_2037, M650_EFFECT_2038, M650_EFFECT_2039
    };
    float speed;
    float distanceSquared;
    float radiusSquared;
    float penetration;
    float velocityLength;
    s16 hit;

    if (work->boneVelocity.x != 0.0f || work->boneVelocity.y != 0.0f ||
        work->boneVelocity.z != 0.0f) {
        work->bounceFrameCount++;
        /* Wall and obstacle rebounds reuse this incoming speed even if an earlier ground bounce
        * changes velocity in the same frame. */
        speed = HuMagVecF(&work->boneVelocity);
        Hu3DModelPosGet(work->model52, &pos);
        pos.x += work->boneVelocity.x;
        pos.y += work->boneVelocity.y;
        pos.z += work->boneVelocity.z;
        horizontal.x = work->boneVelocity.x;
        horizontal.y = 0.0f;
        horizontal.z = work->boneVelocity.z;
        work->horizontalDistance += sqrtf(PSVECSquareMag(&horizontal));
        if (pos.y <= 0.5f) {
            HuAudFXPlay(groundSounds[player]);
            pos.y = 0.5f;
            work->boneVelocity.x *= 0.4;
            work->boneVelocity.y *= -0.4;
            work->boneVelocity.z *= 0.4;
            if (work->boneVelocity.y < 1.0f) {
                work->bounceFrameCount = 0;
                if (lbl_1_bss_0.roundEnding == 0) {
                    work->state = 4;
                    fn_1_22A8(player, 1);
                    /* The player number is evaluated here but does not affect the landing sound. */
                    (void)(int)player;
            HuAudFXPlayPan(M650_EFFECT_1001, 32);
                } else {
                    work->state = 9;
                }
                work->boneVelocity.x = work->boneVelocity.y = work->boneVelocity.z = 0.0f;
            }
        }
        if (pos.x <= -1210.0f) {
            HuAudFXPlay(hitSounds[player]);
            pos.x = -1210.0f;
            normal.x = 1.0f;
            normal.y = normal.z = 0.0f;
            C_VECReflect(&work->boneVelocity, &normal, &reflection);
            work->boneVelocity.x = 0.5 * reflection.x * speed;
            work->boneVelocity.y = 0.5 * reflection.y * speed;
            work->boneVelocity.z = 0.5 * reflection.z * speed;
        }
        if (pos.x >= 1210.0f) {
            HuAudFXPlay(hitSounds[player]);
            pos.x = 1210.0f;
            normal.x = -1.0f;
            normal.y = normal.z = 0.0f;
            C_VECReflect(&work->boneVelocity, &normal, &reflection);
            work->boneVelocity.x = 0.5 * reflection.x * speed;
            work->boneVelocity.y = 0.5 * reflection.y * speed;
            work->boneVelocity.z = 0.5 * reflection.z * speed;
        }
        hit = fn_1_6D88(player, &pos);
        if (hit >= 0) {
            HuAudFXPlay(hitSounds[player]);
            horizontal.x = pos.x;
            horizontal.y = 0.0f;
            horizontal.z = pos.z;
            distanceSquared = PSVECSquareDistance(&lbl_1_data_248[hit].pos, &horizontal);
            radiusSquared = 8100.0f;
            if (distanceSquared < radiusSquared) {
                penetration = sqrtf(radiusSquared - distanceSquared);
                normal.x = normal.y = normal.z = 0.0f;
                velocityLength = sqrtf(PSVECSquareDistance(&work->boneVelocity, &normal));
                if (velocityLength > 0.0f) {
                    pos.x -= (work->boneVelocity.x * penetration) / velocityLength;
                    pos.y -= (work->boneVelocity.y * penetration) / velocityLength;
                    pos.z -= (work->boneVelocity.z * penetration) / velocityLength;
                }
            }
            normal.x = pos.x - lbl_1_data_248[hit].pos.x;
            normal.y = 0.0f;
            normal.z = pos.z - lbl_1_data_248[hit].pos.z;
            PSVECNormalize(&normal, &normal);
            if (work->boneVelocity.x || work->boneVelocity.y || work->boneVelocity.z) {
                C_VECReflect(&work->boneVelocity, &normal, &reflection);
                work->boneVelocity.x = 0.4 * reflection.x * speed;
                work->boneVelocity.y = 0.4 * reflection.y * speed;
                work->boneVelocity.z = 0.4 * reflection.z * speed;
            }
            pos.x = 90.0f * normal.x + lbl_1_data_248[hit].pos.x;
            pos.z = 90.0f * normal.z + lbl_1_data_248[hit].pos.z;
        }
        Hu3DModelPosSetV(work->model52, &pos);
        Hu3DModelPosSet(work->model54, pos.x, 0.2f, pos.z);
        /* Gravity still applies on the frame that clears the velocity and leaves the throw
         * states. */
        work->boneVelocity.y -= 1.6333333f;
    }
}

/* During state 4 of the per-frame player update, guides the player to the bone and handles the
 * finish or a collision with an obstacle or lane wall. */
void fn_1_3F28(s16 player)
{
    M650Player *work = &lbl_1_bss_28[player];
    HuVecF pos;
    HuVecF itemPos;
    HuVecF rot;
    HuVecF delta;
    s32 sounds[4] = { M650_EFFECT_2040, M650_EFFECT_2041, M650_EFFECT_2042, M650_EFFECT_2043 };
    float angle;
    float distance;

    if (work->movementSpeed < 12.0f) work->movementSpeed += 1.0f;
    Hu3DModelPosGet(work->model52, &itemPos);
    Hu3DModelPosGet(work->model20, &pos);
    itemPos.y = 0.0f;
    delta.x = itemPos.x - pos.x;
    delta.z = itemPos.z - pos.z;
    distance = HuMagPoint2D(delta.x, delta.z);
    if (distance > work->movementSpeed) {
        work->direction.x = work->movementSpeed * (delta.x / distance);
        work->direction.z = work->movementSpeed * (delta.z / distance);
    } else {
        work->direction.x = delta.x;
        work->direction.z = delta.z;
    }
    Hu3DModelRotGet(work->model20, &rot);
    if (work->direction.x == 0.0f) {
        /* Straight movement forces yaw to zero even when the bone is behind the runner. */
        angle = 0.0f;
    } else {
        angle = 180.0 * (atan2(work->direction.x, work->direction.z) / M_PI);
    }
    if (rot.y < angle) {
        rot.y += 2.0f;
        if (rot.y > angle) rot.y = angle;
    } else if (rot.y > angle) {
        rot.y -= 2.0f;
        if (rot.y < angle) rot.y = angle;
    }
    Hu3DModelRotSetV(work->model20, &rot);
    if (work->reachedGoal != 0 && pos.z >= 5500.0f) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        work->state = 10;
        fn_1_22A8(player, 0);
    } else if (distance > work->movementSpeed) {
        pos.x += work->direction.x;
        pos.z += work->direction.z;
        Hu3DModelPosSetV(work->model20, &pos);
    } else {
        Hu3DModelPosSetV(work->model20, &itemPos);
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        if (lbl_1_bss_0.roundEnding == 0) {
            if (work->reachedGoal == 0) {
                work->state = 7;
            } else {
                work->state = 10;
                fn_1_22A8(player, 0);
            }
        } else {
            fn_1_22A8(player, 0);
            work->state = 9;
        }
    }
    /* A collision can override the pickup, finish, or stopped state selected above with rebound. */
    if (fn_1_69B0(player)) {
        work->state = 5;
        omVibrate(player, 20, 7, 3);
    }
}

/* During state 5, rebounds the runner along the reflected direction, then starts the spin penalty
 * or stops for results. */
void fn_1_4430(s16 player)
{
    HuVecF pos;
    M650Player *work;
    s16 reboundFrameThreshold;

    work = &lbl_1_bss_28[player];
    /* Rebound movement continues until actionFrame's old value exceeds the integer part of
     * speed. */
    reboundFrameThreshold = work->movementSpeed;
    if (work->actionFrame % 4 == 0) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
    } else if (work->actionFrame % 4 == 2) {
        Hu3DModelAttrReset(work->model52, 1);
        Hu3DModelAttrReset(work->model54, 1);
    }
    if (work->actionFrame++ > reboundFrameThreshold) {
        Hu3DModelAttrSet(work->model52, 1);
        Hu3DModelAttrSet(work->model54, 1);
        if (lbl_1_bss_0.roundEnding == 0) {
            work->state = 6;
            fn_1_2464(player);
        } else {
            work->state = 9;
            fn_1_22A8(player, 0);
        }
        Hu3DMotionSpeedSet(work->model20, 1.0f);
        work->actionFrame = 0;
        return;
    }
    Hu3DModelPosGet(work->model20, &pos);
    pos.x += (work->reflectedDirection.x * work->movementSpeed) / 2.0f;
    pos.z += (work->reflectedDirection.z * work->movementSpeed) / 2.0f;
    if (pos.x < -1150.0f) pos.x = -1150.0f;
    if (pos.x > 1150.0f) pos.x = 1150.0f;
    Hu3DModelPosSetV(work->model20, &pos);
    Hu3DMotionSpeedSet(work->model20, 0.3f);
}

/* During state 6, spins the body until the collision penalty expires, then enters the turn-back
 * state. */
void fn_1_473C(s16 player)
{
    Point3d bodyRotation;
    M650Player *playerWork;

    playerWork = &lbl_1_bss_28[player];
    if (--playerWork->throwWait != 0) {
        Hu3DModelRotGet(playerWork->model20, &bodyRotation);
        bodyRotation.y += 3.0f;
        if (bodyRotation.y >= 180.0f) {
            /* Keep the model yaw within the signed half-turn range. */
            bodyRotation.y -= 360.0f;
        }
        Hu3DModelRotSetV(playerWork->model20, &bodyRotation);
        return;
    }
    fn_1_22A8(player, 0);
    fn_1_23C4(player, 0, 8.0f);
    playerWork->state = 8;
    playerWork->turnAngle = 0.0f;
    playerWork->turnTowardPositiveAngle = 0;
}

/* During state 7's per-frame update, play joint motion 3 for 30 frames before entering state 8. */
void fn_1_48C0(s16 player)
{
    M650Player *playerWork;

    playerWork = &lbl_1_bss_28[player];
    if (playerWork->actionFrame++ == 0) {
        fn_1_22A8(player, 3);
    }
    if (playerWork->actionFrame >= 30) {
        fn_1_22A8(player, 0);
        playerWork->state = 8;
        playerWork->actionFrame = 0;
    }
}

/* During state 8's per-frame update, return body yaw to zero, then resume play or settle for
 * results. */
void fn_1_49C8(s16 player)
{
    Point3d bodyRotation;
    M650Player *playerWork;
    s16 sequenceMode;

    playerWork = &lbl_1_bss_28[player];
    sequenceMode = MgSeqModeGet();
    Hu3DModelRotGet(playerWork->model20, &bodyRotation);
    if (bodyRotation.y < 0.0f) {
        bodyRotation.y += 2.0f;
        if (bodyRotation.y > 0.0f) {
            /* Stop exactly at zero instead of stepping past it. */
            bodyRotation.y = 0.0f;
        }
    } else if (bodyRotation.y > 0.0f) {
        bodyRotation.y -= 2.0f;
        if (bodyRotation.y < 0.0f) {
            bodyRotation.y = 0.0f;
        }
    }
    Hu3DModelRotSetV(playerWork->model20, &bodyRotation);
    if (bodyRotation.y == 0.0f) {
        playerWork->movementSpeed = 0.0f;
        if (sequenceMode == 5) {
            if (lbl_1_bss_28[player].reachedGoal == 0) {
                playerWork->state = 1;
                playerWork->turnAngle = 0.0f;
                fn_1_289C(player);
            } else {
                playerWork->state = 10;
            }
        } else {
            playerWork->state = 9;
            fn_1_22A8(player, 0);
        }
        playerWork->boneVelocity.x = playerWork->boneVelocity.y = playerWork->boneVelocity.z = 0.0f;
    }
}

/* During player state 10, turn the character toward the results pose before starting its next
 * motion. */
void fn_1_4BD4(s16 playerIndex)
{
    Point3d bodyRotation;
    s16 sequenceMode;
    M650Player *playerWork;

    playerWork = &lbl_1_bss_28[playerIndex];
    /* The current sequence mode is read but does not affect this turn or its next motion. */
    sequenceMode = MgSeqModeGet();
    Hu3DModelRotGet(playerWork->model20, &bodyRotation);
    if (bodyRotation.y < 180.0f) {
        bodyRotation.y += 4.0f;
        if (bodyRotation.y > 180.0f) {
            bodyRotation.y = 180.0f;
        }
    } else if (bodyRotation.y > -180.0f) {
        bodyRotation.y -= 4.0f;
        if (bodyRotation.y < -180.0f) {
            /* This clamp cannot run: entering this branch required a yaw of at least 180
             * degrees. */
            bodyRotation.y = 180.0f;
        }
    }
    Hu3DModelRotSetV(playerWork->model20, &bodyRotation);
    if (bodyRotation.y == 180.0f) {
        playerWork->state = 9;
        fn_1_22A8(playerIndex, 3);
    }
}

/* Opening state 3 calls this each frame to split the fullscreen view into four player viewports. */
s16 fn_1_4D58(s16 transitionFrame)
{
    u16 cameraBits[4] = { 1, 2, 4, 8 };
    /* Index 0 is the viewport origin; index 1 is its width or height. */
    f32 viewportX[2];
    f32 viewportY[2];
    s32 cameraIndex;
    f32 outerViewportWidth;
    f32 outerViewportHeight;
    f32 horizontalInset;
    f32 verticalInset;

    outerViewportWidth = 640.0f - (5.3333335f * (f32) transitionFrame);
    outerViewportHeight = 480.0f - (4.0f * (f32) transitionFrame);
    horizontalInset = (16.0f * (f32) transitionFrame) / 60.0f;
    verticalInset = (40.0f * (f32) transitionFrame) / 60.0f;
    /* At frame zero, collapsed quadrants pass -2 scissor extents through unsigned casts without
     * clamping. */
    cameraIndex = 0;
    while (cameraIndex < 4) {
        switch (cameraIndex) {
        case 0:
            viewportX[0] = 0.0f;
            viewportY[0] = 0.0f;
            viewportX[1] = outerViewportWidth + horizontalInset;
            viewportY[1] = outerViewportHeight + verticalInset;
            Hu3DCameraScissorSet(cameraBits[cameraIndex], 0U, 0U, (u32) (outerViewportWidth - 2.0f),
                                 (u32) (outerViewportHeight - 2.0f));
            break;
        case 1:
            viewportX[0] = outerViewportWidth - horizontalInset;
            viewportY[0] = 0.0f;
            viewportX[1] = (640.0f - outerViewportWidth) + horizontalInset;
            viewportY[1] = outerViewportHeight + verticalInset;
            Hu3DCameraScissorSet(cameraBits[cameraIndex], (u32) (2.0f + outerViewportWidth), 0U,
                                 (u32) ((640.0f - outerViewportWidth) - 2.0f),
                                 (u32) (outerViewportHeight - 2.0f));
            break;
        case 2:
            viewportX[0] = 0.0f;
            viewportY[0] = outerViewportHeight - verticalInset;
            viewportX[1] = outerViewportWidth + horizontalInset;
            viewportY[1] = (480.0f - outerViewportHeight) + verticalInset;
            Hu3DCameraScissorSet(cameraBits[cameraIndex], 0U, (u32) (2.0f + outerViewportHeight),
                                 (u32) (outerViewportWidth - 2.0f),
                                 (u32) ((480.0f - outerViewportHeight) - 2.0f));
            break;
        case 3:
            viewportX[0] = outerViewportWidth - horizontalInset;
            viewportY[0] = outerViewportHeight - verticalInset;
            viewportX[1] = (640.0f - outerViewportWidth) + horizontalInset;
            viewportY[1] = (480.0f - outerViewportHeight) + verticalInset;
            Hu3DCameraScissorSet(cameraBits[cameraIndex], (u32) (2.0f + outerViewportWidth),
                                 (u32) (2.0f + outerViewportHeight),
                                 (u32) ((640.0f - outerViewportWidth) - 2.0f),
                                 (u32) ((480.0f - outerViewportHeight) - 2.0f));
            break;
        }
        Hu3DCameraViewportSet(cameraBits[cameraIndex], viewportX[0], viewportY[0], viewportX[1],
                              viewportY[1], 0.0f, 1.0f);
        cameraIndex += 1;
    }
    if (transitionFrame >= 60) {
        return 1;
    }
    return 0;
}

/* The setup callback loads the record outside Decathlon and creates an elapsed-time display
 * that counts from zero to 18000 frames. */
void fn_1_5258(void)
{
    if (_CheckFlag(FLAG_INST_DECA) == 0) {
        lbl_1_bss_0.record = GWRecordGet(GW_RECORD_M650);
        if ((s32) lbl_1_bss_0.record == 0) {
            /* Use a 60-second baseline when no saved record is present. */
            lbl_1_bss_0.record = 3600;
        }
        lbl_1_bss_0.timer = MgTimerCreate(1);
        MgTimerParamSet(lbl_1_bss_0.timer, 0, 18000, lbl_1_bss_0.record);
        MgTimerRecordDispOn(lbl_1_bss_0.timer);
    }
}

/* The gameplay-entry callback starts the shared timer when the timed mode is active. */
void fn_1_5308(void)
{
    if (_CheckFlag(FLAG_INST_DECA) == 0) {
        MgTimerModeOnSet(lbl_1_bss_0.timer, 0);
        HuAudFXPlay(MSM_SE_CMN_14);
    }
}

/* Finish detection outside Decathlon calls this when a runner crosses the goal to stop the shared
 * timer. */
void fn_1_5354(void)
{
    if (_CheckFlag(FLAG_INST_DECA) == 0) {
        MgTimerModeOffSet(lbl_1_bss_0.timer);
    }
}

/* Opening state 4 calls this after the four-view intro to create Decathlon elapsed-time displays
 * that count from zero to 5400 frames. */
void fn_1_5394(void)
{
    HuVecF positions[4] = {
        { 76.0f, 224.0f, 0.0f },
        { 516.0f, 224.0f, 0.0f },
        { 76.0f, 424.0f, 0.0f },
        { 516.0f, 424.0f, 0.0f },
    };
    s32 i;

    if (_CheckFlag(FLAG_INST_DECA) != 0) {
        i = 0;
        while (i < 4) {
            lbl_1_bss_28[i].timer = MgTimerCreate(2);
            MgTimerParamSet(lbl_1_bss_28[i].timer, 0, 5400, 0);
            MgTimerPosSet(lbl_1_bss_28[i].timer, positions[i].x, positions[i].y);
            MgTimerRecordDispOn(lbl_1_bss_28[i].timer);
            i += 1;
        }
    }
}

/* The frame-zero gameplay callback starts the Decathlon timers with a flash finish effect. */
void fn_1_54D4(void)
{
    s32 playerIndex;

    if (_CheckFlag(FLAG_INST_DECA) != 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            MgTimerModeOnSet(lbl_1_bss_28[playerIndex].timer, MGTIMER_OFFTYPE_FLASH);
            playerIndex += 1;
        }
        HuAudFXPlay(MSM_SE_CMN_14);
    }
}

/* Called when a Decathlon runner reaches the finish to stop its timer and trigger the flash
 * effect. */
void fn_1_5544(s16 playerIndex)
{
    if (_CheckFlag(FLAG_INST_DECA) != 0) {
        MgTimerModeOffSet(lbl_1_bss_28[playerIndex].timer);
        if ((s32) lbl_1_bss_28[playerIndex].timer->stopF == 0) {
            lbl_1_bss_28[playerIndex].timer->stopF = 1;
            /* Keep the active timer callback selected with counting stopped so it runs the flash
             * effect. */
            lbl_1_bss_28[playerIndex].timer->mode = MGTIMER_MODE_ON;
        }
    }
}

/* The frame-44 throw callback converts the bone hook height to launch speed and elevation. */
void fn_1_5600(s16 playerIndex, f32 hookHeight, f32 *launchSpeed, f32 *throwElevation)
{
    f32 height;

    height = 5.444444768958626 - hookHeight / 100.0f;
    *throwElevation = (f32) (180.0 * (atan2(height, 8.300000190734863) / 3.141592653589793));
    if (*throwElevation < 90.0f) {
        *launchSpeed =
            (f32) (8.300000190734863 /
                   (0.3333333432674408 * cos((3.141592653589793 * (f64) *throwElevation) / 180.0)));
        return;
    }
    *launchSpeed = 0.0f;
}

/* Called each frame by the player update to detect finish crossings and award the round. */
s16 fn_1_5728(void)
{
    Point3d modelPosition;
    s16 sequenceMode;
    s32 playerIndex;
    s32 elapsedTime;

    sequenceMode = MgSeqModeGet();
    if (sequenceMode != MGSEQ_MODE_MAIN) {
        return 0;
    }
    if (_CheckFlag(FLAG_INST_DECA) != 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            Hu3DModelPosGet(lbl_1_bss_28[playerIndex].model20, &modelPosition);
            if (modelPosition.z >= 5000.0f) {
                lbl_1_bss_28[playerIndex].timerValue =
                    MgTimerValueGet(lbl_1_bss_28[playerIndex].timer);
                fn_1_5544(playerIndex);
                lbl_1_bss_28[playerIndex].reachedGoal = 1;
            }
            playerIndex += 1;
        }
        playerIndex = 0;
        while (playerIndex < 4) {
            if (lbl_1_bss_28[playerIndex].reachedGoal != 0) {
                playerIndex += 1;
            } else {
                break;
            }
        }
        if (playerIndex >= 4) {
            return 1;
        }
    } else {
        /* If several players reach the goal this frame, the lowest player index wins. */
        playerIndex = 0;
        while (playerIndex < 4) {
            Hu3DModelPosGet(lbl_1_bss_28[playerIndex].model20, &modelPosition);
            if (modelPosition.z >= 5000.0f) {
                lbl_1_bss_0.winner = playerIndex;
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = 10;
                }
                elapsedTime = MgTimerValueGet(lbl_1_bss_0.timer);
                if ((lbl_1_bss_28[playerIndex].computerDifficulty == -1) &&
                    (elapsedTime < lbl_1_bss_0.record) &&
                    (_CheckFlag(FLAG_MG_PRACTICE) == 0)) {
                    MgSeqRecordSet(elapsedTime);
                    lbl_1_bss_0.recordChanged = 1;
                    GWRecordSet(GW_RECORD_M650, (u32) elapsedTime);
                }
                fn_1_5354();
                return 1;
            }
            playerIndex += 1;
        }
    }
    return 0;
}

/* Called each frame from the player update to move that player camera for its current state. */
void fn_1_5A24(s16 playerIndex)
{
    OM_CAMERA_VIEW view;
    Point3d pos;
    M650Player *playerWork;
    s16 seqMode;
    s16 time;

    playerWork = &lbl_1_bss_28[playerIndex];
    seqMode = MgSeqModeGet();
    if (seqMode == 5) {
        Hu3DModelPosGet(playerWork->model20, &pos);
        switch (playerWork->state) {                  /* irregular */
        case 1:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -27.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1300.0f;
            time = 30;
            break;
        case 2:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -8.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 500.0f;
            time = 30;
            break;
        case 3:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -8.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -15.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        case 10:
            view.center.x = pos.x;
            view.center.y = 180.0f;
            view.center.z = pos.z;
            view.rot.x = -11.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 2000.0f;
            time = 30;
            break;
        case 9:
            if (_CheckFlag(FLAG_INST_DECA) != 0) {
                view.center.x = pos.x;
                view.center.y = 180.0f;
                view.center.z = pos.z;
                view.rot.x = -11.0f;
                view.rot.y = -180.0f;
                view.rot.z = 0.0f;
                view.zoom = 2000.0f;
                time = 30;
            } else {
                view.center.x = pos.x;
                view.center.y = 280.0f;
                view.center.z = pos.z;
                view.rot.x = -15.0f;
                view.rot.y = -180.0f;
                view.rot.z = 0.0f;
                view.zoom = 1000.0f;
                time = 30;
            }
            break;
        default:
            view.center.x = pos.x;
            view.center.y = 230.0f;
            view.center.z = pos.z;
            view.rot.x = -15.0f;
            view.rot.y = -180.0f;
            view.rot.z = 0.0f;
            view.zoom = 1000.0f;
            time = 30;
            break;
        }
        omCameraViewMoveSimpleMulti(lbl_1_data_150[playerIndex], &view, time);
    }
}

/* No caller is registered here; holding pad 0's enable button adjusts all four views but prints
 * only camera 0. The first RotZ label displays its X rotation. */
void fn_1_5E40(void)
{
    s32 i;

    if ((HuPadBtn[0] & 8192) != 0) {
        for (i = 0; i < 4; i++) {
            if ((HuPadBtn[0] & 1024) != 0) {
                CZoomM[i] += 5.0f;
            }
            if ((HuPadBtn[0] & 2048) != 0) {
                CZoomM[i] -= 5.0f;
            }
            if (HuPadSubStkY[0] > 32) {
                CRotM[i].x += 1.0f;
            }
            if (HuPadSubStkY[0] < -32) {
                CRotM[i].x -= 1.0f;
            }
            if (HuPadSubStkX[0] > 32) {
                CRotM[i].y += 1.0f;
            }
            if (HuPadSubStkX[0] < -32) {
                CRotM[i].y -= 1.0f;
            }
            if ((HuPadStkX[0] > 5) || (HuPadStkX[0] < -5)) {
                CenterM[i].x += HuPadStkX[0];
            }
            if ((HuPadStkY[0] > 5) || (HuPadStkY[0] < -5)) {
                CenterM[i].z -= HuPadStkY[0];
            }
            if ((HuPadBtn[0] & 8) != 0) {
                CenterM[i].y += 5.0f;
            }
            if ((HuPadBtn[0] & 4) != 0) {
                CenterM[i].y -= 5.0f;
            }
        }
        print8(100, 300, 1.8f, "CnrX:%f", CenterM->x);
        print8(100, 316, 1.8f, "CnrY:%f", CenterM->y);
        print8(100, 332, 1.8f, "CnrZ:%f", CenterM->z);
        print8(100, 348, 1.8f, "RotZ:%f", CRotM->x);
        print8(100, 364, 1.8f, "RotY:%f", CRotM->y);
        print8(100, 380, 1.8f, "RotZ:%f", CRotM->z);
        print8(100, 396, 1.8f, "Zoom:%f", CZoomM[0]);
    }
}
