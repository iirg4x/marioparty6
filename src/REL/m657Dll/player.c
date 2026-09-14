#include "REL/m657Dll.h"
#include "game/charman.h"

OMOBJ *lbl_1_bss_30[2];
static s32 lbl_1_data_D8[10] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

void fn_1_21A8(void)
{
    CharModelKill(-1);
}

void fn_1_2694(OMOBJ *obj, BOOL visible)
{
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    HU3D_MODELID center = obj->mdlId[4];

    if (visible) {
        Hu3DModelAttrSet(left, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(right, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrReset(center, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(left, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(right, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(center, HU3D_ATTR_DISPOFF);
    }
}

void fn_1_2840(OMOBJ *obj, s16 motionIndex)
{
    M657Player *work = obj->data;
    s16 charNo = work->player.charNo;
    HU3D_MODELID model = obj->mdlId[0];
    u32 attr = 0;

    if (lbl_1_data_D8[motionIndex]) {
        attr = HU3D_MOTATTR_LOOP;
    }
    if (work->player.motionIndex != motionIndex) {
        CharMotionShiftSet(charNo, obj->mtnId[motionIndex], 15.0f, 16.0f, attr);
    } else {
        CharMotionSet(charNo, obj->mtnId[motionIndex]);
    }
    switch (motionIndex) {
    case 0:
        Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
        break;
    default:
        Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
        break;
    }
    work->player.motionIndex = motionIndex;
}

void fn_1_294C(OMOBJ *obj)
{
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MOTIONID motion = 0;
    u32 attr = HU3D_MOTATTR_LOOP;
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    HU3D_MODELID center = obj->mdlId[4];

    if (work->buttons != work->prevButtons) {
        if (work->buttons & PAD_BUTTON_A) {
            motion = obj->mtnId[4];
            Hu3DModelAttrReset(left, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(right, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(center, HU3D_ATTR_DISPOFF);
        } else {
            motion = obj->mtnId[5];
            Hu3DModelAttrSet(left, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(right, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrReset(center, HU3D_ATTR_DISPOFF);
        }
        Hu3DMotionShiftSet(model, motion, 0.0f, 8.0f, attr);
    }
}

void fn_1_2A50(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MOTIONID motion = obj->mtnId[3];

    Hu3DMotionShiftSet(model, motion, 0.0f, 8.0f, 0);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[3], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrSet(obj->mdlId[4], HU3D_ATTR_DISPOFF);
}

s32 fn_1_2AF0(OMOBJ *obj)
{
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[1];

    if (Hu3DMotionEndCheck(model)) {
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
        work->exitFinished = TRUE;
        return TRUE;
    }
    return FALSE;
}

s32 fn_1_2B64(s16 team)
{
    M657Player *work = lbl_1_bss_30[team]->data;

    return work->exitFinished;
}

void fn_1_2B98(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[5];

    Hu3DMotionSpeedSet(model, 1.0f);
}

void fn_1_2BE4(s16 team, s32 state)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;

    work->player.winner = state;
}

void fn_1_2C20(s32 state)
{
    OMOBJ *obj = NULL;
    int motion;
    M657Player *work = NULL;
    M657PlayerView *player = NULL;
    int i = 0;

    for (i = 0; i < 2; i++) {
        obj = lbl_1_bss_30[i];
        work = obj->data;
        motion = 2;
        player = &work->player;
        if (work->player.winner) {
            motion = 1;
            CharFXPlay(work->player.charNo, 577);
            fn_1_2840(obj, motion);
        } else if (state == 0) {
            fn_1_2840(obj, motion);
        }
    }
}

s32 fn_1_2E78(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[0];

    if (Hu3DMotionEndCheck(model)) {
        return TRUE;
    }
    return FALSE;
}

s32 fn_1_2EEC(s16 team)
{
    M657PlayerView *player;
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;

    player = &work->player;
    if (player->state == 0) {
        return TRUE;
    }
    return player->falling;
}

float *fn_1_2F48(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;

    return &player->progress;
}

void fn_1_306C(s16 team, HuVecF pos)
{
    M657PlayerView *player;
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;

    player = &work->player;
    player->pos = pos;
    omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
}

void fn_1_30FC(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657PlayerView *player;
    M657Player *work = obj->data;

    player = &work->player;
    player->state = 0;
    obj->objFunc = fn_1_3B50;
}

s32 fn_1_3154(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657PlayerView *player;
    M657Player *work = obj->data;

    player = &work->player;
    return player->ready;
}

s32 fn_1_319C(OMOBJ *obj)
{
    M657Player *work = obj->data;

    if (work->inputState < 2) {
        return TRUE;
    }
    return FALSE;
}
