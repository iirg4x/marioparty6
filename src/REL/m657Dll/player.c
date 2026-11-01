/* Creates and updates each team's character, equipment, and fall animation. */
#include "REL/m657Dll.h"
#include "game/charman.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "game/frand.h"
#include "string.h"
#include "math.h"

#define M657_SFX_TEAM0_ACTION_PRESS 2085
#define M657_SFX_TEAM1_ACTION_PRESS 2086
#define M657_SFX_TEAM0_LANDING 2089
#define M657_SFX_TEAM1_LANDING 2090

typedef struct {
    /* Shadow camera field of view in degrees. */
    float fov;
    /* Near clipping distance in world units. */
    float near;
    /* Far clipping distance in world units. */
    float far;
    /* Shadow camera position in world units. */
    HuVecF pos;
    /* Point the shadow camera looks toward, in world units. */
    HuVecF target;
    /* Shadow camera up direction. */
    HuVecF up;
    /* Shadow texture opacity from zero to one. */
    float opacity;
    /* Shadow texture size in pixels. */
    u16 size;
    /* Red channel of the shadow tint. */
    u8 r;
    /* Green channel of the shadow tint. */
    u8 g;
    /* Blue channel of the shadow tint. */
    u8 b;
} M657Shadow;

OMOBJ *lbl_1_bss_30[2];
static s32 lbl_1_data_D8[10] = { 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
static int lbl_1_data_100[10] = {
    DATANUM(0x8e0000, 161), DATANUM(0x8e0000, 10), DATANUM(0x8e0000, 11),
    0, 0, 0, 0, 0, 0, 0
};
static int lbl_1_data_128[3] = {
    DATANUM(0x740000, 12), DATANUM(0x740000, 11), DATANUM(0x740000, 10)
};
static M657Shadow lbl_1_data_134 = {
    30.0f, 1.0f, 13000.0f,
    { 0.0f, 10000.0f, 0.0f },
    { 0.0f, -100.0f, 0.0f },
    { 0.0f, 0.0f, 1.0f },
    0.7f, 240, 25, 25, 25
};
/* Target data has an unreferenced -1 halfword before the motion-table base.
 * Its original purpose is unknown; the live table starts at .data+0x172. */
static s16 lbl_1_data_170 = -1;
static HU3D_MOTIONID lbl_1_data_172[7] = { -1, -1, -1, -1, -1, -1, -1 };

/* Creates the player object for each configured team; called during minigame setup. */
void fn_1_2018(OMOBJMAN *objman)
{
    OMOBJ *obj = NULL;
    M657Player *work = NULL;
    int i = 0;
    int team = 0;
    M657Shadow *shadow = &lbl_1_data_134;

    fn_8002A3B0(shadow->fov, shadow->near, shadow->far, 3);
    fn_8002A674(&shadow->pos, &shadow->up, &shadow->target, 3);
    fn_8002A7D8(shadow->opacity, 3);
    fn_8002A8CC(shadow->size, 3);
    fn_8002AA3C(shadow->r, shadow->g, shadow->b, 3);
    for (i = 0; i < 4; i++) {
        team = lbl_80265728[i].grpNo;
        if (team == 0 || team == 1) {
            obj = lbl_1_bss_30[team] = fn_8003AB38(objman, 110, 6, 10, -1, fn_1_31CC);
            work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657Player), 0x10000000);
            obj->data = work;
            memset(work, 0, sizeof(M657Player));
            work->player.playerNo = i;
            work->player.team = team;
            work->sound = -1;
            work->computer = lbl_80265728[i].type;
            work->difficulty = -1;
            if (work->computer) {
                work->difficulty = lbl_80265728[i].comDif;
            }
        }
    }
}

void fn_1_21A8(void)
{
    fn_8005BB8C(-1);
}

