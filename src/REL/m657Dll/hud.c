#include "REL/m657Dll.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/esprite.h"
#include "string.h"

OMOBJ *lbl_1_bss_48;

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
