/* Battle character motion, player selection, and round results presentation. */
#include "dolphin/math.h"
#include "REL/mgmbattledll/gameplay_types.h"
#include "game/charman.h"
#include "game/hsfex.h"
#include "game/object.h"
#include "humath.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "dolphin.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/gx.h"
#include "dolphin/pad.h"
#include "dolphin/mtx.h"
#include "string.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "game/gamework.h"
#include "game/audio.h"
#include "msm_se.h"
#define MGBATTLE_SE_SELECTION_MOVE 1211
#define MGBATTLE_SE_PANEL_TRANSITION 1212
#define MGBATTLE_SE_TABLE_LOOP 1213
#define MGBATTLE_SE_MINIGAME_TRANSITION 1214
#define MGBATTLE_SE_SELECTION_ACCEPT 1216
#define MGBATTLE_STREAM_RESULTS_JINGLE 62
#define MGBATTLE_CHARFX_SELECTION_START 593
#define MGBATTLE_CHARFX_SELECTION_END 579
#define MGBATTLE_MODE_LIST_END 255
#define MGBATTLE_NO_OVERLAY 65535
#define MGBATTLE_NO_CHOICE 65535
#define MGBATTLE_UNLOCK_GAME_BASE 601
#define MGBATTLE_MODE_LIST_END 255
#define MGBATTLE_NO_OVERLAY 65535
#define MGBATTLE_NO_CHOICE 65535
#define MGBATTLE_SE_SELECTION_MOVE 1211
#define MGBATTLE_SE_PANEL_TRANSITION 1212
#define MGBATTLE_SE_TABLE_LOOP 1213
#define MGBATTLE_SE_MINIGAME_TRANSITION 1214
#define MGBATTLE_SE_SELECTION_ACCEPT 1216
#define MGBATTLE_STREAM_RESULTS_JINGLE 62
#define MGBATTLE_CHARFX_SELECTION_START 593
#define MGBATTLE_CHARFX_SELECTION_END 579
#include "game/sprite.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/main.h"
#include "game/pad.h"
#include "game/window.h"
#include "game/wipe.h"
#include "game/frand.h"
#include "game/process.h"
#include "game/gamemes.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/mg/seqman.h"
#include "game/mg/actman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/board/object.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/tutorial.h"
#include "game/board/masu.h"
#include "game/board/player.h"

/* Applies a motion-table entry to one character model during setup or a mode change. */
void fn_1_AB04(s16 playerIndex, s16 motionEntry) {
    s16 motionIndex;

    if ((playerIndex < 0) || (playerIndex >= 4) || (motionEntry < 0) || (motionEntry >= 14)) {
        return;
    }
    if (motionEntry != lbl_1_bss_4D4[playerIndex].currentMotion) {
        motionIndex = lbl_1_data_4F8[motionEntry].motionIndex;
        if (motionIndex != -1) {
            if (lbl_1_data_4F8[motionEntry].transitionTime <= 1.0f) {
                CharMotionSet(lbl_1_bss_4D4[playerIndex].charNo,
                              lbl_1_bss_4D4[playerIndex].motions[motionIndex]);
                Hu3DModelAttrReset(lbl_1_bss_4D4[playerIndex].model,
                                   Hu3DModelMotionAttrGet(lbl_1_bss_4D4[playerIndex].model) |
                                       HU3D_MOTATTR);
                CharModelAttrSet(lbl_1_bss_4D4[playerIndex].charNo,
                                 lbl_1_data_4F8[motionEntry].modelAttr);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_4D4[playerIndex].model, HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(lbl_1_bss_4D4[playerIndex].charNo,
                                   lbl_1_bss_4D4[playerIndex].motions[motionIndex], 0.0f,
                                   lbl_1_data_4F8[motionEntry].transitionTime,
                                   lbl_1_data_4F8[motionEntry].modelAttr);
            }
            CharModelVoiceFlagSet(lbl_1_bss_4D4[playerIndex].charNo,
                                  lbl_1_data_4F8[motionEntry].voiceFlag);
        }
        lbl_1_bss_4D4[playerIndex].currentMotion = motionEntry;
    }
}

f32 fn_1_AD8C(s16 playerIndex) {
    return lbl_1_bss_4D4[playerIndex].height;
}

/* Creates each player's character model and establishes its initial pose. */
void fn_1_ADA8(void) {
    s16 introMotion;
    s32 player;
    s32 motionIndex;

    for (player = 0; player < 4; player++) {
        lbl_1_bss_4D4[player].charNo = GwPlayerConf[player].charNo;
        lbl_1_bss_4D4[player].padNo = GwPlayerConf[player].padNo;
        lbl_1_bss_4D4[player].grpNo = GwPlayerConf[player].grpNo;
        lbl_1_bss_4D4[player].playerType = GwPlayerConf[player].type;
        lbl_1_bss_4D4[player].comDif = GwPlayerConf[player].comDif;
        lbl_1_bss_4D4[player].mode = 0;
        lbl_1_bss_4D4[player].model = CharModelCreate(
            lbl_1_bss_4D4[player].charNo, 2);
        for (motionIndex = 0; motionIndex < 12; motionIndex++) {
            lbl_1_bss_4D4[player].motions[motionIndex] = CharMotionCreate(
                lbl_1_bss_4D4[player].charNo, lbl_1_data_3F8[motionIndex]);
        }
        CharMotionDataClose(lbl_1_bss_4D4[player].charNo);
        lbl_1_bss_4D4[player].currentMotion = -1;
        fn_1_AB04((s16)player, 0);
        Hu3DModelCameraSet(lbl_1_bss_4D4[player].model, 1);
        Hu3DModelLayerSet(lbl_1_bss_4D4[player].model, 2);
        Hu3DModelShadowSet(lbl_1_bss_4D4[player].model);
        lbl_1_bss_4D4[player].height = CharModelHeightGet(
            lbl_1_bss_4D4[player].charNo);
        lbl_1_bss_4D4[player].state = 0;
        lbl_1_bss_4D4[player].scaleX =
            lbl_1_bss_4D4[player].verticalVelocity =
                lbl_1_bss_4D4[player].scaleZ = 0.0f;
        lbl_1_bss_4D4[player].position = lbl_1_data_428[0][player];
        lbl_1_bss_4D4[player].position.y = 5.0f;
        Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
            &lbl_1_bss_4D4[player].position);
        lbl_1_bss_4D4[player].rotation.x =
            lbl_1_bss_4D4[player].rotation.y =
                lbl_1_bss_4D4[player].rotation.z = 0.0f;
        Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
            &lbl_1_bss_4D4[player].rotation);
        Hu3DLLightCreate(lbl_1_bss_4D4[player].model, 0.0f, 750.0f,
            1500.0f, 0.0f, -0.5f, -1.0f, 192, 192, 192);
        CharMotionVoiceOnSet(lbl_1_bss_4D4[player].charNo,
            (s16)lbl_1_data_3F8[7], 0);
        CharMotionVoiceOnSet(lbl_1_bss_4D4[player].charNo,
            (s16)lbl_1_data_3F8[3], 0);
    }
    CharEffectLayerSet(3);
}

/* Copies stage positions into the player records and prepares the coin bonus display. */
void fn_1_B414(void) {
    s16 player;
    s16 otherPlayer;
    s16 validPlayers;
    s16 coinBonus;

    validPlayers = 0;
    for (player = 0; player < 4; player++) {
        lbl_1_bss_4D4[player].position.x = MgModeWorkFloat[player];
        lbl_1_bss_4D4[player].position.y = MgModeWorkFloat[player + 4];
        lbl_1_bss_4D4[player].position.z = MgModeWorkFloat[player + 8];
        lbl_1_bss_4D4[player].rotation.x = (0.0f);
        lbl_1_bss_4D4[player].rotation.y = MgModeWorkFloat[player + 12];
        lbl_1_bss_4D4[player].rotation.z = (0.0f);
        Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
                         &lbl_1_bss_4D4[player].position);
        Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
                         &lbl_1_bss_4D4[player].rotation);
        GwPlayerConf[player].grpNo = lbl_1_bss_4D4[player].grpNo = MgBattleOrder[player];
        coinBonus = GwPlayer[player].mgCoinBonus;
        lbl_1_data_4F0[player] = coinBonus;
        if (lbl_1_data_4F0[player] < 10) {
            lbl_1_data_4F0[player] = 0;
        } else {
            /* The bonus display is capped at ten coins per eligible player. */
            lbl_1_data_4F0[player] = 10;
            validPlayers++;
        }
    }

    if (*(s16 *)&MgModeSubMode == 0) {
        if (validPlayers > 1) {
            memset(lbl_1_data_4F0, 0, 8);
        }
    } else {
        for (player = 0; player < 4; player++) {
            if (lbl_1_data_4F0[player] != 0) {
                for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
                    if (lbl_1_bss_4D4[player].grpNo !=
                            lbl_1_bss_4D4[otherPlayer].grpNo &&
                        lbl_1_data_4F0[otherPlayer] != 0) {
                        memset(lbl_1_data_4F0, 0, 8);
                    }
                }
            }
        }
        for (player = 0; player < 4; player++) {
            if (lbl_1_data_4F0[player] != 0) {
                for (otherPlayer = 0; otherPlayer < 4; otherPlayer++) {
                    if (lbl_1_bss_4D4[player].grpNo ==
                        lbl_1_bss_4D4[otherPlayer].grpNo) {
            lbl_1_data_4F0[otherPlayer] = 10;
                    }
                }
            }
        }
    }
}

/* Returns the leftmost player X position for the round camera bounds. */
f32 fn_1_B85C(void) {
    f32 minimumX;
    s16 player;

    minimumX = (60000.0f);
    player = 0;
    while (player < 4) {
        if (minimumX > lbl_1_bss_4D4[player].position.x) {
            minimumX = lbl_1_bss_4D4[player].position.x;
        }
        player += 1;
    }
    return minimumX;
}

/* Returns the rightmost player X position for the round camera bounds. */
f32 fn_1_B8E0(void) {
    f32 maximumX;
    s16 player;

    maximumX = (-60000.0f);
    player = 0;
    while (player < 4) {
        if (maximumX < lbl_1_bss_4D4[player].position.x) {
            maximumX = lbl_1_bss_4D4[player].position.x;
        }
        player += 1;
    }
    return maximumX;
}

/* Finds the highest current score among the four players. */
s16 fn_1_B964(void) {
    s16 highestScore;
    s16 player;

    highestScore = MgModeScore[0];
    player = 1;
    while (player < 4) {
        if ((s16) MgModeScore[player] > highestScore) {
            highestScore = MgModeScore[player];
        }
        player += 1;
    }
    return highestScore;
}

/* Measures the widest score gap used when deciding whether a round is settled. */
s16 fn_1_B9E8(void) {
    s16 scoreDifference;
    s16 firstPlayer;
    s16 largestDifference;
    s16 secondPlayer;
    largestDifference = 0;
    firstPlayer = 0;
    while (firstPlayer < 4)
    {
        secondPlayer = 0;
        while (secondPlayer < 4)
        {
            scoreDifference = MgModeScore[firstPlayer] - MgModeScore[secondPlayer];
            if (scoreDifference > largestDifference)
            {
                largestDifference = scoreDifference;
            }
            secondPlayer += 1;
        }
        firstPlayer += 1;
    }
    return largestDifference;
}

/* Returns the character for a player who has reached the 10-coin bonus threshold. */
s16 fn_1_BA94(s16 playerIndex) {
    s16 character = lbl_1_bss_4D4[playerIndex].charNo;

    if (lbl_1_data_4F0[playerIndex] >= 10) {
        return character;
    }
    return -1;
}

/* Returns the character for a player who has reached the round-win threshold. */
s16 fn_1_BAEC(s16 playerIndex) {
    if (MgModeScore[playerIndex] >= MgGameNo) {
        return lbl_1_bss_4D4[playerIndex].charNo;
    }
    return -1;
}

s16 fn_1_BB40(s16 playerIndex) {
    return (s16) lbl_1_bss_4D4[playerIndex].playerType;
}

s16 fn_1_BB60(s16 playerIndex) {
    return lbl_1_bss_4D4[playerIndex].padNo;
}

/* Saves the players' positions and headings before the results sequence. */
void fn_1_BB7C(void) {
    s16 player;

    player = 0;
    while (player < 4) {
        MgModeWorkFloat[player] = lbl_1_bss_4D4[player].position.x;
        MgModeWorkFloat[(player + 4)] = lbl_1_bss_4D4[player].position.y;
        MgModeWorkFloat[(player + 8)] = lbl_1_bss_4D4[player].position.z;
        MgModeWorkFloat[(player + 12)] = lbl_1_bss_4D4[player].rotation.y;
        player += 1;
    }
}