/* Called from the player object's create callback to load its character and start pose. */
void fn_1_21CC(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    M657Shadow *shadow = &lbl_1_data_134;
    u16 cameraMasks[2] = { 1, 2 };
    int team = lbl_80265728[player->playerNo].grpNo;
    HU3D_MODELID model;
    s16 i;
    s16 charNo;

    player->cameraMask = cameraMasks[player->team];
    OSReport("grp : %d ( %d )\n", player->playerNo, player->cameraMask);
    charNo = lbl_80265728[player->playerNo].charNo;
    player->charNo = charNo;
    player->falling = FALSE;
    player->winner = FALSE;
    model = obj->mdlId[0] = fn_80057068(charNo, 2);
    for (i = 0; i < 10; i++) {
        obj->mtnId[i] = fn_8005B0B8(charNo, lbl_1_data_100[i]);
    }
    fn_80026B64(model, player->cameraMask);
    fn_800269E8(model, ((1 << 30) | (1 << 0)));
    fn_80026B8C(model, 4);
    fn_80026E44(model);
    fn_8005B4BC(charNo);
    fn_1_2840(obj, 0);
    work->padNo = lbl_80265728[player->playerNo].padNo;
    work->stickX = work->stickY = 0.0f;
    player->pos.x = player->pos.y = player->pos.z = 0.0f;
    player->rot.x = player->rot.y = player->rot.z = 0.0f;
    player->pos.x = player->pos.z = 0.0f;
    /* Every entrant starts at the same fixed height, regardless of prior position. */
    player->pos.y = 2573.0f;
    omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
    omSetRot(obj, player->rot.x, player->rot.y, player->rot.z);
}

/* Called during player-object creation to attach the rocket body and its left, right, and center
 * parts. */
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
    body = obj->mdlId[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 9), 0x10000000, 2));
    for (i = 0; i < 3; i++) {
        motion = obj->mtnId[i+3] = fn_80030458(body,
            fn_80008DD0(lbl_1_data_128[i], 0x10000000, 2));
        lbl_1_data_172[i] = motion;
    }
    fn_80026E44(body);
    fn_8002C95C(body, motion);
    fn_8002D86C(body, 1.0f);
    fn_800269E8(body, ((1 << 30) | (1 << 0)));
    fn_80026B8C(body, 7);
    hook = fn_8005C0F0(work->player.charNo, 2, 4);
    fn_8002741C(obj->mdlId[0], hook, body);
    left = obj->mdlId[2] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 13), 0x10000000, 2));
    fn_8002D86C(left, 1.0f);
    fn_800269E8(left, ((1 << 30) | (1 << 0)));
    fn_800269E8(left, (1 << 0));
    fn_80026B8C(left, 7);
    fn_8002741C(body, "rocket00-L_itemhook", left);
    right = obj->mdlId[3] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 13), 0x10000000, 2));
    fn_8002D86C(right, 1.0f);
    fn_800269E8(right, ((1 << 30) | (1 << 0)));
    fn_800269E8(right, (1 << 0));
    fn_80026B8C(right, 7);
    fn_8002741C(body, "rocket00-R_itemhook", right);
    center = obj->mdlId[4] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 14), 0x10000000, 2));
    fn_8002D86C(center, 1.0f);
    fn_800269E8(center, ((1 << 30) | (1 << 0)));
    fn_800269E8(center, (1 << 0));
    fn_80026B8C(center, 7);
    fn_8002741C(body, "rocket-Cs_itemhook", center);
}

/* Called by the player state updates to show the center part or hide every rocket part. */
void fn_1_2694(OMOBJ *obj, BOOL visible)
{
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    HU3D_MODELID center = obj->mdlId[4];

    if (visible) {
        fn_800269E8(left, (1 << 0));
        fn_800269E8(right, (1 << 0));
        fn_80026A44(center, (1 << 0));
    } else {
        fn_800269E8(left, (1 << 0));
        fn_800269E8(right, (1 << 0));
        fn_800269E8(center, (1 << 0));
    }
}

/* Called during player-object creation to add the attached effect model and stopped motion. */
void fn_1_2748(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;

    model = obj->mdlId[5] = fn_80025A04(
        fn_80008DD0(DATANUM(0x740000, 4), 0x10000000, 2));
    motion = obj->mtnId[9] = fn_80030458(model,
        fn_80008DD0(DATANUM(0x740000, 5), 0x10000000, 2));
    fn_8002C95C(model, motion);
    fn_8002D86C(model, 0.0f);
    fn_80026B8C(model, 1);
    fn_80026760(model, 0.0f, 0.0f, 0.0f);
    fn_80026FFC(model);
    fn_80026B64(model, player->cameraMask);
}

/* Selects the requested character motion-table entry and records its index. */
void fn_1_2840(OMOBJ *obj, s16 motionIndex)
{
    M657Player *work = obj->data;
    s16 charNo = work->player.charNo;
    HU3D_MODELID model = obj->mdlId[0];
    u32 attr = 0;

    if (lbl_1_data_D8[motionIndex]) {
        attr = ((1 << 30) | (1 << 0));
    }
    if (work->player.motionIndex != motionIndex) {
        fn_8005C2C8(charNo, obj->mtnId[motionIndex], 15.0f, 16.0f, attr);
    } else {
        fn_8005BE80(charNo, obj->mtnId[motionIndex]);
    }
    switch (motionIndex) {
    case 0:
        fn_800269E8(model, ((1 << 30) | (1 << 0)));
        break;
    default:
        fn_80026A44(model, ((1 << 30) | (1 << 0)));
        break;
    }
    work->player.motionIndex = motionIndex;
}

