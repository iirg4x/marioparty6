/* Scene models, player animations, score displays, and moving scenery for Garden Grab. */
#include "REL/m635dll.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "game/sprite.h"
#include "game/charman.h"
#include "game/audio.h"
#include "game/frand.h"
#include "string.h"

typedef struct M635SpritePos {
    float x; /* Horizontal screen position in pixels. */
    float y; /* Vertical screen position in pixels. */
} M635SpritePos;

M635Player lbl_1_bss_BC[4];
s16 lbl_1_bss_B4[2][2];
M635Sprite lbl_1_bss_74[4];
M635MovingModel lbl_1_bss_68;
OMOBJMAN *lbl_1_bss_64;
/* Four static bytes with no known game-facing use. */
u8 lbl_1_bss_60[4];

float lbl_1_data_A0[2] = { 7.0f / 6.0f, 5.0f / 3.0f };
HuVecF lbl_1_data_A8[2] = { { -200, 0, 0 }, { 200, 0, 0 } };
HuVecF lbl_1_data_C0[2] = { { -200, 0, 0 }, { 200, 0, 0 } };
HuVecF lbl_1_data_D8 = { 500, 2000, 400 };
HuVecF lbl_1_data_E4 = { -5, -20, -4 };
GXColor lbl_1_data_F0[2] = { { 192, 192, 192, 192 }, { 192, 192, 192, 192 } };
HuVecF lbl_1_data_F8[4] = {
    { -280, 0, -150 }, { -120, 0, -150 },
    { 120, 0, -150 }, { 280, 0, -150 }
};
HuVecF lbl_1_data_128[4] = {
    { -210, 0, -450 }, { -70, 0, -450 },
    { 70, 0, -450 }, { 210, 0, -450 }
};
M635Motion lbl_1_data_158[6] = {
    { DATANUM(DATA_mario, 84), 0 },
    { CHARMOT_HSF_c000m1_300, 1 },
    { CHARMOT_HSF_c000m1_306, 0 },
    { CHARMOT_HSF_c000m1_307, 0 },
    { CHARMOT_HSF_c000m1_316, 0 },
    { DATANUM(DATA_mario, 21), 0 }
};
M635SpritePos lbl_1_data_188[4] = { { 120, 160 }, { 220, 160 }, { 358, 160 }, { 458, 160 } };
s16 lbl_1_data_1A8[20][8] = {
    { 1, 20, 1, 30, 1, 30, 1, 20 },
    { 71, 90, 71, 100, 71, 100, 71, 90 },
    { 111, 130, 111, 140, 111, 140, 111, 130 },
    { 151, 170, 151, 180, 151, 180, 151, 170 },
    { 191, 210, 191, 220, 191, 220, 191, 210 },
    { 231, 250, 231, 260, 231, 260, 231, 250 },
    { 271, 290, 271, 300, 271, 300, 271, 290 },
    { 311, 330, 311, 340, 311, 340, 311, 330 },
    { 351, 370, 351, 380, 351, 380, 351, 370 },
    { 401, 430, 401, 430, 401, 430, 370, 370 },
    { 431, 460, 431, 460, 431, 460, 370, 370 },
    { 461, 490, 461, 490, 461, 490, 370, 370 },
    { 491, 520, 491, 520, 491, 520, 370, 370 },
    { 521, 550, 521, 550, 521, 550, 370, 370 },
    { 551, 580, 551, 580, 551, 580, 370, 370 },
    { 581, 610, 581, 610, 581, 610, 370, 370 },
    { 611, 640, 611, 640, 611, 640, 370, 370 },
    { 641, 670, 641, 670, 641, 670, 370, 370 },
    { 671, 700, 671, 700, 671, 671, 370, 370 },
    { 710, 830, 710, 830, 710, 830, 710, 830 }
};
M635PositionStep lbl_1_data_2E8[21] = {
    { { -150, -150 }, 20 }, { { -180, -180 }, 20 },
    { { -180, -188 }, 10 }, { { -196, -188 }, 10 },
    { { -196, -204 }, 10 }, { { -212, -204 }, 10 },
    { { -212, -220 }, 10 }, { { -228, -220 }, 10 },
    { { -228, -236 }, 10 }, { { -236, -236 }, 10 },
    { { -238, -238 }, 15 }, { { -240, -240 }, 15 },
    { { -242, -242 }, 15 }, { { -244, -244 }, 15 },
    { { -246, -246 }, 15 }, { { -248, -248 }, 15 },
    { { -250, -250 }, 15 }, { { -252, -252 }, 15 },
    { { -254, -254 }, 15 }, { { -256, -256 }, 15 },
    { { -356, -356 }, 15 }
};
u32 lbl_1_data_3E4[2] = { DATANUM(DATA_m635, 0), DATANUM(DATA_m635, 1) };
u32 lbl_1_data_3EC[2] = { DATANUM(DATA_m635, 2), DATANUM(DATA_m635, 3) };
u32 lbl_1_data_3F4[2][2] = {
    { DATANUM(DATA_m635, 5), DATANUM(DATA_m635, 6) },
    { DATANUM(DATA_m635, 7), DATANUM(DATA_m635, 8) }
};
u32 lbl_1_data_404[2][2] = {
    { DATANUM(DATA_m635, 11), DATANUM(DATA_m635, 12) },
    { DATANUM(DATA_m635, 13), DATANUM(DATA_m635, 14) }
};
u32 lbl_1_data_414[2] = { DATANUM(DATA_m635, 15), DATANUM(DATA_m635, 16) };
u32 lbl_1_data_41C[12] = {
    DATANUM(DATA_m635, 22), DATANUM(DATA_m635, 23),
    DATANUM(DATA_m635, 24), DATANUM(DATA_m635, 25),
    DATANUM(DATA_m635, 26), DATANUM(DATA_m635, 27),
    DATANUM(DATA_m635, 28), DATANUM(DATA_m635, 29),
    DATANUM(DATA_m635, 30), DATANUM(DATA_m635, 31),
    DATANUM(DATA_m635, 32), DATANUM(DATA_m635, 33)
};
u32 lbl_1_data_44C[2] = { DATANUM(DATA_m635, 20), DATANUM(DATA_m635, 21) };

