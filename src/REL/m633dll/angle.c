/* Creates and advances the moving line hazards that cross the arena. */
#include "REL/m633dll.h"

/* Adds a moving arena segment at the given start position and direction. */
int fn_1_5ED0(Point3d *pos, Point3d *dir)
{
    OMOBJ *obj;
    M633Segment *segment;
    float angle;

    dir->y = 0.0f;
    angle = (180.0 * (atan2(dir->x, dir->z) / 3.141592653589793)) - 90.0;
    obj = omAddObjEx(lbl_1_bss_0.workProcess, 10000, 1, 0, 0, fn_1_60F4);
    obj->grpNo = -1;
    obj->memberNo = 0;
    segment = &lbl_1_bss_0.segments[lbl_1_bss_0.segmentCount];
    segment->startPosition = *pos;
    segment->endPosition = *pos;
    segment->direction = *dir;
    obj->work[0] = (u32)segment;
    obj->mdlId[0] = Hu3DModelLink(lbl_1_bss_0.segmentBaseModelId);
    Hu3DModelLayerSet(obj->mdlId[0], 6);
    omSetRot(obj, 0.0f, angle, 0.0f);
    omSetTra(obj, pos->x, pos->y, pos->z);
    omSetSca(obj, 1.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.0f, 0.0f, 0.0f);
    segment->reflected = 0;
    lbl_1_bss_0.segmentCount += 1;
}

