/* Computer-player decisions and the paths made by moving arena segments. */
#include "REL/m633dll.h"

void fn_1_3828(OMOBJ *obj)
{

}

void fn_1_382C(void)
{

}

/* Called for each slot by fn_1_690; sends each active computer slot to its current decision routine. */
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

/* Sets directional input flags toward a player or a random arena heading. */
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
    s32 chooseAttack;

    aiState = &lbl_1_bss_0.aiStates[playerNo];
    lbl_1_bss_0.aiButtons = 0;
    lbl_1_bss_0.aiPressedButtons = 0;
    if (lbl_1_bss_0.activePlayerCount != 0) {
        if ((f32) aiState->decisionCounter < 0.0f) {
            if (frandmod(100) < (u32) lbl_1_data_260[GwPlayerConf[playerNo].comDif]) {
                chooseAttack = 1;
            } else {
                chooseAttack = 0;
            }
            aiState->decisionMode = chooseAttack;
            switch (aiState->decisionMode) {
                do {
                case 0:
                /* Repeat the draw if it selects the current AI player. */
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
                aiState->destination.x = targetOffsetComponent * HuCos(directionCalculation) - targetOffsetComponent * HuSin(directionCalculation);
                aiState->destination.y = 0.0f;
                aiState->destination.z = targetOffsetComponent * HuSin(directionCalculation) + targetOffsetComponent * HuCos(directionCalculation);
                aiState->decisionCounter = frandmod(180) + 180;
                break;
            }
        }
        switch (aiState->decisionMode) {
        case 0:
            targetPlayerNo = aiState->targetPlayerNo;
            targetPlayer = lbl_1_bss_0.players[targetPlayerNo];
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_188, &stageObjectPosition);
            targetPosition = targetPlayer->actor->pos;
            break;
        case 1:
            targetPosition = aiState->destination;
            Hu3DModelObjPosGet(lbl_1_bss_0.rotatingStageModelId, lbl_1_data_188, &stageObjectPosition);
            break;
        }
        aiState->decisionCounter -= 1;
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &stageRotation);
        targetHeadingDegrees = (f32) (180.0 * (atan2((f64) targetPosition.x, (f64) targetPosition.z) / 3.141592653589793));
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
            PSVECSubtract(&targetPosition, &stageObjectPosition, &targetPosition);
            directionCalculation = (stageObjectPosition.x * targetPosition.z) - (stageObjectPosition.z * targetPosition.x);
            if (directionCalculation >= 0.0f) {
                lbl_1_bss_0.aiButtons = PAD_BUTTON_TRIGGER_R;
            } else {
                lbl_1_bss_0.aiButtons = PAD_BUTTON_TRIGGER_L;
            }
        }
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &movementRotation);
        movementYawDegrees = movementRotation.y;
        movementHeadingDegrees = (f32) (180.0 * (atan2((f64) targetPosition.z, (f64) targetPosition.x) / 3.141592653589793));
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
        if ((lbl_1_bss_0.arenaPhase == 2) && (((f32) abs((s32) alignmentDifference) < 20.0f) || ((f32) aiState->decisionCounter < 0.0f))) {
            lbl_1_bss_0.aiPressedButtons = PAD_BUTTON_A;
            aiState->decisionCounter = -1;
        }
    }
}

/* Chooses movement for a computer player, mixing free movement with routes around active arena segments. */
void fn_1_3FFC(s32 playerNo)
{
    Point3d pos;
    MGPLAYER *player;
    M633AI *ai;
    player = lbl_1_bss_0.players[playerNo];
    ai = &lbl_1_bss_0.aiStates[playerNo];
    if (lbl_1_bss_0.segmentCount == 0) {
        fn_1_55D0(player, ai);
        if (ai->decisionMode == 2) {
            pos = player->actor->pos;
            ai->targetHeadingDegrees = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
            ai->destination = pos;
            ai->decisionCounter = 0;
        }
        ai->decisionMode = 0;
    } else if (ai->decisionMode == 0) {
        if (frandmod(100) < (u32)lbl_1_data_270[GwPlayerConf[playerNo].comDif]) {
            ai->decisionMode = 2;
        } else {
            ai->decisionMode = 1;
        }
    }
    if (ai->decisionMode == 1 && lbl_1_bss_0.segmentReflectionSoundPending != 0
        && frandmod(100) < (u32)lbl_1_data_280[GwPlayerConf[playerNo].comDif]) {
        ai->decisionMode = 2;
    }
    switch (ai->decisionMode) {
    case 0:
    case 1:
        fn_1_55D0(player, ai);
        break;
    case 2:
        switch (lbl_1_bss_0.segmentCount) {
        case 0: fn_1_55D0(player, ai); break;
        case 1: fn_1_45EC(player, ai, NULL); break;
        case 2: fn_1_4A00(player, ai, NULL, NULL); break;
        default: fn_1_51D4(player, ai); break;
        }
        break;
    }
}

