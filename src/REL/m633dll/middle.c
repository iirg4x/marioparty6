/* Computer-player decisions and the paths made by moving arena segments. */
#include "REL/m633dll.h"

void fn_1_3828(OMOBJ *obj)
{

}

void fn_1_382C(void)
{

}

/* fn_1_690 calls this once per player slot each round update; active computer players are sent to
 * the group-zero or outside-group decision routine. */
void fn_1_3830(s32 playerNo)
{
    if ((GwPlayerConf[playerNo].type != 0) && ((s32) lbl_1_bss_0.playerRemoved[playerNo] == 0)) {
        if ((s32) lbl_1_bss_0.outsideGroupZero[playerNo] == 0) {
            fn_1_38C0(playerNo);
            return;
        }
        fn_1_3FFC(playerNo);
    }
}

/* Called by fn_1_3830 for a live group-zero computer each round update; chooses a player or random
 * heading, turns toward it, and presses A in phase 2 when aligned or timed out. */
void fn_1_38C0(s32 playerNo)
{
    Point3d stageObjectPosition;
    Point3d targetPosition;
    Point3d stageRotation;
    Point3d movementRotation;
    M633AI *aiState;
    MGPLAYER *targetPlayer;
    f32 targetHeadingDegrees;
    f32 stageYawDegrees;
    f32 movementYawDegrees;
    f32 movementHeadingDegrees;
    f32 directionCalculation;
    f32 targetOffsetComponent;
    f32 turnDifference;
    f32 alignmentDifference;
    s32 targetPlayerNo;
    s32 useRandomHeading;

    aiState = &lbl_1_bss_0.aiStates[playerNo];
    lbl_1_bss_0.aiButtons = 0;
    lbl_1_bss_0.aiPressedButtons = 0;
    if (lbl_1_bss_0.activePlayerCount != 0) {
        if ((f32) aiState->decisionCounter < 0.0f) {
            if (frandmod(100) < (u32) lbl_1_data_260[GwPlayerConf[playerNo].comDif]) {
                useRandomHeading = 1;
            } else {
                useRandomHeading = 0;
            }
            aiState->decisionMode = useRandomHeading;
            switch (aiState->decisionMode) {
                do {
                case 0:
                /* Retry if the draw selects this player or a player already removed. */
retry_target_selection:
                    targetPlayerNo = frandmod(4);
                    if (targetPlayerNo == playerNo) {
                        goto retry_target_selection;
                    }
                } while ((s32) lbl_1_bss_0.playerRemoved[targetPlayerNo] != 0);
                aiState->targetPlayerNo = targetPlayerNo;
                aiState->decisionCounter = frandmod(60) + 60;
                break;
            case 1:
                targetOffsetComponent = 250.0f;
                directionCalculation = (f32) (u32) frandmod(360);
                aiState->targetHeadingDegrees = directionCalculation;
                aiState->destination.x = targetOffsetComponent * HuCos(directionCalculation) -
                                         targetOffsetComponent * HuSin(directionCalculation);
                aiState->destination.y = 0.0f;
                aiState->destination.z = targetOffsetComponent * HuSin(directionCalculation) +
                                         targetOffsetComponent * HuCos(directionCalculation);
                aiState->decisionCounter = frandmod(180) + 180;
                break;
            }
        }
        switch (aiState->decisionMode) {
        case 0:
            targetPlayerNo = aiState->targetPlayerNo;
            targetPlayer = lbl_1_bss_0.players[targetPlayerNo];
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_188,
                               &stageObjectPosition);
            targetPosition = targetPlayer->actor->pos;
            break;
        case 1:
            targetPosition = aiState->destination;
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_188,
                               &stageObjectPosition);
            break;
        }
        aiState->decisionCounter -= 1;
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &stageRotation);
        targetHeadingDegrees =
            (f32) (180.0 *
                   (atan2((f64) targetPosition.x, (f64) targetPosition.z) / 3.141592653589793));
        stageYawDegrees = stageRotation.y;
        while (targetHeadingDegrees < 0.0f) {
            targetHeadingDegrees += 360.0f;
        }
        while (targetHeadingDegrees > 360.0f) {
            targetHeadingDegrees -= 360.0f;
        }
        while (stageYawDegrees < 0.0f) {
            stageYawDegrees += 360.0f;
        }
        while (stageYawDegrees > 360.0f) {
            stageYawDegrees -= 360.0f;
        }
        turnDifference = targetHeadingDegrees - stageYawDegrees;
        if ((f32) abs((s32) turnDifference) >= 15.0f) {
            lbl_1_bss_0.aiTurning = 1;
        } else if (abs((s32) turnDifference) < 7) {
            lbl_1_bss_0.aiTurning = 0;
        }
        if (lbl_1_bss_0.aiTurning != 0) {
            /* Replace the world target with its offset from the nozzle; the later A-button
             * alignment test also uses this modified vector while turning. */
            PSVECSubtract(&targetPosition, &stageObjectPosition, &targetPosition);
            directionCalculation = (stageObjectPosition.x * targetPosition.z) -
                                   (stageObjectPosition.z * targetPosition.x);
            if (directionCalculation >= 0.0f) {
                lbl_1_bss_0.aiButtons = PAD_BUTTON_TRIGGER_R;
            } else {
                lbl_1_bss_0.aiButtons = PAD_BUTTON_TRIGGER_L;
            }
        }
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &movementRotation);
        movementYawDegrees = movementRotation.y;
        movementHeadingDegrees =
            (f32) (180.0 *
                   (atan2((f64) targetPosition.z, (f64) targetPosition.x) / 3.141592653589793));
        while (movementYawDegrees < 0.0f) {
            movementYawDegrees += 360.0f;
        }
        while (movementYawDegrees > 360.0f) {
            movementYawDegrees -= 360.0f;
        }
        while (movementHeadingDegrees < 0.0f) {
            movementHeadingDegrees += 360.0f;
        }
        while (movementHeadingDegrees > 360.0f) {
            movementHeadingDegrees -= 360.0f;
        }
        alignmentDifference = movementHeadingDegrees - movementYawDegrees;
        if ((lbl_1_bss_0.arenaPhase == 2) && (((f32) abs((s32) alignmentDifference) < 20.0f) ||
                                              ((f32) aiState->decisionCounter < 0.0f))) {
            lbl_1_bss_0.aiPressedButtons = PAD_BUTTON_A;
            aiState->decisionCounter = -1;
        }
    }
}

