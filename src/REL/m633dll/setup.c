/* Player object callbacks for controlling the rotating obstacle and handling hits. */
#include "REL/m633dll.h"

/* Creates two moving segments from the stage nozzle and plays their launch sound. */
void fn_1_258C(void)
{
    Point3d nozzlePosition;
    Point3d nozzleDirection;
    f32 nozzleDistance;

    Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &nozzlePosition);
    nozzlePosition.y -= 50.0f;
    nozzleDirection = nozzlePosition;
    PSVECNormalize(&nozzleDirection, &nozzleDirection);
    lbl_1_bss_0.segmentUpdateCountdown = 90;
    fn_1_5ED0(&nozzlePosition, &nozzleDirection);
    nozzleDistance = PSVECMag(&nozzlePosition);
    nozzlePosition.x += nozzleDirection.x * -(2.0f * nozzleDistance);
    nozzlePosition.z += nozzleDirection.z * -(2.0f * nozzleDistance);
    nozzleDirection.x = -nozzleDirection.x;
    nozzleDirection.z = -nozzleDirection.z;
    fn_1_5ED0(&nozzlePosition, &nozzleDirection);
    HuAudFXPlay(M633_SEGMENT_SPAWN_SE_ID);
}

void fn_1_26B4(OMOBJ *obj)
{

}

/* Creates a segment when the arena reaches phase 2 and waits for live segments to expire. */
void fn_1_26B8(OMOBJ *obj)
{
    Point3d nozzleDirection;
    Point3d nozzlePosition;
    MGPLAYER *player;
    s16 playerModelId;
    u32 playerNo;
    f32 nozzleDistance;

    playerNo = obj->work[0];
    player = lbl_1_bss_0.players[playerNo];
    playerModelId = player->actor->mdlId;
    switch (obj->work[1]) {
    case 0:
        fn_1_70A8(0);
        obj->work[1] = 1;
        break;
    case 1:
        if (lbl_1_bss_0.arenaPhase == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &nozzlePosition);
            nozzlePosition.y -= 50.0f;
            nozzleDirection = nozzlePosition;
            PSVECNormalize(&nozzleDirection, &nozzleDirection);
            lbl_1_bss_0.segmentUpdateCountdown = 90;
            fn_1_5ED0(&nozzlePosition, &nozzleDirection);
            nozzleDistance = PSVECMag(&nozzlePosition);
            nozzlePosition.x += nozzleDirection.x * -(2.0f * nozzleDistance);
            nozzlePosition.z += nozzleDirection.z * -(2.0f * nozzleDistance);
            nozzleDirection.x = -nozzleDirection.x;
            nozzleDirection.z = -nozzleDirection.z;
            fn_1_5ED0(&nozzlePosition, &nozzleDirection);
            HuAudFXPlay(M633_SEGMENT_SPAWN_SE_ID);
            obj->work[1] = 2;
            fn_1_70A8(1);
        }
        break;
    case 2:
        if (lbl_1_bss_0.segmentCount == 0) {
            obj->work[1] = 4;
        }
        break;
    }
}

