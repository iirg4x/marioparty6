#include "REL/m657Dll.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/audio.h"
#include "game/wipe.h"
#include "game/esprite.h"
#include "game/gamemes.h"
#include "string.h"
#include "math.h"

/* This module calls the MSL library entry, not the stdlib.h builtin macro. */
int abs(int value);

M657Work lbl_1_bss_0;
M657Work *lbl_1_data_0 = &lbl_1_bss_0;
M657Viewport lbl_1_data_4[2] = {
    { 0, 0, 320, 480 },
    { 0, 320, 320, 480 }
};
s32 lbl_1_data_24[2] = { 1, 2 };
MGSEQ_PARAM lbl_1_data_2C = {
    0, 0, fn_1_B10, fn_1_BD4, fn_1_C74, fn_1_CA0, fn_1_D28,
    fn_1_1090, fn_1_1564, fn_1_15B8, fn_1_15BC
};

void fn_1_0(void)
{
    memset(&lbl_1_bss_0, 0, sizeof(M657Work));
    lbl_1_data_0->music = -1;
    lbl_1_data_0->state = 0;
    lbl_1_data_0->timer = 0;
}

void fn_1_6C(void)
{
}

void fn_1_70(void)
{
    lbl_1_data_0->objman = omInitObjMan(160, 131072);
    omGameSysInit(lbl_1_data_0->objman);
}

void fn_1_BC(OMOBJMAN *objman)
{
    M657Camera *camera;
    M657Viewport *viewport;
    int i;
    int team;
    OMOBJ *obj;

    camera = NULL;
    obj = NULL;
    i = 0;
    for (i = 0; i < 4; i++) {
        team = GwPlayerConf[i].grpNo;
        if (team == 0 || team == 1) {
            viewport = &lbl_1_data_4[team];
            obj = lbl_1_data_0->cameraObj[team] =
                omAddObjEx(objman, 32730, 0, 0, -1, fn_1_428);
            camera = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657Camera), HU_MEMNUM_OVL);
            obj->data = camera;
            memset(camera, 0, sizeof(M657Camera));
            camera->cameraMask = lbl_1_data_24[team];
            Hu3DCameraCreate(camera->cameraMask);
            Hu3DCameraPerspectiveSet(camera->cameraMask, 45.0f, 20.0f,
                100000.0f, 0.6333333f);
            Hu3DCameraViewportSet(camera->cameraMask, viewport->x, viewport->y,
                viewport->width, viewport->height, 0.0f, 1.0f);
            Hu3DCameraScissorSet(camera->cameraMask,
                (u32)viewport->x, (u32)viewport->y, 320, 480);
        }
    }
}

void fn_1_258(void)
{
    M657Camera *camera = lbl_1_data_0->cameraObj[0]->data;
    HuVecF target = { 0, 0, -800 };

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 45.0f, 20.0f, 100000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    camera->pos.x = 0.0f;
    camera->pos.y = 202.0f;
    camera->pos.z = 583.0f;
    Hu3DCameraPosSet(camera->cameraMask,
        camera->pos.x, camera->pos.y, camera->pos.z,
        0.0f, 1.0f, 0.0f, target.x, target.y, target.z);
    lbl_1_data_0->cameraObj[0]->objFunc = fn_1_5C4;
}

void fn_1_3EC(void)
{
}

void fn_1_3F0(s16 team, OMOBJ *target)
{
    M657Camera *camera = lbl_1_data_0->cameraObj[team]->data;
    camera->target = target;
}

void fn_1_428(OMOBJ *obj)
{
    obj->objFunc = fn_1_438;
}