/* Turns a player toward the award position and reports when movement finishes. */
s16 fn_1_BC74(s16 player) {
    Point3d separation;
    Point3d rotatedAxis;
    Point3d xAxis;
    Point3d zAxis;
    Point3d adjustedZAxis;
    Point3d angleSeparation;
    Point3d angleDirection;
    Point3d shortXDirection;
    Point3d shortZDirection;
    Point3d shortAdjustedZDirection;
    Point3d shortAngleSeparation;
    Point3d shortAngleDirection;
    Point3d *xAxisPointer;
    Point3d *zAxisPointer;
    Point3d *adjustedZAxisPointer;
    Point3d *angleDirectionPointer;
    Point3d *angleSeparationPointer;
    Point3d *shortXDirectionPointer;
    Point3d *shortZDirectionPointer;
    Point3d *shortAdjustedZDirectionPointer;
    Point3d *shortAngleDirectionPointer;
    Point3d *shortAngleSeparationPointer;
    f32 angle;
    f32 originalRotation;
    f32 firstDot;
    s16 adjustPosition;
    s16 movingMotion;
    s16 idleMotion;
    s16 keepRunning;

    keepRunning = 0;
    adjustPosition = 0;
    PSVECSubtract(&lbl_1_bss_4D4[player].awardPosition,
        &lbl_1_bss_4D4[player].position, &separation);
    separation.y = 0.0f;

    if (PSVECMag(&separation) > 7.0f) {
        fn_1_AB04(player, 3);

        fn_1_1AF5C(&separation, &separation);
        adjustPosition = 0;
        originalRotation = lbl_1_bss_4D4[player].rotation.y;

        rotatedAxis.x = 1.0f;
        rotatedAxis.y = 0.0f;
        rotatedAxis.z = 0.0f;
        xAxis = rotatedAxis;
        xAxisPointer = &xAxis;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            xAxisPointer, &rotatedAxis);
        firstDot = PSVECDotProduct(&separation, &rotatedAxis);

        rotatedAxis.x = rotatedAxis.y = 0.0f;
        rotatedAxis.z = 1.0f;
        zAxis = rotatedAxis;
        zAxisPointer = &zAxis;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            zAxisPointer, &rotatedAxis);
        angle = PSVECDotProduct(&separation, &rotatedAxis);
        if (angle >= 0.0f) {
            if (firstDot >= 0.0f) {
                lbl_1_bss_4D4[player].rotation.y += 20.0f;
            } else {
                lbl_1_bss_4D4[player].rotation.y -= 20.0f;
            }
            rotatedAxis.x = rotatedAxis.y = 0.0f;
            rotatedAxis.z = 1.0f;
            adjustedZAxis = rotatedAxis;
            adjustedZAxisPointer = &adjustedZAxis;
            fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
                adjustedZAxisPointer, &rotatedAxis);
            angleDirection = rotatedAxis;
            angleDirectionPointer = &angleDirection;
            angleSeparation = separation;
            angleSeparationPointer = &angleSeparation;
            angle = (f32)fn_1_1B16C(angleSeparationPointer,
                angleDirectionPointer);
            if (-20.0f < angle && angle < 20.0f) {
                lbl_1_bss_4D4[player].rotation.y = originalRotation;
                adjustPosition = 1;
            }
        } else if (firstDot >= 0.0f) {
            lbl_1_bss_4D4[player].rotation.y += 20.0f;
        } else {
            lbl_1_bss_4D4[player].rotation.y -= 20.0f;
        }

        if (adjustPosition != 0) {
            PSVECSubtract(&lbl_1_bss_4D4[player].awardPosition,
                &lbl_1_bss_4D4[player].position, &separation);
            separation.y = 0.0f;
            if (PSVECMag(&separation) > 7.0f) {
                fn_1_1AF5C(&separation, &separation);
                PSVECScale(&separation, &separation, 7.0f);
                lbl_1_bss_4D4[player].position.x = separation.x +
                    lbl_1_bss_4D4[player].position.x;
                lbl_1_bss_4D4[player].position.y = 5.0f;
                lbl_1_bss_4D4[player].position.z = separation.z +
                    lbl_1_bss_4D4[player].position.z;
            } else {
                lbl_1_bss_4D4[player].position =
                    lbl_1_bss_4D4[player].awardPosition;
                lbl_1_bss_4D4[player].position.y = 5.0f;
            }
        }
    } else {
        fn_1_AB04(player, 1);

        lbl_1_bss_4D4[player].position =
            lbl_1_bss_4D4[player].awardPosition;
        separation.x = 1.0f;
        separation.y = separation.z = 0.0f;
        rotatedAxis.x = 1.0f;
        rotatedAxis.y = 0.0f;
        rotatedAxis.z = 0.0f;
        shortXDirection = rotatedAxis;
        shortXDirectionPointer = &shortXDirection;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            shortXDirectionPointer, &rotatedAxis);
        firstDot = PSVECDotProduct(&separation, &rotatedAxis);
        rotatedAxis.x = rotatedAxis.y = 0.0f;
        rotatedAxis.z = 1.0f;
        shortZDirection = rotatedAxis;
        shortZDirectionPointer = &shortZDirection;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            shortZDirectionPointer, &rotatedAxis);
        angle = PSVECDotProduct(&separation, &rotatedAxis);
        if (angle >= 0.0f) {
            if (firstDot > 0.0f) {
                lbl_1_bss_4D4[player].rotation.y += 20.0f;
            } else if (firstDot < 0.0f) {
                lbl_1_bss_4D4[player].rotation.y -= 20.0f;
            }
            rotatedAxis.x = rotatedAxis.y = 0.0f;
            rotatedAxis.z = 1.0f;
            shortAdjustedZDirection = rotatedAxis;
            shortAdjustedZDirectionPointer = &shortAdjustedZDirection;
            fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
                shortAdjustedZDirectionPointer, &rotatedAxis);
            shortAngleDirection = rotatedAxis;
            shortAngleDirectionPointer = &shortAngleDirection;
            shortAngleSeparation = separation;
            shortAngleSeparationPointer = &shortAngleSeparation;
            angle = (f32)fn_1_1B16C(shortAngleSeparationPointer,
                shortAngleDirectionPointer);
            if (-20.0f < angle && angle < 20.0f) {
                lbl_1_bss_4D4[player].rotation.y = 90.0f;
                keepRunning = 1;
            }
        } else if (firstDot >= 0.0f) {
            lbl_1_bss_4D4[player].rotation.y += 20.0f;
        } else {
            lbl_1_bss_4D4[player].rotation.y -= 20.0f;
        }
    }

    Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].rotation);
    return keepRunning;
}

u32 lbl_1_data_3F8[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 36),
    DATANUM(DATA_mariomot, 37), DATANUM(DATA_mariomot, 21),
    DATANUM(DATA_mariomot, 34), DATANUM(DATA_mariomot, 63)
};

Point3d lbl_1_data_428[3][4] = {
    { { -300.0f, 0.0f, 200.0f }, { -100.0f, 0.0f, 200.0f }, { 100.0f, 0.0f, 200.0f }, { 300.0f, 0.0f, 200.0f } },
    { { -300.0f, 0.0f, 200.0f }, { -60.0f, 0.0f, 200.0f }, { 120.0f, 0.0f, 200.0f }, { 300.0f, 0.0f, 200.0f } },
    { { -300.0f, 0.0f, 200.0f }, { -125.0f, 0.0f, 200.0f }, { 125.0f, 0.0f, 200.0f }, { 300.0f, 0.0f, 200.0f } }
};

Point3d lbl_1_data_4B8[4] = { { 540.0f, 0.0f, -285.0f },
                              { 540.0f, 0.0f, -95.0f },
                              { 540.0f, 0.0f, 95.0f },
                              { 540.0f, 0.0f, 285.0f } };

f32 lbl_1_data_4E8[2] = { 300.0f, 100.0f };

s16 lbl_1_data_4F0[4] = { 0, 0, 0, 0 };

MgBattleMotion lbl_1_data_4F8[14] = {
    { 0, 0.0f, HU3D_MOTATTR_LOOP, 1 },
    { 0, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 1, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 2, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 3, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 4, 0.0f, HU3D_MOTATTR_LOOP, 1 },
    { 4, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 5, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 6, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 7, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 8, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 9, 8.0f, HU3D_MOTATTR_LOOP, 0 },
    { 10, 8.0f, HU3D_MOTATTR_LOOP, 1 },
    { 11, 8.0f, HU3D_MOTATTR_LOOP, 1 }
};

/* Advances one player's result animation; fn_1_17A74 polls the returned choice. */
s16 fn_1_CA1C(s16 playerIndex, s16 choice) {
    f32 motionEndTime;
    s16 unusedLocalA;
    s16 unusedLocalB;
    s16 unusedLocalC;
    s16 unusedLocalD;
    s16 unusedLocalE;
    s16 unusedLocalF;
    s16 unusedLocalG;
    s16 unusedLocalH;
    s16 unusedLocalI;
    s16 nextChoice;

    nextChoice = choice;
    if (choice < 0) {
        return -1;
    }
    if (choice > 0) {
        switch (lbl_1_bss_4D4[playerIndex].mode) {  /* switch 1 */
            case 0:                                 /* switch 1 */
                lbl_1_bss_4D4[playerIndex].mode = 2;
                break;
            case 2:                                 /* switch 1 */
                if (lbl_1_bss_4D4[playerIndex].currentMotion != 4) {
                    fn_1_AB04(playerIndex, 4);
                    _CharFXPlay(lbl_1_bss_4D4[playerIndex].charNo, MGBATTLE_CHARFX_SELECTION_START,
                                0);
                    lbl_1_bss_4D4[playerIndex].state = 0;
                    lbl_1_bss_4D4[playerIndex].mode = 3;
                } else {
                    lbl_1_bss_4D4[playerIndex].mode = 3;
                    lbl_1_bss_4D4[playerIndex].state = 0;
                }
                break;
            case 3:                                 /* switch 1 */
                if ((lbl_1_bss_4D4[playerIndex].currentMotion == 4) &&
                    (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0)) {
                    motionEndTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
                    if ((Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo)) >= motionEndTime) {
                        CharModelAttrSet(lbl_1_bss_4D4[playerIndex].charNo, HU3D_MOTATTR_PAUSE);
                    }
                }
                lbl_1_bss_4D4[playerIndex].position.x += 5.0f;
                lbl_1_bss_4D4[playerIndex].position.y +=
                    15.0 * cos((3.141592653589793 *
                                (f64) (3.6f * (f32) lbl_1_bss_4D4[playerIndex].state)) /
                               180.0);
                if (lbl_1_bss_4D4[playerIndex].state >= 50) {
                    lbl_1_bss_4D4[playerIndex].mode = 4;
                    lbl_1_bss_4D4[playerIndex].state = 0;
                    lbl_1_bss_4D4[playerIndex].position.y = 0.0f;
                }
                lbl_1_bss_4D4[playerIndex].state += 1;
                break;
            case 4:                                 /* switch 1 */
                if (lbl_1_bss_4D4[playerIndex].currentMotion == 4) {
                    fn_1_AB04(playerIndex, 5);
                    fn_1_3840();
                    if ((s32) lbl_1_bss_4D4[playerIndex].playerType == 0) {
                        omVibrate(playerIndex, 20, 4, 4);
                    }
                } else if ((lbl_1_bss_4D4[playerIndex].currentMotion == 5) &&
                           (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0)) {
                    motionEndTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
                    if ((Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo)) >= motionEndTime) {
                        fn_1_AB04(playerIndex, 1);
                        lbl_1_bss_4D4[playerIndex].mode = 8;
                        lbl_1_bss_4D4[playerIndex].state = 0;
                    }
                }
                break;
            case 8:                                 /* switch 1 */
                if (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
                    fn_1_AB04(playerIndex, 2);
                    lbl_1_bss_4D4[playerIndex].rotation.y *= 0.8f;
                    if ((-1.0f < lbl_1_bss_4D4[playerIndex].rotation.y) &&
                        (lbl_1_bss_4D4[playerIndex].rotation.y < 1.0)) {
                        lbl_1_bss_4D4[playerIndex].rotation.y = 0.0f;
                        lbl_1_bss_4D4[playerIndex].mode = 9;
                        lbl_1_bss_4D4[playerIndex].state = 0;
                    }
                }
                break;
            case 9:                                 /* switch 1 */
                fn_1_4548(1);
                lbl_1_bss_4D4[playerIndex].rotation.y = 0.0f;
                if (lbl_1_bss_4D4[playerIndex].currentMotion != 9) {
                    fn_1_AB04(playerIndex, 9);
                    _CharFXPlay(lbl_1_bss_4D4[playerIndex].charNo, MGBATTLE_CHARFX_SELECTION_END,
                                0);
                } else if (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
                    motionEndTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
                    if ((Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo)) >= motionEndTime) {
    Hu3DModelAttrSet(lbl_1_bss_4D4[playerIndex].model, HU3D_MOTATTR_PAUSE);
                        nextChoice = -1;
                    }
                }
                break;
            default:
                break;
            }
    } else {
        switch (lbl_1_bss_4D4[playerIndex].mode) { /* switch 2 */
            case 0:                                 /* switch 2 */
                if (fn_1_394C() != 0) {
                    lbl_1_bss_4D4[playerIndex].mode = 2;
                    if ((s32) lbl_1_bss_4D4[playerIndex].playerType == 0) {
                        omVibrate(playerIndex, 20, 7, 3);
                    }
                }
                break;
            case 2:                                 /* switch 2 */
                if (lbl_1_bss_4D4[playerIndex].currentMotion != 11) {
                    fn_1_AB04(playerIndex, 11);
                }
                lbl_1_bss_4D4[playerIndex].mode = 3;
                lbl_1_bss_4D4[playerIndex].state = 0;
                break;
            case 3:                                 /* switch 2 */
                Hu3DMotionSpeedSet(lbl_1_bss_4D4[playerIndex].model, 2.0f);
                if (lbl_1_bss_4D4[playerIndex].state >= 60) {
                    lbl_1_bss_4D4[playerIndex].mode = 4;
                }
                lbl_1_bss_4D4[playerIndex].state += 1;
                break;
            case 4:                                 /* switch 2 */
                Hu3DMotionSpeedSet(lbl_1_bss_4D4[playerIndex].model, 1.0f);
                fn_1_AB04(playerIndex, 1);
                lbl_1_bss_4D4[playerIndex].mode = 8;
                lbl_1_bss_4D4[playerIndex].state = 0;
                break;
            case 8:                                 /* switch 2 */
                if (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
                    fn_1_AB04(playerIndex, 2);
                    lbl_1_bss_4D4[playerIndex].rotation.y *= 0.8f;
                    if ((-1.0f < lbl_1_bss_4D4[playerIndex].rotation.y) &&
                        (lbl_1_bss_4D4[playerIndex].rotation.y < 1.0)) {
                        lbl_1_bss_4D4[playerIndex].rotation.y = 0.0f;
                        lbl_1_bss_4D4[playerIndex].mode = 9;
                        lbl_1_bss_4D4[playerIndex].state = 0;
                    }
                }
                break;
            case 9:                                 /* switch 2 */
                lbl_1_bss_4D4[playerIndex].rotation.y = 0.0f;
                if (lbl_1_bss_4D4[playerIndex].currentMotion != 10) {
                    fn_1_AB04(playerIndex, 10);
                } else if (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
                    motionEndTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
                    if ((Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                         CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo)) >= motionEndTime) {
                        Hu3DModelAttrSet(lbl_1_bss_4D4[playerIndex].model, HU3D_MOTATTR_PAUSE);
                        nextChoice = -1;
                    }
                }
                break;
            default:
                break;
            }
    }
    Hu3DModelPosSetV(lbl_1_bss_4D4[playerIndex].model,
                     (Point3d *) &lbl_1_bss_4D4[playerIndex].position.x);
    Hu3DModelRotSetV(lbl_1_bss_4D4[playerIndex].model, &lbl_1_bss_4D4[playerIndex].rotation);
    return nextChoice;
}

/* Advances a nonwinner's exit motion and returns whether its animation is still active. */
s16 fn_1_E770(s16 player, s16 result) {
    f32 motionEndTime;
    s16 nextResult;
    s16 transitionMotion;
    s16 nextMotion;

    nextResult = result;
    if (result < 0) {
        return -1;
    }
    switch (lbl_1_bss_4D4[player].mode) {
    case 2:
        if (lbl_1_bss_4D4[player].currentMotion != 10) {
            fn_1_AB04(player, 10);
        }
        lbl_1_bss_4D4[player].mode = 3;
        lbl_1_bss_4D4[player].state = 0;
        break;
    case 3:
        if (CharMotionShiftIDGet(lbl_1_bss_4D4[player].charNo) < 0) {
            motionEndTime = CharMotionMaxTimeGet(
                lbl_1_bss_4D4[player].charNo);
            if (Hu3DData[lbl_1_bss_4D4[player].model].motWork.speed +
                CharMotionTimeGet(lbl_1_bss_4D4[player].charNo) >=
                    motionEndTime) {
                Hu3DModelAttrSet(lbl_1_bss_4D4[player].model, HU3D_MOTATTR_PAUSE);
                lbl_1_bss_4D4[player].mode = 4;
                lbl_1_bss_4D4[player].state = 0;
            }
        }
        break;
    case 4:
        lbl_1_bss_4D4[player].state++;
        if (lbl_1_bss_4D4[player].state >= 25) {
            fn_1_AB04(player, 1);
            lbl_1_bss_4D4[player].mode = 0;
            lbl_1_bss_4D4[player].state = 0;
            nextResult = -1;
        }
        break;
    case 0:
    default:
        lbl_1_bss_4D4[player].mode = 2;
        break;
    }
    Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].rotation);
    return nextResult;
}

