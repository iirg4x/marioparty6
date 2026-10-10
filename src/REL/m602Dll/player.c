/* Controls Odd Card Out player models, choices, and reactions. */
#define _MATH_H
#include "REL/m602Dll.h"
#include "msm_se.h"

#define MSM_SE_M602_CHOICE 1570
#define MSM_SE_M602_CORRECT 1573
#define MSM_SE_M602_FALLING_WEIGHT 1575
#define MSM_SE_M602_IMPACT 1579

u32 lbl_1_data_3D0[12] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mario, 62),
    DATANUM(DATA_mariomot, 48), DATANUM(DATA_mario, 70),
    DATANUM(DATA_mariomot, 53), DATANUM(DATA_mario, 72),
    DATANUM(DATA_mariomot, 25), DATANUM(DATA_mariomot, 46),
    DATANUM(DATA_mariomot, 22), DATANUM(DATA_mario, 28),
    DATANUM(DATA_mariomot, 38), DATANUM(DATA_mariomot, 40),
};

s32 lbl_1_data_400 = -1;

char lbl_1_data_404[16] = "m602_field-dai1";

char lbl_1_data_414[16] = "m602_field-dai2";

char lbl_1_data_424[16] = "m602_field-dai3";

char lbl_1_data_434[16] = "m602_field-dai4";

char lbl_1_data_444[10] = "m602fudaB";

char lbl_1_data_44E[14] = "f-itemhook-r";

struct _struct_lbl_1_bss_318_0x58 lbl_1_bss_318[4];

u8 lbl_1_bss_314;

s32 lbl_1_bss_310;

s16 lbl_1_bss_30C;

s16 lbl_1_bss_30A;

s16 lbl_1_bss_308;

Point3d lbl_1_bss_2FC;

s16 lbl_1_bss_2F4[4];

s16 lbl_1_bss_2F2;

s16 lbl_1_bss_2F0;

M602Bss250Record lbl_1_bss_250[4];

s16 lbl_1_bss_24E;

s16 lbl_1_bss_24C;

s16 lbl_1_bss_24A;

/* Number of players whose intro presentation motion has finished. */
u8 lbl_1_bss_248;

/* Gameplay callback hook; no player update is performed here. */
void fn_1_4CD0(s16 unused)
{

}

/* Visibility preset: show all stage models except the two center entries. */
void fn_1_4CD4(void)
{
    s16 modelIndex;

    modelIndex = 0;
    while (modelIndex < 24) {
        Hu3DModelAttrReset(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        if ((modelIndex == 4) || (modelIndex == 16)) {
            Hu3DModelAttrSet(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        }
        modelIndex += 1;
    }
}

/* The intro camera calls this to show the model groups used by the centered stage view. */
void fn_1_4D68(void)
{
    s16 modelIndex;

    modelIndex = 0;
    while (modelIndex < 24) {
        if (((modelIndex >= 0) && (modelIndex <= 1)) || ((modelIndex >= 8) && (modelIndex <= 9)) ||
            ((modelIndex >= 12) && (modelIndex <= 13)) ||
            ((modelIndex >= 20) && (modelIndex <= 21))) {
            Hu3DModelAttrReset(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        }
        modelIndex += 1;
    }
}

/* The first part of the intro camera pan shows the first twelve stage models except entry four. */
void fn_1_4E48(void)
{
    s16 modelIndex;

    modelIndex = 0;
    while (modelIndex < 24) {
        if ((modelIndex >= 0) && (modelIndex <= 11) && (modelIndex != 4)) {
            Hu3DModelAttrReset(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        }
        modelIndex += 1;
    }
}

/* The second part of the intro camera pan shows the last twelve stage models except entry
 * sixteen. */
void fn_1_4EEC(void)
{
    s16 modelIndex;

    modelIndex = 0;
    while (modelIndex < 24) {
        if ((modelIndex >= 12) && (modelIndex <= 23) && (modelIndex != 16)) {
            Hu3DModelAttrReset(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_202[modelIndex], HU3D_ATTR_DISPOFF);
        }
        modelIndex += 1;
    }
}

/* The round callback counts players whose chosen-card motion has reached its held pose. */
s16 fn_1_4F90(void)
{
    s16 responseCount;
    s16 player;

    responseCount = 0;
    player = 0;
    while (player < 4) {
        responseCount += fn_1_4FE8(player);
        player += 1;
    }
    return responseCount;
}

/* Player queries report whether this player has a settled response during an active choice. */
s16 fn_1_4FE8(s16 player)
{
    if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].motionIndex == 2)) {
        return 1;
    }
    return 0;
}

/* The reveal callback reads the column selected by this player. */
s16 fn_1_502C(s16 player)
{
    return lbl_1_bss_318[player].cardChoice;
}

/* The reveal callback gets the first player with a settled response, or -1 when none responded. */
s16 fn_1_5048(void)
{
    s16 hasResponse;
    s16 player;

    for (player = 0; player < 4; player++) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].motionIndex == 2)) {
            hasResponse = 1;
        } else {
            hasResponse = 0;
        }
        if (hasResponse != 0) { return player; }
    }
    return -1;
}

/* Scoring and winner queries read this player score. */
s16 fn_1_50D4(s16 player)
{
    return lbl_1_bss_318[player].score;
}

/* Score helper: add one correct answer, capped at the winning score of two. */
void fn_1_50F0(s16 player)
{
    lbl_1_bss_318[player].score += 1;
    if (lbl_1_bss_318[player].score > 2) {
        lbl_1_bss_318[player].score = 2;
    }
}

/* Player setup clears this player score. */
void fn_1_5150(s16 player)
{
    lbl_1_bss_318[player].score = 0;
}

