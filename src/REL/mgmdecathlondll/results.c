/* Decathlon Park result displays, ranking movement and score conversion. */
#include "REL/mgmdecathlondll/module_types.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "datadir_enum.h"
#include "msm_se.h"

#define MSM_SE_MGMDECA_SCORE_UPDATE 1242
#define MSM_SE_MGMDECA_SCORE_COUNT 1243
#define MSM_SE_MGMDECA_RANK_MOVE 1244

typedef struct DecathlonModelGroup {
    u8 rankStateBytes[12]; /* rank and row bookkeeping, unused during model creation */
    HU3D_MODELID playerModels[4]; /* one visible model for the player's tied rank */
    HU3D_MODELID variantModel; /* character-specific marker moved between result rows */
    HU3D_MODELID modeModels[5][10]; /* decimal digit models for the five-place total */
    HU3D_MODELID spotlightModel; /* model beside the four-place event award */
    HU3D_MODELID resultModels[4][10]; /* decimal digit models for the remaining event award */
} DecathlonModelGroup;

extern s16 fn_1_9744(s16 playerNo);
extern void fn_1_C6C8(OMOBJ *obj);
extern void fn_1_C344(s16 enabled);
extern void fn_1_D518(s16 playerNo, s16 positionNo);
extern void fn_1_D788(s16 playerNo);
extern void fn_1_DAEC(s16 playerNo);
extern void fn_1_DCF8(void);

s32 lbl_1_data_5D8 = DATANUM(0x9e0000, 5);
Vec lbl_1_data_5DC[4] = { { 82.0f, 395.0f, -290.0f },
                          { 82.0f, 335.0f, -290.0f },
                          { 82.0f, 275.0f, -290.0f },
                          { 82.0f, 215.0f, -290.0f } };
s32 lbl_1_data_60C[4] = {
    DATANUM(0x9e0000, 16), DATANUM(0x9e0000, 17),
    DATANUM(0x9e0000, 18), DATANUM(0x9e0000, 19)
};
Vec lbl_1_data_61C[4] = { { -215.0f, 400.0f, -290.0f },
                          { -215.0f, 340.0f, -290.0f },
                          { -215.0f, 280.0f, -290.0f },
                          { -215.0f, 220.0f, -290.0f } };
s32 lbl_1_data_64C[14] = {
    DATANUM(0x9e0000, 20), DATANUM(0x9e0000, 21),
    DATANUM(0x9e0000, 22), DATANUM(0x9e0000, 23),
    DATANUM(0x9e0000, 24), DATANUM(0x9e0000, 25),
    DATANUM(0x9e0000, 26), DATANUM(0x9e0000, 27),
    DATANUM(0x9e0000, 28), DATANUM(0x9e0000, 29),
    DATANUM(0x9e0000, 30), DATANUM(0x9e0000, 29),
    DATANUM(0x9e0000, 29), DATANUM(0x9e0000, 29)
};
DecathlonRankPosition lbl_1_data_684[4] = { { { -135.0f, 400.0f, -290.0f } },
                                            { { -135.0f, 340.0f, -290.0f } },
                                            { { -135.0f, 280.0f, -290.0f } },
                                            { { -135.0f, 220.0f, -290.0f } } };
s32 lbl_1_data_6B4[10] = {
    DATANUM(0x9e0000, 6), DATANUM(0x9e0000, 7),
    DATANUM(0x9e0000, 8), DATANUM(0x9e0000, 9),
    DATANUM(0x9e0000, 10), DATANUM(0x9e0000, 11),
    DATANUM(0x9e0000, 12), DATANUM(0x9e0000, 13),
    DATANUM(0x9e0000, 14), DATANUM(0x9e0000, 15)
};
f32 lbl_1_data_6DC[5] = {-85.0f, -55.0f, -25.0f, 5.0f, 35.0f};
f32 lbl_1_data_6F0[4] = {395.0f, 335.0f, 275.0f, 215.0f};
f32 lbl_1_data_700 = -290.0f;
f32 lbl_1_data_704[4] = {130.0f, 160.0f, 190.0f, 220.0f};
f32 lbl_1_data_714[4] = {395.0f, 335.0f, 275.0f, 215.0f};
f32 lbl_1_data_724 = -290.0f;

