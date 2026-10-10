/* CPU target selection, jumping and teammate avoidance for Pixel Perfect. */
#include "dolphin/math.h"
#include "REL/m636dll.h"

#define PIXEL_CELL_COUNT 16
#define PIXEL_PLAYER_SIDE_WORD_INDEX 26
#define PIXEL_PLAYER_SIDE_OFFSET 104
#define PIXEL_PLAYER_RECORD_OFFSET 120
#define PIXEL_CPU_MODE_OFFSET 156
#define PIXEL_CPU_FRAME_COUNTER_OFFSET 172
#define PIXEL_CPU_TARGET_CELL_OFFSET 188
#define PIXEL_CPU_STICK_DIRECTION_OFFSET 204

#define PIXEL_CPU_PAUSE 0
#define PIXEL_CPU_CHOOSE 1
#define PIXEL_CPU_JUMP 2
#define PIXEL_CPU_GROUND_POUND 3
#define PIXEL_CPU_MOVE 4

/* CPU input updates use this when choosing a random target during the play sequence. */
s32 fn_1_3D28(s32 playerIndex)
{
    s32 cellIndex;
    do {
        cellIndex = frandmod(PIXEL_CELL_COUNT);
    } while (cellIndex ==
             *(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_CPU_TARGET_CELL_OFFSET + playerIndex * 4));
    return cellIndex;
}

/* CPU input updates use this to choose an incorrect pixel; it returns -1 when the floor matches. */
s32 fn_1_3D80(s32 playerSlot)
{
    s32 mismatchIndices[16];
    s32 cellIndex;
    s32 mismatchCount;
    s32 side;
    side = ((s32 *)((u8 *)&lbl_1_bss_0 + playerSlot * 4))[PIXEL_PLAYER_SIDE_WORD_INDEX];
    mismatchCount = 0;
    cellIndex = 0;
    for (; cellIndex < 16; cellIndex++) {
        if (fn_1_3A84(side, cellIndex) != 1) {
            mismatchIndices[mismatchCount] = cellIndex;
            mismatchCount++;
        }
    }
    if (mismatchCount == 0) {
        return -1;
    }
    return mismatchIndices[frandmod(mismatchCount)];
}

/* Normal and very hard CPU updates choose a nearby incorrect pixel; teammate targets are avoided
 * only when this CPU already has a target and at least two pixels remain wrong. */
s32 fn_1_3E30(s32 playerIndex){
    Point3d gridPosition;
    Point3d cellCenter;
    Point3d difference;
    f32 distance;
    f32 nearestDistance;
    f32 offsetX;
    f32 offsetZ;
    MGPLAYER *player;
    s32 nearestCell;
    s32 side;
    s32 teammateIndex;
    s32 cellIndex;

    nearestDistance = (99999.0f);
    player = lbl_1_bss_0.players[playerIndex];
    side = lbl_1_bss_0.teamSide[playerIndex];
    gridPosition = player->actor->pos;
    nearestCell = -1;
    offsetX = lbl_1_data_270[side][0];
    offsetZ = lbl_1_data_270[side][1];
    gridPosition.x += offsetX;
    gridPosition.y = (0.0f);
    gridPosition.z += offsetZ;
    for (cellIndex = 0; cellIndex < PIXEL_CELL_COUNT; cellIndex++) {
        if (fn_1_3A84(side, cellIndex) == 0) {
            /* Skip a teammate's target only if this CPU already has one and at least two pixels
             * are wrong. */
            for (teammateIndex = 0; teammateIndex < 4; teammateIndex++) {
                if (teammateIndex != playerIndex &&
                    lbl_1_bss_0.teamSide[teammateIndex] == side &&
                    lbl_1_bss_0.targetCell[playerIndex] != -1 &&
                    lbl_1_bss_0.targetCell[teammateIndex] == cellIndex &&
                    fn_1_39FC(side) >= 2) {
                    break;
                }
            }
            if (teammateIndex == 4) {
                cellCenter.x = (100.0f) + (200.0f) * (cellIndex % 4);
                cellCenter.y = (0.0f);
                cellCenter.z = (100.0f) + (200.0f) * (cellIndex / 4);
                PSVECSubtract(&cellCenter, &gridPosition, &difference);
                distance = PSVECMag(&difference);
                if (distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestCell = cellIndex;
                }
            }
        }
    }
    return nearestCell;
}

/* The CPU difficulty dispatcher runs this each active play frame for easy CPUs; they use random
 * targets, pauses and sometimes skip a jump. */