/* Turns each player's model away after the round when the main loop polls this state. */
s16 fn_1_EE48(s16 player) {
    s16 result;
    s16 secondaryMotion;
    s16 mainMotion;

    result = 0;
    switch (lbl_1_bss_4D4[player].mode) {
    case 0:
        result = 0;
        break;
    case 1:
        break;
    case 2:
    case 3:
    case 4:
    case 8:
    case 9:
        lbl_1_bss_4D4[player].state = 0;
        Hu3DModelAttrReset(lbl_1_bss_4D4[player].model, HU3D_MOTATTR_PAUSE);
        fn_1_AB04(player, 2);
        lbl_1_bss_4D4[player].mode = 10;
        lbl_1_bss_4D4[player].rotation.y = 0.0f;
        result = 1;
        break;
    case 10:
        result = 1;
        lbl_1_bss_4D4[player].rotation.y += 3.0f;
        if (lbl_1_bss_4D4[player].rotation.y > 87.0f) {
            lbl_1_bss_4D4[player].rotation.y = 90.0f;
            fn_1_AB04(player, 1);
            lbl_1_bss_4D4[player].mode = 0;
            lbl_1_bss_4D4[player].state = 0;
            result = 0;
        }
        break;
    default:
        break;
    }
    Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].rotation);
    return result;
}

/* Assigns award positions to winners and the remaining players before the results scene. */
void fn_1_F464(void) {
    Point3d modelPosition;
    s16 characterByPlayer[5];
    s16 playerOrder[4];
    s16 count;
    s16 eligibleCount;
    s16 otherCount;
    s16 eligibleCharacter;
    s16 player;
    s16 placementIndex;
    s16 swappedPlayer;

    count = 1;
    eligibleCount = 0;
    otherCount = 0;
    player = 0;
    while (player < 4) {
        if (MgModeScore[player] >= MgGameNo) {
            eligibleCharacter = lbl_1_bss_4D4[player].charNo;
        } else {
            eligibleCharacter = -1;
        }
        characterByPlayer[player] = eligibleCharacter;
        if (characterByPlayer[player] >= 0) {
            eligibleCount++;
        } else {
            otherCount++;
        }
        playerOrder[player] = player;
        player++;
    }

    player = 0;
    while (player < 4) {
        for (placementIndex = player + 1; placementIndex < 4; placementIndex++) {
            if (lbl_1_bss_4D4[playerOrder[player]].position.z <
                lbl_1_bss_4D4[playerOrder[placementIndex]].position.z) {
                swappedPlayer = playerOrder[player];
                playerOrder[player] = playerOrder[placementIndex];
                playerOrder[placementIndex] = swappedPlayer;
            }
        }
        player++;
    }

    fn_1_3B78(&modelPosition);
    player = 0;
    placementIndex = 0;
    while (player < 4) {
        lbl_1_bss_4D4[playerOrder[player]].awardPosition.x = 0.0f;
        lbl_1_bss_4D4[playerOrder[player]].awardPosition.y = 0.0f;
        lbl_1_bss_4D4[playerOrder[player]].awardPosition.z = lbl_1_data_4E8[0];
        if (characterByPlayer[playerOrder[player]] >= 0) {
            if (eligibleCount == 1) {
                lbl_1_bss_4D4[playerOrder[player]].awardPosition.x = modelPosition.x;
            } else if (eligibleCount == 2) {
                if (placementIndex == 0) {
                    lbl_1_bss_4D4[playerOrder[player]].awardPosition.x =
                        modelPosition.x - 100.0f;
                } else {
                    lbl_1_bss_4D4[playerOrder[player]].awardPosition.x =
                        100.0f + modelPosition.x;
                }
            } else if (eligibleCount == 3) {
                if (placementIndex == 0) {
                    lbl_1_bss_4D4[playerOrder[player]].awardPosition.x =
                        modelPosition.x - 150.0f;
                } else if (placementIndex == 1) {
                    lbl_1_bss_4D4[playerOrder[player]].awardPosition.x =
                        modelPosition.x;
                } else {
                    lbl_1_bss_4D4[playerOrder[player]].awardPosition.x =
                        150.0f + modelPosition.x;
                }
            }
            placementIndex++;
        }
        player++;
    }

    player = 0;
    placementIndex = 0;
    while (player < 4) {
        if (characterByPlayer[player] < 0) {
            lbl_1_bss_4D4[player].awardPosition.y = 0.0f;
            lbl_1_bss_4D4[player].awardPosition.z = lbl_1_data_4E8[1];
            if (otherCount == 1) {
                lbl_1_bss_4D4[player].awardPosition.x = modelPosition.x;
            } else if (otherCount == 2) {
                if (placementIndex == 0) {
                    lbl_1_bss_4D4[player].awardPosition.x = modelPosition.x - 50.0f;
                } else {
                    lbl_1_bss_4D4[player].awardPosition.x = 50.0f + modelPosition.x;
                }
            } else if (otherCount == 3) {
                if (placementIndex == 0) {
                    lbl_1_bss_4D4[player].awardPosition.x = modelPosition.x - 100.0f;
                } else if (placementIndex == 1) {
                    lbl_1_bss_4D4[player].awardPosition.x = modelPosition.x;
                } else {
                    lbl_1_bss_4D4[player].awardPosition.x = 100.0f + modelPosition.x;
                }
            }
            placementIndex++;
        }
        player++;
    }
}

/* Moves a winning character to its award position; the results loop waits while this returns
 * zero. */
s16 fn_1_FA3C(s16 player) {
    Point3d separation;
    Point3d rotatedAxis;
    Point3d xAxis;
    Point3d zAxis;
    Point3d adjustedZAxis;
    Point3d angleSeparation;
    Point3d angleDirection;
    Point3d shortXDirection;
    Point3d shortZDirection;
    Point3d shortAdjustedZDirection;
    Point3d shortAngleSeparation;
    Point3d shortAngleDirection;
    Point3d *xAxisPointer;
    Point3d *zAxisPointer;
    Point3d *adjustedZAxisPointer;
    Point3d *angleDirectionPointer;
    Point3d *angleSeparationPointer;
    Point3d *shortXDirectionPointer;
    Point3d *shortZDirectionPointer;
    Point3d *shortAdjustedZDirectionPointer;
    Point3d *shortAngleDirectionPointer;
    Point3d *shortAngleSeparationPointer;
    f32 angle;
    f32 originalRotation;
    f32 firstDot;
    s16 adjustPosition;
    s16 movingMotion;
    s16 idleMotion;
    s16 keepRunning;

    keepRunning = 0;
    adjustPosition = 0;
    PSVECSubtract(&lbl_1_bss_4D4[player].awardPosition,
        &lbl_1_bss_4D4[player].position, &separation);
    separation.y = 0.0f;

    if (PSVECMag(&separation) > 150.0f) {
        fn_1_AB04(player, 2);

        fn_1_1AF5C(&separation, &separation);
        adjustPosition = 0;
        originalRotation = lbl_1_bss_4D4[player].rotation.y;

        rotatedAxis.x = 1.0f;
        rotatedAxis.y = 0.0f;
        rotatedAxis.z = 0.0f;
        xAxis = rotatedAxis;
        xAxisPointer = &xAxis;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            xAxisPointer, &rotatedAxis);
        firstDot = PSVECDotProduct(&separation, &rotatedAxis);

        rotatedAxis.x = rotatedAxis.y = 0.0f;
        rotatedAxis.z = 1.0f;
        zAxis = rotatedAxis;
        zAxisPointer = &zAxis;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            zAxisPointer, &rotatedAxis);
        angle = PSVECDotProduct(&separation, &rotatedAxis);
        if (angle >= 0.0f) {
            if (firstDot >= 0.0f) {
                lbl_1_bss_4D4[player].rotation.y += 20.0f;
            } else {
                lbl_1_bss_4D4[player].rotation.y -= 20.0f;
            }
            rotatedAxis.x = rotatedAxis.y = 0.0f;
            rotatedAxis.z = 1.0f;
            adjustedZAxis = rotatedAxis;
            adjustedZAxisPointer = &adjustedZAxis;
            fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
                adjustedZAxisPointer, &rotatedAxis);
            angleDirection = rotatedAxis;
            angleDirectionPointer = &angleDirection;
            angleSeparation = separation;
            angleSeparationPointer = &angleSeparation;
            angle = (f32)fn_1_1B16C(angleSeparationPointer,
                angleDirectionPointer);
            if (-20.0f < angle && angle < 20.0f) {
                lbl_1_bss_4D4[player].rotation.y = originalRotation;
                adjustPosition = 1;
            }
        } else if (firstDot >= 0.0f) {
            lbl_1_bss_4D4[player].rotation.y += 20.0f;
        } else {
            lbl_1_bss_4D4[player].rotation.y -= 20.0f;
        }

        if (adjustPosition != 0) {
            PSVECSubtract(&lbl_1_bss_4D4[player].awardPosition,
                &lbl_1_bss_4D4[player].position, &separation);
            separation.y = 0.0f;
            if (PSVECMag(&separation) > 7.0f) {
                fn_1_1AF5C(&separation, &separation);
                PSVECScale(&separation, &separation, 7.0f);
                lbl_1_bss_4D4[player].position.x = separation.x +
                    lbl_1_bss_4D4[player].position.x;
                lbl_1_bss_4D4[player].position.y = 5.0f;
                lbl_1_bss_4D4[player].position.z = separation.z +
                    lbl_1_bss_4D4[player].position.z;
            } else {
                lbl_1_bss_4D4[player].position =
                    lbl_1_bss_4D4[player].awardPosition;
                lbl_1_bss_4D4[player].position.y = 5.0f;
            }
        }
    } else {
        fn_1_AB04(player, 1);

        lbl_1_bss_4D4[player].position =
            lbl_1_bss_4D4[player].awardPosition;
        separation.x = 0.0f;
        separation.y = 0.0f;
        separation.z = 1.0f;
        rotatedAxis.x = 1.0f;
        rotatedAxis.y = 0.0f;
        rotatedAxis.z = 0.0f;
        shortXDirection = rotatedAxis;
        shortXDirectionPointer = &shortXDirection;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            shortXDirectionPointer, &rotatedAxis);
        firstDot = PSVECDotProduct(&separation, &rotatedAxis);
        rotatedAxis.x = rotatedAxis.y = 0.0f;
        rotatedAxis.z = 1.0f;
        shortZDirection = rotatedAxis;
        shortZDirectionPointer = &shortZDirection;
        fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
            shortZDirectionPointer, &rotatedAxis);
        angle = PSVECDotProduct(&separation, &rotatedAxis);
        if (angle >= 0.0f) {
            if (firstDot > 0.0f) {
                lbl_1_bss_4D4[player].rotation.y += 20.0f;
            } else if (firstDot < 0.0f) {
                lbl_1_bss_4D4[player].rotation.y -= 20.0f;
            }
            rotatedAxis.x = rotatedAxis.y = 0.0f;
            rotatedAxis.z = 1.0f;
            shortAdjustedZDirection = rotatedAxis;
            shortAdjustedZDirectionPointer = &shortAdjustedZDirection;
            fn_1_1B0AC(0.0f, lbl_1_bss_4D4[player].rotation.y, 0.0f,
                shortAdjustedZDirectionPointer, &rotatedAxis);
            shortAngleDirection = rotatedAxis;
            shortAngleDirectionPointer = &shortAngleDirection;
            shortAngleSeparation = separation;
            shortAngleSeparationPointer = &shortAngleSeparation;
            angle = (f32)fn_1_1B16C(shortAngleSeparationPointer,
                shortAngleDirectionPointer);
            if (-20.0f < angle && angle < 20.0f) {
                lbl_1_bss_4D4[player].rotation.y = 0.0f;
                keepRunning = 1;
            }
        } else if (firstDot >= 0.0f) {
            lbl_1_bss_4D4[player].rotation.y += 20.0f;
        } else {
            lbl_1_bss_4D4[player].rotation.y -= 20.0f;
        }
    }

    Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].rotation);
    return keepRunning;
}

/* Updates nonwinning characters' fall animation and reports when all have landed. */
s16 fn_1_107F0(void) {
    Point3d viewPosition;
    s16 finaleMotion;
    s16 finishMotion;
    s16 character;
    s16 player;

    player = 0;
    while (player < 4) {
        if (lbl_1_data_628[player] == 0) {
            if (MgModeScore[player] >= MgGameNo) {
                character = lbl_1_bss_4D4[player].charNo;
            } else {
                character = -1;
            }
            if (character >= 0) {
                lbl_1_data_628[player] = 1;
            } else {
                switch (lbl_1_bss_4D4[player].mode) {
                case 5:
                    if (lbl_1_bss_4D4[player].currentMotion != 12) {
            fn_1_AB04(player, 12);
        }
                    lbl_1_bss_4D4[player].mode = 6;
                    lbl_1_bss_4D4[player].state = 0;
                    if (lbl_1_bss_4D4[player].playerType == 0) {
                        omVibrate(player, 50, 20, 0);
                    }
                    break;
                case 6:
                    lbl_1_bss_4D4[player].state++;
                    fn_1_3B78(&viewPosition);
                    viewPosition.x *= 0.5f;
                    if (lbl_1_bss_4D4[player].position.x < viewPosition.x) {
                        lbl_1_bss_4D4[player].position.x += 2.0f;
                    } else if (lbl_1_bss_4D4[player].position.x >
                        viewPosition.x) {
                        lbl_1_bss_4D4[player].position.x -= 2.0f;
                    }
                    lbl_1_bss_4D4[player].position.y -= 10.0f;
                    if (lbl_1_bss_4D4[player].state >= 90) {
                        lbl_1_bss_4D4[player].mode = 7;
                        lbl_1_bss_4D4[player].state = 0;
                    }
                    break;
                case 7:
                    fn_1_AB04(player, 1);
                    lbl_1_bss_4D4[player].mode = 0;
                    lbl_1_bss_4D4[player].state = 0;
                    lbl_1_data_628[player] = 1;
                    break;
                default:
                    lbl_1_bss_4D4[player].mode = 5;
                    break;
                }
                Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
                    &lbl_1_bss_4D4[player].position);
                Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
                    &lbl_1_bss_4D4[player].rotation);
            }
        }
        player++;
    }

    if (lbl_1_data_628[0] != 0 && lbl_1_data_628[1] != 0 &&
        lbl_1_data_628[2] != 0 && lbl_1_data_628[3] != 0) {
        return 1;
    }
    return 0;
}

/* Advances a losing character's finish animation while the results sequence waits. */
s16 fn_1_11020(s16 player) {
    Point3d modelVector;
    f32 motionEndTime;
    s16 closeMotion;
    s16 finishMotion;
    s16 keepRunning = 0;

    switch (lbl_1_bss_4D4[player].mode) {
    case 11:
        Hu3DModelRotGet(lbl_1_bss_4D4[player].model, &modelVector);
        lbl_1_bss_4D4[player].rotation = modelVector;
        Hu3DModelPosGet(lbl_1_bss_4D4[player].model, &modelVector);
        lbl_1_bss_4D4[player].position = modelVector;
        fn_1_AB04(player, 6);
        lbl_1_bss_4D4[player].state = 0;
        lbl_1_bss_4D4[player].mode = 12;
        keepRunning = 1;
        break;
    case 12:
        if (CharMotionShiftIDGet(lbl_1_bss_4D4[player].charNo) < 0) {
            Hu3DModelAttrSet(lbl_1_bss_4D4[player].model, HU3D_MOTATTR_PAUSE);
            lbl_1_bss_4D4[player].state++;
            lbl_1_bss_4D4[player].position.y -=
                (f32)lbl_1_bss_4D4[player].state;
            if (lbl_1_bss_4D4[player].position.y <= 0.0f) {
                lbl_1_bss_4D4[player].position.y = 0.0f;
                Hu3DModelAttrReset(lbl_1_bss_4D4[player].model,
                    HU3D_MOTATTR_PAUSE);
                lbl_1_bss_4D4[player].mode = 13;
            }
        }
        keepRunning = 1;
        break;
    case 13:
        keepRunning = 1;
        if (lbl_1_bss_4D4[player].currentMotion != 1) {
            if (CharMotionShiftIDGet(lbl_1_bss_4D4[player].charNo) < 0) {
                motionEndTime = CharMotionMaxTimeGet(
                    lbl_1_bss_4D4[player].charNo);
                if (Hu3DData[lbl_1_bss_4D4[player].model].motWork.speed +
                    CharMotionTimeGet(lbl_1_bss_4D4[player].charNo) >=
                        motionEndTime) {
                    fn_1_AB04(player, 1);
                    lbl_1_bss_4D4[player].state = 0;
                }
            }
        } else if (CharMotionShiftIDGet(
                lbl_1_bss_4D4[player].charNo) < 0) {
            lbl_1_bss_4D4[player].mode = 0;
            lbl_1_bss_4D4[player].state = 0;
            keepRunning = 0;
        }
        break;
    default:
        lbl_1_bss_4D4[player].mode = 0;
        keepRunning = 0;
        break;
    }
    Hu3DModelPosSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[player].model,
        &lbl_1_bss_4D4[player].rotation);
    return keepRunning;
}

