/* Gondola Glide player setup, button responses, movement effects, and result exits. */
#include "REL/m638Dll.h"

#define M638_TEAM0_REACTION_SFX_ID 1891
#define M638_TEAM1_REACTION_SFX_ID 1892

u32 lbl_1_data_548[5] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mario, 103),
    DATANUM(DATA_mariomot, 38), DATANUM(DATA_mariomot, 40),
    DATANUM(DATA_mariomot, 14)
};

/* Object initializer: create a player model, join its team, and install its update callback. */
void fn_1_67EC(OMOBJ *obj)
{
    struct M638TeamSlotsView {
        M638PlayerView *players[2];
        u8 unknown008[776];
        s16 active;
        s16 button;
        u8 unknown314[36];
        s16 variant;
        u8 unknown33A[10];
    };
    s16 motion;
    s32 i;
    M638PlayerView *player;
    struct M638TeamSlotsView *teams;

    player = obj->data;
    player->object = obj;
    player->playerNo = (s16)obj->work[0];
    player->charNo = GwPlayerConf[player->playerNo].charNo;
    player->group = GwPlayerConf[player->playerNo].grpNo;
    player->padNo = GwPlayerConf[player->playerNo].padNo;
    player->isCom = GwPlayerConf[player->playerNo].type;
    player->difficulty = GwPlayerConf[player->playerNo].comDif;

    *obj->mdlId = CharModelCreate(player->charNo, 2);
    player->model = *obj->mdlId;
    for (i = 0; i < 5; i++) {
        motion = CharMotionCreate(player->charNo, lbl_1_data_548[i]);
        player->motions[i] = motion;
        obj->mtnId[i] = motion;
    }
    CharMotionSet(player->charNo, obj->mtnId[1]);
    CharModelAttrSet(player->charNo, HU3D_MOTATTR_PAUSE);
    CharModelAttrReset(player->charNo, 1U);
    CharMotionDataClose(player->charNo);
    Hu3DModelShadowSet(player->model);

    teams = lbl_1_bss_4->data;
    player->member = (s16)(teams[player->group].players[0] != NULL);
    teams[player->group].players[player->member] = player;
    player->team = (M638TeamInputView *)&teams[player->group];

    player->state = 0;
    player->exitState = 0;
    player->activity = 0.0f;
    player->impulse = 0.0f;
    player->pos.x = 0.0f;
    player->pos.y = 0.0f;
    player->pos.z = 0.0f;
    player->rot.x = 0.0f;
    player->rot.y = 0.0f;
    player->rot.z = 0.0f;
    player->count = 0;
    player->nextCount = (s32)fn_1_7344(player->difficulty) / 2;
    player->phase = 0.0f;
    Hu3DModelPosSetV(player->model, &player->pos);
    omSetTra(player->object, player->pos.x, player->pos.y, player->pos.z);
    Hu3DModelRotSetV(player->model, &player->rot);
    omSetRot(player->object, player->rot.x, player->rot.y, player->rot.z);
    Hu3DModelCameraSet(player->model, (u16)lbl_1_rodata_10[player->group]);
    Hu3DModelLayerSet(player->model, 4);
    obj->objFunc = fn_1_6B00;
}

/* Object update: read player input, update activity and animations, then sync the model pose. */
void fn_1_6B00(OMOBJ *obj)
{
    int pressed;
    M638PlayerView *player;
    player = obj->data;
    pressed = 0;
    player->buttonA = 0;
    player->buttonB = 0;
    if (player->team->active) {
        if (player->isCom) {
            pressed = fn_1_72D0(player);
        } else {
            pressed = fn_1_71DC(player);
        }
        fn_1_745C(player);
    }
    if (pressed) {
        player->activity += 0.5f;
    } else {
        player->activity -= 0.05f;
    }
    if (player->activity < 0.0f) {
        player->activity = 0.0f;
    }
    if (player->activity > 2.0f) {
        player->activity = 2.0f;
    }
    if (player->state == 0 && player->activity > 0.25f) {
        player->state = 1;
    }
    fn_1_6C98(player);
    fn_1_6E40(player);
    Hu3DModelPosSetV(player->model, &player->pos);
    omSetTra(player->object, player->pos.x, player->pos.y, player->pos.z);
    Hu3DModelRotSetV(player->model, &player->rot);
    omSetRot(player->object, player->rot.x, player->rot.y, player->rot.z);
}

