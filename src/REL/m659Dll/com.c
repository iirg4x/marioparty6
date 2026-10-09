/* Computer-player route planning and lane control for Asteroad Rage. */
#include "REL/m659/com.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "string.h"

OMOBJ *lbl_1_bss_60;
int lbl_1_data_1D8[4] = {5, 3, 2, 2};
void fn_1_59E0(OMOBJ *obj);
void fn_1_5A14(OMOBJ *obj);
void fn_1_5B58(OMOBJ *obj);
void fn_1_5CAC(OMOBJ *obj);
int fn_1_5DB4(M659Com *com);
void fn_1_5E08(M659Com *com);
void fn_1_5E9C(M659Com *com);
void fn_1_64B0(M659Com *com);
#include "REL/m659/com.h"
#include "game/memory.h"
#include "REL/m659/objects.h"
#include "REL/m659/player.h"

extern OMOBJ *lbl_1_bss_60;
#include "REL/m659/com.h"
#include "REL/m659/objects.h"
#include "game/main.h"

extern int lbl_1_data_1D8[4];
int abs(int absoluteInput);
double fn_1_4ABC(HuVecF a, HuVecF b);

/* Creates the computer-player manager during minigame startup. */
void fn_1_5944(OMOBJMAN *manager)
{
    M659ComManager *managerData;
    OMOBJ *obj;
    obj = lbl_1_bss_60 = omAddObjEx(manager, 70, 0, 0, -1, fn_1_59E0);
    obj->stat |= 0x100;
    managerData = obj->data =
        HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659ComManager), HU_MEMNUM_OVL);
    memset(managerData, 0, sizeof(M659ComManager));
}

void fn_1_59DC(void)
{
}

/* Manager startup callback that initializes phase state and installs the setup callback. */
void fn_1_59E0(OMOBJ *obj)
{
    M659ComManager *managerData = obj->data;
    managerData->state = 0;
    managerData->frame = 0;
    obj->objFunc = fn_1_5A14;
}

/* Manager callback that waits for course setup, then plans routes for computer players. */
void fn_1_5A14(OMOBJ *obj)
{
    M659ComManager *managerData = obj->data;
    M659Com **playerSlot;
    int i;
    switch (managerData->state) {
    case 0:
        if (MgSeqModeGet() >= 2) {
            managerData->ready = 0;
            managerData->state++;
        }
        break;
    case 1:
        managerData->state++;
        break;
    case 2:
        managerData->state++;
        break;
    case 3:
        managerData->state++;
        break;
    case 4:
        managerData->state++;
        break;
    case 5:
        managerData->state++;
        break;
    case 6:
        managerData->ready = 1;
        obj->objFunc = fn_1_5B58;
        managerData->state = 0;
        managerData->frame = 0;
        playerSlot = NULL;
        for (i = 0; i < 2; i++) {
            playerSlot = &managerData->players[i];
            if ((*playerSlot)->isCom) {
                fn_1_5E9C(*playerSlot);
            }
        }
        break;
    }
    managerData->frame++;
}

/* Manager callback that issues computer lane changes and advances their request states. */
void fn_1_5B58(OMOBJ *obj)
{
    M659ComManager *managerData = obj->data;
    M659Com **playerSlot = NULL;
    int i = 0;
    switch (managerData->state) {
    case 0:
        if (MgSeqModeGet() == 5) {
            managerData->state++;
        }
        managerData->frame = 0;
        break;
    case 1:
        for (i = 0; i < 2; i++) {
            playerSlot = &managerData->players[i];
            if ((*playerSlot)->isCom && fn_1_5DB4(*playerSlot)) {
                fn_1_64B0(*playerSlot);
            }
        }
        managerData->state++;
        break;
    case 2:
        for (i = 0; i < 2; i++) {
            playerSlot = &managerData->players[i];
            if ((*playerSlot)->isCom) {
                fn_1_5E08(*playerSlot);
                if (fn_1_5DB4(*playerSlot)) {
                    managerData->state = 1;
                }
            }
        }
        break;
    }
    managerData->frame++;
}

/* Switches the manager placeholder object to its follow-up callback. */
void fn_1_5C9C(OMOBJ *obj)
{
    obj->objFunc = fn_1_5CAC;
}

void fn_1_5CAC(OMOBJ *obj)
{
}

/* Registers one player object with the computer-player manager during player setup. */
void fn_1_5CB0(OMOBJ *obj)
{
    M659Player *playerData = obj->data;
    M659ComManager *managerData = lbl_1_bss_60->data;
    M659Com **playerSlot = &managerData->players[playerData->info.side];
    if (managerData->state == 0) {
        if (*playerSlot != NULL) {
            /* A second registration logs an error, then replaces the existing player slot. */
            OSReport("Error: M659ComPlayerSet() - !*com != NULL\n");
        }
        *playerSlot = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M659Com), HU_MEMNUM_OVL);
        playerData->com.routeIndex = -1;
        if (*playerSlot) {
            (*playerSlot)->obj = obj;
            (*playerSlot)->isCom = playerData->com.isCom;
        } else {
            OSReport("Error: M659ComPlayerSet() - !omMalloc()\n");
        }
    }
}