/* Advances one award jump from controller input or a timed computer-player choice. */
s16 fn_1_11898(s16 playerIndex) {
    Vec modelPosition;
    f32 baseHeight;
    u16 buttons;
    s16 result = 0;
    s16 motionIndex;
    s16 nextMotion;
    f32 maxMotionTime;

    if (lbl_1_bss_4D4[playerIndex].playerType != 0) {
        if (lbl_1_data_630[playerIndex] == 0) {
            lbl_1_data_630[playerIndex] = frandmod(60) + 25;
        }
        lbl_1_data_630[playerIndex] -= 1;
        if (lbl_1_data_630[playerIndex] < 0) {
            lbl_1_data_630[playerIndex] = -1;
        }
        if (lbl_1_data_630[playerIndex] == 0) {
            buttons = PAD_BUTTON_A;
            lbl_1_data_630[playerIndex] = -1;
        }
    } else {
        buttons = HuPadBtnDown[lbl_1_bss_4D4[playerIndex].padNo];
    }

    switch (lbl_1_bss_4D4[playerIndex].mode) {
    case 2:
        if (lbl_1_bss_4D4[playerIndex].currentMotion != 4) {
            fn_1_AB04(playerIndex, 4);
        }
        lbl_1_bss_4D4[playerIndex].mode = 3;
        lbl_1_bss_4D4[playerIndex].state = 0;
        lbl_1_bss_4D4[playerIndex].verticalVelocity = 20.0f;
        result = -1;
        break;
    case 3:
        if (lbl_1_bss_4D4[playerIndex].currentMotion == 4 &&
            CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
            maxMotionTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
            if (Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                    CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo) >= maxMotionTime) {
                CharModelAttrSet(lbl_1_bss_4D4[playerIndex].charNo, HU3D_MOTATTR_PAUSE);
            }
        }
        lbl_1_bss_4D4[playerIndex].verticalVelocity -= 0.5f;
        lbl_1_bss_4D4[playerIndex].position.y += lbl_1_bss_4D4[playerIndex].verticalVelocity;
        if (lbl_1_bss_4D4[playerIndex].position.y <= 0.0f) {
            lbl_1_bss_4D4[playerIndex].mode = 4;
            lbl_1_bss_4D4[playerIndex].state = 0;
            lbl_1_bss_4D4[playerIndex].position.y = 0.0f;
            lbl_1_bss_4D4[playerIndex].verticalVelocity = 0.0f;
        }
        fn_1_72E4(playerIndex, &modelPosition, &baseHeight);
        baseHeight = modelPosition.y - baseHeight;
        modelPosition.y = lbl_1_bss_4D4[playerIndex].position.y +
            lbl_1_bss_4D4[playerIndex].height;
        if (modelPosition.y >= baseHeight &&
            lbl_1_data_638[playerIndex] == 0) {
            lbl_1_bss_4D4[playerIndex].verticalVelocity = 0.0f;
            lbl_1_data_638[playerIndex] = 1;
            result = 1;
        } else {
            result = -1;
        }
        break;
    case 4:
        result = -1;
        if (lbl_1_bss_4D4[playerIndex].currentMotion == 4) {
            fn_1_AB04(playerIndex, 5);
        } else if (lbl_1_bss_4D4[playerIndex].currentMotion == 5) {
            if (CharMotionShiftIDGet(lbl_1_bss_4D4[playerIndex].charNo) < 0) {
                maxMotionTime = CharMotionMaxTimeGet(lbl_1_bss_4D4[playerIndex].charNo);
                if (Hu3DData[lbl_1_bss_4D4[playerIndex].model].motWork.speed +
                        CharMotionTimeGet(lbl_1_bss_4D4[playerIndex].charNo) >= maxMotionTime) {
                    fn_1_AB04(playerIndex, 1);
                }
            }
        } else {
            result = 0;
        }
        break;
    default:
    case 0:
        if ((buttons & PAD_BUTTON_A) != 0) {
            lbl_1_bss_4D4[playerIndex].mode = 2;
        }
        result = -1;
        break;
    }

    Hu3DModelPosSetV(lbl_1_bss_4D4[playerIndex].model, &lbl_1_bss_4D4[playerIndex].position);
    Hu3DModelRotSetV(lbl_1_bss_4D4[playerIndex].model, &lbl_1_bss_4D4[playerIndex].rotation);
    return result;
}

/* Clears every player's mode state before the next battle sequence. */
void fn_1_124FC(void) {
    s16 player;

    player = 0;
    while (player < 4) {
        lbl_1_bss_4D4[player].mode = 0;
        player += 1;
    }
}

/* Returns the current local position of a member sprite in its group. */
void fn_1_12544(s16 groupId, s16 memberIndex, f32 *positionX, f32 *positionY) {
    HUSPRITE *sprite;

    sprite = &HuSprData[HuSprGrpData[groupId].sprId[memberIndex]];
    *positionX = sprite->pos.x;
    *positionY = sprite->pos.y;
}

s16 lbl_1_data_628[4] = { 0, 0, 0, 0 };

s16 lbl_1_data_630[4] = { 0, 0, 0, 0 };

s16 lbl_1_data_638[4] = { 0, 0, 0, 0 };

s32 lbl_1_data_640[16] = {
    DATANUM(DATA_mgmbattle, 14), DATANUM(DATA_mgmbattle, 15),
    DATANUM(DATA_mgmbattle, 16), DATANUM(DATA_mgmbattle, 17),
    DATANUM(DATA_mgmbattle, 18), DATANUM(DATA_mgmbattle, 19),
    DATANUM(DATA_mgmbattle, 20), DATANUM(DATA_mgmbattle, 21),
    DATANUM(DATA_mgmbattle, 22), DATANUM(DATA_mgmbattle, 23),
    DATANUM(DATA_mgmbattle, 24), DATANUM(DATA_mgmbattle, 25),
    DATANUM(DATA_mgmbattle, 26), DATANUM(DATA_mgmbattle, 29),
    DATANUM(DATA_mgmbattle, 32), DATANUM(DATA_mgmbattle, 35)
};

s32 lbl_1_data_680[3][3] = {
    { DATANUM(DATA_mgmbattle, 26), DATANUM(DATA_mgmbattle, 27), DATANUM(DATA_mgmbattle, 28) },
    { DATANUM(DATA_mgmbattle, 29), DATANUM(DATA_mgmbattle, 30), DATANUM(DATA_mgmbattle, 31) },
    { DATANUM(DATA_mgmbattle, 32), DATANUM(DATA_mgmbattle, 33), DATANUM(DATA_mgmbattle, 34) }
};

s32 lbl_1_data_6A4 = DATANUM(DATA_mgmbattle, 37);

Point3d lbl_1_data_6A8 = { 288.0f, 122.0f, 0.0f };

Point3d lbl_1_data_6B4 = { 288.0f, 122.0f, 0.0f };

Point3d lbl_1_data_6C0 = { 0.0f, -105.0f, 0.0f };

Point3d lbl_1_data_6CC = { 0.0f, 0.0f, 0.0f };

Point3d lbl_1_data_6D8 = { 288.0f, 165.0f, 0.0f };

Point3d lbl_1_data_6E4[3][4] = {
    { { 200.0f, 90.0f, 0.0f }, { 200.0f, 125.0f, 0.0f }, { 200.0f, 160.0f, 0.0f }, { 200.0f, 195.0f, 0.0f } },
    { { 165.0f, 90.0f, 0.0f }, { 165.0f, 125.0f, 0.0f }, { 165.0f, 160.0f, 0.0f }, { 165.0f, 195.0f, 0.0f } },
    { { 130.0f, 90.0f, 0.0f }, { 130.0f, 125.0f, 0.0f }, { 130.0f, 160.0f, 0.0f }, { 130.0f, 195.0f, 0.0f } }
};

Point3d lbl_1_data_774[2][4] = {
    { { -200.0f, 325.0f, 1000.0f }, { -10.0f, 325.0f, 1000.0f }, { 90.0f, 325.0f, 1000.0f }, { 190.0f, 325.0f, 1000.0f } },
    { { -200.0f, 325.0f, 1000.0f }, { -90.0f, 325.0f, 1000.0f }, { 90.0f, 325.0f, 1000.0f }, { 200.0f, 325.0f, 1000.0f } }
};

Point3d lbl_1_data_7D4[2][4] = {
    { { -197.0f, 400.0f, 1010.0f }, { -10.0f, 400.0f, 1010.0f }, { 90.0f, 400.0f, 1010.0f }, { 190.0f, 400.0f, 1010.0f } },
    { { -197.0f, 400.0f, 1010.0f }, { -90.0f, 400.0f, 1010.0f }, { 90.0f, 400.0f, 1010.0f }, { 197.0f, 400.0f, 1010.0f } }
};

Point3d lbl_1_data_834[2] = { { 210.0f, 257.0f, 0.0f }, { 287.0f, 255.0f, 0.0f } };

Point3d lbl_1_data_84C[2][4] = {
    { { 137.0f, 205.0f, 0.0f }, { 280.0f, 205.0f, 0.0f }, { 357.0f, 205.0f, 0.0f }, { 433.0f, 205.0f, 0.0f } },
    { { 137.0f, 205.0f, 0.0f }, { 218.0f, 205.0f, 0.0f }, { 357.0f, 205.0f, 0.0f }, { 438.0f, 205.0f, 0.0f } }
};

