/* Arena grid generation and CPU movement helpers. */
#define _MATH_H
#include "REL/m632dll.h"

/* Model-hook callback registered by fn_1_4C90; it leaves the supplied model transform unchanged. */
void fn_1_4C8C(HU3D_MODEL *modelP, Mtx *mtx)
{

}

/* Called during fn_1_3934 scene setup; creates one camera-2 hook and initializes CPU timers and gridA weights. */
void fn_1_4C90(void)
{
    s32 index;
    s16 model;

    model = Hu3DHookFuncCreate(fn_1_4C8C);
    Hu3DModelCameraSet(model, 2U);
    index = 0;
    while (index < 4) {
        lbl_1_bss_0.playerTimer[index] = 0;
                lbl_1_bss_0.playerTimer[index] = 0;
        index += 1;
    }
    index = 0;
    while (index < 64) {
        lbl_1_bss_0.gridA[index] = 128;
        index += 1;
    }
}

/* Called by fn_1_535C during gameplay updates to build the 8-by-8 obstacle-weight map. */
void fn_1_4D48(void)
{
    Point3d collisionPosition;
    int collisionIndex;
    int cellX;
    int cellZ;
    int actorCellX;
    int actorCellZ;
    int spreadX;
    int spreadZ;
    int cellIndex;
    u8 *patternData;
    int found; /* Set when a collision actor occupies the current cell. */

    patternData = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    cellIndex = 0;
    while (cellIndex < 64) {
        lbl_1_bss_0.gridB[cellIndex] = 10;
        /* Fill base weights before adding the obstacle map. */
        patternData += 1;
        cellIndex += 1;
    }
    patternData = lbl_1_data_2A8[lbl_1_bss_0.patternIndex];
    cellIndex = 0;
    while (cellIndex < 64) {
        cellX = cellIndex % 8;
        cellZ = cellIndex / 8;
        if (((s8) patternData[0] >= 2) && ((s8) patternData[0] <= 3)) {
            lbl_1_bss_0.gridB[cellIndex] += 255;
        } else {
            found = 0;
            collisionIndex = 0;
            while (collisionIndex < lbl_1_bss_0.collisionCount) {
                collisionPosition = lbl_1_bss_0.collisionActors[collisionIndex]->pos;
                actorCellX = (s32) ((400.0f + collisionPosition.x) / 100.0f);
                actorCellZ = (s32) ((400.0f + collisionPosition.z) / 100.0f);
                if (actorCellX < 0) {
                    actorCellX = 0;
                }
                if (actorCellX >= 8) {
                    actorCellX = 7;
                }
                if (actorCellZ < 0) {
                    actorCellZ = 0;
                }
                if (actorCellZ >= 8) {
                    actorCellZ = 7;
                }
                if ((cellX == actorCellX) && (cellZ == actorCellZ)) {
                    int spreadWeights[121] = {
                        4,4,4,4,8,8,8,8,4,4,4,
                        4,4,4,8,16,16,16,16,8,4,4,
                        4,4,8,16,32,32,32,32,16,4,4,
                        4,8,16,32,64,64,64,32,16,8,4,
                        8,16,32,64,128,128,128,64,32,16,8,
                        8,16,32,64,128,255,128,64,32,16,8,
                        8,16,32,64,128,128,128,64,32,16,8,
                        4,8,16,32,64,64,64,32,16,8,4,
                        4,4,8,16,32,32,32,16,8,4,4,
                        4,4,4,8,16,16,16,8,4,4,4,
                        4,4,4,4,8,8,8,4,4,4,4
                    };
                    spreadZ = -5;
                    while (spreadZ <= 5) {
                        spreadX = -5;
                        while (spreadX <= 5) {
                            if (((s32) (cellX + spreadX) >= 0) && ((s32) (cellX + spreadX) < 8) && ((s32) (cellZ + spreadZ) >= 0) && ((s32) (cellZ + spreadZ) < 8)) {
                                lbl_1_bss_0.gridB[cellX + spreadX + (cellZ + spreadZ) * 8] += spreadWeights[(spreadX + 5) + (spreadZ + 5) * 11];
                            }
                            spreadX += 1;
                        }
                        spreadZ += 1;
                    }
                    found = 1;
                }
                collisionIndex += 1;
            }
        }
        patternData += 1;
        cellIndex += 1;
    }
    cellIndex = 0;
    while (cellIndex < 64) {
        if (lbl_1_bss_0.gridB[cellIndex] < 0) {
            lbl_1_bss_0.gridB[cellIndex] = 0;
        } else if (lbl_1_bss_0.gridB[cellIndex] > 255) {
            lbl_1_bss_0.gridB[cellIndex] = 255;
        }
        cellIndex += 1;
    }
}