void fn_1_438(OMOBJ *obj)
{
    M657Camera *camera = obj->data;
    M657PlayerView *player;
    float *weight;
    void *playerData = camera->target->data;
    HuVecF target;

    player = playerData;
    {
        s16 cameraNo = camera->cameraMask - 1;
        HuVecF farPos = { 0, 3334, 3549 };
        HuVecF nearPos = { 0, 202, 691 };

        camera->pos.x = 0.0f;
        weight = fn_1_2F48(player->team);
        camera->pos.y = farPos.y + *weight * (nearPos.y - farPos.y);
        camera->pos.z = farPos.z + *weight * (nearPos.z - farPos.z);
        target = player->pos;
        target.x = 0.0f;
        target.y = player->pos.y / 2.0f;
        target.z -= 800.0f;
        Hu3DCameraPosSet(camera->cameraMask,
            camera->pos.x, camera->pos.y, camera->pos.z,
            0.0f, 1.0f, 0.0f, target.x, target.y, target.z);
    }
}

void fn_1_5C4(OMOBJ *obj)
{
}

void fn_1_5C8(void)
{
    HU3D_LIGHTID light;
    GXColor color = { 255, 255, 255, 255 };
    HuVecF pos = { 0, 3000, 3000 };
    HuVecF dir = { 0.3f, -0.8f, 0.3f };
    HuVecF lightParam = { 20, 45, 1000 };

    /* Retail passes this vector as the initial color, then overwrites it. */
    light = lbl_1_data_0->light = Hu3DGLightCreateV(&pos, &dir, (GXColor *)&lightParam);
    Hu3DGLightPointSet(light, 1000.0f, 1.0f, GX_DA_MEDIUM);
    Hu3DGLightStaticSet(light, TRUE);
    Hu3DGLightColorSet(light, color.r, color.g, color.b, color.a);
}

void fn_1_6E0(void)
{
}

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_784();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

void fn_1_784(void)
{
    fn_1_0();
    fn_1_70();
    fn_1_BC(lbl_1_data_0->objman);
    fn_1_5C8();
    fn_1_4E58(lbl_1_data_0->objman);
    fn_1_2018(lbl_1_data_0->objman);
    fn_1_15C0(lbl_1_data_0->objman);
    fn_1_3E1C(lbl_1_data_0->objman);
    fn_1_4570(lbl_1_data_0->objman);
    MgSeqCreate(&lbl_1_data_2C);
}

void fn_1_B10(s16 mode, s16 frameNo)
{
    int team;
    s16 i;

    for (i = 0; i < 4; i++) {
        team = GwPlayerConf[i].grpNo;
        if (team == 0 || team == 1) {
            fn_1_3F0(team, lbl_1_bss_30[team]);
            fn_1_2BE4(team, 0);
        }
    }
    MgSeqModeNext();
}

void fn_1_BD4(s16 mode, s16 frameNo)
{
    s16 i;
    s16 count = 0;

    if (MgSeqFrameNoGet() == 0) {
        lbl_1_data_0->music = HuAudBGMPlay(87);
    }
    fn_1_4EE4(0);
    fn_1_4EE4(1);
    for (i = 0; i < 2; i++) {
        if (fn_1_3154(i) != 0) {
            count++;
        }
    }
    if (count >= 2) {
        MgSeqModeNext();
    }
}

void fn_1_C74(s16 mode, s16 frameNo)
{
    fn_1_4EE4(0);
    fn_1_4EE4(1);
}

void fn_1_CA0(s16 mode, s16 frameNo)
{
    s16 i;
    s16 count = 0;

    fn_1_4EE4(0);
    fn_1_4EE4(1);
    for (i = 0; i < 2; i++) {
        if (fn_1_2EEC(i) == 0) {
            count++;
        }
    }
    if (count == 2 || fn_1_3F2C() == -600) {
        MgSeqModeNext();
    }
}