/* Loads and arranges character, score, and award sprites before the results screen. */
void fn_1_125A0(void) {
    f32 baseX;
    Point3d positionOffset;
    f32 spriteX;
    f32 spriteY;
    s16 spriteIndex;
    s16 player;
    ANIMDATA *animationData;

    if (MgGameNo == 5) {
        lbl_1_data_640[12] = lbl_1_data_680[0][1];
        lbl_1_data_640[13] = lbl_1_data_680[1][1];
        lbl_1_data_640[14] = lbl_1_data_680[2][1];
    }
    if (MgGameNo == 7) {
        lbl_1_data_640[12] = lbl_1_data_680[0][2];
        lbl_1_data_640[13] = lbl_1_data_680[1][2];
        lbl_1_data_640[14] = lbl_1_data_680[2][2];
    }
    player = 0;
    while (player < 16) {
        lbl_1_bss_494[player] =
            HuSprAnimRead(HuDataSelHeapReadNum(lbl_1_data_640[player], HU_MEMNUM_OVL, HEAP_MODEL));
        player += 1;
    }
    lbl_1_bss_492 = HuSprGrpCreate(6);
    lbl_1_bss_486[0] = HuSprCreate(lbl_1_bss_494[5], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 0, lbl_1_bss_486[0]);
    HuSprPosSet(lbl_1_bss_492, 0, 0.0f, 0.0f);
    lbl_1_bss_486[1] = HuSprCreate(lbl_1_bss_494[6], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 1, lbl_1_bss_486[1]);
    HuSprPosSet(lbl_1_bss_492, 1, 0.0f, 0.0f);
    lbl_1_bss_486[2] = HuSprCreate(lbl_1_bss_494[7], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 2, lbl_1_bss_486[2]);
    HuSprPosSet(lbl_1_bss_492, 2, 0.0f, 0.0f);
    lbl_1_bss_486[3] = HuSprCreate(lbl_1_bss_494[0], 1, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 3, lbl_1_bss_486[3]);
    HuSprPosSet(lbl_1_bss_492, 3, 0.0f, 0.0f);
    lbl_1_bss_486[4] = HuSprCreate(lbl_1_bss_494[3], 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 4, lbl_1_bss_486[4]);
    HuSprPosSet(lbl_1_bss_492, 4, -116.0f, 0.0f);
    lbl_1_bss_486[5] = HuSprCreate(lbl_1_bss_494[4], 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_492, 5, lbl_1_bss_486[5]);
    HuSprPosSet(lbl_1_bss_492, 5, 116.0f, 0.0f);
    HuSprGrpPosSet(lbl_1_bss_492, lbl_1_data_6A8.x, lbl_1_data_6A8.y);
    HuSprGrpScaleSet(lbl_1_bss_492, 0.0f, 0.0f);
    lbl_1_bss_484 = HuSprGrpCreate(6);
    lbl_1_bss_478[0] = HuSprCreate(lbl_1_bss_494[8], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 0, lbl_1_bss_478[0]);
    HuSprPosSet(lbl_1_bss_484, 0, 0.0f, 0.0f);
    lbl_1_bss_478[1] = HuSprCreate(lbl_1_bss_494[9], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 1, lbl_1_bss_478[1]);
    HuSprPosSet(lbl_1_bss_484, 1, 0.0f, 0.0f);
    lbl_1_bss_478[2] = HuSprCreate(lbl_1_bss_494[10], 2, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 2, lbl_1_bss_478[2]);
    HuSprPosSet(lbl_1_bss_484, 2, 0.0f, 0.0f);
    lbl_1_bss_478[3] = HuSprCreate(lbl_1_bss_494[0], 1, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 3, lbl_1_bss_478[3]);
    HuSprPosSet(lbl_1_bss_484, 3, 0.0f, 0.0f);
    lbl_1_bss_478[4] = HuSprCreate(lbl_1_bss_494[3], 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 4, lbl_1_bss_478[4]);
    HuSprPosSet(lbl_1_bss_484, 4, -116.0f, 0.0f);
    lbl_1_bss_478[5] = HuSprCreate(lbl_1_bss_494[4], 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_484, 5, lbl_1_bss_478[5]);
    HuSprPosSet(lbl_1_bss_484, 5, 116.0f, 0.0f);
    HuSprGrpPosSet(lbl_1_bss_484, lbl_1_data_6B4.x, lbl_1_data_6B4.y);
    HuSprGrpScaleSet(lbl_1_bss_484, 0.0f, 0.0f);
    lbl_1_bss_476 = HuSprGrpCreate(3);
    lbl_1_bss_470[0] = HuSprCreate(lbl_1_bss_494[2], 5000, 0);
    HuSprGrpMemberSet(lbl_1_bss_476, 0, lbl_1_bss_470[0]);
    HuSprPosSet(lbl_1_bss_476, 0, 0.0f, 0.0f);
    lbl_1_bss_470[1] = HuSprCreate(lbl_1_bss_494[3], 4500, 0);
    HuSprGrpMemberSet(lbl_1_bss_476, 1, lbl_1_bss_470[1]);
    HuSprPosSet(lbl_1_bss_476, 1, -218.0f, 0.0f);
    lbl_1_bss_470[2] = HuSprCreate(lbl_1_bss_494[4], 4500, 0);
    HuSprGrpMemberSet(lbl_1_bss_476, 2, lbl_1_bss_470[2]);
    HuSprPosSet(lbl_1_bss_476, 2, 218.0f, 0.0f);
    HuSprGrpPosSet(lbl_1_bss_476, 288.0f, 240.0f);
    HuSprGrpScaleSet(lbl_1_bss_476, 0.0f, 0.0f);
    lbl_1_bss_43A = HuSprGrpCreate(4);
    player = 0;
    while (player < 4) {
        lbl_1_bss_468[player] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 36), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_468[player], 1U);
        Hu3DModelLayerSet(lbl_1_bss_468[player], 5);
        animationData = HuSprAnimRead(HuDataSelHeapReadNum(
            lbl_1_data_6A4 + lbl_1_bss_4D4[player].charNo, HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_448[player][0] = Hu3DAnimCreate(animationData, lbl_1_bss_468[player], "ys77120");
        lbl_1_bss_448[player][1] = Hu3DAnimCreate(animationData, lbl_1_bss_468[player], "ys77121");
        lbl_1_bss_448[player][2] = Hu3DAnimCreate(animationData, lbl_1_bss_468[player], "ys77060");
        lbl_1_bss_448[player][3] = Hu3DAnimCreate(animationData, lbl_1_bss_468[player], "ys77061");
        Hu3DAnimBankSet(lbl_1_bss_448[player][0], 0U);
        Hu3DAnimBankSet(lbl_1_bss_448[player][1], 0U);
        Hu3DAnimBankSet(lbl_1_bss_448[player][2], 0U);
        Hu3DAnimBankSet(lbl_1_bss_448[player][3], 0U);
        Hu3DModelAmbSet(lbl_1_bss_468[player], 2.0f, 2.0f, 2.0f);
        Hu3DModelAttrSet(lbl_1_bss_468[player], 1U);
        Hu3DModelPosSetV(lbl_1_bss_468[player], &lbl_1_data_774[0][player]);
        lbl_1_bss_43C[player] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 49), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_43C[player], 1U);
        Hu3DModelLayerSet(lbl_1_bss_43C[player], 5);
        Hu3DModelAmbSet(lbl_1_bss_43C[player], 2.0f, 2.0f, 2.0f);
        Hu3DModelAttrSet(lbl_1_bss_43C[player], 1U);
        Hu3DModelPosSetV(lbl_1_bss_43C[player], &lbl_1_data_7D4[0][player]);
        Hu3DAnimCreate(HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 50),
                                                          HU_MEMNUM_OVL, HEAP_MODEL)),
                       lbl_1_bss_43C[player], "icon");
        animationData = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 51), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_432[player] = HuSprCreate(animationData, 2, player);
        HuSprGrpMemberSet(lbl_1_bss_43A, player, lbl_1_bss_432[player]);
        HuSprPosSet(lbl_1_bss_43A, player, lbl_1_data_84C[0][player].x, 0.0f);
        player += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_43A, 0.0f, lbl_1_data_84C[0][0].y);
    HuSprGrpScaleSet(lbl_1_bss_43A, 0.0f, 0.0f);
    lbl_1_bss_446 = HuSprGrpCreate(1);
    animationData =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_444 = HuSprCreate(animationData, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_446, 0, lbl_1_bss_444);
    HuSprPosSet(lbl_1_bss_446, 0, lbl_1_data_834[0].x, 0.0f);
    HuSprScaleSet(lbl_1_bss_446, 0, 0.5f, 0.5f);
    HuSprGrpPosSet(lbl_1_bss_446, 0.0f, lbl_1_data_834[0].y);
    HuSprGrpScaleSet(lbl_1_bss_446, 0.0f, 0.0f);
    HuSprExecLayerSet(64, 4);
    HuSprGrpDrawNoSet(lbl_1_bss_476, 64);
    lbl_1_bss_42A[0] = HuSprGrpCreate(32);
    lbl_1_bss_3EA[0] = HuSprCreate(lbl_1_bss_494[11], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_42A[0], 0, lbl_1_bss_3EA[0]);
    HuSprPosSet(lbl_1_bss_42A[0], 0, lbl_1_data_6C0.x, lbl_1_data_6C0.y);
    lbl_1_bss_3EA[1] = HuSprCreate(lbl_1_bss_494[12], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_42A[0], 1, lbl_1_bss_3EA[1]);
    HuSprPosSet(lbl_1_bss_42A[0], 1, lbl_1_data_6CC.x, lbl_1_data_6CC.y);
    lbl_1_bss_3EA[2] = HuSprCreate(lbl_1_bss_494[13], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_42A[0], 2, lbl_1_bss_3EA[2]);
    HuSprPosSet(lbl_1_bss_42A[0], 2, lbl_1_data_6CC.x, lbl_1_data_6CC.y);
    lbl_1_bss_3EA[3] = HuSprCreate(lbl_1_bss_494[14], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_42A[0], 3, lbl_1_bss_3EA[3]);
    HuSprPosSet(lbl_1_bss_42A[0], 3, lbl_1_data_6CC.x, lbl_1_data_6CC.y);
    if (MgGameNo == 3) {
        baseX = 230.0f;
        positionOffset.x = 180.0f;
    } else if (MgGameNo == 5) {
        baseX = 230.0f;
        positionOffset.x = 216.0f;
    } else {
        baseX = 230.0f;
        positionOffset.x = 250.0f;
    }
    positionOffset.y = 85.0f;
    player = 0;
    while (player < 4) {
        spriteIndex = 0;
        while (spriteIndex < 7) {
            lbl_1_bss_3EA[(player * 7) + 4 + spriteIndex] =
                HuSprCreate(lbl_1_bss_494[15], 50, 0);
            HuSprGrpMemberSet(lbl_1_bss_42A[0], (s16) ((player * 7) + 4 + spriteIndex),
                              lbl_1_bss_3EA[(player * 7) + 4 + spriteIndex]);
            spriteX = (baseX + (f32) ((spriteIndex * 35) + 17)) - positionOffset.x;
            spriteY = (f32) ((player * 33) + 32) - positionOffset.y;
            HuSprPosSet(lbl_1_bss_42A[0], (s16) ((player * 7) + 4 + spriteIndex), spriteX, spriteY);
            spriteIndex += 1;
        }
        player += 1;
    }
    HuSprGrpPosSet(lbl_1_bss_42A[0], lbl_1_data_6D8.x, lbl_1_data_6D8.y);
    HuSprGrpScaleSet(lbl_1_bss_42A[0], 0.0f, 0.0f);
}

/* Slides the view center back to the origin while the battle menu enters. */
void fn_1_139BC(void) {
    f32 step = (0.0f);
    s16 moving;

    moving = 1;
    step = (12.0f);
    while (moving) {
        Center.x -= step;
        step -= (0.09f);
        if (step < (0.09f)) {
            step = (0.09f);
        }
        if (Center.x <= (0.0f)) {
            Center.x = (0.0f);
            moving = 0;
        }
        HuPrcVSleep();
    }
}

/* Runs the player selection for the number of minigames in a battle set. */
void fn_1_13AAC(void) {
    s16 active;
    s16 selectedPlayer;
    s16 currentSelection;
    s16 playerIndex;
    u16 stick;
    u16 buttons;
    f32 scale;
    f32 inverseScale;
    f32 pulseScale;

    playerIndex = 0;
    selectedPlayer = -1;
    active = 1;
    fn_1_1408(2);
    fn_1_1648(2);
    fn_1_1528(2);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_4D4[playerIndex].playerType == 0) {
            selectedPlayer = playerIndex;
            break;
        }
    }

    HuSprGrpScaleSet(lbl_1_bss_492, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 0, 1.0f, 1.0f);
    HuSprPosSet(lbl_1_bss_492, 0, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 1, 0.0f, 0.0f);
    HuSprPosSet(lbl_1_bss_492, 1, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 2, 0.0f, 0.0f);
    HuSprPosSet(lbl_1_bss_492, 2, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 4, 1.0f, 1.0f);
    HuSprScaleSet(lbl_1_bss_492, 5, 1.0f, 1.0f);
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);

    scale = 0.0f;
    while (scale < 1.0f) {
        scale += 0.1f;
        if (scale >= 1.0f) {
            scale = 1.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_492, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_492, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_492, lbl_1_data_6A8.x, lbl_1_data_6A8.y);
    fn_1_1408(3);
    fn_1_1648(3);
    currentSelection = 3;
    MgGameNo = 3;

    while (active != 0) {
        stick = 0;
        buttons = 0;
        if (selectedPlayer >= 0) {
            stick = HuPadDStkRep[lbl_1_bss_4D4[selectedPlayer].padNo];
            buttons = HuPadBtnDown[lbl_1_bss_4D4[selectedPlayer].padNo];
        }

        if (MgGameNo == 3) {
            if (currentSelection != 3) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_492, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_492, 0, 1.0f, 1.0f);
                    if (currentSelection == 5) {
                        HuSprScaleSet(lbl_1_bss_492, 1, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 7) {
                        HuSprScaleSet(lbl_1_bss_492, 2, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(3);
            }
            currentSelection = MgGameNo;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgGameNo = -1;
            } else if ((stick & 1) != 0) {
                MgGameNo = 7;
            } else if ((stick & 2) != 0) {
                MgGameNo = 5;
            }
        } else if (MgGameNo == 5) {
            if (currentSelection != 5) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_492, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_492, 1, 1.0f, 1.0f);
                    if (currentSelection == 7) {
                        HuSprScaleSet(lbl_1_bss_492, 2, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 3) {
                        HuSprScaleSet(lbl_1_bss_492, 0, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(4);
            }
            currentSelection = MgGameNo;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgGameNo = -1;
            } else if ((stick & 1) != 0) {
                MgGameNo = 3;
            } else if ((stick & 2) != 0) {
                MgGameNo = 7;
            }
        } else if (MgGameNo == 7) {
            if (currentSelection != 7) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_492, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_492, 2, 1.0f, 1.0f);
                    if (currentSelection == 3) {
                        HuSprScaleSet(lbl_1_bss_492, 0, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 5) {
                        HuSprScaleSet(lbl_1_bss_492, 1, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_492, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(5);
            }
            currentSelection = MgGameNo;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgGameNo = -1;
            } else if ((stick & 1) != 0) {
                MgGameNo = 5;
            } else if ((stick & 2) != 0) {
                MgGameNo = 3;
            }
        }
        HuPrcVSleep();
    }

    if (MgGameNo > 0) {
        HuAudFXPlay(MGBATTLE_SE_SELECTION_ACCEPT);
    } else {
        HuAudFXPlay(MSM_SE_CMN_04);
    }
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);
    scale = 1.0f;
    while (scale > 0.0f) {
        scale -= 0.1f;
        if (scale <= 0.0f) {
            scale = 0.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_492, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_492, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 0, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 1, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_492, 2, 0.0f, 0.0f);
    fn_1_1528(3);
    HuPrcVSleep();
}

/* Handles player input for the battle submode selection screen. */
void fn_1_148BC(void) {
    s16 selectedPlayer;
    s16 active;
    s16 playerIndex;
    s16 currentSelection;
    u16 stick;
    u16 buttons;
    f32 scale;
    f32 inverseScale;
    f32 pulseScale;

    playerIndex = 0;
    selectedPlayer = -1;
    active = 1;
    fn_1_1408(7);
    fn_1_1648(7);
    fn_1_1528(7);
    playerIndex = 0;
    for (; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_4D4[playerIndex].playerType == 0) {
            selectedPlayer = playerIndex;
            break;
        }
    }

    HuSprGrpScaleSet(lbl_1_bss_484, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 0, 1.0f, 1.0f);
    HuSprPosSet(lbl_1_bss_484, 0, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 1, 0.0f, 0.0f);
    HuSprPosSet(lbl_1_bss_484, 1, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 2, 0.0f, 0.0f);
    HuSprPosSet(lbl_1_bss_484, 2, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 4, 1.0f, 1.0f);
    HuSprScaleSet(lbl_1_bss_484, 5, 1.0f, 1.0f);
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);

    scale = 0.0f;
    while (scale < 1.0f) {
        scale += 0.1f;
        if (scale >= 1.0f) {
            scale = 1.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_484, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_484, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_484, lbl_1_data_6B4.x, lbl_1_data_6B4.y);
    fn_1_1408(8);
    fn_1_1648(8);
    currentSelection = 0;
    MgModeSubMode = 0;

    while (active != 0) {
        stick = 0;
        buttons = 0;
        if (selectedPlayer >= 0) {
            stick = HuPadDStkRep[lbl_1_bss_4D4[selectedPlayer].padNo];
            buttons = HuPadBtnDown[lbl_1_bss_4D4[selectedPlayer].padNo];
        }

        if (MgModeSubMode == 0) {
            if (currentSelection != 0) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_484, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_484, 0, 1.0f, 1.0f);
                    if (currentSelection == 1) {
                        HuSprScaleSet(lbl_1_bss_484, 1, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 2) {
                        HuSprScaleSet(lbl_1_bss_484, 2, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(8);
            }
            currentSelection = MgModeSubMode;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgModeSubMode = -1;
            } else if ((stick & 1) != 0) {
                MgModeSubMode = 2;
            } else if ((stick & 2) != 0) {
                MgModeSubMode = 1;
            }
        } else if (MgModeSubMode == 1) {
            if (currentSelection != 1) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_484, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_484, 1, 1.0f, 1.0f);
                    if (currentSelection == 2) {
                        HuSprScaleSet(lbl_1_bss_484, 2, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 0) {
                        HuSprScaleSet(lbl_1_bss_484, 0, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(9);
            }
            currentSelection = MgModeSubMode;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgModeSubMode = -1;
            } else if ((stick & 1) != 0) {
                MgModeSubMode = 0;
            } else if ((stick & 2) != 0) {
                MgModeSubMode = 2;
            }
        } else if (MgModeSubMode == 2) {
            if (currentSelection != 2) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                HuSprGrpScaleSet(lbl_1_bss_484, 1.0f, 1.0f);
                scale = inverseScale = pulseScale = 0.0f;
                while (scale < 1.0f) {
                    scale += 0.1f;
                    if (scale >= 1.0f) {
                        scale = 1.0f;
                    }
                    inverseScale = 1.0f - scale;
                    pulseScale = (0.5f) * ((128.0f) - (128.0f) * scale);
                    HuSprScaleSet(lbl_1_bss_484, 2, 1.0f, 1.0f);
                    if (currentSelection == 0) {
                        HuSprScaleSet(lbl_1_bss_484, 0, 0.0f, 1.0f);
                        pulseScale *= (-1.0f);
                        pulseScale += (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 4, pulseScale, pulseScale);
                    } else if (currentSelection == 1) {
                        HuSprScaleSet(lbl_1_bss_484, 1, 0.0f, 1.0f);
                        pulseScale -= (64.0f);
                        pulseScale = 1.0f + (1.0f - scale) * (1.0f - scale);
                        HuSprScaleSet(lbl_1_bss_484, 5, pulseScale, pulseScale);
                    }
                    HuPrcVSleep();
                }
                fn_1_1648(10);
            }
            currentSelection = MgModeSubMode;
            if ((buttons & PAD_BUTTON_A) != 0) {
                active = 0;
            } else if ((buttons & PAD_BUTTON_B) != 0) {
                active = 0;
                MgModeSubMode = -1;
            } else if ((stick & 1) != 0) {
                MgModeSubMode = 1;
            } else if ((stick & 2) != 0) {
                MgModeSubMode = 0;
            }
        }
        HuPrcVSleep();
    }

    if (MgModeSubMode >= 0) {
        HuAudFXPlay(MGBATTLE_SE_SELECTION_ACCEPT);
    } else {
        HuAudFXPlay(MSM_SE_CMN_04);
    }
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);
    scale = 1.0f;
    while (scale > 0.0f) {
        scale -= 0.1f;
        if (scale <= 0.0f) {
            scale = 0.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_484, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_484, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 0, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 1, 0.0f, 0.0f);
    HuSprScaleSet(lbl_1_bss_484, 2, 0.0f, 0.0f);
    fn_1_1528(8);
    HuPrcVSleep();
}

