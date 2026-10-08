/* Creates and updates the arena gauge and the two team position markers. */
#include "REL/m657Dll.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/esprite.h"
#include "string.h"

OMOBJ *lbl_1_bss_48;

/* Creates the HUD object and allocates its sprite handles during setup. */
void fn_1_4E58(OMOBJMAN *objman)
{
    OMOBJ *obj;
    M657SpriteWork *spriteWork;

    obj = omAddObjEx(objman, 70, 0, 0, -1, fn_1_5248);
    lbl_1_bss_48 = obj;
    spriteWork = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M657SpriteWork), HU_MEMNUM_OVL);
    obj->data = spriteWork;
    memset(spriteWork, 0, sizeof(M657SpriteWork));
}

void fn_1_4EE0(void)
{
}

extern HuVec2f lbl_1_data_338[2];

/* Called by the sequence callbacks to move a team's marker with its arena height. */
void fn_1_4EE4(s16 team)
{
    OMOBJ *obj = lbl_1_bss_30[team];
    M657PlayerView *player;
    M657SpriteWork *sprites;
    M657Player *playerWork = obj->data;
    float playerHeight;
    float descentFraction;
    float gaugeHeight;
    float markerOffset;

    player = &playerWork->player;
    sprites = lbl_1_bss_48->data;
    playerHeight = player->pos.y;
    gaugeHeight = 366.0f;
    if (playerHeight > 1547.0f) {
        /* Keep the marker at the gauge's top until the player begins descending. */
        playerHeight = 1547.0f;
    }
    descentFraction = (1547.0f - playerHeight) / 1547.0f;
    markerOffset = gaugeHeight * descentFraction;
    espPosSet(sprites->playerSprites[team], lbl_1_data_338[team].x,
        markerOffset + lbl_1_data_338[team].y);
}

/* Hides the gauge and both player markers when the result view takes over. */
void fn_1_5020(void)
{
    M657SpriteWork *spriteWork = lbl_1_bss_48->data;
    s16 spriteIndex;

    for (spriteIndex = 0; spriteIndex < 9; spriteIndex++) {
        espDispOff(spriteWork->sprites[spriteIndex]);
    }
    espDispOff(spriteWork->playerSprites[0]);
    espDispOff(spriteWork->playerSprites[1]);
}

/* Sprite data IDs for the gauge: one entry 17 sprite and eight entry 16 sprites. */
s32 lbl_1_data_280[9] = {
    DATANUM(DATA_m657, 17), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16),
    DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16),
    DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16), DATANUM(DATA_m657, 16)
};
/* Sprite priorities paired with the gauge cap and segment data above. */
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
/* Screen positions for the gauge cap and its eight vertical segments. */
HuVec2f lbl_1_data_2F0[9] = {
    { 288, 430 }, { 288, 0 }, { 288, 64 }, { 288, 128 }, { 288, 192 },
    { 288, 256 }, { 288, 320 }, { 288, 384 }, { 288, 448 }
};
/* HUD marker origins for team 0 and team 1, respectively. */
HuVec2f lbl_1_data_338[2] = { { 272, 52 }, { 304, 52 } };

/* Creates the gauge sprites and one character marker for each configured team. */
void fn_1_5094(void)
{
    s16 spriteIndex;
    M657SpriteWork *spriteWork = lbl_1_bss_48->data;
    s16 teamIndex;

    for (spriteIndex = 0; spriteIndex < 9; spriteIndex++) {
        spriteWork->sprites[spriteIndex] =
            espEntry(lbl_1_data_280[spriteIndex], lbl_1_data_2A4[spriteIndex], 0);
        espPosSet(spriteWork->sprites[spriteIndex], lbl_1_data_2F0[spriteIndex].x,
                  lbl_1_data_2F0[spriteIndex].y);
    }
    for (spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
        teamIndex = GwPlayerConf[spriteIndex].grpNo;
        if (teamIndex == 0 || teamIndex == 1) {
            spriteWork->playerSprites[teamIndex] =
                espEntry(lbl_1_data_2B8[GwPlayerConf[spriteIndex].charNo], 50, 0);
            espPosSet(spriteWork->playerSprites[teamIndex], lbl_1_data_338[teamIndex].x,
                      lbl_1_data_338[teamIndex].y);
        }
    }
}

/* Object-create callback that builds the HUD sprites and installs its update. */
void fn_1_5248(OMOBJ *obj)
{
    fn_1_5094();
    obj->objFunc = fn_1_5414;
}

void fn_1_5414(OMOBJ *obj)
{
}
