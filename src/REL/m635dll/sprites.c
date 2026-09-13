#include "REL/m635dll.h"
#include "game/sprite.h"

void fn_1_2330(void)
{
    M635Sprite *sprite;
    int i;
    s16 member;

    for (i = 0; i < 4; i++) {
        sprite = &lbl_1_bss_74[i];
        member = lbl_1_bss_4.team[i / 2].buttonIndex[i % 2] * 2;
        switch (sprite->state) {
        case 5:
            break;
        case 0:
            break;
        case 1:
            sprite->scale += 0.2;
            if (sprite->scale >= 1.0f) {
                sprite->scale = 1.0f;
                sprite->state = 0;
            }
            HuSprScaleSet(sprite->group, member, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, 12, sprite->scale, sprite->scale);
            break;
        case 3:
            sprite->timer--;
            if (sprite->timer <= 0) {
                HuSprAttrSet(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrSet(sprite->group, 13, HUSPR_ATTR_DISPOFF);
                sprite->state = 5;
                sprite->scale = 0.0f;
            }
            break;
        case 4:
            if (sprite->scale < 1.0f) {
                sprite->scale += 0.2;
            }
            if (sprite->timer % 8 == 0) {
                HuSprAttrSet(sprite->group, sprite->member, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
            } else if (sprite->timer % 8 == 4) {
                HuSprAttrSet(sprite->group, sprite->member + 1, HUSPR_ATTR_DISPOFF);
                HuSprAttrReset(sprite->group, sprite->member, HUSPR_ATTR_DISPOFF);
            }
            HuSprScaleSet(sprite->group, sprite->member, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, sprite->member + 1, sprite->scale, sprite->scale);
            HuSprScaleSet(sprite->group, 13, sprite->scale, sprite->scale);
            sprite->timer++;
            break;
        }
    }
}

void fn_1_25EC(s16 team, s16 player, s16 state)
{
    M635Sprite *sprite;
    int i;
    s16 member;

    sprite = &lbl_1_bss_74[player + team * 2];
    sprite->state = state;
    member = lbl_1_bss_4.team[team].buttonIndex[player] * 2;
    for (i = 0; i < 12; i++) {
        HuSprAttrSet(sprite->group, i, HUSPR_ATTR_DISPOFF);
    }
    for (i = 0; i < 2; i++) {
        HuSprAttrSet(sprite->group, i + 12, HUSPR_ATTR_DISPOFF);
    }
    switch (state) {
    case 1:
        sprite->scale = 0.0f;
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 12, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(sprite->group, member, 0.0f, 0.0f);
        HuSprScaleSet(sprite->group, 12, 0.0f, 0.0f);
        sprite->member = member;
        break;
    case 3:
        sprite->timer = 8;
        sprite->scale = 1.0f;
        HuSprAttrReset(sprite->group, member + 1, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        break;
    case 4:
        HuSprAttrReset(sprite->group, 13, HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(sprite->group, member, HUSPR_ATTR_DISPOFF);
        sprite->member = member;
        sprite->timer = 0;
        sprite->scale = 0.0f;
        break;
    }
}