/* The round-end callback checks whether any player has reached two correct answers. */
s16 fn_1_5170(void)
{
    s16 player;

    for (player = 0; player < 4; player++) {
        if (fn_1_50D4(player) >= 2) { return 1; }
    }
    return 0;
}

/* Result callbacks get the first player with two correct answers, or -1 when nobody won. */
s16 fn_1_51D4(void)
{
    s16 player;

    for (player = 0; player < 4; player++) {
        if (fn_1_50D4(player) >= 2) { return player; }
    }
    return -1;
}

/* The result callback reads the character number to register as a winner. */
s16 fn_1_5238(s16 player)
{
    return lbl_1_bss_318[player].character;
}

/* Intro setup creates the characters, held cards, player highlights, and falling reaction model. */
void fn_1_5254(void)
{
    ANIMDATA *cardAnimationData;
    s32 player;
    s32 motionOrChoice;

    cardAnimationData =
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_m602, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    for (player = 0; player < 4; player++) {
        lbl_1_bss_318[player].playerIndex = player;
        lbl_1_bss_318[player].character = GwPlayerConf[player].charNo;
        lbl_1_bss_318[player].controller = GwPlayerConf[player].padNo;
        lbl_1_bss_318[player].team = GwPlayerConf[player].grpNo;
        if (GwPlayerConf[player].type != 0) {
            lbl_1_bss_318[player].cpuDifficulty = GwPlayerConf[player].comDif;
        } else {
            lbl_1_bss_318[player].cpuDifficulty = -1;
        }
        lbl_1_bss_318[player].model = CharModelCreate(lbl_1_bss_318[player].character, CHAR_MODEL2);
        motionOrChoice = 0;
        while (motionOrChoice < 12) {
            lbl_1_bss_318[player].motion[motionOrChoice] =
                CharMotionCreate(lbl_1_bss_318[player].character, lbl_1_data_3D0[motionOrChoice]);
            motionOrChoice += 1;
        }
        CharMotionDataClose(lbl_1_bss_318[player].character);
        lbl_1_bss_318[player].motionIndex = 3;
        CharMotionSet(lbl_1_bss_318[player].character,
                      lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
        CharModelAttrSet(lbl_1_bss_318[player].character, HU3D_MOTATTR_LOOP);
        Hu3DModelCameraSet(lbl_1_bss_318[player].model, 8U);
        Hu3DModelLayerSet(lbl_1_bss_318[player].model, 1);
        lbl_1_bss_318[player].pos.x = lbl_1_bss_318[player].pos.y = lbl_1_bss_318[player].pos.z =
            0.0f;
        lbl_1_bss_318[player].rot.x = lbl_1_bss_318[player].rot.y = lbl_1_bss_318[player].rot.z =
            0.0f;
        lbl_1_bss_318[player].scale.x = lbl_1_bss_318[player].scale.y =
            lbl_1_bss_318[player].scale.z = 1.0f;
        if (player == 0) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_404, &lbl_1_bss_318[player].pos);
            lbl_1_bss_318[player].rot.y = 14.0f;
        }
        if (player == 1) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_414, &lbl_1_bss_318[player].pos);
            lbl_1_bss_318[player].rot.y = 5.0f;
        }
        if (player == 2) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_424, &lbl_1_bss_318[player].pos);
            lbl_1_bss_318[player].rot.y = -5.0f;
        }
        if (player == 3) {
            Hu3DModelObjPosGet(lbl_1_bss_23C[0], lbl_1_data_434, &lbl_1_bss_318[player].pos);
            lbl_1_bss_318[player].rot.y = -14.0f;
        }
        Hu3DModelPosSetV(lbl_1_bss_318[player].model, &lbl_1_bss_318[player].pos);
        Hu3DModelRotSetV(lbl_1_bss_318[player].model, &lbl_1_bss_318[player].rot);
        Hu3DModelScaleSetV(lbl_1_bss_318[player].model, &lbl_1_bss_318[player].scale);
        Hu3DModelShadowSet(lbl_1_bss_318[player].model);
        lbl_1_bss_318[player].localLight = Hu3DLLightCreate(
            lbl_1_bss_318[player].model, 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f, 255U, 255U, 128U);
        Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight, 0.0f,
                         -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        lbl_1_bss_318[player].cardModel =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 5), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_318[player].cardAnimation =
            Hu3DAnimCreate(cardAnimationData, lbl_1_bss_318[player].cardModel, lbl_1_data_444);
        motionOrChoice = (u8) frand() % 3;
        if (motionOrChoice == 0) {
            lbl_1_bss_318[player].cardChoice = 1;
        } else if (motionOrChoice == 1) {
            lbl_1_bss_318[player].cardChoice = 2;
        } else {
            lbl_1_bss_318[player].cardChoice = 0;
        }
        Hu3DAnimBankSet(lbl_1_bss_318[player].cardAnimation,
                        (u16) lbl_1_bss_318[player].cardChoice);
        Hu3DModelCameraSet(lbl_1_bss_318[player].cardModel, 8U);
        Hu3DModelLayerSet(lbl_1_bss_318[player].cardModel, 1);
        Hu3DModelHookSet(lbl_1_bss_318[player].model, lbl_1_data_44E,
                         lbl_1_bss_318[player].cardModel);
        fn_1_5150(player);
        lbl_1_bss_318[player].choiceButtons = 0;
        lbl_1_bss_2F4[player] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m602, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_2F4[player], 8U);
        Hu3DModelLayerSet(lbl_1_bss_2F4[player], 2);
        Hu3DModelAttrSet(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(lbl_1_bss_2F4[player], &lbl_1_bss_318[player].pos);
    }
    lbl_1_bss_314 = 0;
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
    CharEffectLayerSet(2);
    lbl_1_bss_30A =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m602, 21), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_308 = Hu3DJointMotion(
        lbl_1_bss_30A, HuDataSelHeapReadNum(DATANUM(DATA_m602, 22), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_30A, lbl_1_bss_308);
    Hu3DModelCameraSet(lbl_1_bss_30A, 8U);
    Hu3DModelLayerSet(lbl_1_bss_30A, 3);
    Hu3DModelAttrSet(lbl_1_bss_30A, HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(lbl_1_bss_30A, HU3D_MOTATTR_LOOP);
    lbl_1_bss_2FC.x = lbl_1_bss_2FC.y = lbl_1_bss_2FC.z = 0.0f;
    Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
    fn_1_96EC();
}

static const s16 lbl_1_rodata_138[6] = { 60, 105, 150, 195, 240, 0 };

/* The intro callback starts each player presentation at its scheduled frame and waits for all
 * four. */
s8 fn_1_5C7C(s16 frameNo)
{
    f32 motionEnd;
    s16 player;

    player = 0;
    while (player < 4) {
        if (frameNo == lbl_1_rodata_138[player]) {
            lbl_1_bss_318[player].motionIndex = 4;
            CharMotionShiftSet(lbl_1_bss_318[player].character,
                               lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        }
        if ((lbl_1_bss_318[player].motionIndex == 4) &&
            (Hu3DMotionShiftIDGet(lbl_1_bss_318[player].model) == -1)) {
            motionEnd = CharMotionMaxTimeGet(lbl_1_bss_318[player].character);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >= motionEnd) {
                lbl_1_bss_318[player].motionIndex = 3;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                lbl_1_bss_248 += 1;
            }
        }
        player += 1;
    }
    player = 0;
    while (player < 4) {
        if ((lbl_1_rodata_138[player] <= frameNo) && (frameNo < lbl_1_rodata_138[player + 1])) {
            Hu3DModelAttrReset(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
            Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight, 0.0f,
                             -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f);
        } else {
            Hu3DModelAttrSet(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
            Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight, 0.0f,
                             -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        }
        player += 1;
    }
    if (lbl_1_bss_248 >= 4U) {
        return 1;
    }
    return 0;
}

/* After the intro presentations, restore every player idle motion and disable their highlights. */
void fn_1_6078(void)
{
    s16 player;

    player = 0;
    while (player < 4) {
        lbl_1_bss_318[player].motionIndex = 3;
        CharMotionShiftSet(lbl_1_bss_318[player].character,
                           lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex], 0.0f,
                           10.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
        Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight, 0.0f,
                         -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
        player += 1;
    }
}

/* During the round shuffle, restore idle poses and hidden cards for players who are not
* flattened, then clear the responding player. Hidden-card restoration shares one counter
* across players and forces restored cards to choice zero. */
void fn_1_61E0(void)
{
    Point3d smokePos;
    s32 player;

    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_318[player].motionIndex != 3) && (lbl_1_bss_318[player].motionIndex != 8)) {
            lbl_1_bss_318[player].motionIndex = 3;
            CharMotionShiftSet(lbl_1_bss_318[player].character,
                               lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                               0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        }
        if ((lbl_1_bss_318[player].cardChoice == 3) && (lbl_1_bss_318[player].motionIndex != 8)) {
            if (lbl_1_bss_24A == 0) {
                Hu3DModelObjPosGet(lbl_1_bss_318[player].model, lbl_1_data_44E, &smokePos);
                smokePos.z += 70.0f;
                CharEffectSmokeCreate(8, &smokePos);
                lbl_1_bss_24A += 1;
            } else if (lbl_1_bss_24A == 7) {
                lbl_1_bss_318[player].cardChoice = 0;
                Hu3DAnimBankSet(lbl_1_bss_318[player].cardAnimation,
                                (u16) lbl_1_bss_318[player].cardChoice);
                Hu3DModelScaleSet(lbl_1_bss_318[player].cardModel, 1.0f, 1.0f, 1.0f);
                lbl_1_bss_24A = 0;
            } else {
                lbl_1_bss_24A += 1;
            }
        }
        player += 1;
    }
    lbl_1_bss_314 = 0;
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
}