/* Lets human players choose team order and confirms the resulting lineup. */
s16 fn_1_156CC(void) {
    Mtx matrix;
    f32 scale;
    f32 angle;
    s16 unusedLocalA;
    s16 i;
    s16 j;

    s16 humanPlayerIndex;
    s16 resultCode;
    s16 layoutIndex;

    s16 stateOrDestination;
    u16 buttonDown;
    u16 repeatedDirection;

    resultCode = 0;
    layoutIndex = 0;
    humanPlayerIndex = -1;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_4D4[i].grpNo = i;
        MgBattleOrder[i] = (u8) i;
    }
    if (MgModeSubMode != 1 && MgModeSubMode != 2) {
        return resultCode;
    }
    if (MgModeSubMode == 1) {
        layoutIndex = 0;
    }
    if (MgModeSubMode == 2) {
        layoutIndex = 1;
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_4D4[i].playerType == 0) {
            humanPlayerIndex = i;
            break;
        } else {
        }
    }
    fn_1_1408(22);
    fn_1_1648(22);
    fn_1_1528(22);
    HuPrcVSleep();
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);
    for (scale = 0.0f; scale < 1.0f;) {
        scale += 0.1f;
        if (scale >= 1.0f) {
            scale = 1.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_476, scale, scale);
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_CMN_53);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_468[i], 1U);
        Hu3DModelPosSetV(lbl_1_bss_468[i], &lbl_1_data_774[layoutIndex][i]);
    }
    for (j = 0; j < 12; j++) {
        scale = 45.0f * (f32) j;
        angle = 0.0833f * (f32) j;
        PSMTXRotRad((f32 (*)[4]) matrix, 121, 0.017453292f * scale);
        for (i = 0; i < 4; i++) {
            Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
            Hu3DModelRotSet(lbl_1_bss_468[i], CRot.x, CRot.y, CRot.z);
            Hu3DModelScaleSet(lbl_1_bss_468[i], angle, angle, angle);
        }
        HuPrcVSleep();
    }
    PSMTXIdentity((f32 (*)[4]) matrix);
    for (i = 0; i < 4; i++) {
        Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
        Hu3DModelScaleSet(lbl_1_bss_468[i], 1.0f, 1.0f, 1.0f);
    }
    HuSprPosSet(lbl_1_bss_446, 0, lbl_1_data_834[layoutIndex].x, 0.0f);
    HuSprGrpPosSet(lbl_1_bss_446, 0.0f, lbl_1_data_834[layoutIndex].y);
    HuSprGrpScaleSet(lbl_1_bss_446, 1.0f, 1.0f);
    HuPrcVSleep();
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrReset(lbl_1_bss_43C[i], 1U);
        Hu3DModelPosSetV(lbl_1_bss_43C[i], &lbl_1_data_7D4[layoutIndex][i]);
        j = i;
        if (lbl_1_bss_4D4[i].playerType != 0) {
            j = 4;
        }
        HuSprBankSet(lbl_1_bss_43A, i, j);
        HuSprPosSet(lbl_1_bss_43A, i, lbl_1_data_84C[layoutIndex][i].x, 0.0f);
    }
    HuSprGrpPosSet(lbl_1_bss_43A, 0.0f, lbl_1_data_84C[layoutIndex][0].y);
    HuSprGrpScaleSet(lbl_1_bss_43A, 1.0f, 1.0f);
    HuPrcVSleep();
    fn_1_1408(102);
    fn_1_1648(102);
    HuPrcVSleep();
    stateOrDestination = 1;
    while (stateOrDestination != 0) {
        repeatedDirection = 0;
        buttonDown = 0;
        if (humanPlayerIndex >= 0) {
            repeatedDirection = HuPadDStkRep[lbl_1_bss_4D4[humanPlayerIndex].padNo];
            buttonDown = HuPadBtnDown[lbl_1_bss_4D4[humanPlayerIndex].padNo];
        }
        if (MgModeSubMode == 1) {
            j = 0;
            if ((repeatedDirection & 1) != 0) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                for (i = 0; i < 3; i++) {
                    MgBattleOrder[i + 1] = (u8) lbl_1_bss_4D4[i].grpNo;
                }
                MgBattleOrder[0] = (u8) lbl_1_bss_4D4[3].grpNo;
                j = 1;
            } else if ((repeatedDirection & 2) != 0) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                MgBattleOrder[3] = (u8) lbl_1_bss_4D4->grpNo;
                for (i = 1; i < 4; i++) {
                    MgBattleOrder[i - 1] = (u8) lbl_1_bss_4D4[i].grpNo;
                }
                j = 2;
            }
            if (j != 0) {
                angle = 2.0f;
                for (j = 0; j < 10; j++) {
                    scale = 20.0f * (f32) j;
                    PSMTXRotRad((f32 (*)[4]) matrix, 121, 0.017453292f * scale);
                    for (i = 0; i < 4; i++) {
                        Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
                        if (j == 6) {
                            Hu3DModelPosSetV(lbl_1_bss_468[i],
                                             &lbl_1_data_774[layoutIndex][MgBattleOrder[i]]);
                        }
                    }
                    angle -= 0.1f;
                    if ((repeatedDirection & 1) != 0) {
                        HuSprScaleSet(lbl_1_bss_476, 1, angle, angle);
                    } else if ((repeatedDirection & 2) != 0) {
                        HuSprScaleSet(lbl_1_bss_476, 2, angle, angle);
                    }
                    HuPrcVSleep();
                }
                HuSprGrpScaleSet(lbl_1_bss_43A, 1.0f, 1.0f);
                for (i = 0; i < 4; i++) {
                    j = i;
                    if (lbl_1_bss_4D4[i].playerType != 0) {
                        j = 4;
                    }
                    HuSprBankSet(lbl_1_bss_43A, (s16) MgBattleOrder[i], j);
                    PSMTXIdentity((f32 (*)[4]) matrix);
                    Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
                }
                HuPrcVSleep();
                for (i = 0; i < 4; i++) {
                    lbl_1_bss_4D4[i].grpNo = MgBattleOrder[i];
                }
            }
        } else if (MgModeSubMode == 2) {
            j = 0;
            if ((repeatedDirection & 1) != 0) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                for (i = 1; i < 3; i++) {
                    MgBattleOrder[i + 1] = (u8) lbl_1_bss_4D4[i].grpNo;
                }
                MgBattleOrder[1] = (u8) lbl_1_bss_4D4[3].grpNo;
                j = 1;
            } else if ((repeatedDirection & 2) != 0) {
                HuAudFXPlay(MGBATTLE_SE_SELECTION_MOVE);
                MgBattleOrder[3] = (u8) lbl_1_bss_4D4[1].grpNo;
                for (i = 2; i < 4; i++) {
                    MgBattleOrder[i - 1] = (u8) lbl_1_bss_4D4[i].grpNo;
                }
                j = 2;
            }
            if (j != 0) {
                angle = 2.0f;
                for (j = 0; j < 10; j++) {
                    scale = 20.0f * (f32) j;
                    PSMTXRotRad((f32 (*)[4]) matrix, 121, 0.017453292f * scale);
                    for (i = 1; i < 4; i++) {
                        Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
                        if (j == 6) {
                            Hu3DModelPosSetV(lbl_1_bss_468[i],
                                             &lbl_1_data_774[layoutIndex][MgBattleOrder[i]]);
                        }
                    }
                    angle -= 0.1f;
                    if ((repeatedDirection & 1) != 0) {
                        HuSprScaleSet(lbl_1_bss_476, 1, angle, angle);
                    } else if ((repeatedDirection & 2) != 0) {
                        HuSprScaleSet(lbl_1_bss_476, 2, angle, angle);
                    }
                    HuPrcVSleep();
                }
                for (i = 0; i < 4; i++) {
                    j = i;
                    if (lbl_1_bss_4D4[i].playerType != 0) {
                        j = 4;
                    }
                    HuSprBankSet(lbl_1_bss_43A, (s16) MgBattleOrder[i], j);
                    PSMTXIdentity((f32 (*)[4]) matrix);
                    Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
                }
                HuPrcVSleep();
                for (i = 0; i < 4; i++) {
                    lbl_1_bss_4D4[i].grpNo = MgBattleOrder[i];
                }
            }
        }
        if ((buttonDown & PAD_BUTTON_A) != 0) {
            stateOrDestination = 0;
            resultCode = 0;
            HuAudFXPlay(MGBATTLE_SE_SELECTION_ACCEPT);
        } else if ((buttonDown & PAD_BUTTON_B) != 0) {
            stateOrDestination = 0;
            resultCode = -1;
            HuAudFXPlay(MSM_SE_CMN_04);
        }
        HuPrcVSleep();
    }
    if (resultCode == 0) {
        if (MgModeSubMode == 1) {
            resultCode = 1;
        } else if (MgModeSubMode == 2) {
            resultCode = 2;
        }
        for (i = 0; i < 4; i++) {
            lbl_1_bss_4D4[i].position = lbl_1_data_428[resultCode][lbl_1_bss_4D4[i].grpNo];
            Hu3DModelPosSetV(lbl_1_bss_4D4[i].model,
                &lbl_1_bss_4D4[i].position);
            if (MgModeSubMode == 1) {
                if (lbl_1_bss_4D4[i].grpNo <= 0) {
                    lbl_1_bss_4D4[i].grpNo = 0;
                } else {
                    lbl_1_bss_4D4[i].grpNo = 1;
                }
            } else if (MgModeSubMode == 2) {
                if (lbl_1_bss_4D4[i].grpNo <= 1) {
                    lbl_1_bss_4D4[i].grpNo = 0;
                } else {
                    lbl_1_bss_4D4[i].grpNo = 1;
                }
            }
        }
        resultCode = 0;
        for (i = 0; i < 4; i++) {
            GwPlayerConf[i].grpNo = MgBattleOrder[i] = (u8) lbl_1_bss_4D4[i].grpNo;
        }
    }
    fn_1_1528(102);
    HuPrcVSleep();
    HuSprGrpScaleSet(lbl_1_bss_43A, 0.0f, 0.0f);
    HuSprGrpScaleSet(lbl_1_bss_446, 0.0f, 0.0f);
    for (i = 0; i < 4; i++) {
        Hu3DModelAttrSet(lbl_1_bss_43C[i], 1U);
    }
    HuPrcVSleep();
    HuAudFXPlay(MSM_SE_CMN_54);
    scale = 1.0f;
    angle = 0.0f;
    while (scale > 0.0f) {
        scale -= 0.1f;
        if (scale <= 0.0f) {
            scale = 0.0f;
        }
        angle += 45.0f;
        PSMTXRotRad((f32 (*)[4]) matrix, 121, 0.017453292f * angle);
        for (i = 0; i < 4; i++) {
            Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
            Hu3DModelScaleSet(lbl_1_bss_468[i], scale, scale, scale);
        }
        HuPrcVSleep();
    }
    PSMTXIdentity((f32 (*)[4]) matrix);
    for (i = 0; i < 4; i++) {
        Hu3DModelMtxSet(lbl_1_bss_468[i], (f32 (*)[3][4]) matrix);
        Hu3DModelScaleSet(lbl_1_bss_468[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(lbl_1_bss_468[i], 1U);
    }
    HuPrcVSleep();
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);
    for (scale = 1.0f; scale > 0.0f;) {
        scale -= 0.1f;
        if (scale <= 0.0f) {
            scale = 0.0f;
        }
        HuSprGrpScaleSet(lbl_1_bss_476, scale, scale);
        HuPrcVSleep();
    }
    HuSprGrpScaleSet(lbl_1_bss_476, 0.0f, 0.0f);
    HuPrcVSleep();
    return resultCode;
}

