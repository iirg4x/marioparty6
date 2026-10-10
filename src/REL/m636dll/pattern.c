/* Picture-cell reveals and the teams' floor-panel pixel patterns for Pixel Perfect. */
#include "dolphin/math.h"
#include "REL/m636dll.h"

#define PIXEL_CELL_COUNT 16
#define PIXEL_PICTURE_ROW_BYTES 16
#define PIXEL_PICTURE_PIXEL_BYTES 256
#define PIXEL_CELL_MODEL_SET_BYTES 32
#define PIXEL_PLAYER_SIDE_OFFSET 104
#define PIXEL_PLAYER_RECORD_OFFSET 120
#define PIXEL_LANDING_COUNTER_OFFSET 136
#define PIXEL_TIMER_OFFSET 68
#define PIXEL_TEAM_PIXELS_OFFSET 530
#define PIXEL_RIGHT_TEAM_PIXELS_OFFSET 546
#define PIXEL_FLOOR_PANEL_MODELS_OFFSET 408
#define PIXEL_RIGHT_FLOOR_MODELS_OFFSET 440
#define PIXEL_PANEL_OFF_MOTION_OFFSET 400
#define PIXEL_PANEL_ON_MOTION_OFFSET 402
#define PIXEL_PANEL_TURN_ON_MOTION_OFFSET 404
#define PIXEL_PANEL_TURN_OFF_MOTION_OFFSET 406
#define PIXEL_FLOOR_DRAW_HOOK_OFFSET 528
#define PIXEL_SELECTED_PICTURE_OFFSET 504
#define PIXEL_ROUND_CELL_CHOICES_OFFSET 508
#define PIXEL_PICTURE_CELL_MODELS_OFFSET 286
#define PIXEL_PICTURE_INTRO_MODELS_OFFSET 274
#define PIXEL_REFERENCE_MODEL_OFFSET 270
#define PIXEL_REFERENCE_CELL_MODELS_OFFSET 578
#define PIXEL_TARGET_PIXELS_OFFSET 562

#define PIXEL_CELL_TRANSITION_SOUND 1869
#define PIXEL_GROUND_POUND_LAND_FLAG 0x200
#define PIXEL_FLOOR_GROUND_FLAG 0x1

/* Alternating initial floor pixels; the final array element is zero. */
u8 lbl_1_data_140[16] = {0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1};

/* Player creation installs this collision-correction hook; a grounded ground-pound landing
 * toggles that team's floor pixel during play. */
