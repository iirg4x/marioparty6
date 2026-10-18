/* Decathlon Park player models, motion changes and result reactions. */
#include "REL/mgmdecathlondll/module_types.h"

typedef struct DecathlonModelGroup {
    u8 reservedBytes[12]; /* storage unused by the player actor code */
    HU3D_MODELID playerModels[4]; /* four tied-rank label model IDs */
    HU3D_MODELID variantModel; /* alternate display model ID */
    HU3D_MODELID modeModels[5][10]; /* five total-score digit positions, each with ten digit
                                     * models */
    HU3D_MODELID spotlightModel; /* spotlight model ID */
    HU3D_MODELID resultModels[4][10]; /* four event-award digit positions, each with ten digit
                                       * models */
} DecathlonModelGroup;

#include <math.h>

#include "game/object.h"

#include "game/hu3d.h"

#include "game/charman.h"

#include "game/memory.h"

#include "game/gamework.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/audio.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

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

u32 lbl_1_data_4A8[8] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 36),
    DATANUM(DATA_mariomot, 37), DATANUM(DATA_mariomot, 57)
};
HuVecF lbl_1_data_4C8[4] = {
    { -280.0f, 0.0f, 0.0f }, { -120.0f, 0.0f, 0.0f }, { 120.0f, 0.0f, 0.0f }, { 280.0f, 0.0f, 0.0f }
};
DecathlonMotionSetting lbl_1_data_4F8[9] = {
    {0, 0.0f, HU3D_MOTATTR_LOOP, 1},
    {0, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {1, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {2, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {3, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {4, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {5, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {6, 8.0f, HU3D_MOTATTR_LOOP, 1},
    {7, 8.0f, HU3D_MOTATTR_LOOP, 1},
};

extern s32 fn_1_FB2C(Vec *direction, Vec *unitDirection);
extern void fn_1_FC84(f32 angle_x, f32 angle_y, f32 angle_z,
                      Vec source, Vec *destination);
extern f64 fn_1_FD44(Vec first, Vec second);

extern void fn_1_9F30(OMOBJ *object);

extern s16 MgModeScore[4];
extern void fn_1_65A0(s16 playerNo);

extern s16 fn_1_9744(s16 playerNo);
extern void fn_1_C6C8(OMOBJ *obj);
extern void fn_1_C344(s16 enabled);
extern void fn_1_D518(s16 playerNo, s16 positionNo);
extern void fn_1_D788(s16 playerNo);
extern void fn_1_DAEC(s16 playerNo);
extern void fn_1_DCF8(void);

/* The player actor update calls this when a movement or result state selects a motion. */
void fn_1_90DC(OMOBJ *object, s16 motionSettingIndex) {
    s16 motionIndex;
    DecathlonPlayerActorData *motionData;

    motionData = object->data;
    if ((motionData->playerNo < 0) || (motionData->playerNo >= 4) ||
        (motionSettingIndex < 0) || (motionSettingIndex >= 9)) {
        return;
    }
    if (motionData->currentMotion != motionSettingIndex) {
        motionIndex = lbl_1_data_4F8[motionSettingIndex].motionIndex;
        if (motionIndex != -1) {
            if (lbl_1_data_4F8[motionSettingIndex].duration <= 1.0f) {
                CharMotionSet(motionData->characterNo, object->mtnId[motionIndex]);
                Hu3DModelAttrReset(object->mdlId[0],
                                   Hu3DModelMotionAttrGet(object->mdlId[0]) | 0x40000000U);
                CharModelAttrSet(motionData->characterNo,
                                 lbl_1_data_4F8[motionSettingIndex].modelAttributes);
            } else {
                Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
                CharMotionShiftSet(motionData->characterNo, object->mtnId[motionIndex],
                                   0.0f, lbl_1_data_4F8[motionSettingIndex].duration,
                                   lbl_1_data_4F8[motionSettingIndex].modelAttributes);
            }
            CharModelVoiceFlagSet(motionData->characterNo,
                                  lbl_1_data_4F8[motionSettingIndex].voiceFlag);
        }
        motionData->currentMotion = motionSettingIndex;
        /* The immediate idle setting becomes the blended idle setting for later requests. */
        if (motionData->currentMotion == 0) {
            motionData->currentMotion = 1;
        }
    }
}

/* Decathlon setup calls this to create and position the four player characters. */
void fn_1_92B8(void) {
    s32 playerNo;
    s32 motionNo;
    OMOBJ *object;
    DecathlonPlayerActorData *playerData;
    DecathlonPlayerActorData *motionData;
    s16 motionIndex;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        object = omAddObjEx(lbl_1_bss_C0, 4864, 1, 8, -1, fn_1_9F30);
        lbl_1_bss_A8[playerNo] = object;
        object->stat |= OM_STAT_MODELPAUSE;
        object->data = HuMemDirectMallocNum(HEAP_HEAP, 60, HU_MEMNUM_OVL);
        memset(object->data, 0, 60);
        playerData = object->data;
        playerData->playerNo = playerNo;
        playerData->characterNo = GwPlayerConf[playerNo].charNo;
        playerData->padNo = GwPlayerConf[playerNo].padNo;
        playerData->groupNo = GwPlayerConf[playerNo].grpNo;
        playerData->playerType = GwPlayerConf[playerNo].type;
        playerData->cpuDifficulty = GwPlayerConf[playerNo].comDif;
        playerData->state = 0;
        *object->mdlId = CharModelCreate(playerData->characterNo, 2);
        for (motionNo = 0; motionNo < 8; motionNo++) {
            object->mtnId[motionNo] = CharMotionCreate(playerData->characterNo,
                                                       lbl_1_data_4A8[motionNo]);
        }
        CharMotionDataClose(playerData->characterNo);
        playerData->currentMotion = -1;
        motionData = object->data;
        if ((motionData->playerNo >= 0) && (motionData->playerNo < 4) &&
            (motionData->currentMotion != 0)) {
            motionIndex = lbl_1_data_4F8[0].motionIndex;
            if (motionIndex != -1) {
                if (lbl_1_data_4F8[0].duration <= 1.0f) {
                    CharMotionSet(motionData->characterNo, object->mtnId[motionIndex]);
                    Hu3DModelAttrReset(*object->mdlId,
                                       Hu3DModelMotionAttrGet(*object->mdlId) | 0x40000000U);
                    CharModelAttrSet(motionData->characterNo,
                                     lbl_1_data_4F8[0].modelAttributes);
                } else {
                    Hu3DModelAttrSet(*object->mdlId, HU3D_MOTATTR_PAUSE);
                    CharMotionShiftSet(motionData->characterNo,
                                       object->mtnId[motionIndex], 0.0f,
                                       lbl_1_data_4F8[0].duration,
                                       lbl_1_data_4F8[0].modelAttributes);
                }
                CharModelVoiceFlagSet(motionData->characterNo,
                                      lbl_1_data_4F8[0].voiceFlag);
            }
            motionData->currentMotion = 0;
            if (motionData->currentMotion == 0) {
                motionData->currentMotion = 1;
            }
        }
        Hu3DModelCameraSet(*object->mdlId, 1);
        Hu3DModelLayerSet(*object->mdlId, 3);
        Hu3DModelShadowSet(*object->mdlId);
        playerData->modelHeight = CharModelHeightGet(playerData->characterNo);
        playerData->position = lbl_1_data_4C8[playerNo];
        Hu3DModelPosSetV(*object->mdlId, &playerData->position);
        playerData->rotation.x = playerData->rotation.y = playerData->rotation.z =
            0.0f;
        Hu3DModelRotSetV(*object->mdlId, &playerData->rotation);
        Hu3DLLightCreate(*object->mdlId, 0.0f, 1500.0f,
                         1500.0f, 0.0f, -0.7f,
                         -0.7f, 192, 192, 192);
    }
    CharEffectLayerSet(4);
}

/* After a minigame returns, the mode sequence parks all player models offscreen. */
void fn_1_9688(void) {
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        OMOBJ *playerObject = lbl_1_bss_A8[playerNo];
        DecathlonPlayerActorData *playerData = playerObject->data;

        playerData->position = lbl_1_data_4C8[playerNo];
        playerData->position.x -= 1000.0f;
        Hu3DModelPosSetV(playerObject->mdlId[0], &playerData->position);
    }
}

/* Result and menu code uses this to get the character selected for a player slot. */
s16 fn_1_9744(s16 playerNo) {
    DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;
s16 characterNo;

    characterNo = playerData->characterNo;
    return characterNo;
}

/* Input and result code uses this to check whether a player slot belongs to a CPU. */
s16 fn_1_9784(s16 playerNo)
{
    DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;

    return (s16)playerData->playerType;
}

/* Window input builds its human-player pad mask from this actor's controller assignment. */
s16 fn_1_97BC(s16 playerNo) {
    DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;

    return playerData->padNo;
}

/* The mode sequence sends every player actor to its offscreen position and waits for arrival. */
void fn_1_97F0(void) {
    s16 playerNo;
    s16 waiting;
    DecathlonPlayerActorData *playerData;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        playerData = (DecathlonPlayerActorData *)lbl_1_bss_A8[playerNo]->data;
        playerData->destination = playerData->position;
        playerData->destination.x -= 1000.0f;
        playerData->state = 1;
    }
    HuPrcVSleep();
    waiting = 1;
    while (waiting != 0) {
        waiting = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            playerData = (DecathlonPlayerActorData *)lbl_1_bss_A8[playerNo]->data;
            if (playerData->state != 0) {
                waiting = 1;
            }
        }
        HuPrcVSleep();
    }
}

/* The mode sequence brings all four players to their positions, then plays their arrival voices. */
void fn_1_98F4(void) {
    s16 playerNo;
    s16 waiting;
    s16 pan;
    DecathlonPlayerActorData *playerData;

    for (playerNo = 0; playerNo < 4; playerNo++) {
        playerData = (DecathlonPlayerActorData *)lbl_1_bss_A8[playerNo]->data;
        playerData->destination = lbl_1_data_4C8[playerNo];
        playerData->state = 1;
    }
    HuPrcVSleep();
    waiting = 1;
    while (waiting != 0) {
        waiting = 0;
        for (playerNo = 0; playerNo < 4; playerNo++) {
            playerData = (DecathlonPlayerActorData *)lbl_1_bss_A8[playerNo]->data;
            if (playerData->state != 0) {
                waiting = 1;
            }
        }
        HuPrcVSleep();
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        playerData = (DecathlonPlayerActorData *)lbl_1_bss_A8[playerNo]->data;
        pan = (s16)(playerNo * 21 + 32);
        HuAudFXPlayPan(lbl_1_data_C98[playerData->characterNo], pan);
    }
}

/* The score-result sequence selects the lead or lower-score animation state for a player. */
void fn_1_9A5C(s16 playerNo, s16 motionState) {
    DecathlonPlayerActorData *playerData;

    playerData = lbl_1_bss_A8[playerNo]->data;
    if (motionState != 0) {
        playerData->state = 7;
        return;
    }
    playerData->state = 10;
}

/* Player result handling selects one of two alternate celebration state pairs here. */
void fn_1_9AAC(s16 playerNo, s16 motionState) {
    DecathlonPlayerActorData *playerData;

    playerData = lbl_1_bss_A8[playerNo]->data;
    if (motionState != 0) {
        playerData->state = 13;
        return;
    }
    playerData->state = 16;
}

/* The final results sequence celebrates all players tied for the highest total and starts the
 * jingle. */
void fn_1_9AFC(void) {
    s16 playerNo;
    s16 scoreMax;
    s16 gameMessageArgs[4];

    scoreMax = 0;
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (scoreMax < MgModeScore[playerNo]) {
            scoreMax = MgModeScore[playerNo];
        }
    }
    for (playerNo = 0; playerNo < 4; playerNo++) {
        if (scoreMax <= MgModeScore[playerNo]) {
            fn_1_9A5C(playerNo, 1);
            fn_1_65A0(playerNo);
            gameMessageArgs[playerNo] = fn_1_9744(playerNo);
            if (!fn_1_9784(playerNo)) {
                omVibrate(playerNo, 20, 20, 0);
            }
        } else {
            fn_1_9A5C(playerNo, 0);
            gameMessageArgs[playerNo] = -1;
        }
    }
    GameMesCreate(4, 3, gameMessageArgs[0], gameMessageArgs[1], gameMessageArgs[2],
                  gameMessageArgs[3]);
    lbl_1_data_C94 = HuAudJinglePlay(MSM_STREAM_MGMUS_12);
}

/* The main sequence advances each player's active celebration into its result state. */
void fn_1_9CD0(void) {
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; ++playerNo) {
        DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;

        if ((playerData->state == 7) || (playerData->state == 8)) {
            playerData->state = 9;
        }
        if ((playerData->state == 10) || (playerData->state == 11)) {
            playerData->state = 12;
        }
        if ((playerData->state == 13) || (playerData->state == 14)) {
            playerData->state = 15;
        }
        if ((playerData->state == 16) || (playerData->state == 17)) {
            playerData->state = 18;
        }
    }
}

