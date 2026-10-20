/* Updates player movement, route selection, rankings, and completion phases. */
#include "dolphin/math.h"
#include "REL/m609dll/state.h"
#include "game/mg/actman.h"
#include "game/charman.h"
#include "game/frand.h"
void fn_1_11E2C(void);
void fn_1_125C4(void);
s16 fn_1_126F0(M609Player *movement, s16 advanceAfterChase);
s16 fn_1_12830(M609Player *action);
s16 fn_1_12990(M609Player *action);
s16 fn_1_12B28(M609Player *player);
void fn_1_12CDC(M609Player *player);
void fn_1_12FFC(M609Player *update);
s32 fn_1_1351C(M609Player *query);
s16 fn_1_137B0(M609Player *query);
s16 fn_1_13AA8(M609Player *query, s16 candidateSample);
s32 fn_1_13B88(M609Player *player, s16 option, s16 group);
s16 fn_1_13D58(M609Player *player, s16 allowCurrent);
void fn_1_14B64(M609Player *record);
s16 fn_1_1507C(M609Player *record);
void fn_1_159D4(M609Player *record);
void fn_1_15C24(M609Player *record);
void fn_1_164FC(void);

s16 lbl_1_data_3F8[] = {
    46, 47, 51, 52, 56, 60, 64, 61, 65, 48, 76, 78, 49, 68, 72, 69,
    73, 53, 54, 77, 79, 70, 74, 71, 75, 62, 66, 63, 67, 57, 58, 59,
    55, 50
};
Vec lbl_1_data_43C[] = { { 300.0f, 0.0f, 100.0f },  { 300.0f, 0.0f, -100.0f },
                         { 100.0f, 0.0f, 100.0f },  { 100.0f, 0.0f, -100.0f },
                         { -100.0f, 0.0f, 100.0f }, { -100.0f, 0.0f, -100.0f },
                         { -300.0f, 0.0f, 100.0f }, { -300.0f, 0.0f, -100.0f } };
Vec lbl_1_data_49C[] = { { 100.0f, 0.0f, 100.0f },
                         { 100.0f, 0.0f, -100.0f },
                         { -100.0f, 0.0f, 100.0f },
                         { -100.0f, 0.0f, -100.0f } };
Vec lbl_1_data_4CC[] = {
    { 300.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { -300.0f, 0.0f, 0.0f }
};
Vec lbl_1_data_4FC[] = {
    { 300.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { -300.0f, 0.0f, 0.0f }
};
Vec lbl_1_data_52C[] = {
    { 300.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { -300.0f, 0.0f, 0.0f }
};
Vec lbl_1_data_55C[] = {
    { 300.0f, 0.0f, 0.0f }, { 100.0f, 0.0f, 0.0f }, { -100.0f, 0.0f, 0.0f }, { -300.0f, 0.0f, 0.0f }
};
Vec lbl_1_data_58C[] = {{100.0f, 0.0f, 0.0f}, {-100.0f, 0.0f, 0.0f}};
Vec lbl_1_data_5A4[] = {{100.0f, 0.0f, 0.0f}, {-100.0f, 0.0f, 0.0f}};
Vec lbl_1_data_5BC[] = { { 188.0f, 0.0f, 70.5f },  { 188.0f, 0.0f, -70.5f },
                         { 0.0f, 0.0f, 70.5f },    { 0.0f, 0.0f, -70.5f },
                         { -188.0f, 0.0f, 70.5f }, { -188.0f, 0.0f, -70.5f } };
Vec lbl_1_data_604[] = { { 188.0f, 0.0f, 70.5f },  { 188.0f, 0.0f, -120.5f },
                         { 0.0f, 0.0f, 70.5f },    { 0.0f, 0.0f, -70.5f },
                         { -188.0f, 0.0f, 70.5f }, { -188.0f, 0.0f, -120.5f } };

/* Sets computer-player pacing and starting ranks during minigame setup. */
void fn_1_11E2C(void) {
    struct RandomRange {
        s16 baseIncrement;
        s16 randomSpan;
    } randomRanges[4] = {{15, 5}, {8, 3}, {1, 2}, {0, 1}};
    s16 playerOrder[4];
    s16 difficultyProbabilities[4] = {25, 5, 0, 0};
    s16 playerNo;
    s16 participantCount;
    s16 orderIndex;
    s16 randomValue;

    lbl_1_bss_6E28[0] = lbl_1_data_43C;
    lbl_1_bss_6E50[0] = 8;
    lbl_1_bss_6E28[1] = lbl_1_data_49C;
    lbl_1_bss_6E50[1] = 4;
    lbl_1_bss_6E28[2] = lbl_1_data_4CC;
    lbl_1_bss_6E50[2] = 4;
    lbl_1_bss_6E28[3] = lbl_1_data_4FC;
    lbl_1_bss_6E50[3] = 4;
    lbl_1_bss_6E28[4] = lbl_1_data_52C;
    lbl_1_bss_6E50[4] = 4;
    lbl_1_bss_6E28[5] = lbl_1_data_55C;
    lbl_1_bss_6E50[5] = 4;
    lbl_1_bss_6E28[6] = lbl_1_data_58C;
    lbl_1_bss_6E50[6] = 2;
    lbl_1_bss_6E28[7] = lbl_1_data_5A4;
    lbl_1_bss_6E50[7] = 2;
    lbl_1_bss_6E28[8] = lbl_1_data_5BC;
    lbl_1_bss_6E50[8] = 6;
    lbl_1_bss_6E28[9] = lbl_1_data_604;
    lbl_1_bss_6E50[9] = 6;

    playerOrder[0] = playerOrder[1] = playerOrder[2] = playerOrder[3] = -1;

    for (playerNo = 0, participantCount = 0; playerNo < 4; playerNo++) {
        lbl_1_bss_24C[playerNo].chaseSpeed =
            (10.0f);
        lbl_1_bss_24C[playerNo].chaseRadius =
            (10.1f);
        lbl_1_bss_24C[playerNo].routeSpeed =
            (10.0f);
        lbl_1_bss_24C[playerNo].routeRadius =
            (10.1f);
        lbl_1_bss_24C[playerNo].wait = 0;
        lbl_1_bss_24C[playerNo].waitTimer = 0;
        lbl_1_bss_24C[playerNo].period = 0;
        lbl_1_bss_24C[playerNo].rank = 0;
        randomValue = (s16)(
            randomRanges[lbl_1_bss_24C[playerNo].difficulty].baseIncrement +
            frandmod(randomRanges[
                lbl_1_bss_24C[playerNo].difficulty].randomSpan));
        lbl_1_bss_24C[playerNo].increment =
            randomValue;
        lbl_1_bss_24C[playerNo].probability =
            difficultyProbabilities[lbl_1_bss_24C[playerNo].difficulty];
        if (lbl_1_bss_24C[playerNo].participantType != 0) {
            playerOrder[participantCount] = participantCount;
            participantCount++;
        }
    }

    if (participantCount != 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_24C[playerNo].participantType != 0) {
                do {
                    orderIndex = frandmod(participantCount);
                } while (playerOrder[orderIndex] == -1);
                lbl_1_bss_24C[playerNo].rank =
                    playerOrder[orderIndex];
                playerOrder[orderIndex] = -1;
            }
        }
    }

    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_24C[playerNo].participantType != 0) {
            for (orderIndex = 0; orderIndex < 4; orderIndex++) {
                if (lbl_1_bss_24C[orderIndex].participantType != 0 &&
                    lbl_1_bss_24C[playerNo].difficulty >
                        lbl_1_bss_24C[orderIndex].difficulty &&
                    lbl_1_bss_24C[playerNo].rank >
                        lbl_1_bss_24C[orderIndex].rank) {
                    playerOrder[0] =
                        lbl_1_bss_24C[playerNo].rank;
                    lbl_1_bss_24C[playerNo].rank =
                        lbl_1_bss_24C[orderIndex].rank;
                    lbl_1_bss_24C[orderIndex].rank =
                        playerOrder[0];
                }
            }
        }
    }

    /* Repeat the same difficulty/rank swap sweep a second time. */
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_24C[playerNo].participantType != 0) {
            for (orderIndex = 0; orderIndex < 4; orderIndex++) {
                if (lbl_1_bss_24C[orderIndex].participantType != 0 &&
                    lbl_1_bss_24C[playerNo].difficulty >
                        lbl_1_bss_24C[orderIndex].difficulty &&
                    lbl_1_bss_24C[playerNo].rank >
                        lbl_1_bss_24C[orderIndex].rank) {
                    playerOrder[0] =
                        lbl_1_bss_24C[playerNo].rank;
                    lbl_1_bss_24C[playerNo].rank =
                        lbl_1_bss_24C[orderIndex].rank;
                    lbl_1_bss_24C[orderIndex].rank =
                        playerOrder[0];
                }
            }
        }
    }
}