/* Called each active player update to change the rocket animation when the action button
 * changes. */
void fn_1_294C(OMOBJ *obj)
{
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MOTIONID motion = 0;
    u32 attr = ((1 << 30) | (1 << 0));
    HU3D_MODELID left = obj->mdlId[2];
    HU3D_MODELID right = obj->mdlId[3];
    HU3D_MODELID center = obj->mdlId[4];

    if (work->buttons != work->prevButtons) {
        if (work->buttons & PAD_BUTTON_A) {
            motion = obj->mtnId[4];
            fn_80026A44(left, (1 << 0));
            fn_80026A44(right, (1 << 0));
            fn_800269E8(center, (1 << 0));
        } else {
            motion = obj->mtnId[5];
            fn_800269E8(left, (1 << 0));
            fn_800269E8(right, (1 << 0));
            fn_80026A44(center, (1 << 0));
        }
        fn_8002CCC0(model, motion, 0.0f, 8.0f, attr);
    }
}

/* Starts the rocket's exit motion and hides its attached parts during the exit state. */
void fn_1_2A50(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[1];
    HU3D_MOTIONID motion = obj->mtnId[3];

    fn_8002CCC0(model, motion, 0.0f, 8.0f, 0);
    fn_800269E8(obj->mdlId[2], (1 << 0));
    fn_800269E8(obj->mdlId[3], (1 << 0));
    fn_800269E8(obj->mdlId[4], (1 << 0));
}

/* Polled by the player update until the exit motion ends, then marks it complete. */
s32 fn_1_2AF0(OMOBJ *obj)
{
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[1];

    if (fn_8002D75C(model)) {
        fn_800269E8(model, (1 << 0));
        work->exitFinished = TRUE;
        return TRUE;
    }
    return FALSE;
}

/* Reports whether the team's rocket exit animation has finished. */
s32 fn_1_2B64(s16 team)
{
    M657Player *work = lbl_1_bss_30[team]->data;

    return work->exitFinished;
}

/* Starts the attached player effect motion after the player lands. */
void fn_1_2B98(OMOBJ *obj)
{
    HU3D_MODELID model = obj->mdlId[5];

    fn_8002D86C(model, 1.0f);
}

/* Sets the team's winner flag for the result sequence. */
void fn_1_2BE4(s16 team, s32 state)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;

    work->player.winner = state;
}

/* Sets winners to motion 1 and plays effect 577; sets non-winners to motion 2 only when state is
 * zero. */
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

/* Reports whether the team's character motion has reached its end. */
s32 fn_1_2E78(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    HU3D_MODELID model = obj->mdlId[0];

    if (fn_8002D75C(model)) {
        return TRUE;
    }
    return FALSE;
}

/* Returns true for player state zero, otherwise reports whether the player is falling. */
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

/* Returns the team's normalized descent progress for its following camera. */
float *fn_1_2F48(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;

    return &player->progress;
}

/* Reassigns the player's model and shadow configuration to the specified camera mask. */
void fn_1_2F90(s16 team, u16 cameraMask)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HU3D_MODELID model = obj->mdlId[0];
    M657Shadow *shadow;

    player->cameraMask = cameraMask;
    fn_80026B64(model, player->cameraMask);
    fn_80026E44(model);
    shadow = &lbl_1_data_134;
    fn_8002A3B0(shadow->fov, shadow->near, shadow->far, player->cameraMask);
    fn_8002A674(&shadow->pos, &shadow->up, &shadow->target, player->cameraMask);
    fn_8002A7D8(shadow->opacity, player->cameraMask);
    fn_8002A8CC(shadow->size, player->cameraMask);
    fn_8002AA3C(shadow->r, shadow->g, shadow->b, player->cameraMask);
}

/* Places the team's character at a world-space position. */
void fn_1_306C(s16 team, HuVecF pos)
{
    M657PlayerView *player;
    OMOBJ *obj = lbl_1_bss_30[team];
    M657Player *work = obj->data;

    player = &work->player;
    player->pos = pos;
    omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
}