/* Called by fn_1_3830 for each live outside-group computer; chooses free movement or routes around
 * active segments. With no segments, it runs the free-movement helper both before resetting the
 * decision mode and again in the mode-0 dispatch. */
void fn_1_3FFC(s32 playerNo)
{
    Point3d playerPosition;
    MGPLAYER *player;
    M633AI *aiState;
    player = lbl_1_bss_0.players[playerNo];
    aiState = &lbl_1_bss_0.aiStates[playerNo];
    if (lbl_1_bss_0.segmentCount == 0) {
        fn_1_55D0(player, aiState);
        if (aiState->decisionMode == 2) {
            playerPosition = player->actor->pos;
            aiState->targetHeadingDegrees =
                180.0 * (atan2(playerPosition.z, playerPosition.x) / 3.141592653589793);
            aiState->destination = playerPosition;
            aiState->decisionCounter = 0;
        }
        aiState->decisionMode = 0;
    } else if (aiState->decisionMode == 0) {
        if (frandmod(100) < (u32)lbl_1_data_270[GwPlayerConf[playerNo].comDif]) {
            aiState->decisionMode = 2;
        } else {
            aiState->decisionMode = 1;
        }
    }
    if (aiState->decisionMode == 1 && lbl_1_bss_0.segmentReflectionSoundPending != 0
        && frandmod(100) < (u32)lbl_1_data_280[GwPlayerConf[playerNo].comDif]) {
        aiState->decisionMode = 2;
    }
    switch (aiState->decisionMode) {
    case 0:
    case 1:
        fn_1_55D0(player, aiState);
        break;
    case 2:
        switch (lbl_1_bss_0.segmentCount) {
        case 0: fn_1_55D0(player, aiState); break;
        case 1: fn_1_45EC(player, aiState, NULL); break;
        case 2: fn_1_4A00(player, aiState, NULL, NULL); break;
        default: fn_1_51D4(player, aiState); break;
        }
        break;
    }
}

/* Called by the segment-routing helpers to project a player or point onto a segment; rejects
 * x/y-equal endpoints and projections beyond either end. */