/* Per-frame player control: turns the arena models and checks active players near either point measured from its nozzle. */
void fn_1_28A4(OMOBJ *obj)
{
    Point3d oppositeNozzlePosition;
    Point3d oppositeNozzleDirection;
    Point3d segmentNozzlePosition;
    Point3d secondNozzlePosition;
    Point3d segmentNozzleDirection;
    Point3d playerPosition;
    Point3d playerToNozzleDelta;
    Point3d eliminatedPlayerPosition;
    Point3d nozzleDirection;
    Point3d nozzlePosition;
    Point3d projectedPlayerPosition;
    f32 segmentDistance;
    f32 oppositeNozzleDistance;
    f32 offsetNozzleDistance;
    f32 nozzleDistance;
    s32 playerIndex;
    u32 playerNo;
    u16 buttonMask;
    MGPLAYER *player;
    s32 soundPanning;
    s32 collisionResult;
    s32 buttonChanged;
    u16 buttonState;
    u16 pressedButtonState;
    u16 directionButtonMask;
    s16 modelId;
    s32 sound;


    playerNo = obj->work[0];
    if (GwPlayerConf[playerNo].type != 0) {
        buttonState = lbl_1_bss_0.aiButtons;
        pressedButtonState = lbl_1_bss_0.aiPressedButtons;
    } else {
        buttonState = HuPadBtn[lbl_1_bss_0.padNumbers[playerNo]];
        pressedButtonState = HuPadBtnDown[lbl_1_bss_0.padNumbers[playerNo]];
    }
    if (lbl_1_bss_0.segmentCount == 0) {
        directionButtonMask = PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R;
        buttonMask = buttonState & directionButtonMask;
        buttonChanged = 0;
        if (buttonMask == directionButtonMask) {
            buttonMask = 0;
        }
        if (lbl_1_bss_0.previousDirectionButtons != buttonMask) {
            buttonChanged = 1;
        }
        lbl_1_bss_0.previousDirectionButtons = buttonMask;
        if (buttonMask == PAD_BUTTON_TRIGGER_R) {
            lbl_1_bss_0.arenaYawDegrees += 358.0f;
            if (buttonChanged != 0) {
                CharMotionShiftSet((s16) lbl_1_bss_0.characterNumbers[playerNo], lbl_1_bss_0.players[playerNo]->omObj->mtnId[12], 6.0f, 6.0f, HU3D_MOTATTR_REV);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageAuxiliaryIds[0], 6.0f, 6.0f, HU3D_MOTATTR_REV);
            }
            if (lbl_1_bss_0.rotationSoundHandle == -1) {
                lbl_1_bss_0.rotationSoundHandle = HuAudFXPlay(M633_ROTATION_CONTROL_SE_ID);
            }
        } else if (buttonMask == PAD_BUTTON_TRIGGER_L) {
            lbl_1_bss_0.arenaYawDegrees += 2.0f;
            if (buttonChanged != 0) {
                Hu3DModelAttrReset((s16) lbl_1_bss_0.players[playerNo]->actor->mdlId, HU3D_MOTATTR_REV);
                CharMotionShiftSet((s16) lbl_1_bss_0.characterNumbers[playerNo], lbl_1_bss_0.players[playerNo]->omObj->mtnId[12], 6.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageAuxiliaryIds[0], 6.0f, 6.0f, 0U);
            }
            if (lbl_1_bss_0.rotationSoundHandle == -1) {
                lbl_1_bss_0.rotationSoundHandle = HuAudFXPlay(M633_ROTATION_CONTROL_SE_ID);
            }
        } else {
            player = lbl_1_bss_0.players[playerNo];
            modelId = player->actor->mdlId;
            if ((Hu3DMotionShiftIDGet(modelId) < 0) && (player->omObj->mtnId[13] != Hu3DMotionIDGet(modelId))) {
                CharMotionShiftSet(player->charNo, player->omObj->mtnId[13], 0.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageIdleMotionId, 0.0f, 6.0f, 0U);
            }
            if (lbl_1_bss_0.rotationSoundHandle != -1) {
                HuAudFXStop(lbl_1_bss_0.rotationSoundHandle);
                lbl_1_bss_0.rotationSoundHandle = -1;
            }
        }
        while (lbl_1_bss_0.arenaYawDegrees >= 360.0f) {
            lbl_1_bss_0.arenaYawDegrees -= 360.0f;
        }
        Hu3DModelRotSet(lbl_1_bss_0.rotatingStageModelId, 0.0f, lbl_1_bss_0.arenaYawDegrees, 0.0f);
        Hu3DModelRotSet(lbl_1_bss_0.secondRotatingModelId, 0.0f, lbl_1_bss_0.arenaYawDegrees, 0.0f);
        buttonMask = pressedButtonState;
        if ((lbl_1_bss_0.arenaPhase == 2) && ((s32) (buttonMask & PAD_BUTTON_A) != 0)) {
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &nozzlePosition);
            nozzlePosition.y -= 50.0f;
            nozzleDirection = nozzlePosition;
            PSVECNormalize(&nozzleDirection, &nozzleDirection);
            lbl_1_bss_0.segmentUpdateCountdown = 90;
            fn_1_5ED0(&nozzlePosition, &nozzleDirection);
            nozzleDistance = PSVECMag(&nozzlePosition);
            nozzlePosition.x += nozzleDirection.x * -(2.0f * nozzleDistance);
            nozzlePosition.z += nozzleDirection.z * -(2.0f * nozzleDistance);
            nozzleDirection.x = -nozzleDirection.x;
            nozzleDirection.z = -nozzleDirection.z;
            fn_1_5ED0(&nozzlePosition, &nozzleDirection);
            HuAudFXPlay(M633_SEGMENT_SPAWN_SE_ID);
            fn_1_70A8(1);
            omVibrate((s16) playerNo, 20, 4, 4);
            if (lbl_1_bss_0.rotationSoundHandle != -1) {
                HuAudFXStop(lbl_1_bss_0.rotationSoundHandle);
                lbl_1_bss_0.rotationSoundHandle = -1;
            }
        }
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &oppositeNozzlePosition);
        oppositeNozzleDirection = oppositeNozzlePosition;
        PSVECNormalize(&oppositeNozzleDirection, &oppositeNozzleDirection);
        oppositeNozzleDistance = PSVECMag(&oppositeNozzlePosition);
        oppositeNozzlePosition.x += oppositeNozzleDirection.x * -(2.0f * oppositeNozzleDistance);
        oppositeNozzlePosition.z += oppositeNozzleDirection.z * -(2.0f * oppositeNozzleDistance);
        Hu3DModelPosSetV(lbl_1_bss_0.nozzleModelIds[1], &oppositeNozzlePosition);
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &segmentNozzlePosition);
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &segmentNozzleDirection);
        PSVECNormalize(&segmentNozzleDirection, &segmentNozzleDirection);
        offsetNozzleDistance = PSVECMag(&segmentNozzlePosition);
        secondNozzlePosition.x = segmentNozzlePosition.x + (segmentNozzleDirection.x * -(2.0f * offsetNozzleDistance));
        secondNozzlePosition.y = segmentNozzlePosition.y = 0.0f;
        secondNozzlePosition.z = segmentNozzlePosition.z + (segmentNozzleDirection.z * -(2.0f * offsetNozzleDistance));
        playerIndex = 0;
        while (playerIndex < 4) {
            playerPosition = lbl_1_bss_0.players[playerIndex]->actor->pos;
            collisionResult = 0;
            playerPosition.y = 0.0f;
            if ((lbl_1_bss_0.outsideGroupZero[playerIndex] != 0) && (lbl_1_bss_0.playerRemoved[playerIndex] == 0)) {
                PSVECSubtract(&segmentNozzlePosition, &playerPosition, &playerToNozzleDelta);
                segmentDistance = PSVECMag(&playerToNozzleDelta);
                if (segmentDistance < lbl_1_bss_0.nozzleCollisionRadius) {
                    collisionResult += 1;
                }
                PSVECSubtract(&secondNozzlePosition, &playerPosition, &playerToNozzleDelta);
                segmentDistance = PSVECMag(&playerToNozzleDelta);
                if (segmentDistance < lbl_1_bss_0.nozzleCollisionRadius) {
                    collisionResult += 1;
                }
                if (collisionResult != 0) {
                    OSReport(lbl_1_data_177, segmentDistance);
                    MgPlayerAttrSet(lbl_1_bss_0.players[playerIndex], 1U);
                    MgPlayerDespawn(lbl_1_bss_0.players[playerIndex]);
                    lbl_1_bss_0.playerRemoved[playerIndex] = 1;
                    lbl_1_bss_0.activePlayerCount -= 1;
                    lbl_1_bss_0.playerObjects[playerIndex]->objFunc = fn_1_34B0;
                    eliminatedPlayerPosition = lbl_1_bss_0.players[playerIndex]->actor->pos;
                    Hu3D3Dto2D(&playerPosition, 1, &projectedPlayerPosition);
                    soundPanning = (s32) projectedPlayerPosition.x;
                    soundPanning /= 5;
                    if ((s32) soundPanning < 48) {
                        soundPanning = 48;
                    } else if ((s32) soundPanning > 127) {
                        soundPanning = 127;
                    }
                    sound = HuAudFXPlay(M633_PLAYER_ELIMINATION_SE_ID);
                    HuAudFXPanning(sound, (s16) soundPanning);
                }
            }
            playerIndex += 1;
        }
    }
}

