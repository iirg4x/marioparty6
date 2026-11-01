/* Creates the minigame camera, stage models, players, and sequence on entry. */
#include <math.h>
#include "REL/m633dll.h"

/* Called by the REL prolog to initialize minigame state, configure camera 1, and register the
 * sequence callbacks. */
void fn_1_128C(void)
{
    s32 setupIndex;
    f32 timerX;
    f32 timerY;

    lbl_1_bss_0.workProcess = fn_80080418();
    lbl_1_bss_0.arenaYawDegrees = 0.0f;
    lbl_1_bss_0.segmentCount = 0;
    lbl_1_bss_0.arenaPhase = 3;
    lbl_1_bss_0.activePlayerCount = 3;
    lbl_1_bss_0.previousRotationButtonMask = 0;
    lbl_1_bss_0.nozzleCollisionRadius = 0.0f;

    setupIndex = 0;
    while (setupIndex < 4) {
        memset(&lbl_1_bss_0.aiStates[setupIndex], 0, sizeof(M633AI));
        lbl_1_bss_0.aiStates[setupIndex].decisionCounter = -1;
        lbl_1_bss_0.aiStates[setupIndex].reservedDecisionWord = -1;
        setupIndex += 1;
    }

    setupIndex = 0;
    while (setupIndex < 50) {
        lbl_1_bss_0.segments[setupIndex].endpointModelIndex = setupIndex;
        setupIndex += 1;
    }

    lbl_80266A40.x = -35.0f;
    lbl_80266A40.y = 0.0f;
    lbl_80266A40.z = 0.0f;
    lbl_80266A4C.x = 0.0f;
    lbl_80266A4C.y = 700.0f;
    lbl_80266A4C.z = 400.0f;
    lbl_802C028C = 1000.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);

    timerX = 528.0f;
    timerY = 64.0f;
    lbl_1_bss_0.timer = MgTimerCreate(0);
    MgTimerPosSet(lbl_1_bss_0.timer, timerX, timerY);
    lbl_1_bss_0.object = fn_8003AB38(lbl_1_bss_0.workProcess, 8192, 1U, 5U, -1, fn_1_1598);
    fn_80070788(&lbl_1_data_0);
}

/* The one-shot stage object callback creates arena assets and player objects, then clears its
 * callback after setup. */
