/* Player object callbacks for controlling the rotating obstacle and handling hits. */
#include "REL/m633dll.h"

#define M633_PLAYER_HIT_REACTION_CHAR_SE_ID 576 /* Character sound effect used when a player is
                                                 * knocked out. */

/* Builds two moving hazards from opposite nozzle positions, sets their shared countdown, and
 * plays the launch effect. */
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

/* Installed on group-zero player objects while their sequence callback has no active work. */
void fn_1_26B4(OMOBJ *groupZeroPlayerObject)
{

}

/* When the arena reaches phase 2, launches two opposite segments and waits until no live segments
 * remain. */
void fn_1_26B8(OMOBJ *playerObject)
{
    Point3d nozzleDirection;
    Point3d nozzlePosition;
    MGPLAYER *player;
    s16 playerModelId;
    u32 playerNo;
    f32 nozzleDistance;

    playerNo = playerObject->work[0];
    player = lbl_1_bss_0.players[playerNo];
    /* The actor model handle is read here but not otherwise used by this callback. */
    playerModelId = player->actor->mdlId;
    switch (playerObject->work[1]) {
    case 0:
        fn_1_70A8(0);
        playerObject->work[1] = 1;
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
            playerObject->work[1] = 2;
            fn_1_70A8(1);
        }
        break;
    case 2:
        if (lbl_1_bss_0.segmentCount == 0) {
            playerObject->work[1] = 4;
        }
        break;
    }
}

/* When no segments are live, group-zero players can rotate the arena (L adds 2 degrees; R adds 358,
 * equivalent to -2), launch two segments with a new A press in phase 2, and check surviving
 * outside-group players against both nozzle points. Pressing both rotation buttons cancels
 * rotation. */
