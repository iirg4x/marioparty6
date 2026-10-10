/* Creates the minigame camera, stage models, players, and sequence on entry. */
#include "REL/m633dll.h"

/* Called by the REL prolog to initialize minigame state, configure camera 1, and register the
 * sequence callbacks. */
void fn_1_128C(void)
{
    s32 setupIndex;
    f32 timerX;
    f32 timerY;

    lbl_1_bss_0.workProcess = MgActorObjectSetup();
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

    CRot.x = -35.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    Center.x = 0.0f;
    Center.y = 700.0f;
    Center.z = 400.0f;
    CZoom = 1000.0f;
    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 8000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);

    timerX = 528.0f;
    timerY = 64.0f;
    lbl_1_bss_0.timer = MgTimerCreate(0);
    MgTimerPosSet(lbl_1_bss_0.timer, timerX, timerY);
    lbl_1_bss_0.object = omAddObjEx(lbl_1_bss_0.workProcess, 8192, 1U, 5U, -1, fn_1_1598);
    MgSeqCreate(&lbl_1_data_0);
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
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 3), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(lbl_1_bss_0.placementModelId, HU3D_ATTR_DISPOFF);
    setupIndex = 0;
    while (setupIndex < 4) {
        lbl_1_bss_0.cameraMotionIds[setupIndex] = Hu3DMotionCreate(
            HuDataSelHeapReadNum(lbl_1_data_1B8[setupIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_0.cameraModelIds[setupIndex] =
            Hu3DModelCameraCreate(lbl_1_bss_0.cameraMotionIds[setupIndex], 1U);
        Hu3DCameraMotionOff(lbl_1_bss_0.cameraModelIds[setupIndex]);
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
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m606, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    setupIndex = 1;
    while (setupIndex < 4) {
        lbl_1_bss_0.hiddenLoopingModelIds[setupIndex] =
            Hu3DModelLink(lbl_1_bss_0.hiddenLoopingModelIds[0]);
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelLayerSet(lbl_1_bss_0.hiddenLoopingModelIds[setupIndex], 5);
        setupIndex += 1;
    }
    lbl_1_bss_0.rotatingStageModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 16), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.secondRotatingModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 17), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.stageIdleMotionId =
        Hu3DJointMotion(lbl_1_bss_0.rotatingStageModelId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 18), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.stageAuxiliaryIds[0] =
        Hu3DJointMotion(lbl_1_bss_0.rotatingStageModelId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.rotatingStageModelId, 5);
    Hu3DModelLayerSet(lbl_1_bss_0.secondRotatingModelId, 4);
    Hu3DModelShadowMapObjSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_104);
    Hu3DMotionSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageIdleMotionId);
    Hu3DReflectNoSet(2);
    lbl_1_bss_0.stageAuxiliaryIds[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.segmentBaseModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 21), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.stageAuxiliaryIds[1], 6);
    Hu3DModelLayerSet(lbl_1_bss_0.segmentBaseModelId, 6);
    setupIndex = 0;
    while (setupIndex < 50) {
        if (setupIndex == 0) {
            lbl_1_bss_0.segmentEndpointModelIds[setupIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m633, 22), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_0.segmentEndpointModelIds[setupIndex] =
                Hu3DModelLink(lbl_1_bss_0.segmentEndpointModelIds[0]);
        }
        Hu3DModelLayerSet(lbl_1_bss_0.segmentEndpointModelIds[setupIndex], 6);
        Hu3DModelAttrSet(lbl_1_bss_0.segmentEndpointModelIds[setupIndex], HU3D_ATTR_DISPOFF);
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 2) {
        lbl_1_bss_0.nozzleModelIds[setupIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m633, 23), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelLayerSet(lbl_1_bss_0.nozzleModelIds[setupIndex], 5);
        setupIndex += 1;
    }
    setupIndex = 0;
    while (setupIndex < 4) {
        lbl_1_bss_0.firstNozzleMotionIds[setupIndex] = Hu3DJointMotion(
            lbl_1_bss_0.nozzleModelIds[0],
            HuDataSelHeapReadNum(lbl_1_data_1D0[setupIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_0.secondNozzleMotionIds[setupIndex] = Hu3DJointMotion(
            lbl_1_bss_0.nozzleModelIds[1],
            HuDataSelHeapReadNum(lbl_1_data_1D0[setupIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        setupIndex += 1;
    }
    Hu3DModelHookSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_11F,
                     lbl_1_bss_0.nozzleModelIds[0]);
    Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_11F, &nozzlePosition);
    nozzleDirection = nozzlePosition;
    PSVECNormalize(&nozzleDirection, &nozzleDirection);
    nozzleDistance = PSVECMag(&nozzlePosition);
    nozzlePosition.x += nozzleDirection.x * -(2.0f * nozzleDistance);
    nozzlePosition.z += nozzleDirection.z * -(2.0f * nozzleDistance);
    Hu3DModelPosSetV(lbl_1_bss_0.nozzleModelIds[1], &nozzlePosition);
    *stageObject->mdlId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    collisionModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 2), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(*stageObject->mdlId, 2);
    stageObject->mtnId[0] =
        Hu3DJointMotion(*stageObject->mdlId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    stageObject->mtnId[1] =
        Hu3DJointMotion(*stageObject->mdlId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    stageObject->mtnId[2] =
        Hu3DJointMotion(*stageObject->mdlId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 6), HU_MEMNUM_OVL, HEAP_MODEL));
    stageObject->mtnId[3] =
        Hu3DJointMotion(*stageObject->mdlId,
                        HuDataSelHeapReadNum(DATANUM(DATA_m633, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(*stageObject->mdlId, stageObject->mtnId[0]);
    Hu3DModelAttrSet(*stageObject->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(*stageObject->mdlId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(collisionModelId, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapObjSet(*stageObject->mdlId, lbl_1_data_136);
    lbl_1_bss_0.arenaModelId =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 1), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelLayerSet(lbl_1_bss_0.arenaModelId, 2);
    lbl_1_bss_0.arenaPhaseModelIds[0] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.arenaPhaseModelIds[1] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.arenaPhaseModelIds[2] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 10), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_0.arenaPhaseModelIds[3] =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m633, 11), HU_MEMNUM_OVL, HEAP_MODEL));
    setupIndex = 0;
    while (setupIndex < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.arenaPhaseModelIds[setupIndex], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(lbl_1_bss_0.arenaPhaseModelIds[setupIndex], 2);
        setupIndex += 1;
    }
    Hu3DModelAttrReset(lbl_1_bss_0.arenaPhaseModelIds[0], HU3D_ATTR_DISPOFF);
    MgActorColMapInit(&collisionModelId, 1, 25);
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
        lbl_1_bss_0.characterNumbers[setupIndex] = (s32) GwPlayerConf[setupIndex].charNo;
        lbl_1_bss_0.padNumbers[setupIndex] = (s32) GwPlayerConf[setupIndex].padNo;
        if (GwPlayerConf[setupIndex].grpNo == 0) {
            lbl_1_bss_0.outsideGroupZero[setupIndex] = 0;
        } else {
            lbl_1_bss_0.outsideGroupZero[setupIndex] = 1;
        }
        /* Character 3 uses a separate set of player motions. Player creation rewrites entry 4 of
         * the chosen shared motion table from CHARMOT_HSF_c000m1_304 to CHARMOT_HSF_c000m1_464
         * in place. */
        if ((s32) lbl_1_bss_0.characterNumbers[setupIndex] == 3) {
            lbl_1_bss_0.players[setupIndex] =
                MgPlayerCreate((s16) setupIndex, &actorParam, 2, 1U, -15U, lbl_1_data_220);
        } else {
            lbl_1_bss_0.players[setupIndex] =
                MgPlayerCreate((s16) setupIndex, &actorParam, 2, 1U, -15U, lbl_1_data_1E0);
        }
        MgPlayerVibrateCreate(lbl_1_bss_0.players[setupIndex]);
        /* Each player actor receives the next parameter value. */
        actorParam.param += 1;
        playerLightId = Hu3DLLightCreate((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId, 0.0f,
                                         0.0f, 0.0f, 0.0f, 0.0f, -1.0f, 160U, 160U, 160U);
        Hu3DLLightInfinitytSet((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId, playerLightId);
        /* Group-zero players attach to the stage and are paused; other-group players are placed at
         * result markers and initialized for computer movement. */
        if ((s32) lbl_1_bss_0.outsideGroupZero[setupIndex] == 0) {
            Hu3DModelHookSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_14F,
                             (s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId);
            MgPlayerDespawn(lbl_1_bss_0.players[setupIndex]);
            lbl_1_bss_0.playerObjects[setupIndex] =
                omAddObjEx(lbl_1_bss_0.workProcess, 32730, 0U, 0U, -1, fn_1_26B4);
            CharMotionShiftSet((s16) lbl_1_bss_0.characterNumbers[setupIndex],
                               lbl_1_bss_0.players[setupIndex]->omObj->mtnId[13], 0.0f, 0.0f,
                               HU3D_MOTATTR_LOOP);
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_0.placementModelId, lbl_1_data_F4[resultMarkerIndex],
                               &aiSpawnPosition);
            lbl_1_bss_0.players[setupIndex]->actor->pos = aiSpawnPosition;
            lbl_1_bss_0.playerObjects[setupIndex] =
                omAddObjEx(lbl_1_bss_0.workProcess, 32730, 0U, 0U, -1, fn_1_34AC);
            MgPlayerDespawn(lbl_1_bss_0.players[setupIndex]);
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
        Hu3DModelShadowSet((s16) lbl_1_bss_0.players[setupIndex]->actor->mdlId);
        setupIndex += 1;
    }
    CharEffectLayerSet(5);
    lbl_1_bss_0.rotationSoundHandle = -1;
    lbl_1_bss_0.bgmHandle = -1;
    fn_1_70A8(3);
    stageObject->objFunc = NULL;
}
