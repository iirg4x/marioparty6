#include "REL/m635dll.h"
#include "game/sprite.h"

void fn_1_2268(s16 team, s16 player)
{
    M635Sprite *sprite;
    s16 member;

    sprite = &lbl_1_bss_74[player + team * 2];
    if (sprite->state == 4) {
        member = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
        HuSprAttrSet(sprite->group, member, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, member + 1, HUSPR_ATTR_DISPOFF);
        sprite->timer = 0;
    }
}
