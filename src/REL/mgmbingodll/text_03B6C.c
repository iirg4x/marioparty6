/* Sets up Treetop Bingo's boards, player turns, scoring, and effects. */
#include "REL/mgmbingodll/module_types.h"
#include "game/thpmain.h"

#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "msm_se.h"

#define MGM_BINGO_SOUND_MARKER_ACTIVATION 1265
#define MGM_BINGO_SOUND_CURSOR_CONFIRM 1268
#define MGM_BINGO_SOUND_CURSOR_MOVE 1263
#define MGM_BINGO_SOUND_RESULT_REVEAL 1212
#define MGM_BINGO_SOUND_RESULT_SPIN 1215
#define MGM_BINGO_SOUND_RESULT_STOP 1214

#define BINGO_BOARD_WORK_BASE 101
#define BINGO_CELL_NUMBER_MASK 0x7F
#define BINGO_CELL_MARKED_FLAG (1 << 7)
#define BINGO_CELL_REMOVED_PHASE 10000
#define BINGO_STAR_FINISHED_DELAY 32767

extern void fn_1_10CC(OMOBJ *object);

s16 fn_1_CF30(s16 player);
s16 fn_1_CF74(s16 player);

extern void fn_1_109E4(void);

void fn_1_5F58(OMOBJ *object);
void fn_1_6FE4(void);
void fn_1_77F0(s16 playerIndex, s16 modelId, char *animationName);
void fn_1_A3A0(void);
void fn_1_B2F8(void);
extern s32 fn_1_8838(s16 boardIndex, s16 lineIndex);
void fn_1_AAC8(HuVecF *position);

void fn_1_79AC(OMOBJ *object);

s16 fn_1_CF30(s16 player);
void fn_1_799C(s16 enable);
s32 fn_1_8838(s16 boardIndex, s16 lineIndex);
s16 fn_1_CF30(s16 playerNo);

extern u8 MgModeWork[256];
u16 fn_1_C8B8(s16 playerIndex, s16 targetCell);
void fn_1_799C(s16 enable);
void fn_1_347C(void);
void fn_1_108C(s16 pathIndex, s16 frames);
void fn_1_30C8(void);
void fn_1_3074(void);
s16 fn_1_CF30(s16 playerNo);

extern s16 MgModeScore[4];
s16 fn_1_CEF0(s16 player);

extern void fn_1_B524(OMOBJ *object);

extern void fn_1_A704(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_12BE0(f32 angleX, f32 angleY, f32 angleZ,
                const HuVecF *source, HuVecF *destination);

s16 fn_1_7AD0(s16);
extern int MgModeWorkInt[16];

s32 lbl_1_data_2A8[71] = {
    DATANUM(DATA_mgmbingo, 2), DATANUM(DATA_mgmbingo, 0),
    DATANUM(DATA_mgmbingo, 1), DATANUM(DATA_mgmbingo, 5),
    DATANUM(DATA_mgmbingo, 6), DATANUM(DATA_mgmbingo, 7),
    DATANUM(DATA_mgmbingo, 8), DATANUM(DATA_mgmbingo, 4),
    DATANUM(DATA_mgmbingo, 4), DATANUM(DATA_mgmbingo, 4),
    DATANUM(DATA_mgmbingo, 4), DATANUM(DATA_mgmbingo, 3),
    DATANUM(DATA_mgmbingo, 3), DATANUM(DATA_mgmbingo, 3),
    DATANUM(DATA_mgmbingo, 3), DATANUM(DATA_mgmbingo, 10),
    DATANUM(DATA_mgmbingo, 10), DATANUM(DATA_mgmbingo, 10),
    DATANUM(DATA_mgmbingo, 10), DATANUM(DATA_mgmbingo, 10),
    DATANUM(DATA_mgmbingo, 10), DATANUM(DATA_mgmbingo, 10),
    DATANUM(DATA_mgmbingo, 10), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 13),
    DATANUM(DATA_mgmbingo, 13), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 13),
    DATANUM(DATA_mgmbingo, 13), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 13),
    DATANUM(DATA_mgmbingo, 13), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 11), DATANUM(DATA_mgmbingo, 11),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 12),
    DATANUM(DATA_mgmbingo, 12), DATANUM(DATA_mgmbingo, 13),
    DATANUM(DATA_mgmbingo, 13)
};

s32 lbl_1_data_3C4[3] = {
    DATANUM(DATA_mgmbingo, 16), DATANUM(DATA_mgmbingo, 18), DATANUM(DATA_mgmbingo, 17)};

char *lbl_1_data_400[4] = {"f-itemfook1", "f-itemfook2", "f-itemfook3", "f-itemfook4"};

char *lbl_1_data_440[4] = {"f-itemfook5", "f-itemfook6", "f-itemfook7", "f-itemfook8"};

char *lbl_1_data_51C[30] = {
    "f-ha1", "f-ha2", "f-ha3", "f-ha4", "f-ha5", "f-ha6", "f-ha7", "f-ha8",
    "f-ha9", "f-ha10", "f-ha11", "f-ha12", "f-ha13", "f-ha14", "f-ha15", "f-ha16",
    "f-ha17", "f-ha18", "f-ha19", "f-ha20", "f-ha21", "f-ha22", "f-ha23", "f-ha24",
    "f-ha25", "f-ha26", "f-ha27", "f-ha28", "f-ha29", "f-ha30"
};

char *lbl_1_data_5A0 = "f-ha_top";

Point3d lbl_1_data_5A4[5][5] = {
    {{-100.0f, 100.0f, 2.0f}, {-50.0f, 100.0f, 2.0f}, {0.0f, 100.0f, 2.0f}, {50.0f, 100.0f, 2.0f}, {100.0f, 100.0f, 2.0f}},
    {{-100.0f, 50.0f, 2.0f}, {-50.0f, 50.0f, 2.0f}, {0.0f, 50.0f, 2.0f}, {50.0f, 50.0f, 2.0f}, {100.0f, 50.0f, 2.0f}},
    {{-100.0f, 0.0f, 2.0f}, {-50.0f, 0.0f, 2.0f}, {0.0f, 0.0f, 2.0f}, {50.0f, 0.0f, 2.0f}, {100.0f, 0.0f, 2.0f}},
    {{-100.0f, -50.0f, 2.0f}, {-50.0f, -50.0f, 2.0f}, {0.0f, -50.0f, 2.0f}, {50.0f, -50.0f, 2.0f}, {100.0f, -50.0f, 2.0f}},
    {{-100.0f, -100.0f, 2.0f}, {-50.0f, -100.0f, 2.0f}, {0.0f, -100.0f, 2.0f}, {50.0f, -100.0f, 2.0f}, {100.0f, -100.0f, 2.0f}}
};

/* Called during mode setup to create the board models, arrange their pieces, and load the number
 * animations. */
void fn_1_3B6C(void) {
    OMOBJ *object;
    Point3d playerPosition;
    HU3D_ANIMID animationId;
    ANIMDATA *animation;
    s32 playerIndex;
    s32 modelIndex;
    s16 modelLayer;

    playerIndex = 0;
    while (playerIndex < 4) {
        lbl_1_bss_A34[playerIndex] = 0;
        playerIndex++;
    }
    object = omAddObjEx(lbl_1_bss_B00, 4096, 71, 0, -1, fn_1_5F58);
    lbl_1_bss_A30 = object;
    object->stat |= (1 << 8);
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;

    playerIndex = 0;
    while (playerIndex < 71) {
        object->mdlId[playerIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_2A8[playerIndex],
                                 HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(object->mdlId[playerIndex], 1);
        Hu3DModelLayerSet(object->mdlId[playerIndex], 1);
        modelIndex = 0;
        while (modelIndex < 12) {
            if (modelIndex + 23 == playerIndex) {
                Hu3DModelLayerSet(object->mdlId[playerIndex], 3);
            }
            if (modelIndex + 35 == playerIndex) {
                Hu3DModelLayerSet(object->mdlId[playerIndex], 4);
            }
            if (modelIndex + 47 == playerIndex) {
                Hu3DModelLayerSet(object->mdlId[playerIndex], 5);
            }
            if (modelIndex + 59 == playerIndex) {
                Hu3DModelLayerSet(object->mdlId[playerIndex], 6);
            }
            modelIndex++;
        }
        if ((playerIndex != 7) && (playerIndex != 8) && (playerIndex != 9) &&
            (playerIndex != 10) && (playerIndex != 11) && (playerIndex != 12) &&
            (playerIndex != 13) && (playerIndex != 14)) {
            Hu3DModelAttrSet(object->mdlId[playerIndex], HU3D_MOTATTR_LOOP);
        }
        playerIndex++;
    }
    Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE |
                     HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                     HU3D_MOTATTR_OVL_PAUSE);
    Hu3DMotionSpeedSet(object->mdlId[1], (0.5f));

    playerIndex = 0;
    while (playerIndex < 4) {
        Hu3DModelHookSet(object->mdlId[0], lbl_1_data_440[playerIndex],
                         object->mdlId[playerIndex + 3]);
        Hu3DModelAttrSet(object->mdlId[playerIndex + 3], HU3D_MOTATTR_PAUSE |
                         HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                         HU3D_MOTATTR_OVL_PAUSE);
        Hu3DModelObjPosGet(object->mdlId[0], lbl_1_data_400[playerIndex],
                           &lbl_1_bss_A3C[playerIndex]);
        Hu3DModelPosSetV(object->mdlId[playerIndex + 11],
                         &lbl_1_bss_A3C[playerIndex]);
        Hu3DModelPosSetV(object->mdlId[playerIndex + 7],
                         &lbl_1_bss_A3C[playerIndex]);
        for (modelIndex = 0; modelIndex < 2; modelIndex++) {
            Hu3DModelAttrSet(
                object->mdlId[(playerIndex * 2) + 15 + modelIndex], HU3D_ATTR_DISPOFF);
        }
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            playerPosition.x = lbl_1_data_5A4[2][modelIndex].x +
                               lbl_1_bss_A3C[playerIndex].x;
            playerPosition.y = lbl_1_data_5A4[2][modelIndex].y +
                               lbl_1_bss_A3C[playerIndex].y;
            playerPosition.z = (6.0f) +
                               lbl_1_bss_A3C[playerIndex].z;
            Hu3DModelPosSetV(
                object->mdlId[(playerIndex * 12) + 23 + modelIndex],
                &playerPosition);
            Hu3DModelAttrSet(
                object->mdlId[(playerIndex * 12) + 23 + modelIndex], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(
                object->mdlId[(playerIndex * 12) + 23 + modelIndex],
                HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                    HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        }
        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            playerPosition.x = lbl_1_bss_A3C[playerIndex].x +
                               lbl_1_data_5A4[modelIndex][2].x;
            playerPosition.y = lbl_1_bss_A3C[playerIndex].y +
                               lbl_1_data_5A4[modelIndex][2].y;
            playerPosition.z = (6.0f) +
                               lbl_1_bss_A3C[playerIndex].z;
            Hu3DModelPosSetV(
                object->mdlId[(playerIndex * 12) + 28 + modelIndex],
                &playerPosition);
            Hu3DModelAttrSet(
                object->mdlId[(playerIndex * 12) + 28 + modelIndex], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(
                object->mdlId[(playerIndex * 12) + 28 + modelIndex],
                HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                    HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
        }
        playerPosition = lbl_1_bss_A3C[playerIndex];
        playerPosition.z += (6.0f);
        Hu3DModelPosSetV(
            object->mdlId[(playerIndex * 12) + 33], &playerPosition);
        Hu3DModelPosSetV(
            object->mdlId[(playerIndex * 12) + 34], &playerPosition);
        Hu3DModelRotSet(object->mdlId[(playerIndex * 12) + 34],
                        (0.0f), (0.0f), (270.0f));
        Hu3DModelAttrSet(object->mdlId[(playerIndex * 12) + 33], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(object->mdlId[(playerIndex * 12) + 34], HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(
            object->mdlId[(playerIndex * 12) + 33], HU3D_MOTATTR_PAUSE |
                HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                HU3D_MOTATTR_OVL_PAUSE);
        Hu3DModelAttrSet(
            object->mdlId[(playerIndex * 12) + 34], HU3D_MOTATTR_PAUSE |
                HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                HU3D_MOTATTR_OVL_PAUSE);
        playerIndex++;
    }

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        lbl_1_data_3C4[0], HU_MEMNUM_OVL, HEAP_MODEL));
    animationId = Hu3DAnimCreate(animation, object->mdlId[12], "ha_number01");
    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        lbl_1_data_3C4[1], HU_MEMNUM_OVL, HEAP_MODEL));
    animationId = Hu3DAnimCreate(animation, object->mdlId[13], "ha_number01");
    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        lbl_1_data_3C4[2], HU_MEMNUM_OVL, HEAP_MODEL));
    animationId = Hu3DAnimCreate(animation, object->mdlId[14], "ha_number01");
    fn_1_6FE4();
    fn_1_77F0(0, object->mdlId[3], "suuji_dummy");
    fn_1_77F0(1, object->mdlId[4], "suuji_dummy");
    fn_1_77F0(2, object->mdlId[5], "suuji_dummy");
    fn_1_77F0(3, object->mdlId[6], "suuji_dummy");
    fn_1_A3A0();
    fn_1_B2F8();
}

