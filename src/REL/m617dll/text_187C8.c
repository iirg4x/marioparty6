/* Player score-box and progress-marker sprite setup and display. */
#include "dolphin/math.h"
#include "REL/m617dll/shared.h"
#include "game/data.h"
#include "game/gamemes.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "datadir_enum.h"

typedef struct M617PositionPair {
    M617Vector2 playerPositions[2]; /* Screen positions for the two players' score displays. */
} M617PositionPair;

void HuSprAttrSet(s16 groupId, s16 memberNo, s32 attributes);
void HuSprAttrReset(s16 groupId, s16 memberNo, s32 attributes);

/* fn_1_93D0 calls this to create each player's score box and progress-marker sprites. */
void fn_1_187C8(void)
{
    ANIMDATA *progressAnimations[3];
    M617PositionPair positions = { { { 120.0f, 58.0f }, { 455.0f, 58.0f } } };
    s16 playerIndex;
    s16 markerIndex;
    M617ScoreBoxEntry *entry;
    M617SpriteGroup *group;
    M617PlayerStateData *player;

    progressAnimations[1] = HuSprAnimDataRead(DATANUM(DATA_mgconst, 31));
    progressAnimations[2] = HuSprAnimDataRead(DATANUM(DATA_mgconst, 30));
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        player = &lbl_1_bss_AB0[playerIndex];
        entry = &lbl_1_bss_0[playerIndex + 20];

        entry->position = positions.playerPositions[playerIndex];
        entry->width = 216;
        entry->height = 48;
        entry->opacity = 0.0f;
        entry->red = entry->green = entry->blue = entry->alpha = 64;
        entry->tpLevel = 0.0f;
        entry->spriteId = MgScoreBoxCreateChar(
            entry->width, entry->height, player->character);
        MgScoreBoxPosSet(entry->spriteId, entry->position.x,
                         entry->position.y);
        MgScoreBoxColorSet(entry->spriteId, entry->red,
                           entry->green, entry->blue);
        MgScoreBoxTPLvlSet(entry->spriteId, entry->tpLevel);
        MgScoreBoxDispSet(entry->spriteId, 1);

        group = &lbl_1_bss_370[playerIndex];
        group->position = positions.playerPositions[playerIndex];
        group->scaleY = group->scaleX = 0.0f;
        group->groupId = HuSprGrpCreate(10);
        HuSprGrpPosSet(group->groupId, group->position.x,
                       group->position.y);
        HuSprGrpScaleSet(group->groupId, group->scaleX, group->scaleY);

        for (markerIndex = 0; markerIndex < 5; markerIndex++) {
            entry = &lbl_1_bss_0[markerIndex + playerIndex * 10];

            entry->groupId = group->groupId;
            entry->memberIndex = (s16)markerIndex;
            entry->position.x = 16.0f +
                32.0f *
                    (-2.0f + (f32)markerIndex);
            entry->position.y = 0.0f;
            entry->drawPriority = 1;
            entry->spriteId = HuSprCreate(progressAnimations[1], entry->drawPriority, 0);
            HuSprGrpMemberSet(entry->groupId, entry->memberIndex,
                              entry->spriteId);
            HuSprPosSet(entry->groupId, entry->memberIndex,
                        entry->position.x, entry->position.y);
            HuSprAttrSet(entry->groupId, entry->memberIndex, HUSPR_ATTR_DISPOFF);
        }

        for (markerIndex = 0; markerIndex < 5; markerIndex++) {
            entry = &lbl_1_bss_0[(playerIndex * 10 + 5) + markerIndex];

            entry->groupId = group->groupId;
            entry->memberIndex = markerIndex + 5;
            entry->position.x = 16.0f +
                32.0f *
                    (-2.0f + (f32)markerIndex);
            entry->position.y = 0.0f;
            entry->drawPriority = 2;
            entry->spriteId = HuSprCreate(progressAnimations[2], entry->drawPriority, 0);
            HuSprGrpMemberSet(entry->groupId, entry->memberIndex,
                              entry->spriteId);
            HuSprPosSet(entry->groupId, entry->memberIndex,
                        entry->position.x, entry->position.y);
            HuSprAttrReset(entry->groupId, entry->memberIndex,
                           HUSPR_ATTR_ADDCOL | HUSPR_ATTR_INVCOL);
        }
    }
}

/* fn_1_5DB4 calls this during score updates to reveal earned and hide unearned markers. */
void fn_1_18BD4(void)
{
    M617PlayerStateData *playerWork;
    M617ScoreBoxEntry *spriteWork;
    s16 spriteIndex;
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        playerWork = &lbl_1_bss_AB0[playerIndex];
        for (spriteIndex = 0; spriteIndex < playerWork->progress; spriteIndex++) {
            if (playerWork->progress < 6) {
                spriteWork = &lbl_1_bss_0[spriteIndex + playerIndex * 10];
                HuSprAttrReset(spriteWork->groupId, spriteWork->memberIndex,
                               HUSPR_ATTR_DISPOFF);
            }
        }
        for (spriteIndex = 4; spriteIndex >= playerWork->progress; spriteIndex--) {
            spriteWork = &lbl_1_bss_0[spriteIndex + playerIndex * 10];
            HuSprAttrSet(spriteWork->groupId, spriteWork->memberIndex,
                         HUSPR_ATTR_DISPOFF);
        }
    }
}