/* Each answer frame reads human and CPU choices, selects one simultaneous responder, and plays the
 * choice motion. */
void fn_1_647C(void)
{
    f32 motionEnd;
    s16 responseCountOrSelection;
    s16 player;

    fn_1_6C5C();
    fn_1_9D10();
    responseCountOrSelection = 0;
    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[player].cardChoice != 3) &&
            (((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_B) != 0) ||
             ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_A) != 0) ||
             ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_TRIGGER_R) != 0))) {
            responseCountOrSelection += 1;
        }
        player += 1;
    }
    /* Multiple presses in one frame share a random draw; all but one are discarded. */
    if (responseCountOrSelection > 1) {
        responseCountOrSelection = frandmod(responseCountOrSelection);
        player = 0;
        while (player < 4) {
            if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[player].cardChoice != 3) &&
                (((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_B) != 0) ||
                 ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_A) != 0) ||
                 ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_TRIGGER_R) != 0))) {
                if (responseCountOrSelection != 0) {
                    lbl_1_bss_318[player].choiceButtons = 0;
                }
                responseCountOrSelection -= 1;
            }
            player += 1;
        }
    }
    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_314 == 0) && (lbl_1_bss_318[player].cardChoice != 3)) {
            /* A later test wins if the same player presses more than one choice button. */
            if ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_B) != 0) {
                lbl_1_bss_318[player].motionIndex = 1;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[player].cardChoice = 0;
                Hu3DAnimBankSet(lbl_1_bss_318[player].cardAnimation,
                                (u16) lbl_1_bss_318[player].cardChoice);
                omVibrate(player, 20, 7, 3);
                if (player == 0) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 32);
                } else if (player == 1) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 53);
                } else if (player == 2) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 75);
                } else if (player == 3) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 96);
                }
            }
            if ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_BUTTON_A) != 0) {
                lbl_1_bss_318[player].motionIndex = 1;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[player].cardChoice = 1;
                Hu3DAnimBankSet(lbl_1_bss_318[player].cardAnimation,
                                (u16) lbl_1_bss_318[player].cardChoice);
                omVibrate(player, 20, 7, 3);
                if (player == 0) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 32);
                } else if (player == 1) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 53);
                } else if (player == 2) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 75);
                } else if (player == 3) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 96);
                }
            }
            if ((s32) (lbl_1_bss_318[player].choiceButtons & PAD_TRIGGER_R) != 0) {
                lbl_1_bss_318[player].motionIndex = 1;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                lbl_1_bss_314 = 1;
                lbl_1_bss_318[player].cardChoice = 2;
                Hu3DAnimBankSet(lbl_1_bss_318[player].cardAnimation,
                                (u16) lbl_1_bss_318[player].cardChoice);
                omVibrate(player, 20, 7, 3);
                if (player == 0) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 32);
                } else if (player == 1) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 53);
                } else if (player == 2) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 75);
                } else if (player == 3) {
                    HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CHOICE), 96);
                }
            }
        }
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].motionIndex == 1)) {
            motionEnd = CharMotionMaxTimeGet(lbl_1_bss_318[player].character);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >= motionEnd) {
                lbl_1_bss_318[player].motionIndex = 2;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
            }
        }
        player += 1;
    }
}