/* Called when board state is prepared; assigns cell order and positions each player's board
 * pieces. */
void fn_1_4300(s16 eventNo) {
    s16 availableCells[30];
    s16 cellIndex;
    s16 boardIndex;
    s16 randomBoardIndex;
    s16 remainingCells;
    s16 cellNo;
    OMOBJ *object;
    HSF_OBJECT *modelObject;

    object = lbl_1_bss_A30;
    if (eventNo == 0) {

        randomBoardIndex = 0;
        while (randomBoardIndex < 4) {
            remainingCells = 30;
            for (boardIndex = 0; boardIndex < 30; boardIndex++) {
                availableCells[boardIndex] = 1;
            }
            for (boardIndex = 0; boardIndex < 25; boardIndex++) {
                cellNo = frandmod(remainingCells);
                for (cellIndex = 0; cellIndex < 30; cellIndex++) {
                    if (availableCells[cellIndex] != 0) {
                        cellNo--;
                        if (cellNo < 0) {
                            availableCells[cellIndex] = 0;
                            break;
                        }
                    }
                }
                MgModeWork[BINGO_BOARD_WORK_BASE + randomBoardIndex * 25 + boardIndex] = cellIndex;
                remainingCells--;
            }
            // The center is a free space on every newly generated card.
            MgModeWork[(randomBoardIndex * 25) + 113] |= BINGO_CELL_MARKED_FLAG;
            randomBoardIndex++;
        }
    }

    for (cellIndex = 0; cellIndex < 4; cellIndex++) {
        for (boardIndex = 0; boardIndex < 30; boardIndex++) {
            lbl_1_bss_57E[cellIndex][boardIndex] = cellIndex * 90;
            lbl_1_bss_48E[cellIndex][boardIndex] = 0;
            lbl_1_bss_39E[cellIndex][boardIndex] = 45;
        }
    }

    for (boardIndex = 0; boardIndex < 4; boardIndex++) {
        for (cellIndex = 0; cellIndex < 30; cellIndex++) {
            lbl_1_bss_850[boardIndex][cellIndex] = Hu3DModelObjPtrGet(
                object->mdlId[boardIndex + 11], lbl_1_data_51C[cellIndex]);
            lbl_1_bss_670[boardIndex][cellIndex] = Hu3DModelObjPtrGet(
                object->mdlId[boardIndex + 7], lbl_1_data_51C[cellIndex]);
        }
    }

    for (boardIndex = 0; boardIndex < 4; boardIndex++) {
        modelObject = Hu3DModelObjPtrGet(object->mdlId[boardIndex + 11],
                                         "f-ha_hoshi");
        modelObject->mesh.base.pos.z = (-4.0f);
        modelObject = Hu3DModelObjPtrGet(object->mdlId[boardIndex + 7],
                                         "f-ha_hoshi");
        modelObject->mesh.base.pos.z = (4.0f);
        for (cellIndex = 0; cellIndex < 30; cellIndex++) {
            availableCells[cellIndex] = 1;
        }
        for (cellIndex = 0; cellIndex < 25; cellIndex++) {
            availableCells[MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                           BINGO_CELL_NUMBER_MASK] = 0;
            lbl_1_bss_850[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.x = lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].x;
            lbl_1_bss_850[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.y = lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].y;
            lbl_1_bss_850[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.z =
                (2.0f) + lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].z;
            if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                 BINGO_CELL_MARKED_FLAG) != 0) {
                lbl_1_bss_850[boardIndex]
                             [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                              BINGO_CELL_NUMBER_MASK]
                                 ->mesh.base.pos.y = (-1000.0f);
                lbl_1_bss_57E[boardIndex]
                             [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                              BINGO_CELL_NUMBER_MASK] = BINGO_CELL_REMOVED_PHASE;
                lbl_1_bss_39E[boardIndex]
                             [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                              BINGO_CELL_NUMBER_MASK] = 0;
            }
            lbl_1_bss_670[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.x = lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].x;
            lbl_1_bss_670[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.y = lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].y;
            lbl_1_bss_670[boardIndex]
                         [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                          BINGO_CELL_NUMBER_MASK]
                             ->mesh.base.pos.z = lbl_1_data_5A4[cellIndex / 5][cellIndex % 5].z;
            if (cellIndex == 12) {
                lbl_1_bss_670[boardIndex]
                             [MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                              BINGO_CELL_NUMBER_MASK]
                                 ->mesh.base.pos.y = (-1000.0f);
            }
        }
        for (cellIndex = 0; cellIndex < 30; cellIndex++) {
            if (availableCells[cellIndex] != 0) {
                lbl_1_bss_850[boardIndex][cellIndex]->mesh.base.pos.y = (-1000.0f);
                lbl_1_bss_670[boardIndex][cellIndex]->mesh.base.pos.y = (-1000.0f);
                lbl_1_bss_57E[boardIndex][cellIndex] = BINGO_CELL_REMOVED_PHASE;
                lbl_1_bss_39E[boardIndex][cellIndex] = 0;
            }
        }
    }

    for (boardIndex = 0; boardIndex < 4; boardIndex++) {
        for (cellIndex = 0; cellIndex < 12; cellIndex++) {
            if ((s16)fn_1_8838(boardIndex, cellIndex) != 0) {
                Hu3DModelAttrReset(
                    object->mdlId[boardIndex * 12 + 23 + cellIndex], 1U);
                Hu3DModelAttrSet(
                    object->mdlId[boardIndex * 12 + 23 + cellIndex],
                HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                    HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                Hu3DMotionTimeSet(
                    object->mdlId[boardIndex * 12 + 23 + cellIndex],
                    Hu3DMotionMaxTimeGet(
                        object->mdlId[boardIndex * 12 + 23 + cellIndex]));
            }
        }
    }
}

Point3d lbl_1_data_6F4 = {-25.0f, 25.0f, 14.0f};

s32 lbl_1_data_700[5] = {
    -1, -1, DATANUM(DATA_mgmbingo, 28), DATANUM(DATA_mgmbingo, 29),
    DATANUM(DATA_mgmbingo, 30)
};

s32 lbl_1_data_714[14] = {
    DATANUM(DATA_mgmbingo, 31), DATANUM(DATA_mgmbingo, 32),
    DATANUM(DATA_mgmbingo, 33), DATANUM(DATA_mgmbingo, 34),
    DATANUM(DATA_mgmbingo, 35), DATANUM(DATA_mgmbingo, 36),
    DATANUM(DATA_mgmbingo, 37), DATANUM(DATA_mgmbingo, 38),
    DATANUM(DATA_mgmbingo, 39), DATANUM(DATA_mgmbingo, 40),
    DATANUM(DATA_mgmbingo, 41), DATANUM(DATA_mgmbingo, 40),
    DATANUM(DATA_mgmbingo, 40), DATANUM(DATA_mgmbingo, 40)
};

Point3d lbl_1_data_74C[5][4] = {
    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{-32.0f, 0.0f, 0.0f}, {32.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{-32.0f, -16.0f, 0.0f}, {32.0f, -16.0f, 0.0f}, {0.0f, 32.0f, 0.0f}, {0.0f, 0.0f, 0.0f}},
    {{-24.0f, -24.0f, 0.0f}, {24.0f, -24.0f, 0.0f}, {-24.0f, 24.0f, 0.0f}, {24.0f, 24.0f, 0.0f}}
};

s16 lbl_1_data_83C[4] = {-1, -1, -1, -1};