void fn_1_28A4(OMOBJ *playerObject)
{
    Point3d oppositeNozzlePosition;
    Point3d oppositeNozzleDirection;
    Point3d firstNozzlePosition;
    Point3d secondNozzlePosition;
    Point3d firstNozzleDirection;
    Point3d playerPosition;
    Point3d playerToNozzleDelta;
    Point3d eliminatedPlayerPosition;
    Point3d nozzleDirection;
    Point3d nozzlePosition;
    Point3d projectedPlayerPosition;
    f32 playerToNozzleDistance;
    f32 oppositeNozzleDistance;
    f32 offsetNozzleDistance;
    f32 nozzleDistance;
    s32 checkedPlayerNo;
    u32 playerNo;
    u16 buttonMask;
    MGPLAYER *player;
    s32 eliminationSoundPan;
    s32 collisionResult;
    s32 rotationDirectionChanged;
    u16 heldButtons;
    u16 newlyPressedButtons;
    u16 rotationButtonMask;
    s16 modelId;
    s32 soundHandle;

    playerNo = playerObject->work[0];
    if (GwPlayerConf[playerNo].type != 0) {
        heldButtons = lbl_1_bss_0.aiButtons;
        newlyPressedButtons = lbl_1_bss_0.aiPressedButtons;
    } else {
        heldButtons = HuPadBtn[lbl_1_bss_0.padNumbers[playerNo]];
        newlyPressedButtons = HuPadBtnDown[lbl_1_bss_0.padNumbers[playerNo]];
    }
    if (lbl_1_bss_0.segmentCount == 0) {
        rotationButtonMask = PAD_BUTTON_TRIGGER_L | PAD_BUTTON_TRIGGER_R;
        buttonMask = heldButtons & rotationButtonMask;
        rotationDirectionChanged = 0;
        /* Holding both rotation buttons cancels the turn. */
        if (buttonMask == rotationButtonMask) {
            buttonMask = 0;
        }
        if (lbl_1_bss_0.previousRotationButtonMask != buttonMask) {
            rotationDirectionChanged = 1;
        }
        lbl_1_bss_0.previousRotationButtonMask = buttonMask;
        if (buttonMask == PAD_BUTTON_TRIGGER_R) {
            /* Adding 358 degrees turns the arena backward by 2 degrees after wrapping. */
            lbl_1_bss_0.arenaYawDegrees += 358.0f;
            if (rotationDirectionChanged != 0) {
                CharMotionShiftSet((s16) lbl_1_bss_0.characterNumbers[playerNo],
                                   lbl_1_bss_0.players[playerNo]->omObj->mtnId[12], 6.0f, 6.0f,
                                   HU3D_MOTATTR_REV);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId,
                                   lbl_1_bss_0.stageAuxiliaryIds[0], 6.0f, 6.0f, HU3D_MOTATTR_REV);
            }
            if (lbl_1_bss_0.rotationSoundHandle == -1) {
                lbl_1_bss_0.rotationSoundHandle = HuAudFXPlay(M633_ROTATION_CONTROL_SE_ID);
            }
        } else if (buttonMask == PAD_BUTTON_TRIGGER_L) {
            lbl_1_bss_0.arenaYawDegrees += 2.0f;
            if (rotationDirectionChanged != 0) {
                Hu3DModelAttrReset((s16) lbl_1_bss_0.players[playerNo]->actor->mdlId,
                                   HU3D_MOTATTR_REV);
                CharMotionShiftSet((s16) lbl_1_bss_0.characterNumbers[playerNo],
                                   lbl_1_bss_0.players[playerNo]->omObj->mtnId[12], 6.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId,
                                   lbl_1_bss_0.stageAuxiliaryIds[0], 6.0f, 6.0f, 0U);
            }
            if (lbl_1_bss_0.rotationSoundHandle == -1) {
                lbl_1_bss_0.rotationSoundHandle = HuAudFXPlay(M633_ROTATION_CONTROL_SE_ID);
            }
        } else {
            player = lbl_1_bss_0.players[playerNo];
            modelId = player->actor->mdlId;
            if ((Hu3DMotionShiftIDGet(modelId) < 0) &&
                (player->omObj->mtnId[13] != Hu3DMotionIDGet(modelId))) {
                CharMotionShiftSet(player->charNo, player->omObj->mtnId[13], 0.0f, 6.0f, 0U);
                Hu3DMotionShiftSet(lbl_1_bss_0.rotatingStageModelId, lbl_1_bss_0.stageIdleMotionId,
                                   0.0f, 6.0f, 0U);
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
        /* Rotation reads held buttons; the launch action triggers only on a new A press. */
        buttonMask = newlyPressedButtons;
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
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160,
                           &oppositeNozzlePosition);
        oppositeNozzleDirection = oppositeNozzlePosition;
        PSVECNormalize(&oppositeNozzleDirection, &oppositeNozzleDirection);
        oppositeNozzleDistance = PSVECMag(&oppositeNozzlePosition);
        oppositeNozzlePosition.x += oppositeNozzleDirection.x * -(2.0f * oppositeNozzleDistance);
        oppositeNozzlePosition.z += oppositeNozzleDirection.z * -(2.0f * oppositeNozzleDistance);
        Hu3DModelPosSetV(lbl_1_bss_0.nozzleModelIds[1], &oppositeNozzlePosition);
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &firstNozzlePosition);
        Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_160, &firstNozzleDirection);
        PSVECNormalize(&firstNozzleDirection, &firstNozzleDirection);
        offsetNozzleDistance = PSVECMag(&firstNozzlePosition);
        secondNozzlePosition.x =
            firstNozzlePosition.x + (firstNozzleDirection.x * -(2.0f * offsetNozzleDistance));
        secondNozzlePosition.y = firstNozzlePosition.y = 0.0f;
        secondNozzlePosition.z =
            firstNozzlePosition.z + (firstNozzleDirection.z * -(2.0f * offsetNozzleDistance));
        checkedPlayerNo = 0;
        while (checkedPlayerNo < 4) {
            playerPosition = lbl_1_bss_0.players[checkedPlayerNo]->actor->pos;
            collisionResult = 0;
            playerPosition.y = 0.0f;
            if ((lbl_1_bss_0.outsideGroupZero[checkedPlayerNo] != 0) &&
                (lbl_1_bss_0.playerRemoved[checkedPlayerNo] == 0)) {
                PSVECSubtract(&firstNozzlePosition, &playerPosition, &playerToNozzleDelta);
                playerToNozzleDistance = PSVECMag(&playerToNozzleDelta);
                if (playerToNozzleDistance < lbl_1_bss_0.nozzleCollisionRadius) {
                    collisionResult += 1;
                }
                PSVECSubtract(&secondNozzlePosition, &playerPosition, &playerToNozzleDelta);
                playerToNozzleDistance = PSVECMag(&playerToNozzleDelta);
                if (playerToNozzleDistance < lbl_1_bss_0.nozzleCollisionRadius) {
                    collisionResult += 1;
                }
                if (collisionResult != 0) {
                    /* The log reports the second nozzle distance even if only the first point
                     * caused the hit. */
                    OSReport(lbl_1_data_177, playerToNozzleDistance);
                    MgPlayerAttrSet(lbl_1_bss_0.players[checkedPlayerNo], MGPLAYER_ATTR_COMSTK);
                    MgPlayerDespawn(lbl_1_bss_0.players[checkedPlayerNo]);
                    lbl_1_bss_0.playerRemoved[checkedPlayerNo] = 1;
                    lbl_1_bss_0.activePlayerCount -= 1;
                    lbl_1_bss_0.playerObjects[checkedPlayerNo]->objFunc = fn_1_34B0;
                    eliminatedPlayerPosition = lbl_1_bss_0.players[checkedPlayerNo]->actor->pos;
                    /* The refreshed actor position is stored in eliminatedPlayerPosition but never
                     * read; panning below uses playerPosition captured before despawn. */
                    Hu3D3Dto2D(&playerPosition, 1, &projectedPlayerPosition);
                    eliminationSoundPan = (s32) projectedPlayerPosition.x;
                    eliminationSoundPan /= 5;
                    if (eliminationSoundPan < 48) {
                        eliminationSoundPan = 48;
                    } else if (eliminationSoundPan > 127) {
                        eliminationSoundPan = 127;
                    }
                    soundHandle = HuAudFXPlay(M633_PLAYER_ELIMINATION_SE_ID);
                    HuAudFXPanning(soundHandle, (s16) eliminationSoundPan);
                }
            }
            checkedPlayerNo += 1;
        }
    }
}

