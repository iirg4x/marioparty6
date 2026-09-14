#include "REL/m657Dll.h"
#include "game/charman.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "game/frand.h"
#include "string.h"
#include "math.h"

typedef struct {
    float fov;
    float near;
    float far;
    HuVecF pos;
    HuVecF target;
    HuVecF up;
    float opacity;
    u16 size;
    u8 r;
    u8 g;
    u8 b;
} M657Shadow;

OMOBJ *lbl_1_bss_30[2];
static s32 lbl_1_data_D8[10] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int lbl_1_data_100[10] = {
    DATANUM(DATA_mario, 161), DATANUM(DATA_mario, 10), DATANUM(DATA_mario, 11),
    0, 0, 0, 0, 0, 0, 0
};
static int lbl_1_data_128[3] = {
    DATANUM(DATA_m657, 12), DATANUM(DATA_m657, 11), DATANUM(DATA_m657, 10)
};
static M657Shadow lbl_1_data_134 = {
    30.0f, 1.0f, 13000.0f,
    { 0.0f, 10000.0f, 0.0f },
    { 0.0f, -100.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    0.7f, 240, 25, 25, 25
};
static HU3D_MOTIONID lbl_1_data_170[8] = { -1, -1, -1, -1, -1, -1, -1, -1 };

void fn_1_2018(OMOBJMAN *objman)
{
    OMOBJ *obj = NULL;
    M657Player *work = NULL;
    int i = 0;
    int team = 0;
    M657Shadow *shadow = &lbl_1_data_134;

    Hu3DShadowMultiCreate(shadow->fov, shadow->near, shadow->far, 3);
    Hu3DShadowMultiPosSet(&shadow->pos, &shadow->up, &shadow->target, 3);
    Hu3DShadowMultiTPLvlSet(shadow->opacity, 3);
    Hu3DShadowMultiSizeSet(shadow->size, 3);
    Hu3DShadowMultiColSet(shadow->r, shadow->g, shadow->b, 3);
    for (i = 0; i < GW_PLAYER_MAX; i++) {
        team = GwPlayerConf[i].grpNo;
        if (team == 0 || team == 1) {
            obj = lbl_1_bss_30[team] = omAddObjEx(objman, 110, 6, 10, -1, fn_1_31CC);
            work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657Player), HU_MEMNUM_OVL);
            obj->data = work;
            memset(work, 0, sizeof(M657Player));
            work->player.playerNo = i;
            work->player.team = team;
            work->sound = -1;
            work->computer = GwPlayerConf[i].type;
            work->difficulty = -1;
            if (work->computer) {
                work->difficulty = GwPlayerConf[i].comDif;
            }
        }
    }
}

void fn_1_21A8(void)
{
    CharModelKill(-1);
}

void fn_1_21CC(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    M657Shadow *shadow = &lbl_1_data_134;
    u16 cameraMasks[2] = { 1, 2 };
    int team = GwPlayerConf[player->playerNo].grpNo;
    HU3D_MODELID model;
    s16 i;
    s16 charNo;

    player->cameraMask = cameraMasks[player->team];
    OSReport("grp : %d ( %d )\n", player->playerNo, player->cameraMask);
    charNo = GwPlayerConf[player->playerNo].charNo;
    player->charNo = charNo;
    player->falling = FALSE;
    player->winner = FALSE;
    model = obj->mdlId[0] = CharModelCreate(charNo, 2);
    for (i = 0; i < 10; i++) {
        obj->mtnId[i] = CharMotionCreate(charNo, lbl_1_data_100[i]);
    }
    Hu3DModelCameraSet(model, player->cameraMask);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 4);
    Hu3DModelShadowSet(model);
    CharMotionDataClose(charNo);
    fn_1_2840(obj, 0);
    work->padNo = GwPlayerConf[player->playerNo].padNo;
    work->stickX = work->stickY = 0.0f;
    player->pos.x = player->pos.y = player->pos.z = 0.0f;
    player->rot.x = player->rot.y = player->rot.z = 0.0f;
    player->pos.x = player->pos.z = 0.0f;
    player->pos.y = 2573.0f;
    omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
    omSetRot(obj, player->rot.x, player->rot.y, player->rot.z);
}