/* Places one of a player's two markers on the selected entry while a marker slot remains. */
void fn_1_4DF8(s16 playerIndex, s16 entryIndex, s16 pathIndex) {
    OMOBJ *object;
    Point3d position;
    s16 modelIndex;

    object = lbl_1_bss_A30;
    if (lbl_1_bss_A34[playerIndex] < 2) {
        position = lbl_1_bss_A3C[playerIndex];
        position.x += lbl_1_data_5A4[pathIndex][entryIndex].x;
        position.y += lbl_1_data_5A4[pathIndex][entryIndex].y;
        position.z += (6.0f);
        modelIndex = playerIndex * 2 + 15 + lbl_1_bss_A34[playerIndex];
        Hu3DModelPosSetV(object->mdlId[modelIndex], &position);
        Hu3DModelAttrReset(object->mdlId[modelIndex], 1U);
        Hu3DModelAttrReset(object->mdlId[modelIndex], HU3D_MOTATTR_PAUSE |
                           HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                           HU3D_MOTATTR_OVL_PAUSE);
        Hu3DMotionTimeSet(
            object->mdlId[modelIndex], (0.0f));
        lbl_1_bss_A34[playerIndex] += 1;
    }
}

/* During the entrance, controls all four board animations; a zero request also emits particles
 * at each board hook. Negative requests disable the animations without emitting particles. */
void fn_1_4FB4(s16 animationEnabled) {
    HuVecF objectPosition;
    HuVecF callbackPosition;
    HuVecF *callbackArgument;
    s16 objectIndex;

    lbl_1_bss_A30->work[0] = animationEnabled;
    if (animationEnabled < 0) {
        lbl_1_bss_A30->work[0] = 0;
    }
    if (animationEnabled == 0) {
        objectIndex = 0;
        while (objectIndex < 4) {
            Hu3DModelObjPosGet(
                *lbl_1_bss_A30->mdlId,
                lbl_1_data_440[objectIndex],
                &objectPosition);
            objectPosition.z -= (50.0f);
            callbackPosition = objectPosition;
            callbackArgument = &callbackPosition;
            fn_1_AAC8(callbackArgument);
            objectIndex++;
        }
    }
}

/* Called to let the base board model continue past its current motion end. */
void fn_1_50B4(void) {
    lbl_1_bss_A30->work[1] = 1;
}

/* Called to pause the base board model when its current motion reaches the end. */
void fn_1_50CC(void) {
    lbl_1_bss_A30->work[1] = 0;
}

/* Called by selection and result handlers to place a marker at an entry's row and column. */
void fn_1_50E4(s16 playerIndex, s16 entryIndex) {
    MgModeWork[BINGO_BOARD_WORK_BASE + playerIndex * 25 + entryIndex] |= BINGO_CELL_MARKED_FLAG;
    fn_1_4DF8(playerIndex, entryIndex % 5, entryIndex / 5);
}

/* Used by board input and scoring to test whether an entry is marked for a player. */
u8 fn_1_5358(s16 groupNo, s16 entryNo) {
    return MgModeWork[BINGO_BOARD_WORK_BASE + groupNo * 25 + entryNo] & BINGO_CELL_MARKED_FLAG;
}

/* Returns the board position of an entry for the drawing and movement routines. */
void fn_1_5384(s16 groupNo, s16 entryNo, Point3d *out) {
    out->x = lbl_1_bss_A3C[groupNo].x + lbl_1_data_5A4[entryNo / 5][entryNo % 5].x;
    out->y = lbl_1_bss_A3C[groupNo].y + lbl_1_data_5A4[entryNo / 5][entryNo % 5].y;
    out->z = lbl_1_bss_A3C[groupNo].z + lbl_1_data_5A4[entryNo / 5][entryNo % 5].z;
}

/* Called after a mark to mark the same bingo number on other boards and rumble their human
 * players. */
void fn_1_54FC(s16 playerNo) {
    s16 entryIndex;
    s16 selectedPlayer;
    s16 boardIndex;

    selectedPlayer = playerNo;
    if (selectedPlayer < 0) {
        selectedPlayer = 0;
    }
    if (selectedPlayer >= 4) {
        selectedPlayer = 3;
    }
    for (entryIndex = 0; entryIndex < 25; entryIndex++) {
        if (fn_1_5358(selectedPlayer, entryIndex) && (entryIndex != 12)) {
            for (boardIndex = 0; boardIndex < 25; boardIndex++) {
                if ((playerNo != 0) &&
                    (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex] ==
                     (s32) (MgModeWork[BINGO_BOARD_WORK_BASE + selectedPlayer * 25 + entryIndex] &
                            BINGO_CELL_NUMBER_MASK))) {
                    fn_1_50E4(0, boardIndex);
                    if (fn_1_CF30(0) == 0) {
                        omVibrate(0, 20, 4, 4);
                    }
                }
                if ((playerNo != 1) &&
                    (MgModeWork[126 + boardIndex] ==
                     (s32) (MgModeWork[BINGO_BOARD_WORK_BASE + selectedPlayer * 25 + entryIndex] &
                            BINGO_CELL_NUMBER_MASK))) {
                    fn_1_50E4(1, boardIndex);
                    if (fn_1_CF30(1) == 0) {
                        omVibrate(1, 20, 4, 4);
                    }
                }
                if ((playerNo != 2) &&
                    (MgModeWork[151 + boardIndex] ==
                     (s32) (MgModeWork[BINGO_BOARD_WORK_BASE + selectedPlayer * 25 + entryIndex] &
                            BINGO_CELL_NUMBER_MASK))) {
                    fn_1_50E4(2, boardIndex);
                    if (fn_1_CF30(2) == 0) {
                        omVibrate(2, 20, 4, 4);
                    }
                }
                if ((playerNo != 3) &&
                    (MgModeWork[176 + boardIndex] ==
                     (s32) (MgModeWork[BINGO_BOARD_WORK_BASE + selectedPlayer * 25 + entryIndex] &
                            BINGO_CELL_NUMBER_MASK))) {
                    fn_1_50E4(3, boardIndex);
                    if (fn_1_CF30(3) == 0) {
                        omVibrate(3, 20, 4, 4);
                    }
                }
            }
        }
    }
    fn_1_799C(1);
    HuPrcVSleep();
    fn_1_799C(0);
    HuPrcSleep(60);
}

/* Advances the bingo boards' motions and checks player progress each frame. */
void fn_1_5F58(OMOBJ *object) {
    s16 boardIndex;
    s16 cellIndex;
    f32 value;
    f32 motionMaxTime;
    s16 indexValue;

    if (object->work[1] != 0) {
        Hu3DModelAttrReset(object->mdlId[0], HU3D_MOTATTR_PAUSE |
                           HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                           HU3D_MOTATTR_OVL_PAUSE);
    } else {
        if ((Hu3DData[object->mdlId[0]].motWork.speed + Hu3DMotionTimeGet(object->mdlId[0])) >=
            (motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[0]))) {
            Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE |
                             HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                             HU3D_MOTATTR_OVL_PAUSE);
        }
    }

    if (lbl_1_bss_F0 == 0) {
        for (boardIndex = 0; boardIndex < 4; boardIndex++) {
            for (cellIndex = 0; cellIndex < 12; cellIndex++) {
                indexValue = boardIndex * 12 + 23 + cellIndex;
                if ((Hu3DModelAttrGet(
                         object->mdlId[indexValue]) &
                     1U) != 0) {
                    if ((s16)fn_1_8838(boardIndex, cellIndex) != 0) {
                        Hu3DModelAttrReset(
                            object->mdlId[indexValue],
                            1U);
                        Hu3DModelAttrReset(
                            object->mdlId[indexValue],
                            HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                        Hu3DMotionTimeSet(
                            object->mdlId[indexValue],
                            (0.0f));
            HuAudFXPlay(MGM_BINGO_SOUND_MARKER_ACTIVATION);
                        if (fn_1_CF30(boardIndex) == 0) {
                            omVibrate(boardIndex, 20, 20, 0);
                        }
                    }
                } else {
                    if ((Hu3DData[object->mdlId[indexValue]].motWork.speed +
                         Hu3DMotionTimeGet(object->mdlId[indexValue])) >=
                        (motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[indexValue]))) {
                        Hu3DModelAttrReset(
                            object->mdlId[indexValue],
                            1U);
                        Hu3DModelAttrSet(
                            object->mdlId[indexValue],
                            HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
                        Hu3DMotionTimeSet(
                            object->mdlId[indexValue],
                         Hu3DMotionMaxTimeGet(
                                object->mdlId[indexValue]));
                    }
                }
            }
        }

        for (boardIndex = 0; boardIndex < 4; boardIndex++) {
            for (cellIndex = 0; cellIndex < 25; cellIndex++) {
                if (((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                      BINGO_CELL_MARKED_FLAG) != 0) &&
                    (lbl_1_bss_48E[boardIndex][(
                         MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                         BINGO_CELL_NUMBER_MASK)] <= 0)) {
                    lbl_1_bss_48E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)] = 20;
                }
                if (((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                      BINGO_CELL_MARKED_FLAG) == 0) &&
                    (lbl_1_bss_48E[boardIndex][(
                         MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                         BINGO_CELL_NUMBER_MASK)] <= 0) &&
                    (frandmod(1000) == 0)) {
                    lbl_1_bss_48E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)] = 20;
                }
            }
        }

        for (boardIndex = 0; boardIndex < 4; boardIndex++) {
            if (object->work[0] != 0) {
                Hu3DModelAttrReset(object->mdlId[boardIndex + 3],
                                   HU3D_MOTATTR_PAUSE | HU3D_MOTATTR_SHIFT_PAUSE |
                                       HU3D_MOTATTR_SHAPE_PAUSE | HU3D_MOTATTR_OVL_PAUSE);
            } else if (Hu3DMotionShiftIDGet(object->mdlId[boardIndex + 3]) < 0) {
                if ((Hu3DData[object->mdlId[boardIndex + 3]].motWork.speed +
                     Hu3DMotionTimeGet(object->mdlId[boardIndex + 3])) >=
                    (motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[boardIndex + 3]))) {
                    Hu3DModelAttrSet(object->mdlId[boardIndex + 3], HU3D_MOTATTR_PAUSE |
                                     HU3D_MOTATTR_SHIFT_PAUSE | HU3D_MOTATTR_SHAPE_PAUSE |
                                     HU3D_MOTATTR_OVL_PAUSE);
                    Hu3DMotionTimeSet(object->mdlId[boardIndex + 3], (0.0f));
                }
            }
        }

        for (boardIndex = 0; boardIndex < 4; boardIndex++) {
            for (cellIndex = 0; cellIndex < 25; cellIndex++) {
                if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                     BINGO_CELL_MARKED_FLAG) != 0) {
                    lbl_1_bss_39E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]--;
                    if (lbl_1_bss_39E[boardIndex][(
                            MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                            BINGO_CELL_NUMBER_MASK)] <= 0) {
                        lbl_1_bss_39E[boardIndex][(
                            MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                            BINGO_CELL_NUMBER_MASK)] = 0;
                        lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                              boardIndex * 25 + cellIndex] &
                                                   BINGO_CELL_NUMBER_MASK)]
                            ->mesh.base.pos.z = (50.0f);
                        lbl_1_bss_57E[boardIndex][(
                            MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                            BINGO_CELL_NUMBER_MASK)] += 2;
                        if (lbl_1_bss_57E[boardIndex][(
                                MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                BINGO_CELL_NUMBER_MASK)] >= 2500) {
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.pos.y = (-1000.0f);
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.scale.x = (0.0f);
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.scale.y = (0.0f);
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.scale.z = (0.0f);
                            lbl_1_bss_57E[boardIndex][(
                                MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                BINGO_CELL_NUMBER_MASK)] = 2500;
                        } else {
                            value = (f32) ((1.5) *
                                           sin(((3.141592653589793) *
                                                (f64) (lbl_1_bss_57E[boardIndex][(
                                                           MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                      boardIndex * 25 + cellIndex] &
                                                           BINGO_CELL_NUMBER_MASK)] %
                                                       360)) /
                                               (180.0)));
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.pos.x += value;
                            value = (f32) ((1.7000000476837158) *
                                           sin(((3.141592653589793) *
                                                (f64) (lbl_1_bss_57E[boardIndex][(
                                                           MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                      boardIndex * 25 + cellIndex] &
                                                           BINGO_CELL_NUMBER_MASK)] %
                                                       360)) /
                                               (180.0)));
                            if (value > (0.0f)) {
                                value *= (-1.0f);
                            }
                            lbl_1_bss_850[boardIndex][(MgModeWork[BINGO_BOARD_WORK_BASE +
                                                                  boardIndex * 25 + cellIndex] &
                                                       BINGO_CELL_NUMBER_MASK)]
                                ->mesh.base.pos.y += value;
                        }
                    }
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.x = (1.0f);
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.y = (1.0f);
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.z = (1.0f);
                } else {
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.x = (0.0f);
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.y = (0.0f);
                    lbl_1_bss_670[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)]
                                     ->mesh.base.scale.z = (0.0f);
                }
                indexValue =
                    lbl_1_bss_48E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)] %
                    10;
                indexValue *= 2;
                if (indexValue > 10) {
                    if (indexValue <= 20) {
                        indexValue = 10 - (indexValue - 10);
                    } else if (indexValue <= 30) {
                        indexValue = (indexValue - 20) * -1;
                    } else {
                        indexValue = (10 - (indexValue - 30)) * -1;
                    }
                }
                lbl_1_bss_850[boardIndex]
                             [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                               BINGO_CELL_NUMBER_MASK)]
                                 ->mesh.base.rot.z = indexValue;
                lbl_1_bss_48E[boardIndex]
                             [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                               BINGO_CELL_NUMBER_MASK)]--;
                if (lbl_1_bss_48E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)] <= 0) {
                    lbl_1_bss_48E[boardIndex]
                                 [(MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + cellIndex] &
                                   BINGO_CELL_NUMBER_MASK)] = 0;
                }
            }
        }

        for (boardIndex = 0; boardIndex < 4; boardIndex++) {
            for (cellIndex = 0; cellIndex < 2; cellIndex++) {
                indexValue = boardIndex * 2 + 15 + cellIndex;
                if ((Hu3DData[object->mdlId[indexValue]].motWork.speed +
                     Hu3DMotionTimeGet(object->mdlId[indexValue])) >=
                    (motionMaxTime = Hu3DMotionMaxTimeGet(object->mdlId[indexValue]))) {
                    Hu3DModelAttrSet(object->mdlId[indexValue], 1U);
                }
            }
            lbl_1_bss_A34[boardIndex] = 0;
        }
    }
}