/* Per-frame callback for the other player slots; after the segment phase, restores their idle motion. */
void fn_1_32C0(OMOBJ *obj)
{
    Point3d playerPosition;
    MGPLAYER *player;
    s16 model;
    u32 playerNo;

    playerNo = obj->work[0];
    player = lbl_1_bss_0.players[playerNo];
    model = player->actor->mdlId;
    switch (obj->work[1]) {
    case 0:
        playerPosition = player->actor->pos;
        Hu3DModelPosSetV(model, &playerPosition);
        obj->work[1] = 1;
        obj->work[2] = 0;
        break;
    case 1:
        if (lbl_1_bss_0.segmentCount != 0) {
            obj->work[2] += 1;
            if ((u32) obj->work[2] >= 30U) {
                obj->work[1] = 3;
                obj->work[2] = 0;
                CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo, lbl_1_bss_0.players[playerNo]->omObj->mtnId[14], 0.0f, 6.0f, 0U);
            }
        }
    case 2:
        break;
    case 3:
        if ((Hu3DMotionShiftIDGet(model) == -1) && (Hu3DMotionEndCheck(model) != 0)) {
            CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo, lbl_1_bss_0.players[playerNo]->omObj->mtnId[0], 0.0f, 6.0f, 0U);
            obj->work[1] = 4;
        }
        break;
    }
}