s32 fn_1_4260(Point3d *startPosition, Point3d *endPosition, Point3d *pointToProject,
              Point3d *nearestPoint, float *distanceToNearestPoint)
{
    Point3d segmentVector, pointOffsetFromStart, nearestPointDelta;
    float projectionScalar, segmentLength;

    if (startPosition->x == endPosition->x && startPosition->y == endPosition->y) {
        return 0;
    }
    PSVECSubtract(endPosition, startPosition, &segmentVector);
    PSVECSubtract(pointToProject, startPosition, &pointOffsetFromStart);
    projectionScalar = PSVECDotProduct(&segmentVector, &pointOffsetFromStart);
    segmentLength = PSVECMag(&segmentVector);
    if (projectionScalar >= 0.0f &&
        projectionScalar <= segmentLength * segmentLength) {
        PSVECNormalize(&segmentVector, &segmentVector);
        projectionScalar = PSVECDotProduct(&pointOffsetFromStart, &segmentVector);
        nearestPoint->x = startPosition->x + projectionScalar * segmentVector.x;
        nearestPoint->y = startPosition->y + projectionScalar * segmentVector.y;
        nearestPoint->z = startPosition->z + projectionScalar * segmentVector.z;
        if (distanceToNearestPoint) {
            PSVECSubtract(nearestPoint, pointToProject, &nearestPointDelta);
            *distanceToNearestPoint = PSVECMag(&nearestPointDelta);
        }
        return 1;
    }
    return 0;
}

/* Called by the AI route selectors; flattens the segment endpoints and player point to y=0 before
 * projecting onto the segment. */
s32 fn_1_43D4(M633Segment *segment, Point3d *pointToProject, Point3d *nearestPoint,
              float *distanceToNearestPoint)
{
    Point3d segmentStart, segmentEnd;
    s32 unusedZeroValue;
    unusedZeroValue = 0;
    segmentStart = segment->startPosition;
    segmentEnd = segment->endPosition;
    segmentStart.y = segmentEnd.y = pointToProject->y = 0.0f;
    return fn_1_4260(&segmentStart, &segmentEnd, pointToProject, nearestPoint,
                     distanceToNearestPoint);
}

/* Used by fn_1_4A00 to get the ground-plane midpoint between a segment's endpoints. */
void fn_1_4598(M633Segment *segment, Point3d *pos)
{
    pos->x = (segment->startPosition.x + segment->endPosition.x) / 2.0f;
    pos->y = 0.0f;
    pos->z = (segment->startPosition.z + segment->endPosition.z) / 2.0f;
}

/* Called by fn_1_3FFC, fn_1_4A00, and fn_1_51D4 to route a computer player around one segment; a
 * null pointer selects the first stored segment. */
void fn_1_45EC(MGPLAYER *player, M633AI *ai, M633Segment *segment)
{
    Point3d playerPosition, nearestPoint, movementDelta;
    float distance;
    Point3d segmentEnd, segmentStart;
    s32 unusedZeroValue;
    M633Segment *current;

    current = segment == NULL ? lbl_1_bss_0.segments : segment;
    playerPosition = player->actor->pos;
    if (current->startPosition.x == current->endPosition.x &&
        current->startPosition.z == current->endPosition.z) {
        fn_1_55D0(player, ai);
        return;
    }
    playerPosition.y = 0.0f;
    unusedZeroValue = 0;
    segmentStart = current->startPosition;
    segmentEnd = current->endPosition;
    segmentStart.y = segmentEnd.y = playerPosition.y = 0.0f;
    if (fn_1_4260(&segmentStart, &segmentEnd, &playerPosition, &nearestPoint, &distance)) {
        if (distance < 100.0f) {
            PSVECSubtract(&playerPosition, &nearestPoint, &movementDelta);
            PSVECNormalize(&movementDelta, &movementDelta);
            MgPlayerPadSet(player, (s32) (56.0f * movementDelta.x),
                           (s32) (-56.0f * movementDelta.z), 0, 0);
            return;
        }
        ai->targetHeadingDegrees =
            180.0 * (atan2(playerPosition.z, playerPosition.x) / 3.141592653589793);
        ai->destination = playerPosition;
        return;
    }
    PSVECSubtract(&playerPosition, &current->endPosition, &movementDelta);
    distance = PSVECMag(&movementDelta);
    if (distance < 300.0f) {
        PSVECNormalize(&movementDelta, &movementDelta);
        ai->decisionCounter = 0;
        ai->destination.x = playerPosition.x + 100.0f * movementDelta.x;
        ai->destination.y = 0.0f;
        ai->destination.z = playerPosition.z + 100.0f * movementDelta.z;
        fn_1_55D0(player, ai);
        return;
    }
    ai->targetHeadingDegrees =
        180.0 * (atan2(playerPosition.z, playerPosition.x) / 3.141592653589793);
    ai->destination = playerPosition;
    fn_1_55D0(player, ai);
}