/* The mode sequence calls this to create and arrange the result-screen models. */
void fn_1_BC00(void) {
    OMOBJ *modelObject;
    DecathlonModelGroup *modelGroups;
    s16 groupIndex;
    s16 rowIndex;
    s16 modelIndex;

    modelObject = fn_8003AB38(lbl_1_bss_C0, 4352, 0, 0, -1, fn_1_C6C8);
    lbl_1_bss_B8 = modelObject;
    modelObject->stat |= (1 << 8);
    modelObject->work[0] = modelObject->work[1] =
        modelObject->work[2] = modelObject->work[3] = 0;
    modelObject->data = HuMemDirectMallocNum(HEAP_HEAP, 816, 0x10000000);
    memset(modelObject->data, 0, 816);
    modelGroups = modelObject->data;

    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        modelGroups[0].playerModels[groupIndex] =
            fn_80025A04(fn_80008DD0(lbl_1_data_60C[groupIndex],
                                                 0x10000000, 2));
        fn_80026B64(modelGroups[0].playerModels[groupIndex], 1);
        fn_80026B8C(modelGroups[0].playerModels[groupIndex], 2);
    }

    modelGroups[0].variantModel =
        fn_80025A04(fn_80008DD0(lbl_1_data_64C[fn_1_9744(0)],
                                             0x10000000, 2));
    fn_80026B64(modelGroups[0].variantModel, 1);
    fn_80026B8C(modelGroups[0].variantModel, 2);

    for (groupIndex = 0; groupIndex < 10; groupIndex++) {
        modelGroups[0].modeModels[0][groupIndex] =
            fn_80025A04(fn_80008DD0(lbl_1_data_6B4[groupIndex],
                                                 0x10000000, 2));
        fn_80026B64(modelGroups[0].modeModels[0][groupIndex], 1);
        fn_80026B8C(modelGroups[0].modeModels[0][groupIndex], 2);
    }

    for (groupIndex = 1; groupIndex < 5; groupIndex++) {
        for (modelIndex = 0; modelIndex < 10; modelIndex++) {
            modelGroups[0].modeModels[groupIndex][modelIndex] =
                fn_80025D4C(modelGroups[0].modeModels[0][modelIndex]);
            fn_80026B64(modelGroups[0].modeModels[groupIndex][modelIndex], 1);
            fn_80026B8C(modelGroups[0].modeModels[groupIndex][modelIndex], 2);
        }
    }

    modelGroups[0].spotlightModel =
        fn_80025A04(fn_80008DD0(lbl_1_data_5D8,
                                             0x10000000, 2));
    fn_80026B64(modelGroups[0].spotlightModel, 1);
    fn_80026B8C(modelGroups[0].spotlightModel, 2);

    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        for (modelIndex = 0; modelIndex < 10; modelIndex++) {
            modelGroups[0].resultModels[groupIndex][modelIndex] =
                fn_80025D4C(modelGroups[0].modeModels[0][modelIndex]);
            fn_80026B64(modelGroups[0].resultModels[groupIndex][modelIndex], 1);
            fn_80026B8C(modelGroups[0].resultModels[groupIndex][modelIndex], 2);
        }
    }

    for (groupIndex = 1; groupIndex < 4; groupIndex++) {
        for (modelIndex = 0; modelIndex < 4; modelIndex++) {
            modelGroups[groupIndex].playerModels[modelIndex] =
                fn_80025D4C(modelGroups[0].playerModels[modelIndex]);
            fn_80026B64(modelGroups[groupIndex].playerModels[modelIndex], 1);
            fn_80026B8C(modelGroups[groupIndex].playerModels[modelIndex], 2);
        }

        modelGroups[groupIndex].variantModel =
            fn_80025A04(fn_80008DD0(lbl_1_data_64C[fn_1_9744(groupIndex)],
                                                 0x10000000, 2));
        fn_80026B64(modelGroups[groupIndex].variantModel, 1);
        fn_80026B8C(modelGroups[groupIndex].variantModel, 2);

        for (modelIndex = 0; modelIndex < 5; modelIndex++) {
            for (rowIndex = 0; rowIndex < 10; rowIndex++) {
                modelGroups[groupIndex].modeModels[modelIndex][rowIndex] =
                    fn_80025D4C(modelGroups[0].modeModels[0][rowIndex]);
                fn_80026B64(modelGroups[groupIndex].modeModels[modelIndex][rowIndex], 1);
                fn_80026B8C(modelGroups[groupIndex].modeModels[modelIndex][rowIndex], 2);
            }
        }

        modelGroups[groupIndex].spotlightModel =
            fn_80025D4C(modelGroups[0].spotlightModel);
        fn_80026B64(modelGroups[groupIndex].spotlightModel, 1);
        fn_80026B8C(modelGroups[groupIndex].spotlightModel, 2);

        for (modelIndex = 0; modelIndex < 4; modelIndex++) {
            for (rowIndex = 0; rowIndex < 10; rowIndex++) {
                modelGroups[groupIndex].resultModels[modelIndex][rowIndex] =
                    fn_80025D4C(modelGroups[0].modeModels[0][rowIndex]);
                fn_80026B64(modelGroups[groupIndex].resultModels[modelIndex][rowIndex], 1);
                fn_80026B8C(modelGroups[groupIndex].resultModels[modelIndex][rowIndex], 2);
            }
        }
    }

    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        fn_1_D518(groupIndex, groupIndex);
    }
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        fn_1_D788(groupIndex);
    }
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        fn_1_DAEC(groupIndex);
    }
    fn_1_C344(0);
    fn_1_DCF8();
    lbl_1_bss_BC = 0;
}

s16 lbl_1_bss_BC;
OMOBJ *lbl_1_bss_B8;

extern void fn_1_D788(s16 playerNo);
extern void fn_1_DCF8(void);
extern void fn_1_DAEC(s16 playerNo);

/* The mode sequence refreshes every player's score digits and rank display here. */
void fn_1_C2FC(void) {
    s16 playerNo;

    playerNo = 0;
    while (playerNo < 4) {
        fn_1_D788(playerNo);
        playerNo += 1;
    }
    fn_1_DCF8();
}