void fn_1_34A8(void)
{

}

void fn_1_34AC(OMOBJ *obj)
{

}

/* Starts the hit reaction after a player is removed, then hands animation updates to fn_1_36BC. */
void fn_1_34B0(OMOBJ *obj)
{
    Point3d knockbackDirection;
    Point3d projectedPosition;
    Point3d playerPosition;
    s32 effect;
    s32 panning;
    u32 playerNo;

    playerNo = obj->work[0];
    omVibrate((s16) playerNo, 60, 20, 0);
    obj->objFunc = fn_1_36BC;
    knockbackDirection = lbl_1_bss_0.players[playerNo]->actor->pos;
    PSVECNormalize(&knockbackDirection, &knockbackDirection);
    lbl_1_bss_0.hitDirections[playerNo] = knockbackDirection;
    lbl_1_bss_0.hitUpdateCounts[playerNo] = 0;
    lbl_1_bss_0.hitVerticalOffsets[playerNo] = 40.0f;
    CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo,
                       lbl_1_bss_0.players[playerNo]->omObj->mtnId[11],
                       0.0f, 5.0f, 0U);
    playerPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
    effect = CharFXPlay((s16) lbl_1_bss_0.characterNumbers[playerNo], 576);
    Hu3D3Dto2D(&playerPosition, 1, &projectedPosition);
    panning = (s32) projectedPosition.x;
    panning /= 5;
    if (panning < 48) {
        panning = 48;
    } else if (panning > 127) {
        panning = 127;
    }
    HuAudFXPanning(effect, (s16) panning);
}
