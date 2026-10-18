/* Puzzle models, randomized layouts and round introduction. */
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
#include "REL/m662dll/game.h"

void fn_1_3C4(void), fn_1_808(void), fn_1_8C08(void), fn_1_90AC(void), fn_1_9910(void),
    fn_1_AC88(void);
void fn_1_984(void), fn_1_5884(void), fn_1_2670(void), fn_1_4EAC(void);
s16 fn_1_9C1C(void);
s32 fn_1_A170(void);
int fn_1_2DA0(f32 *, f32, f32, f32);
f32 fn_1_3C74(f32);
void fn_1_5008(s16, s16, void *);
extern s16 lbl_1_bss_C60;
/* Upper and lower X limits for each falling model spawn band. */
typedef struct PositionPair_9910 { f32 xMaximum, xMinimum; } PositionPair_9910;
/* Optional model-value debug hook; no output in the release game. */
static inline void DebugValue(s16 modelId) { }

/* During scene setup, creates players, scenery, tree pairs and falling models, then initializes
 * computer choices. */
int fn_1_8B34(void)
{
    lbl_1_bss_2F4[0].slot = -1;
    lbl_1_bss_2F4[1].slot = -1;
    fn_1_3C4();
    fn_1_808();
    fn_1_8C08();
    fn_1_90AC();
    fn_1_9910();
    fn_1_AC88();
    return 0;
}

/* Fade-in sequence callback: advances the introduction and scene updates, returning introduction
 * completion. */
s16 fn_1_8B8C() {
    s16 introductionFinished = 0;
    introductionFinished = (s16)fn_1_9C1C();
    fn_1_984();
    fn_1_5884();
    fn_1_2670();
    return introductionFinished;
}

/* Start sequence callback: keeps the introduction and scene updating while returning zero. */
s16 fn_1_8BCC() {
    s16 phaseFinished = 0;
    fn_1_9C1C();
    fn_1_984();
    fn_1_5884();
    fn_1_2670();
    return phaseFinished;
}

/* During scene setup, loads the scenery models, configures their visibility and motions, and stores
 * horizontal activation bounds. */
void fn_1_8C08(void)
{
    M662Bounds motionBounds[13] = {
        { -491.0f, 0.0f, -783.0f, 50.0f },
        { -402.0f, 0.0f, -789.0f, 15.0f },
        { -327.0f, 0.0f, -726.0f, 60.0f },
        { -240.0f, 0.0f, -783.0f, 40.0f },
        { -193.0f, 0.0f, -850.0f, 10.0f },
        { -113.0f, 0.0f, -728.0f, 60.0f },
        { -20.0f, 0.0f, -789.0f, 35.0f },
        { 65.0f, 0.0f, -783.0f, 35.0f },
        { 143.0f, 0.0f, -741.0f, 60.0f },
        { 239.0f, 0.0f, -789.0f, 15.0f },
        { 285.0f, 0.0f, -783.0f, 45.0f },
        { 362.0f, 0.0f, -758.0f, 50.0f },
        { 478.0f, 0.0f, -771.0f, 50.0f }
    };
    s16 modelIndex = 2;
    s16 assetIndex = 0;

    while (modelIndex < 32) {
        lbl_1_bss_4B4[modelIndex].modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_448[assetIndex], HU_MEMNUM_OVL,
                                 HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        if (modelIndex != 16) {
            Hu3DModelAttrReset(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        }
        Hu3DModelCameraSet(lbl_1_bss_4B4[modelIndex].modelId, 1);
        if ((modelIndex == 2) || (modelIndex == 12)) {
            Hu3DModelShadowMapSet(lbl_1_bss_4B4[modelIndex].modelId);
        }
        Hu3DModelLayerSet(lbl_1_bss_4B4[modelIndex].modelId, 0);
        if ((modelIndex == 14) || (modelIndex == 15) ||
            (modelIndex == 17)) {
            Hu3DModelLayerSet(lbl_1_bss_4B4[modelIndex].modelId, 1);
        }
        if ((modelIndex == 13) || (modelIndex == 11) ||
            (modelIndex == 16)) {
            Hu3DModelLayerSet(lbl_1_bss_4B4[modelIndex].modelId, 2);
        }
        Hu3DModelAttrReset(lbl_1_bss_4B4[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                           0.0f);
        Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId,
                          0.0f);
        if (modelIndex == 16) {
            Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId,
                             HU3D_MOTATTR_LOOP);
        }
        if ((modelIndex >= 19) && (modelIndex <= 31)) {
            Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                               1.0f);
            Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId,
                              24.9f);
        }
        if ((modelIndex == 11) || (modelIndex == 18)) {
            Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId,
                             HU3D_MOTATTR_LOOP);
            Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                               1.0f);
        }
        lbl_1_bss_4B4[modelIndex].position.x = 0.0f;
        lbl_1_bss_4B4[modelIndex].position.y = 0.0f;
        lbl_1_bss_4B4[modelIndex].position.z = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        assetIndex++;
    }

    modelIndex = 0;
    while (modelIndex < 13) {
        lbl_1_bss_0[modelIndex] = motionBounds[modelIndex];
        modelIndex++;
    }
}