void fn_1_40E0(s32 playerIndex)
{
    Point3d gridPosition;
    Point3d cellCenter;
    Point3d difference;
    Point3d direction;
    f32 offsetX;
    f32 offsetZ;
    MGPLAYER *player;
    s32 stickX;
    s32 stickY;
    s32 pressedButtons;
    s32 heldButtons;

    player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    gridPosition = player->actor->pos;
    stickX = 0;
    stickY = 0;
    pressedButtons = 0;
    heldButtons = 0;
    offsetX =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [0];
    offsetZ =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [1];
    gridPosition.x += offsetX;
    gridPosition.y = (0.0f);
    gridPosition.z += offsetZ;
    switch (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET)) {
    case PIXEL_CPU_CHOOSE: {
        s32 chosenCell;
        {
            s32 choiceRoll = frandmod(100);
            if (choiceRoll < 5) {
                chosenCell = fn_1_3D80(playerIndex);
            } else if (choiceRoll < 15) {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_PAUSE;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) =
                    frandmod(120) + 60;
                break;
            } else {
                chosenCell = fn_1_3D28(playerIndex);
            }
        }
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET) = chosenCell;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) = PIXEL_CPU_MOVE;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        break;
    }
    case PIXEL_CPU_MOVE: {
        s32 targetCell;
        f32 distance;
        targetCell = *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET);
        cellCenter.x = (100.0f) + (200.0f) * (targetCell % 4);
        cellCenter.y = (0.0f);
        cellCenter.z = (100.0f) + (200.0f) * (targetCell / 4);
        PSVECSubtract(&cellCenter, &gridPosition, &difference);
        distance = PSVECMag(&difference);
        if (distance < (100.0f)) {
            if ((u32) frandmod(100) < 60 &&
                fn_1_3A84(
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET),
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                              PIXEL_CPU_TARGET_CELL_OFFSET)) == 0) {
                pressedButtons = PAD_BUTTON_A;
                heldButtons = PAD_BUTTON_A;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_JUMP;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to jump\n");
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_CHOOSE;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            }
        } else {
            f32 directionLength;
            PSVECSubtract(&cellCenter, &gridPosition, &direction);
            directionLength = PSVECMag(&direction);
            if ((0.0f) != directionLength) {
                direction.x /= directionLength;
                direction.y /= directionLength;
                direction.z /= directionLength;
            }
            gridPosition.x += (56.0f) * direction.x;
            gridPosition.y += (56.0f) * direction.y;
            gridPosition.z += (56.0f) * direction.z;
            fn_1_5F54(playerIndex, &gridPosition, &cellCenter, &difference, 1);
            stickX = (s32)((56.0f) * difference.x);
            stickY = (s32)((56.0f) * difference.z);
            *(Point3d *) ((u8 *) &lbl_1_bss_0 + playerIndex * 12 +
                          PIXEL_CPU_STICK_DIRECTION_OFFSET) = difference;
        }
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            120) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            break;
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        /* The earlier 120-frame exit makes this 180-frame timeout unreachable. */
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            180) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    }
    case PIXEL_CPU_PAUSE:
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))--;
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <= 0) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    case PIXEL_CPU_JUMP:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) < 15) {
            heldButtons = PAD_BUTTON_A;
        } else if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                             PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 16) {
            if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <
                17) {
                pressedButtons = PAD_BUTTON_A;
                heldButtons = PAD_BUTTON_A;
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_GROUND_POUND;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to hipdrop\n");
                break;
            }
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    case PIXEL_CPU_GROUND_POUND:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 10) {
            pressedButtons = PAD_BUTTON_A;
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            OSReport("to search\n");
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    }
    MgPlayerPadSet(player, stickX, stickY, pressedButtons, heldButtons);
}

/* The CPU difficulty dispatcher runs this each active play frame for normal CPUs, mixing nearby
 * incorrect pixels with random targets. */