/* The answer update copies new B, A, and X presses; X is stored using the trigger-R bit. */
void fn_1_6C5C(void)
{
    s16 player;

    player = 0;
    while (player < 4) {
        lbl_1_bss_318[player].choiceButtons = 0;
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[player].controller] & PAD_BUTTON_B) != 0) {
            lbl_1_bss_318[player].choiceButtons |= PAD_BUTTON_B;
        }
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[player].controller] & PAD_BUTTON_A) != 0) {
            lbl_1_bss_318[player].choiceButtons |= PAD_BUTTON_A;
        }
        if ((s32) (HuPadBtnDown[lbl_1_bss_318[player].controller] & PAD_BUTTON_X) != 0) {
            lbl_1_bss_318[player].choiceButtons |= PAD_TRIGGER_R;
        }
        player += 1;
    }
}

/* During the card reveal, finish the responding player choice motion and hold its selected pose. */
void fn_1_6DB8(void)
{
    f32 motionEnd;
    s16 player;

    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].motionIndex == 1)) {
            motionEnd = CharMotionMaxTimeGet(lbl_1_bss_318[player].character);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >= motionEnd) {
                lbl_1_bss_318[player].motionIndex = 2;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
            }
        }
        player += 1;
    }
}

/* The reveal callback starts the player reaction sequence at its first step. */
void fn_1_6F00(void)
{
    lbl_1_bss_24C = 0;
}

/* Each reaction frame advances the celebration, impact, or return to standing when nobody
 * answers, returning -1 after the sequence ends. A winning answer leaves flattened players
 * down; phases that restore them to idle advance when any one player's motion finishes. */