/* Advances computer route actions during the course selection update. */
void fn_1_125C4(void) {
    s16 playerNo;

    if (lbl_1_bss_30.resultsReady == 0) {
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (lbl_1_bss_24C[playerNo].participantType != 0 &&
                lbl_1_bss_24C[playerNo].phase == 0 &&
                lbl_1_bss_24C[playerNo].mode != 1) {
                M609Player *record = &lbl_1_bss_24C[playerNo];
                record->computerButtonsHeld = 0;
                record->computerButtonsDown = 0;
                record->computerStickX = 0;
                record->computerStickY = 0;
                switch (record->routeState) {
                case 0:
                    if (record->segmentModel >= 46) {
                        fn_1_14B64(record);
                    }
                    break;
                case 1:
                    break;
                }
            }
        }
    }
}

/* Moves a player toward a route point during route selection, setup and travel updates. */
s16 fn_1_126F0(M609Player *movement, s16 advanceAfterChase) {
    s16 complete = 0;
    s16 chased = 0;

    switch (movement->movementState) {
    case 0:
        MgPlayerComStkOn(lbl_1_bss_4.players[movement->playerNo]);
        movement->movementState++;
        break;
    case 1: {
        HuVecF position = movement->routePosition;
        chased = MgPlayerVecChase(
            lbl_1_bss_4.players[movement->playerNo],
            &position,
            movement->chaseSpeed,
            movement->chaseRadius);
        if (chased != 0) {
            if (advanceAfterChase != 0) {
                movement->movementState++;
            } else {
                complete = 1;
            }
        }
        break;
    }
    case 2:
        MgPlayerComStkOff(lbl_1_bss_4.players[movement->playerNo]);
        movement->movementState = 0;
        complete = 1;
        break;
    }

    return complete;
}

/* After fn_1_14B64 selects a target, drives jump and chase input until a collision mesh
 * is reached. The chase-arrival result is ignored. */
s16 fn_1_12830(M609Player *action) {
    s16 complete = 0;

    switch (action->routeStep) {
    case 0:
        if (lbl_1_bss_4.players[action->playerNo]->mode != 1) {
            action->computerButtonsDown |= 0x100;
        } else {
            MgPlayerComStkOn(lbl_1_bss_4.players[action->playerNo]);
            action->computerButtonsHeld |= 0x100;
            action->chaseUpdateCount++;
            action->routeStep++;
        }
        break;
    case 1:
        if (action->collisionMesh < 0) {
            goto chasePlayer;
        }
        MgPlayerComStkOff(lbl_1_bss_4.players[action->playerNo]);
        action->chaseUpdateCount = 0;
        action->routeStep = 0;
        complete = 1;
        break;
    chasePlayer:
        MgPlayerVecChase(
            lbl_1_bss_4.players[action->playerNo],
            &action->chasePosition,
            action->routeSpeed,
            action->routeRadius);
        action->computerButtonsHeld |= 0x100;
        action->chaseUpdateCount++;
        break;
    }

    return complete;
}