void fn_1_48FC(s32 playerIndex)
{
    Point3d gridPosition;
    Point3d cellCenter;
    Point3d difference;
    Point3d direction;
    f32 offsetX;
    f32 offsetZ;
    MGPLAYER *player;
    s32 stickX;
    s32 stickY;
    s32 pressedButtons;
    s32 heldButtons;

    player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    gridPosition = player->actor->pos;
    stickX = 0;
    stickY = 0;
    pressedButtons = 0;
    heldButtons = 0;
    offsetX =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [0];
    offsetZ =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [1];
    gridPosition.x += offsetX;
    gridPosition.y = (0.0f);
    gridPosition.z += offsetZ;
    switch (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET)) {
    case PIXEL_CPU_CHOOSE: {
        s32 chosenCell;
        {
            s32 choiceRoll = frandmod(100);
            if (choiceRoll < 5) {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_PAUSE;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) =
                    frandmod(120) + 60;
                break;
            } else if (choiceRoll < 30) {
                chosenCell = fn_1_3E30(playerIndex);
            } else {
                chosenCell = fn_1_3D28(playerIndex);
            }
        }
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET) = chosenCell;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) = PIXEL_CPU_MOVE;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        break;
    }
    case PIXEL_CPU_MOVE: {
        s32 targetCell;
        f32 distance;
        targetCell = *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET);
        cellCenter.x = (100.0f) + (200.0f) * (targetCell % 4);
        cellCenter.y = (0.0f);
        cellCenter.z = (100.0f) + (200.0f) * (targetCell / 4);
        PSVECSubtract(&cellCenter, &gridPosition, &difference);
        distance = PSVECMag(&difference);
        if (distance < (80.0f)) {
            if ((u32) frandmod(100) < 80 &&
                fn_1_3A84(
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET),
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                              PIXEL_CPU_TARGET_CELL_OFFSET)) == 0) {
                pressedButtons = PAD_BUTTON_A;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_JUMP;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to jump\n");
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_CHOOSE;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            }
        } else {
            f32 directionLength;
            PSVECSubtract(&cellCenter, &gridPosition, &direction);
            directionLength = PSVECMag(&direction);
            if ((0.0f) != directionLength) {
                direction.x /= directionLength;
                direction.y /= directionLength;
                direction.z /= directionLength;
            }
            gridPosition.x += (56.0f) * direction.x;
            gridPosition.y += (56.0f) * direction.y;
            gridPosition.z += (56.0f) * direction.z;
            fn_1_5F54(playerIndex, &gridPosition, &cellCenter, &difference, 1);
            stickX = (s32)((56.0f) * difference.x);
            stickY = (s32)((56.0f) * difference.z);
            *(Point3d *) ((u8 *) &lbl_1_bss_0 + playerIndex * 12 +
                          PIXEL_CPU_STICK_DIRECTION_OFFSET) = difference;
        }
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            120) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            break;
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        /* The earlier 120-frame exit makes this 180-frame timeout unreachable. */
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            180) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    }
    case PIXEL_CPU_PAUSE:
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))--;
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <= 0) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    case PIXEL_CPU_JUMP:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) < 15) {
            heldButtons = PAD_BUTTON_A;
        } else if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                             PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 16) {
            if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <
                17) {
                pressedButtons = PAD_BUTTON_A;
                heldButtons = PAD_BUTTON_A;
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_GROUND_POUND;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to hipdrop\n");
                break;
            }
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    case PIXEL_CPU_GROUND_POUND:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 10) {
            pressedButtons = PAD_BUTTON_A;
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            OSReport("to search\n");
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    }
    MgPlayerPadSet(player, stickX, stickY, pressedButtons, heldButtons);
}

/* The CPU difficulty dispatcher runs this each active play frame for hard CPUs; it targets
 * incorrect pixels. */