/* Create the camera and set the starting view during sequence initialization. */
void fn_1_1774(OMOBJMAN *objman)
{
    /* Called during sequence initialization to create the camera and establish the starting
     * view. */
    OMOBJ *cameraViewObject;

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 20.0f, 60.0f, 25000.0f, 1.2f);
    Hu3DCameraPosSet(1, 0.0f, 800.0f, 2000.0f,
        0.0f, 1.0f, 0.0f, 0.0f, 100.0f, -500.0f);
    cameraViewObject = omAddObjEx(objman, 32730, 0, 0, -1, omOutView);
    Center.x = Center.y = 0.0f;
    Center.z = -400.0f;
    CRot.x = -30.0f;
    CRot.y = 0.0f;
    CRot.z = 0.0f;
    CZoom = 3000.0f;
}

/* Create lighting, shadows, and shared stage models during sequence initialization. */
void fn_1_195C(void)
{
    /* Create lighting, shadows, and shared stage models during sequence initialization. */
    s16 modelId;
    int teamIndex;
    s16 lightId;
    s16 nightMode;
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;

    nightMode = GwMgNightF;
    lbl_1_bss_4.nightF = nightMode;
    lightId = Hu3DGLightCreateV(&lbl_1_data_D8, &lbl_1_data_E4, &lbl_1_data_F0[lbl_1_bss_4.nightF]);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_4.light = lightId;
    Hu3DShadowCreate(30.0f, 20.0f, 10000.0f);
    shadowPos.x = 500.0f;
    shadowPos.y = 2000.0f;
    shadowPos.z = 400.0f;
    shadowUp.y = 1.0f;
    shadowUp.x = shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    modelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_3E4[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    Hu3DModelShadowMapSet(modelId);
    Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
    if (lbl_1_bss_4.nightF == 0) {
        Hu3DModelShadowMapTPLvlSet(modelId, 0.8f);
    } else {
        Hu3DModelShadowMapTPLvlSet(modelId, 0.3f);
    }
    lbl_1_bss_4.shadowedGardenModel = modelId;
    modelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_3EC[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(modelId, HU3D_CAM0);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(modelId, 1.0f, 1.0f, 1.0f);
    lbl_1_bss_4.gardenStageModel = modelId;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_3F4[lbl_1_bss_4.nightF][teamIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_C0[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        Hu3DModelShadowMapSet(modelId);
        if (lbl_1_bss_4.nightF == 0) {
            Hu3DModelShadowMapTPLvlSet(modelId, 0.8f);
        } else {
            Hu3DModelShadowMapTPLvlSet(modelId, 0.3f);
        }
        lbl_1_bss_4.team[teamIndex].teamProgressModel.model = modelId;
        modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_404[lbl_1_bss_4.nightF][teamIndex], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].lateStageModel = modelId;
        modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 4), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_A8[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].teamScoreModel.model = modelId;
        modelId =
            Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m635, 9), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_A8[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].memberOneScoreModel.model = modelId;
        modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m635, 10), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_A8[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].memberZeroScoreModel.model = modelId;
        modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_414[lbl_1_bss_4.nightF], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_A8[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].teamCueModel = modelId;
        modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m635, 17), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(modelId, HU3D_CAM0);
        Hu3DModelPosSetV(modelId, &lbl_1_data_A8[teamIndex]);
        Hu3DMotionSpeedSet(modelId, 0.0f);
        lbl_1_bss_4.team[teamIndex].teamFinishModel = modelId;
    }
}