/* Returns the manager ready flag. */
int fn_1_5D8C(void)
{
    M659ComManager *managerData = lbl_1_bss_60->data;
    return managerData->ready;
}

/* Returns whether the selected computer player has completed its current request. */
int fn_1_5DB4(M659ComSlot *slot)
{
    void *managerData = lbl_1_bss_60->data; /* This manager-data read does not affect the
                                             * player-state completion check. */
    if (slot->isCom) {
        M659Player *playerData = slot->obj->data;
        if (playerData->com.active == 0) {
            return 1;
        }
    }
    return 0;
}

/* Advances the computer player's lane-change request sequence and clears it at its final step. */
void fn_1_5E08(M659ComSlot *slot)
{
    M659Player *playerData = slot->obj->data;
    switch (playerData->com.state) {
    case 0:
        playerData->com.state++;
        /* fallthrough */
    case 1:
        playerData->com.state++;
        break;
    case 2:
        playerData->com.state++;
        /* fallthrough */
    case 3:
        playerData->com.state++;
        break;
    case 4:
        playerData->com.active = 0;
        playerData->com.state = 0;
        break;
    }
}

/* Plans a lane route across all obstacle rows for a computer player. */
void fn_1_5E9C(M659ComSlot *slot)
{
    M659Player *playerData = slot->obj->data;
    M659ComRecord *computerData = &playerData->com;
    M659OMData *obstacleCourse = lbl_1_bss_4C->data;
    /* This row lookup does not affect route selection. */
    M659OMRow *currentPlayerRow = &obstacleCourse->rows[obstacleCourse->playerRow];
    int previousColumn = 3;
    HuVecF playerStart = { 0.0f, 0.0f, 5950.0f };
    if (computerData->isCom) {
        s16 rowIndex;
        for (rowIndex = 0; rowIndex < 22; rowIndex++) {
            M659OMRow *rowData = &obstacleCourse->rows[rowIndex];
            M659ComRoute *routePlan = &computerData->routes[rowIndex];
            int unusedRouteValue = 0; /* Initialized but not used in route selection. */
            int openLaneCount = 0;
            double candidateDistances[3];
            int candidateColumns[3];
            HuVecF candidatePositions[3];
            s16 lane, requestDelayFrames;
            int laneSpread;
            double bestDistance;
            int selectedCandidate, lowerLane, upperLane, candidateIndex;
            routePlan->distance = 10000.0;
            candidateDistances[0] = 100000.0;
            candidateDistances[1] = 100000.0;
            candidateDistances[2] = 100000.0;
            candidateColumns[0] = 20;
            candidateColumns[1] = 20;
            candidateColumns[2] = 20;
            for (lane = 0; lane < 7; lane++) {
                HuVecF *precedingPosition = NULL;
                HuVecF lanePosition = { 0.0f, 0.0f, 0.0f };
                lanePosition.x = lbl_1_data_70[lane];
                lanePosition.z = 9000.0f + 3050.0f * rowIndex;
                if (rowData->slots[lane] == NULL) {
                    if (rowIndex == 0) {
                        precedingPosition = &playerStart;
                    } else {
                        precedingPosition = &computerData->routes[rowIndex - 1].position;
                    }
                    bestDistance = fn_1_4ABC(lanePosition, *precedingPosition);
                    candidateDistances[openLaneCount] = bestDistance;
                    candidateColumns[openLaneCount] = lane;
                    candidatePositions[openLaneCount] = lanePosition;
                    openLaneCount++;
                }
            }
            switch (computerData->difficulty) {
            case 0:
                laneSpread = 3;
                requestDelayFrames = rand8() % lbl_1_data_1D8[0] + 1;
                break;
            case 1:
                laneSpread = 2;
                requestDelayFrames = rand8() % lbl_1_data_1D8[1] + 1;
                break;
            case 2:
                laneSpread = 1;
                requestDelayFrames = rand8() % lbl_1_data_1D8[2];
                break;
            case 3:
                laneSpread = 1;
                requestDelayFrames = rand8() % lbl_1_data_1D8[3];
                break;
            }
            bestDistance = 10000.0;
            selectedCandidate = rand8() % openLaneCount;
            lowerLane = previousColumn - laneSpread;
            upperLane = previousColumn + laneSpread;
            if (lowerLane < 0) {
                lowerLane = 0;
            }
            if (upperLane > 7) {
                upperLane = 7;
            }
            /* This bound check reads one past the last candidate when all three lanes are open. */
            if (lowerLane <= candidateColumns[0] && candidateColumns[openLaneCount] <= upperLane) {
                requestDelayFrames = rand8() % 10 + 1;
            } else {
                /* Difficulties 0-2 accept closer candidates when rand8() % 100 is below 50, 70, and
                 * 80, respectively; difficulty 3 always selects a closer candidate. */
                for (candidateIndex = 0; candidateIndex < openLaneCount; candidateIndex++) {
                    switch (computerData->difficulty) {
                    case 0:
                        if (candidateDistances[candidateIndex] < bestDistance &&
                            rand8() % 100 < 50) {
                            selectedCandidate = candidateIndex;
                            bestDistance = candidateDistances[candidateIndex];
                        }
                        break;
                    case 1:
                        if (candidateDistances[candidateIndex] < bestDistance &&
                            rand8() % 100 < 70) {
                            selectedCandidate = candidateIndex;
                            bestDistance = candidateDistances[candidateIndex];
                        }
                        break;
                    case 2:
                        if (candidateDistances[candidateIndex] < bestDistance &&
                            rand8() % 100 < 80) {
                            selectedCandidate = candidateIndex;
                            bestDistance = candidateDistances[candidateIndex];
                        }
                        break;
                    case 3:
                        if (candidateDistances[candidateIndex] < bestDistance) {
                            selectedCandidate = candidateIndex;
                            bestDistance = candidateDistances[candidateIndex];
                        }
                        break;
                    }
                }
            }
            routePlan->distance = candidateDistances[selectedCandidate];
            routePlan->column = candidateColumns[selectedCandidate];
            routePlan->position = candidatePositions[selectedCandidate];
            if (abs(previousColumn - candidateColumns[selectedCandidate]) <= 1) {
                /* A route within one lane replaces the prior delay with a random 1-10 frame
                 * delay. */
                requestDelayFrames = rand8() % 10 + 1;
            }
            routePlan->delay = requestDelayFrames;
            previousColumn = candidateColumns[selectedCandidate];
        }
    }
}