void fn_1_50A8(s32 playerIndex)
{
    Point3d gridPosition;
    Point3d cellCenter;
    Point3d difference;
    Point3d direction;
    f32 offsetX;
    f32 offsetZ;
    MGPLAYER *player;
    s32 stickX;
    s32 stickY;
    s32 pressedButtons;
    s32 heldButtons;

    player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    gridPosition = player->actor->pos;
    stickX = 0;
    stickY = 0;
    pressedButtons = 0;
    heldButtons = 0;
    offsetX =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [0];
    offsetZ =
        lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)]
                      [1];
    gridPosition.x += offsetX;
    gridPosition.y = (0.0f);
    gridPosition.z += offsetZ;
    switch (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET)) {
    case PIXEL_CPU_CHOOSE: {
        s32 chosenCell = fn_1_3D80(playerIndex);
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET) = chosenCell;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) = PIXEL_CPU_MOVE;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        break;
    }
    case PIXEL_CPU_MOVE: {
        s32 targetCell;
        f32 distance;
        targetCell = *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET);
        if (targetCell < 0) {
            break;
        }
        cellCenter.x = (100.0f) + (200.0f) * (targetCell % 4);
        cellCenter.y = (0.0f);
        cellCenter.z = (100.0f) + (200.0f) * (targetCell / 4);
        PSVECSubtract(&cellCenter, &gridPosition, &difference);
        distance = PSVECMag(&difference);
        if (distance < (80.0f)) {
            if (fn_1_3A84(
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET),
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                              PIXEL_CPU_TARGET_CELL_OFFSET)) == 0) {
                pressedButtons = PAD_BUTTON_A;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_JUMP;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to jump\n");
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_CHOOSE;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            }
        } else {
            f32 directionLength;
            PSVECSubtract(&cellCenter, &gridPosition, &direction);
            directionLength = PSVECMag(&direction);
            if ((0.0f) != directionLength) {
                direction.x /= directionLength;
                direction.y /= directionLength;
                direction.z /= directionLength;
            }
            gridPosition.x += (56.0f) * direction.x;
            gridPosition.y += (56.0f) * direction.y;
            gridPosition.z += (56.0f) * direction.z;
            fn_1_5F54(playerIndex, &gridPosition, &cellCenter, &difference, 1);
            stickX = (s32)((56.0f) * difference.x);
            stickY = (s32)((56.0f) * difference.z);
            *(Point3d *) ((u8 *) &lbl_1_bss_0 + playerIndex * 12 +
                          PIXEL_CPU_STICK_DIRECTION_OFFSET) = difference;
        }
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            120) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            break;
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        /* The earlier 120-frame exit makes this 180-frame timeout unreachable. */
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            180) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    }
    case PIXEL_CPU_PAUSE:
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))--;
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <= 0) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        }
        break;
    case PIXEL_CPU_JUMP:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) < 15) {
            heldButtons = PAD_BUTTON_A;
        } else if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                             PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 16) {
            if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <
                17) {
                pressedButtons = PAD_BUTTON_A;
                heldButtons = PAD_BUTTON_A;
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_GROUND_POUND;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to hipdrop\n");
                break;
            }
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    case PIXEL_CPU_GROUND_POUND:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 10) {
            pressedButtons = PAD_BUTTON_A;
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            OSReport("to search\n");
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    }
    MgPlayerPadSet(player, stickX, stickY, pressedButtons, heldButtons);
}

/* The difficulty dispatcher updates very hard CPUs each active play frame; a zero mode timer
 * chooses a random target, otherwise they favor nearby incorrect pixels. */
void fn_1_582C(s32 playerIndex)
{
    Point3d gridPosition;
    Point3d difference;
    Point3d cellCenter;
    Point3d direction;
    MGPLAYER *player;
    s32 stickX;
    s32 stickY;
    s32 pressedButtons;
    s32 heldButtons;

    player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    stickX = 0;
    stickY = 0;
    pressedButtons = 0;
    heldButtons = 0;
    switch (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET)) {
    case PIXEL_CPU_JUMP:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) < 15) {
            heldButtons = PAD_BUTTON_A;
        } else if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                             PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 16) {
            if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) <
                17) {
                pressedButtons = PAD_BUTTON_A;
                heldButtons = PAD_BUTTON_A;
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_GROUND_POUND;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to hipdrop(%d)\n", playerIndex);
                break;
            }
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    case PIXEL_CPU_GROUND_POUND:
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >= 10) {
            pressedButtons = PAD_BUTTON_A;
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            OSReport("to search(%d)\n", playerIndex);
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    case PIXEL_CPU_CHOOSE: {
        s32 chosenCell;
        if (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) == 0) {
            chosenCell = fn_1_3D28(playerIndex);
        } else if ((u32)frandmod(100) < 20) {
            chosenCell = fn_1_3D80(playerIndex);
        } else {
            chosenCell = fn_1_3E30(playerIndex);
        }
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET) = chosenCell;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) = PIXEL_CPU_MOVE;
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
        break;
    }
    case PIXEL_CPU_MOVE: {
        s32 targetCell;
        f32 distance;
        targetCell = *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET);
        gridPosition = player->actor->pos;
        gridPosition.x += lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                                                   PIXEL_PLAYER_SIDE_OFFSET)][0];
        gridPosition.y = (0.0f);
        gridPosition.z += lbl_1_data_270[*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                                                   PIXEL_PLAYER_SIDE_OFFSET)][1];
        if (targetCell == -1) {
            OSReport("idx : -1\n");
        }
        if (targetCell == -1) {
            break;
        }
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_TARGET_CELL_OFFSET) = targetCell;
        cellCenter.x = (100.0f) + (200.0f) * (targetCell % 4);
        cellCenter.y = (0.0f);
        cellCenter.z = (100.0f) + (200.0f) * (targetCell / 4);
        PSVECSubtract(&cellCenter, &gridPosition, &difference);
        distance = PSVECMag(&difference);
        if (distance < (80.0f)) {
            if (fn_1_3A84(
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET),
                    *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 +
                              PIXEL_CPU_TARGET_CELL_OFFSET)) == 0) {
                pressedButtons = PAD_BUTTON_A;
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_JUMP;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
                OSReport("to jump(%d)\n", playerIndex);
            } else {
                *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                    PIXEL_CPU_CHOOSE;
                *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            }
        } else {
            f32 directionLength;
            PSVECSubtract(&cellCenter, &gridPosition, &direction);
            directionLength = PSVECMag(&direction);
            if ((0.0f) != directionLength) {
                direction.x /= directionLength;
                direction.y /= directionLength;
                direction.z /= directionLength;
            }
            gridPosition.x += (56.0f) * direction.x;
            gridPosition.y += (56.0f) * direction.y;
            gridPosition.z += (56.0f) * direction.z;
            fn_1_5F54(playerIndex, &gridPosition, &cellCenter, &difference, 1);
            stickX = (s32)((56.0f) * difference.x);
            stickY = (s32)((56.0f) * difference.z);
        }
        if (*(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) >=
            120) {
            *(s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_MODE_OFFSET) =
                PIXEL_CPU_CHOOSE;
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET) = 0;
            break;
        }
        (*(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_CPU_FRAME_COUNTER_OFFSET))++;
        break;
    }
    }
    MgPlayerPadSet(player, stickX, stickY, pressedButtons, heldButtons);
}

