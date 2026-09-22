#include "REL/m638Dll.h"

u32 lbl_1_data_548[5] = { 9633792, 9306215, 9633830, 9633832, 9633806 };

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
    CharModelAttrSet(player->charNo, 1073741826U);
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

void fn_1_6C98(M638PlayerView *player)
{
    f32 speed;
    switch (player->state) {
    case 1:
        if (player->team->variant == 0) {
            HuAudFXPlay(1891);
        } else {
            HuAudFXPlay(1892);
        }
        Hu3DModelAttrReset(player->model, 1073741826U);
        Hu3DModelAttrReset(player->model, 1073741828U);
        Hu3DModelAttrReset(player->attachedModel, 1073741826U);
        Hu3DModelAttrReset(player->attachedModel, 1073741828U);
        speed = player->activity;
        if (speed < 0.25f) {
            speed = 0.25f;
        }
        if (speed > 2.0f) {
            speed = 2.0f;
        }
        Hu3DMotionSpeedSet(player->model, speed);
        Hu3DMotionSpeedSet(player->attachedModel, speed);
        player->state++;
        /* fallthrough */
    case 2:
        if (Hu3DMotionEndCheck(player->model)) {
            Hu3DModelAttrSet(player->model, 1073741828U);
            Hu3DModelAttrSet(player->attachedModel, 1073741828U);
            player->state++;
        }
        break;
    case 0:
        break;
    case 3:
        if (Hu3DMotionEndCheck(player->model)) {
            Hu3DModelAttrSet(player->model, 1073741826U);
            Hu3DModelAttrSet(player->attachedModel, 1073741826U);
            player->state = 0;
        }
        break;
    }
}

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
        scaleZ = 1.0089999437332153 + (0.008999999612569809 * sin((2.0f * player->phase * pi) / 180.0f));
    }
    Hu3DModelScaleSet(player->model, 1.0f, scaleY, scaleZ);
    omSetSca(player->object, 1.0f, scaleY, scaleZ);
}

s32 fn_1_6FE4(void *data)
{
    M638PlayerView *player;
    f32 time;
    f32 scale;
    int done;
    done = 0;
    player = data;
    switch (player->exitState) {
    case 0:
        Hu3DModelAttrReset(player->model, 1073741828U);
        Hu3DModelAttrSet(player->model, 1073741826U);
        Hu3DMotionShiftSet(player->model, player->motions[4], 0.0f, 10.0f, 0U);
        Hu3DMotionTimeSet(player->model, 0.0f);
        player->elapsed = 0;
        player->exitState++;
        break;
    case 1:
        player->elapsed++;
        time = player->elapsed / 58.0f;
        scale = 1.0f - (0.5f * time);
        Hu3DModelScaleSet(player->attachedModel, scale, scale, scale);
        player->rot.y = -180.0f * time;
        if (player->elapsed >= 58) {
            Hu3DMotionShiftSet(player->model, player->motions[0], 60.0f, 5.0f, 1073741825U);
            Hu3DModelScaleSet(player->attachedModel, 0.5f, 0.5f, 0.5f);
            player->rot.y = -180.0f;
            player->exitState = 2;
            done = 1;
        }
        break;
    }
    return done;
}

s32 fn_1_71DC(void *arg0)
{
    u16 button_masks[2] = { PAD_BUTTON_A, PAD_BUTTON_B };
    s32 var_r30;

    var_r30 = 0;
    if ((s32)(HuPadBtnDown[((M638PlayerView *)arg0)->padNo] &
              button_masks[((M638PlayerView *)arg0)->team->button ^ 1]) != 0) {
        var_r30 = 1;
        ((M638PlayerView *)arg0)->buttonB = 1;
        omVibrate(((M638PlayerView *)arg0)->playerNo, 20, 7, 3);
    } else if ((s32)(HuPadBtnDown[((M638PlayerView *)arg0)->padNo] &
                     button_masks[((M638PlayerView *)arg0)->team->button]) != 0) {
        var_r30 = 1;
        ((M638PlayerView *)arg0)->buttonA = 1;
    }
    return var_r30;
}

s32 fn_1_72D0(void *arg0)
{
    s32 var_r30;

    var_r30 = 0;
    ((M638PlayerView *)arg0)->count = (s32)(((M638PlayerView *)arg0)->count + 1);
    if ((s32)((M638PlayerView *)arg0)->count >= (s32)((M638PlayerView *)arg0)->nextCount) {
        ((M638PlayerView *)arg0)->count = 0;
        ((M638PlayerView *)arg0)->nextCount = fn_1_7344((s32)((M638PlayerView *)arg0)->difficulty);
        ((M638PlayerView *)arg0)->buttonA = 1;
        var_r30 = 1;
    }
    return var_r30;
}

u32 fn_1_7344(s32 difficulty)
{
    int base[4] = { 30, 12, 7, 7 };
    int range[4] = { 30, 8, 3, 2 };
    int cumulative[3] = { 33, 90, 100 };
    int random;
    int i;
    if (difficulty == 2) {
        random = frandmod(100);
        for (i = 0; i < 3; i++) {
            if (random < cumulative[i]) {
                break;
            }
        }
        return i + base[difficulty];
    }
    return base[difficulty] + frandmod(range[difficulty]);
}

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

void fn_1_7554(void *arg0)
{
    Hu3DModelPosSetV(((M638PlayerView *)arg0)->model, &((M638PlayerView *)arg0)->pos);
    omSetTra(((M638PlayerView *)arg0)->object, ((M638PlayerView *)arg0)->pos.x,
             ((M638PlayerView *)arg0)->pos.y, ((M638PlayerView *)arg0)->pos.z);
    Hu3DModelRotSetV(((M638PlayerView *)arg0)->model, &((M638PlayerView *)arg0)->rot);
    omSetRot(((M638PlayerView *)arg0)->object, ((M638PlayerView *)arg0)->rot.x,
             ((M638PlayerView *)arg0)->rot.y, ((M638PlayerView *)arg0)->rot.z);
}