void fn_1_D28(s16 mode, s16 frameNo)
{
    M657PlayerView *player;
    s16 i;
    OMOBJ *obj;
    u16 status;
    s32 result[2];

    if (MgSeqFrameNoGet() == 0) {
        s16 charNo[2] = { -1, -1 };
        s16 playerNo[2] = { -1, -1 };

        obj = lbl_1_bss_30[0];
        player = obj->data;
        charNo[0] = player->charNo;
        playerNo[0] = player->playerNo;
        obj = lbl_1_bss_30[1];
        player = obj->data;
        charNo[1] = player->charNo;
        playerNo[1] = player->playerNo;
        if (lbl_1_data_0->music != -1) {
            HuAudSStreamFadeOut(lbl_1_data_0->music, 100);
            lbl_1_data_0->music = -1;
        }
        for (i = 0; i < 2; i++) {
            lbl_1_data_0->winners[i] = -1;
            result[i] = fn_1_3EEC(i);
        }
        lbl_1_data_0->winnerCount = 0;
        if (result[0] + result[1] == 0) {
            status = MgSeqWinnerSet2(charNo[0], charNo[1]);
            fn_1_2BE4(0, 1);
            fn_1_2BE4(1, 1);
            lbl_1_data_0->winners[0] = 0;
            lbl_1_data_0->winners[1] = 1;
            lbl_1_data_0->winnerCount = 2;
        } else if (result[0] == result[1]) {
            status = MgSeqWinnerSet(-1, -1, -1, -1);
        } else if (abs(result[0]) < abs(result[1])) {
            status = MgSeqWinnerSet1(charNo[0]);
            fn_1_2BE4(0, 1);
            lbl_1_data_0->winners[0] = 0;
            lbl_1_data_0->winnerCount = 1;
            lbl_1_data_0->unk_22 = 1;
            GWMgCoinBonusSet(playerNo[0], 10);
            OSReport("winnner : %d\n", playerNo[0]);
        } else {
            status = MgSeqWinnerSet1(charNo[1]);
            fn_1_2BE4(1, 1);
            lbl_1_data_0->winners[0] = 1;
            lbl_1_data_0->winnerCount = 1;
            lbl_1_data_0->unk_22 = 0;
            GWMgCoinBonusSet(playerNo[1], 10);
            OSReport("winnner : %d\n", playerNo[1]);
        }
    }
}

void fn_1_1090(s16 mode, s16 frameNo)
{
    int frame;
    s16 i = 0;
    s16 count = 0;

    if (lbl_1_data_0->winnerCount == 0 && lbl_1_data_0->state < 5) {
        lbl_1_data_0->timer = 30;
        lbl_1_data_0->state = 5;
    }
    switch (lbl_1_data_0->state) {
        case 0:
            for (i = 0; i < 2; i++) {
                if (fn_1_2B64(i) != 0) {
                    count++;
                }
            }
            if (count == 2) {
                lbl_1_data_0->state++;
            }
            break;
        case 1:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            for (frame = 0; frame < 60; frame++) {
                HuPrcVSleep();
            }
            Hu3DCameraKill(3);
            lbl_1_data_0->state++;
            break;
        case 2:
            fn_1_258();
            fn_1_5020();
            fn_1_3F54();
            for (frame = 0; frame < 2; frame++) {
                fn_1_2F90(frame, 1);
            }
            switch (lbl_1_data_0->winnerCount) {
                case 1:
                    fn_1_30FC(lbl_1_data_0->unk_22);
                    break;
                case 2:
                {
                    HuVecF pos1 = { -80, 0, 0 };
                    HuVecF pos2 = { 80, 0, 0 };
                    fn_1_306C(0, pos1);
                    fn_1_306C(1, pos2);
                    break;
                }
            }
            lbl_1_data_0->state++;
            break;
        case 3:
            WipeCreate(WIPE_MODE_IN, WIPE_TYPE_PREV, 60);
            for (frame = 0; frame < 60; frame++) {
                HuPrcVSleep();
            }
            lbl_1_data_0->state++;
            break;
        case 4:
            lbl_1_data_0->timer = 0;
            lbl_1_data_0->state++;
            break;
        case 5:
            if (lbl_1_data_0->timer > 60.0f) {
                lbl_1_data_0->state++;
            }
            lbl_1_data_0->timer++;
            break;
        case 6:
            MgSeqModeNext();
            break;
    }
}

void fn_1_1564(s16 mode, s16 frameNo)
{
    if (MgSeqFrameNoGet() == 0) {
        if (lbl_1_data_0->winnerCount == 0) {
            fn_1_2C20(0);
        } else {
            fn_1_2C20(1);
        }
    }
}