/* Advances one segment, checks wall reflections and player hits, then removes it when its animation ends. */
void fn_1_60F4(OMOBJ *obj)
{
    MGACTOR_COLMAP_POLY wallCollision;
    Point3d segmentMotionVector;
    Point3d proposedSegmentEnd;
    Point3d reflectedPos;
    Point3d wallNormal;
    Point3d modelPos;
    Point3d playerPosition;
    Point3d segmentStart;
    Point3d segmentEnd;
    Point3d segmentDirection;
    Point3d playerOffsetFromStart;
    Point3d closestPointToPlayer;
    Point3d hitPlayerPosition;
    Point3d laterPlayerPosition;
    Point3d laterSegmentStart;
    Point3d laterSegmentEnd;
    Point3d laterSegmentDirection;
    Point3d laterPlayerOffsetFromStart;
    Point3d laterClosestPointToPlayer;
    Point3d soundPosition;
    f32 segmentLength;
    f32 segmentScale;
    f32 segmentDistance;
    f32 segmentRetractionScale;
    f32 segmentAppearanceScale;
    s16 endpointModelId;
    s32 playerNo;
    M633Segment *segment;

    segment = (M633Segment *) obj->work[0];
    switch (segment->reflected) {
    case 1:
        break;
    case 0:
        segmentMotionVector.x = 22.5f * segment->direction.x;
        segmentMotionVector.y = 22.5f * segment->direction.y;
        segmentMotionVector.z = 22.5f * segment->direction.z;
        proposedSegmentEnd.x = segment->endPosition.x + segmentMotionVector.x;
        proposedSegmentEnd.y = segment->endPosition.y + segmentMotionVector.y;
        proposedSegmentEnd.z = segment->endPosition.z + segmentMotionVector.z;
        if ((MgActorColMapPolyGet(&segment->endPosition, &proposedSegmentEnd, 1U, &wallCollision) == 1) && (wallNormal = *(Point3d *)((HSF_FACE *)wallCollision.obj->mesh.face->data)[wallCollision.triNo].nbt, (((u32) (wallCollision.code & 128) == 0) != 0)) && (PSVECDotProduct(&segment->direction, &wallNormal) <= 0.0f)) {
            C_VECReflect(&segment->direction, &wallNormal, &segmentMotionVector);
            reflectedPos.x = wallCollision.pos.x + segmentMotionVector.x;
            reflectedPos.y = wallCollision.pos.y + segmentMotionVector.y;
            reflectedPos.z = wallCollision.pos.z + segmentMotionVector.z;
            fn_1_5ED0(&reflectedPos, &segmentMotionVector);
            segment->reflected = 1;
            segment->endPosition = wallCollision.pos;
            Hu3DModelAttrSet(lbl_1_bss_0.segmentEndpointModelIds[segment->endpointModelIndex], HU3D_ATTR_DISPOFF);
            lbl_1_bss_0.segmentReflectionSoundPending = 1;
        } else {
            segment->endPosition = proposedSegmentEnd;
        }
        break;
    }
    if (segment->reflected == 0) {
        segmentAppearanceScale = 1.0f;
        if (lbl_1_bss_0.segmentUpdateCountdown <= 0) {
            segmentAppearanceScale = (f32) (lbl_1_bss_0.segmentUpdateCountdown + 6) / 6.0f;
            segmentAppearanceScale += 0.2f;
            if (segmentAppearanceScale > 1.0f) {
                segmentAppearanceScale = 1.0f;
            }
        }
        modelPos = segment->endPosition;
        endpointModelId = lbl_1_bss_0.segmentEndpointModelIds[segment->endpointModelIndex];
        Hu3DModelPosSet(endpointModelId, modelPos.x, (70.0f + modelPos.y) - (70.0f * (1.0f - segmentAppearanceScale)), modelPos.z);
        Hu3DModelScaleSet(endpointModelId, segmentAppearanceScale, segmentAppearanceScale, segmentAppearanceScale);
        Hu3DModelAttrReset(endpointModelId, HU3D_ATTR_DISPOFF);
    }
    if (lbl_1_bss_0.segmentUpdateCountdown <= 0) {
        if (lbl_1_bss_0.segmentUpdateCountdown < -6) {
            lbl_1_bss_0.segmentCount -= 1;
            if ((lbl_1_bss_0.segmentCount == 0) && (lbl_1_bss_0.preventArenaRestart == 0)) {
                fn_1_70A8(0);
            }
            Hu3DModelAttrSet(lbl_1_bss_0.segmentEndpointModelIds[segment->endpointModelIndex], HU3D_ATTR_DISPOFF);
            Hu3DModelKill(*obj->mdlId);
            *obj->mdlId = -1;
            omDelObjEx(lbl_1_bss_0.workProcess, obj);
            return;
        }
        segmentRetractionScale = (f32) (lbl_1_bss_0.segmentUpdateCountdown + 6) / 6.0f;
        PSVECSubtract(&segment->endPosition, &segment->startPosition, &segmentMotionVector);
        segmentLength = PSVECMag(&segmentMotionVector);
        omSetSca(obj, segmentLength / 100.0f, segmentRetractionScale, segmentRetractionScale);
        Hu3DModelScaleSet(*obj->mdlId, segmentLength / 100.0f, segmentRetractionScale, segmentRetractionScale);
        playerNo = 0;
        while (playerNo < 4) {
            playerPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
            segmentStart = segment->startPosition;
            segmentEnd = segment->endPosition;
            if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] != 0) {
                f32 playerDistance;
                f32 hitRadius;

                hitRadius = 90.0f * segmentRetractionScale;
                playerPosition.y = segmentStart.y = segmentEnd.y = 0.0f;
                PSVECSubtract(&segmentEnd, &segmentStart, &segmentDirection);
                PSVECSubtract(&playerPosition, &segmentStart, &playerOffsetFromStart);
                segmentScale = PSVECDotProduct(&segmentDirection, &playerOffsetFromStart);
                playerDistance = PSVECMag(&segmentDirection);
                if ((segmentScale >= 0.0f) && (segmentScale <= (playerDistance * playerDistance))) {
                    PSVECNormalize(&segmentDirection, &segmentDirection);
                    segmentScale = PSVECDotProduct(&playerOffsetFromStart, &segmentDirection);
                    closestPointToPlayer.x = segmentStart.x + (segmentScale * segmentDirection.x);
                    closestPointToPlayer.y = segmentStart.y + (segmentScale * segmentDirection.y);
                    closestPointToPlayer.z = segmentStart.z + (segmentScale * segmentDirection.z);
                    PSVECSubtract(&closestPointToPlayer, &playerPosition, &closestPointToPlayer);
                    playerDistance = PSVECMag(&closestPointToPlayer);
                    if (playerDistance < hitRadius) {
                        if ((s32) lbl_1_bss_0.playerRemoved[playerNo] == 0) {
                            OSReport(lbl_1_data_1A0, playerDistance);
                            MgPlayerAttrSet(lbl_1_bss_0.players[playerNo], 1U);
                            MgPlayerDespawn(lbl_1_bss_0.players[playerNo]);
                            lbl_1_bss_0.playerRemoved[playerNo] = 1;
                            lbl_1_bss_0.activePlayerCount -= 1;
                            (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_34B0;
                            hitPlayerPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
                            fn_1_24EC(M633_PLAYER_ELIMINATION_SE_ID, &hitPlayerPosition);
                        }
                    }
                }
            }
            playerNo += 1;
        }
        return;
    }
    if (lbl_1_bss_0.segmentUpdateCountdown > 1) {
        f32 unitScale;

        unitScale = 1.0f;
        PSVECSubtract(&segment->endPosition, &segment->startPosition, &segmentMotionVector);
        segmentLength = PSVECMag(&segmentMotionVector);
        omSetSca(obj, segmentLength / 100.0f, unitScale, unitScale);
        Hu3DModelScaleSet(*obj->mdlId, segmentLength / 100.0f, unitScale, unitScale);
        playerNo = 0;
        while (playerNo < 4) {
            laterPlayerPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
            laterSegmentStart = segment->startPosition;
            laterSegmentEnd = segment->endPosition;
            if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] != 0) {
                f32 collisionRadius;
                f32 hitRadius;

                hitRadius = 90.0f * unitScale;
                laterPlayerPosition.y = laterSegmentStart.y = laterSegmentEnd.y = 0.0f;
                PSVECSubtract(&laterSegmentEnd, &laterSegmentStart, &laterSegmentDirection);
                PSVECSubtract(&laterPlayerPosition, &laterSegmentStart, &laterPlayerOffsetFromStart);
                segmentDistance = PSVECDotProduct(&laterSegmentDirection, &laterPlayerOffsetFromStart);
                collisionRadius = PSVECMag(&laterSegmentDirection);
                if ((segmentDistance >= 0.0f) && (segmentDistance <= (collisionRadius * collisionRadius))) {
                    PSVECNormalize(&laterSegmentDirection, &laterSegmentDirection);
                    segmentDistance = PSVECDotProduct(&laterPlayerOffsetFromStart, &laterSegmentDirection);
                    laterClosestPointToPlayer.x = laterSegmentStart.x + (segmentDistance * laterSegmentDirection.x);
                    laterClosestPointToPlayer.y = laterSegmentStart.y + (segmentDistance * laterSegmentDirection.y);
                    laterClosestPointToPlayer.z = laterSegmentStart.z + (segmentDistance * laterSegmentDirection.z);
                    PSVECSubtract(&laterClosestPointToPlayer, &laterPlayerPosition, &laterClosestPointToPlayer);
                    collisionRadius = PSVECMag(&laterClosestPointToPlayer);
                    if ((collisionRadius < hitRadius) && ((s32) lbl_1_bss_0.playerRemoved[playerNo] == 0)) {
                        OSReport(lbl_1_data_1A0, collisionRadius);
                        MgPlayerAttrSet(lbl_1_bss_0.players[playerNo], 1U);
                        MgPlayerDespawn(lbl_1_bss_0.players[playerNo]);
                        CharMotionShiftSet((lbl_1_bss_0.players[playerNo])->charNo, (((lbl_1_bss_0.players[playerNo])->omObj)->mtnId)[11], 0.0f, 5.0f, 0U);
                        lbl_1_bss_0.playerRemoved[playerNo] = 1;
                        lbl_1_bss_0.activePlayerCount -= 1;
                        (lbl_1_bss_0.playerObjects[playerNo])->objFunc = fn_1_34B0;
                        soundPosition = lbl_1_bss_0.players[playerNo]->actor->pos;
                        fn_1_24EC(M633_PLAYER_ELIMINATION_SE_ID, &soundPosition);
                    }
                }
            }
            playerNo += 1;
        }
    }
}