/* Called during board setup to create and configure the shared board model. */
void fn_1_6FE4(void) {
    lbl_1_bss_39C = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmbingo, 49), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_39C, 1U);
    Hu3DModelLayerSet(lbl_1_bss_39C, 2);
    Hu3DModelAttrSet(lbl_1_bss_39C, HU3D_MOTATTR_LOOP);
    Hu3DModelScaleSet(lbl_1_bss_39C, (0.25f), (0.25f), (0.25f));
    Hu3DModelAttrSet(lbl_1_bss_39C, 1U);
}

/* Reads the active player's cursor input and confirms a board entry. */
void fn_1_70AC(s16 playerIndex) {
    s16 cursorCell = 12;
    s16 shouldContinue = 1;
    s16 hasMoved;
    s16 row;
    s16 column;
    Point3d cursorPosition;
    Point3d modelPosition;
    u16 buttons;

    while (shouldContinue != 0) {
        hasMoved = 0;
        cursorPosition.x = lbl_1_bss_A3C[playerIndex].x +
                           lbl_1_data_5A4[cursorCell / 5][cursorCell % 5].x;
        cursorPosition.y = lbl_1_bss_A3C[playerIndex].y +
                           lbl_1_data_5A4[cursorCell / 5][cursorCell % 5].y;
        cursorPosition.z = lbl_1_bss_A3C[playerIndex].z +
                           lbl_1_data_5A4[cursorCell / 5][cursorCell % 5].z;
        PSVECAdd(&cursorPosition, &lbl_1_data_6F4, &modelPosition);
        Hu3DModelPosSetV(lbl_1_bss_39C, &modelPosition);
        Hu3DModelAttrReset(lbl_1_bss_39C, 1);
        HuPrcVSleep();
        buttons = fn_1_C8B8(playerIndex, cursorCell);
        if ((buttons & PAD_BUTTON_A) &&
            !fn_1_5358(playerIndex, cursorCell)) {
            shouldContinue = 0;
            HuAudFXPlay(MGM_BINGO_SOUND_CURSOR_CONFIRM);
        } else if ((buttons & PAD_BUTTON_A) &&
                   fn_1_5358(playerIndex, cursorCell)) {
            HuAudFXPlay(MSM_SE_CMN_05);
        } else if (buttons & PAD_TRIGGER_R) {
            Hu3DModelAttrSet(lbl_1_bss_39C, 1);
            fn_1_347C();
            fn_1_108C(1, 30);
            HuPrcSleep(30);
            fn_1_30C8();
            shouldContinue = 1;
            while (shouldContinue != 0) {
                buttons = fn_1_C8B8(playerIndex, cursorCell);
                if (buttons & PAD_TRIGGER_R) {
                    shouldContinue = 0;
                }
                HuPrcVSleep();
            }
            shouldContinue = 1;
            fn_1_347C();
            if (playerIndex == 0) {
                fn_1_108C(2, 30);
            } else if (playerIndex == 1) {
                fn_1_108C(3, 30);
            } else if (playerIndex == 2) {
                fn_1_108C(4, 30);
            } else {
                fn_1_108C(5, 30);
            }
            HuPrcSleep(30);
            Hu3DModelAttrReset(lbl_1_bss_39C, 1);
            fn_1_3074();
        } else {
            row = cursorCell / 5;
            column = cursorCell % 5;
            if (buttons & 1) {
                column--;
                hasMoved = 1;
            }
            if (buttons & 2) {
                column++;
                hasMoved = 1;
            }
            if (buttons & 8) {
                row--;
                hasMoved = 1;
            }
            if (buttons & 4) {
                row++;
                hasMoved = 1;
            }
            if (row < 0) {
                row += 5;
            }
            if (row >= 5) {
                row -= 5;
            }
            if (column < 0) {
                column += 5;
            }
            if (column >= 5) {
                column -= 5;
            }
            cursorCell = column + (row * 5);
            if (hasMoved != 0) {
                HuAudFXPlay(MGM_BINGO_SOUND_CURSOR_MOVE);
            }
        }
    }

    Hu3DModelAttrSet(lbl_1_bss_39C, 1);
    if (fn_1_CF30(playerIndex) == 0) {
        omVibrate(playerIndex, 20, 4, 4);
    }
    fn_1_799C(1);
    MgModeWork[BINGO_BOARD_WORK_BASE + playerIndex * 25 + cursorCell] |= BINGO_CELL_MARKED_FLAG;
    fn_1_4DF8(playerIndex, cursorCell % 5, cursorCell / 5);
    HuPrcVSleep();
    fn_1_799C(0);
    HuPrcSleep(60);
}

/* During setup, attaches the player's number animation to the supplied model texture and
 * saves the initial card cells for later progress comparisons. */
void fn_1_77F0(s16 playerIndex, s16 modelId, char *animationName) {
    s16 keyFrame;

    lbl_1_bss_388[playerIndex] = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_37E[playerIndex] = Hu3DAnimCreate(
        lbl_1_bss_388[playerIndex], modelId, animationName);
    if (playerIndex == 0) {
        lbl_1_bss_398 = omAddObjEx(
            lbl_1_bss_B00, 4096, 0, 0, -1, fn_1_79AC);
        lbl_1_bss_398->stat |= (1 << 8);
        lbl_1_bss_398->work[0] = lbl_1_bss_398->work[1] =
            lbl_1_bss_398->work[2] = lbl_1_bss_398->work[3] = 0;
    }
    keyFrame = 0;
    while (keyFrame < 25) {
        lbl_1_bss_2B6[playerIndex].cells[keyFrame] =
            MgModeWork[BINGO_BOARD_WORK_BASE + playerIndex * 25 + keyFrame];
        keyFrame++;
    }
}

/* Enables or disables refreshing completed-line totals and the players' number displays. */
void fn_1_799C(s16 enable)
{
    lbl_1_bss_F2 = enable;
}

/* Updates each player's displayed number from the current mode values each frame. */
void fn_1_79AC(OMOBJ *object) {
    s16 playerIndex;

    if (lbl_1_bss_F2 != 0) {
        playerIndex = 0;
        while (playerIndex < 4) {
            MgModeWorkInt[playerIndex] = (s32) fn_1_7AD0(playerIndex);
            if ((s32) MgModeWorkInt[playerIndex] < 0) {
                MgModeWorkInt[playerIndex] = 0;
            }
            if ((s32) MgModeWorkInt[playerIndex] > 5) {
                MgModeWorkInt[playerIndex] = 5;
            }
            Hu3DAnimBankSet(lbl_1_bss_37E[playerIndex], (u16) MgModeWorkInt[playerIndex]);
            playerIndex += 1;
        }
    }
}