/* Helper for the unused route-travel path: completes on mesh contact outside jump mode,
 * or continues chasing without a mesh contact. The chase-arrival result is ignored. */
s16 fn_1_12990(M609Player *action) {
    s16 complete = 0;

    switch (action->routeStep) {
    case 0:
        if (lbl_1_bss_4.players[action->playerNo]->mode != 1) {
            action->computerButtonsDown |= 0x100;
        } else {
            MgPlayerComStkOn(lbl_1_bss_4.players[action->playerNo]);
            action->computerButtonsHeld |= 0x100;
            action->chaseUpdateCount++;
            action->routeStep++;
        }
        break;
    case 1:
        if (action->collisionMesh >= 0 &&
            lbl_1_bss_4.players[action->playerNo]->mode != 1) {
            MgPlayerComStkOff(lbl_1_bss_4.players[action->playerNo]);
            action->chaseUpdateCount = 0;
            action->routeStep = 0;
            complete = 1;
        } else if (action->collisionMesh == -1) {
            MgPlayerVecChase(
                lbl_1_bss_4.players[action->playerNo],
                &action->chasePosition,
                action->routeSpeed,
                action->routeRadius);
            action->computerButtonsHeld |= 0x100;
            action->chaseUpdateCount++;
        }
        break;
    }

    return complete;
}

/* Before route sampling, checks same-segment computer ranks and may swap ranks with a
 * scaled-down player of equal difficulty. Returns whether route choice may proceed. */
s16 fn_1_12B28(M609Player *player) {
    s16 playerNo;
    s16 result;
    s16 eligibleButNoSwap;
    s16 savedRank;

    result = 0;
    eligibleButNoSwap = 0;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_24C[playerNo].participantType != 0 &&
            playerNo != player->playerNo &&
            lbl_1_bss_24C[playerNo].phase == 0 &&
            player->segmentModel == lbl_1_bss_24C[playerNo].segmentModel &&
            player->rank > lbl_1_bss_24C[playerNo].rank) {
            if (lbl_1_bss_24C[playerNo].scaledDown != 0 &&
                player->difficulty == lbl_1_bss_24C[playerNo].difficulty) {
                savedRank = player->rank;
                player->rank = lbl_1_bss_24C[playerNo].rank;
                lbl_1_bss_24C[playerNo].rank = savedRank;
            } else {
                eligibleButNoSwap += 1;
            }
        }
    }
    if (eligibleButNoSwap <= 0) {
        result = 1;
    } else if (frandmod(100) == 0) {
        result = 1;
    }
    return result;
}

/* Builds route choices during computer selection and in the unused route-travel path. */
void fn_1_12CDC(M609Player *player)
{
    s16 sourceIndex;
    s16 candidate;
    s16 extraCandidates[8];

    sourceIndex = lbl_1_bss_8BC[player->segmentModel].status;
    /* Read the default successor before applying special cases. For final segment 50 this
     * reads one entry beyond the table, then its case replaces the result with -1. */
    player->candidateSegments[0] = lbl_1_data_3F8[sourceIndex + 1];
    player->candidateSegments[1] = -1;
    player->candidateSegments[2] = -1;
    player->candidateSegments[3] = -1;

    switch (lbl_1_data_3F8[sourceIndex]) {
    case 46:
        player->candidateSegments[0] = 47;
        break;
    case 47:
        if (player->position.x < (0.0f)) {
            player->candidateSegments[0] = 52;
            player->candidateSegments[1] = 51;
        } else {
            player->candidateSegments[0] = 51;
            player->candidateSegments[1] = 52;
        }
        break;
    case 51:
    case 52:
        player->candidateSegments[0] = 56;
        break;
    case 56:
        player->candidateSegments[0] = 64;
        player->candidateSegments[1] = 61;
        player->candidateSegments[2] = 65;
        player->candidateSegments[3] = 60;
        break;
    case 49:
        player->candidateSegments[0] = 72;
        player->candidateSegments[1] = 69;
        player->candidateSegments[2] = 73;
        player->candidateSegments[3] = 68;
        break;
    case 60:
    case 61:
    case 64:
    case 65:
        player->candidateSegments[0] = 48;
        break;
    case 68:
    case 69:
    case 72:
    case 73:
        player->candidateSegments[0] = 53;
        break;
    case 79:
        if (player->position.x < (0.0f)) {
            player->candidateSegments[0] = 63;
            player->candidateSegments[1] = 66;
            player->candidateSegments[2] = 67;
            player->candidateSegments[3] = 62;
        } else {
            player->candidateSegments[0] = 74;
            player->candidateSegments[1] = 71;
            player->candidateSegments[2] = 75;
            player->candidateSegments[3] = 70;
        }
        extraCandidates[0] = 66;
        extraCandidates[1] = 63;
        extraCandidates[2] = 67;
        extraCandidates[3] = 62;
        extraCandidates[4] = 71;
        extraCandidates[5] = 74;
        extraCandidates[6] = 75;
        extraCandidates[7] = 70;
        break;
    case 62:
    case 63:
    case 66:
    case 67:
    case 70:
    case 71:
    case 74:
    case 75:
        player->candidateSegments[0] = 57;
        break;
    case 50:
        player->candidateSegments[0] = -1;
        player->candidateSegments[1] = -1;
        break;
    default:
        break;
    }

    if (lbl_1_bss_30.resultFlag != 0) {
        if (lbl_1_data_3F8[sourceIndex] != 79) {
            for (candidate = 0; candidate < 4; candidate++) {
                if (player->candidateSegments[candidate] == lbl_1_bss_30.skippedSegmentModel) {
                    player->candidateSegments[0] = 50;
                    player->candidateSegments[1] = -1;
                    player->candidateSegments[2] = -1;
                    player->candidateSegments[3] = -1;
                    break;
                }
            }
        } else {
            for (candidate = 0; candidate < 8; candidate++) {
                if (lbl_1_bss_30.skippedSegmentModel == extraCandidates[candidate]) {
                    player->candidateSegments[0] = 50;
                    player->candidateSegments[1] = -1;
                    player->candidateSegments[2] = -1;
                    player->candidateSegments[3] = -1;
                    break;
                }
            }
        }
    }
}