/* Advance the player's reaction animation and restore the idle pose when it ends. */
void fn_1_6C98(M638PlayerView *player)
{
    f32 motionSpeed;
    switch (player->state) {
    case 1:
        if (player->team->variant == 0) {
            HuAudFXPlay(M638_TEAM0_REACTION_SFX_ID);
        } else {
            HuAudFXPlay(M638_TEAM1_REACTION_SFX_ID);
        }
        Hu3DModelAttrReset(player->model, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(player->model, HU3D_MOTATTR_REV);
        Hu3DModelAttrReset(player->attachedModel, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(player->attachedModel, HU3D_MOTATTR_REV);
        motionSpeed = player->activity;
        if (motionSpeed < 0.25f) {
            motionSpeed = 0.25f;
        }
        if (motionSpeed > 2.0f) {
            motionSpeed = 2.0f;
        }
        Hu3DMotionSpeedSet(player->model, motionSpeed);
        Hu3DMotionSpeedSet(player->attachedModel, motionSpeed);
        player->state++;
        /* fallthrough */
    case 2:
        if (Hu3DMotionEndCheck(player->model)) {
            Hu3DModelAttrSet(player->model, HU3D_MOTATTR_REV);
            Hu3DModelAttrSet(player->attachedModel, HU3D_MOTATTR_REV);
            player->state++;
        }
        break;
    case 0:
        break;
    case 3:
        if (Hu3DMotionEndCheck(player->model)) {
            Hu3DModelAttrSet(player->model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(player->attachedModel, HU3D_MOTATTR_PAUSE);
            player->state = 0;
        }
        break;
    }
}

/* Apply the player's small rhythmic body-scale animation during object updates. */
void fn_1_6E40(M638PlayerView *player)
{
    f32 scaleY;
    f32 scaleZ;
    player->phase += 3.0f;
    while (player->phase > 180.0f) {
        player->phase -= 180.0f;
    }
    {
        f32 pi = acos(-1.0);
        scaleY = 1.0 + (0.01899999938905239 * sin((player->phase * pi) / 180.0f));
    }
    {
        f32 pi = acos(-1.0);
        scaleZ =
            1.0089999437332153 + (0.008999999612569809 * sin((2.0f * player->phase * pi) / 180.0f));
    }
    Hu3DModelScaleSet(player->model, 1.0f, scaleY, scaleZ);
    omSetSca(player->object, 1.0f, scaleY, scaleZ);
}

/* Result callback: play the player's exit animation and return when it reaches the next pose. */
s32 fn_1_6FE4(void *playerContext)
{
    M638PlayerView *player;
    f32 exitProgress;
    f32 attachedModelScale;
    int animationFinished;
    animationFinished = 0;
    player = playerContext;
    switch (player->exitState) {
    case 0:
        Hu3DModelAttrReset(player->model, HU3D_MOTATTR_REV);
        Hu3DModelAttrSet(player->model, HU3D_MOTATTR_PAUSE);
        Hu3DMotionShiftSet(player->model, player->motions[4], 0.0f, 10.0f, 0U);
        Hu3DMotionTimeSet(player->model, 0.0f);
        player->elapsed = 0;
        player->exitState++;
        break;
    case 1:
        player->elapsed++;
        exitProgress = player->elapsed / 58.0f;
        attachedModelScale = 1.0f - (0.5f * exitProgress);
        Hu3DModelScaleSet(player->attachedModel, attachedModelScale, attachedModelScale,
                          attachedModelScale);
        player->rot.y = -180.0f * exitProgress;
        if (player->elapsed >= 58) {
            Hu3DMotionShiftSet(player->model, player->motions[0], 60.0f, 5.0f, HU3D_MOTATTR_LOOP);
            Hu3DModelScaleSet(player->attachedModel, 0.5f, 0.5f, 0.5f);
            player->rot.y = -180.0f;
            player->exitState = 2;
            animationFinished = 1;
        }
        break;
    }
    return animationFinished;
}

/* Human input path used by the active player's object update. The team's selected button counts as
 * A; the other counts as B. */
s32 fn_1_71DC(void *playerContext)
{
    u16 padButtons[2] = { PAD_BUTTON_A, PAD_BUTTON_B };
    s32 inputAccepted;

    inputAccepted = 0;
    if ((s32)(HuPadBtnDown[((M638PlayerView *)playerContext)->padNo] &
              padButtons[((M638PlayerView *)playerContext)->team->button ^ 1]) != 0) {
        inputAccepted = 1;
        ((M638PlayerView *)playerContext)->buttonB = 1;
        /* Rumble is suppressed during wipe transitions, when vibration is off, or for a computer
         * player. */
        omVibrate(((M638PlayerView *)playerContext)->playerNo, 20, 7, 3);
    } else if ((s32)(HuPadBtnDown[((M638PlayerView *)playerContext)->padNo] &
                     padButtons[((M638PlayerView *)playerContext)->team->button]) != 0) {
        inputAccepted = 1;
        ((M638PlayerView *)playerContext)->buttonA = 1;
    }
    return inputAccepted;
}

/* Computer input path used by the active player's object update; generate an A press after the
 * selected delay. */
s32 fn_1_72D0(void *playerContext)
{
    s32 inputAccepted;

    inputAccepted = 0;
    ((M638PlayerView *)playerContext)->count = (s32)(((M638PlayerView *)playerContext)->count + 1);
    if ((s32) ((M638PlayerView *) playerContext)->count >=
        (s32) ((M638PlayerView *) playerContext)->nextCount) {
        ((M638PlayerView *)playerContext)->count = 0;
        ((M638PlayerView *) playerContext)->nextCount =
            fn_1_7344((s32) ((M638PlayerView *) playerContext)->difficulty);
        ((M638PlayerView *)playerContext)->buttonA = 1;
        inputAccepted = 1;
    }
    return inputAccepted;
}

/* Choose the next computer-player input delay in updates; difficulty 2 uses its three weighted
 * outcomes. */
u32 fn_1_7344(s32 difficulty)
{
    int baseDelayFrames[4] = { 30, 12, 7, 7 };
    int delayRangeFrames[4] = { 30, 8, 3, 2 };
    int difficulty2Cutoffs[3] = { 33, 90, 100 };
    int roll;
    int outcomeIndex;
    if (difficulty == 2) {
        roll = frandmod(100);
        for (outcomeIndex = 0; outcomeIndex < 3; outcomeIndex++) {
            if (roll < difficulty2Cutoffs[outcomeIndex]) {
                break;
            }
        }
        return outcomeIndex + baseDelayFrames[difficulty];
    }
    return baseDelayFrames[difficulty] + frandmod(delayRangeFrames[difficulty]);
}

/* Raise or decay the player's response impulse from the current A/B input. */
void fn_1_745C(M638PlayerView *player)
{
    f32 amount;
    amount = 10.0f - player->impulse;
    if (amount < 1.0f) {
        amount = 1.0f;
    }
    player->impulse -= 0.125f;
    if (player->buttonB) {
        player->impulse *= 0.3f;
    } else if (player->buttonA) {
        player->impulse += amount;
    }
    if (player->impulse < 0.0f) {
        player->impulse = 0.0f;
    }
    if (player->impulse > 20.0f) {
        player->impulse = 20.0f;
    }
}

/* Copy a player's stored position and rotation to its model and object transforms. */
void fn_1_7554(void *playerContext)
{
    Hu3DModelPosSetV(((M638PlayerView *) playerContext)->model,
                     &((M638PlayerView *) playerContext)->pos);
    omSetTra(((M638PlayerView *)playerContext)->object, ((M638PlayerView *)playerContext)->pos.x,
             ((M638PlayerView *)playerContext)->pos.y, ((M638PlayerView *)playerContext)->pos.z);
    Hu3DModelRotSetV(((M638PlayerView *) playerContext)->model,
                     &((M638PlayerView *) playerContext)->rot);
    omSetRot(((M638PlayerView *)playerContext)->object, ((M638PlayerView *)playerContext)->rot.x,
             ((M638PlayerView *)playerContext)->rot.y, ((M638PlayerView *)playerContext)->rot.z);
}