typedef struct MgmDecathlonPlayerData_s {
    u8 precedingDisplayBytes[122]; /* rank state and models not changed by this visibility helper */
    HU3D_MODELID modelId; /* marker beside the event award digits */
    u8 awardDigitBytes[80]; /* four sets of ten digit model IDs, unused by this helper */
} MgmDecathlonPlayerData;

/* The main result sequence shows or hides the marker beside each player's event-award digits. */
void fn_1_C344(s16 enabled) {
    s16 index;
    MgmDecathlonPlayerData *players;

    players = lbl_1_bss_B8->data;
    for (index = 0; index < 4; index++) {
        if (enabled != 0) {
            fn_80026A44(players[index].modelId, (1 << 0));
        } else {
            fn_800269E8(players[index].modelId, (1 << 0));
        }
    }
}

/* The main result sequence refreshes score digits and plays their update cue. */
void fn_1_C3DC(void) {
    s16 playerNo;

    playerNo = 0;
    while (playerNo < 4) {
        fn_1_D788(playerNo);
        playerNo += 1;
    }
    HuAudFXPlay(MSM_SE_MGMDECA_SCORE_UPDATE);
}

/* The main sequence hides each player's four-by-ten score digit models. */
void fn_1_C428(void) {
    s16 playerNo;

    playerNo = 0;
    while (playerNo < 4) {
        fn_1_DAEC(playerNo);
        playerNo += 1;
    }
}

/* The main results sequence starts transferring event awards into the total scores. */
void fn_1_C46C(void) {
    lbl_1_bss_BC = 1;
    lbl_1_bss_B8->work[0] = lbl_1_bss_B8->work[1] = lbl_1_bss_B8->work[2] = lbl_1_bss_B8->work[3] =
        0;
}

/* After the score count, the main sequence starts rearranging players into their new ranks. */
void fn_1_C4C4(void) {
    lbl_1_bss_BC = 4;
    lbl_1_bss_B8->work[0] = lbl_1_bss_B8->work[1] = lbl_1_bss_B8->work[2] = lbl_1_bss_B8->work[3] =
        0;
}

/* The main sequence polls this result-animation state before advancing. */
s32 fn_1_C51C(void) {
    if (lbl_1_bss_BC == 0) {
        return 1;
    }
    return 0;
}

typedef struct DecathlonPlayerResult {
    s16 currentRank; /* tied rank currently displayed; zero is first */
    u16 targetRank; /* next tied rank, unused by the reaction helpers */
    s16 previousRank; /* tied rank before the latest score update */
    u8 remainingDisplayBytes[198]; /* row state and models unused by the reaction helpers */
} DecathlonPlayerResult;

/* After the ranking animation, the main sequence selects reactions to improved or worsened
 * ranks. */
void fn_1_C540(void) {
    DecathlonPlayerResult *results = (DecathlonPlayerResult *)lbl_1_bss_B8->data;
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (results[playerNo].currentRank < results[playerNo].previousRank) {
            fn_1_9AAC(playerNo, 1);
        }
        if (results[playerNo].currentRank > results[playerNo].previousRank) {
            fn_1_9AAC(playerNo, 0);
        }
    }
}

/* The result callback rumbles players whose tied rank changed after the count. */
void fn_1_C5FC(void) {
    DecathlonPlayerResult *results = (DecathlonPlayerResult *)lbl_1_bss_B8->data;
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (results[playerNo].currentRank < results[playerNo].previousRank) {
            fn_8006A030(playerNo, 20, 7, 3);
        }
        if (results[playerNo].currentRank > results[playerNo].previousRank) {
            fn_8006A030(playerNo, 20, 7, 3);
        }
    }
}

typedef struct MgmDecathlonEntry_s {
    u32 currentAndTargetRankBytes; /* the current and target tied ranks, unused here */
    s16 previousRank; /* tied rank before the latest score update */
    u8 remainingDisplayBytes[198]; /* row state and models unused by the rank formatter */
} MgmDecathlonEntry;

extern void fn_1_E058(s16 *order, s16 *scores);

typedef struct DecathlonRankPlayer {
    s16 currentScoreRank; /* currently displayed tied rank, zero through three */
    s16 targetScoreRank; /* tied rank after counting the event award */
    s16 previousScoreRank; /* tied rank before the latest update */
    s16 currentRank; /* current screen row, zero through three */
    s16 targetRank; /* screen row after sorting the updated totals */
    s16 previousRank; /* screen row before movement */
    u8 rankModelBytes[8]; /* four tied-rank model IDs, unused by movement */
    HU3D_MODELID modelId; /* character marker moved between screen rows */
    u8 scoreModelBytes[182]; /* total and event-award display models, unused by movement */
} DecathlonRankPlayer;

extern void fn_1_D788(s16 playerNo);
extern void fn_1_DAEC(s16 playerNo);
extern void fn_1_DB94(s16 playerNo);
extern void fn_1_DCF8(void);
extern void fn_1_DE84(s16 playerNo);
extern void fn_1_DFB0(void);