s16 fn_1_6F14(void)
{
    Point3d smokePos;
    f32 heightOrScale;
    s16 player;
    s16 count;

    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_314 != 0) && (lbl_1_bss_318[player].motionIndex == 1)) {
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                lbl_1_bss_318[player].motionIndex = 2;
                CharMotionSet(lbl_1_bss_318[player].character,
                              lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
            }
        }
        player += 1;
    }
    if (lbl_1_bss_24C < 0) {
        return -1;
    }
    if (lbl_1_bss_30C >= 0) {
        if (lbl_1_bss_310 != 0) {
            {
                switch (lbl_1_bss_24C) {
                case 0:
                    lbl_1_bss_2F2 += 1;
                    player = lbl_1_bss_30C;
                    if (lbl_1_bss_2F2 == 2) {
                        Hu3DModelAttrReset(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
                        Hu3DLLightPosSet(lbl_1_bss_318[player].model,
                                         lbl_1_bss_318[player].localLight, 0.0f, -5000.0f, 0.0f,
                                         0.0f, 1.0f, 0.0f);
                        if (player == 0) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CORRECT), 32);
                        } else if (player == 1) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CORRECT), 53);
                        } else if (player == 2) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CORRECT), 75);
                        } else {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_CORRECT), 96);
                        }
                    }
                    if (lbl_1_bss_2F2 >= 30) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 1:
                    player = 0;
                    while (player < 4) {
                        if (lbl_1_bss_30C == player) {
                            lbl_1_bss_318[player].motionIndex = 4;
                            CharMotionShiftSet(
                                lbl_1_bss_318[player].character,
                                lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                0.0f, 10.0f, HU3D_MOTATTR_LOOP);
                        } else if (lbl_1_bss_318[player].motionIndex != 8) {
                            lbl_1_bss_318[player].motionIndex = 5;
                            CharMotionShiftSet(
                                lbl_1_bss_318[player].character,
                                lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                0.0f, 10.0f, HU3D_MOTATTR_LOOP);
                        }
                        player += 1;
                    }
                    fn_1_48B4();
                    lbl_1_bss_24C += 1;
                    break;
                case 2:
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if ((lbl_1_bss_318[player].motionIndex == 3) ||
                            (lbl_1_bss_318[player].motionIndex == 8)) {
                            count += 1;
                        }
                        if ((lbl_1_bss_318[player].motionIndex != 3) &&
                            (lbl_1_bss_318[player].motionIndex != 8) &&
                            (Hu3DMotionShiftIDGet(lbl_1_bss_318[player].model) == -1)) {
                            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                                CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                                lbl_1_bss_318[player].motionIndex = 3;
                                CharMotionSet(lbl_1_bss_318[player].character,
                                              lbl_1_bss_318[player]
                                                  .motion[lbl_1_bss_318[player].motionIndex]);
                            }
                        }
                        player += 1;
                    }
                    if (count >= 4) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 3:
                    lbl_1_bss_2F2 += 1;
                    if (lbl_1_bss_2F2 >= 30) {
                        fn_1_4998();
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 4:
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if ((lbl_1_bss_318[lbl_1_bss_30C].score < 2) &&
                            (lbl_1_bss_318[player].motionIndex == 8)) {
                            lbl_1_bss_318[player].motionIndex = 9;
                            CharMotionShiftSet(
                                lbl_1_bss_318[player].character,
                                lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                0.0f, 4.0f, HU3D_MOTATTR_LOOP);
                            count += 1;
                        }
                        player += 1;
                    }
                    if (count == 0) {
                        lbl_1_bss_24C = 6;
                    } else {
                        lbl_1_bss_24C += 1;
                    }
                    break;
                case 5:
                    count = 0;
                    player = 0;
                    while (player < 4) {
                        if (lbl_1_bss_318[player].motionIndex == 9) {
                            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                                CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                                lbl_1_bss_318[player].motionIndex = 3;
                                CharMotionSet(lbl_1_bss_318[player].character,
                                              lbl_1_bss_318[player]
                                                  .motion[lbl_1_bss_318[player].motionIndex]);
                                count += 1;
                            }
                        }
                        player += 1;
                    }
                    if (count != 0) {
                        lbl_1_bss_24C += 1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                case 6:
                    lbl_1_bss_2F2 += 1;
                    if (lbl_1_bss_2F2 >= 30) {
                        player = lbl_1_bss_30C;
                        Hu3DModelAttrSet(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
                        Hu3DLLightPosSet(lbl_1_bss_318[player].model,
                                         lbl_1_bss_318[player].localLight, 0.0f, -5000.0f, 0.0f,
                                         0.0f, -1.0f, 0.0f);
                        lbl_1_bss_24C = -1;
                        lbl_1_bss_2F2 = 0;
                    }
                    break;
                }
            }
        } else {
            switch (lbl_1_bss_24C) {
            case 0:
                count = 0;
                player = 0;
                while (player < 4) {
                    if (lbl_1_bss_318[player].motionIndex == 8) {
                        lbl_1_bss_318[player].motionIndex = 9;
                        CharMotionSet(
                            lbl_1_bss_318[player].character,
                            lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                        count += 1;
                    }
                    player += 1;
                }
                if (count == 0) {
                    lbl_1_bss_24C = 2;
                } else {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 1:
                count = 0;
                player = 0;
                while (player < 4) {
                    if (lbl_1_bss_318[player].motionIndex == 9) {
                        if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                            CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                            lbl_1_bss_318[player].motionIndex = 3;
                            CharMotionSet(
                                lbl_1_bss_318[player].character,
                                lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                            count += 1;
                        }
                    }
                    player += 1;
                }
                if (count != 0) {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 2:
                player = lbl_1_bss_30C;
                lbl_1_bss_318[player].motionIndex = 3;
                CharMotionShiftSet(lbl_1_bss_318[player].character,
                                   lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                   0.0f, 15.0f, HU3D_MOTATTR_LOOP);
                lbl_1_bss_24C += 1;
                lbl_1_bss_2F2 = 0;
                break;
            case 3:
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 15) {
                    Hu3DModelObjPosGet(lbl_1_bss_318[lbl_1_bss_30C].model, lbl_1_data_44E,
                                       &smokePos);
                    smokePos.z += 70.0f;
                    CharEffectSmokeCreate(8, &smokePos);
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 4:
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 15) {
                    player = lbl_1_bss_30C;
                    Hu3DModelScaleSet(lbl_1_bss_318[player].cardModel, 0.0f, 0.0f, 0.0f);
                    lbl_1_bss_318[player].cardChoice = 3;
                }
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_2F2 = 0;
                    lbl_1_bss_24C += 1;
                }
                break;
            case 5:
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 6:
                player = lbl_1_bss_30C;
                lbl_1_bss_318[player].motionIndex = 6;
                CharMotionShiftSet(lbl_1_bss_318[player].character,
                                   lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                   0.0f, 4.0f, HU3D_MOTATTR_LOOP);
                lbl_1_bss_24C += 1;
                break;
            case 7:
                count = 0;
                player = 0;
                while (player < 4) {
                    if (player == lbl_1_bss_30C) {
                        if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                            CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                            CharModelAttrSet(lbl_1_bss_318[player].character, HU3D_MOTATTR_PAUSE);
                            lbl_1_bss_2FC = lbl_1_bss_318[player].pos;
                            lbl_1_bss_2FC.y += 1000.0f;
                            Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                            Hu3DModelAttrReset(lbl_1_bss_30A, HU3D_ATTR_DISPOFF);
                            count += 1;
                            lbl_1_data_400 = HuAudFXPlay(MSM_SE_M602_FALLING_WEIGHT);
                            if (player == 0) {
                                HuAudFXPanning(lbl_1_data_400, 32);
                            } else if (player == 1) {
                                HuAudFXPanning(lbl_1_data_400, 53);
                            } else if (player == 2) {
                                HuAudFXPanning(lbl_1_data_400, 75);
                            } else {
                                HuAudFXPanning(lbl_1_data_400, 96);
                            }
                        }
                    }
                    player += 1;
                }
                if (count != 0) {
                    lbl_1_bss_24C += 1;
                }
                break;
            case 8:
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 30) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                    lbl_1_bss_24E = 1;
                }
                break;
            case 9:
                player = lbl_1_bss_30C;
                /* Height lookup uses the player slot, rather than that player character number. */
                heightOrScale = 70.0f + CharModelHeightGet(player);
                lbl_1_bss_2FC.y -= 50.0f;
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                if (lbl_1_bss_2FC.y <= heightOrScale) {
                    lbl_1_bss_318[player].motionIndex = 7;
                    CharMotionSet(lbl_1_bss_318[player].character,
                                  lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                    CharMotionStartEndSet(lbl_1_bss_318[player].character, 0.0f, 11.0f);
                    CharModelAttrReset(lbl_1_bss_318[player].character, HU3D_MOTATTR_PAUSE);
                    fn_1_2FDC(130);
                    lbl_1_bss_24C += 1;
                    if (lbl_1_bss_24E != 0) {
                        if (lbl_1_data_400 >= 0) {
                            HuAudFXStop(lbl_1_data_400);
                        }
                        lbl_1_data_400 = -1;
                        if (player == 0) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_IMPACT), 32);
                            HuAudFXPlayPan(MSM_SE_GUIDE_15, 32);
                        } else if (player == 1) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_IMPACT), 53);
                            HuAudFXPlayPan(MSM_SE_GUIDE_15, 53);
                        } else if (player == 2) {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_IMPACT), 75);
                            HuAudFXPlayPan(MSM_SE_GUIDE_15, 75);
                        } else {
                            HuAudFXPanning(HuAudFXPlay(MSM_SE_M602_IMPACT), 96);
                            HuAudFXPlayPan(MSM_SE_GUIDE_15, 96);
                        }
                    }
                    lbl_1_bss_24E = 0;
                }
                break;
            case 10:
                player = lbl_1_bss_30C;
                CharModelAttrSet(lbl_1_bss_318[player].character, HU3D_MOTATTR_PAUSE);
                heightOrScale = 50.0f;
                lbl_1_bss_2FC.y -= heightOrScale;
                if (lbl_1_bss_2FC.y < 70.0f) {
                    lbl_1_bss_2FC.y = 70.0f;
                }
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                heightOrScale = (lbl_1_bss_2FC.y - 70.0f) / CharModelHeightGet(player);
                if (heightOrScale < 0.05f) {
                    heightOrScale = 0.05f;
                }
                if (heightOrScale > 1.0f) {
                    heightOrScale = 1.0f;
                }
                Hu3DModelScaleSet(lbl_1_bss_318[player].model, 1.0f, heightOrScale, 1.0f);
                if (lbl_1_bss_2FC.y <= 70.0f) {
                    lbl_1_bss_318[player].motionIndex = 8;
                    CharMotionSet(lbl_1_bss_318[player].character,
                                  lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                    Hu3DModelScaleSet(lbl_1_bss_318[player].model, 1.0f, 0.05f, 1.0f);
                    Hu3DMotionTimeSet(lbl_1_bss_30A, 0.0f);
                    smokePos = lbl_1_bss_2FC;
                    smokePos.z += 100.0f;
                    CharEffectSmokeCreate(8, &smokePos);
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                    omVibrate(player, 20, 20, 0);
                    fn_1_1418(30);
                }
                break;
            case 11:
                lbl_1_bss_2F2 += 1;
                if (lbl_1_bss_2F2 >= 120) {
                    lbl_1_bss_24C += 1;
                    lbl_1_bss_2F2 = 0;
                }
                break;
            case 12:
                player = lbl_1_bss_30C;
                lbl_1_bss_2FC.y += 20.0f;
                Hu3DModelPosSetV(lbl_1_bss_30A, &lbl_1_bss_2FC);
                if (lbl_1_bss_2FC.y > 2200.0f) {
                    Hu3DModelAttrSet(lbl_1_bss_30A, HU3D_ATTR_DISPOFF);
                    lbl_1_bss_24C = -1;
                    lbl_1_bss_2F2 = 0;
                }
                /* As the weight rises, restore player height with a damped rebound.
                 * The exit-height test resets the rebound counter before this final scale update.
                 */
                heightOrScale = (lbl_1_bss_2FC.y - 570.0f) / CharModelHeightGet(player);
                if (heightOrScale < 0.05f) {
                    heightOrScale = 0.05f;
                }
                if (heightOrScale >= 1.0f) {
                    if (lbl_1_bss_2F2 < 3) {
                        heightOrScale = 1.0f + (0.06f * (f32) lbl_1_bss_2F2);
                    } else if (lbl_1_bss_2F2 < 9) {
                        heightOrScale = 1.2f - (0.06f * (f32) (lbl_1_bss_2F2 - 3));
                    } else if (lbl_1_bss_2F2 < 15) {
                        heightOrScale = 0.8f + (0.05f * (f32) (lbl_1_bss_2F2 - 9));
                    } else if (lbl_1_bss_2F2 < 21) {
                        heightOrScale = 1.1f - (0.016f * (f32) (lbl_1_bss_2F2 - 15));
                    } else {
                        heightOrScale = 1.0f;
                        CharModelAttrReset(lbl_1_bss_318[player].character, HU3D_MOTATTR_PAUSE);
                    }
                    lbl_1_bss_2F2 += 1;
                }
                Hu3DModelScaleSet(lbl_1_bss_318[player].model, 1.0f, heightOrScale, 1.0f);
                break;
            }
        }
    } else {
        /* With no responding player, only restore any flattened players. */
        switch (lbl_1_bss_24C) {
        case 0:
            count = 0;
            player = 0;
            while (player < 4) {
                if (lbl_1_bss_318[player].motionIndex == 8) {
                    lbl_1_bss_318[player].motionIndex = 9;
                    CharMotionSet(lbl_1_bss_318[player].character,
                                  lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                    count += 1;
                }
                player += 1;
            }
            if (count == 0) {
                lbl_1_bss_24C = -1;
            } else {
                lbl_1_bss_24C += 1;
            }
            break;
        case 1:
            count = 0;
            player = 0;
            while (player < 4) {
                if (lbl_1_bss_318[player].motionIndex == 9) {
                    if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >=
                        CharMotionMaxTimeGet(lbl_1_bss_318[player].character)) {
                        lbl_1_bss_318[player].motionIndex = 3;
                        CharMotionSet(
                            lbl_1_bss_318[player].character,
                            lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex]);
                        count += 1;
                    }
                }
                player += 1;
            }
            if (count != 0) {
                lbl_1_bss_24C += 1;
                lbl_1_bss_2F2 = 0;
            }
            break;
        case 2:
            lbl_1_bss_2F2 += 1;
            if (lbl_1_bss_2F2 >= 30) {
                lbl_1_bss_24C = -1;
                lbl_1_bss_2F2 = 0;
            }
            break;
        }
    }
    return 0;
}

