/* Picture-cell reveals and the teams' floor-panel pixel patterns for Pixel Perfect. */
#include <math.h>
#include "REL/m636dll.h"


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
    side = lbl_1_bss_0.teamSide[playerIndex];
    player = lbl_1_bss_0.players[playerIndex];
    if (fn_80087114(player, 0x200) != 0 &&
        (actor->colGroundAttr & 0x1) != 0) {
        fn_80086BC8(actor, &pos);
        lbl_1_bss_0.landingCounter[playerIndex] = 90;
        /* Each floor is translated to a positive 800-by-800 grid of 200-unit pixels. */
        offsetX = (s32)lbl_1_data_270[side][0];
        offsetZ = (s32)lbl_1_data_270[side][1];
        column = (s32)((pos.x + offsetX) / 200.0f);
        row = (s32)((pos.z + offsetZ) / 200.0f);
        if (fn_1_3AE0(side, column, row) == 1) {
            return;
        }
        if (fn_1_3984(0) != 0 || fn_1_3984(1) != 0 ||
            MgTimerDoneCheck(lbl_1_bss_0.timer) != 0) {
            return;
        }
        if (lbl_1_bss_0.teamPixels[side][column + row * 4] == 1) {
            lbl_1_bss_0.teamPixels[side][column + row * 4] = 0;
            fn_8002C95C(lbl_1_bss_0.panelModels[side][column + row * 4],
                          lbl_1_bss_0.panelMotions[3]);
        } else {
            lbl_1_bss_0.teamPixels[side][column + row * 4] = 1;
            fn_8002C95C(lbl_1_bss_0.panelModels[side][column + row * 4],
                          lbl_1_bss_0.panelMotions[2]);
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

    lbl_1_bss_0.floorDrawHook = fn_800260F4(fn_1_3C94);
    fn_80026B8C(lbl_1_bss_0.floorDrawHook, 0);

    for (cellIndex = 0; cellIndex < 16; cellIndex++) {
        lbl_1_bss_0.teamPixels[0][cellIndex] = lbl_1_data_140[cellIndex];
        lbl_1_bss_0.teamPixels[1][cellIndex] = lbl_1_data_140[cellIndex];

        panelModel = lbl_1_bss_0.panelModels[0][cellIndex];
        if (lbl_1_bss_0.teamPixels[0][cellIndex] == 0) {
            fn_8002C95C(panelModel, lbl_1_bss_0.panelMotions[0]);
        } else {
            fn_8002C95C(panelModel, lbl_1_bss_0.panelMotions[1]);
        }

        panelModel = lbl_1_bss_0.panelModels[1][cellIndex];
        if (lbl_1_bss_0.teamPixels[1][cellIndex] == 0) {
            fn_8002C95C(panelModel, lbl_1_bss_0.panelMotions[0]);
        } else {
            fn_8002C95C(panelModel, lbl_1_bss_0.panelMotions[1]);
        }
    }

    lbl_1_bss_0.selectedPicture = frand() % 3;
    previousWasCorner = 0;
    isCorner = 0;

    for (cellIndex = 0; cellIndex < 5; cellIndex++) {
        for (;;) {
            chosenCell = frand() & 0xF;

            for (previousChoice = 0; previousChoice < cellIndex; previousChoice++) {
                if (lbl_1_bss_0.roundCellChoices[previousChoice] == chosenCell) {
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
            if (lbl_1_bss_0.selectedPicture == 2) {
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
            lbl_1_bss_0.roundCellChoices[cellIndex] = chosenCell;
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

    modelId = lbl_1_bss_0.pictureCellModels[pictureIndex][cellIndex];
    fn_800269E8(
        lbl_1_bss_0.pictureIntroModels[pictureIndex],
        (1 << 0));
    otherCell = 0;
    while (otherCell < 16) {
        fn_800269E8(lbl_1_bss_0.pictureCellModels[pictureIndex][otherCell],
                         (1 << 0));
        otherCell++;
    }
    fn_80026A44(modelId, (1 << 0));
    fn_8002D414(modelId, 0.0f);

    fn_800269E8(lbl_1_bss_0.referenceModel,
                     (1 << 0));
    otherCell = 0;
    while (otherCell < 16) {
        fn_800269E8(
            lbl_1_bss_0.referenceCellModels[otherCell],
            (1 << 0));
        otherCell++;
    }
    fn_8002D414(
        lbl_1_bss_0.referenceCellModels[cellIndex],
        0.0f);
    fn_80026A44(
        lbl_1_bss_0.referenceCellModels[cellIndex],
        (1 << 0));
    fn_80026A44(
        lbl_1_bss_0.referenceCellModels[cellIndex],
        ((1 << 30) | (1 << 2)));
    HuAudFXPlay(MSM_SE_MG636_CELL_FLIP);
}

/* The round sequence polls this while waiting for the selected picture-cell reveal to finish. */
BOOL fn_1_35E0(s32 pictureIndex, s32 cellIndex)
{
    s16 modelId;

    modelId = lbl_1_bss_0.pictureCellModels[pictureIndex][cellIndex];
    if (fn_8002D75C(modelId) == 1) {
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

    modelId = lbl_1_bss_0.pictureCellModels[pictureIndex][cellIndex];
    lastFrame = Hu3DMotionMotionMaxTimeGet(fn_8002D3C4(modelId));
    otherCell = 0;
    while (otherCell < 16) {
        fn_800269E8(lbl_1_bss_0.pictureCellModels[pictureIndex][otherCell],
                         (1 << 0));
        otherCell++;
    }
    fn_80026A44(modelId, (1 << 0));
    fn_800269E8(modelId, ((1 << 30) | (1 << 2)));
    /* Reverse playback begins at the last frame rather than restarting at zero. */
    fn_8002D414(modelId, lastFrame);

    otherCell = 0;
    while (otherCell < 16) {
        fn_800269E8(
            lbl_1_bss_0.referenceCellModels[otherCell],
            (1 << 0));
        otherCell++;
    }
    modelId = lbl_1_bss_0.referenceCellModels[cellIndex];
    lastFrame = Hu3DMotionMotionMaxTimeGet(fn_8002D3C4(modelId));
    fn_8002D414(modelId, lastFrame);
    fn_80026A44(modelId, (1 << 0));
    fn_800269E8(modelId, ((1 << 30) | (1 << 2)));
    HuAudFXPlay(MSM_SE_MG636_CELL_FLIP);
}

/* The round and finish sequences poll this until reverse playback reaches its endpoint or time
 * zero. */
s32 fn_1_37A8(s32 pictureIndex, s32 cellIndex)
{
    HU3D_MODELID modelId;

    modelId = lbl_1_bss_0.pictureCellModels[pictureIndex][cellIndex];
    if (fn_8002D75C(modelId) == 1 ||
        fn_8002D4E0(modelId) <= 0.0f) {
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
        memcpy(&lbl_1_bss_0.targetPixels[rowIndex * 4], sourceRow, 4);
        sourceRow += PIXEL_PICTURE_ROW_BYTES;
    }
    for (rowIndex = 0; rowIndex < 4; rowIndex++) {
        OSReport("%02X %02X %02X %02X\n",
            lbl_1_bss_0.targetPixels[rowIndex * 4],
            lbl_1_bss_0.targetPixels[rowIndex * 4 + 1],
            lbl_1_bss_0.targetPixels[rowIndex * 4 + 2],
            lbl_1_bss_0.targetPixels[rowIndex * 4 + 3]);
    }
}

/* The play loop and landing hook call this to check whether a team copied all target pixels. */
int fn_1_3984(s32 side)
{
    u8 *teamPixels;
    u8 *targetPixels;

    teamPixels = lbl_1_bss_0.teamPixels[side];
    targetPixels = lbl_1_bss_0.targetPixels;
    if (memcmp(teamPixels, targetPixels, 16) == 0) {
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

    teamPixels = lbl_1_bss_0.teamPixels[side];
    targetPixels = lbl_1_bss_0.targetPixels;
    mismatchCount = 0;
    for (cellIndex = 0; cellIndex < 16; cellIndex++) {
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

    teamPixels = lbl_1_bss_0.teamPixels[side];
    targetPixels = lbl_1_bss_0.targetPixels;
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

    panelModel = lbl_1_bss_0.panelModels[side][column + row * 4];
    motionId = fn_8002D3C4(panelModel);
    /* The play update replaces these motions after they end, releasing the pixel for another
     * flip. */
    if (motionId == lbl_1_bss_0.panelMotions[2]
        || motionId == lbl_1_bss_0.panelMotions[3]) {
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
        while (cellIndex < 16) {
            modelId = lbl_1_bss_0.panelModels[side][cellIndex];
            if (fn_8002D75C(modelId) == 1) {
                if ((s16)fn_8002D3C4(modelId)
                    == lbl_1_bss_0.panelMotions[2]) {
                    fn_8002C95C(modelId,
                                  lbl_1_bss_0.panelMotions[1]);
                } else if ((s16)fn_8002D3C4(modelId)
                           == lbl_1_bss_0.panelMotions[3]) {
                    fn_8002C95C(modelId,
                                  lbl_1_bss_0.panelMotions[0]);
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
    switch (lbl_80265728[playerIndex].comDif) {
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