void fn_1_15B8(s16 mode, s16 frameNo)
{
}

void fn_1_15BC(s16 mode, s16 frameNo)
{
}

void fn_1_4E58(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657SpriteWork *work;

    obj = omAddObjEx(objman, 70, 0, 0, -1, fn_1_5248);
    lbl_1_bss_48 = obj;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657SpriteWork), HU_MEMNUM_OVL);
    obj->data = work;
    memset(work, 0, sizeof(M657SpriteWork));
}

void fn_1_4EE0(void)
{
}

extern HuVec2f lbl_1_data_338[2];

void fn_1_4EE4(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657PlayerView *player;
    M657SpriteWork *sprites;
    M657Player *work = obj->data;
    float height;
    float fraction;
    float range;
    float offset;

    player = &work->player;
    sprites = lbl_1_bss_48->data;
    height = player->pos.y;
    range = 366.0f;
    if (height > 1547.0f) {
        height = 1547.0f;
    }
    fraction = (1547.0f - height) / 1547.0f;
    offset = range * fraction;
    espPosSet(sprites->playerSprites[team], lbl_1_data_338[team].x,
        offset + lbl_1_data_338[team].y);
}

void fn_1_5020(void)
{
    M657SpriteWork *work = lbl_1_bss_48->data;
    s16 i;

    for (i = 0; i < 9; i++) {
        espDispOff(work->sprites[i]);
    }
    espDispOff(work->playerSprites[0]);
    espDispOff(work->playerSprites[1]);
}

s32 lbl_1_data_280[9] = {
    DATANUM(DATA_m657, 17), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16),
    DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16),
    DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16)
};
s16 lbl_1_data_2A4[9] = { 60, 70, 70, 70, 70, 70, 70, 70, 70 };
s32 lbl_1_data_2B8[14] = {
    DATANUM(DATA_mgconst, 32), DATANUM(DATA_mgconst, 33),
    DATANUM(DATA_mgconst, 34), DATANUM(DATA_mgconst, 35),
    DATANUM(DATA_mgconst, 36), DATANUM(DATA_mgconst, 37),
    DATANUM(DATA_mgconst, 38), DATANUM(DATA_mgconst, 39),
    DATANUM(DATA_mgconst, 40), DATANUM(DATA_mgconst, 41),
    DATANUM(DATA_mgconst, 42), DATANUM(DATA_mgconst, 43),
    DATANUM(DATA_mgconst, 44), DATANUM(DATA_mgconst, 45)
};
HuVec2f lbl_1_data_2F0[9] = {
    { 288, 430 }, { 288, 0 }, { 288, 64 }, { 288, 128 }, { 288, 192 },
    { 288, 256 }, { 288, 320 }, { 288, 384 }, { 288, 448 }
};
HuVec2f lbl_1_data_338[2] = { { 272, 52 }, { 304, 52 } };

void fn_1_5094(void)
{
    s16 i;
    M657SpriteWork *work = lbl_1_bss_48->data;
    s16 team;

    for (i = 0; i < 9; i++) {
        work->sprites[i] = espEntry(lbl_1_data_280[i], lbl_1_data_2A4[i], 0);
        espPosSet(work->sprites[i], lbl_1_data_2F0[i].x, lbl_1_data_2F0[i].y);
    }
    for (i = 0; i < 4; i++) {
        team = GwPlayerConf[i].grpNo;
        if (team == 0 || team == 1) {
            work->playerSprites[team] = espEntry(lbl_1_data_2B8[GwPlayerConf[i].charNo], 50, 0);
            espPosSet(work->playerSprites[team], lbl_1_data_338[team].x, lbl_1_data_338[team].y);
        }
    }
}

void fn_1_5248(OMOBJ *obj)
{
    fn_1_5094();
    obj->objFunc = fn_1_5414;
}

void fn_1_5414(OMOBJ *obj)
{
}