/* Called by fn_1_3FFC or fn_1_51D4 when two segments are selected; chooses an avoidance or roaming
* route, or sets a destination midway between their projections. */
void fn_1_4A00(MGPLAYER *player, M633AI *ai, M633Segment *first, M633Segment *second)
{
    Point3d firstMidpoint, secondMidpoint;
    Point3d nearestPoint, firstProjection, secondProjection, movementDelta, playerPosition;
    float distance, firstDistance, secondDistance;
    M633Segment *firstSegment, *secondSegment;
    s32 firstHit, secondHit;

    firstSegment = first == NULL ? lbl_1_bss_0.segments : first;
    secondSegment = second == NULL ? &lbl_1_bss_0.segments[1] : second;
    playerPosition = player->actor->pos;
    playerPosition.y = 0.0f;
    firstHit = fn_1_43D4(firstSegment, &playerPosition, &firstProjection, &firstDistance);
    secondHit = fn_1_43D4(secondSegment, &playerPosition, &secondProjection, &secondDistance);
    /* The segment midpoints are calculated here, although the route below uses the projected
     * points. */
    fn_1_4598(firstSegment, &firstMidpoint);
    fn_1_4598(secondSegment, &secondMidpoint);
    if (firstHit == 0 && secondHit == 0) {
        PSVECSubtract(&playerPosition, &firstSegment->endPosition, &movementDelta);
        distance = PSVECMag(&movementDelta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, firstSegment);
            return;
        }
        PSVECSubtract(&playerPosition, &secondSegment->endPosition, &movementDelta);
        distance = PSVECMag(&movementDelta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, secondSegment);
            return;
        }
        fn_1_55D0(player, ai);
        return;
    }
    if (firstHit != secondHit) {
        if (firstHit != 0) {
            PSVECSubtract(&playerPosition, &secondSegment->endPosition, &movementDelta);
            distance = PSVECMag(&movementDelta);
            if (distance < 300.0f) {
                fn_1_45EC(player, ai, secondSegment);
            } else {
                fn_1_45EC(player, ai, firstSegment);
            }
            return;
        }
        PSVECSubtract(&playerPosition, &firstSegment->endPosition, &movementDelta);
        distance = PSVECMag(&movementDelta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, firstSegment);
        } else {
            fn_1_45EC(player, ai, secondSegment);
        }
        return;
    }
    if (fn_1_4260(&firstProjection, &secondProjection, &playerPosition, &nearestPoint, &distance) ==
        0) {
        if (firstDistance < secondDistance) {
            fn_1_45EC(player, ai, firstSegment);
        } else {
            fn_1_45EC(player, ai, secondSegment);
        }
    }
    /* The midpoint calculation still runs after a failed projection selected a fallback route
     * above. */
    nearestPoint.x = (firstProjection.x + secondProjection.x) / 2.0f;
    nearestPoint.y = 0.0f;
    nearestPoint.z = (firstProjection.z + secondProjection.z) / 2.0f;
    PSVECSubtract(&nearestPoint, &playerPosition, &movementDelta);
    if (PSVECMag(&movementDelta) > 70.0f) {
        ai->decisionCounter = 0;
        ai->destination = nearestPoint;
    }
}

/* Called by fn_1_3FFC when an outside-group computer has several hazards to avoid; selects up to
 * two segments using projected or endpoint distances, then dispatches their route choice. Empty
 * slots accept projections at any distance or endpoints within 300 units; nearer candidates can
 * replace an occupied slot. */