/* Chooses a lane change when the computer player reaches a planned obstacle row. */
void fn_1_64B0(M659ComSlot *slot)
{
    M659ComManager *managerData = lbl_1_bss_60->data;
    M659Player *playerData = slot->obj->data;
    M659Motion *motion = &playerData->motion;
    M659OMData *obstacleCourse = lbl_1_bss_4C->data;
    int unusedZeroA = 0, unusedZeroB = 0, unusedZeroC = 0; /* Initialized to zero but not read by
                                                            * lane selection. */
    M659ComSlot **playerSlot = &managerData->players[playerData->info.side];
    s16 difficulty = playerData->com.difficulty; /* Captured here but not used by lane selection. */
    float ignoredDistance = 1500.0f; /* Initialized but not used by lane selection. */
    s16 obstacleRow = fn_1_33B0();
    if (obstacleRow != playerData->com.routeIndex) {
        (*playerSlot)->countdown = playerData->com.routes[obstacleRow].delay;
        playerData->com.routeIndex = obstacleRow;
    }
    if (obstacleRow < 22) {
        if ((*playerSlot)->countdown != 0) {
            (*playerSlot)->countdown--;
            return;
        }
        if (motion->state == 0) {
            if (playerData->com.routes[obstacleRow].column > motion->column) {
                /* Only request a lane change when the adjacent lane in the preceding row is empty
                 * or its obstacle has passed the player. */
                if (obstacleRow == 0 ||
                    obstacleCourse->rows[obstacleRow - 1].slots[motion->column + 1] == NULL) {
                    fn_1_6714(slot, 1);
                    return;
                }
                if (obstacleCourse->rows[obstacleRow - 1].slots[motion->column + 1]->position.z <
                    0.0f) {
                    fn_1_6714(slot, 1);
                    return;
                }
            } else if (playerData->com.routes[obstacleRow].column < motion->column) {
                if (obstacleRow == 0 ||
                    obstacleCourse->rows[obstacleRow - 1].slots[motion->column - 1] == NULL) {
                    fn_1_6714(slot, 0);
                    return;
                }
                if (obstacleCourse->rows[obstacleRow - 1].slots[motion->column - 1]->position.z <
                    0.0f) {
                    fn_1_6714(slot, 0);
                }
            }
        }
    }
}

/* Starts a computer lane-change request in the requested direction. */
void fn_1_6714(M659ComSlot *slot, s16 direction)
{
    M659Player *playerData = slot->obj->data;
    playerData->com.direction = direction;
    playerData->com.active = 1;
    playerData->com.state = 0;
}