/* Resets the player's state and switches its object callback to the winner fly-by. */
void fn_1_30FC(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657PlayerView *player;
    M657Player *work = obj->data;

    player = &work->player;
    player->state = 0;
    obj->objFunc = fn_1_3B50;
}

/* Returns whether the team's player has reached the arena start. */
s32 fn_1_3154(s32 team)
{
    OMOBJ *obj = lbl_1_bss_30[(s16)team];
    M657PlayerView *player;
    M657Player *work = obj->data;

    player = &work->player;
    return player->ready;
}

/* Reports whether the computer's A-button hold phase is active. */
s32 fn_1_319C(OMOBJ *obj)
{
    M657Player *work = obj->data;

    if (work->inputState < 2) {
        return TRUE;
    }
    return FALSE;
}

/* Object-create callback that builds player models and installs the active update. */
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

/* Per-frame callback installed after player creation; runs the entrance state machine. */
void fn_1_32F4(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    float speed = 6.0f;

    switch (player->state) {
    case 0:
        if (fn_80071A8C() >= 2) {
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

/* Per-frame callback installed after the entrance; handles input, falling, and result states. */
void fn_1_3468(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    M657Shadow *shadow;
    float speed = 6.0f;
    HuVecF shadowPos = player->pos;

    shadow = &lbl_1_data_134;
    shadowPos.y = 2500.0f + 3.8f*shadowPos.y;
    fn_8002A674(&shadowPos, &shadow->up, &shadow->target, player->cameraMask);
    switch (player->state) {
    case 0:
        if (fn_80071A8C() < 5) {
            player->pos.y -= speed;
            omSetTra(obj, player->pos.x, player->pos.y, player->pos.z);
        } else {
            fn_1_2694(obj, TRUE);
            player->falling = TRUE;
            player->state++;
        }
        break;
    case 1:
        if (fn_80071A8C() > 5) {
            player->state = 4;
        } else {
            work->prevButtons = work->buttons;
            if (work->computer) {
                work->buttons = 0;
                if (fn_1_319C(obj)) {
                    work->buttons |= PAD_BUTTON_A;
                }
            } else {
                work->buttons = lbl_802C0010[work->padNo] & PAD_BUTTON_A;
            }
            fn_1_294C(obj);
            if (work->buttons & PAD_BUTTON_A) {
                int sounds[2] = { M657_SFX_TEAM0_ACTION_PRESS, M657_SFX_TEAM1_ACTION_PRESS };

                if (work->prevButtons != work->buttons) {
                    work->sound = HuAudFXPlayPan(sounds[player->team], 48+32*player->team);
                }
                /* Holding A slows the descent; the sound starts only on the press edge. */
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
        if (fn_80071A8C() > 5) {
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
        if (fn_80071A8C() == 8) {
            work->player.state++;
        }
        break;
    }
    if (player->falling) {
        player->pos.y -= speed;
        if (player->pos.y <= 0.0f) {
            int sounds[2] = { M657_SFX_TEAM0_LANDING, M657_SFX_TEAM1_LANDING };

            HuAudFXPlayPan(sounds[player->team], 48+32*player->team);
            if (work->sound != -1) {
                HuAudFXStop(work->sound);
                work->sound = -1;
            }
            player->pos.y = 0.0f;
            fn_8006A030(player->playerNo, 20, 7, 3);
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

/* Per-frame callback that animates the losing player's fly-by during a single-winner result. */
void fn_1_3B50(OMOBJ *obj)
{
    M657Player *work = obj->data;
    M657PlayerView *player = &work->player;
    HuVecF target;
    int height;
    int halfHeight;

    switch (player->state) {
    case 0:
        if ((frand() & 0x1) == 0) {
            player->pos.x = 1000.0f;
            target.x = -1000.0f;
        } else {
            player->pos.x = -1000.0f;
            target.x = 1000.0f;
        }
        target.z = player->pos.z = -1000.0f;
        height = 550;
        halfHeight = height/2;
        if ((frand() & 0x1) == 0) {
            player->pos.y = frand()%halfHeight;
            target.y = halfHeight + frand()%height;
        } else {
            player->pos.y = halfHeight + frand()%height;
            target.y = frand()%halfHeight;
        }
        PSVECSubtract(&target, &work->player.pos, &player->vel);
        PSVECNormalize(&player->vel, &player->vel);
        player->rot.z = 180.0*(atan2(player->vel.y, player->vel.x)/3.141592653589793)-90.0;
        player->state++;
        /* fallthrough */
    case 1:
        if (fn_80071A8C() == 8) {
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
