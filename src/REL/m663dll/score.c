/* Per-player score boxes and the sprite icons that show each player's count. */
#include "dolphin/types.h"

#include "game/memory.h"
#include "REL/m663dll/score.h"
#include "REL/m663dll/player.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/gamemes.h"

void fn_1_D83C(void)
{
    void *animation[3];
    HuVec2f positions[2] = {
        { 86.0f, 58.0f },
        { 488.0f, 58.0f },
    };
    s16 player;
    s16 memberIndex;
    M663ScoreAttribute *attribute;
    M663SpriteGroup *group;
    M663PlayerWork *playerWork;

    animation[1] = HuSprAnimRead(fn_80008DD0(0x9B001F, 0x10000000, 2));
    animation[2] = HuSprAnimRead(fn_80008DD0(0x9B001E, 0x10000000, 2));
    for (player = 0; player < 2; player++) {
        playerWork = &lbl_1_bss_4F0[player];
        attribute = &lbl_1_bss_120[player + 0xC];
        attribute->position = positions[player];
        attribute->width = 0x96;
        attribute->height = 0x30;
        attribute->depth = 0.0f;
        attribute->red = attribute->green = attribute->blue = attribute->alpha = 0x40;
        attribute->transparency = 0.0f;
        attribute->spriteId = MgScoreBoxCreateChar(attribute->width, attribute->height, playerWork->characterNo);
        MgScoreBoxPosSet(attribute->spriteId, attribute->position.x, attribute->position.y);
        MgScoreBoxColorSet(attribute->spriteId, attribute->red, attribute->green, attribute->blue);
        MgScoreBoxTPLvlSet(attribute->spriteId, attribute->transparency);
        MgScoreBoxDispSet(attribute->spriteId, 1);
        group = &lbl_1_bss_350[player];
        group->position = positions[player];
        group->scaleY = (group->scaleX = 0.0f);
        group->groupId = HuSprGrpCreate(6);
        HuSprGrpPosSet(group->groupId, group->position.x, group->position.y);
        HuSprGrpScaleSet(group->groupId, group->scaleX, group->scaleY);
        for (memberIndex = 0; memberIndex < 3; memberIndex++) {
            attribute = &lbl_1_bss_120[memberIndex + player * 6];
            attribute->groupId = group->groupId;
            attribute->memberId = (s16)memberIndex;
            attribute->position.x = 16.0f + 32.0f * (-1.0f + (f32)memberIndex);
            attribute->position.y = 0.0f;
            attribute->animationId = 1;
            attribute->spriteId = HuSprCreate(animation[1], attribute->animationId, 0);
            HuSprGrpMemberSet(attribute->groupId, attribute->memberId, attribute->spriteId);
            HuSprPosSet(attribute->groupId, attribute->memberId, attribute->position.x, attribute->position.y);
            HuSprAttrSet(attribute->groupId, attribute->memberId, 4);
        }
        for (memberIndex = 0; memberIndex < 3; memberIndex++) {
            attribute = &lbl_1_bss_120[player * 6 + 3 + memberIndex];
            attribute->groupId = group->groupId;
            attribute->memberId = memberIndex + 3;
            attribute->position.x = 16.0f + 32.0f * (-1.0f + (f32)memberIndex);
            attribute->position.y = 0.0f;
            attribute->animationId = 2;
            attribute->spriteId = HuSprCreate(animation[2], attribute->animationId, 0);
            HuSprGrpMemberSet(attribute->groupId, attribute->memberId, attribute->spriteId);
            HuSprPosSet(attribute->groupId, attribute->memberId, attribute->position.x, attribute->position.y);
            HuSprAttrReset(attribute->groupId, attribute->memberId, 0x180);
        }
    }
}

/* Shows each player's first progress score icons while the count is below 4. */
void fn_1_DC48(void) {
    s16 player;
    M663PlayerWork *playerWork;
    M663ScoreAttribute *attribute;
    s16 index;

    for (player = 0; player < 2; player++) {
        playerWork = &lbl_1_bss_4F0[player];

        for (index = 0; index < playerWork->progress; index++) {
            if (playerWork->progress < 4) {
                attribute = &lbl_1_bss_120[index + player * 6];
                HuSprAttrReset(attribute->groupId, attribute->memberId, 4);
            }
        }
    }
}