/* Builds a randomized list of unlocked minigames for the selected Battle Bridge mode. */
void fn_1_169FC(void) {
    s16 playerOrder[4];
    s16 gameType;
    s16 innerIndex;
    s16 modeCount;
    s16 outerIndex;
    s32 includeMode;
    Vec centerStart;
    Vec rotationStart;
    Vec target;
    f32 zoomStart;
    gameType = 0;
    if (MgModeSubMode == 1)
    {
        gameType = 1;
    }
    else
    {
        if (MgModeSubMode == 2)
        {
            gameType = 2;
        }
    }
    MgModePlayNum = 0;
    for (outerIndex = 0, modeCount = 0; MgDataTbl[outerIndex].ovl != MGBATTLE_NO_OVERLAY;
         outerIndex++) {
        includeMode = outerIndex + MGBATTLE_UNLOCK_GAME_BASE;
        if (GWMgUnlockGet(includeMode) != 0 && gameType == (s32) MgDataTbl[outerIndex].type)
        {
            includeMode = 1;
            if ((MgDataTbl[outerIndex].flag & (1 << 9)) != 0)
            {
                includeMode = 0;
            }
            if (outerIndex == 21 || outerIndex == 22 || outerIndex == 41)
            {
                includeMode = 0;
            }
            if (includeMode != 0)
            {
                MgModeWork[modeCount] = (u8) outerIndex;
                modeCount++;
            }
        }
        MgModeWork[modeCount] = MGBATTLE_MODE_LIST_END;
    }
    for (; modeCount < 256; ++modeCount)
    {
        MgModeWork[modeCount] = MGBATTLE_MODE_LIST_END;
    }
    for (outerIndex = 0; outerIndex < 256 && MgModeWork[outerIndex] != MGBATTLE_MODE_LIST_END;
         outerIndex++) {
        for (modeCount = 0; modeCount < 256 && MgModeWork[modeCount] != MGBATTLE_MODE_LIST_END;
             modeCount++) {
            if ((u32)(frandmod(100) + 1) < 50U)
            {
                innerIndex = MgModeWork[outerIndex];
                MgModeWork[outerIndex] = MgModeWork[modeCount];
                MgModeWork[modeCount] = innerIndex;
            }
        }
    }
    fn_1_35CC();
    for (outerIndex = 0; outerIndex < 4; outerIndex++)
    {
        playerOrder[outerIndex] = outerIndex;
    }
    for (outerIndex = 0; outerIndex < 4; outerIndex++)
    {
        for (modeCount = outerIndex + 1; modeCount < 4; modeCount++)
        {
            if (lbl_1_bss_4D4[playerOrder[outerIndex]].position.x >
                lbl_1_bss_4D4[playerOrder[modeCount]].position.x) {
                innerIndex = playerOrder[outerIndex];
                playerOrder[outerIndex] = playerOrder[modeCount];
                playerOrder[modeCount] = innerIndex;
            }
        }
    }
    for (outerIndex = 0; outerIndex < 4; outerIndex++)
    {
        lbl_1_bss_4D4[playerOrder[outerIndex]].awardPosition = lbl_1_data_4B8[outerIndex];
    }
    includeMode = 0;
    outerIndex = 1;
    centerStart = Center;
    rotationStart = CRot;
    zoomStart = CZoom;
    for (; outerIndex != 0; includeMode++)
    {
        outerIndex = 0;
        if (includeMode <= 120)
        {
            innerIndex = includeMode;
            target.x = (-29.0f);
            target.y = (0.0f);
            target.z = (0.0f);
            CRot.x += (0.04f) * (target.x - CRot.x);
            CRot.y = CRot.y + ((0.04f) * (target.y - CRot.y));
            CRot.z = ((target.z - CRot.z) * (0.04f)) + CRot.z;
            if (innerIndex >= 120)
            {
                CRot = target;
            }
            if (MgGameNo == 3)
            {
                target.x = (1000.0f);
            }
            else
            {
                if (MgGameNo == 5)
                {
                    target.x = (1250.0f);
                }
                else
                {
                    target.x = (1500.0f);
                }
            }
            target.y = (230.0f);
            target.z = (230.0f);
            Center.x = centerStart.x + ((f32) innerIndex * ((target.x - centerStart.x) / (120.0f)));
            Center.y = centerStart.y + ((f32) innerIndex * ((target.y - centerStart.y) / (120.0f)));
            Center.z = centerStart.z + ((f32) innerIndex * ((target.z - centerStart.z) / (120.0f)));
            if (innerIndex >= 120)
            {
                Center = target;
            }
            target.z = (5000.0f);
            CZoom += (0.04f) * (target.z - CZoom);
            if (innerIndex >= 120)
            {
                CZoom = target.z;
            }
            outerIndex = 1;
        }
        else
        {
            if (includeMode <= 165)
            {
                centerStart = Center;
                rotationStart = CRot;
                zoomStart = CZoom;
                outerIndex = 1;
            }
            else
            {
                innerIndex = includeMode - 165;
                target.x = (-29.0f);
                target.y = (0.0f);
                target.z = (0.0f);
                CRot.x += (0.06f) * (target.x - CRot.x);
                CRot.y = CRot.y + ((0.06f) * (target.y - CRot.y));
                CRot.z = ((0.06f) * (target.z - CRot.z)) + CRot.z;
                if (innerIndex >= 100)
                {
                    CRot = target;
                }
                target.x = (975.0f);
                target.y = (230.0f);
                target.z = (230.0f);
                Center.x += (0.06f) * (target.x - Center.x);
                Center.y = Center.y + ((0.06f) * (target.y - Center.y));
                Center.z = ((0.06f) * (target.z - Center.z)) + Center.z;
                if (innerIndex >= 100)
                {
                    Center = target;
                }
                target.z = (2730.293f);
                CZoom += (0.06f) * (target.z - CZoom);
                if (innerIndex >= 100)
                {
                    CZoom = target.z;
                }
                outerIndex = 1;
                if (innerIndex >= 100)
                {
                    outerIndex = 0;
                }
            }
        }
        for (modeCount = 0; modeCount < 4; modeCount++)
        {
            if (fn_1_BC74(modeCount) == 0)
            {
                outerIndex = 1;
            }
        }
        HuPrcVSleep();
    }
    fn_1_1408(11);
    fn_1_1648(11);
    fn_1_1528(11);
}

/* Creates the table model and hides it until its entrance animation begins. */
void fn_1_174FC(void) {
    Vec modelData = { 0.0f, 0.0f, 0.0f };

    lbl_1_bss_3E8 = fn_1_0(0, 0, &modelData);
    Hu3DModelRotSetV(lbl_1_bss_3E8, &CRot);
    Hu3DModelScaleSet(lbl_1_bss_3E8, (0.0f), (0.0f), (0.0f));
}

/* Builds the unlocked minigame list for the selected battle mode and previews up to five
 * entries. */
void fn_1_175A0(void) {
    u8 modeOrder[128];
    s32 includeMode;
    s16 gameType;
    s16 modeCount;
    s16 modeIndex;

    memset(modeOrder, 0, sizeof(modeOrder));

    gameType = 0;
    if (*(s16 *)&MgModeSubMode == 1) {
        gameType = 1;
    } else if (*(s16 *)&MgModeSubMode == 2) {
        gameType = 2;
    }

    modeIndex = 0;
    modeCount = 0;
    for (; MgDataTbl[modeIndex].ovl != MGBATTLE_NO_OVERLAY; modeIndex++) {
        includeMode = modeIndex + MGBATTLE_UNLOCK_GAME_BASE;
        if (GWMgUnlockGet(includeMode) != 0 &&
            gameType == (s32)MgDataTbl[modeIndex].type) {
            includeMode = 1;
            if ((MgDataTbl[modeIndex].flag & (1 << 9)) != 0) {
                includeMode = 0;
            }
            if (modeIndex == 21 || modeIndex == 22 || modeIndex == 41) {
                includeMode = 0;
            }
            if (includeMode != 0) {
                modeOrder[modeCount] = (u8)modeIndex;
                modeCount++;
            }
        }
        modeOrder[modeCount] = MGBATTLE_MODE_LIST_END;
    }

    HuPrcVSleep();
    if (modeCount >= 5) {
        fn_1_2A4(5, modeOrder);
    } else {
        fn_1_2A4(modeCount, modeOrder);
    }
    HuPrcVSleep();
}