/* Updates the arena motion phase and progress during sequence callbacks. */
void fn_1_6DE8(void)
{
    s32 i, finished;
    switch (lbl_1_bss_0.arenaPhase) {
    case 3:
        lbl_1_bss_0.nozzleCollisionRadius = 0.0f;
        break;
    case 2:
        lbl_1_bss_0.nozzleCollisionRadius = 90.0f;
        break;
    case 0:
        finished = 0;
        for (i = 0; i < 2; i++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_0.nozzleModelIds[i]) == 1) {
                fn_1_70A8(2);
                finished = 1;
            } else if (i == 0) {
                float time, maxTime;
                maxTime = Hu3DMotionMotionMaxTimeGet(lbl_1_bss_0.nozzleModelIds[i]);
                time = Hu3DMotionTimeGet(lbl_1_bss_0.nozzleModelIds[i]);
                lbl_1_bss_0.nozzleCollisionRadius = (90.0f * time) / maxTime;
            }
        }
        if (finished && lbl_1_bss_0.preventArenaRestart == 0) {
            for (i = 0; i < 4; i++) {
                if (lbl_1_bss_0.outsideGroupZero[i] == 0) {
                    omVibrate((s16)i, 20, 7, 3);
                    break;
                }
            }
        }
        break;
    case 1:
        for (i = 0; i < 2; i++) {
            if (Hu3DMotionEndCheck(lbl_1_bss_0.nozzleModelIds[i]) == 1) {
                fn_1_70A8(3);
            } else if (i == 0) {
                float time, maxTime;
                maxTime = Hu3DMotionMotionMaxTimeGet(lbl_1_bss_0.nozzleModelIds[i]);
                time = Hu3DMotionTimeGet(lbl_1_bss_0.nozzleModelIds[i]);
                lbl_1_bss_0.nozzleCollisionRadius = (90.0f * time) / maxTime;
            }
        }
        break;
    }
    if (lbl_1_bss_0.nozzleCollisionRadius > 90.0f) {
        lbl_1_bss_0.nozzleCollisionRadius = 90.0f;
    }
}