/* Called by fn_1_597C for a group-1 CPU player to add nearby group-1 players to gridA and clamp its weights. */
void fn_1_50A0(s32 playerIndex)
{
    int neighborWeights[25] = {
        0,0,16,0,0, 0,16,32,16,0, 16,32,64,32,16,
        0,16,32,16,0, 0,0,16,0,0
    };
    Point3d playerPosition;
    int otherPlayerIndex;
    int deltaX;
    int deltaZ;
    int playerGridX;
    int playerGridZ;
    MGPLAYER *player;

    memcpy(lbl_1_bss_0.gridA, lbl_1_bss_0.gridB, sizeof(lbl_1_bss_0.gridA));
    otherPlayerIndex = 0;
    while (otherPlayerIndex < 4) {
        if ((otherPlayerIndex != playerIndex) && (lbl_1_bss_0.group[otherPlayerIndex] == 1) && (lbl_1_bss_0.playerState[otherPlayerIndex] == 0)) {
            player = lbl_1_bss_0.players[otherPlayerIndex];
            playerPosition = player->actor->pos;
            playerGridX = (s32) ((400.0f + playerPosition.x) / 100.0f);
            playerGridZ = (s32) ((400.0f + playerPosition.z) / 100.0f);
            if (playerGridX < 0) {
                playerGridX = 0;
            }
            if (playerGridX >= 8) {
                playerGridX = 7;
            }
            if (playerGridZ < 0) {
                playerGridZ = 0;
            }
            if (playerGridZ >= 8) {
                playerGridZ = 7;
            }
            deltaZ = -2;
            while (deltaZ <= 2) {
                deltaX = -2;
                while (deltaX <= 2) {
                    if (((s32) (playerGridX + deltaX) >= 0) && ((s32) (playerGridX + deltaX) < 8) && ((s32) (playerGridZ + deltaZ) >= 0) && ((s32) (playerGridZ + deltaZ) < 8)) {
                        lbl_1_bss_0.gridA[playerGridX + deltaX + (playerGridZ + deltaZ) * 8] += neighborWeights[(deltaX + 2) + (deltaZ + 2) * 5];
                    }
                    deltaX += 1;
                }
                deltaZ += 1;
            }
        }
        otherPlayerIndex += 1;
    }
    otherPlayerIndex = 0;
    while (otherPlayerIndex < 64) {
        if ((s32) lbl_1_bss_0.gridA[otherPlayerIndex] < 0) {
            lbl_1_bss_0.gridA[otherPlayerIndex] = 0;
        } else if ((s32) lbl_1_bss_0.gridA[otherPlayerIndex] > 255) {
            lbl_1_bss_0.gridA[otherPlayerIndex] = 255;
        }
        otherPlayerIndex += 1;
    }
}

/* Called by fn_1_17C0 during play to refresh the arena map and update active CPU movement decisions. */
void fn_1_535C(void)
{
    s32 value9;

    fn_1_4D48();
    value9 = 0;
    while (value9 < 4) {
        if ((GwPlayerConf[value9].type != 0) && ((s32) lbl_1_bss_0.playerState[value9] == 0)) {
            if ((s32) lbl_1_bss_0.group[value9] == 0) {
                fn_1_5400(value9);
            } else {
                fn_1_597C(value9);
            }
        }
        value9 += 1;
    }
}