/* During scene setup, creates four tree pairs and builds their twelve-step routes from three
 * distinct randomized patterns. */
void fn_1_90AC(void)
{
    s16 columnOrder[4] = { 0, 1, 2, 3 };
    HuVecF startPositions[4] = {
        { -450.0f, 0.0f, -900.0f },
        { -150.0f, 0.0f, -900.0f },
        { 150.0f, 0.0f, -900.0f },
        { 450.0f, 0.0f, -900.0f },
    };
    s16 pathLayouts[5][4][4] = {
        { { 12, 10, 7, -1 }, { 14, 15, 16, -1 }, { -1, -1, -1, -1 }, { -1, -1, -1, -1 } },
        { { 0, 1, 2, 3 }, { 4, 4, 5, 5 }, { 6, 7, 8, 9 }, { -1, -1, -1, -1 } },
        { { 0, -1, -1, 1 }, { 3, 3, 2, 2 }, { 5, -1, -1, 4 }, { -1, -1, -1, -1 } },
        { { 8, 10, 9, 11 }, { 7, -1, -1, 6 }, { -1, 1, 0, -1 }, { -1, -1, -1, -1 } },
        { { 8, -1, 9, -1 }, { 2, 3, -1, -1 }, { 10, -1, -1, 11 }, { 13, 2, 12, 3 } },
    };
    s16 modelIndex;
    s16 assetIndex;
    s16 patternIndex;
    s16 firstColumn;
    s16 secondColumn;
    s16 swappedColumn;
    s16 pathStep;
    s16 shufflePass;

    modelIndex = 32;
    assetIndex = 30;
    while (modelIndex <= 39) {
        lbl_1_bss_4B4[modelIndex].modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_448[assetIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_4B4[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_4B4[modelIndex].modelId, 1);
        Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId, 0.0f);
        if ((modelIndex >= 32) && (modelIndex <= 35)) {
            Hu3DModelAttrReset(lbl_1_bss_4B4[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        }
        lbl_1_bss_4B4[modelIndex].position.x = 0.0f;
        lbl_1_bss_4B4[modelIndex].position.y = 0.0f;
        lbl_1_bss_4B4[modelIndex].position.z = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.z = 0.0f;
        modelIndex++;
        assetIndex++;
    }

    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        lbl_1_bss_D0[modelIndex].firstModel = modelIndex + 32;
        lbl_1_bss_D0[modelIndex].secondModel = modelIndex + 36;
        lbl_1_bss_D0[modelIndex].objName = lbl_1_data_4E8[modelIndex];
        lbl_1_bss_D0[modelIndex].countdown = 0;
        lbl_1_bss_4B4[lbl_1_bss_D0[modelIndex].firstModel].position = startPositions[modelIndex];
        lbl_1_bss_4B4[lbl_1_bss_D0[modelIndex].secondModel].position = startPositions[modelIndex];
        DebugValue(lbl_1_bss_4B4[lbl_1_bss_D0[modelIndex].firstModel].modelId);
        DebugValue(lbl_1_bss_4B4[lbl_1_bss_D0[modelIndex].secondModel].modelId);
    }

    /* Choose one of the first two patterns and one of the other three. */
    modelIndex = frandmod(2);
    lbl_1_bss_210.patterns[modelIndex] = frandmod(2);
    lbl_1_bss_210.patterns[modelIndex ^ 1] = frandmod(3) + 2;
    /* The third pattern must differ from both earlier choices. */
    do {
        modelIndex = 0;
        assetIndex = frandmod(5);
        if (lbl_1_bss_210.patterns[0] == assetIndex) {
            modelIndex = 1;
        }
        if (lbl_1_bss_210.patterns[1] == assetIndex) {
            modelIndex = 1;
        }
    } while (modelIndex != 0);
    lbl_1_bss_210.patterns[2] = assetIndex;

    for (patternIndex = 0, pathStep = 0; patternIndex < 3; patternIndex++) {
        for (assetIndex = 0; assetIndex < 4; assetIndex++, pathStep++) {
            for (modelIndex = 0; modelIndex < 4; modelIndex++) {
                lbl_1_bss_D0[columnOrder[modelIndex]].paths[pathStep].index =
                    pathLayouts[lbl_1_bss_210.patterns[patternIndex]][assetIndex][modelIndex];
                if (lbl_1_bss_210.patterns[patternIndex] == 1) {
                    lbl_1_bss_D0[columnOrder[modelIndex]].paths[pathStep].mode = 1;
                } else {
                    lbl_1_bss_D0[columnOrder[modelIndex]].paths[pathStep].mode = 0;
                }
            }
            if (((lbl_1_bss_210.patterns[patternIndex] == 0) && (assetIndex == 0)) ||
                ((lbl_1_bss_210.patterns[patternIndex] == 1) && (assetIndex == 1))) {
                /* Carry the changed column order into the following path steps. */
                for (shufflePass = 0; shufflePass < 1000; shufflePass++) {
                    firstColumn = frandmod(4);
                    secondColumn = frandmod(4);
                    swappedColumn = columnOrder[firstColumn];
                    columnOrder[firstColumn] = columnOrder[secondColumn];
                    columnOrder[secondColumn] = swappedColumn;
                }
            } else if (((lbl_1_bss_210.patterns[patternIndex] == 2) ||
                        (lbl_1_bss_210.patterns[patternIndex] == 3)) &&
                       (assetIndex == 2)) {
                firstColumn = columnOrder[0];
                columnOrder[0] = columnOrder[3];
                columnOrder[3] = firstColumn;
                firstColumn = columnOrder[1];
                columnOrder[1] = columnOrder[2];
                columnOrder[2] = firstColumn;
            } else if ((lbl_1_bss_210.patterns[patternIndex] == 4) && (assetIndex == 3)) {
                firstColumn = columnOrder[2];
                columnOrder[2] = columnOrder[3];
                columnOrder[3] = firstColumn;
            }
        }
    }
}

/* During scene setup, creates six hidden falling models in randomized X bands with staggered start
 * delays. */
void fn_1_9910(void)
{
    PositionPair_9910 spawnRanges[6] = {
        { 525.0f, 175.0f },
        { 175.0f, -175.0f },
        { -175.0f, -525.0f },
        { 525.0f, 175.0f },
        { 175.0f, -175.0f },
        { -175.0f, -525.0f }
    };
    s16 modelIndex;
    s16 rangeIndex;
    s16 assetIndex;
    f32 positionX;

    modelIndex = 40;
    assetIndex = 38;
    rangeIndex = 0;
    while (modelIndex <= 45) {

        if (modelIndex >= 43) {
            assetIndex = 39;
        }
        lbl_1_bss_4B4[modelIndex].modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_448[assetIndex], HU_MEMNUM_OVL,
                                 HEAP_MODEL));
        Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(lbl_1_bss_4B4[modelIndex].modelId, 1);
        Hu3DModelLayerSet(lbl_1_bss_4B4[modelIndex].modelId, 0);
        Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_MOTATTR_LOOP);
        Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId, 0.0f);
        positionX = spawnRanges[rangeIndex].xMinimum +
                    fn_1_3C74(350.0f);
        lbl_1_bss_4B4[modelIndex].position.x = positionX;
        lbl_1_bss_4B4[modelIndex].position.y = 750.0f;
        lbl_1_bss_4B4[modelIndex].position.z = -930.0f;
        lbl_1_bss_4B4[modelIndex].rotation.x = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.y = 0.0f;
        lbl_1_bss_4B4[modelIndex].rotation.z = 0.0f;
        lbl_1_bss_4B4[modelIndex].motionFrame = frandmod(90);
        modelIndex++;
        rangeIndex++;
    }
}