/* Projects a moving course segment for computer route choice and the unused route-travel path. */
void fn_1_12FFC(M609Player *update) {
    s16 frameLimit = 36;
    HuVecF start = lbl_1_bss_8BC[update->targetSegment].position;
    HuVecF motionReferenceOrPlayerDelta = lbl_1_bss_8BC[update->targetSegment].positionStep;
    HuVecF gates = lbl_1_bss_8BC[update->targetSegment].movementSelectors;
    HuVecF projectedMovementValues = lbl_1_bss_8BC[update->targetSegment].movementValues;
    s16 frame;
    s16 segmentState = lbl_1_bss_8BC[update->targetSegment].stateOrTimer;
    s16 phase = lbl_1_bss_8BC[update->targetSegment].movementPhase;
    frame = 0;

    for (; frame < frameLimit; frame++) {
        start.z += lbl_1_bss_30.scroll;
        if (start.z >= (100.0f)) {
            segmentState = 2;
        }

        if (gates.x) {
            if ((segmentState >= 2) && (projectedMovementValues.x < (0.5f)) &&
                (projectedMovementValues.x > (-0.5f))) {
                projectedMovementValues.y = (0.0f);
                projectedMovementValues.x = (0.0f);
            }
            projectedMovementValues.x += projectedMovementValues.y;
            start.x += projectedMovementValues.x;
            switch (phase) {
            case 0:
                if (start.x < motionReferenceOrPlayerDelta.x) {
                    projectedMovementValues.y *= (-1.0f);
                    phase++;
                }
                break;
            case 1:
                if (start.x > motionReferenceOrPlayerDelta.x) {
                    projectedMovementValues.y *= (-1.0f);
                    phase = 0;
                }
                break;
            }
        } else if (gates.z) {
            if (segmentState >= 2) {
                projectedMovementValues.y = (0.0f);
                projectedMovementValues.z = (0.0f);
            }
            projectedMovementValues.z += projectedMovementValues.y;
            motionReferenceOrPlayerDelta.y += projectedMovementValues.z;
            start.z += projectedMovementValues.z;
            switch (phase) {
            case 0:
                if (motionReferenceOrPlayerDelta.y < (0.0f)) {
                    projectedMovementValues.y *= (-1.0f);
                    phase++;
                }
                break;
            case 1:
                if (motionReferenceOrPlayerDelta.y > (0.0f)) {
                    projectedMovementValues.y *= (-1.0f);
                    phase = 0;
                }
                break;
            }
        }
    }

    update->origin = start;
    motionReferenceOrPlayerDelta.x = start.x - update->position.x;
    motionReferenceOrPlayerDelta.z = start.z - update->position.z;
    /* Store the horizontal distance to the projected segment origin;
     * this field is not read by this module. */
    update->distance118 =
        sqrt((double) ((motionReferenceOrPlayerDelta.x * motionReferenceOrPlayerDelta.x) +
                       (motionReferenceOrPlayerDelta.z * motionReferenceOrPlayerDelta.z)));
}

/* Unused route test: returns one when the target is within two player radii
 * of another phase-0 player. */
s32 fn_1_1351C(M609Player *query) {
    Vec difference;
    f32 distanceSquared;
    f32 distance;
    f32 searchRange;
    s32 found = 0;
    s16 playerNo;

    searchRange = lbl_1_bss_30.playerRadius +
                  lbl_1_bss_30.playerRadius;
    playerNo = 0;
    while (playerNo < 4) {
        if (query->playerNo != lbl_1_bss_24C[playerNo].playerNo &&
            lbl_1_bss_24C[playerNo].phase == 0) {
            difference.x = query->target.x - lbl_1_bss_24C[playerNo].position.x;
            difference.z = query->target.z - lbl_1_bss_24C[playerNo].position.z;
            distanceSquared = (difference.x * difference.x) +
                              (difference.z * difference.z);
            distance = (f32)sqrt((f64)distanceSquared);
            if (distance < searchRange) {
                found = 1;
                break;
            }
        }
        playerNo++;
    }
    return found;
}

/* Rejects a candidate target near an auxiliary model linked to the projected target segment. */
s16 fn_1_137B0(M609Player *query) {
    Vec difference;
    f32 distance;
    f32 searchRange;
    s16 found = 0;
    s16 auxiliaryModelIndex;
    s16 candidate;

    searchRange = lbl_1_bss_30.playerRadius +
                  lbl_1_bss_30.hazardRadius;
    candidate = 0;
    while (candidate < 6) {
        if (lbl_1_bss_8BC[query->targetSegment].linkedRecords[candidate] != -1) {
            auxiliaryModelIndex = lbl_1_bss_8BC[query->targetSegment].linkedRecords[candidate];
            difference.x = query->target.x -
                           (query->origin.x +
                             lbl_1_bss_8BC[auxiliaryModelIndex].movementValues.x);
            difference.y = query->target.y -
                           (lbl_1_bss_30.raftOffset.y +
                            (query->origin.y +
                              lbl_1_bss_8BC[auxiliaryModelIndex].movementValues.y));
            difference.z = query->target.z -
                           (query->origin.z +
                             lbl_1_bss_8BC[auxiliaryModelIndex].movementValues.z);
            distance = (f32)sqrt((f64)((difference.z * difference.z) +
                              ((difference.x * difference.x) +
                               (difference.y * difference.y))));
            if (distance < searchRange) {
                found = 1;
                break;
            }
        }
        candidate++;
    }
    return found;
}