/* The result object's frame callback counts scores and animates the final rankings. */
void fn_1_C6C8(OMOBJ *object) {
    s16 rankCount;
    DecathlonRankPlayer *players;
    Vec position;
    s16 playerNo;

    players = object->data;
    switch (lbl_1_bss_BC) {
    case 1:
        lbl_1_bss_BC = 2;
        /* Start one counting cue, using the first player with an award still to transfer. */
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (MgModeWorkInt[playerNo] > 0) {
                lbl_1_data_C78[playerNo] = HuAudFXPlay(MSM_SE_MGMDECA_SCORE_COUNT);
                return;
            }
        }
        return;
    case 2:
        /* Transfer up to ten points per player each frame and refresh both digit displays. */
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (MgModeWorkInt[playerNo] > 10) {
                MgModeScore[playerNo] += 10;
                MgModeWorkInt[playerNo] -= 10;
            } else {
                MgModeScore[playerNo] += MgModeWorkInt[playerNo];
                MgModeWorkInt[playerNo] = 0;
            }
            fn_1_D788(playerNo);
        }
        rankCount = 1;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (MgModeWorkInt[playerNo] > 0) {
                rankCount = 0;
            }
        }
        if (rankCount != 0) {
            lbl_1_bss_BC = 3;
            if (lbl_1_data_C78[0] >= 0) {
                HuAudFXStop(lbl_1_data_C78[0]);
                lbl_1_data_C78[0] = -1;
            }
            if (lbl_1_data_C78[1] >= 0) {
                HuAudFXStop(lbl_1_data_C78[1]);
                lbl_1_data_C78[1] = -1;
            }
            if (lbl_1_data_C78[2] >= 0) {
                HuAudFXStop(lbl_1_data_C78[2]);
                lbl_1_data_C78[2] = -1;
            }
            if (lbl_1_data_C78[3] >= 0) {
                HuAudFXStop(lbl_1_data_C78[3]);
                lbl_1_data_C78[3] = -1;
            }
        }
        break;
    case 3:
        for (playerNo = 0; playerNo < 4; playerNo++) {
            fn_1_D788(playerNo);
        }
        lbl_1_bss_BC = 0;
        return;
    case 4:
        fn_1_DFB0();
        for (playerNo = 0, rankCount = 0; playerNo < 4; playerNo++) {
            if (players[playerNo].targetRank != players[playerNo].currentRank) {
                rankCount++;
            }
        }
        /* Hide changing rows from bottom to top, one row every twenty frames. */
        if ((object->work[0] < 80) && (rankCount != 0)) {
            for (playerNo = 0; playerNo < 4; playerNo++) {
                fn_1_DAEC(playerNo);
            }
            if ((object->work[0] % 20) == 0) {
                rankCount = object->work[0] / 20;
                rankCount = 3 - rankCount;
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    if ((rankCount == players[playerNo].currentRank) &&
                        (players[playerNo].currentRank != players[playerNo].targetRank)) {
                        fn_1_DB94(playerNo);
                    }
                }
            }
            object->work[0]++;
            return;
        }
        lbl_1_bss_BC = 5;
        if (rankCount == 0) {
            lbl_1_bss_BC = 6;
        }
        object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
        return;
    case 5:
        /* Falling markers move right before descending; rising markers take the left path. */
        rankCount = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            if (players[playerNo].targetRank != players[playerNo].currentRank) {
                rankCount++;
                fn_800267CC(players[playerNo].modelId, &position);
                if (players[playerNo].targetRank > players[playerNo].currentRank) {
                    if (object->work[playerNo] == 0) {
                        if ((30.0f + lbl_1_data_684[players[playerNo].currentRank].position.x) >
                            position.x) {
                            position.x += 5.0f;
                        }
                        if ((30.0f + lbl_1_data_684[players[playerNo].currentRank].position.x) <=
                            position.x) {
                            position.x =
                                30.0f + lbl_1_data_684[players[playerNo].currentRank].position.x;
                            object->work[playerNo]++;
                        }
                    } else if (object->work[playerNo] == 1) {
                        if (lbl_1_data_684[players[playerNo].targetRank].position.y < position.y) {
                            position.y -= 5.0f;
                        }
                        if (lbl_1_data_684[players[playerNo].targetRank].position.y >= position.y) {
                            position.y = lbl_1_data_684[players[playerNo].targetRank].position.y;
                            object->work[playerNo]++;
                        }
                    } else if (object->work[playerNo] == 2) {
                        if (lbl_1_data_684[players[playerNo].targetRank].position.x < position.x) {
                            position.x -= 5.0f;
                        }
                        if (lbl_1_data_684[players[playerNo].targetRank].position.x >= position.x) {
                            position.x = lbl_1_data_684[players[playerNo].targetRank].position.x;
                            object->work[playerNo] = 0;
                            players[playerNo].previousRank = players[playerNo].currentRank;
                            players[playerNo].currentRank = players[playerNo].targetRank;
                        }
                    }
                }
                if (players[playerNo].targetRank < players[playerNo].currentRank) {
                    if (object->work[playerNo] == 0) {
                        if ((lbl_1_data_684[players[playerNo].currentRank].position.x - 30.0f) <
                            position.x) {
                            position.x -= 5.0f;
                        }
                        if ((lbl_1_data_684[players[playerNo].currentRank].position.x - 30.0f) >=
                            position.x) {
                            position.x =
                                lbl_1_data_684[players[playerNo].currentRank].position.x - 30.0f;
                            object->work[playerNo]++;
                        }
                    } else if (object->work[playerNo] == 1) {
                        if (lbl_1_data_684[players[playerNo].targetRank].position.y > position.y) {
                            position.y += 5.0f;
                        }
                        if (lbl_1_data_684[players[playerNo].targetRank].position.y <= position.y) {
                            position.y = lbl_1_data_684[players[playerNo].targetRank].position.y;
                            object->work[playerNo]++;
                        }
                    } else if (object->work[playerNo] == 2) {
                        if (lbl_1_data_684[players[playerNo].targetRank].position.x > position.x) {
                            position.x += 5.0f;
                        }
                        if (lbl_1_data_684[players[playerNo].targetRank].position.x <= position.x) {
                            position.x = lbl_1_data_684[players[playerNo].targetRank].position.x;
                            object->work[playerNo] = 0;
                            players[playerNo].previousRank = players[playerNo].currentRank;
                            players[playerNo].currentRank = players[playerNo].targetRank;
                        }
                    }
                }
                fn_80026790(players[playerNo].modelId, &position);
            }
        }
        if (rankCount == 0) {
            lbl_1_bss_BC = 6;
            object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
            return;
        }
        break;
    case 6: {
        s16 movementCount;

        for (playerNo = 0, movementCount = 0; playerNo < 4; playerNo++) {
            if (players[playerNo].targetRank != players[playerNo].currentRank) {
                movementCount++;
            }
            if (players[playerNo].targetScoreRank != players[playerNo].currentScoreRank) {
                movementCount++;
            }
        }
        /* Restore labels from bottom to top, one row every fifteen frames. */
        if ((object->work[0] < 60) && (movementCount != 0)) {
            if ((object->work[0] % 15) == 0) {
                rankCount = object->work[0] / 15;
                rankCount = 3 - rankCount;
                for (playerNo = 0; playerNo < 4; playerNo++) {
                    if (rankCount == players[playerNo].currentRank) {
                        fn_1_D788(playerNo);
                        fn_1_DAEC(playerNo);
                        fn_1_DE84(playerNo);
                        if (players[playerNo].targetScoreRank !=
                            players[playerNo].currentScoreRank) {
                            HuAudFXPlay(MSM_SE_MGMDECA_RANK_MOVE);
                        }
                    }
                }
            }
            object->work[0]++;
            return;
        }
        fn_1_DCF8();
        lbl_1_bss_BC = 0;
        object->work[0] = object->work[1] = object->work[2] = object->work[3] = 0;
        fn_1_C5FC();
        return;
    }
    default:
        lbl_1_bss_BC = 0;
    case 0:
        break;
    }
}