void fn_1_51D4(MGPLAYER *player, M633AI *ai)
{
    Point3d playerPosition, playerToEndpoint, nearestPoint;
    M633Segment *selected[2] = { NULL, NULL };
    float distances[2] = { 99999.0f, 99999.0f };
    float distance, endDistance, selectedDistance;
    M633Segment *segment;
    s32 segmentIndex, selectionIndex;

    playerPosition = player->actor->pos;
    playerPosition.y = 0.0f;
    segment = lbl_1_bss_0.segments;
    for (segmentIndex = 0; segmentIndex < lbl_1_bss_0.segmentCount; segment++, segmentIndex++) {
        s32 hit = fn_1_43D4(segment, &playerPosition, &nearestPoint, &distance);
        PSVECSubtract(&playerPosition, &segment->endPosition, &playerToEndpoint);
        endDistance = PSVECMag(&playerToEndpoint);
        for (selectionIndex = 0; selectionIndex < 2; selectionIndex++) {
            if (selected[selectionIndex] == NULL) {
                if (hit != 0 || endDistance < 300.0f) {
                    selected[selectionIndex] = segment;
                    distances[selectionIndex] = hit != 0 ? distance : endDistance;
                }
            } else {
                selectedDistance = hit != 0 ? distance : endDistance;
                if (selectedDistance < distances[selectionIndex]) {
                    selected[selectionIndex] = segment;
                    distances[selectionIndex] = selectedDistance;
                } else {
                    continue;
                }
            }
            break;
        }
    }
    if (selected[0] == NULL && selected[1] == NULL) {
        fn_1_55D0(player, ai);
        return;
    }
    if (selected[0] != NULL && selected[1] != NULL) {
        fn_1_4A00(player, ai, selected[0], selected[1]);
        return;
    }
    if (selected[0] != NULL) {
        fn_1_45EC(player, ai, selected[0]);
        return;
    }
    fn_1_45EC(player, ai, selected[1]);
}

/* Called by the outside-group routing callbacks to update a computer's roaming target and pad
 * input. A negative counter skips pad writes; the later bearing test can end that wait early. */
void fn_1_55D0(MGPLAYER *player, M633AI *ai)
{
    Point3d playerPosition;
    Point3d destinationPosition;
    Point3d movementDirection;
    Point3d playerPositionForBearing;
    Point3d arenaRotation;
    f32 playerBearingDegrees;
    f32 arenaYawDegrees;
    f32 movementAmount;
    f32 targetHeading;
    f32 distanceToTarget;
    f32 stageAngleDifference;
    f32 targetX;
    f32 targetZ;
    s32 directionChoice;
    s32 targetReached;

    targetReached = 0;
    if (ai->decisionCounter < 0) {
        ai->decisionCounter += 1;
        if (ai->decisionCounter >= 0) {
            targetReached = 1;
        }
    } else {
        playerPosition = player->actor->pos;
        destinationPosition = ai->destination;
        ai->decisionCounter += 1;
        if (ai->decisionCounter >= 90) {
            targetReached = 1;
            ai->targetHeadingDegrees =
                (f32) (180.0 *
                       (atan2((f64) playerPosition.z, (f64) playerPosition.x) / 3.141592653589793));
            ai->turnDirection = -ai->turnDirection;
        }
        playerPosition.y = 0.0f;
        PSVECSubtract(&destinationPosition, &playerPosition, &movementDirection);
        distanceToTarget = PSVECMag(&movementDirection);
        if (distanceToTarget < 20.0f) {
            targetReached = 1;
        }
        fn_1_5B20(player, &playerPosition, &destinationPosition, &movementDirection);
        MgPlayerPadSet(player, (s32) (56.0f * movementDirection.x),
                       (s32) (-56.0f * movementDirection.z), 0, 0);
    }
    directionChoice = frandmod(100);
    /* These original boolean-indexed checks read difficulty slots 0 and 1 rather than the current
     * player's slot. */
    if ((GwPlayerConf[player->playerNo == 3].comDif != 0) ||
        (GwPlayerConf[player->playerNo == 2].comDif != 0)) {
        playerPositionForBearing = player->actor->pos;
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &arenaRotation);
        playerBearingDegrees = (f32) (180.0 * (atan2((f64) playerPositionForBearing.x,
                                                     (f64) playerPositionForBearing.z) /
                                               3.141592653589793));
        arenaYawDegrees = arenaRotation.y;
        while (playerBearingDegrees < 0.0f) {
            playerBearingDegrees += 360.0f;
        }
        while (playerBearingDegrees > 360.0f) {
            playerBearingDegrees -= 360.0f;
        }
        while (arenaYawDegrees < 0.0f) {
            arenaYawDegrees += 360.0f;
        }
        while (arenaYawDegrees > 360.0f) {
            arenaYawDegrees -= 360.0f;
        }
        stageAngleDifference = playerBearingDegrees - arenaYawDegrees;
        if ((abs((s32) stageAngleDifference) < 30) && (ai->decisionCounter < 0)) {
            directionChoice = 100;
            targetReached = 1;
        }
    }
    if (targetReached != 0) {
        if (directionChoice < 10) {
            ai->decisionCounter = -(frandmod(120) + 60);
            return;
        }
        if (frandmod(100) < 4U) {
            ai->turnDirection = -ai->turnDirection;
        }
        /* Add frandmod(30) + 35 degrees to the target heading in the current turn direction. */
        movementAmount = (f32) (u32) (frandmod(30) + 35);
        ai->targetHeadingDegrees += movementAmount * ai->turnDirection;
        targetHeading = ai->targetHeadingDegrees;
        /* Rotate a 310-by-310 offset by the selected heading to set the next destination. */
        movementAmount = 310.0f;
        targetX = movementAmount * HuCos(targetHeading) - movementAmount * HuSin(targetHeading);
        targetZ = movementAmount * HuSin(targetHeading) + movementAmount * HuCos(targetHeading);
        ai->destination.x = targetX;
        ai->destination.y = 0.0f;
        ai->destination.z = targetZ;
        ai->decisionCounter = 0;
    }
}