/* Called by fn_1_535C for a group-0 CPU player to choose its free-movement direction. */
void fn_1_5400(s32 playerIndex)
{
    Point3d direction;
    Point3d targetPosition;
    Point3d collisionCenter;
    Point3d currentDirection;
    MGPLAYER *player;
    f32 randomAngle;
    s32 targetPlayerIndex;
    s32 collisionIndex;
    u32 chancePercent;
    MGACTOR *collisionActor;

    player = lbl_1_bss_0.players[playerIndex];
    switch (GwPlayerConf[playerIndex].comDif) {
    case 0:
        chancePercent = 30U;
        break;
    case 1:
        chancePercent = 15U;
        break;
    case 2:
        chancePercent = 5U;
        break;
    case 3:
        chancePercent = 0U;
        break;
    }
    if ((s32) lbl_1_bss_0.playerTimer[playerIndex] < 0) {
        if (frandmod(100) < chancePercent) {
            lbl_1_bss_0.playerAIState[playerIndex] = 0;
            lbl_1_bss_0.playerTimer[playerIndex] = (s32) (frandmod(60) + 30);
        } else {
            lbl_1_bss_0.playerAIState[playerIndex] = 1;
            switch (GwPlayerConf[playerIndex].comDif) {
            case 0:
                chancePercent = 20U;
                break;
            case 1:
                chancePercent = 30U;
                break;
            case 2:
                chancePercent = 55U;
                break;
            case 3:
                chancePercent = 80U;
                break;
            }
            if (frandmod(100) < chancePercent) {
                collisionCenter.x = collisionCenter.y = collisionCenter.z = 0.0f;
                collisionIndex = 0;
                while (collisionIndex < (s32) lbl_1_bss_0.collisionCount) {
                    collisionActor = lbl_1_bss_0.collisionActors[collisionIndex];
                    collisionCenter.x += collisionActor->pos.x;
                    collisionCenter.z += collisionActor->pos.z;
                    collisionIndex += 1;
                }
                collisionCenter.x /= (f32) lbl_1_bss_0.collisionCount;
                collisionCenter.z /= (f32) lbl_1_bss_0.collisionCount;
                do {
                    targetPlayerIndex = frandmod(4);
                } while (lbl_1_bss_0.group[targetPlayerIndex] != 1 || lbl_1_bss_0.playerState[targetPlayerIndex] != 0);
                {
                    MGPLAYER *selectedPlayer = lbl_1_bss_0.players[targetPlayerIndex];
                    targetPosition = selectedPlayer->actor->pos;
                }
                PSVECSubtract(&targetPosition, &collisionCenter, &direction);
                direction.z = -direction.z;
                if (PSVECMag(&direction) >= 0.01f) {
                    PSVECNormalize(&direction, &direction);
                }
                lbl_1_bss_0.playerDirection[playerIndex] = direction;
                lbl_1_bss_0.playerTimer[playerIndex] = (s32) (frandmod(20) + 20);
            } else {
                randomAngle = (f32) (u32) frandmod(360);
                direction.x = (f32) (cos((3.141592653589793 * (f64) randomAngle) / 180.0) - sin((3.141592653589793 * (f64) randomAngle) / 180.0));
                direction.y = 0.0f;
                direction.z = (f32) (sin((3.141592653589793 * (f64) randomAngle) / 180.0) + cos((3.141592653589793 * (f64) randomAngle) / 180.0));
                PSVECNormalize(&direction, &direction);
                lbl_1_bss_0.playerDirection[playerIndex] = direction;
                lbl_1_bss_0.playerTimer[playerIndex] = (s32) (frandmod(60) + 30);
            }
        }
    }
    switch (lbl_1_bss_0.playerAIState[playerIndex]) {
    case 0:
        break;
    case 1:
        currentDirection = lbl_1_bss_0.playerDirection[playerIndex];
        lbl_1_bss_0.cpuStickX = 28.0f * currentDirection.x;
        lbl_1_bss_0.cpuStickZ = 28.0f * currentDirection.z;
        break;
    }
    lbl_1_bss_0.playerTimer[playerIndex]--;
}