/* After a minigame returns, the main sequence requests motion setting eight for all players. */
void fn_1_9DA0(void) {
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; ++playerNo) {
        DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;
        playerData->state = 4;
    }
}

/* After the score message, the main sequence returns all four players to idle. */
void fn_1_9DF8(void) {
    s16 playerNo;

    for (playerNo = 0; playerNo < 4; ++playerNo) {
        DecathlonPlayerActorData *playerData = lbl_1_bss_A8[playerNo]->data;
        playerData->state = 6;
    }
}

/* Scenery updates call this to find the players' average world position. */
void fn_1_9E50(Vec *center) {
    s16 playerNo;
    DecathlonPlayerActorData *playerData;

    center->x = center->y = center->z = 0.0f;
    for (playerNo = 0; playerNo < 4; ++playerNo) {
        playerData = lbl_1_bss_A8[playerNo]->data;
        center->x += playerData->position.x;
        center->y += playerData->position.y;
        center->z += playerData->position.z;
    }
    center->x *= 0.25f;
    center->y *= 0.25f;
    center->z *= 0.25f;
}

/* The object manager calls this each frame to move, turn, and animate a player. */
void fn_1_9F30(OMOBJ *object) {
    DecathlonPlayerActorData * playerData = object->data;
    Vec remainingDistance;
    Vec movementDirection;
    f32 turnValue; /* alignment dot products while choosing a turn, then angles in degrees */
    f32 motionEndTime;
    switch (playerData->state)
    {
    case 0 :
        fn_1_90DC(object, 1);
        break;
    case 1 :
        /* Turn toward the requested destination before beginning the walk. */
        fn_1_90DC(object, 2);
        {
            PSVECSubtract(&playerData->destination, &playerData->position, &remainingDistance);
            fn_1_FB2C(& remainingDistance, & movementDirection);
            {
                remainingDistance.x = remainingDistance.y = 0.0f;
                remainingDistance.z = 1.0f;
                fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                          &remainingDistance);
            }
            {
                turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
                if (turnValue >= 0.0f)
                {
                    remainingDistance.x = 1.0f;
                    remainingDistance.y = remainingDistance.z = 0.0f;
                    fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                              &remainingDistance);
                    turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
                    if (turnValue >= 0.0f)
                    {
                        playerData->rotation.y += 10.0f;
                    }
                    else
                    {
                        playerData->rotation.y -= 10.0f;
                    }
                    {
                        remainingDistance.x = remainingDistance.y = 0.0f;
                        remainingDistance.z = 1.0f;
                        fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                                  &remainingDistance);
                    }
                    turnValue = (f32) fn_1_FD44(remainingDistance, movementDirection);
                    if (-10.0f < turnValue && turnValue < 10.0f)
                    {
                        remainingDistance.x = remainingDistance.y = 0.0f;
                        remainingDistance.z = 1.0f;
                        turnValue = (f32) fn_1_FD44(remainingDistance, movementDirection);
                        if (playerData->rotation.y < 0.0f)
                        {
                            playerData->rotation.y = - turnValue;
                        }
                        else
                        {
                            if (playerData->rotation.y > 0.0f)
                            {
                                playerData->rotation.y = turnValue;
                            }
                        }
                        playerData->state = 2;
                    }
                }
                else
                {
                    remainingDistance.x = 1.0f;
                    remainingDistance.y = remainingDistance.z = 0.0f;
                    fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                              &remainingDistance);
                    turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
                    if (turnValue >= 0.0f)
                    {
                        playerData->rotation.y += 10.0f;
                    }
                    else
                    {
                        playerData->rotation.y -= 10.0f;
                    }
                }
            }
        }
        break;
    case 2 :
        /* Walk six world units per frame, snapping once the target is within one step. */
        fn_1_90DC(object, 3);
        {
            PSVECSubtract(&playerData->destination, &playerData->position, &remainingDistance);
            if (PSVECSquareMag(& remainingDistance) <= 36.0f)
            {
                playerData->position = playerData->destination;
                playerData->state = 3;
            }
            else
            {
                remainingDistance.x = remainingDistance.y = 0.0f;
                remainingDistance.z = 6.0f;
                fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                          &remainingDistance);
                movementDirection = playerData->position;
                PSVECAdd(& remainingDistance, & movementDirection, & playerData->position);
            }
        }
        break;
    case 3 :
        /* Face the front again before signaling arrival with the idle state. */
        fn_1_90DC(object, 2);
        {
            movementDirection.x = movementDirection.y = 0.0f;
            movementDirection.z = 1.0f;
            remainingDistance.x = remainingDistance.y = 0.0f;
            remainingDistance.z = 1.0f;
            fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance, & remainingDistance);
            turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
            if (turnValue >= 0.0f)
            {
                remainingDistance.x = 1.0f;
                remainingDistance.y = remainingDistance.z = 0.0f;
                fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                          &remainingDistance);
                turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
                if (turnValue >= 0.0f)
                {
                    playerData->rotation.y += 10.0f;
                }
                else
                {
                    playerData->rotation.y -= 10.0f;
                }
                remainingDistance.x = remainingDistance.y = 0.0f;
                remainingDistance.z = 1.0f;
                fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                          &remainingDistance);
                turnValue = (f32) fn_1_FD44(remainingDistance, movementDirection);
                if (-10.0f < turnValue && turnValue < 10.0f)
                {
                    remainingDistance.x = remainingDistance.y = 0.0f;
                    remainingDistance.z = 1.0f;
                    turnValue = (f32) fn_1_FD44(remainingDistance, movementDirection);
                    if (playerData->rotation.y < 0.0f)
                    {
                        playerData->rotation.y = - turnValue;
                    }
                    else
                    {
                        if (playerData->rotation.y > 0.0f)
                        {
                            playerData->rotation.y = turnValue;
                        }
                    }
                    playerData->state = 0;
                }
            }
            else
            {
                remainingDistance.x = 1.0f;
                remainingDistance.y = remainingDistance.z = 0.0f;
                fn_1_FC84(0.0f, playerData->rotation.y, 0.0f, remainingDistance,
                          &remainingDistance);
                turnValue = PSVECDotProduct(& remainingDistance, & movementDirection);
                if (turnValue >= 0.0f)
                {
                    playerData->rotation.y += 10.0f;
                }
                else
                {
                    playerData->rotation.y = playerData->rotation.y - 10.0f;
                }
            }
        }
        break;
    case 4 :
        fn_1_90DC(object, 8);
        playerData->state = 5;
        break;
    case 6 :
        fn_1_90DC(object, 1);
        playerData->state = 0;
        break;
    case 7 :
        fn_1_90DC(object, 4);
        playerData->state = 8;
        break;
    case 8 :
        /* Celebration states hold their last frame until the sequence requests idle. */
        if (CharMotionShiftIDGet(playerData->characterNo) < 0)
        {
            motionEndTime = CharMotionMaxTimeGet(playerData->characterNo);
            if (Hu3DData[*object->mdlId].motWork.speed +
                    CharMotionTimeGet(playerData->characterNo) >=
                motionEndTime) {
                Hu3DModelAttrSet(* object->mdlId, HU3D_MOTATTR_PAUSE);
            }
        }
        break;
    case 9 :
        fn_1_90DC(object, 1);
        playerData->state = 0;
        break;
    case 10 :
        fn_1_90DC(object, 5);
        playerData->state = 11;
        break;
    case 11 :
        if (0 > CharMotionShiftIDGet(playerData->characterNo))
        {
            motionEndTime = CharMotionMaxTimeGet(playerData->characterNo);
            if (Hu3DData[*object->mdlId].motWork.speed +
                    CharMotionTimeGet(playerData->characterNo) >=
                motionEndTime) {
                Hu3DModelAttrSet(* object->mdlId, HU3D_MOTATTR_PAUSE);
            }
        }
        break;
    case 12 :
        fn_1_90DC(object, 1);
        playerData->state = 0;
        break;
    case 13 :
        fn_1_90DC(object, 6);
        playerData->state = 14;
        break;
    case 14 :
        if (CharMotionShiftIDGet(playerData->characterNo) < 0)
        {
            motionEndTime = CharMotionMaxTimeGet(playerData->characterNo);
            if (Hu3DData[*object->mdlId].motWork.speed +
                    CharMotionTimeGet(playerData->characterNo) >=
                motionEndTime) {
                Hu3DModelAttrSet(* object->mdlId, HU3D_MOTATTR_PAUSE);
            }
        }
        break;
    case 15 :
        fn_1_90DC(object, 1);
        playerData->state = 0;
        break;
    case 16 :
        fn_1_90DC(object, 7);
        playerData->state = 17;
        break;
    case 17 :
        if (CharMotionShiftIDGet(playerData->characterNo) < 0)
        {
            motionEndTime = CharMotionMaxTimeGet(playerData->characterNo);
            if (Hu3DData[*object->mdlId].motWork.speed +
                    CharMotionTimeGet(playerData->characterNo) >=
                motionEndTime) {
                Hu3DModelAttrSet(* object->mdlId, HU3D_MOTATTR_PAUSE);
            }
        }
        break;
    case 18 :
        fn_1_90DC(object, 1);
        playerData->state = 0;
        break;
    default :
        break;
    }
    Hu3DModelRotSetV(* object->mdlId, & playerData->rotation);
    Hu3DModelPosSetV(* object->mdlId, & playerData->position);
}

OMOBJ *lbl_1_bss_A8[4];