void fn_1_2F10(MGACTOR *actor, int playerSlot)
{
    Point3d pos;
    MGPLAYER *player;
    s32 offsetX;
    s32 offsetZ;
    s32 playerIndex;
    s32 side;
    s32 row;
    s32 column;

    playerIndex = playerSlot;
    side = *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_SIDE_OFFSET);
    player = *(MGPLAYER **)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_PLAYER_RECORD_OFFSET);
    if (MgPlayerVibAttrCheck(player, PIXEL_GROUND_POUND_LAND_FLAG) != 0 &&
        (actor->colGroundAttr & PIXEL_FLOOR_GROUND_FLAG) != 0) {
        MgActorPosGet(actor, &pos);
        *(s32 *)((u8 *)&lbl_1_bss_0 + playerIndex * 4 + PIXEL_LANDING_COUNTER_OFFSET) = 90;
        /* Each floor is translated to a positive 800-by-800 grid of 200-unit pixels. */
        offsetX = (s32)lbl_1_data_270[side][0];
        offsetZ = (s32)lbl_1_data_270[side][1];
        column = (s32)((pos.x + offsetX) / (200.0f));
        row = (s32)((pos.z + offsetZ) / (200.0f));
        if (fn_1_3AE0(side, column, row) == 1) {
            return;
        }
        if (fn_1_3984(0) != 0 || fn_1_3984(1) != 0 ||
            MgTimerDoneCheck(*(MGTIMER **)((u8 *)&lbl_1_bss_0 + PIXEL_TIMER_OFFSET)) != 0) {
            return;
        }
        if (((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_COUNT +
             PIXEL_TEAM_PIXELS_OFFSET)[column + row * 4] == 1) {
            ((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_COUNT +
             PIXEL_TEAM_PIXELS_OFFSET)[column + row * 4] = 0;
            Hu3DMotionSet(*(s16 *) ((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_MODEL_SET_BYTES +
                                    (column + row * 4) * 2 + PIXEL_FLOOR_PANEL_MODELS_OFFSET),
                          *(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_PANEL_TURN_OFF_MOTION_OFFSET));
        } else {
            ((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_COUNT +
             PIXEL_TEAM_PIXELS_OFFSET)[column + row * 4] = 1;
            Hu3DMotionSet(*(s16 *) ((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_MODEL_SET_BYTES +
                                    (column + row * 4) * 2 + PIXEL_FLOOR_PANEL_MODELS_OFFSET),
                          *(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_PANEL_TURN_ON_MOTION_OFFSET));
        }
    }
}

/* The scene setup callback calls this to initialize both floors and choose five distinct cells for
 * the round's picture. */
void fn_1_31BC(void)
{
    s32 cellIndex;
    s16 panelModel;
    s32 previousChoice;
    s32 chosenCell;
    s32 isCorner;
    s32 previousWasCorner;

    *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_FLOOR_DRAW_HOOK_OFFSET) = Hu3DHookFuncCreate(fn_1_3C94);
    Hu3DModelLayerSet(*(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_FLOOR_DRAW_HOOK_OFFSET), 0);

    for (cellIndex = 0; cellIndex < 16; cellIndex++) {
        *((u8 *)&lbl_1_bss_0 + PIXEL_TEAM_PIXELS_OFFSET + cellIndex) = lbl_1_data_140[cellIndex];
        *((u8 *) &lbl_1_bss_0 + PIXEL_RIGHT_TEAM_PIXELS_OFFSET + cellIndex) =
            lbl_1_data_140[cellIndex];

        panelModel = *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_FLOOR_PANEL_MODELS_OFFSET + cellIndex * 2);
        if (*((u8 *)&lbl_1_bss_0 + PIXEL_TEAM_PIXELS_OFFSET + cellIndex) == 0) {
            Hu3DMotionSet(panelModel, *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_OFF_MOTION_OFFSET));
        } else {
            Hu3DMotionSet(panelModel, *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_ON_MOTION_OFFSET));
        }

        panelModel = *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_RIGHT_FLOOR_MODELS_OFFSET + cellIndex * 2);
        if (*((u8 *)&lbl_1_bss_0 + PIXEL_RIGHT_TEAM_PIXELS_OFFSET + cellIndex) == 0) {
            Hu3DMotionSet(panelModel, *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_OFF_MOTION_OFFSET));
        } else {
            Hu3DMotionSet(panelModel, *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_ON_MOTION_OFFSET));
        }
    }

    *(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) = frand() % 3;
    previousWasCorner = 0;
    isCorner = 0;

    for (cellIndex = 0; cellIndex < 5; cellIndex++) {
        for (;;) {
            chosenCell = frand() & 0xF;

            for (previousChoice = 0; previousChoice < cellIndex; previousChoice++) {
                if (*(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_ROUND_CELL_CHOICES_OFFSET +
                              previousChoice * 4) == chosenCell) {
                    break;
                }
            }
            if (previousChoice != cellIndex) {
                continue;
            }

            if (chosenCell == 0 || chosenCell == 3 || chosenCell == 12 || chosenCell == 15) {
                isCorner = 1;
            } else {
                isCorner = 0;
            }

            /* The third picture never selects either top corner. */
            if (*(s32 *)((u8 *)&lbl_1_bss_0 + PIXEL_SELECTED_PICTURE_OFFSET) == 2) {
                if (chosenCell == 0 || chosenCell == 3) {
                    if (chosenCell == 0) {
                        OSReport("左上出たのでスキップ\n");
                    }
                    if (chosenCell == 3) {
                        OSReport("右上出たのでスキップ\n");
                    }
                    continue;
                }
            }

            /* Corner picture cells cannot occur on consecutive rounds. */
            if (isCorner != 0 && previousWasCorner != 0) {
                continue;
            }

            previousWasCorner = isCorner;
            *(s32 *) ((u8 *) &lbl_1_bss_0 + PIXEL_ROUND_CELL_CHOICES_OFFSET + cellIndex * 4) =
                chosenCell;
            OSReport("piece %d ... %d\n", cellIndex, chosenCell);
            break;
        }
    }
}