/* During the introduction, lowers the camera, brings both players forward and waits for the tree
 * reveal to finish. */
s16 fn_1_9C1C(void)
{
    s16 readyToStart = 0;
    s32 unfinishedMoves = 0;
    float durationFrames;

    switch (lbl_1_bss_210.sequenceState) {
    case 0:
        fn_1_4EAC();
        if (lbl_1_bss_210.effectTimer == 60) {
            fn_1_5008(0, 5, 0);
        }
        if (lbl_1_bss_210.effectTimer++ > 120) {
            lbl_1_bss_210.effectTimer = 0;
            lbl_1_bss_210.sequenceState++;
        }
        break;
    case 1:
        durationFrames = 120.0f;
        unfinishedMoves = fn_1_2DA0(&Center.y, durationFrames,
                             1573.0f, 214.0f);
        if ((s16)unfinishedMoves == 0) {
            lbl_1_bss_3F4[0].requestedMotion = 1;
            lbl_1_bss_3F4[0].requestedSpeed = 1.0f;
            lbl_1_bss_3F4[0].requestedTime = 0.0f;
            lbl_1_bss_3F4[0].requestedAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[0].resetAttributes = 0;
            lbl_1_bss_3F4[1].requestedMotion = 1;
            lbl_1_bss_3F4[1].requestedSpeed = 1.0f;
            lbl_1_bss_3F4[1].requestedTime = 0.0f;
            lbl_1_bss_3F4[1].requestedAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[1].resetAttributes = 0;
            lbl_1_bss_2F4[0].startX = lbl_1_bss_2F4[0].position.x;
            lbl_1_bss_2F4[1].startX = lbl_1_bss_2F4[1].position.x;
            lbl_1_bss_2F4[0].startZ = lbl_1_bss_2F4[0].position.z;
            lbl_1_bss_2F4[1].startZ = lbl_1_bss_2F4[1].position.z;
            lbl_1_bss_210.sequenceState++;
        }
        break;
    case 2:
        durationFrames = 90.0f;
        unfinishedMoves = fn_1_2DA0(&lbl_1_bss_2F4[0].position.x, durationFrames,
                             lbl_1_bss_2F4[0].startX, -150.0f);
        unfinishedMoves += fn_1_2DA0(&lbl_1_bss_2F4[1].position.x, durationFrames,
                              lbl_1_bss_2F4[1].startX, 150.0f);
        unfinishedMoves += fn_1_2DA0(&lbl_1_bss_2F4[0].position.z, durationFrames,
                              lbl_1_bss_2F4[0].startZ, 0.0f);
        unfinishedMoves += fn_1_2DA0(&lbl_1_bss_2F4[1].position.z, durationFrames,
                              lbl_1_bss_2F4[1].startZ, 0.0f);
        if ((s16)unfinishedMoves == 0) {
            lbl_1_bss_210.sequenceState++;
        }
        break;
    case 3:
        durationFrames = 20.0f;
        unfinishedMoves = fn_1_2DA0(&lbl_1_bss_2F4[0].rotation.y, durationFrames,
                             135.0f, 0.0f);
        unfinishedMoves += fn_1_2DA0(&lbl_1_bss_2F4[1].rotation.y, durationFrames,
                              -135.0f, 0.0f);
        if ((s16)unfinishedMoves == 0) {
            lbl_1_bss_3F4[0].shiftState = 1;
            lbl_1_bss_3F4[0].shiftMotion = 0;
            lbl_1_bss_3F4[0].shiftStartFrame = 0.0f;
            lbl_1_bss_3F4[0].blendDuration = 10.0f;
            lbl_1_bss_3F4[0].shiftAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[0].resetAttributes = 0;
            lbl_1_bss_3F4[1].shiftState = 1;
            lbl_1_bss_3F4[1].shiftMotion = 0;
            lbl_1_bss_3F4[1].shiftStartFrame = 0.0f;
            lbl_1_bss_3F4[1].blendDuration = 10.0f;
            lbl_1_bss_3F4[1].shiftAttributes = HU3D_MOTATTR_LOOP;
            lbl_1_bss_3F4[1].resetAttributes = 0;
            lbl_1_bss_210.sequenceState++;
        }
        break;
    case 4:
        if (lbl_1_bss_210.effectTimer++ > 1) {
            lbl_1_bss_210.effectTimer = 0;
            lbl_1_bss_210.sequenceState++;
        }
        break;
    case 5:
        readyToStart = 1;
        break;
    }
    if (lbl_1_bss_210.sequenceState >= 2) {
        readyToStart *= fn_1_A170();
    }
    return readyToStart;
}