/* Called by fn_1_55D0 before pad input is set; bends the current-to-destination direction around
 * nearby, nonremoved outside-group players. */
void fn_1_5B20(MGPLAYER *player, Point3d *startPosition, Point3d *destinationPosition,
               Point3d *movementDirection)
{
    Point3d nearbyPlayerPosition;
    Point3d avoidanceDelta;
    Point3d playerPosition;
    MGPLAYER *movingPlayer;
    f32 vectorMagnitude;
    f32 travelHeading;
    f32 sideOffset;
    f32 avoidanceHeadingOffset;
    f32 distanceToObstacle;
    f32 travelDistance;
    s32 nearbyPlayerFound;
    s32 playerNo;
    MGPLAYER *otherPlayer;

    avoidanceHeadingOffset = 0.0f;
    nearbyPlayerFound = 0;
    playerPosition = *startPosition;
    PSVECSubtract(destinationPosition, startPosition, movementDirection);
    travelDistance = PSVECMag(movementDirection);
    movingPlayer = player;
    travelHeading = (f32) (180.0 * (atan2((f64) movementDirection->z, (f64) movementDirection->x) /
                                    3.141592653589793));
    playerNo = 0;
    while (playerNo < 4) {
        otherPlayer = lbl_1_bss_0.players[playerNo];
        if ((movingPlayer != otherPlayer) && (lbl_1_bss_0.outsideGroupZero[playerNo] != 0) &&
            (lbl_1_bss_0.playerRemoved[playerNo] == 0)) {
            nearbyPlayerPosition = otherPlayer->actor->pos;
            nearbyPlayerPosition.y = 0.0f;
            PSVECSubtract(destinationPosition, &nearbyPlayerPosition, &avoidanceDelta);
            distanceToObstacle = PSVECMag(&avoidanceDelta);
            if (!(travelDistance < distanceToObstacle) && !(distanceToObstacle < 40.0f)) {
                nearbyPlayerFound = 1;
                PSVECSubtract(&playerPosition, &nearbyPlayerPosition, &avoidanceDelta);
                vectorMagnitude = PSVECMag(&avoidanceDelta);
                if (vectorMagnitude < 140.0f) {
                    sideOffset =
                        (nearbyPlayerPosition.x - destinationPosition->x) * HuSin(travelHeading) +
                        (destinationPosition->z - nearbyPlayerPosition.z) * HuCos(travelHeading);
                    sideOffset /= 90.0f;
                    if (sideOffset >= 0.0f) {
                        avoidanceHeadingOffset += 80.0f / (1.0f + sideOffset);
                    } else {
                        avoidanceHeadingOffset -= 80.0f / (1.0f - sideOffset);
                    }
                }
            }
        }
        playerNo += 1;
    }
    if (nearbyPlayerFound != 0) {
        travelHeading += avoidanceHeadingOffset;
        movementDirection->x = (f32) cos((3.141592653589793 * (f64) travelHeading) / 180.0);
        movementDirection->z = (f32) sin((3.141592653589793 * (f64) travelHeading) / 180.0);
    }
    vectorMagnitude = PSVECMag(movementDirection);
    if (vectorMagnitude != 0.0f) {
        movementDirection->x /= vectorMagnitude;
        movementDirection->y /= vectorMagnitude;
        movementDirection->z /= vectorMagnitude;
    }
}