/* Projects a point onto a finite horizontal segment; returns whether the projection lies on it. */
s32 fn_1_4260(Point3d *start, Point3d *end, Point3d *pos, Point3d *nearest, float *distance)
{
    Point3d line, offset, delta;
    float projection, magnitude;

    if (start->x == end->x && start->y == end->y) {
        return 0;
    }
    PSVECSubtract(end, start, &line);
    PSVECSubtract(pos, start, &offset);
    projection = PSVECDotProduct(&line, &offset);
    magnitude = PSVECMag(&line);
    if (projection >= 0.0f && projection <= magnitude * magnitude) {
        PSVECNormalize(&line, &line);
        projection = PSVECDotProduct(&offset, &line);
        nearest->x = start->x + projection * line.x;
        nearest->y = start->y + projection * line.y;
        nearest->z = start->z + projection * line.z;
        if (distance) {
            PSVECSubtract(nearest, pos, &delta);
            *distance = PSVECMag(&delta);
        }
        return 1;
    }
    return 0;
}

/* Tests a player position against one arena segment after flattening all points to the ground plane. */
s32 fn_1_43D4(M633Segment *segment, Point3d *pos, Point3d *nearest, float *distance)
{
    Point3d start, end;
    s32 unusedZero;
    unusedZero = 0;
    start = segment->startPosition;
    end = segment->endPosition;
    start.y = end.y = pos->y = 0.0f;
    return fn_1_4260(&start, &end, pos, nearest, distance);
}

/* Returns the ground-plane midpoint between a segment’s endpoints. */
void fn_1_4598(M633Segment *segment, Point3d *pos)
{
    pos->x = (segment->startPosition.x + segment->endPosition.x) / 2.0f;
    pos->y = 0.0f;
    pos->z = (segment->startPosition.z + segment->endPosition.z) / 2.0f;
}

/* Moves a computer player along one segment or toward its endpoint; uses the first segment when none is supplied. */
void fn_1_45EC(MGPLAYER *player, M633AI *ai, M633Segment *segment)
{
    Point3d pos, nearest, delta;
    float distance;
    Point3d end, start;
    s32 unusedZero;
    M633Segment *current;

    current = segment == NULL ? lbl_1_bss_0.segments : segment;
    pos = player->actor->pos;
    if (current->startPosition.x == current->endPosition.x && current->startPosition.z == current->endPosition.z) {
        fn_1_55D0(player, ai);
        return;
    }
    pos.y = 0.0f;
    unusedZero = 0;
    start = current->startPosition;
    end = current->endPosition;
    start.y = end.y = pos.y = 0.0f;
    if (fn_1_4260(&start, &end, &pos, &nearest, &distance)) {
        if (distance < 100.0f) {
            PSVECSubtract(&pos, &nearest, &delta);
            PSVECNormalize(&delta, &delta);
            MgPlayerPadSet(player, (s32)(56.0f * delta.x), (s32)(-56.0f * delta.z), 0, 0);
            return;
        }
        ai->targetHeadingDegrees = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
        ai->destination = pos;
        return;
    }
    PSVECSubtract(&pos, &current->endPosition, &delta);
    distance = PSVECMag(&delta);
    if (distance < 300.0f) {
        PSVECNormalize(&delta, &delta);
        ai->decisionCounter = 0;
        ai->destination.x = pos.x + 100.0f * delta.x;
        ai->destination.y = 0.0f;
        ai->destination.z = pos.z + 100.0f * delta.z;
        fn_1_55D0(player, ai);
        return;
    }
    ai->targetHeadingDegrees = 180.0 * (atan2(pos.z, pos.x) / 3.141592653589793);
    ai->destination = pos;
    fn_1_55D0(player, ai);
}