/* During route choice, compares another phase-0 player's current segment and stored
 * sample with the queried target segment and candidate sample; positions are not tested. */
s16 fn_1_13AA8(M609Player *query, s16 candidateSample) {
    s16 result = 0;
    s16 playerNo = 0;

    while (playerNo < 4) {
        if (query->playerNo != lbl_1_bss_24C[playerNo].playerNo &&
            lbl_1_bss_24C[playerNo].phase == 0 &&
            query->targetSegment == lbl_1_bss_24C[playerNo].segmentModel &&
            candidateSample == lbl_1_bss_24C[playerNo].sample) {
            result = 1;
        } else {
            playerNo += 1;
            continue;
        }
        break;
    }
    return result;
}

/* Filters course-side mismatches while a candidate route sample is evaluated. */
s32 fn_1_13B88(M609Player *player, s16 option, s16 group) {
    s32 result;
    f32 lateralDifference[1];

    result = 0;
    switch (player->segmentModel) {
    case 51:
        if (player->target.x < (0.0f)) {
            result = 1;
        }
        break;
    case 52:
        if (player->target.x > (0.0f)) {
            result = 1;
        }
        break;
    case 79:
        if (player->position.x < (0.0f)) {
            if (player->targetSegment == 70 ||
                player->targetSegment == 74 ||
                player->targetSegment == 71 ||
                player->targetSegment == 75) {
                result = 1;
            }
        } else if (player->targetSegment == 62 ||
                   player->targetSegment == 66 ||
                   player->targetSegment == 63 ||
                   player->targetSegment == 67) {
            result = 1;
        }
        break;
    case 57:
        if (option == 1 || option == 2) {
            result = 1;
        }
        break;
    case 58:
        lateralDifference[0] =
            lbl_1_bss_8BC[player->segmentModel].position.x - player->position.x;
        if (lateralDifference[0] < (0.0f)) {
            if (option == 2 || option == 3) {
                result = 1;
            }
        }
        if (lateralDifference[0] > (0.0f)) {
            if (option == 0 || option == 1) {
                result = 1;
            }
        }
        break;
    default:
        break;
    }
    return result;
}

/* Chooses route samples during computer choice and travel updates. The current-segment
 * fallback can bypass restrictions and omits hazard and other-player stored-sample checks. */