/* Introduction helper: plays four model motions in turn, then restarts them together and hides them
 * at frame five. */
s32 fn_1_A170(void)
{
    s16 modelIndex;
    s32 sequenceFinished = 0;

    if (lbl_1_bss_210.stage <= 3) {
        modelIndex = lbl_1_bss_210.stage + 32;
        Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                           1.0f);

        if (lbl_1_bss_C60 == 0) {
            fn_1_5008(
                2, 6,
                &lbl_1_bss_4B4[lbl_1_bss_D0[lbl_1_bss_210.stage].secondModel].position);
            lbl_1_bss_C60 = 1;
        }

        if (Hu3DMotionEndCheck(lbl_1_bss_4B4[modelIndex].modelId) != 0) {
            lbl_1_bss_C60 = 0;
            lbl_1_bss_210.stage++;
        }
    } else if (lbl_1_bss_210.stage <= 30) {
        if (lbl_1_bss_210.stage++ >= 30) {
            for (modelIndex = 32; modelIndex <= 35; modelIndex++) {
                Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId,
                                  0.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                                   1.0f);
            }
            lbl_1_bss_210.stage = 31;
        }
    } else if (lbl_1_bss_210.stage == 31) {
        if (Hu3DMotionTimeGet(lbl_1_bss_4B4[32].modelId) >=
            5.0f) {
            for (modelIndex = 32; modelIndex <= 35; modelIndex++) {
                Hu3DModelAttrSet(lbl_1_bss_4B4[modelIndex].modelId, HU3D_ATTR_DISPOFF);
                Hu3DMotionTimeSet(lbl_1_bss_4B4[modelIndex].modelId,
                                  5.0f);
                Hu3DMotionSpeedSet(lbl_1_bss_4B4[modelIndex].modelId,
                                   0.0f);
            }
            lbl_1_bss_210.stage++;
        }
    } else {
        sequenceFinished = 1;
    }

    return sequenceFinished;
}