/* The close callback has no extra scene resources to release here. */
void fn_1_2014(void)
{
    /* This minigame has no additional work in its sequence close callback. */
}

/* Called from the MGSEQ setup callback fn_1_F0 to create score and button sprites before play. */
void fn_1_2018(void)
{
    ANIMDATA *buttonAnimations[12];
    ANIMDATA *backgroundAnimations[2];
    int animationIndex;
    int spriteMember;
    s16 groupId;

    for (animationIndex = 0; animationIndex < 12; animationIndex++) {
        buttonAnimations[animationIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_41C[animationIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (animationIndex = 0; animationIndex < 2; animationIndex++) {
        backgroundAnimations[animationIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(lbl_1_data_44C[animationIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (animationIndex = 0; animationIndex < 4; animationIndex++) {
        groupId = lbl_1_bss_74[animationIndex].group = HuSprGrpCreate(14);
        HuSprGrpPosSet(groupId, lbl_1_data_188[animationIndex].x, lbl_1_data_188[animationIndex].y);
        for (spriteMember = 0; spriteMember < 12; spriteMember++) {
            HuSprGrpMemberSet(groupId, spriteMember,
                              HuSprCreate(buttonAnimations[spriteMember], 10, 0));
            HuSprPosSet(groupId, spriteMember, 0.0f, -5.0f);
            HuSprAttrSet(groupId, spriteMember, HUSPR_ATTR_DISPOFF);
        }
        for (spriteMember = 0; spriteMember < 2; spriteMember++) {
            HuSprGrpMemberSet(groupId, spriteMember + 12,
                              HuSprCreate(backgroundAnimations[spriteMember], 15, 0));
            HuSprAttrSet(groupId, spriteMember + 12, HUSPR_ATTR_DISPOFF);
            HuSprTPLvlSet(groupId, spriteMember + 12, 0.75f);
        }
        lbl_1_bss_74[animationIndex].member = -1;
        lbl_1_bss_74[animationIndex].state = 5;
        lbl_1_bss_74[animationIndex].scale = 0.0f;
    }
}

/* Update a player's button prompt after the input state changes. */
void fn_1_2268(s16 team, s16 player)
{
    /* Update one player's button prompt after the input state changes. */
    M635Sprite *prompt;
    s16 buttonSprite;

    prompt = &lbl_1_bss_74[player + team * 2];
    if (prompt->state == 4) {
        buttonSprite = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
        HuSprAttrSet(prompt->group, buttonSprite, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(prompt->group, buttonSprite + 1, HUSPR_ATTR_DISPOFF);
        prompt->timer = 0;
    }
}

/* Update button prompt scales, blinking, and hide timers once per gameplay frame. */
void fn_1_2330(void)
{
    /* Per-frame update for button prompt scales, blinking, and hide timers. */
    M635Sprite *prompt;
    int spriteIndex;
    s16 buttonSprite;

    for (spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
        prompt = &lbl_1_bss_74[spriteIndex];
        buttonSprite = lbl_1_bss_4.team[spriteIndex / 2].buttonIndex[spriteIndex % 2] * 2;
        switch (prompt->state) {
        case 5:
            break;
        case 0:
            break;
        case 1:
            prompt->scale += 0.2;
            if (prompt->scale >= 1.0f) {
                prompt->scale = 1.0f;
                prompt->state = 0;
            }
            HuSprScaleSet(prompt->group, buttonSprite, prompt->scale, prompt->scale);
            HuSprScaleSet(prompt->group, 12, prompt->scale, prompt->scale);
            break;
        case 3:
            prompt->timer--;
            if (prompt->timer <= 0) {
                HuSprAttrSet(prompt->group, prompt->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrSet(prompt->group, 13, HUSPR_ATTR_DISPOFF);
                prompt->state = 5;
                prompt->scale = 0.0f;
            }
            break;
        case 4:
            if (prompt->scale < 1.0f) {
                prompt->scale += 0.2;
            }
            if (prompt->timer % 8 == 0) {
                HuSprAttrSet(prompt->group, prompt->member, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(prompt->group, prompt->member + 1, HUSPR_ATTR_DISPOFF);
            } else if (prompt->timer % 8 == 4) {
                HuSprAttrSet(prompt->group, prompt->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(prompt->group, prompt->member, HUSPR_ATTR_DISPOFF);
            }
            HuSprScaleSet(prompt->group, prompt->member, prompt->scale, prompt->scale);
            HuSprScaleSet(prompt->group, prompt->member + 1, prompt->scale, prompt->scale);
            HuSprScaleSet(prompt->group, 13, prompt->scale, prompt->scale);
            prompt->timer++;
            break;
        }
    }
}

/* Set a player's prompt state and initialize the corresponding sprite members. */
void fn_1_25EC(s16 team, s16 player, s16 state)
{
    /* Set a player's button prompt state and initialize its visible sprite members. */
    M635Sprite *sprite;
    int i;
    s16 member;

    sprite = &lbl_1_bss_74[player + team * 2];
    sprite->state = state;
    member = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    for (i = 0; i < 2; i++) {
        HuSprAttrSet(sprite->group, i + 12, HUSPR_ATTR_DISPOFF);
    }
    switch (state) {
    case 1:
        sprite->scale = 0.0f;
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 12, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(sprite->group, member, 0.0f, 0.0f);
        HuSprScaleSet(sprite->group, 12, 0.0f, 0.0f);
        sprite->member = member;
        break;
    case 3:
        sprite->timer = 8;
        sprite->scale = 1.0f;
        HuSprAttrReset(sprite->group, member + 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        break;
    case 4:
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        sprite->member = member;
        sprite->timer = 0;
        sprite->scale = 0.0f;
        break;
    }
}

/* Return the current button prompt state for one team member. */
s16 fn_1_27E4(s16 team, s16 player)
{
    /* Return the current button prompt state for one team member. */
    return lbl_1_bss_74[player + team * 2].state;
}

/* Show one selected sprite member and the shared prompt for a player. */
void fn_1_280C(s16 team, s16 player, s16 member)
{
    /* Show a selected sprite member and the shared prompt for one player. */
    M635Sprite *sprite;
    int i;

    sprite = &lbl_1_bss_74[player + team * 2];
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

/* Hide the shared prompt sprite for one player. */
void fn_1_28A8(s16 team, s16 player)
{
    /* Hide the shared prompt sprite for one player. */
    M635Sprite *sprite;

    sprite = &lbl_1_bss_74[player + team * 2];
    HuSprAttrSet(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

/* Destroy all four button-prompt sprite groups when the minigame exits. */
void fn_1_2904(void)
{
    /* Destroy all four button-prompt sprite groups when the minigame exits. */
    int i;

    for (i = 0; i < 4; i++) {
        HuSprGrpKill(lbl_1_bss_74[i].group);
    }
}

/* Build player-to-team mappings, create character models, and load their motions. */
void fn_1_2954(void)
{
    /* Build player-to-team mappings, create character models, and load their motions. */
    int i;
    s16 charNo;
    int member;
    int motion;
    s16 player;
    s16 memberCountA;
    s16 memberCountB;
    s16 model;
    s16 lookupCountA;
    s16 lookupCountB;
    s16 team;
    s16 motionId;
    s16 motionTime;

    lookupCountA = 0;
    lookupCountB = 0;
    memberCountA = 0;
    memberCountB = 0;
    for (i = 0; i < 4; i++) {
        lbl_1_bss_BC[i].playerNo = i;
        charNo = GwPlayerConf[i].charNo;
        lbl_1_bss_BC[i].charNo = charNo;
        lbl_1_bss_BC[i].padNo = GwPlayerConf[i].padNo;
        team = lbl_1_bss_BC[i].teamNo = GwPlayerConf[i].grpNo;
        if (team == 0) {
            lbl_1_bss_BC[i].memberNo = memberCountA;
            memberCountA++;
        } else {
            lbl_1_bss_BC[i].memberNo = memberCountB;
            memberCountB++;
        }
        lbl_1_bss_BC[i].difficulty = GwPlayerConf[i].comDif;
        lbl_1_bss_BC[i].comF = GwPlayerConf[i].type;
        lbl_1_bss_BC[i].timer = fn_1_1308(GwPlayerConf[i].comDif);
        if (team == 0) {
            lbl_1_bss_B4[0][lookupCountA++] = i;
        } else {
            lbl_1_bss_B4[1][lookupCountB++] = i;
        }
    }
    for (i = 0; i < 2; i++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[i][member];
            charNo = lbl_1_bss_BC[player].charNo;
            model = CharModelCreate(charNo, 2);
            Hu3DModelPosSetV(model, &lbl_1_data_F8[member + i * 2]);
            lbl_1_bss_BC[player].z = lbl_1_data_F8[member + i * 2].z;
            lbl_1_bss_BC[player].model = model;
            for (motion = 0; motion < 6; motion++) {
                motionId = CharMotionCreate(charNo, lbl_1_data_158[motion].dataNum);
                lbl_1_bss_BC[player].motions[motion] = motionId;
            }
            Hu3DModelShadowSet(model);
            CharMotionDataClose(charNo);
            CharMotionSet(charNo, lbl_1_bss_BC[player].motions[0]);
            motionTime = CharMotionMaxTimeGet(charNo);
            CharMotionTimeSet(charNo, motionTime);
        }
    }
}

/* Update character depth each frame along the scripted result-position steps. */
void fn_1_2CE8(void)
{
    /* Per-frame update that moves characters through the scripted result positions. */
    int member;
    int team;
    s16 state;
    s16 player;
    s16 model;
    float z;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            state = lbl_1_bss_BC[player].state;
            if (state != 0) {
                model = lbl_1_bss_BC[player].model;
                z = lbl_1_bss_BC[player].z;
                z += (lbl_1_data_2E8[state].z[member] - lbl_1_data_2E8[state - 1].z[member])
                    / lbl_1_data_2E8[state].frames;
                if (z <= lbl_1_data_2E8[state].z[member]) {
                    /* Clamp the local value at the target; this branch does not write it back. */
                    z = lbl_1_data_2E8[state].z[member];
                } else {
                    lbl_1_bss_BC[player].z = z;
                    Hu3DModelPosSet(model,
                        lbl_1_data_F8[member + team * 2].x,
                        lbl_1_data_F8[member + team * 2].y,
                        lbl_1_bss_BC[player].z);
                }
            }
        }
    }
}

/* Set a character's scripted position state when the requested index is valid. */
void fn_1_2F08(s16 player, s16 state)
{
    /* Set a character's scripted position state when the requested index is valid. */
    if (state < 0 || state > 11) {
        return;
    }
    lbl_1_bss_BC[player].state = state;
}

/* Advance a character to the next scripted position state after a press or result. */
void fn_1_2F40(s16 player)
{
    /* Advance a character to the next scripted position state after a press or result. */
    if (lbl_1_bss_BC[player].state > 8) {
        lbl_1_bss_BC[player].state++;
    } else if (lbl_1_bss_BC[player].state == 8) {
        lbl_1_bss_BC[player].state = 10;
    } else if (lbl_1_bss_BC[player].state == 0) {
        lbl_1_bss_BC[player].state = 1;
    } else if (lbl_1_bss_BC[player].state == 1) {
        if (lbl_1_bss_BC[player].memberNo == 0) {
            lbl_1_bss_BC[player].state = 3;
        } else {
            lbl_1_bss_BC[player].state = 2;
        }
    } else if (lbl_1_bss_BC[player].state < 8) {
        lbl_1_bss_BC[player].state += 2;
    }
}

/* Play a character reaction, vibrate that player's controller, and trigger team motion. */
BOOL fn_1_30C8(s16 player, s16 charNo)
{
    /* Play a character reaction, vibrate that player's controller, and trigger team motion. */
    float maxTime;
    M635Player *work;

    work = &lbl_1_bss_BC[player];
    /* The maximum is queried here but the reaction uses the fixed blend below. */
    maxTime = CharMotionMaxTimeGet(charNo);
    fn_1_31BC(charNo);
    CharMotionShiftSet(charNo, lbl_1_bss_BC[player].motions[0],
        0.0f, 8.0f, lbl_1_data_158[0].attr);
    omVibrate(player, 20, 4, 4);
    fn_1_3B7C(lbl_1_bss_BC[player].teamNo, lbl_1_bss_BC[player].memberNo, player);
    return TRUE;
}

/* Request the character's cry twice for each successful button press. */
void fn_1_31BC(s16 charNo)
{
    /* Request the character's cry twice for each successful button press. */
    int i;

    for (i = 0; i < 2; i++) {
        CharModelCryCreate(charNo, 30.0f, 0.8f);
    }
}

/* Blend the selected character motion over the requested number of frames. */
void fn_1_3218(s16 player, s16 motion, float time)
{
    /* Blend the selected character motion over the requested number of frames. */
    s16 charNo;

    if (motion < 0 || motion >= 6) {
        return;
    }
    charNo = lbl_1_bss_BC[player].charNo;
    CharMotionShiftSet(charNo, lbl_1_bss_BC[player].motions[motion],
        0.0f, time, lbl_1_data_158[motion].attr);
}

/* Map a team and member slot to the corresponding global player index. */
s16 fn_1_32E0(s16 team, s16 member)
{
    /* Map a team and member slot to the corresponding global player index. */
    if (team < 0 || team >= 2 || member < 0 || member >= 2) {
        return 0;
    }
    return lbl_1_bss_B4[team][member];
}

/* Reposition all four characters for the tied-result scene. */
void fn_1_3340(void)
{
    /* Reposition all four characters for the tied-result scene. */
    int team;
    int member;
    s16 player;
    s16 model;

    for (team = 0; team < 2; team++) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            model = lbl_1_bss_BC[player].model;
            Hu3DModelPosSetV(model, &lbl_1_data_128[member + team * 2]);
        }
    }
}

/* The close callback has no additional garden animation cleanup in this helper. */
void fn_1_33F8(void)
{
}

/* Advance one team's garden cue and trigger character reactions at its threshold. */
void fn_1_33FC(s16 team)
{
    /* Advance this team's garden animation and trigger character reactions at its threshold. */
    s16 player;
    int member;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    work->scoreStep++;
    Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].teamCueModel, 1.0f);
    if (work->scoreStep >= 20) {
        Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].teamFinishModel, 1.0f);
        for (member = 0; member < 2; member++) {
            player = fn_1_32E0(team, member);
            fn_1_3218(player, 4, 8.0f);
            fn_1_2F40(player);
            OSReport("%d\n", lbl_1_bss_BC[player].state);
            omVibrate(player, 30, 20, 0);
        }
    }
}

/* Update a team's score step and animate both team members when the step changes. */
void fn_1_3738(s16 team, s16 value)
{
    /* Update a team's score step and animate both team members when the step changes. */
    s16 player;
    int member;
    s16 players[2];
    s16 chars[2];
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (work->scoreStep != value) {
        for (member = 0; member < 2; member++) {
            player = lbl_1_bss_B4[team][member];
            players[member] = lbl_1_bss_BC[player].playerNo;
            chars[member] = lbl_1_bss_BC[player].charNo;
            fn_1_30C8(players[member], chars[member]);
            fn_1_2F40(player);
        }
        if (value > 9) {
            if (team == 0) {
                HuAudFXPlay(M635_SFX_RACE_TEAM_0);
            } else {
                HuAudFXPlay(M635_SFX_RACE_TEAM_1);
            }
        }
    }
    work->scoreStep = value;
}

/* Advance one score animation and restart it when the late animation frames are reached. */
void fn_1_3AC4(s16 team)
{
    /* Advance one score animation and restart its loop when it reaches the late frames. */
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    if (work->scoreStep < 10) {
        work->scoreStep++;
    }
    if (work->teamScoreModel.time >= 430) {
        model = &work->teamScoreModel;
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

/* Restart the selected teammate's score animation when it has reached its end. */
void fn_1_3B7C(s16 team, s16 member, s16 player)
{
    /* Restart the selected teammate's score animation when it has reached its end. */
    M635Model *model;
    M635Team *work;

    work = &lbl_1_bss_4.team[team];
    /* The member slot selects the model; player is not used by this helper. */
    if (member != 0) {
        model = &work->memberOneScoreModel;
    } else {
        model = &work->memberZeroScoreModel;
    }
    if (model->time >= 430) {
        model->time = 401;
        Hu3DMotionTimeSet(model->model, model->time);
        Hu3DMotionSpeedSet(model->model, 1.0f);
    }
}

/* Update score-model frames and play team cues as each score step advances. */
void fn_1_3C34(void)
{
    /* Per-frame score animation update: seek to step-specific frames and play team cues. */
    M635Model *model;
    s16 step;
    int team;
    M635Team *work;

    for (team = 0; team < 2; team++) {
        work = &lbl_1_bss_4.team[team];
        step = work->scoreStep;
        if (step != 0) {
            model = &work->teamScoreModel;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][0]) {
                if (lbl_1_data_1A8[step - 1][0] < 400 || lbl_1_data_1A8[step - 1][0] > 700) {
                    model->time = lbl_1_data_1A8[step - 1][0];
                    Hu3DMotionTimeSet(model->model, model->time);
                    if (team == 0) {
                        HuAudFXPlay(M635_SFX_SCORE_TEAM_0);
                    } else {
                        HuAudFXPlay(M635_SFX_SCORE_TEAM_1);
                    }
                }
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][1]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            if (model->time == 769) {
                Hu3DMotionSpeedSet(lbl_1_bss_4.team[team].lateStageModel, 1.0f);
            } else if (model->time == 710) {
                if (team == 0) {
                    HuAudFXPlay(M635_SFX_TEAM_0_STAGE_CUE);
                } else {
                    HuAudFXPlay(M635_SFX_TEAM_1_STAGE_CUE);
                }
            }
            model = &work->memberOneScoreModel;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][2]) {
                model->time = lbl_1_data_1A8[step - 1][2];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][3]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->memberZeroScoreModel;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][4]) {
                model->time = lbl_1_data_1A8[step - 1][4];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][5]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
            model = &work->teamProgressModel;
            model->time = Hu3DMotionTimeGet(model->model);
            if (model->time < lbl_1_data_1A8[step - 1][6]) {
                model->time = lbl_1_data_1A8[step - 1][6];
                Hu3DMotionTimeSet(model->model, model->time);
                Hu3DMotionSpeedSet(model->model, 1.0f);
            }
            if (model->time >= lbl_1_data_1A8[step - 1][7]) {
                Hu3DMotionSpeedSet(model->model, 0.0f);
            }
        }
    }
}

/* Create the moving garden prop and choose its initial side and facing. */
void fn_1_4114(void)
{
    /* Create the moving garden prop and choose its initial side and facing. */
    s16 model;
    s16 direction;
    float x;
    float rotation;

    memset(&lbl_1_bss_68, 0, sizeof(lbl_1_bss_68));
    direction = frandmod(2);
    lbl_1_bss_68.direction = direction;
    if (lbl_1_bss_4.nightF == 0) {
        model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m635, 18), HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m635, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelCameraSet(model, HU3D_CAM0);
    Hu3DModelShadowSet(model);
    if (direction != 0) {
        x = -600.0f;
        rotation = 90.0f;
    } else {
        x = 600.0f;
        rotation = -90.0f;
    }
    Hu3DModelPosSet(model, x, 0.0f, -550.0f);
    Hu3DModelRotSet(model, 0.0f, rotation, 0.0f);
    lbl_1_bss_68.x = x;
    lbl_1_bss_68.model = model;
}

/* Move the garden prop across the stage and pause briefly at each edge. */
void fn_1_42A0(void)
{
    /* Move the garden prop across the stage and pause briefly at each edge. */
    if (lbl_1_bss_68.timer > 0) {
        lbl_1_bss_68.timer--;
        return;
    }
    if (lbl_1_bss_68.direction != 0) {
        lbl_1_bss_68.x += lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x >= 600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 0;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, -90.0f, 0.0f);
        }
    } else {
        lbl_1_bss_68.x -= lbl_1_data_A0[lbl_1_bss_4.nightF];
        if (lbl_1_bss_68.x <= -600.0f) {
            lbl_1_bss_68.timer = 120;
            lbl_1_bss_68.direction = 1;
            Hu3DModelRotSet(lbl_1_bss_68.model, 0.0f, 90.0f, 0.0f);
        }
    }
    Hu3DModelPosSet(lbl_1_bss_68.model, lbl_1_bss_68.x, 0.0f, -550.0f);
}

void fn_1_448C(void)
{
    /* Hide the moving garden prop before the result camera takes over. */
    Hu3DModelAttrSet(lbl_1_bss_68.model, HU3D_ATTR_DISPOFF);
}