/* Result messages use this to obtain each sorted player's rank before the latest update. */
void fn_1_D484(s16 *values) {
    s16 order[4];
    s16 index;
    MgmDecathlonEntry *entries;

    entries = lbl_1_bss_B8->data;
    fn_1_E058(order, values);
    for (index = 0; index < 4; index++) {
        values[index] = entries[order[index]].previousRank;
    }
}

typedef struct DecathlonPlayerModels {
    s16 tiedRanks[3]; /* current, target and previous tied ranks */
    s16 rowOrder[3]; /* current, target and previous screen rows */
    s16 startModels[4]; /* models for the four tied-rank labels */
    s16 transitionModel; /* character-specific marker positioned at its screen row */
    s16 groupedModels[5][10]; /* five total-score digits, each with ten model choices */
    s16 resultModel; /* marker beside the event-award digits */
    s16 resultGroups[4][10]; /* four event-award digits, each with ten model choices */
} DecathlonPlayerModels;

/* Setup and rank updates place one player's result models in the selected screen row. */
void fn_1_D518(s16 playerNo, s16 positionNo) {
    HuVecF position;
    DecathlonPlayerModels *playerModels;
    s16 group;
    s16 model;

    playerModels = lbl_1_bss_B8->data;
    playerModels[playerNo].rowOrder[2] = playerModels[playerNo].rowOrder[0];
    playerModels[playerNo].rowOrder[0] = positionNo;
    playerModels[playerNo].rowOrder[1] = positionNo;
    for (group = 0; group < 4; group++) {
        fn_80026790(playerModels[playerNo].startModels[group],
                         (HuVecF *)&lbl_1_data_61C[positionNo]);
    }
    fn_80026790(playerModels[playerNo].transitionModel,
                     &lbl_1_data_684[positionNo].position);
    position.y = lbl_1_data_6F0[positionNo];
    position.z = lbl_1_data_700;
    for (group = 0; group < 5; group++) {
        position.x = lbl_1_data_6DC[group];
        for (model = 0; model < 10; model++) {
            fn_80026790(playerModels[playerNo].groupedModels[group][model], &position);
        }
    }
    fn_80026790(playerModels[playerNo].resultModel,
                     (HuVecF *)&lbl_1_data_5DC[positionNo]);
    position.y = lbl_1_data_714[positionNo];
    position.z = lbl_1_data_724;
    for (group = 0; group < 4; group++) {
        position.x = lbl_1_data_704[group];
        for (model = 0; model < 10; model++) {
            fn_80026790(playerModels[playerNo].resultGroups[group][model], &position);
        }
    }
}