/* The reveal callback records the responding player and whether the choice was correct, awarding a
 * correct answer. */
void fn_1_8D90(s16 player, s32 correct)
{
    if (player >= 0) {
        if (correct != 0) {
            lbl_1_bss_318[player].score += 1;
            if (lbl_1_bss_318[player].score > 2) {
                lbl_1_bss_318[player].score = 2;
            }
            lbl_1_bss_310 = 1;
            lbl_1_bss_30C = player;
            return;
        }
        lbl_1_bss_310 = 0;
        lbl_1_bss_30C = player;
        return;
    }
    lbl_1_bss_310 = 0;
    lbl_1_bss_30C = -1;
}

/* The result intro restores idle poses for players who are not flattened, removes held cards
* with smoke, and returns completion at frame 42. */
s16 fn_1_8E64(s16 frameNo)
{
    Point3d cardScaleOrSmokePos;
    s16 player;

    if (frameNo == 0) {
        player = 0;
        while (player < 4) {
            if (lbl_1_bss_318[player].motionIndex != 8) {
                lbl_1_bss_318[player].motionIndex = 3;
                CharMotionShiftSet(lbl_1_bss_318[player].character,
                                   lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                   0.0f, 4.0f, HU3D_MOTATTR_LOOP);
            }
            player += 1;
        }
    } else if (frameNo == 5) {
        player = 0;
        while (player < 4) {
            Hu3DModelScaleGet(lbl_1_bss_318[player].cardModel, &cardScaleOrSmokePos);
            if (cardScaleOrSmokePos.y > 0.5) {
                Hu3DModelObjPosGet(lbl_1_bss_318[player].model, lbl_1_data_44E,
                                   &cardScaleOrSmokePos);
                cardScaleOrSmokePos.z += 70.0f;
                CharEffectSmokeCreate(8, &cardScaleOrSmokePos);
            }
            player += 1;
        }
    } else if (frameNo == 12) {
        player = 0;
        while (player < 4) {
            Hu3DModelScaleSet(lbl_1_bss_318[player].cardModel, 0.0f, 0.0f, 0.0f);
            lbl_1_bss_318[player].cardChoice = 3;
            player += 1;
        }
    } else if (frameNo == 42) {
        return 1;
    }
    return 0;
}