/* Counts completed rows, columns and diagonals on the player's current card, capped at five. */
s16 fn_1_7AD0(s16 playerIndex) {
    s16 completeLineCount;
    s16 lineIndex;
    s16 boardIndex;

    boardIndex = playerIndex;
    completeLineCount = 0;
    lineIndex = 0;
    while (lineIndex < 5) {
        if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 0] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 1] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 2] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 3] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 4] &
             BINGO_CELL_MARKED_FLAG)) {
            completeLineCount++;
        }
        if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 0] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 5] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 10] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 15] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 20] &
             BINGO_CELL_MARKED_FLAG)) {
            completeLineCount++;
        }
        lineIndex++;
    }

    if ((MgModeWork[boardIndex * 25 + BINGO_BOARD_WORK_BASE] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 107] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 113] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 119] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 125] & BINGO_CELL_MARKED_FLAG)) {
        completeLineCount++;
    }
    if ((MgModeWork[boardIndex * 25 + 105] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 109] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 113] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 117] & BINGO_CELL_MARKED_FLAG) &&
        (MgModeWork[boardIndex * 25 + 121] & BINGO_CELL_MARKED_FLAG)) {
        completeLineCount++;
    }
    if (completeLineCount > 5) {
        completeLineCount = 5;
    }
    return completeLineCount;
}

/* Counts completed lines in the saved card, capped at five, for comparison with current
 * progress. */
s16 fn_1_7E6C(s16 playerIndex) {
    s16 boardIndex;
    s16 completeLineCount;
    s16 lineIndex;

    boardIndex = playerIndex;
    completeLineCount = 0;
    lineIndex = 0;
    while (lineIndex < 5) {
        if (lbl_1_bss_2B6[boardIndex].cells[lineIndex * 5] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex * 5 + 1] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex * 5 + 2] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex * 5 + 3] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex * 5 + 4] & BINGO_CELL_MARKED_FLAG) {
            completeLineCount++;
        }
        if (lbl_1_bss_2B6[boardIndex].cells[lineIndex] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex + 5] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex + 10] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex + 15] & BINGO_CELL_MARKED_FLAG &&
            lbl_1_bss_2B6[boardIndex].cells[lineIndex + 20] & BINGO_CELL_MARKED_FLAG) {
            completeLineCount++;
        }
        lineIndex++;
    }
    if (lbl_1_bss_2B6[boardIndex].cells[0] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[6] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[12] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[18] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[24] & BINGO_CELL_MARKED_FLAG) {
        completeLineCount++;
    }
    if (lbl_1_bss_2B6[boardIndex].cells[4] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[8] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[12] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[16] & BINGO_CELL_MARKED_FLAG &&
        lbl_1_bss_2B6[boardIndex].cells[20] & BINGO_CELL_MARKED_FLAG) {
        completeLineCount++;
    }
    if (completeLineCount > 5) {
        completeLineCount = 5;
    }
    return completeLineCount;
}

/* For reach messages, returns the most lines a single remaining mark could complete on the current
 * card. */
s16 fn_1_8250(s16 playerIndex) {
    typedef struct {
        u8 otherModeWork[101]; /* Mode work bytes before the board records. */
        u8 boardCellFlags[155]; /* 25 cell bytes for each of four boards. */
    } MgmbingoWorkView;
    u8 lineMarks[25];
    s16 row;
    s16 candidate;
    s16 boardIndex = playerIndex;
    s16 markedCellCount;
    s16 maximumLineCount = 0;
    memset(lineMarks, 0, 25U);
    for (row = 0; row < 5; row++)
    {
        s16 column;
        markedCellCount = 0;
        for (column = 0; column < 5; column++)
        {
            if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + column] &
                BINGO_CELL_MARKED_FLAG) {
                markedCellCount++;
            } else {
                candidate = column;
            }
        }
        if (markedCellCount == 4)
        {
            lineMarks[row * 5 + candidate]++;
        }
        markedCellCount = 0;
        for (column = 0; column < 5; column++)
        {
            if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + column * 5 + row] &
                BINGO_CELL_MARKED_FLAG) {
                markedCellCount++;
            } else {
                candidate = column;
            }
        }
        if (markedCellCount == 4)
        {
            lineMarks[row + candidate * 5]++;
        }
    }
    markedCellCount = 0;
    for (row = 0; row < 5; row++)
    {
        if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + row] &
            BINGO_CELL_MARKED_FLAG) {
            markedCellCount++;
        } else {
            candidate = row;
        }
    }
    if (markedCellCount == 4)
    {
        lineMarks[(candidate * 5) + candidate]++;
    }
    markedCellCount = 0;
    for (row = 0; row < 5; row++)
    {
        if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + (4 - row)] &
            BINGO_CELL_MARKED_FLAG) {
            markedCellCount++;
        } else {
            candidate = row;
        }
    }
    if (markedCellCount == 4)
    {
        lineMarks[(4 - candidate) + (candidate * 5)]++;
    }
    maximumLineCount = 0;
    boardIndex = 0;
    for (; boardIndex < 25; boardIndex++)
    {
        if (maximumLineCount < lineMarks[boardIndex])
        {
            maximumLineCount = lineMarks[boardIndex];
        }
    }
    if (maximumLineCount > 5)
    {
        maximumLineCount = 5;
    }
    return maximumLineCount;
}

/* Returns the most lines a single mark could complete on the saved card, for reach-message
 * comparisons. */
s16 fn_1_853C(s16 playerIndex) {
    u8 lineMarks[25];
    s16 completed;
    s16 candidate;
    s16 boardIndex = playerIndex;
    s16 maximumLineCount = 0;
    s16 linePosition;
    memset(lineMarks, 0, 25U);
    for (linePosition = 0; linePosition < 5; linePosition++)
    {
        s16 column;
        completed = 0;
        for (column = 0; column < 5; column++)
        {
            if (lbl_1_bss_2B6[boardIndex].cells[linePosition * 5 + column] & BINGO_CELL_MARKED_FLAG)
            {
                completed++;
            }
            else
            {
                candidate = column;
            }
        }
        if (completed == 4)
        {
            lineMarks[linePosition * 5 + candidate]++;
        }
        completed = 0;
        for (column = 0; column < 5; column++)
        {
            if (lbl_1_bss_2B6[boardIndex].cells[linePosition + column * 5] & BINGO_CELL_MARKED_FLAG)
            {
                completed++;
            }
            else
            {
                candidate = column;
            }
        }
        if (completed == 4)
        {
            lineMarks[linePosition + candidate * 5]++;
        }
    }
    completed = 0;
    for (linePosition = 0; linePosition < 5; linePosition++)
    {
        if (lbl_1_bss_2B6[boardIndex].cells[linePosition * 5 + linePosition] &
            BINGO_CELL_MARKED_FLAG) {
            completed++;
        } else {
            candidate = linePosition;
        }
    }
    if (completed == 4)
    {
        lineMarks[(candidate * 5) + candidate]++;
    }
    completed = 0;
    for (linePosition = 0; linePosition < 5; linePosition++)
    {
        if (lbl_1_bss_2B6[boardIndex].cells[(4 - linePosition) + linePosition * 5] &
            BINGO_CELL_MARKED_FLAG) {
            completed++;
        } else {
            candidate = linePosition;
        }
    }
    if (completed == 4)
    {
        lineMarks[(4 - candidate) + candidate * 5]++;
    }
    maximumLineCount = 0;
    boardIndex = 0;
    for (; boardIndex < 25; boardIndex++)
    {
        if (maximumLineCount < lineMarks[boardIndex])
        {
            maximumLineCount = lineMarks[boardIndex];
        }
    }
    if (maximumLineCount > 5)
    {
        maximumLineCount = 5;
    }
    return maximumLineCount;
}