/* Switches the arena phase motion and visibility set after timer or segment events. */
void fn_1_70A8(s32 phase)
{
    s16 visibleModelIndex;
    s32 modelIndex;
    s32 loopsMotion;
    s16 leftMotion;
    s16 rightMotion;
    s16 stageMotion;

    switch (phase) {
    case 3:
        loopsMotion = 1;
        leftMotion = lbl_1_bss_0.firstNozzleMotionIds[phase];
        rightMotion = lbl_1_bss_0.secondNozzleMotionIds[phase];
        stageMotion = *lbl_1_bss_0.object->mtnId;
        visibleModelIndex = 0;
        break;
    case 2:
        loopsMotion = 1;
        leftMotion = lbl_1_bss_0.firstNozzleMotionIds[phase];
        rightMotion = lbl_1_bss_0.secondNozzleMotionIds[phase];
        stageMotion = lbl_1_bss_0.object->mtnId[2];
        visibleModelIndex = 2;
        break;
    case 0:
        loopsMotion = 0;
        leftMotion = lbl_1_bss_0.firstNozzleMotionIds[phase];
        rightMotion = lbl_1_bss_0.secondNozzleMotionIds[phase];
        stageMotion = lbl_1_bss_0.object->mtnId[1];
        visibleModelIndex = 1;
        break;
    case 1:
        loopsMotion = 0;
        leftMotion = lbl_1_bss_0.firstNozzleMotionIds[phase];
        rightMotion = lbl_1_bss_0.secondNozzleMotionIds[phase];
        stageMotion = lbl_1_bss_0.object->mtnId[3];
        visibleModelIndex = 3;
        break;
    }
    Hu3DMotionSet(lbl_1_bss_0.nozzleModelIds[0], leftMotion);
    Hu3DMotionSet(lbl_1_bss_0.nozzleModelIds[1], rightMotion);
    Hu3DMotionSet(*lbl_1_bss_0.object->mdlId, stageMotion);
    modelIndex = 0;
    while (modelIndex < 4) {
        Hu3DModelAttrSet(lbl_1_bss_0.arenaPhaseModelIds[modelIndex], HU3D_ATTR_DISPOFF);
        modelIndex += 1;
    }
    Hu3DModelAttrReset(lbl_1_bss_0.arenaPhaseModelIds[visibleModelIndex], HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(lbl_1_bss_0.arenaPhaseModelIds[visibleModelIndex], 0.0f);
    if (loopsMotion != 0) {
        Hu3DModelAttrSet(lbl_1_bss_0.nozzleModelIds[0], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_0.nozzleModelIds[1], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(*lbl_1_bss_0.object->mdlId, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_0.arenaPhaseModelIds[visibleModelIndex], HU3D_MOTATTR_LOOP);
    } else {
        Hu3DModelAttrReset(lbl_1_bss_0.nozzleModelIds[0], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(lbl_1_bss_0.nozzleModelIds[1], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(*lbl_1_bss_0.object->mdlId, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrReset(lbl_1_bss_0.arenaPhaseModelIds[visibleModelIndex], HU3D_MOTATTR_LOOP);
    }
    lbl_1_bss_0.arenaPhase = phase;
}