/* The pre-winner callback starts the winner's result motion and each unflattened loser's
* result motion. A draw starts the losing motion only for players who are not flattened. */
void fn_1_90B4(void)
{
    s16 winner;
    s16 player;

    winner = fn_1_51D4();
    if (winner >= 0) {
        player = 0;
        while (player < 4) {
            if (player == winner) {
                lbl_1_bss_318[player].motionIndex = 10;
                CharMotionShiftSet(lbl_1_bss_318[player].character,
                                   lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                   0.0f, 10.0f, HU3D_MOTATTR_LOOP);
            } else if (lbl_1_bss_318[player].motionIndex != 8) {
                lbl_1_bss_318[player].motionIndex = 11;
                CharMotionShiftSet(lbl_1_bss_318[player].character,
                                   lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                                   0.0f, 10.0f, HU3D_MOTATTR_LOOP);
            }
            player += 1;
        }
        return;
    }
    player = 0;
    while (player < 4) {
        if (lbl_1_bss_318[player].motionIndex != 8) {
            lbl_1_bss_318[player].motionIndex = 11;
            CharMotionShiftSet(lbl_1_bss_318[player].character,
                               lbl_1_bss_318[player].motion[lbl_1_bss_318[player].motionIndex],
                               0.0f, 10.0f, HU3D_MOTATTR_LOOP);
        }
        player += 1;
    }
}