/* Score tally and result updates call this to select the visible score digits. */
void fn_1_D788(s16 playerNo) {
    DecathlonPlayerModels *playerModels;
    s16 digits[5];
    s16 digitIndex;
    s16 model;
    s16 score;

    playerModels = lbl_1_bss_B8->data;
    score = MgModeScore[playerNo];
    for (digitIndex = 0; digitIndex < 5; digitIndex++) {
        digits[4 - digitIndex] = score % 10;
        if (score <= 0) {
            digits[4 - digitIndex] = -1;
        } else {
            score -= digits[4 - digitIndex];
            score /= 10;
        }
    }
    /* Suppress leading zeroes, but always leave the units digit visible. */
    if (digits[4] < 0) {
        digits[4] = 0;
    }
    for (digitIndex = 0; digitIndex < 5; digitIndex++) {
        for (model = 0; model < 10; model++) {
            if (model == digits[digitIndex]) {
                fn_80026A44(playerModels[playerNo].groupedModels[digitIndex][model],
                                   (1 << 0));
            } else {
                fn_800269E8(playerModels[playerNo].groupedModels[digitIndex][model],
                                 (1 << 0));
            }
        }
    }
    score = MgModeWorkInt[playerNo];
    for (digitIndex = 0; digitIndex < 4; digitIndex++) {
        digits[3 - digitIndex] = score % 10;
        if (score <= 0) {
            digits[3 - digitIndex] = -1;
        } else {
            score -= digits[3 - digitIndex];
            score /= 10;
        }
    }
    if (digits[3] < 0) {
        digits[3] = 0;
    }
    for (digitIndex = 0; digitIndex < 4; digitIndex++) {
        for (model = 0; model < 10; model++) {
            if (model == digits[digitIndex]) {
                fn_80026A44(playerModels[playerNo].resultGroups[digitIndex][model],
                                   (1 << 0));
            } else {
                fn_800269E8(playerModels[playerNo].resultGroups[digitIndex][model],
                                 (1 << 0));
            }
        }
    }
}

typedef struct MgmDecathlonModelData_s {
    u8 precedingDisplayBytes[124]; /* rank state and other models, unused by this helper */
    HU3D_MODELID models[4][10]; /* four event-award digits, each with ten model choices */
} MgmDecathlonModelData;

typedef struct MgmDecathlonPlayerVisuals_s {
    u8 rankStateBytes[12]; /* rank and row bookkeeping, unused by this helper */
    HU3D_MODELID models[4]; /* tied-rank label models */
    u16 characterMarkerId; /* character-specific marker; kept visible during movement */
    HU3D_MODELID bonusModels[5][10]; /* five total-score digits, each with ten choices */
    HU3D_MODELID focusModel; /* marker beside the event-award digits */
    HU3D_MODELID resultsModels[4][10]; /* four event-award digits, each with ten choices */
} MgmDecathlonPlayerVisuals;

/* Result updates call this to hide one player's mode-score digit models. */
void fn_1_DAEC(s16 playerIndex) {
    s16 row;
    s16 column;
    MgmDecathlonModelData *players;

    players = lbl_1_bss_B8->data;
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 10; column++) {
            fn_800269E8(players[playerIndex].models[row][column], (1 << 0));
        }
    }
}

/* The rank animation calls this to hide one player's result and score models. */
void fn_1_DB94(s16 playerNo) {
    MgmDecathlonPlayerVisuals *players = lbl_1_bss_B8->data;
    s16 row;
    s16 column;

    for (row = 0; row < 4; row++) {
        fn_800269E8(players[playerNo].models[row], (1 << 0));
    }
    for (row = 0; row < 5; row++) {
        for (column = 0; column < 10; column++) {
            fn_800269E8(players[playerNo].bonusModels[row][column], (1 << 0));
        }
    }
    fn_800269E8(players[playerNo].focusModel, (1 << 0));
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 10; column++) {
            fn_800269E8(players[playerNo].resultsModels[row][column], (1 << 0));
        }
    }
}

typedef struct MgmDecathlonSelectedPlayer_s {
    s16 currentRank; /* currently displayed tied rank */
    s16 targetRank; /* target tied rank after the score update */
    s16 previousRank; /* tied rank before the latest update */
    u8 rowStateBytes[6]; /* current, target and previous screen rows, unused here */
    HU3D_MODELID modelIds[4]; /* tied-rank label model IDs */
    u8 scoreModelBytes[184]; /* character marker and score models, unused here */
} MgmDecathlonSelectedPlayer;

extern void fn_1_E058(s16 *playerOrder, s16 *scoreOrder);
extern void fn_1_D518(s16 playerIndex, s16 orderIndex);

/* The result sequence sorts players and updates each player's displayed rank. */
void fn_1_DCF8(void) {
    s16 playerOrder[4];
    s16 scoreOrder[4];
    s16 playerIndex;
    s16 modelIndex;
    MgmDecathlonSelectedPlayer *players;

    players = lbl_1_bss_B8->data;
    fn_1_E058(playerOrder, scoreOrder);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        for (modelIndex = 0; modelIndex < 4; modelIndex++) {
            fn_800269E8(players[playerOrder[playerIndex]].modelIds[modelIndex],
                             (1 << 0));
        }
        fn_80026A44(players[playerOrder[playerIndex]].modelIds[scoreOrder[playerIndex]],
                           (1 << 0));
        players[playerOrder[playerIndex]].previousRank =
            players[playerOrder[playerIndex]].currentRank;
        players[playerOrder[playerIndex]].currentRank = scoreOrder[playerIndex];
        players[playerOrder[playerIndex]].targetRank = scoreOrder[playerIndex];
        fn_1_D518(playerOrder[playerIndex], playerIndex);
    }
}