/* Before players copy the floor pattern each round, the opening step highlights its 4-by-4 region
 * in the sample picture. */
void fn_1_3460(s32 pictureIndex, s32 cellIndex)
{
    s32 otherCell;
    s16 modelId;

    modelId = *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_CELL_MODELS_OFFSET +
                        pictureIndex * PIXEL_CELL_MODEL_SET_BYTES + cellIndex * 2);
    Hu3DModelAttrSet(
        *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_INTRO_MODELS_OFFSET + pictureIndex * 2),
        HU3D_ATTR_DISPOFF);
    otherCell = 0;
    while (otherCell < PIXEL_CELL_COUNT) {
        Hu3DModelAttrSet(*(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_CELL_MODELS_OFFSET +
                                   pictureIndex * PIXEL_CELL_MODEL_SET_BYTES + otherCell * 2),
                         HU3D_ATTR_DISPOFF);
        otherCell++;
    }
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(modelId, (0.0f));

    Hu3DModelAttrSet(*(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_MODEL_OFFSET),
                     HU3D_ATTR_DISPOFF);
    otherCell = 0;
    while (otherCell < PIXEL_CELL_COUNT) {
        Hu3DModelAttrSet(
            *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + otherCell * 2),
            HU3D_ATTR_DISPOFF);
        otherCell++;
    }
    Hu3DMotionTimeSet(
        *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + cellIndex * 2),
        (0.0f));
    Hu3DModelAttrReset(
        *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + cellIndex * 2),
        HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(
        *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + cellIndex * 2),
        HU3D_MOTATTR_REV);
    HuAudFXPlay(PIXEL_CELL_TRANSITION_SOUND);
}

/* The round sequence polls this while waiting for the selected picture-cell reveal to finish. */
BOOL fn_1_35E0(s32 pictureIndex, s32 cellIndex)
{
    s16 modelId;

    modelId = (*(s16 *) ((s8 *) (&lbl_1_bss_0) + (pictureIndex * PIXEL_CELL_MODEL_SET_BYTES) +
                         (cellIndex * 2) + (PIXEL_PICTURE_CELL_MODELS_OFFSET)));
    if (Hu3DMotionEndCheck(modelId) == 1) {
        return 1;
    }
    return 0;
}

/* The round sequence retracts the selected picture cell between rounds; the Finish hook retracts
 * the final cell. */
void fn_1_364C(s32 pictureIndex, s32 cellIndex)
{
    s32 otherCell;
    s16 modelId;
    f32 lastFrame;

    modelId = *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_CELL_MODELS_OFFSET +
                        pictureIndex * PIXEL_CELL_MODEL_SET_BYTES + cellIndex * 2);
    lastFrame = Hu3DMotionMotionMaxTimeGet(Hu3DMotionIDGet(modelId));
    otherCell = 0;
    while (otherCell < PIXEL_CELL_COUNT) {
        Hu3DModelAttrSet(*(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_PICTURE_CELL_MODELS_OFFSET +
                                   pictureIndex * PIXEL_CELL_MODEL_SET_BYTES + otherCell * 2),
                         HU3D_ATTR_DISPOFF);
        otherCell++;
    }
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_REV);
    /* Reverse playback begins at the last frame rather than restarting at zero. */
    Hu3DMotionTimeSet(modelId, lastFrame);

    otherCell = 0;
    while (otherCell < PIXEL_CELL_COUNT) {
        Hu3DModelAttrSet(
            *(s16 *) ((s8 *) &lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + otherCell * 2),
            HU3D_ATTR_DISPOFF);
        otherCell++;
    }
    modelId = *(s16 *)((s8 *)&lbl_1_bss_0 + PIXEL_REFERENCE_CELL_MODELS_OFFSET + cellIndex * 2);
    lastFrame = Hu3DMotionMotionMaxTimeGet(Hu3DMotionIDGet(modelId));
    Hu3DMotionTimeSet(modelId, lastFrame);
    Hu3DModelAttrReset(modelId, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_REV);
    HuAudFXPlay(PIXEL_CELL_TRANSITION_SOUND);
}