/* Each results frame pauses completed non-base motions, leaving flattened players alone. */
void fn_1_9378(void)
{
    f32 motionEnd;
    s16 player;

    player = 0;
    while (player < 4) {
        if ((lbl_1_bss_318[player].motionIndex != 0) && (lbl_1_bss_318[player].motionIndex != 8)) {
            motionEnd = CharMotionMaxTimeGet(lbl_1_bss_318[player].character);
            if ((1.0f + CharMotionTimeGet(lbl_1_bss_318[player].character)) >= motionEnd) {
                CharModelAttrSet(lbl_1_bss_318[player].character, HU3D_MOTATTR_PAUSE);
            }
        }
        player += 1;
    }
}

/* Each results frame flashes the winning player highlight in ten-frame intervals; reset restarts
 * the cycle. */
void fn_1_9480(s32 reset)
{
    s16 winner;
    s16 player;
    s32 highlightOn;

    winner = fn_1_51D4();
    if (reset != 0) {
        lbl_1_bss_2F0 = 0;
    }
    highlightOn = lbl_1_bss_2F0 / 10;
    if (highlightOn != 0) {
        highlightOn %= 2;
    } else {
        highlightOn = 0;
    }
    player = 0;
    while (player < 4) {
        if (player == winner) {
            if (highlightOn != 0) {
                Hu3DModelAttrReset(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
                Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight,
                                 0.0f, -5000.0f, 0.0f, 0.0f, 1.0f, 0.0f);
            } else {
                Hu3DModelAttrSet(lbl_1_bss_2F4[player], HU3D_ATTR_DISPOFF);
                Hu3DLLightPosSet(lbl_1_bss_318[player].model, lbl_1_bss_318[player].localLight,
                                 0.0f, -5000.0f, 0.0f, 0.0f, -1.0f, 0.0f);
            }
        }
        player += 1;
    }
    lbl_1_bss_2F0 += 1;
}

/* Player setup prepares ten CPU answers and timer triggers, with accuracy increasing by
 * difficulty. */
void fn_1_96EC(void)
{
    s16 accuracyRoll;
    s16 player;
    s16 round;
    s16 answer;

    for (player = 0; player < 4; player++) {
        round = 0;
        while (round < 10) {
            lbl_1_bss_250[player].responseChoice[round] = 0;
            lbl_1_bss_250[player].choiceTriggerValue[round] = 0;
            if (lbl_1_bss_3C[round].choices[0] == lbl_1_bss_3C[round].choices[1]) {
                answer = 2;
            } else if (lbl_1_bss_3C[round].choices[0] == lbl_1_bss_3C[round].choices[2]) {
                answer = 1;
            } else {
                answer = 0;
            }
            accuracyRoll = (u8)frand() % 100;
            if (lbl_1_bss_318[player].cpuDifficulty == 0) {
                if (accuracyRoll < 20) {
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else if (accuracyRoll < 60) {
                    answer += 1;
                    if (answer > 2) {
                        answer = 0;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else {
                    answer -= 1;
                    if (answer < 0) {
                        answer = 2;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                }
                lbl_1_bss_250[player].choiceTriggerValue[round] =
                    (s16) (60 - ((u8) frand() % 60));
            }
            if (lbl_1_bss_318[player].cpuDifficulty == 1) {
                if (accuracyRoll < 40) {
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else if (accuracyRoll < 70) {
                    answer += 1;
                    if (answer > 2) {
                        answer = 0;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else {
                    answer -= 1;
                    if (answer < 0) {
                        answer = 2;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                }
                lbl_1_bss_250[player].choiceTriggerValue[round] =
                    (s16) (120 - ((u8) frand() % 60));
            }
            if (lbl_1_bss_318[player].cpuDifficulty == 2) {
                if (accuracyRoll < 60) {
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else if (accuracyRoll < 80) {
                    answer += 1;
                    if (answer > 2) {
                        answer = 0;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else {
                    answer -= 1;
                    if (answer < 0) {
                        answer = 2;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                }
                lbl_1_bss_250[player].choiceTriggerValue[round] =
                    (s16) (180 - ((u8) frand() % 60));
            }
            if (lbl_1_bss_318[player].cpuDifficulty == 3) {
                if (accuracyRoll < 85) {
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else if (accuracyRoll < 92) {
                    answer += 1;
                    if (answer > 2) {
                        answer = 0;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                } else {
                    answer -= 1;
                    if (answer < 0) {
                        answer = 2;
                    }
                    lbl_1_bss_250[player].responseChoice[round] = answer;
                }
                lbl_1_bss_250[player].choiceTriggerValue[round] =
                    (s16) (240 - ((u8) frand() % 60));
            }
            round += 1;
        }
    }
}

/* While the timer is positive, the answer update replaces CPU controller input with its
* prepared response at the chosen timer value. Otherwise, copied controller presses remain
* unchanged. */
void fn_1_9D10(void)
{
    s16 round;
    s16 player;
    s32 timerValue;

    timerValue = fn_1_9EEC();
    round = fn_1_178C() - 1;
    if (timerValue > 0) {
        player = 0;
        while (player < 4) {
            if (lbl_1_bss_318[player].cpuDifficulty >= 0) {
                lbl_1_bss_318[player].choiceButtons = 0;
                if (timerValue == lbl_1_bss_250[player].choiceTriggerValue[round]) {
                    if (lbl_1_bss_250[player].responseChoice[round] == 0) {
                        lbl_1_bss_318[player].choiceButtons |= PAD_BUTTON_B;
                    } else if (lbl_1_bss_250[player].responseChoice[round] == 1) {
                        lbl_1_bss_318[player].choiceButtons |= PAD_BUTTON_A;
                    } else if (lbl_1_bss_250[player].responseChoice[round] == 2) {
                        lbl_1_bss_318[player].choiceButtons |= PAD_TRIGGER_R;
                    }
                }
            }
            player += 1;
        }
    }
}