/* Chooses movement input from two nearby segments, following an edge or moving toward the space between them. */
void fn_1_4A00(MGPLAYER *player, M633AI *ai, M633Segment *first, M633Segment *second)
{
    Point3d midFirst, midSecond;
    Point3d nearest, projectedFirst, projectedSecond, delta, pos;
    float distance, firstDistance, secondDistance;
    M633Segment *a, *b;
    s32 firstHit, secondHit;

    a = first == NULL ? lbl_1_bss_0.segments : first;
    b = second == NULL ? &lbl_1_bss_0.segments[1] : second;
    pos = player->actor->pos;
    pos.y = 0.0f;
    firstHit = fn_1_43D4(a, &pos, &projectedFirst, &firstDistance);
    secondHit = fn_1_43D4(b, &pos, &projectedSecond, &secondDistance);
    fn_1_4598(a, &midFirst);
    fn_1_4598(b, &midSecond);
    if (firstHit == 0 && secondHit == 0) {
        PSVECSubtract(&pos, &a->endPosition, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, a);
            return;
        }
        PSVECSubtract(&pos, &b->endPosition, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, b);
            return;
        }
        fn_1_55D0(player, ai);
        return;
    }
    if (firstHit != secondHit) {
        if (firstHit != 0) {
            PSVECSubtract(&pos, &b->endPosition, &delta);
            distance = PSVECMag(&delta);
            if (distance < 300.0f) {
                fn_1_45EC(player, ai, b);
            } else {
                fn_1_45EC(player, ai, a);
            }
            return;
        }
        PSVECSubtract(&pos, &a->endPosition, &delta);
        distance = PSVECMag(&delta);
        if (distance < 300.0f) {
            fn_1_45EC(player, ai, a);
        } else {
            fn_1_45EC(player, ai, b);
        }
        return;
    }
    if (fn_1_4260(&projectedFirst, &projectedSecond, &pos, &nearest, &distance) == 0) {
        if (firstDistance < secondDistance) {
            fn_1_45EC(player, ai, a);
        } else {
            fn_1_45EC(player, ai, b);
        }
    }
    nearest.x = (projectedFirst.x + projectedSecond.x) / 2.0f;
    nearest.y = 0.0f;
    nearest.z = (projectedFirst.z + projectedSecond.z) / 2.0f;
    PSVECSubtract(&nearest, &pos, &delta);
    if (PSVECMag(&delta) > 70.0f) {
        ai->decisionCounter = 0;
        ai->destination = nearest;
    }
}