/* Called by fn_1_535C for a group-1 CPU player to choose grid movement and write its pad input. */
void fn_1_597C(s32 playerIndex)
{
    Point3d pos;
    Point3d direction;
    MGPLAYER *player;
    int buttonDownMask;
    int buttonMask;
    int neighborHeat;
    int gridX;
    int gridZ;

    player = lbl_1_bss_0.players[playerIndex];
    pos = player->actor->pos;
    gridX = (s32) ((400.0f + pos.x) / 100.0f);
    gridZ = (s32) ((400.0f + pos.z) / 100.0f);
    if (gridX < 0) {
        gridX = 0;
    }
    if (gridX >= 8) {
        gridX = 7;
    }
    if (gridZ < 0) {
        gridZ = 0;
    }
    if (gridZ >= 8) {
        gridZ = 7;
    }
    fn_1_50A0(playerIndex);
    switch (lbl_1_bss_0.playerAIState[playerIndex]) {
    case 0:
        if ((s32) lbl_1_bss_0.gridA[gridX + (gridZ * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[playerIndex].comDif]) {
            lbl_1_bss_0.playerAIState[playerIndex] = 2;
            fn_1_5D34(playerIndex);
        }
        break;
    case 2:
        buttonDownMask = 0;
        buttonMask = 0;
        if ((s32) lbl_1_bss_0.gridA[gridX + (gridZ * 8)] >= (s32) lbl_1_data_198[GwPlayerConf[playerIndex].comDif]) {
            fn_1_5D34(playerIndex);
        } else {
            neighborHeat = 0;
            if ((s32) (gridX - 1) >= 0) {
                neighborHeat += lbl_1_bss_0.gridA[(gridX - 1) + (gridZ * 8)];
            }
            if ((s32) (gridX + 1) < 8) {
                neighborHeat += lbl_1_bss_0.gridA[(gridX + 1) + (gridZ * 8)];
            }
            if ((s32) (gridZ - 1) >= 0) {
                neighborHeat += lbl_1_bss_0.gridA[gridX + ((gridZ - 1) * 8)];
            }
            if ((s32) (gridZ + 1) < 8) {
                neighborHeat += lbl_1_bss_0.gridA[gridX + ((gridZ + 1) * 8)];
            }
            if (neighborHeat < (s32) lbl_1_data_198[GwPlayerConf[playerIndex].comDif]) {
                lbl_1_bss_0.playerAIState[playerIndex] = 0;
                return;
            }
        }
        direction = lbl_1_bss_0.playerDirection[playerIndex];
        if ((s32) lbl_1_bss_0.gridA[gridX + (gridZ * 8)] >= 144) {
            buttonDownMask = PAD_BUTTON_A;
            buttonMask = PAD_BUTTON_A;
        }
        MgPlayerPadSet(player, (s32) (56.0f * direction.x), (s32) (56.0f * -direction.z), buttonDownMask, buttonMask);
        break;
    case 1:
        break;
    }
}

/* Called by fn_1_597C when the risk threshold is reached to update that player's grid direction. */
void fn_1_5D34(s32 playerIndex)
{
    Point3d pos;
    Point3d direction;
    s32 targetGridX;
    s32 targetGridZ;
    f32 riskThreshold;
    s32 currentGridX;
    s32 currentGridZ;
    MGPLAYER *player;

    player = lbl_1_bss_0.players[playerIndex];
    pos = player->actor->pos;
    currentGridX = (s32) ((400.0f + pos.x) / 100.0f);
    currentGridZ = (s32) ((400.0f + pos.z) / 100.0f);
    if (currentGridX < 0) {
        currentGridX = 0;
    }
    if (currentGridX >= 8) {
        currentGridX = 7;
    }
    if (currentGridZ < 0) {
        currentGridZ = 0;
    }
    if (currentGridZ >= 8) {
        currentGridZ = 7;
    }
    switch (GwPlayerConf[playerIndex].comDif) {
    case 0:
        riskThreshold = 128.0f;
        break;
    case 1:
        riskThreshold = 80.0f;
        break;
    case 2:
        riskThreshold = 40.0f;
        break;
    case 3:
        riskThreshold = 16.0f;
        break;
    }
    if ((fn_1_604C(playerIndex, currentGridX, currentGridZ, &targetGridX, &targetGridZ) != 0) && ((currentGridX != targetGridX) || (currentGridZ != targetGridZ)) && ((f32) lbl_1_bss_0.gridA[currentGridX + currentGridZ * 8] >= riskThreshold)) {
        direction.x = (f32) (targetGridX - currentGridX);
        direction.y = 0.0f;
        direction.z = (f32) (targetGridZ - currentGridZ);
        PSVECNormalize(&direction, &direction);
        lbl_1_bss_0.playerDirection[playerIndex] = direction;
    }
}

/* Comparator passed to qsort in fn_1_604C; orders candidates by their distance-weighted cost. */
int fn_1_5FE4(const void *a, const void *b)
{
    const M632GridCandidate *candidateA = a;
    const M632GridCandidate *candidateB = b;
    if ((candidateA->distance == candidateB->distance) && (candidateA->cost == candidateB->cost)) {
        return 0;
    }
    if (candidateA->cost < candidateB->cost) {
        return -1;
    }
    return 1;
}

/* Called by fn_1_5D34 to rank the 8-by-8 cells, compare candidates with active group-1 player positions, and report whether selection changed. */
s32 fn_1_604C(s32 playerIndex, s32 startX, s32 startZ, s32 *selectedX, s32 *selectedZ)
{
    M632GridCandidate candidates[64];
    Point3d otherPlayerPosition;
    Point3d candidateOffset;
    M632GridCandidate *candidate;
    f32 distanceWeight;
    s32 selectionChanged;
    s32 activeGroupPlayerCount;
    s32 candidateAvailable;
    s32 otherPlayerIndex;
    s32 gridCellX;
    s32 gridCellZ;
    s32 candidateIndex;

    candidate = &candidates[0];
    if (startX < 0) {
        startX = 0;
    }
    if (startX >= 8) {
        startX = 7;
    }
    if (startZ < 0) {
        startZ = 0;
    }
    if (startZ >= 8) {
        startZ = 7;
    }
    gridCellZ = 0;
    while (gridCellZ < 8) {
        gridCellX = 0;
        while (gridCellX < 8) {
            candidateOffset.x = (f32) (gridCellX - startX);
            candidateOffset.y = 0.0f;
            candidateOffset.z = (f32) (gridCellZ - startZ);
            candidate->x = gridCellX;
            candidate->z = gridCellZ;
            candidate->distance = PSVECMag(&candidateOffset);
            distanceWeight = candidate->distance;
            if (distanceWeight < 1.0f) {
                distanceWeight = 1.0f;
            }
            candidate->cost = (s32) ((f32) lbl_1_bss_0.gridA[gridCellX + gridCellZ * 8] * distanceWeight);
            candidate += 1;
            gridCellX += 1;
        }
        gridCellZ += 1;
    }
    fn_1_6574(&candidates[0], 64U, sizeof(candidates[0]), fn_1_5FE4);
    candidate = &candidates[0];
    candidateAvailable = 0;
    activeGroupPlayerCount = 0;
    candidateIndex = 0;
    while (candidateIndex < 4) {
        if ((lbl_1_bss_0.group[candidateIndex] == 1) && (lbl_1_bss_0.playerState[candidateIndex] == 0)) {
            activeGroupPlayerCount += 1;
        }
        candidateIndex += 1;
    }
    candidateIndex = 0;
    while (candidateIndex < 64) {
        if (activeGroupPlayerCount > 1) {
            otherPlayerIndex = 0;
            while (otherPlayerIndex < 4) {
                if ((lbl_1_bss_0.group[otherPlayerIndex] == 1) && (lbl_1_bss_0.playerState[otherPlayerIndex] == 0) && (otherPlayerIndex != playerIndex)) {
                    otherPlayerPosition = lbl_1_bss_0.players[otherPlayerIndex]->actor->pos;
                    gridCellX = (s32) ((400.0f + otherPlayerPosition.x) / 100.0f);
                    gridCellZ = (s32) ((400.0f + otherPlayerPosition.z) / 100.0f);
                    if ((gridCellX != candidate->x) || (gridCellZ != candidate->z)) {
                        candidateAvailable = 1;
                        break;
                    }
                }
                otherPlayerIndex += 1;
            }
        } else {
            candidateAvailable = 1;
        }
        if (candidateAvailable != 0) {
            *selectedX = candidate->x;
            *selectedZ = candidate->z;
            break;
        }
        candidate += 1;
        candidateIndex += 1;
    }
    selectionChanged = 1;
    if ((*selectedX == startX) && (*selectedZ == startZ)) {
        selectionChanged = 0;
    }
    return selectionChanged;
}