/* Opens the table view and starts the next minigame after its entrance animation. */
void fn_1_17740(void) {
    Point3d position;
    Point3d offset = { 0.0f, -900.0f, -1800.0f };
    Point3d view;
    Point3d *viewVector;
    f32 scale;
    s16 frame;
    s16 frameState[1];

    position.x = position.y = (0.0f);
    position.z = CZoom;
    view = position;
    viewVector = &view;
    fn_1_1B0AC(CRot.x, CRot.y, CRot.z, viewVector, &position);
    PSVECAdd(&Center, &position, &position);
    PSVECAdd(&position, &offset, &position);
    HuPrcVSleep();

    while (fn_1_45D4() != 0) {
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    Hu3DModelPosSetV(lbl_1_bss_3E8, &position);
    Hu3DModelRotSetV(lbl_1_bss_3E8, &CRot);
    scale = (0.0f);
    Hu3DModelScaleSet(lbl_1_bss_3E8, scale, scale, scale);
    HuPrcVSleep();
    fn_1_43C();
    HuPrcVSleep();
    HuAudFXPlay(MGBATTLE_SE_PANEL_TRANSITION);

    while (scale < (1.0f)) {
        Hu3DModelScaleSet(lbl_1_bss_3E8, scale, scale, scale);
        scale += (0.1f);
        if (scale >= (1.0f)) {
            scale = (1.0f);
        }
        HuPrcVSleep();
    }
    Hu3DModelScaleSet(
        lbl_1_bss_3E8, (1.0f), (1.0f), (1.0f));
    HuPrcVSleep();
    lbl_1_data_8E4 = HuAudFXPlay(MGBATTLE_SE_TABLE_LOOP);
    frameState[0] = 0;

    frame = 1;
    while (frame != 0) {
        fn_1_43C();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        HuPrcVSleep();
        frame++;
        if (frame > 25) {
            frame = 0;
        }
    }

    fn_1_4F8();
    if (lbl_1_data_8E4 >= 0) {
        HuAudFXStop(lbl_1_data_8E4);
    }
    lbl_1_data_8E4 = -1;
    if (frandmod(100) < 50U) {
        MgModeWorkInt[0] = 1;
    } else {
        MgModeWorkInt[0] = 0;
    }
    HuAudFXPlay(MGBATTLE_SE_MINIGAME_TRANSITION);
    fn_1_5E0(MgModeWork[MgModePlayNum], (s16)MgModeWorkInt[0]);
    fn_1_6F4();
    HuPrcVSleep();
    fn_1_1408(12);
    fn_1_1648(12);
}

/* After a minigame, advances scores and updates each player's result choice until all finish. */
s32 fn_1_17A74(void) {
    s16 pending = 0;
    s32 completedScores = 0;
    s16 activePlayers = 0;
    MgBattleChoices available = { { 0, 0, 0, 0 } };
    MgBattleChoices choices = { { 0, 0, 0, 0 } };
    s16 player;
    s16 validCharNo;
    s16 charNo;

    fn_1_1408(14);
    fn_1_1648(14);
    fn_1_1528(14);

    for (player = 0; player < 4; player++) {
        charNo = lbl_1_bss_4D4[player].charNo;
        if (lbl_1_data_4F0[player] >= 10) {
            validCharNo = charNo;
        } else {
            validCharNo = -1;
        }
        if (validCharNo >= 0) {
            available.value[player] = 1;
            activePlayers++;
            MgModeScore[player]++;
            if (MgModeScore[player] >= MgGameNo) {
                MgModeScore[player] = MgGameNo;
            }
        }
    }

    for (player = 0; player < 4; player++) {
        choices.value[player] = available.value[player];
    }

    pending = 1;
    while (pending != 0) {
        pending = 0;
        for (player = 0; player < 4; player++) {
            if (activePlayers > 0) {
                choices.value[player] = fn_1_CA1C(player, choices.value[player]);
                if (choices.value[player] >= 0 &&
                    lbl_1_bss_4D4[player].mode == 8 &&
                    (MgModeScore[0] >= MgGameNo || MgModeScore[1] >= MgGameNo ||
                     MgModeScore[2] >= MgGameNo || MgModeScore[3] >= MgGameNo)) {
                    lbl_1_bss_4D4[player].mode = 0;
                    choices.value[player] = MGBATTLE_NO_CHOICE;
                }
            } else {
                choices.value[player] = fn_1_E770(player, choices.value[player]);
            }
            if (choices.value[player] >= 0) {
                pending = 1;
            }
        }
        HuPrcVSleep();
    }

    completedScores = 0;
    for (player = 0; player < 4; player++) {
        if (MgModeScore[player] >= MgGameNo) {
            completedScores++;
        }
    }
    HuPrcVSleep();
    return completedScores;
}

/* Runs the finish sequence after players complete the required Battle Bridge score. */
void fn_1_17E40(void) {
    Point3d modelRotation;
    s16 selectedCharacter;
    s16 motionIndex;
    s16 waitForCompletion;
    s16 player;

    fn_1_F464();
    waitForCompletion = 1;
    while (waitForCompletion != 0) {
        waitForCompletion = 0;
        player = 0;
        while (player < 4) {
            s16 readyCharacter;

            if (MgModeScore[player] >= MgGameNo) {
                readyCharacter = lbl_1_bss_4D4[player].charNo;
            } else {
                readyCharacter = -1;
            }
            if (readyCharacter >= 0 && fn_1_FA3C(player) == 0) {
                waitForCompletion = 1;
            }
            player++;
        }
        HuPrcVSleep();
    }

    fn_1_3974();
    HuPrcVSleep();
    waitForCompletion = 1;
    selectedCharacter = 0;
    while (waitForCompletion != 0) {
        waitForCompletion = 0;
        if (selectedCharacter == 0) {
            selectedCharacter = fn_1_3A80();
        }
        if (selectedCharacter != 0) {
            fn_1_1B358();
            if (fn_1_1B324() != 0) {
                waitForCompletion = 1;
            }
            if (fn_1_107F0() == 0) {
                waitForCompletion = 1;
            }
        } else {
            waitForCompletion = 1;
        }
        HuPrcVSleep();
    }

    HuPrcVSleep();
    player = 0;
    while (player < 4) {
        s16 endingCharacter;

        if (MgModeScore[player] >= MgGameNo) {
            endingCharacter = lbl_1_bss_4D4[player].charNo;
        } else {
            endingCharacter = -1;
        }
        if (endingCharacter < 0) {
            lbl_1_bss_4D4[player].mode = 11;
            fn_1_AB04(player, 13);
            fn_1_5808(player, lbl_1_bss_4D4[player].model);
        }
        player++;
    }

    HuPrcVSleep();
    waitForCompletion = 1;
    while (waitForCompletion != 0) {
        waitForCompletion = fn_1_5E24();
        player = 0;
        while (player < 4) {
            if (fn_1_5DA8(player) != 0) {
                if (fn_1_11020(player) != 0) {
                    waitForCompletion = 1;
                }
            } else {
                waitForCompletion = 1;
            }
            player++;
        }
        HuPrcVSleep();
    }

    HuPrcVSleep();
    player = 0;
    while (player < 4) {
        Hu3DModelRotGet(lbl_1_bss_4D4[player].model, &modelRotation);
        lbl_1_bss_4D4[player].rotation = modelRotation;
        Hu3DModelPosGet(lbl_1_bss_4D4[player].model, &modelRotation);
        lbl_1_bss_4D4[player].position = modelRotation;
        player++;
    }
    if ((s16)fn_1_18434() != 0) {
        fn_1_18508();
    }
    HuPrcVSleep();
    fn_1_1408(17);
    if ((s16)fn_1_18434() != 0) {
        fn_1_1648(171);
    } else {
        fn_1_1648(17);
        fn_1_1648(172);
    }
    fn_1_1648(18);
    HuAudSStreamFadeOut(lbl_1_data_8D8, 200);
    fn_1_1528(18);
    HuPrcVSleep();
    fn_1_1899C();
    HuPrcVSleep();
    if (lbl_1_data_8E8 >= 0) {
        waitForCompletion = 1;
        while (waitForCompletion != 0) {
            if (HuAudSStreamStatGet(lbl_1_data_8E8) == 0) {
                waitForCompletion = 0;
            }
            HuPrcVSleep();
        }
    }
    lbl_1_data_8DC = HuAudSStreamPlay(MSM_STREAM_MGMUS_KOOPA);
    fn_1_19080();
    HuPrcVSleep();
}

/* Reports whether players on different teams share a score. */
s32 fn_1_18434(void) {
    s16 player;
    s16 otherPlayer;

    for (player = 0; player < 4; player++) {
        for (otherPlayer = player + 1; otherPlayer < 4; otherPlayer++) {
            if (MgModeScore[player] == MgModeScore[otherPlayer] &&
                lbl_1_bss_4D4[player].grpNo !=
                    lbl_1_bss_4D4[otherPlayer].grpNo) {
                return 1;
            }
        }
    }
    return 0;
}

/* Runs the tie award sequence for players who share a score across teams. */
void fn_1_18508(void) {
    MGBATTLE_AWARD_FLAGS awardFlags = { { 0, 0, 0, 0 } };
    s16 player;
    s16 otherPlayer;
    s16 cleanupIndex;
    s16 keepGoing;
    Point3d playerPosition;
    Point3d position;
    Point3d direction;
    Point3d targetPosition;
    Point3d *directionPointer;
    Point3d *targetPositionPointer;

    fn_1_1408(20);
    fn_1_1648(20);
    fn_1_1528(20);
    HuPrcVSleep();

    for (player = 0; player < 4; player++) {
        for (otherPlayer = player + 1; otherPlayer < 4; otherPlayer++) {
            if (MgModeScore[player] == MgModeScore[otherPlayer]) {
                if (lbl_1_bss_4D4[player].grpNo !=
                    lbl_1_bss_4D4[otherPlayer].grpNo) {
                    awardFlags.player[player] = awardFlags.player[otherPlayer] = 1;
                }
            }
        }
    }

    for (player = 0; player < 4; player++) {
        if (awardFlags.player[player] != 0) {
            Hu3DModelPosGet(lbl_1_bss_4D4[player].model, &playerPosition);
            playerPosition.y += (300.0f);
            position.x = position.y = (0.0f);
            position.z = CZoom;
            direction = position;
            directionPointer = &direction;
            fn_1_1B0AC(CRot.x, CRot.y, CRot.z, directionPointer, &position);
            PSVECAdd(&Center, &position, &position);
            PSVECSubtract(&position, &playerPosition, &position);
            fn_1_1AF5C(&position, &position);
            PSVECScale(&position, &position, (1000.0f));
            PSVECAdd(&playerPosition, &position, &playerPosition);
            CharEffectSmokeCreate(1, &playerPosition);
        }
    }

    for (player = 0; player < 4; player++) {
        HuPrcVSleep();
    }

    for (player = 0; player < 4; player++) {
        if (awardFlags.player[player] != 0) {
            Hu3DModelPosGet(lbl_1_bss_4D4[player].model, &playerPosition);
            playerPosition.y += (300.0f);
            targetPosition = playerPosition;
            targetPositionPointer = &targetPosition;
            fn_1_71AC(player, targetPositionPointer);
        }
    }

    for (player = 0; player < 12; player++) {
        HuPrcVSleep();
    }
    fn_1_1408(100);
    fn_1_1648(100);
    HuPrcVSleep();

    keepGoing = 1;
    while (keepGoing != 0) {
        keepGoing = 0;
        for (player = 0; player < 4; player++) {
            if (awardFlags.player[player] > 0) {
                otherPlayer = fn_1_11898(player);
                if (otherPlayer == 0) {
                    awardFlags.player[player] = -1;
                }
                if (otherPlayer == 1) {
                    fn_1_734C(player);
                }
                keepGoing = 1;
            }
        }
        HuPrcVSleep();
    }

    for (cleanupIndex = 0; cleanupIndex < 4; cleanupIndex++) {
        lbl_1_bss_4D4[cleanupIndex].mode = 0;
    }
    fn_1_1528(100);
    for (player = 0; player < 60; player++) {
        HuPrcVSleep();
    }
    fn_1_1408(21);
    fn_1_1648(21);
    for (player = 0; player < 4; player++) {
        if (awardFlags.player[player] != 0) {
            fn_1_7370(player);
        }
    }
    fn_1_1528(21);
    HuPrcVSleep();
}

/* Starts the results jingle and holds each winning character's finish motion. */
void fn_1_1899C(void) {
    s16 characterIds[4];
    f32 motionEndTime;
    s16 player;
    s16 frame;

    frame = 0;
    fn_1_4548(1);
    player = 0;
    while (player < 4) {
        s16 character;

        if (MgModeScore[player] >= MgGameNo) {
            character = lbl_1_bss_4D4[player].charNo;
        } else {
            character = -1;
        }
        characterIds[player] = character;
        if (characterIds[player] >= 0 &&
            lbl_1_bss_4D4[player].playerType == 0) {
            omVibrate(player, 20, 20, 0);
        }
        player++;
    }

    GameMesCreate(4, 3, characterIds[0], characterIds[1], characterIds[2],
        characterIds[3]);
    lbl_1_data_8E8 = HuAudJinglePlay(MGBATTLE_STREAM_RESULTS_JINGLE);
    fn_1_3BB4();
    if (MgGameNo != 3) {
        if (MgGameNo == 5) {
            fn_1_7AF0();
        } else {
            fn_1_8428();
        }
    }

    player = 0;
    while (player < 4) {
        s16 character;

        if (MgModeScore[player] >= MgGameNo) {
            character = lbl_1_bss_4D4[player].charNo;
        } else {
            character = -1;
        }
        if (character >= 0) {
            fn_1_AB04(player, 7);
        } else {
            fn_1_AB04(player, 8);
        }
        player++;
    }

    HuPrcVSleep();
    frame = 0;
    while (frame < 300) {
        player = 0;
        while (player < 4) {
            if (CharMotionShiftIDGet(
                    lbl_1_bss_4D4[player].charNo) < 0) {
                motionEndTime = CharMotionMaxTimeGet(
                    lbl_1_bss_4D4[player].charNo);
                if (Hu3DData[lbl_1_bss_4D4[player].model].motWork.speed +
                    CharMotionTimeGet(lbl_1_bss_4D4[player].charNo) >=
                    motionEndTime) {
                    Hu3DModelAttrSet(lbl_1_bss_4D4[player].model,
                        HU3D_MOTATTR_PAUSE);
                    lbl_1_bss_4D4[player].mode = 0;
                }
            }
            player++;
        }
        HuPrcVSleep();
        frame++;
    }
    fn_1_4548(0);
}

/* Shows the Battle Bridge results board, waits for the human player to continue, then closes it. */
void fn_1_19080(void) {
    Point3d tablePosition;
    Point3d initialWindowPosition;
    Point3d scrollingWindowPosition;
    Point3d selectionWindowPosition;
    Point3d closingWindowPosition;
    s16 otherRank;
    s16 spriteGroupId;
    Point3d *initialPositionPointer;
    Point3d *scrollingPositionPointer;
    Point3d *selectionPositionPointer;
    s16 playerOrder[4];
    s16 playerRank[4];
    HUSPRITE *sprite;
    f32 spriteX;
    f32 spriteY;
    f32 unusedLocalA;
    f32 animationValue;
    s16 swapOrCloseFrames;
    s16 i;
    s16 j;
    s16 humanPlayerIndex;
    s16 presentationActive;
    u16 pressedButtons;

    humanPlayerIndex = -1;
    presentationActive = 1;
    fn_1_1B380();
    animationValue = CRot.x;
    tablePosition.x = Center.y;
    tablePosition.y = Center.z;
    tablePosition.z = CZoom;
    while (presentationActive != 0) {
        CRot.x += (-21.0f - animationValue) / 30.0f;
        Center.y += (240.0f - tablePosition.x) / 30.0f;
        Center.z += (230.0f - tablePosition.y) / 30.0f;
        CZoom += (2900.0f - tablePosition.z) / 30.0f;
        if (CZoom >= 2900.0f) {
            presentationActive = 0;
        }
        HuPrcVSleep();
    }
    CRot.x = -21.0f;
    Center.y = 240.0f;
    Center.z = 230.0f;
    CZoom = 2900.0f;
    presentationActive = 1;
    i = 0;
    while (i < 4) {
        if (lbl_1_bss_4D4[i].playerType == 0) {
            humanPlayerIndex = i;
            break;
        }
        i += 1;
    }
    for (i = 0; i < 4; i++) {
        playerOrder[i] = i;
        playerRank[i] = lbl_1_bss_4D4[i].grpNo;
    }
    for (i = 0; i < 4; i++) {
        for (j = i + 1; j < 4; j++) {
            if (playerRank[i] > playerRank[j]) {
                swapOrCloseFrames = playerRank[i];
                playerRank[i] = playerRank[j];
                playerRank[j] = swapOrCloseFrames;
                swapOrCloseFrames = playerOrder[i];
                playerOrder[i] = playerOrder[j];
                playerOrder[j] = swapOrCloseFrames;
            }
        }
    }
    if (MgModeSubMode == 0) {
        for (i = 0; i < 4; i++) {
            for (j = i + 1; j < 4; j++) {
                if (MgModeScore[playerOrder[i]] < MgModeScore[playerOrder[j]]) {
                    swapOrCloseFrames = playerRank[i];
                    playerRank[i] = playerRank[j];
                    playerRank[j] = swapOrCloseFrames;
                    swapOrCloseFrames = playerOrder[i];
                    playerOrder[i] = playerOrder[j];
                    playerOrder[j] = swapOrCloseFrames;
                } else if (MgModeScore[playerOrder[i]] == MgModeScore[playerOrder[j]]) {
                    swapOrCloseFrames = fn_1_7394(playerOrder[i]);
                    otherRank = fn_1_7394(playerOrder[j]);
                    if (swapOrCloseFrames < otherRank) {
                        swapOrCloseFrames = playerRank[i];
                        playerRank[i] = playerRank[j];
                        playerRank[j] = swapOrCloseFrames;
                        swapOrCloseFrames = playerOrder[i];
                        playerOrder[i] = playerOrder[j];
                        playerOrder[j] = swapOrCloseFrames;
                    }
                }
            }
        }
    }
    HuSprGrpScaleSet(lbl_1_bss_42A[0], 1.0f, 1.0f);
    if (MgModeSubMode == 0) {
        HuSprScaleSet(lbl_1_bss_42A[0], 1, 1.0f, 1.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 2, 0.0f, 0.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 3, 0.0f, 0.0f);
    } else if (MgModeSubMode == 1) {
        HuSprScaleSet(lbl_1_bss_42A[0], 1, 0.0f, 0.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 2, 1.0f, 1.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 3, 0.0f, 0.0f);
    } else {
        HuSprScaleSet(lbl_1_bss_42A[0], 1, 0.0f, 0.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 2, 0.0f, 0.0f);
        HuSprScaleSet(lbl_1_bss_42A[0], 3, 1.0f, 1.0f);
    }
    tablePosition.x = lbl_1_data_6D8.x;
    tablePosition.y = lbl_1_data_6D8.y - 260.0f;
    HuSprGrpPosSet(lbl_1_bss_42A[0], tablePosition.x, tablePosition.y);
    if (MgGameNo == 3) {
        j = 0;
    } else if (MgGameNo == 5) {
        j = 1;
    } else {
        j = 2;
    }
    i = 0;
    while (i < 4) {
        tablePosition = lbl_1_data_6E4[j][i];
        tablePosition.y -= 260.0f;
        initialWindowPosition = tablePosition;
        initialPositionPointer = &initialWindowPosition;
        fn_1_2FA0(i, initialPositionPointer);
        i += 1;
    }
    i = 0;
    while (i < 4) {
        fn_1_2F10(i, playerOrder[i]);
        i += 1;
    }
    i = 0;
    while (i < 4) {
        j = 0;
        while (j < 7) {
            HuSprScaleSet(lbl_1_bss_42A[0], (s16) ((i * 7) + 4 + j), 0.0f, 0.0f);
            j += 1;
        }
        i += 1;
    }
    if (MgModeSubMode == 0) {
        i = 0;
        while (i < 4) {
            j = 0;
            while (j < 7) {
                if (j < MgModeScore[playerOrder[i]]) {
                    HuSprScaleSet(lbl_1_bss_42A[0], (s16) ((i * 7) + 4 + j), 1.0f, 1.0f);
                }
                j += 1;
            }
            i += 1;
        }
    } else if (MgModeSubMode == 1) {
        i = 0;
        while (i < 4) {
            j = 0;
            while (j < 7) {
                if (j < MgModeScore[playerOrder[i]]) {
                    HuSprScaleSet(lbl_1_bss_42A[0], (s16) ((i * 7) + 4 + j), 1.0f, 1.0f);
                }
                j += 1;
            }
            i += 2;
        }
    } else {
        i = 0;
        while (i < 4) {
            j = 0;
            while (j < 7) {
                if (j < MgModeScore[playerOrder[i]]) {
                    HuSprScaleSet(lbl_1_bss_42A[0], (s16) ((i * 7) + 4 + j), 1.0f, 1.0f);
                    spriteGroupId = lbl_1_bss_42A[0];
                    sprite = HuSprData +
                        HuSprGrpData[spriteGroupId].sprId[(s16) ((i * 7) + 4 + j)];
                    spriteX = sprite->pos.x;
                    spriteY = sprite->pos.y;
                    spriteY += 16.5f;
                    HuSprPosSet(lbl_1_bss_42A[0], (s16) ((i * 7) + 4 + j), spriteX, spriteY);
                }
                j += 1;
            }
            i += 2;
        }
    }
    HuPrcVSleep();
    if (MgGameNo == 3) {
        j = 0;
    } else if (MgGameNo == 5) {
        j = 1;
    } else {
        j = 2;
    }
    animationValue = 260.0f;
    while (animationValue >= 0.0f) {
        tablePosition.x = lbl_1_data_6D8.x;
        tablePosition.y = lbl_1_data_6D8.y - animationValue;
        HuSprGrpPosSet(lbl_1_bss_42A[0], tablePosition.x, tablePosition.y);
        i = 0;
        while (i < 4) {
            tablePosition = lbl_1_data_6E4[j][i];
            tablePosition.y -= animationValue;
            scrollingWindowPosition = tablePosition;
            scrollingPositionPointer = &scrollingWindowPosition;
            fn_1_2FA0(i, scrollingPositionPointer);
            i += 1;
        }
        animationValue -= 6.0f;
        HuPrcVSleep();
    }
    HuSprGrpPosSet(lbl_1_bss_42A[0], lbl_1_data_6D8.x, lbl_1_data_6D8.y);
    if (MgGameNo == 3) {
        j = 0;
    } else if (MgGameNo == 5) {
        j = 1;
    } else {
        j = 2;
    }
    i = 0;
    while (i < 4) {
        selectionWindowPosition = lbl_1_data_6E4[j][i];
        selectionPositionPointer = &selectionWindowPosition;
        fn_1_2FA0(i, selectionPositionPointer);
        i += 1;
    }
    HuPrcVSleep();
    fn_1_1408(101);
    fn_1_1648(101);
    HuPrcVSleep();
    while (presentationActive != 0) {
        pressedButtons = 0;
        if (humanPlayerIndex >= 0) {
            pressedButtons = HuPadBtnDown[lbl_1_bss_4D4[humanPlayerIndex].padNo];
        }
        if ((s32) (pressedButtons & PAD_BUTTON_A) != 0) {
            presentationActive = 0;
            HuAudFXPlay(MSM_SE_CMN_02);
        }
        HuPrcVSleep();
    }
    fn_1_1528(101);
    if (MgGameNo == 3) {
        j = 0;
    } else if (MgGameNo == 5) {
        j = 1;
    } else {
        j = 2;
    }
    swapOrCloseFrames = 300;
    animationValue = 0.0f;
    while (animationValue <= swapOrCloseFrames) {
        tablePosition.x = lbl_1_data_6D8.x;
        tablePosition.y = lbl_1_data_6D8.y - animationValue;
        HuSprGrpPosSet(lbl_1_bss_42A[0], tablePosition.x, tablePosition.y);
        i = 0;
        while (i < 4) {
            Point3d *closingPositionPointer;

            tablePosition = lbl_1_data_6E4[j][i];
            tablePosition.y -= animationValue;
            closingWindowPosition = tablePosition;
            closingPositionPointer = &closingWindowPosition;
            fn_1_2FA0(i, closingPositionPointer);
            i += 1;
        }
        animationValue += 6.0f;
        HuPrcVSleep();
    }
    fn_1_3018();
    HuSprGrpScaleSet(lbl_1_bss_42A[0], 0.0f, 0.0f);
    HuPrcVSleep();
}
MgBattleRecord lbl_1_bss_4D4[4];
ANIMDATA * lbl_1_bss_494[16];
s16 lbl_1_bss_492;
s16 lbl_1_bss_486[6];
s16 lbl_1_bss_484;
s16 lbl_1_bss_478[6];
s16 lbl_1_bss_476;
s16 lbl_1_bss_470[3];
s16 lbl_1_bss_468[4];
s16 lbl_1_bss_448[4][4];
s16 lbl_1_bss_446;
s16 lbl_1_bss_444;
s16 lbl_1_bss_43C[4];
s16 lbl_1_bss_43A;
s16 lbl_1_bss_432[4];
s16 lbl_1_bss_42A[4];
s16 lbl_1_bss_3EA[32];
s16 lbl_1_bss_3E8;