/* Finds the nearest one or two active arena segments and dispatches the computer player’s route choice. */
void fn_1_51D4(MGPLAYER *player, M633AI *ai)
{
    Point3d pos, delta, nearest;
    M633Segment *selected[2] = { NULL, NULL };
    float distances[2] = { 99999.0f, 99999.0f };
    float distance, endDistance, selectedDistance;
    M633Segment *segment;
    s32 i, j;

    pos = player->actor->pos;
    pos.y = 0.0f;
    segment = lbl_1_bss_0.segments;
    for (i = 0; i < lbl_1_bss_0.segmentCount; segment++, i++) {
        s32 hit = fn_1_43D4(segment, &pos, &nearest, &distance);
        PSVECSubtract(&pos, &segment->endPosition, &delta);
        endDistance = PSVECMag(&delta);
        for (j = 0; j < 2; j++) {
            if (selected[j] == NULL) {
                if (hit != 0 || endDistance < 300.0f) {
                    selected[j] = segment;
                    distances[j] = hit != 0 ? distance : endDistance;
                }
            } else {
                selectedDistance = hit != 0 ? distance : endDistance;
                if (selectedDistance < distances[j]) {
                    selected[j] = segment;
                    distances[j] = selectedDistance;
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

/* Updates a computer player’s free-roaming target and supplies pad input toward it. */
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
            ai->targetHeadingDegrees = (f32) (180.0 * (atan2((f64) playerPosition.z, (f64) playerPosition.x) / 3.141592653589793));
            ai->turnDirection = -ai->turnDirection;
        }
        playerPosition.y = 0.0f;
        PSVECSubtract(&destinationPosition, &playerPosition, &movementDirection);
        distanceToTarget = PSVECMag(&movementDirection);
        if (distanceToTarget < 20.0f) {
            targetReached = 1;
        }
        fn_1_5B20(player, &playerPosition, &destinationPosition, &movementDirection);
        MgPlayerPadSet(player, (s32) (56.0f * movementDirection.x), (s32) (-56.0f * movementDirection.z), 0, 0);
    }
    directionChoice = frandmod(100);
    if ((GwPlayerConf[player->playerNo == 3].comDif != 0) || (GwPlayerConf[player->playerNo == 2].comDif != 0)) {
        playerPositionForBearing = player->actor->pos;
        Hu3DModelRotGet(lbl_1_bss_0.rotatingStageModelId, &arenaRotation);
        playerBearingDegrees = (f32) (180.0 * (atan2((f64) playerPositionForBearing.x, (f64) playerPositionForBearing.z) / 3.141592653589793));
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
        /* Advance the target heading by 35 to 64 degrees in the current turn direction. */
        movementAmount = (f32) (u32) (frandmod(30) + 35);
        ai->targetHeadingDegrees += movementAmount * ai->turnDirection;
        targetHeading = ai->targetHeadingDegrees;
        /* Use the same 310-unit component on both axes to set the next diagonal arena offset. */
        movementAmount = 310.0f;
        targetX = movementAmount * HuCos(targetHeading) - movementAmount * HuSin(targetHeading);
        targetZ = movementAmount * HuSin(targetHeading) + movementAmount * HuCos(targetHeading);
        ai->destination.x = targetX;
        ai->destination.y = 0.0f;
        ai->destination.z = targetZ;
        ai->decisionCounter = 0;
    }
}

/* Adjusts a movement vector around nearby active players outside group 0. */
void fn_1_5B20(MGPLAYER *player, Point3d *first, Point3d *second, Point3d *third)
{
    Point3d nearbyPlayerPosition;
    Point3d avoidanceDelta;
    Point3d playerPosition;
    MGPLAYER *movingPlayer;
    f32 playerSeparation;
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
    playerPosition = *first;
    PSVECSubtract(second, first, third);
    travelDistance = PSVECMag(third);
    movingPlayer = player;
    travelHeading = (f32) (180.0 * (atan2((f64) third->z, (f64) third->x) / 3.141592653589793));
    playerNo = 0;
    while (playerNo < 4) {
        otherPlayer = lbl_1_bss_0.players[playerNo];
        if ((movingPlayer != otherPlayer) && (lbl_1_bss_0.outsideGroupZero[playerNo] != 0) && (lbl_1_bss_0.playerRemoved[playerNo] == 0)) {
            nearbyPlayerPosition = otherPlayer->actor->pos;
            nearbyPlayerPosition.y = 0.0f;
            PSVECSubtract(second, &nearbyPlayerPosition, &avoidanceDelta);
            distanceToObstacle = PSVECMag(&avoidanceDelta);
            if (!(travelDistance < distanceToObstacle) && !(distanceToObstacle < 40.0f)) {
                nearbyPlayerFound = 1;
                PSVECSubtract(&playerPosition, &nearbyPlayerPosition, &avoidanceDelta);
                playerSeparation = PSVECMag(&avoidanceDelta);
                if (playerSeparation < 140.0f) {
                    sideOffset = (nearbyPlayerPosition.x - second->x) * HuSin(travelHeading)
                        + (second->z - nearbyPlayerPosition.z) * HuCos(travelHeading);
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
        third->x = (f32) cos((3.141592653589793 * (f64) travelHeading) / 180.0);
        third->z = (f32) sin((3.141592653589793 * (f64) travelHeading) / 180.0);
    }
    playerSeparation = PSVECMag(third);
    if (playerSeparation != 0.0f) {
        third->x /= playerSeparation;
        third->y /= playerSeparation;
        third->z /= playerSeparation;
    }
}