s16 fn_1_13D58(M609Player *player, s16 allowCurrent)
{
    Vec delta;
    s32 randomRollOrRestriction;
    s16 result;
    s16 candidateAllowed;
    s16 candidate;
    s16 attemptIndexOrAllowed;
    s16 group;
    s32 sample;
    f32 extraRange = 0.0f;
    f32 distance;
    f32 distanceSquared;

    result = 0;
    candidateAllowed = 1;
    if (fn_1_12B28(player)) {
        if (player->waitTimer-- <= 0) {
            fn_1_12CDC(player);
            for (candidate = 0; candidate < 4; candidate++) {
                if (player->candidateSegments[candidate] != -1) {
                    player->targetSegment = player->candidateSegments[candidate];
                    fn_1_12FFC(player);
                    /* candidateAllowed is shared by every attempt and candidate;
                     * a rejection is not reset here. */
                    for (attemptIndexOrAllowed = 0; attemptIndexOrAllowed < 30;
                         attemptIndexOrAllowed++) {
                        group = lbl_1_bss_8BC[player->targetSegment].pathIndex;
                        sample = frandmod(lbl_1_bss_6E50[group]);
                        player->target.x = player->origin.x + lbl_1_bss_6E28[group][sample].x;
                        player->target.y = 50.0f;
                        player->target.z = player->origin.z + lbl_1_bss_6E28[group][sample].z;
                        delta.x = player->target.x - player->position.x;
                        delta.z = player->target.z - player->position.z;
                        distanceSquared = delta.x * delta.x + delta.z * delta.z;
                        distance = sqrt(distanceSquared);
                        if (lbl_1_bss_8BC[player->segmentModel].position.z >= 95.0f) {
                            extraRange = 100.0f;
                        }
                        if (player->scaledDown == 0) {
                            player->range = 430.0f;
                        } else {
                            player->range = 120.0f;
                        }
                        switch (player->targetSegment) {
                        case 46: case 47: case 48: case 49:
                        case 50: case 51: case 52: case 53:
                        case 54: case 55:
                        case 76: case 77: case 78: case 79:
                            if (sample % 2) {
                                candidateAllowed = 0;
                            }
                            break;
                        case 59:
                            if (lbl_1_bss_8BC[player->segmentModel].position.x >
                                    player->position.x &&
                                (sample == 0 || sample == 1)) {
                                candidateAllowed = 0;
                            } else if (lbl_1_bss_8BC[player->segmentModel].position.x <
                                           player->position.x &&
                                       (sample == 2 || sample == 3)) {
                                candidateAllowed = 0;
                            }
                            break;
                        }
                        if (player->target.z > -800.0f &&
                            distance < player->range + extraRange && candidateAllowed) {
                            if (!fn_1_137B0(player)) {
                                if (!fn_1_13AA8(player, sample) &&
                                    (s16)fn_1_13B88(player, sample, group) == 0) {
                                    player->chasePosition = *(Vec *)&player->target.x;
                                    player->sample = sample;
                                    player->sampleGroup = group;
                                    player->period += player->increment;
                                    player->waitTimer = player->period;
                                    attemptIndexOrAllowed = candidate = 99;
                                    result = 3;
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    if (player->wait-- <= 0 && result == 0 && allowCurrent) {
        group = lbl_1_bss_8BC[player->segmentModel].pathIndex;
        player->movementSegment = 0;
        randomRollOrRestriction = frandmod(100);
        if (player->probability <= randomRollOrRestriction) {
            randomRollOrRestriction = 1;
        } else {
            randomRollOrRestriction = 0;
        }
        candidate = 0;
        attemptIndexOrAllowed = 1;
        /* attemptIndexOrAllowed is retained across samples; once a restriction clears it,
         * later samples stay rejected. */
        for (; candidate < 30; candidate++) {
            sample = frandmod(lbl_1_bss_6E50[group]);
            player->movementSegment = player->segmentModel;
            if (randomRollOrRestriction) {
                /* This current-segment fallback applies the restriction using the stored
                 * targetSegment, which may come from an earlier candidate search. */
                switch (player->targetSegment) {
                case 46: case 47: case 48: case 49:
                case 50: case 51: case 52: case 53:
                case 54: case 55:
                case 76: case 77: case 78: case 79:
                    if (!(sample % 2)) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                }
                switch (player->segmentModel) {
                case 47:
                    if ((player->position.x > 0.0f && sample >= 4) ||
                        (player->position.x < 0.0f && sample <= 3)) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                case 60: case 64: case 65:
                    player->movementSegment = 61;
                    break;
                case 68: case 69: case 73:
                    player->movementSegment = 72;
                    break;
                case 70: case 74: case 75:
                    player->movementSegment = 71;
                    break;
                case 62: case 63: case 67:
                    player->movementSegment = 66;
                    break;
                case 48:
                    if (sample == 0 || sample == 1 || sample == 6 || sample == 7) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                case 49:
                    if (1.25f * player->position.x + player->position.z >
                            lbl_1_bss_8BC[player->segmentModel].position.z &&
                        sample != 0 && sample != 1) {
                        attemptIndexOrAllowed = 0;
                    } else if (1.25f * player->position.x + player->position.z <
                                   lbl_1_bss_8BC[player->segmentModel].position.z &&
                               sample != 6 && sample != 7) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                case 77:
                    if (sample != 2 && sample != 3) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                case 79:
                    if (player->sample == 0 || player->sample == 2 || player->sample == 4) {
                        if (player->sample == 0 && sample == 5) {
                            attemptIndexOrAllowed = 0;
                        }
                        if (player->sample == 4 && sample == 1) {
                            attemptIndexOrAllowed = 0;
                        }
                        if (sample == 3) {
                            attemptIndexOrAllowed = 0;
                        }
                        /* After checking the random sample, stored samples 0 and 4 force the chosen
                         * sample to 1 and 5 respectively; the rejection flag is retained. */
                        if (player->sample == 0) {
                            sample = 1;
                        }
                        if (player->sample == 4) {
                            sample = 5;
                        }
                    } else {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                case 57: case 59:
                    attemptIndexOrAllowed = 0;
                    break;
                case 58:
                    if (lbl_1_bss_8BC[player->segmentModel].position.x > player->position.x &&
                        (sample == 0 || sample == 1)) {
                        attemptIndexOrAllowed = 0;
                    } else if (lbl_1_bss_8BC[player->segmentModel].position.x <
                                   player->position.x &&
                               (sample == 2 || sample == 3)) {
                        attemptIndexOrAllowed = 0;
                    }
                    break;
                }
            }
            if (attemptIndexOrAllowed) {
                player->sampleGroup = group;
                player->sample = sample;
                /* The modulus is one, so the wait is always zero. */
                player->wait = frandmod(1);
                result = 2;
                break;
            }
        }
    }
    return result;
}

/* Advances computer route choices during the course-selection update. */
void fn_1_14B64(M609Player *record) {
    s16 result = 0;
    s16 nextState = 0;
    Vec position;

    if (lbl_1_bss_30.resultFlag != 0) {
        record->increment = 0;
        if (frandmod(3) == 0 && --record->period < 0) {
            record->period = 0;
        }
    }

    switch (record->actionState) {
    case 0:
        if ((s16)(nextState = fn_1_13D58(record, 1)) != 0) {
            record->timer = 0;
            record->actionState = nextState;
        }
        break;
    case 1:
        if (record->timer++ >= 5) {
            record->actionState = 0;
            return;
        }
        break;
    case 2: {
        record->routePosition.x =
            lbl_1_bss_8BC[record->movementSegment].position.x +
            lbl_1_bss_6E28[record->sampleGroup][record->sample].x;
        record->routePosition.z =
            lbl_1_bss_8BC[record->movementSegment].position.z +
            lbl_1_bss_6E28[record->sampleGroup][record->sample].z;
        nextState = fn_1_126F0(record, 1);
        if ((s16)nextState != 0) {
            record->actionState = 0;
            return;
        }

        if ((s16)(nextState = fn_1_13D58(record, 0)) == 3) {
            MgPlayerComStkOff(lbl_1_bss_4.players[record->playerNo]);
            record->movementState = 0;
            record->timer = 0;
            record->actionState = nextState;
            return;
        }
        if (record->linkCount > 60) {
            MgPlayerComStkOff(lbl_1_bss_4.players[record->playerNo]);
            record->movementState = 0;
            record->timer = 0;
            record->actionState = 0;
            return;
        }
        break;
    }
    case 3:
        switch (record->routePhase) {
        case 0:
            nextState = fn_1_12830(record);
            if ((s16)nextState != 0) {
                record->routePhase++;
                return;
            }
            break;
        case 1:
            if (record->timer++ >= 10) {
                record->routePhase++;
                return;
            }
            break;
        case 2:
            switch (record->segmentModel) {
            case 60: case 61: case 62: case 63:
            case 64: case 65: case 66: case 67:
            case 68: case 69: case 70: case 71:
            case 72: case 73: case 74: case 75:
                record->actionState = 1;
                record->routePhase = 0;
                return;
            }
            record->actionState = 1;
            record->routePhase = 0;
            break;
        }
        break;
    }
}

/* Moves a player between course landmarks in the unused route-travel path. */
s16 fn_1_1507C(M609Player *record)
{
    s16 result = 0;
    s16 changed = 0;

    switch (record->phaseStep) {
    case 0:
        record->phasePosition.x = 0.0f;
        record->phasePosition.z =
            50.0f - lbl_1_bss_8BC[record->segmentModel].bounds.z;
        record->secondLegRequired = 0;
        record->movementSegment = record->segmentModel;
        switch (record->segmentModel) {
        case 47:
            if (record->position.x < 0.0f) {
                record->phasePosition.x = -150.0f;
            } else {
                record->phasePosition.x = 150.0f;
            }
            break;
        case 56:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 61:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 72:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 60:
        case 64:
        case 65:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            record->movementSegment = 61;
            break;
        case 68:
        case 69:
        case 73:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            record->movementSegment = 72;
            break;
        case 71:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 63:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 70:
        case 74:
        case 75:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            record->movementSegment = 71;
            break;
        case 62:
        case 66:
        case 67:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            record->movementSegment = 63;
            break;
        case 76:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 78:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = -150.0f;
            break;
        case 49:
            record->phasePosition.z = 50.0f -
                lbl_1_bss_8BC[record->segmentModel].bounds.z;
            if ((f32)(-1.25f * record->position.x) +
                record->position.z >
                lbl_1_bss_8BC[record->segmentModel].position.z) {
                record->phasePosition.x = 50.0f -
                    lbl_1_bss_8BC[record->segmentModel].bounds.x;
            } else {
                record->phasePosition.x =
                    lbl_1_bss_8BC[record->segmentModel].bounds.x -
                    50.0f;
            }
            record->secondLegRequired = 1;
            break;
        case 77:
            record->phasePosition.x = record->position.x -
                lbl_1_bss_8BC[record->segmentModel].position.x;
            record->phasePosition.z = record->position.z -
                lbl_1_bss_8BC[record->segmentModel].position.z;
            break;
        case 79:
            if (record->position.z <=
                lbl_1_bss_8BC[record->segmentModel].position.z -
                50.0f) {
                record->phasePosition.x = record->position.x -
                    lbl_1_bss_8BC[record->segmentModel].position.x;
                record->phasePosition.z = record->position.z -
                    lbl_1_bss_8BC[record->segmentModel].position.z;
            } else {
                if (record->position.x < 0.0f) {
                    record->phasePosition.x = -200.0f;
                } else {
                    record->phasePosition.x = 200.0f;
                }
                record->phasePosition.z = record->position.z -
                    lbl_1_bss_8BC[record->segmentModel].position.z;
                record->secondLegRequired = 1;
            }
            break;
        case 57:
            record->phasePosition.x = 100.0f;
            record->phasePosition.z = 0.0f;
            break;
        case 58:
            if (record->position.x <
                lbl_1_bss_8BC[record->segmentModel].position.x) {
                record->phasePosition.x = -200.0f;
            } else {
                record->phasePosition.x = 200.0f;
            }
            record->phasePosition.z = 0.0f;
            break;
        case 59:
            record->phasePosition.x = 0.0f;
            record->phasePosition.z = 0.0f;
            break;
        }
        (record->phaseStep)++;
    case 1:
        record->routePosition.x =
            record->phasePosition.x +
            lbl_1_bss_8BC[record->movementSegment].position.x;
        record->routePosition.z =
            record->phasePosition.z +
            lbl_1_bss_8BC[record->movementSegment].position.z;
        switch (record->segmentModel) {
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            break;
        default:
            if (record->routePosition.z > record->position.z) {
                record->routePosition.z = record->position.z;
            }
        }
        changed = fn_1_126F0(record, 1);
        if ((s16)changed) {
            if (record->secondLegRequired == 0) {
                record->phaseStep = 0;
                result = 1;
            } else {
                (record->phaseStep)++;
            }
        }
        break;
    case 2:
        switch (record->segmentModel) {
        case 49:
            record->phasePosition.x = record->position.x -
                lbl_1_bss_8BC[record->segmentModel].position.x;
            record->phasePosition.z = 50.0f -
                lbl_1_bss_8BC[record->segmentModel].bounds.z;
            break;
        case 79:
            record->phasePosition.x = record->position.x -
                lbl_1_bss_8BC[record->segmentModel].position.x;
            record->phasePosition.z = -100.0f;
            break;
        }
        (record->phaseStep)++;
    case 3:
        record->routePosition.x =
            record->phasePosition.x +
            lbl_1_bss_8BC[record->segmentModel].position.x;
        record->routePosition.z =
            record->phasePosition.z +
            lbl_1_bss_8BC[record->segmentModel].position.z;
        switch (record->segmentModel) {
        case 60:
        case 61:
        case 62:
        case 63:
        case 64:
        case 65:
        case 66:
        case 67:
        case 68:
        case 69:
        case 70:
        case 71:
        case 72:
        case 73:
        case 74:
        case 75:
            break;
        default:
            if (record->routePosition.z > record->position.z) {
                record->routePosition.z = record->position.z;
            }
        }
        changed = fn_1_126F0(record, 1);
        if ((s16)changed) {
            record->phaseStep = 0;
            result = 1;
        }
        break;
    }
    return result;
}

/* Chooses a route arrival offset when the player's next travel target is prepared. */
void fn_1_159D4(M609Player *record) {
    record->arrivalOffsetX = (0.0f);
    record->arrivalOffsetZ = (0.0f);

    if (record->targetSegment != 50) {
        switch (record->segmentModel) {
    case 51:
        record->arrivalOffsetX = (200.0f);
        break;
    case 52:
        record->arrivalOffsetX = (-200.0f);
        break;
    case 60:
    case 61:
    case 64:
    case 65:
        record->arrivalOffsetX = record->position.x;
        if (record->arrivalOffsetX > (200.0f)) {
            record->arrivalOffsetX = (200.0f);
        } else if (record->arrivalOffsetX < (-200.0f)) {
            record->arrivalOffsetX = (-200.0f);
        }
        record->arrivalOffsetZ = (150.0f);
        break;
    case 78:
        if (record->position.x < (125.0f)) {
            record->arrivalOffsetX = (-200.0f);
            record->arrivalOffsetZ = (50.0f);
        } else {
            record->arrivalOffsetX = (200.0f);
            record->arrivalOffsetZ = (0.0f);
        }
        break;
    case 49:
        if (record->position.x < (0.0f)) {
            record->arrivalOffsetX = (-200.0f);
        } else {
            record->arrivalOffsetX = (200.0f);
        }
        break;
    case 77:
        record->arrivalOffsetZ = (100.0f);
        break;
    case 57:
        if (record->position.x < (0.0f)) {
            record->arrivalOffsetX = (-200.0f);
        } else {
            record->arrivalOffsetX = (200.0f);
        }
        break;
        }
    } else {
        record->arrivalOffsetX = record->position.x;
        if (record->arrivalOffsetX > (380.0f)) {
            record->arrivalOffsetX = (380.0f);
        } else if (record->arrivalOffsetX < (-380.0f)) {
            record->arrivalOffsetX = (-380.0f);
        }
        record->arrivalOffsetZ = (150.0f);
    }
}

/* Advances route-travel phases when called by the unused fn_1_164FC entry. */
void fn_1_15C24(M609Player *record) {
    HuVecF difference;
    f32 distance;
    s16 changed = 0;

    switch (record->travelState) {
    case 0: {
        f32 distanceSquared;
        changed = fn_1_1507C(record);
        if ((s16)changed != 0) {
            (record->travelState)++;
            return;
        }

        fn_1_12CDC(record);
        record->targetSegment =
            record->candidateSegments[0];
        fn_1_12FFC(record);
        fn_1_159D4(record);

        record->target.x =
            record->origin.x +
            record->arrivalOffsetX;
        record->target.y = (50.0f);
        record->target.z =
            record->origin.z +
            record->arrivalOffsetZ;

        difference.x = record->target.x -
            record->position.x;
        difference.z = record->target.z -
            record->position.z;
        distanceSquared = difference.x * difference.x +
            difference.z * difference.z;
        distance = (f32)sqrt((f64)distanceSquared);
        record->range = (380.0f);
        if (record->target.z > (-1000.0f) &&
            distance < record->range) {
            record->phaseStep = 0;
            record->secondLegRequired = 0;
            MgPlayerComStkOff(lbl_1_bss_4.players[record->playerNo]);
            record->movementState = 0;
            record->routePhase = 0;
            record->chasePosition =
                record->target;
            record->travelState = 2;
        }
        break;
    }
    case 1: {
        f32 distanceSquared;
        fn_1_12CDC(record);
        record->targetSegment =
            record->candidateSegments[0];
        fn_1_12FFC(record);
        fn_1_159D4(record);

        record->target.x =
            record->origin.x +
            record->arrivalOffsetX;
        record->target.y = (50.0f);
        record->target.z =
            record->origin.z +
            record->arrivalOffsetZ;

        difference.x = record->target.x -
            record->position.x;
        difference.z = record->target.z -
            record->position.z;
        distanceSquared = difference.x * difference.x +
            difference.z * difference.z;
        distance = (f32)sqrt((f64)distanceSquared);
        record->range = (400.0f);

        if (record->target.z > (-800.0f) &&
            distance < record->range) {
            record->phaseStep = 0;
            record->secondLegRequired = 0;
            MgPlayerComStkOff(lbl_1_bss_4.players[record->playerNo]);
            record->movementState = 0;
            record->routePhase = 0;
            record->chasePosition =
                record->target;
            (record->travelState)++;
        } else {
            switch (record->segmentModel) {
            case 60:
            case 61:
            case 62:
            case 63:
            case 64:
            case 65:
            case 66:
            case 67:
            case 68:
            case 69:
            case 70:
            case 71:
            case 72:
            case 73:
            case 74:
            case 75:
                fn_1_1507C(record);
                break;
            }
        }
        break;
    }
    case 2: {
        switch (record->routePhase) {
        case 0:
            changed = fn_1_12990(record);
            if ((s16)changed != 0) {
                if (record->targetSegment !=
                    record->segmentModel) {
                    (record->routePhase)++;
                } else {
                    record->travelState = 0;
                    record->routePhase = 0;
                }
            }
            break;
        case 1:
            record->routePosition.x =
                record->arrivalOffsetX +
                lbl_1_bss_8BC[record->segmentModel].position.x;
            record->routePosition.z =
                record->arrivalOffsetZ +
                lbl_1_bss_8BC[record->segmentModel].position.z;
            changed = fn_1_126F0(record, 1);
            if ((s16)changed != 0) {
                record->travelState = 0;
                record->routePhase = 0;
            }
            break;
        }
        break;
    }
    }
}

/* Unused travel entry: forces mode-0 players to computer control on their first update,
 * then advances their route travel. */
void fn_1_164FC(void) {
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (lbl_1_bss_24C[playerNo].mode == 0) {
            M609Player *record = &lbl_1_bss_24C[playerNo];

            record->computerButtonsHeld = 0;
            record->computerButtonsDown = 0;
            record->computerStickX = 0;
            record->computerStickY = 0;

            switch (record->sequencePhase) {
            case 0:
                MgPlayerComStkOff(lbl_1_bss_4.players[record->playerNo]);
                record->movementState = 0;
                record->routeStep = 0;
                record->routePhase = 0;
                record->disableInput = 0;
                record->participantType = 1;
                record->sequencePhase++;
                /* fall through */
            case 1:
                if (record->segmentModel >= 46) {
                    fn_1_15C24(record);
                }
                break;
            }
        }
    }
}

s16 lbl_1_bss_6E50[12];
Vec *lbl_1_bss_6E28[10];