/* CPU updates and the result lineup call this to build stick direction; it bends movement around
 * nearby grounded teammates. */
void fn_1_5F54(s32 playerIndex, Point3d *start, Point3d *target, Point3d *stickDirection,
               s32 gridCoordinates)
{
    Point3d teammatePosition;
    Point3d difference;
    Point3d startPosition;
    MGPLAYER *movingPlayer;
    f32 targetDistance;
    f32 distance;
    f32 directionAngle;
    f32 lateralOffset;
    f32 avoidanceAngle;
    s32 teammateSeen;
    s32 teammateIndex;
    MGPLAYER *teammate;

    avoidanceAngle = (0.0f);
    teammateSeen = 0;
    startPosition = *start;
    PSVECSubtract(target, start, stickDirection);
    /* Distance and player are fetched here but do not affect the avoidance calculation. */
    targetDistance = PSVECMag(stickDirection);
    movingPlayer =
        *(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    directionAngle = (f32) ((180.0) * (atan2((f64) stickDirection->z, (f64) stickDirection->x) /
                                       (3.141592653589793)));
    for (teammateIndex = 0; teammateIndex < 4; teammateIndex++) {
        teammate =
            *(MGPLAYER **) ((u8 *) &lbl_1_bss_0 + teammateIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
        if (playerIndex != teammateIndex &&
            *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET) ==
                *(s32 *)((u8 *)&lbl_1_bss_0 + teammateIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET) &&
            MgPlayerModeAttrCheck(teammate, MGPLAYER_MODEATTR_AIR) != MGPLAYER_MODEATTR_AIR) {
            teammatePosition = teammate->actor->pos;
            if (gridCoordinates != 0) {
                teammatePosition.x += lbl_1_data_270[*(
                    s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)][0];
                teammatePosition.y = (0.0f);
                teammatePosition.z += lbl_1_data_270[*(
                    s32 *) ((u8 *) &lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET)][1];
            }
            teammateSeen = 1;
            PSVECSubtract(&startPosition, &teammatePosition, &difference);
            distance = PSVECMag(&difference);
            if (distance < (160.0f)) {
                lateralOffset = (teammatePosition.x - target->x) *
                                    sin(((3.141592653589793) * (f64) directionAngle) / (180.0)) +
                                (target->z - teammatePosition.z) *
                                    cos(((3.141592653589793) * (f64) directionAngle) / (180.0));
                lateralOffset /= (90.0f);
                if (lateralOffset >= (0.0f)) {
                    avoidanceAngle += (80.0f) / ((1.0f) + lateralOffset);
                } else {
                    avoidanceAngle -= (80.0f) / ((1.0f) - lateralOffset);
                }
            }
        }
    }
    if (teammateSeen != 0) {
        /* Nearby teammates bend the heading; a distant teammate still selects this path. */
        directionAngle += avoidanceAngle;
        stickDirection->x = (f32)cos(((3.141592653589793) * (f64)directionAngle) / (180.0));
        stickDirection->z = (f32)sin(((3.141592653589793) * (f64)directionAngle) / (180.0));
    }
    distance = PSVECMag(stickDirection);
    if ((0.0f) != distance) {
        stickDirection->x /= distance;
        stickDirection->y /= distance;
        stickDirection->z /= distance;
    }
    /* World-space Z runs opposite the controller's vertical stick axis. */
    stickDirection->z *= (-1.0f);
}
