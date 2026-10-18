/* Solo menu sprite layouts, character navigation, and earned-prize descriptions. */
#ifndef REL_MDSINGDLL_MENU_DATA_H
#define REL_MDSINGDLL_MENU_DATA_H

#include "dolphin/types.h"
#include "dolphin/mtx.h"

typedef struct MdsingCharacterNeighbors {
    /* Character reached in each navigation direction; -1 means no neighbor. */
    s16 neighborCharacter[8];
    /* -1 is available; any other value reserves the character during selection. */
    s16 selectionState;
} MDSING_CHARACTER_NEIGHBORS;

typedef struct MdsingSpriteDesc {
    /* Destination sprite group and member indices. */
    s16 groupNo;
    s16 memberNo;
    /* Index into the menu's loaded sprite animation resources. */
    s16 animNo;
    /* Sprite draw priority and initial animation bank. */
    s16 priority;
    s16 bank;
    /* Local screen position in pixels and X/Y scale. */
    HuVec2f pos;
    HuVec2f scale;
    /* Initial rotation in degrees around the screen's Z axis. */
    f32 zRot;
} MDSING_SPRITE_DESC;

typedef struct MdsingPrizeRow {
    /* Prize flag tested for fixed awards. */
    s16 prizeFlag;
    /* Row kind: -1 ends the table, 0 is an award, 1 is a heading, 10-19 calculate totals. */
    s16 rowKind;
    /* Result-row message ID. */
    s32 messageNo;
    /* Additional message ID stored with the prize; -1 means none. Not used by the result list. */
    s32 unusedMessageNo;
    /* Fixed reward amount or multiplier for a calculated reward. */
    s16 rewardAmount;
} MDSING_PRIZE_ROW;

#endif