void fn_1_1598(OMOBJ *stageObject)
{
    Point3d shadowPosition;
    Point3d shadowNormal;
    Point3d shadowTarget;
    Point3d nozzlePosition;
    Point3d nozzleDirection;
    Point3d aiSpawnPosition;
    MGACTOR_PARAM actorParam;
    s16 collisionModelId;
    f32 nozzleDistance;
    f32 aiTurnDirection;
    s32 resultMarkerIndex;
    s32 setupIndex;
    HU3D_LIGHTID lightId;
    HU3D_LLIGHTID playerLightId;

    lbl_1_bss_0.placementModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 3), 0x10000000, 2));
    fn_800269E8(lbl_1_bss_0.placementModelId, (1 << 0));
    setupIndex = 0;
    while (setupIndex < 4) {
        lbl_1_bss_0.cameraMotionIds[setupIndex] = fn_8002C4C0(
            fn_80008DD0(lbl_1_data_1B8[setupIndex], 0x10000000, 2));
        lbl_1_bss_0.cameraModelIds[setupIndex] =
            fn_800283F4(lbl_1_bss_0.cameraMotionIds[setupIndex], 1U);
        fn_8002858C(lbl_1_bss_0.cameraModelIds[setupIndex]);
        setupIndex += 1;
    }
    lbl_1_bss_0.activeCameraModelId = -1;
    lightId = Hu3DGLightCreate(0.0f, 300.0f, 0.0f, 0.0f, 1.0f, 0.0f, 255U, 255U, 255U);
    Hu3DGLightPointSet(lightId, 1600.0f, 1.0f, GX_DA_OFF);
    Hu3DGLightStaticSet(0, 1);
    shadowPosition.x = 0.0f;
    shadowPosition.y = 10000.0f;
    shadowPosition.z = 0.0f;
    shadowNormal.x = 0.0f;
    shadowNormal.y = 1.0f;
    shadowNormal.z = 0.0f;
    shadowTarget.x = 0.0f;
    shadowTarget.y = 0.0f;
    shadowTarget.z = -0.1f;
    Hu3DShadowCreate(8.5f, 5000.0f, 13000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowNormal, &shadowTarget);
    lbl_1_bss_0.hiddenLoopingModelIds[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x410000, 20), 0x10000000, 2));
    setupIndex = 1;
    while (setupIndex < 4) {
        lbl_1_bss_0.hiddenLoopingModelIds[setupIndex] =
            fn_80025D4C(lbl_1_bss_0.hiddenLoopingModelIds[0]);
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 4) {
        fn_800269E8(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], (1 << 0));
        fn_800269E8(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], ((1 << 30) | (1 << 0)));
        fn_80026B8C(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], 5);
        setupIndex += 1;
    }
    lbl_1_bss_0.rotatingStageModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 16), 0x10000000, 2));
    lbl_1_bss_0.secondRotatingModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 17), 0x10000000, 2));
    lbl_1_bss_0.stageIdleMotionId =
        fn_80030458(lbl_1_bss_0.rotatingStageModelId,
                        fn_80008DD0(DATANUM(0x5c0000, 18), 0x10000000, 2));
    lbl_1_bss_0.stageAuxiliaryIds[0] =
        fn_80030458(lbl_1_bss_0.rotatingStageModelId,
                        fn_80008DD0(DATANUM(0x5c0000, 19), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_0.rotatingStageModelId, 5);
    fn_80026B8C(lbl_1_bss_0.secondRotatingModelId, 4);
    fn_80027080(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_104);
    fn_8002C95C(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageIdleMotionId);
    fn_8002A2A4(2);
    lbl_1_bss_0.stageAuxiliaryIds[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 20), 0x10000000, 2));
    lbl_1_bss_0.segmentBaseModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 21), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_0.stageAuxiliaryIds[1], 6);
    fn_80026B8C(lbl_1_bss_0.segmentBaseModelId, 6);
    setupIndex = 0;
    while (setupIndex < 50) {
        if (setupIndex == 0) {
            lbl_1_bss_0.segmentEndpointModelIds[setupIndex] = fn_80025A04(
                fn_80008DD0(DATANUM(0x5c0000, 22), 0x10000000, 2));
        } else {
            lbl_1_bss_0.segmentEndpointModelIds[setupIndex] =
                fn_80025D4C(lbl_1_bss_0.segmentEndpointModelIds[0]);
        }
        fn_80026B8C(lbl_1_bss_0.segmentEndpointModelIds[setupIndex], 6);
        fn_800269E8(lbl_1_bss_0.segmentEndpointModelIds[setupIndex], (1 << 0));
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 2) {
        lbl_1_bss_0.nozzleModelIds[setupIndex] = fn_80025A04(
            fn_80008DD0(DATANUM(0x5c0000, 23), 0x10000000, 2));
        fn_80026B8C(lbl_1_bss_0.nozzleModelIds[setupIndex], 5);
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 4) {
        lbl_1_bss_0.firstNozzleMotionIds[setupIndex] = fn_80030458(
            lbl_1_bss_0.nozzleModelIds[0],
            fn_80008DD0(lbl_1_data_1D0[setupIndex], 0x10000000, 2));
        lbl_1_bss_0.secondNozzleMotionIds[setupIndex] = fn_80030458(
            lbl_1_bss_0.nozzleModelIds[1],
            fn_80008DD0(lbl_1_data_1D0[setupIndex], 0x10000000, 2));
        setupIndex += 1;
    }
    fn_8002741C(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_11F,
                     lbl_1_bss_0.nozzleModelIds[0]);
    Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_11F, &nozzlePosition);
    nozzleDirection = nozzlePosition;
    PSVECNormalize(&nozzleDirection, &nozzleDirection);
    nozzleDistance = PSVECMag(&nozzlePosition);
    nozzlePosition.x += nozzleDirection.x * -(2.0f * nozzleDistance);
    nozzlePosition.z += nozzleDirection.z * -(2.0f * nozzleDistance);
    fn_80026790(lbl_1_bss_0.nozzleModelIds[1], &nozzlePosition);
    *stageObject->mdlId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 0), 0x10000000, 2));
    collisionModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 2), 0x10000000, 2));
    fn_80026B8C(*stageObject->mdlId, 2);
    stageObject->mtnId[0] =
        fn_80030458(*stageObject->mdlId,
                        fn_80008DD0(DATANUM(0x5c0000, 4), 0x10000000, 2));
    stageObject->mtnId[1] =
        fn_80030458(*stageObject->mdlId,
                        fn_80008DD0(DATANUM(0x5c0000, 5), 0x10000000, 2));
    stageObject->mtnId[2] =
        fn_80030458(*stageObject->mdlId,
                        fn_80008DD0(DATANUM(0x5c0000, 6), 0x10000000, 2));
    stageObject->mtnId[3] =
        fn_80030458(*stageObject->mdlId,
                        fn_80008DD0(DATANUM(0x5c0000, 7), 0x10000000, 2));
    fn_8002C95C(*stageObject->mdlId, stageObject->mtnId[0]);
    fn_800269E8(*stageObject->mdlId, ((1 << 30) | (1 << 0)));
    fn_80026A44(*stageObject->mdlId, (1 << 0));
    fn_800269E8(collisionModelId, (1 << 0));
    fn_80027080(*stageObject->mdlId, lbl_1_data_136);
    lbl_1_bss_0.arenaModelId =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 1), 0x10000000, 2));
    fn_80026B8C(lbl_1_bss_0.arenaModelId, 2);
    lbl_1_bss_0.arenaPhaseModelIds[0] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 8), 0x10000000, 2));
    lbl_1_bss_0.arenaPhaseModelIds[1] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 9), 0x10000000, 2));
    lbl_1_bss_0.arenaPhaseModelIds[2] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 10), 0x10000000, 2));
    lbl_1_bss_0.arenaPhaseModelIds[3] =
        fn_80025A04(fn_80008DD0(DATANUM(0x5c0000, 11), 0x10000000, 2));
    setupIndex = 0;
    while (setupIndex < 4) {
        fn_800269E8(lbl_1_bss_0.arenaPhaseModelIds[setupIndex], (1 << 0));
        fn_80026B8C(lbl_1_bss_0.arenaPhaseModelIds[setupIndex], 2);
        setupIndex += 1;
    }
    fn_80026A44(lbl_1_bss_0.arenaPhaseModelIds[0], (1 << 0));
    fn_800804E8(&collisionModelId, 1, 25);
    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.param = 0;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = 0;
    actorParam.correctHook = 0;
    resultMarkerIndex = 1;
    setupIndex = 0;
    while (setupIndex < 4) {
        lbl_1_bss_0.characterNumbers[setupIndex] = (s32) lbl_80265728[setupIndex].charNo;
        lbl_1_bss_0.padNumbers[setupIndex] = (s32) lbl_80265728[setupIndex].padNo;
        if (lbl_80265728[setupIndex].grpNo == 0) {
            lbl_1_bss_0.outsideGroupZero[setupIndex] = 0;
        } else {
            lbl_1_bss_0.outsideGroupZero[setupIndex] = 1;
        }
        /* Character 3 uses a separate set of player motions. Player creation rewrites entry 4 of
         * the chosen shared motion table from CHARMOT_HSF_c000m1_304 to CHARMOT_HSF_c000m1_464
         * in place. */
        if ((s32) lbl_1_bss_0.characterNumbers[setupIndex] == 3) {
            lbl_1_bss_0.players[setupIndex] =
                fn_80080728((s16) setupIndex, &actorParam, 2, 1U, -15U, lbl_1_data_220);
        } else {
            lbl_1_bss_0.players[setupIndex] =
                fn_80080728((s16) setupIndex, &actorParam, 2, 1U, -15U, lbl_1_data_1E0);
        }
        fn_8007ED60(lbl_1_bss_0.players[setupIndex]);
        /* Each player actor receives the next parameter value. */
        actorParam.param += 1;
        playerLightId = Hu3DLLightCreate((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId, 0.0f,
                                         0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        fn_80028B58((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId, playerLightId);
        /* Group-zero players attach to the stage and are paused; other-group players are placed at
         * result markers and initialized for computer movement. */
        if ((s32) lbl_1_bss_0.outsideGroupZero[setupIndex] == 0) {
            fn_8002741C(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_14F,
                             (s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId);
            fn_80086E3C(lbl_1_bss_0.players[setupIndex]);
            lbl_1_bss_0.playerObjects[setupIndex] =
                fn_8003AB38(lbl_1_bss_0.workProcess, 32730, 0U, 0U, -1, fn_1_26B4);
            fn_8005C2C8((s16) lbl_1_bss_0.characterNumbers[setupIndex],
                               lbl_1_bss_0.players[setupIndex]->omObj->mtnId[13], 0.0f, 0.0f,
                               ((1 << 30) | (1 << 0)));
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_0.placementModelId, lbl_1_data_F4[resultMarkerIndex],
                               &aiSpawnPosition);
            lbl_1_bss_0.players[setupIndex]->actor->pos = aiSpawnPosition;
            lbl_1_bss_0.playerObjects[setupIndex] =
                fn_8003AB38(lbl_1_bss_0.workProcess, 32730, 0U, 0U, -1, fn_1_34AC);
            fn_80086E3C(lbl_1_bss_0.players[setupIndex]);
            lbl_1_bss_0.aiStates[setupIndex].targetHeadingDegrees =
                (f32) (180.0 * (atan2((f64) aiSpawnPosition.z, (f64) aiSpawnPosition.x) /
                                3.141592653589793));
            if (frandmod(100) < 50U) {
                aiTurnDirection = -1.0f;
            } else {
                aiTurnDirection = 1.0f;
            }
            lbl_1_bss_0.aiStates[setupIndex].turnDirection = aiTurnDirection;
            resultMarkerIndex += 1;
        }
        lbl_1_bss_0.playerObjects[setupIndex]->work[0] = setupIndex;
        lbl_1_bss_0.playerObjects[setupIndex]->work[1] = 0;
        fn_80026E44((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId);
        setupIndex += 1;
    }
    fn_8005DDEC(5);
    lbl_1_bss_0.rotationSoundHandle = -1;
    lbl_1_bss_0.bgmHandle = -1;
    fn_1_70A8(3);
    stageObject->objFunc = NULL;
}