typedef struct MgmDecathlonPlayerResult_s {
    u16 currentRank; /* currently displayed tied rank, unused by this update */
    s16 targetRank; /* tied rank calculated from the new totals */
    u8 previousRankAndCurrentRowBytes[4]; /* prior tied rank and current row, unused here */
    s16 targetRow; /* sorted screen row, zero through three */
    u8 remainingDisplayBytes[194]; /* previous row and model IDs, unused here */
} MgmDecathlonPlayerResult;

extern void fn_1_E058(s16 *playerOrder, s16 *scoreOrder);

typedef struct MgmDecathlonPlayerModelSlots_s {
    u8 rankStateBytes[12]; /* rank and row bookkeeping, unused by this helper */
    HU3D_MODELID modelIds[4]; /* tied-rank label model IDs */
    u8 scoreModelBytes[184]; /* character marker and score models, unused here */
} MgmDecathlonPlayerModelSlots;

extern void fn_1_D518(s16 playerIndex, s16 orderIndex);

/* The rank animation selects the model for this player's current score rank. */
void fn_1_DE84(s16 playerNo) {
    s16 playerOrder[4];
    s16 scoreOrder[4];
    s16 index;
    s16 selectedOrderIndex;
    MgmDecathlonPlayerModelSlots *players;

    players = lbl_1_bss_B8->data;
    fn_1_E058(playerOrder, scoreOrder);
    for (index = 0; index < 4; index++) {
        if (playerNo == playerOrder[index]) {
            selectedOrderIndex = index;
        }
    }
    for (index = 0; index < 4; index++) {
        fn_800269E8(players[playerOrder[selectedOrderIndex]].modelIds[index],
                         (1 << 0));
    }
    fn_80026A44(
        players[playerOrder[selectedOrderIndex]].modelIds[scoreOrder[selectedOrderIndex]],
        (1 << 0));
    fn_1_D518(playerOrder[selectedOrderIndex], selectedOrderIndex);
}

/* Before moving result rows, the callback records each player's target tied rank and sorted row. */
void fn_1_DFB0(void) {
    s16 playerOrder[4];
    s16 scoreOrder[4];
    s16 index;
    MgmDecathlonPlayerResult *players;

    players = lbl_1_bss_B8->data;
    fn_1_E058(playerOrder, scoreOrder);
    for (index = 0; index < 4; index++) {
        players[playerOrder[index]].targetRank = scoreOrder[index];
        players[playerOrder[index]].targetRow = index;
    }
}

extern s16 MgModeScore[4];

/* Score displays call this to sort four players and assign tied rank positions. */
void fn_1_E058(s16 *playerOrder, s16 *scores) {
    s16 ranks[4];
    s16 player;
    s16 nextPlayer;
    s16 swapValue;

    for (player = 0; player < 4; player++) {
        playerOrder[player] = player;
        scores[player] = MgModeScore[player];
    }
    for (player = 0; player < 4; player++) {
        for (nextPlayer = player + 1; nextPlayer < 4; nextPlayer++) {
            if (scores[player] < scores[nextPlayer]) {
                swapValue = scores[player];
                scores[player] = scores[nextPlayer];
                scores[nextPlayer] = swapValue;
                swapValue = playerOrder[player];
                playerOrder[player] = playerOrder[nextPlayer];
                playerOrder[nextPlayer] = swapValue;
            }
        }
    }
    for (player = 0; player < 4; player++) {
        ranks[player] = player;
    }
    for (player = 1; player < 4; player++) {
        if (scores[player] == scores[player - 1]) {
            ranks[player] = ranks[player - 1];
        }
    }
    /* Return tied ranks in sorted order; the temporary raw scores are no longer needed. */
    for (player = 0; player < 4; player++) {
        scores[player] = ranks[player];
    }
}

typedef struct DecathlonScoreBand {
    s32 active; /* nonzero for a valid band; zero ends the event's table */
    s32 score; /* raw minigame score at this band boundary */
    s32 award; /* Decathlon points at this band boundary */
} DecathlonScoreBand;

