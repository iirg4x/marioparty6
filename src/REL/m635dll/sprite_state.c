#include "REL/m635dll.h"
#include "game/sprite.h"

s16 fn_1_27E4(s16 team, s16 player)
{
    return lbl_1_bss_74[player + team * 2].state;
}

void fn_1_280C(s16 team, s16 player, s16 member)
{
    M635Sprite *sprite;
    int i;

    sprite = &lbl_1_bss_74[player + team * 2];
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

void fn_1_28A8(s16 team, s16 player)
{
    M635Sprite *sprite;

    sprite = &lbl_1_bss_74[player + team * 2];
    HuSprAttrSet(sprite->group, 13, HUSPR_ATTR_DISPOFF);
}

void fn_1_2904(void)
{
    int i;

    for (i = 0; i < 4; i++) {
        HuSprGrpKill(lbl_1_bss_74[i].group);
    }
}