/* After 30 updates with a live segment, starts motion 14 for this outside-group player, then
 * returns to motion 0 when it ends. */
void fn_1_32C0(OMOBJ *playerObject)
{
    Point3d playerPosition;
    MGPLAYER *player;
    s16 model;
    u32 playerNo;

    playerNo = playerObject->work[0];
    player = lbl_1_bss_0.players[playerNo];
    model = player->actor->mdlId;
    switch (playerObject->work[1]) {
    case 0:
        playerPosition = player->actor->pos;
        Hu3DModelPosSetV(model, &playerPosition);
        playerObject->work[1] = 1;
        playerObject->work[2] = 0;
        break;
    case 1:
        if (lbl_1_bss_0.segmentCount != 0) {
            playerObject->work[2] += 1;
            if ((u32) playerObject->work[2] >= 30U) {
                playerObject->work[1] = 3;
                playerObject->work[2] = 0;
                CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo,
                                   lbl_1_bss_0.players[playerNo]->omObj->mtnId[14], 0.0f, 6.0f, 0U);
            }
        }
    case 2:
        break;
    case 3:
        if ((Hu3DMotionShiftIDGet(model) == -1) && (Hu3DMotionEndCheck(model) != 0)) {
            CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo,
                               lbl_1_bss_0.players[playerNo]->omObj->mtnId[0], 0.0f, 6.0f, 0U);
            playerObject->work[1] = 4;
        }
        break;
    }
}

void fn_1_34A8(void)
{

}

/* Empty callback installed at setup and again for outside-group players when the round starts;
 * a hazard hit replaces it with fn_1_34B0. */
void fn_1_34AC(OMOBJ *outsideGroupPlayerObject)
{

}

/* Starts the hit reaction after a player is removed, then hands animation updates to fn_1_36BC. */
void fn_1_34B0(OMOBJ *playerObject)
{
    Point3d knockbackDirection;
    Point3d projectedPosition;
    Point3d playerPosition;
    s32 soundHandle;
    s32 screenPan;
    u32 playerNo;

    playerNo = playerObject->work[0];
    omVibrate((s16) playerNo, 60, 20, 0);
    playerObject->objFunc = fn_1_36BC;
    knockbackDirection = lbl_1_bss_0.players[playerNo]->actor->pos;
    PSVECNormalize(&knockbackDirection, &knockbackDirection);
    lbl_1_bss_0.hitDirections[playerNo] = knockbackDirection;
    lbl_1_bss_0.hitUpdateCounts[playerNo] = 0;
    lbl_1_bss_0.hitVerticalOffsets[playerNo] = 40.0f;
    CharMotionShiftSet(lbl_1_bss_0.players[playerNo]->charNo,
                       lbl_1_bss_0.players[playerNo]->omObj->mtnId[11],
                       0.0f, 5.0f, 0U);
    playerPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
    soundHandle = CharFXPlay((s16) lbl_1_bss_0.characterNumbers[playerNo],
                             M633_PLAYER_HIT_REACTION_CHAR_SE_ID);
    Hu3D3Dto2D(&playerPosition, 1, &projectedPosition);
    screenPan = (s32) projectedPosition.x;
    screenPan /= 5;
    if (screenPan < 48) {
        screenPan = 48;
    } else if (screenPan > 127) {
        screenPan = 127;
    }
    HuAudFXPanning(soundHandle, (s16) screenPan);
}