/* The round and finish sequences poll this until reverse playback reaches its endpoint or time
 * zero. */
s32 fn_1_37A8(s32 pictureIndex, s32 cellIndex)
{
    HU3D_MODELID modelId;

    modelId = *(HU3D_MODELID *) ((u8 *) &lbl_1_bss_0 + PIXEL_PICTURE_CELL_MODELS_OFFSET +
                                 pictureIndex * PIXEL_CELL_MODEL_SET_BYTES + cellIndex * 2);
    if (Hu3DMotionEndCheck(modelId) == 1 ||
        Hu3DMotionTimeGet(modelId) <= (0.0f)) {
        return 1;
    }
    return 0;
}

/* The opening and round sequences call this to copy a selected 4-by-4 block of the reference
 * picture into the floor target pixels. */
void fn_1_3834(s32 pictureIndex, s32 cellIndex)
{
    s32 pixelColumn;
    s32 pixelRow;
    u8 *picturePixels;
    u8 *sourceRow;
    s32 rowIndex;

    pixelColumn = (cellIndex % 4) * 4;
    pixelRow = (cellIndex / 4) * 4;
    picturePixels = lbl_1_data_378 + pictureIndex * PIXEL_PICTURE_PIXEL_BYTES;
    sourceRow = picturePixels + pixelRow * PIXEL_PICTURE_ROW_BYTES + pixelColumn;
    OSReport("***********\n");
    OSReport("pic:%d, pat:%d (%d, %d)\n", pictureIndex, cellIndex, pixelColumn, pixelRow);
    /* Picture rows contain sixteen pixels; the floor target rows contain four. */
    for (rowIndex = 0; rowIndex < 4; rowIndex++) {
        memcpy((u8 *)&lbl_1_bss_0 + rowIndex * 4 + PIXEL_TARGET_PIXELS_OFFSET, sourceRow, 4);
        sourceRow += PIXEL_PICTURE_ROW_BYTES;
    }
    for (rowIndex = 0; rowIndex < 4; rowIndex++) {
        OSReport("%02X %02X %02X %02X\n",
            ((u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET)[rowIndex * 4],
            ((u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET)[rowIndex * 4 + 1],
            ((u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET)[rowIndex * 4 + 2],
            ((u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET)[rowIndex * 4 + 3]);
    }
}

/* The play loop and landing hook call this to check whether a team copied all target pixels. */
int fn_1_3984(s32 side)
{
    u8 *teamPixels;
    u8 *targetPixels;

    teamPixels = (u8 *)&lbl_1_bss_0 + (side * PIXEL_CELL_COUNT) + PIXEL_TEAM_PIXELS_OFFSET;
    targetPixels = (u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET;
    if (memcmp(teamPixels, targetPixels, PIXEL_CELL_COUNT) == 0) {
        return 1;
    }
    return 0;
}

/* CPU target selection calls this to count incorrect team pixels before sharing a teammate
 * target. */
s32 fn_1_39FC(s32 side)
{
    u8 *teamPixels;
    u8 *targetPixels;
    s32 cellIndex;
    s32 mismatchCount;

    teamPixels = (u8 *)&lbl_1_bss_0 + side * PIXEL_CELL_COUNT + PIXEL_TEAM_PIXELS_OFFSET;
    targetPixels = (u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET;
    mismatchCount = 0;
    for (cellIndex = 0; cellIndex < PIXEL_CELL_COUNT; cellIndex++) {
        if (*teamPixels != *targetPixels) {
            mismatchCount++;
        }
        teamPixels++;
        targetPixels++;
    }
    return mismatchCount;
}

/* CPU target selection and jump logic call this to compare one floor pixel with the target. */
s32 fn_1_3A84(s32 side, s32 cellIndex)
{
    u8 *teamPixels;
    u8 *targetPixels;

    teamPixels = (u8 *)&lbl_1_bss_0 + PIXEL_TEAM_PIXELS_OFFSET + (side * PIXEL_CELL_COUNT);
    targetPixels = (u8 *)&lbl_1_bss_0 + PIXEL_TARGET_PIXELS_OFFSET;
    if (teamPixels[cellIndex] == targetPixels[cellIndex]) {
        return 1;
    } else {
        return 0;
    }
}

/* The landing hook calls this to reject a second toggle while the panel is transitioning. */
s32 fn_1_3AE0(s32 side, s32 column, s32 row)
{
    HU3D_MOTIONID motionId;

    HU3D_MODELID panelModel;

    panelModel = *(s16 *) ((u8 *) &lbl_1_bss_0 + side * PIXEL_CELL_MODEL_SET_BYTES +
                           PIXEL_FLOOR_PANEL_MODELS_OFFSET + (column + row * 4) * 2);
    motionId = Hu3DMotionIDGet(panelModel);
    /* The play update replaces these motions after they end, releasing the pixel for another
     * flip. */
    if (motionId == *(s16*)((u8*)&lbl_1_bss_0 + PIXEL_PANEL_TURN_ON_MOTION_OFFSET)
        || motionId == *(s16*)((u8*)&lbl_1_bss_0 + PIXEL_PANEL_TURN_OFF_MOTION_OFFSET)) {
        OSReport("crossfading...\n");
        return 1;
    }
    return 0;
}

/* The main play hook calls this each frame to replace completed panel transitions with steady
 * on/off motions. */
void fn_1_3BA0(void)
{
    s32 side;
    s32 cellIndex;
    s16 modelId;

    side = 0;
    while (side < 2) {
        cellIndex = 0;
        while (cellIndex < PIXEL_CELL_COUNT) {
            modelId = *(s16 *)((u8 *)&lbl_1_bss_0 + (side * PIXEL_CELL_MODEL_SET_BYTES)
                     + (cellIndex * 2) + PIXEL_FLOOR_PANEL_MODELS_OFFSET);
            if (Hu3DMotionEndCheck(modelId) == 1) {
                if ((s16)Hu3DMotionIDGet(modelId)
                    == *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_TURN_ON_MOTION_OFFSET)) {
                    Hu3DMotionSet(modelId,
                                  *(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_PANEL_ON_MOTION_OFFSET));
                } else if ((s16)Hu3DMotionIDGet(modelId)
                           == *(s16 *)((u8 *)&lbl_1_bss_0 + PIXEL_PANEL_TURN_OFF_MOTION_OFFSET)) {
                    Hu3DMotionSet(modelId,
                                  *(s16 *) ((u8 *) &lbl_1_bss_0 + PIXEL_PANEL_OFF_MOTION_OFFSET));
                }
            }
            cellIndex += 1;
        }
        side += 1;
    }
}

/* Scene setup creates a separate layer-0 hook model using this empty draw callback. */
void fn_1_3C94(HU3D_MODEL *model, Mtx *matrix)
{
}

/* The main play hook calls this for each CPU player to dispatch input by configured difficulty. */
void fn_1_3C98(s32 playerIndex)
{
    switch (GwPlayerConf[playerIndex].comDif) {
    case 0:
        fn_1_40E0(playerIndex);
        break;
    case 1:
        fn_1_48FC(playerIndex);
        break;
    case 2:
        fn_1_50A8(playerIndex);
        break;
    case 3:
        fn_1_582C(playerIndex);
        break;
    }
}