void fn_1_23F4(OMOBJ *obj)
{
    M657Player *work = obj->data;
    s16 i;
    HU3D_MODELID body;
    HU3D_MODELID left;
    HU3D_MODELID right;
    HU3D_MODELID center;
    HU3D_MOTIONID motion;
    char *hook;

    hook = NULL;
    i = 0;
    body = obj->mdlId[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 9), HU_MEMNUM_OVL, HEAP_MODEL));
    for (i = 0; i < 3; i++) {
        motion = obj->mtnId[i+3] = Hu3DJointMotion(body,
            HuDataSelHeapReadNum(lbl_1_data_128[i], HU_MEMNUM_OVL, HEAP_MODEL));
        (&lbl_1_data_170[1])[i] = motion;
    }
    Hu3DModelShadowSet(body);
    Hu3DMotionSet(body, motion);
    Hu3DMotionSpeedSet(body, 1.0f);
    Hu3DModelAttrSet(body, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(body, 7);
    hook = CharModelItemHookGet(work->player.charNo, 2, 4);
    Hu3DModelHookSet(obj->mdlId[0], hook, body);
    left = obj->mdlId[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 13), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(left, 1.0f);
    Hu3DModelAttrSet(left, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(left, HU3D_ATTR_DISPOFF);
    Hu3DModelLayerSet(left, 7);
    Hu3DModelHookSet(body, "rocket00-L_itemhook", left);
    right = obj->mdlId[3] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 13), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(right, 1.0f);
    Hu3DModelAttrSet(right, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(right, HU3D_ATTR_DISPOFF);
    Hu3DModelLayerSet(right, 7);
    Hu3DModelHookSet(body, "rocket00-R_itemhook", right);
    center = obj->mdlId[4] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 14), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(center, 1.0f);
    Hu3DModelAttrSet(center, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrSet(center, HU3D_ATTR_DISPOFF);
    Hu3DModelLayerSet(center, 7);
    Hu3DModelHookSet(body, "rocket-Cs_itemhook", center);
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

void fn_1_2748(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;

    model = obj->mdlId[5] = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 4), HU_MEMNUM_OVL, HEAP_MODEL));
    motion = obj->mtnId[9] = Hu3DJointMotion(model,
        HuDataSelHeapReadNum(DATANUM(DATA_m657, 5), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, 0.0f);
    Hu3DModelLayerSet(model, 1);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelShadowMapSet(model);
    Hu3DModelCameraSet(model, player->cameraMask);
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

void fn_1_2F90(s16 team, u16 cameraMask)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HU3D_MODELID model = obj->mdlId[0];
    M657Shadow *shadow;

    player->cameraMask = cameraMask;
    Hu3DModelCameraSet(model, player->cameraMask);
    Hu3DModelShadowSet(model);
    shadow = &lbl_1_data_134;
    Hu3DShadowMultiCreate(shadow->fov, shadow->near, shadow->far, player->cameraMask);
    Hu3DShadowMultiPosSet(&shadow->pos, &shadow->up, &shadow->target, player->cameraMask);
    Hu3DShadowMultiTPLvlSet(shadow->opacity, player->cameraMask);
    Hu3DShadowMultiSizeSet(shadow->size, player->cameraMask);
    Hu3DShadowMultiColSet(shadow->r, shadow->g, shadow->b, player->cameraMask);
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

s32 fn_1_3154(s32 team)
{
    OMOBJ *obj = lbl_1_bss_30[(s16)team];
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

void fn_1_31CC(OMOBJ *obj)
{
    M657Player *work = obj->data;

    fn_1_21CC(obj);
    fn_1_23F4(obj);
    fn_1_2748(obj);
    fn_1_47EC(obj);
    work->player.ready = FALSE;
    obj->objFunc = fn_1_32F4;
}

void fn_1_32F4(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    float speed = 6.0f;

    switch (player->state) {
    case 0:
        if (MgSeqModeGet() >= MGSEQ_MODE_FADEIN) {
            fn_1_1AD4();
            fn_1_2694(obj, TRUE);
            player->state++;
        }
        break;
    case 1:
        if (fn_1_1B20()) {
            player->state++;
        }
        break;
    case 2:
        player->pos.y -= speed;
        omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
        if (player->pos.y <= 2373.0f) {
            player->ready = TRUE;
            player->state = 0;
            obj->objFunc = fn_1_3468;
        }
        break;
    }
    player->progress = (2573.0f-player->pos.y)/2573.0f;
}

void fn_1_3468(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    M657Shadow *shadow;
    float speed = 6.0f;
    HuVecF shadowPos = player->pos;

    shadow = &lbl_1_data_134;
    shadowPos.y = 2500.0f + 3.8f*shadowPos.y;
    Hu3DShadowMultiPosSet(&shadowPos, &shadow->up, &shadow->target, player->cameraMask);
    switch (player->state) {
    case 0:
        if (MgSeqModeGet() < MGSEQ_MODE_MAIN) {
            player->pos.y -= speed;
            omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
        } else {
            fn_1_2694(obj, TRUE);
            player->falling = TRUE;
            player->state++;
        }
        break;
    case 1:
        if (MgSeqModeGet() > MGSEQ_MODE_MAIN) {
            player->state = 4;
        } else {
            work->prevButtons = work->buttons;
            if (work->computer) {
                work->buttons = 0;
                if (fn_1_319C(obj)) {
                    work->buttons |= PAD_BUTTON_A;
                }
            } else {
                work->buttons = HuPadBtn[work->padNo] & PAD_BUTTON_A;
            }
            fn_1_294C(obj);
            if (work->buttons & PAD_BUTTON_A) {
                int sounds[2] = { 2085, 2086 };

                if (work->prevButtons != work->buttons) {
                    work->sound = HuAudFXPlayPan(sounds[player->team], 48+32*player->team);
                }
                speed = 1.0f;
            } else if (work->sound != -1) {
                HuAudFXStop(work->sound);
                work->sound = -1;
            }
        }
        break;
    case 2:
        work->buttons = 0;
        fn_1_294C(obj);
        fn_1_2694(obj, FALSE);
        player->state++;
        break;
    case 3:
        if (MgSeqModeGet() > MGSEQ_MODE_MAIN) {
            player->state = 4;
        }
        break;
    case 4:
        fn_1_2A50(obj);
        player->state++;
        break;
    case 5:
        if (fn_1_2AF0(obj)) {
            work->player.state++;
        }
        break;
    case 6:
        if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
            work->player.state++;
        }
        break;
    }
    if (player->falling) {
        player->pos.y -= speed;
        if (player->pos.y <= 0.0f) {
            int sounds[2] = { 2089, 2090 };

            HuAudFXPlayPan(sounds[player->team], 48+32*player->team);
            if (work->sound != -1) {
                HuAudFXStop(work->sound);
                work->sound = -1;
            }
            player->pos.y = 0.0f;
            omVibrate(player->playerNo, 20, 7, 3);
            player->falling = FALSE;
            fn_1_3EA8(player->team);
            fn_1_2B98(obj);
            if (player->state != 5) {
                player->state++;
            }
        }
        omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
    }
    player->progress = (2573.0f-player->pos.y)/2573.0f;
}

void fn_1_3B50(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HuVecF target;
    int height;
    int halfHeight;

    switch (player->state) {
    case 0:
        if ((frand() & 1) == 0) {
            player->pos.x = 1000.0f;
            target.x = -1000.0f;
        } else {
            player->pos.x = -1000.0f;
            target.x = 1000.0f;
        }
        target.z = player->pos.z = -1000.0f;
        height = 550;
        halfHeight = height/2;
        if ((frand() & 1) == 0) {
            player->pos.y = frand()%halfHeight;
            target.y = halfHeight + frand()%height;
        } else {
            player->pos.y = halfHeight + frand()%height;
            target.y = frand()%halfHeight;
        }
        PSVECSubtract(&target, &work->player.pos, &player->vel);
        PSVECNormalize(&player->vel, &player->vel);
        player->rot.z = 180.0*(atan2(player->vel.y, player->vel.x)/M_PI)-90.0;
        player->state++;
        /* fallthrough */
    case 1:
        if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
            player->state++;
        }
        break;
    case 2:
        player->pos.x += 10.0f*player->vel.x;
        player->pos.y += 10.0f*player->vel.y;
        player->rot.y += 10.0f;
        break;
    }
    omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
    omSetRot(obj, player->rot.x, player->rot.y, player->rot.z);
}