DecathlonScoreBand lbl_1_data_744[10][11] = {
    {{1, 0, 0}, {1, 5, 100}, {1, 10, 300}, {1, 15, 600}, {1, 20, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 50, 100}, {1, 100, 300}, {1, 150, 500}, {1, 170, 700}, {1, 190, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 900, 1000}, {1, 1200, 600}, {1, 1800, 300}, {1, 3600, 100}, {1, 5400, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 900, 100}, {1, 1800, 300}, {1, 3600, 500}, {1, 6300, 700}, {1, 8460, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 10, 100}, {1, 20, 300}, {1, 30, 500}, {1, 35, 700}, {1, 40, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 10, 100}, {1, 20, 300}, {1, 40, 500}, {1, 50, 750}, {1, 65, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 5, 100}, {1, 10, 300}, {1, 15, 600}, {1, 20, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 1560, 1000}, {1, 1620, 800}, {1, 1800, 600}, {1, 2100, 400}, {1, 2400, 300}, {1, 3600, 100}, {1, 5400, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 300, 100}, {1, 1000, 300}, {1, 2000, 600}, {1, 3500, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
    {{1, 0, 0}, {1, 10, 100}, {1, 15, 300}, {1, 20, 500}, {1, 25, 700}, {1, 27, 1000}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}},
};

/* The mode sequence converts a minigame score into its Decathlon award points. */
s32 fn_1_E23C(s16 eventNo, s16 playerNo) {
    f32 scoreRange;
    f32 scoreFraction;
    f32 scoreDelta;
    s32 playerScore;
    s32 storedScore;
    s32 award;
    s16 band;

    if (eventNo >= 10) {
        return 0;
    }
    storedScore = lbl_80265750[playerNo].mgScore;
    playerScore = storedScore;
    band = award = 0;
    for (band = 0; band < 11 && lbl_1_data_744[eventNo][band].active != 0; band++) {
        if (playerScore <= 0) {
            award = 0;
        } else if (lbl_1_data_744[eventNo][band].score >= playerScore && band == 0) {
            award = lbl_1_data_744[eventNo][band].award;
        } else if (lbl_1_data_744[eventNo][band].score <= playerScore &&
                   lbl_1_data_744[eventNo][band + 1].active == 0) {
            award = lbl_1_data_744[eventNo][band].award;
        } else if (lbl_1_data_744[eventNo][band].score <= playerScore &&
                   playerScore <= lbl_1_data_744[eventNo][band + 1].score &&
                   lbl_1_data_744[eventNo][band + 1].active != 0) {
            scoreRange = (f32)(lbl_1_data_744[eventNo][band + 1].score -
                               lbl_1_data_744[eventNo][band].score);
            scoreDelta = (f32)(playerScore - lbl_1_data_744[eventNo][band].score);
            scoreFraction = 0.0f;
            if (0.0f != scoreRange) {
                scoreFraction = scoreDelta / scoreRange;
            }
            scoreRange = (f32)(lbl_1_data_744[eventNo][band + 1].award -
                               lbl_1_data_744[eventNo][band].award);
            scoreRange *= scoreFraction;
            award = (s32)(scoreRange +
                          (f32)lbl_1_data_744[eventNo][band].award);
        }
    }
    return award;
}

extern int MgModeWorkInt[16];
extern void fn_1_E870(s16 playerNo);

/* After each event, the mode sequence records the score and updates its best mark. */
void fn_1_E64C(s16 eventNo, s16 playerNo) {
    s32 playerScore;
    s32 comparisonScore;

    lbl_802669DC[playerNo].mgScore[eventNo] = (u16)MgModeWorkInt[playerNo];
    playerScore = lbl_80265750[playerNo].mgScore;
    comparisonScore = playerScore;
    if (lbl_80265728[playerNo].type == 0) {
        /* Events two and seven reward lower positive times; the other events reward higher
         * scores. */
        if (((eventNo == 2) || (eventNo == 7)) &&
            (MgModeWorkInt[eventNo + 4] > comparisonScore) && (comparisonScore > 0)) {
            MgModeWorkInt[eventNo + 4] = comparisonScore;
        }
        if ((eventNo != 2) && (eventNo != 7) &&
            (MgModeWorkInt[eventNo + 4] < comparisonScore) && (comparisonScore > 0)) {
            MgModeWorkInt[eventNo + 4] = comparisonScore;
        }
    }
}

/* At Decathlon's end, the mode sequence saves player totals and records. */
void fn_1_E7A4(void) {
    s16 playerNo;

    playerNo = 0;
    while (playerNo < 4) {
        lbl_802669DC[playerNo].charNo = (s8)lbl_80265728[playerNo].charNo;
        lbl_802669DC[playerNo].finalScore = (u16)MgModeScore[playerNo];
        if (lbl_80265728[playerNo].type == 0) {
            fn_1_E870(playerNo);
        }
        playerNo += 1;
    }
}

/* The end-of-mode save inserts this human player's total into the record table. */
void fn_1_E870(s16 playerNo) {
    GW_DECA_SCORE movingScore;
    GW_DECA_SCORE displacedScore;
    s16 index;

    movingScore = lbl_802669DC[playerNo];
    for (index = 0; index < 10; index++) {
        if ((s8)movingScore.charNo < 0) {
            break;
        }
        /* New totals are placed before existing ties; an empty slot also accepts the entry. */
        if ((s8)lbl_80265E30.decaScore[index].charNo < 0 ||
            lbl_80265E30.decaScore[index].finalScore <= movingScore.finalScore) {
            displacedScore = lbl_80265E30.decaScore[index];
            lbl_80265E30.decaScore[index] = movingScore;
            movingScore = displacedScore;
        }
    }
    for (index = 0; index < 10; index++) {
        if (index != 2 && index != 7 &&
            (s32)lbl_80265E30.decaMgRecord[index] < (s32)MgModeWorkInt[index + 4]) {
            lbl_80265E30.decaMgRecord[index] = (u16)MgModeWorkInt[index + 4];
        }
    }
    for (index = 0; index < 10; index++) {
        if ((index == 2 || index == 7) &&
            (s32)lbl_80265E30.decaMgRecord[index] > (s32)MgModeWorkInt[index + 4] &&
            (s32)MgModeWorkInt[index + 4] > 0) {
            lbl_80265E30.decaMgRecord[index] = (u16)MgModeWorkInt[index + 4];
        }
    }
}