/* Tests whether a board line is complete: columns 0-4, rows 5-9, then diagonals 10 and 11. */
s32 fn_1_8838(s16 boardIndex, s16 lineIndex) {
    s32 lineComplete;

    lineComplete = 0;
    if (lineIndex < 5) {
        if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 0] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 5] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 10] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 15] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex + 20] &
             BINGO_CELL_MARKED_FLAG)) {
            lineComplete = 1;
        }
    } else if (lineIndex < 10) {
        lineIndex = lineIndex - 5;
        if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 0] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 1] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 2] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 3] &
             BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + lineIndex * 5 + 4] &
             BINGO_CELL_MARKED_FLAG)) {
            lineComplete = 1;
        }
    } else if (lineIndex == 10) {
        if ((MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[107 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[113 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[119 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[125 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG)) {
            lineComplete = 1;
        }
    } else if (lineIndex == 11) {
        if ((MgModeWork[105 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[109 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[113 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[117 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG) &&
            (MgModeWork[121 + boardIndex * 25] & BINGO_CELL_MARKED_FLAG)) {
            lineComplete = 1;
        }
    }
    return lineComplete;
}

/* Scores each unmarked cell by how many lines through it contain exactly requiredMarks marks. */
void fn_1_8BDC(s16 playerIndex, s16 requiredMarks, s16 *lineScores) {
    typedef struct {
        u8 otherModeWork[101]; /* Mode work bytes before the board records. */
        u8 boardCellFlags[155]; /* 25 cell bytes for each of four boards. */
    } MgmbingoWorkView;
    s16 boardIndex;
    s16 row;
    s16 column;
    s16 completedMarks;
    u8 unmarked[5];

    boardIndex = playerIndex;
    memset(lineScores, 0, 50);
    for (row = 0; row < 5; row++) {
        completedMarks = 0;
        memset(unmarked, 0, sizeof(unmarked));
        for (column = 0; column < 5; column++) {
            if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + column] &
                BINGO_CELL_MARKED_FLAG) {
                completedMarks++;
            } else {
                unmarked[column] = 1;
            }
        }
        if (completedMarks == requiredMarks) {
            for (column = 0; column < 5; column++) {
                if (unmarked[column] != 0) {
                    lineScores[column + (row * 5)]++;
                }
            }
        }

        completedMarks = 0;
        memset(unmarked, 0, sizeof(unmarked));
        for (column = 0; column < 5; column++) {
            if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + column * 5 + row] &
                BINGO_CELL_MARKED_FLAG) {
                completedMarks++;
            } else {
                unmarked[column] = 1;
            }
        }
        if (completedMarks == requiredMarks) {
            for (column = 0; column < 5; column++) {
                if (unmarked[column] != 0) {
                    lineScores[row + (column * 5)]++;
                }
            }
        }
    }

    completedMarks = 0;
    memset(unmarked, 0, sizeof(unmarked));
    for (row = 0; row < 5; row++) {
        if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + row] &
            BINGO_CELL_MARKED_FLAG) {
            completedMarks++;
        } else {
            unmarked[row] = 1;
        }
    }
    if (completedMarks == requiredMarks) {
        for (row = 0; row < 5; row++) {
            if (unmarked[row] != 0) {
                lineScores[(row * 5) + row]++;
            }
        }
    }

    completedMarks = 0;
    memset(unmarked, 0, sizeof(unmarked));
    for (row = 0; row < 5; row++) {
        if (MgModeWork[BINGO_BOARD_WORK_BASE + boardIndex * 25 + row * 5 + (4 - row)] &
            BINGO_CELL_MARKED_FLAG) {
            completedMarks++;
        } else {
            unmarked[row] = 1;
        }
    }
    if (completedMarks == requiredMarks) {
        for (row = 0; row < 5; row++) {
            if (unmarked[row] != 0) {
                lineScores[(4 - row) + (row * 5)]++;
            }
        }
    }
}

/* Returns the index of the highest positive line score, or -1 when none is positive. */
s16 fn_1_8FA8(s16 *scores) {
    s16 bestIndex = -1;
    s16 bestScore = 0;
    s16 scoreIndex = 0;

    while (scoreIndex < 25) {
        if (scores[scoreIndex] > bestScore) {
            bestScore = scores[scoreIndex];
            bestIndex = scoreIndex;
        }
        scoreIndex++;
    }
    return bestIndex;
}

/* Selects a random positive-scoring entry, returning -1 when no entry is available. */
s16 fn_1_901C(s16 *entryValues) {
    s16 result = -1;
    s16 entryIndex = 0;
    s16 activeEntryCount = 0;

    for (; entryIndex < 25; entryIndex += 1) {
        if (entryValues[entryIndex] > 0) {
            activeEntryCount++;
        }
    }
    if (activeEntryCount > 0) {
        activeEntryCount = (s16)frandmod(activeEntryCount);

        for (entryIndex = 0; entryIndex < 25; entryIndex += 1) {
            if ((entryValues[entryIndex] > 0) && (--activeEntryCount < 0)) {
                result = entryIndex;
                break;
            }
        }
    }
    return result;
}

/* During session setup, creates tie-break roulette sprites when two to four players have positive
 * round scores. */
void fn_1_90F4(void) {
    s16 playerCount;
    s16 player;
    s16 playerSpriteCount;

    playerCount = 0;
    player = 0;
    while (player < 4) {
        if (MgModeScore[player] > 0) {
            lbl_1_data_83C[player] = fn_1_CEF0(player);
            playerCount++;
        }
        player++;
    }
    lbl_1_bss_2B4 = playerCount;
    if ((lbl_1_bss_2B4 != 2) && (lbl_1_bss_2B4 != 3) && (lbl_1_bss_2B4 != 4)) {
        return;
    }

    lbl_1_bss_288 = HuSprGrpCreate(lbl_1_bss_2B4 + 2);
    lbl_1_bss_28C[0] = HuSprAnimRead(
        HuDataSelHeapReadNum(lbl_1_data_700[lbl_1_bss_2B4], HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_274[0] = HuSprCreate(lbl_1_bss_28C[0], 10, 0);
    HuSprGrpMemberSet(lbl_1_bss_288, 0, lbl_1_bss_274[0]);
    HuSprPosSet(lbl_1_bss_288, 0, (0.0f), (0.0f));
    HuSprScaleSet(lbl_1_bss_288, 0, (1.2f), (1.2f));

    player = 0;
    playerSpriteCount = 0;
    while (player < 4) {
        if (lbl_1_data_83C[player] >= 0) {
            lbl_1_bss_28C[playerSpriteCount + 2] = HuSprAnimRead(
                HuDataSelHeapReadNum(lbl_1_data_714[lbl_1_data_83C[player]],
                                     HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_274[playerSpriteCount + 2] = HuSprCreate(
                lbl_1_bss_28C[playerSpriteCount + 2], 9, 0);
            HuSprGrpMemberSet(lbl_1_bss_288, playerSpriteCount + 2,
                              lbl_1_bss_274[playerSpriteCount + 2]);
            HuSprPosSet(lbl_1_bss_288, playerSpriteCount + 2,
                        lbl_1_data_74C[lbl_1_bss_2B4][playerSpriteCount].x,
                        lbl_1_data_74C[lbl_1_bss_2B4][playerSpriteCount].y);
            playerSpriteCount++;
        }
        player++;
    }

    lbl_1_bss_28C[1] = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 27), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_274[1] = HuSprCreate(lbl_1_bss_28C[1], 8, 0);
    HuSprGrpMemberSet(lbl_1_bss_288, 1, lbl_1_bss_274[1]);
    HuSprData[lbl_1_bss_274[1]].data->pat->centerY = 48;
    HuSprZRotSet(lbl_1_bss_288, 1, (0.0f));
    HuSprGrpScaleSet(lbl_1_bss_288, (0.0f), (0.0f));
    HuSprGrpPosSet(lbl_1_bss_288, (288.0f), (138.4f));
}

/* Breaks a round tie with roulette, setting the chosen participant's score to 10 and the other
 * participants' scores to zero, then flashes the chosen player's sprite. */
void fn_1_9554(void) {
    f32 angle;
    f32 spinAmount;
    f32 decelerationSum;
    f32 rotation;
    s16 activePlayer;
    s16 turnIndex;

    if ((lbl_1_bss_2B4 == 2) || (lbl_1_bss_2B4 == 3) ||
        (lbl_1_bss_2B4 == 4)) {
        HuAudFXPlay(MGM_BINGO_SOUND_RESULT_REVEAL);
        angle = (0.0f);
        while (angle < (1.0f)) {
            HuSprGrpScaleSet(lbl_1_bss_288, angle, angle);
            HuPrcVSleep();
            angle += (0.1f);
        }
        HuSprGrpScaleSet(lbl_1_bss_288, (1.0f),
                         (1.0f));
        decelerationSum = (0.0f);
        angle = (18.0f);
        while (angle > (0.0f)) {
            angle -= (0.4f);
            if (angle <= (0.0f)) {
                angle = (0.0f);
            }
            decelerationSum += angle;
        }
        spinAmount = (720.0f) + (f32)(frandmod(36) * 10);
        angle = spinAmount + decelerationSum;
        angle = (f32)((s32)angle % 360);
        if (lbl_1_bss_2B4 == 2) {
            if (((0.0f) <= angle) && (angle <= (20.0f))) {
                spinAmount += (20.0f);
            } else if (((160.0) <= (f64)angle) &&
                       (angle < (180.0f))) {
                spinAmount -= (20.0f);
            } else if (((180.0f) <= angle) &&
                       (angle <= (200.0f))) {
                spinAmount += (20.0f);
            } else if (((340.0f) <= angle) &&
                       (angle <= (360.0f))) {
                spinAmount -= (20.0f);
            }
        } else if (lbl_1_bss_2B4 == 3) {
            if (((0.0f) <= angle) && (angle <= (20.0f))) {
                spinAmount += (20.0f);
            } else if (((100.0f) <= angle) &&
                       (angle < (120.0f))) {
                spinAmount -= (20.0f);
            } else if (((120.0f) <= angle) &&
                       (angle <= (140.0f))) {
                spinAmount += (20.0f);
            } else if (((220.0f) <= angle) &&
                       (angle < (240.0f))) {
                spinAmount -= (20.0f);
            } else if (((240.0f) <= angle) &&
                       (angle <= (260.0f))) {
                spinAmount += (20.0f);
            } else if (((340.0f) <= angle) &&
                       (angle <= (360.0f))) {
                spinAmount -= (20.0f);
            }
        } else {
            if (((0.0f) <= angle) && (angle <= (20.0f))) {
                spinAmount += (20.0f);
            } else if (((70.0f) <= angle) &&
                       (angle < (90.0f))) {
                spinAmount -= (20.0f);
            } else if (((90.0f) <= angle) &&
                       (angle <= (110.0f))) {
                spinAmount += (20.0f);
            } else if (((160.0f) <= angle) &&
                       (angle < (180.0f))) {
                spinAmount -= (20.0f);
            } else if (((180.0f) <= angle) &&
                       (angle <= (200.0f))) {
                spinAmount += (20.0f);
            } else if (((250.0f) <= angle) &&
                       (angle < (270.0f))) {
                spinAmount -= (20.0f);
            } else if (((270.0f) <= angle) &&
                       (angle <= (290.0f))) {
                spinAmount += (20.0f);
            } else if (((340.0f) <= angle) &&
                       (angle <= (360.0f))) {
                spinAmount -= (20.0f);
            }
        }

        lbl_1_data_AD8 = HuAudFXPlay(MGM_BINGO_SOUND_RESULT_SPIN);
        turnIndex = 1;
        rotation = angle = (0.0f);
        while (turnIndex != 0) {
            rotation += angle;
            if (rotation >= (360.0f)) {
                rotation -= (360.0f);
            }
            HuSprZRotSet(lbl_1_bss_288, 1, rotation);
            HuPrcVSleep();
            spinAmount -= angle;
            if (spinAmount > (0.0f)) {
                angle += (0.4f);
                if (angle > (18.0f)) {
                    angle = (18.0f);
                }
            } else {
                angle -= (0.4f);
                if (angle <= (0.0f)) {
                    angle = (0.0f);
                    turnIndex = 0;
                }
            }
        }
        if (lbl_1_data_AD8 >= 0) {
            HuAudFXStop(lbl_1_data_AD8);
        }
        lbl_1_data_AD8 = -1;
        HuPrcVSleep();
        HuAudFXPlay(MGM_BINGO_SOUND_RESULT_STOP);
    }

    if (lbl_1_bss_2B4 == 2) {
        if (rotation < (180.0f)) {
            turnIndex = 2;
        } else {
            turnIndex = 1;
        }
        for (activePlayer = 0; activePlayer < 4; activePlayer++) {
            if (lbl_1_data_83C[activePlayer] >= 0) {
                turnIndex--;
                if (turnIndex == 0) {
                    MgModeScore[activePlayer] = 10;
                } else {
                    MgModeScore[activePlayer] = 0;
                }
            }
        }
        if (rotation < (180.0f)) {
            turnIndex = 3;
        } else {
            turnIndex = 2;
        }
        for (activePlayer = 0; activePlayer < 100; activePlayer++) {
            if ((activePlayer / 5) % 2) {
                HuSprAttrSet(lbl_1_bss_288, turnIndex, 4);
            } else {
                HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
            }
            HuPrcVSleep();
        }
        HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
    } else if (lbl_1_bss_2B4 == 3) {
        if (rotation < (120.0f)) {
            turnIndex = 2;
        } else if (rotation < (240.0f)) {
            turnIndex = 3;
        } else {
            turnIndex = 1;
        }
        for (activePlayer = 0; activePlayer < 4; activePlayer++) {
            if (lbl_1_data_83C[activePlayer] >= 0) {
                turnIndex--;
                if (turnIndex == 0) {
                    MgModeScore[activePlayer] = 10;
                } else {
                    MgModeScore[activePlayer] = 0;
                }
            }
        }
        if (rotation < (120.0f)) {
            turnIndex = 3;
        } else if (rotation < (240.0f)) {
            turnIndex = 4;
        } else {
            turnIndex = 2;
        }
        for (activePlayer = 0; activePlayer < 100; activePlayer++) {
            if ((activePlayer / 5) % 2) {
                HuSprAttrSet(lbl_1_bss_288, turnIndex, 4);
            } else {
                HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
            }
            HuPrcVSleep();
        }
        HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
    } else if (lbl_1_bss_2B4 == 4) {
        if (rotation < (90.0f)) {
            turnIndex = 2;
        } else if (rotation < (180.0f)) {
            turnIndex = 4;
        } else if (rotation < (270.0f)) {
            turnIndex = 3;
        } else {
            turnIndex = 1;
        }
        for (activePlayer = 0; activePlayer < 4; activePlayer++) {
            if (lbl_1_data_83C[activePlayer] >= 0) {
                turnIndex--;
                if (turnIndex == 0) {
                    MgModeScore[activePlayer] = 10;
                } else {
                    MgModeScore[activePlayer] = 0;
                }
            }
        }
        if (rotation < (90.0f)) {
            turnIndex = 3;
        } else if (rotation < (180.0f)) {
            turnIndex = 5;
        } else if (rotation < (270.0f)) {
            turnIndex = 4;
        } else {
            turnIndex = 2;
        }
        for (activePlayer = 0; activePlayer < 100; activePlayer++) {
            if ((activePlayer / 5) % 2) {
                HuSprAttrSet(lbl_1_bss_288, turnIndex, 4);
            } else {
                HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
            }
            HuPrcVSleep();
        }
        HuSprAttrReset(lbl_1_bss_288, turnIndex, 4);
    }
    HuPrcVSleep();
}

/* On results with 2-4 players, plays the reveal sound, shrinks the result group, and hides it. */
void fn_1_A2A4(void) {
    f32 groupScale;

    if ((lbl_1_bss_2B4 == 2) || (lbl_1_bss_2B4 == 3) || (lbl_1_bss_2B4 == 4)) {
        HuAudFXPlay(MGM_BINGO_SOUND_RESULT_REVEAL);
        groupScale = (1.0f);
        while (groupScale > (0.0f)) {
            HuSprGrpScaleSet(lbl_1_bss_288, groupScale, groupScale);
            HuPrcVSleep();
            groupScale -= (0.1f);
        }
        HuSprGrpScaleSet(lbl_1_bss_288, (0.0f), (0.0f));
        HuSprGrpAttrSet(lbl_1_bss_288, 4);
    }
}

/* Called during board setup to prepare particle effects for bingo celebrations. */
void fn_1_A3A0(void) {
    ANIMDATA *animation;
    s16 particleIndex;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
        DATANUM(DATA_mgmbingo, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    for (particleIndex = 0; particleIndex < 20; particleIndex++) {
        HU3D_PARTICLE *particle;
        HU3D_PARTICLE_DATA *particleData;
        s16 dataIndex;

        lbl_1_bss_24C[particleIndex] = Hu3DParticleCreate(animation, 10);
        Hu3DModelPosSet(
            lbl_1_bss_24C[particleIndex], (0.0f), (0.0f),
            (0.0f));
        Hu3DModelScaleSet(
            lbl_1_bss_24C[particleIndex], (1.0f),
            (1.0f), (1.0f));
        Hu3DModelLayerSet(lbl_1_bss_24C[particleIndex], 2);
        Hu3DParticleTPLvlSet(lbl_1_bss_24C[particleIndex], (1.0f));
        Hu3DParticleBlendModeSet(lbl_1_bss_24C[particleIndex], 0);
        Hu3DParticleColSet(lbl_1_bss_24C[particleIndex], 255, 255, 255);
        Hu3DParticleScaleSet(lbl_1_bss_24C[particleIndex], (1.0f));
        Hu3DParticleHookSet(lbl_1_bss_24C[particleIndex], fn_1_A704);

        particle = Hu3DData[lbl_1_bss_24C[particleIndex]].hookData;
        particleData = particle->data;
        particle->work = 0;
        particle->dataCnt = particle->emitCnt = 0;
        particle->pos.x = particle->pos.y = particle->pos.z = (0.0f);
        particle->spawnCenter.x = particle->spawnCenter.y = particle->spawnCenter.z = (0.0f);

        for (dataIndex = 0; dataIndex < particle->maxCnt; dataIndex++) {
            particleData[dataIndex].scale = (0.0f);
            particleData[dataIndex].time = particleData[dataIndex].parManId = 0;
            particleData[dataIndex].vel.x = particleData[dataIndex].vel.y =
                particleData[dataIndex].vel.z = (0.0f);
            particleData[dataIndex].accel.x = particleData[dataIndex].accel.y =
                particleData[dataIndex].accel.z = (0.0f);
            particleData[dataIndex].speedDecay = particleData[dataIndex].colorIdx =
                particleData[dataIndex].scaleBase = (0.0f);
        }
    }
}

/* Updates celebration particles and deactivates them on reaching the burst's configured cutoff
 * height. */
void fn_1_A704(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix) {
    HU3D_PARTICLE_DATA *particleData;
    s16 particleIndex;
    s16 hasActiveParticle;
    f32 verticalWave;

    particleData = particle->data;
    particleIndex = 0;
    hasActiveParticle = 0;
    while (particleIndex < particle->maxCnt) {
        if (particleData[particleIndex].scale) {
            hasActiveParticle = 1;
            particleData[particleIndex].pos.x += particleData[particleIndex].vel.x;
            particleData[particleIndex].pos.z += particleData[particleIndex].vel.z;
            particleData[particleIndex].vel.x *= (0.9f);
            particleData[particleIndex].vel.z *= (0.9f);
            particleData[particleIndex].pos.y += particleData[particleIndex].accel.y;
            particleData[particleIndex].accel.y -= (0.4f);
            if (particleData[particleIndex].accel.y <= (0.0f)) {
                particleData[particleIndex].accel.y = (0.0f);
                verticalWave = (f32)(
                    (f64)particleData[particleIndex].colorIdx *
                    sin(((3.141592653589793) * (f64)particleData[particleIndex].time) /
                        (180.0)));
                if (verticalWave > (0.0f)) {
                    verticalWave *= (-1.0f);
                }
                particleData[particleIndex].pos.y += verticalWave;
                verticalWave = (f32)(
                    (f64)particleData[particleIndex].speedDecay *
                    sin(((3.141592653589793) * (f64)particleData[particleIndex].parManId) /
                        (180.0)));
                particleData[particleIndex].pos.x += verticalWave;
                particleData[particleIndex].time =
                    (particleData[particleIndex].time + 1) % 360;
                particleData[particleIndex].parManId =
                    (particleData[particleIndex].parManId + 1) % 360;
            }
            if (particle->pos.y >= particleData[particleIndex].pos.y) {
                particleData[particleIndex].scale = (0.0f);
            }
        }
        particleIndex++;
    }
    particle->dataCnt = hasActiveParticle;
}

/* Emits a rising particle burst at the supplied position for entrances and jumps; particles
 * disappear after falling 1000 units below that position. */
void fn_1_AAC8(HuVecF *position) {
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleDataArray;
    s16 particleIndex;
    f32 angle;
    HuVecF acceleration;
    HuVecF velocity;
    HuVecF *velocityPointer;

    particleDataArray = NULL;
    particleIndex = 0;
    while (particleIndex < 20) {
        particle = Hu3DData[lbl_1_bss_24C[particleIndex]].hookData;
        if (particle->dataCnt == 0) {
            particleDataArray = particle->data;
            break;
        }
        particleIndex++;
    }
    if (particleDataArray != NULL) {
        particle->dataCnt = 1;
        particle->pos.y = position->y - (1000.0f);
        for (particleIndex = 0; particleIndex < particle->maxCnt; particleIndex++) {
            particleDataArray[particleIndex].scale = (10.0f);
            particleDataArray[particleIndex].pos = *position;
            particleDataArray[particleIndex].time = frandmod(4) * 90;
            particleDataArray[particleIndex].parManId =
                particleDataArray[particleIndex].time;
            angle = (30.0f) * frandmod(12);
            acceleration.x = acceleration.y = (0.0f);
            acceleration.z = (1.0f);
            velocity = acceleration;
            velocityPointer = &velocity;
            fn_1_12BE0((0.0f), angle, (0.0f),
                       velocityPointer, &acceleration);
            particleDataArray[particleIndex].vel.x =
                (10.0f) * acceleration.x;
            particleDataArray[particleIndex].vel.y = (0.0f);
            particleDataArray[particleIndex].vel.z =
                (10.0f) * acceleration.z;
            particleDataArray[particleIndex].accel.x = (0.0f);
            particleDataArray[particleIndex].accel.y =
                (11.1f) + (0.1f) * frandmod(20);
            particleDataArray[particleIndex].accel.z = (0.0f);
            particleDataArray[particleIndex].speedDecay =
                (1.0f) + (0.1f) * frandmod(10);
            particleDataArray[particleIndex].colorIdx =
                (1.0f) + (0.1f) * frandmod(10);
            particleDataArray[particleIndex].scaleBase = (0.0f);
            particleDataArray[particleIndex].zRot =
                (45.0f) * frandmod(8);
        }
    }
}

/* Emits a particle burst at a star's launch position; particles disappear on falling back to that
 * height. */
void fn_1_AEE8(HuVecF *position) {
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleDataArray;
    s16 particleIndex;
    f32 angle;
    HuVecF acceleration;
    HuVecF velocity;
    HuVecF *velocityPointer;

    particleDataArray = NULL;
    particleIndex = 0;
    while (particleIndex < 20) {
        particle = Hu3DData[lbl_1_bss_24C[particleIndex]].hookData;
        if (particle->dataCnt == 0) {
            particleDataArray = particle->data;
            break;
        }
        particleIndex++;
    }
    if (particleDataArray != NULL) {
        particle->dataCnt = 1;
        particle->pos.y = position->y;
        for (particleIndex = 0; particleIndex < particle->maxCnt; particleIndex++) {
            particleDataArray[particleIndex].scale = (15.0f);
            particleDataArray[particleIndex].pos = *position;
            particleDataArray[particleIndex].time = frandmod(4) * 90;
            particleDataArray[particleIndex].parManId =
                particleDataArray[particleIndex].time;
            angle = (30.0f) * frandmod(12);
            acceleration.x = acceleration.y = (0.0f);
            acceleration.z = (1.0f);
            velocity = acceleration;
            velocityPointer = &velocity;
            fn_1_12BE0((0.0f), angle, (0.0f),
                       velocityPointer, &acceleration);
            particleDataArray[particleIndex].vel.x =
                (10.0f) * acceleration.x;
            particleDataArray[particleIndex].vel.y = (0.0f);
            particleDataArray[particleIndex].vel.z =
                (10.0f) * acceleration.z;
            particleDataArray[particleIndex].accel.x = (0.0f);
            particleDataArray[particleIndex].accel.y =
                (11.1f) + (0.1f) * frandmod(20);
            particleDataArray[particleIndex].accel.z = (0.0f);
            particleDataArray[particleIndex].speedDecay =
                (1.0f) + (0.1f) * frandmod(10);
            particleDataArray[particleIndex].colorIdx =
                (1.0f) + (0.1f) * frandmod(10);
            particleDataArray[particleIndex].scaleBase = (0.0f);
            particleDataArray[particleIndex].zRot =
                (45.0f) * frandmod(8);
        }
    }
}

/* Called during setup to create the falling-star models and their shared motion. */
void fn_1_B2F8(void) {
    OMOBJ *object;
    s16 modelIndex;

    object = omAddObjEx(lbl_1_bss_B00, 4096, 13, 0, -1, fn_1_B524);
    lbl_1_bss_248 = object;
    object->stat |= (1 << 8);
    object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
    *object->mdlId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(*object->mdlId, 1);
    Hu3DModelLayerSet(*object->mdlId, 1);
    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(*object->mdlId, 1);
    Hu3DModelScaleSet(
        *object->mdlId, (3.0f), (3.0f),
        (3.0f));
    Hu3DMotionSpeedSet(*object->mdlId, (2.0f));
    modelIndex = 1;
    while (modelIndex < 13) {
        object->mdlId[modelIndex] = Hu3DModelLink(*object->mdlId);
        Hu3DModelScaleSet(
            object->mdlId[modelIndex], (3.0f),
            (3.0f), (3.0f));
        modelIndex++;
    }
}

/* Called when a star effect starts to assign each model its initial delay. */
void fn_1_B498(void) {
    s16 index;

    lbl_1_bss_248->work[0] = 1;
    index = 0;
    while (index < 13) {
        lbl_1_bss_F4[index] = frandmod(5) * 8;
        index++;
    }
    lbl_1_bss_A30->work[1] = 0;
}

/* Updates the falling-star celebration, launching and removing stars as they cross the board. */
void fn_1_B524(OMOBJ *object) {
    Point3d position;
    Point3d rotationSource;
    HuVecF particlePosition;
    Point3d *rotationSourcePointer;
    HuVecF *particlePositionPointer;
    s16 index;
    f32 rotationX;
    f32 rotationY;

    if (object->work[0] == 0) {
        index = 0;
        while (index < 13) {
            Hu3DModelAttrSet(object->mdlId[index], 1);
            index++;
        }
        return;
    }

    index = 0;
    while (index < 13) {
        if (lbl_1_bss_F4[index] > 0) {
            if (lbl_1_bss_F4[index] != BINGO_STAR_FINISHED_DELAY) {
                lbl_1_bss_F4[index]--;
            }
            Hu3DModelAttrSet(object->mdlId[index], 1);
        } else if (lbl_1_bss_F4[index] == 0) {
            lbl_1_bss_F4[index] = -1;
            Hu3DModelAttrReset(object->mdlId[index], 1);
            position.x = (-300.0f) + (f32)frandmod(601);
            position.y = (2150.0f);
            position.z = (f32)frandmod(300) - (150.0f);
            if (position.z < 0.0f) {
                position.z -= (150.0f);
            } else {
                position.z += (150.0f);
            }
            Hu3DModelPosSetV(object->mdlId[index], &position);

            lbl_1_bss_1AC[index].x = lbl_1_bss_1AC[index].y = 0.0f;
            lbl_1_bss_1AC[index].z = 1.0f;

            rotationX = (f32)(frandmod(21) + 310);
            if (position.z > 0.0f) {
                rotationY = (f32)(frandmod(71) + 20 +
                                 frandmod(2) * 250);
            } else {
                rotationY = (f32)(frandmod(31) + 90 +
                                 frandmod(2) * 150);
            }

            rotationSource = lbl_1_bss_1AC[index];
            rotationSourcePointer = &rotationSource;
            fn_1_12BE0(rotationX, rotationY, 0.0f,
                       rotationSourcePointer, &lbl_1_bss_1AC[index]);
            Hu3DModelRotSet(object->mdlId[index],
                            rotationX, rotationY, 0.0f);

            lbl_1_bss_110[index].x =
                (f32)((0.1) * (f64)lbl_1_bss_1AC[index].x);
            lbl_1_bss_110[index].y =
                (f32)((0.1) * (f64)lbl_1_bss_1AC[index].y);
            lbl_1_bss_110[index].z =
                (f32)((0.1) * (f64)lbl_1_bss_1AC[index].z);

            particlePosition = position;
            particlePositionPointer = &particlePosition;
            fn_1_AEE8(particlePositionPointer);
        } else {
            Hu3DModelAttrReset(object->mdlId[index], 1);
            Hu3DModelPosGet(object->mdlId[index], &position);
            lbl_1_bss_110[index].x +=
                (0.1) * (f64)lbl_1_bss_1AC[index].x;
            if (lbl_1_bss_110[index].x >
                ((5.0f) * lbl_1_bss_1AC[index].x)) {
                lbl_1_bss_110[index].x = (5.0f) * lbl_1_bss_1AC[index].x;
            }
            lbl_1_bss_110[index].y +=
                (0.1) * (f64)lbl_1_bss_1AC[index].y;
            if (lbl_1_bss_110[index].y >
                ((5.0f) * lbl_1_bss_1AC[index].y)) {
                lbl_1_bss_110[index].y = (5.0f) * lbl_1_bss_1AC[index].y;
            }
            lbl_1_bss_110[index].z +=
                (0.1) * (f64)lbl_1_bss_1AC[index].z;
            if (lbl_1_bss_110[index].z >
                ((5.0f) * lbl_1_bss_1AC[index].z)) {
                lbl_1_bss_110[index].z = (5.0f) * lbl_1_bss_1AC[index].z;
            }
            position.x += lbl_1_bss_110[index].x;
            position.y += lbl_1_bss_110[index].y;
            position.z += lbl_1_bss_110[index].z;
            Hu3DModelPosSetV(object->mdlId[index], &position);
            if ((position.x < (-600.0f)) ||
                (600.0f < position.x)) {
                ((s16 *)lbl_1_bss_F4)[index] = BINGO_STAR_FINISHED_DELAY;
            }
        }
        index++;
    }
}

/* During the final-results camera transition, stops board updates and hides the card, line and
 * marker models before the star celebration starts. */
void fn_1_BDB4(void) {
    s16 playerIndex;
    s16 modelIndex;

    lbl_1_bss_F0 = 1;
    playerIndex = 0;
    while (playerIndex < 4) {
        Hu3DModelAttrSet(lbl_1_bss_A30->mdlId[playerIndex + 7], 1);
        Hu3DModelAttrSet(lbl_1_bss_A30->mdlId[playerIndex + 11], 1);
        modelIndex = 0;
        while (modelIndex < 12) {
            Hu3DModelAttrSet(
                lbl_1_bss_A30->mdlId[(playerIndex * 12 + 23) + modelIndex],
                1);
            modelIndex++;
        }
        modelIndex = 0;
        while (modelIndex < 2) {
            Hu3DModelAttrSet(
                lbl_1_bss_A30->mdlId[(playerIndex * 2 + 15) + modelIndex],
                1);
            modelIndex++;
        }
        Hu3DModelAttrSet(lbl_1_bss_A30->mdlId[playerIndex + 3], 1);
        playerIndex++;
    }
    Hu3DModelShadowMapObjSet(lbl_1_bss_A30->mdlId[0], lbl_1_data_5A0);
}

Point3d lbl_1_bss_A3C[4];
s16 lbl_1_bss_A34[4];
OMOBJ *lbl_1_bss_A30;
HSF_OBJECT *lbl_1_bss_850[4][30];
HSF_OBJECT *lbl_1_bss_670[4][30];
s16 lbl_1_bss_57E[4][30];
s16 lbl_1_bss_48E[4][30];
s16 lbl_1_bss_39E[4][30];
HU3D_MODELID lbl_1_bss_39C;
OMOBJ *lbl_1_bss_398;
ANIMDATA *lbl_1_bss_388[4];
s16 lbl_1_bss_37E[4];
MgmbingoBoardSnapshot lbl_1_bss_2B6[4];
s16 lbl_1_bss_2B4;
ANIMDATA *lbl_1_bss_28C[10];
s16 lbl_1_bss_288;
s16 lbl_1_bss_274[10];
s16 lbl_1_bss_24C[20];
OMOBJ *lbl_1_bss_248;
Point3d lbl_1_bss_1AC[13];
Point3d lbl_1_bss_110[13];
s16 lbl_1_bss_F4[14];
s16 lbl_1_bss_F2;
s16 lbl_1_bss_F0;
