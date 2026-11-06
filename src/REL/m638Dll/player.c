/* Gondola Glide player setup, button responses, movement effects, and result exits. */
#include <math.h>
#include "REL/m638Dll.h"




u32 lbl_1_data_548[5] = {
    DATANUM(0x930000, 0), DATANUM(0x8e0000, 103),
    DATANUM(0x930000, 38), DATANUM(0x930000, 40),
    DATANUM(0x930000, 14)
};

/* Object initializer: create a player model, join its team, and install its update callback. */
void fn_1_67EC(OMOBJ *obj)
{
    s16 motion;
    s32 i;
    M638PlayerView *player;
    M638TeamView *teams;

    player = obj->data;
    player->object = obj;
    player->playerNo = (s16)obj->work[0];
    player->charNo = lbl_80265728[player->playerNo].charNo;
    player->group = lbl_80265728[player->playerNo].grpNo;
    player->padNo = lbl_80265728[player->playerNo].padNo;
    player->isCom = lbl_80265728[player->playerNo].type;
    player->difficulty = lbl_80265728[player->playerNo].comDif;

    *obj->mdlId = fn_80057068(player->charNo, 2);
    player->model = *obj->mdlId;
    for (i = 0; i < 5; i++) {
        motion = fn_8005B0B8(player->charNo, lbl_1_data_548[i]);
        player->motions[i] = motion;
        obj->mtnId[i] = motion;
    }
    fn_8005BE80(player->charNo, obj->mtnId[1]);
    fn_8005C430(player->charNo, ((1 << 30) | (1 << 1)));
    fn_8005C47C(player->charNo, 1U);
    fn_8005B4BC(player->charNo);
    fn_80026E44(player->model);

    teams = lbl_1_bss_4->data;
    player->member = (s16)(teams[player->group].players[0] != NULL);
    teams[player->group].players[player->member] = player;
    player->team = &teams[player->group];

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
    fn_80026790(player->model, &player->pos);
    omSetTra(player->object, player->pos.x, player->pos.y, player->pos.z);
    fn_80026838(player->model, &player->rot);
    omSetRot(player->object, player->rot.x, player->rot.y, player->rot.z);
    fn_80026B64(player->model, (u16)lbl_1_rodata_10[player->group]);
    fn_80026B8C(player->model, 4);
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
    fn_80026790(player->model, &player->pos);
    omSetTra(player->object, player->pos.x, player->pos.y, player->pos.z);
    fn_80026838(player->model, &player->rot);
    omSetRot(player->object, player->rot.x, player->rot.y, player->rot.z);
}

/* Advance the player's reaction animation and restore the idle pose when it ends. */
void fn_1_6C98(M638PlayerView *player)
{
    f32 motionSpeed;
    switch (player->state) {
    case 1:
        if (player->team->variant == 0) {
            HuAudFXPlay(1891);
        } else {
            HuAudFXPlay(1892);
        }
        fn_80026A44(player->model, ((1 << 30) | (1 << 1)));
        fn_80026A44(player->model, ((1 << 30) | (1 << 2)));
        fn_80026A44(player->attachedModel, ((1 << 30) | (1 << 1)));
        fn_80026A44(player->attachedModel, ((1 << 30) | (1 << 2)));
        motionSpeed = player->activity;
        if (motionSpeed < 0.25f) {
            motionSpeed = 0.25f;
        }
        if (motionSpeed > 2.0f) {
            motionSpeed = 2.0f;
        }
        fn_8002D86C(player->model, motionSpeed);
        fn_8002D86C(player->attachedModel, motionSpeed);
        player->state++;
        /* fallthrough */
    case 2:
        if (fn_8002D75C(player->model)) {
            fn_800269E8(player->model, ((1 << 30) | (1 << 2)));
            fn_800269E8(player->attachedModel, ((1 << 30) | (1 << 2)));
            player->state++;
        }
        break;
    case 0:
        break;
    case 3:
        if (fn_8002D75C(player->model)) {
            fn_800269E8(player->model, ((1 << 30) | (1 << 1)));
            fn_800269E8(player->attachedModel, ((1 << 30) | (1 << 1)));
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
        scaleY = 1.0 + (0.019f * sin((player->phase * pi) / 180.0f));
    }
    {
        f32 pi = acos(-1.0);
        scaleZ =
            1.009f + (0.009f * sin((2.0f * player->phase * pi) / 180.0f));
    }
    fn_800268B0(player->model, 1.0f, scaleY, scaleZ);
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
        fn_80026A44(player->model, ((1 << 30) | (1 << 2)));
        fn_800269E8(player->model, ((1 << 30) | (1 << 1)));
        fn_8002CCC0(player->model, player->motions[4], 0.0f, 10.0f, 0U);
        fn_8002D414(player->model, 0.0f);
        player->elapsed = 0;
        player->exitState++;
        break;
    case 1:
        player->elapsed++;
        exitProgress = player->elapsed / 58.0f;
        attachedModelScale = 1.0f - (0.5f * exitProgress);
        fn_800268B0(player->attachedModel, attachedModelScale, attachedModelScale,
                          attachedModelScale);
        player->rot.y = -180.0f * exitProgress;
        if (player->elapsed >= 58) {
            fn_8002CCC0(player->model, player->motions[0], 60.0f, 5.0f,
                ((1 << 30) | (1 << 0)));
            fn_800268B0(player->attachedModel, 0.5f, 0.5f, 0.5f);
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
s32 fn_1_71DC(M638PlayerView *player)
{
    u16 padButtons[2] = { PAD_BUTTON_A, PAD_BUTTON_B };
    s32 inputAccepted;

    inputAccepted = 0;
    if ((s32)(lbl_802C0008[player->padNo] &
              padButtons[player->team->button ^ 1]) != 0) {
        inputAccepted = 1;
        player->buttonB = 1;
        /* Rumble is suppressed during wipe transitions, when vibration is off, or for a computer
         * player. */
        fn_8006A030(player->playerNo, 20, 7, 3);
    } else if ((s32)(lbl_802C0008[player->padNo] &
                     padButtons[player->team->button]) != 0) {
        inputAccepted = 1;
        player->buttonA = 1;
    }
    return inputAccepted;
}

/* Computer input path used by the active player's object update; generate an A press after the
 * selected delay. */
s32 fn_1_72D0(M638PlayerView *player)
{
    s32 inputAccepted;

    inputAccepted = 0;
    player->count = (s32)(player->count + 1);
    if ((s32) player->count >= (s32) player->nextCount) {
        player->count = 0;
        player->nextCount = fn_1_7344((s32) player->difficulty);
        player->buttonA = 1;
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
void fn_1_7554(M638PlayerView *player)
{
    fn_80026790(player->model, &player->pos);
    omSetTra(player->object, player->pos.x, player->pos.y, player->pos.z);
    fn_80026838(player->model, &player->rot);
    omSetRot(player->object, player->rot.x, player->rot.y, player->rot.z);
}