/* Scenery, tree-pair and falling-model resources in scene model order. */
s32 lbl_1_data_448[40] = {
    DATANUM(DATA_m662, 0), DATANUM(DATA_m662, 1), DATANUM(DATA_m662, 2), DATANUM(DATA_m662, 3),
    DATANUM(DATA_m662, 4), DATANUM(DATA_m662, 5), DATANUM(DATA_m662, 6), DATANUM(DATA_m662, 7),
    DATANUM(DATA_m662, 8), DATANUM(DATA_m662, 9), DATANUM(DATA_m662, 10), DATANUM(DATA_m662, 11),
    DATANUM(DATA_m662, 12), DATANUM(DATA_m662, 13), DATANUM(DATA_m662, 14), DATANUM(DATA_m662, 15),
    DATANUM(DATA_m662, 16), DATANUM(DATA_m662, 17), DATANUM(DATA_m662, 18), DATANUM(DATA_m662, 19),
    DATANUM(DATA_m662, 20), DATANUM(DATA_m662, 21), DATANUM(DATA_m662, 22), DATANUM(DATA_m662, 23),
    DATANUM(DATA_m662, 24), DATANUM(DATA_m662, 25), DATANUM(DATA_m662, 26), DATANUM(DATA_m662, 27),
    DATANUM(DATA_m662, 28), DATANUM(DATA_m662, 29), DATANUM(DATA_m662, 30), DATANUM(DATA_m662, 30),
    DATANUM(DATA_m662, 30), DATANUM(DATA_m662, 30), DATANUM(DATA_m662, 31), DATANUM(DATA_m662, 32),
    DATANUM(DATA_m662, 33), DATANUM(DATA_m662, 34), DATANUM(DATA_m662, 35), DATANUM(DATA_m662, 36)
};

/* Named attachment points for the four paired models. */
char lbl_1_data_4E8[4][32] = { "eye_A_dodai", "eye_B_dodai", "eye_C_dodai", "eye_D_dodai" };
