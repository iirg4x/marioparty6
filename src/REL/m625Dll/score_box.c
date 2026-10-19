/* Score displays and countdown digits for the three opposing players. */
#include "dolphin/math.h"
#include "game/gamemes.h"
#include "game/main.h"
#include "game/gamework.h"
#include "game/mg/score.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "datadir_enum.h"

typedef struct M625ByteArray_s {
    /* Four setup bytes copied into the score-box state. */
    u8 bytes[4];
} M625ByteArray;

typedef struct M625ScoreBoxState_s {
    /* Sprite group containing the countdown digits. */
    s16 scoreBox;
    /* Two bytes between the sprite id and active state. */
    u8 beforeActive[2];
    /* Nonzero while this opponent's score box is in the active display state. */
    s32 active;
    /* Frame countdown decremented by the per-frame score-box update. */
    s16 frames;
    /* Minigame score-box id for the opponent's character. */
    s16 secondaryBox;
    /* Opacity multiplier applied to the character score box. */
    f32 opacity;
    /* Four bytes initialized when this score-box slot is set up. */
    M625ByteArray beforeRemainingFrames;
    /* Remaining minigame time in frames, clamped to 0 through 600. */
    s16 remainingFrames;
    /* Two bytes following the remaining-frame count. */
    u8 trailing[2];
} M625ScoreBoxState;

typedef struct M625ScoreBoxPlacement_s {
    /* Score-box x coordinate in screen pixels. */
    f32 x;
    /* Score-box y coordinate in screen pixels. */
    f32 y;
    /* Stored z coordinate, initialized to zero. */
    f32 z;
} M625ScoreBoxPlacement;

M625ByteArray lbl_1_data_410 = { { 64, 64, 64, 0 } };

M625ScoreBoxPlacement lbl_1_data_414[3] = {
    { 58.0f, 58.0f, 0.0f },
    { 174.0f, 58.0f, 0.0f },
    { 290.0f, 58.0f, 0.0f }
};

f32 lbl_1_data_438[4] = { 18.0f, 0.0f, 0.0f, 0.0f };

M625ScoreBoxState lbl_1_bss_7C8[3];
extern void fn_1_131BC(s16, f32);

/* Called during minigame setup to create score boxes and countdown sprites for opponents. */
void fn_1_1284C(void)
{
    ANIMDATA *animations[3];
    HUSPRID sprite;
    s16 activePlayer;
    s16 boxIndex;

    boxIndex = 0;
    activePlayer = 0;
    while (activePlayer < 4) {
        if (GwPlayerConf[activePlayer].grpNo != 0 && boxIndex < 3) {
            lbl_1_bss_7C8[boxIndex].frames = 0;
            lbl_1_bss_7C8[boxIndex].active = 0;
            lbl_1_bss_7C8[boxIndex].opacity = 0.8f;
            lbl_1_bss_7C8[boxIndex].beforeRemainingFrames = lbl_1_data_410;
            lbl_1_bss_7C8[boxIndex].remainingFrames = 600;
            lbl_1_bss_7C8[boxIndex].secondaryBox = MgScoreBoxCreateChar(
                84, 36, GwPlayerConf[activePlayer].charNo);
            MgScoreBoxTPLvlSet(lbl_1_bss_7C8[boxIndex].secondaryBox,
                               lbl_1_bss_7C8[boxIndex].opacity);
            MgScoreBoxPosSet(lbl_1_bss_7C8[boxIndex].secondaryBox,
                             lbl_1_data_414[boxIndex].x,
                             lbl_1_data_414[boxIndex].y);
            MgScoreBoxDispSet(lbl_1_bss_7C8[boxIndex].secondaryBox, 0);
            lbl_1_bss_7C8[boxIndex].scoreBox = HuSprGrpCreate(3);
            animations[1] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 12), HU_MEMNUM_OVL, HEAP_MODEL));
            sprite = HuSprCreate(animations[1], 1, 0);
            HuSprGrpMemberSet(lbl_1_bss_7C8[boxIndex].scoreBox, 1, sprite);
            HuSprPosSet(lbl_1_bss_7C8[boxIndex].scoreBox, 1,
                        lbl_1_data_438[0], lbl_1_data_438[1]);
            HuSprBankSet(lbl_1_bss_7C8[boxIndex].scoreBox, 1, 0);
            animations[2] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_m625, 13), HU_MEMNUM_OVL, HEAP_MODEL));
            sprite = HuSprCreate(animations[2], 2, 0);
            HuSprGrpMemberSet(lbl_1_bss_7C8[boxIndex].scoreBox, 2, sprite);
            HuSprPosSet(lbl_1_bss_7C8[boxIndex].scoreBox, 2,
                        lbl_1_data_438[0], lbl_1_data_438[1]);
            HuSprAttrSet(lbl_1_bss_7C8[boxIndex].scoreBox, 2, HUSPR_ATTR_INVCOL);
            HuSprBankSet(lbl_1_bss_7C8[boxIndex].scoreBox, 2, 0);
            HuSprGrpPosSet(lbl_1_bss_7C8[boxIndex].scoreBox,
                           lbl_1_data_414[boxIndex].x,
                           lbl_1_data_414[boxIndex].y);
            HuSprGrpScaleSet(lbl_1_bss_7C8[boxIndex].scoreBox,
                             0.0f, 0.0f);
            boxIndex++;
        }
        activePlayer++;
    }
}

/* Called when results begin to reveal the opponents' score boxes. */
void fn_1_12C80(void)
{
    s16 index;

    index = 0;
    while (index < 3) {
        MgScoreBoxDispSet(lbl_1_bss_7C8[index].scoreBox, 1);
        HuSprGrpScaleSet(lbl_1_bss_7C8[index].scoreBox,
                         1.0f, 1.0f);
        lbl_1_bss_7C8[index].active = 1;
        lbl_1_bss_7C8[index].frames = 20;
        MgScoreBoxDispSet(lbl_1_bss_7C8[index].secondaryBox, 1);
        fn_1_131BC(index, 0.0f);
        index++;
    }
}

/* Called by winner handling to highlight one player's score box. */
void fn_1_12D7C(s16 player)
{
    s16 boxIndex;
    s16 playerIndex;

    boxIndex = 0;
    playerIndex = 0;
    while (playerIndex < 4) {
        if (GwPlayerConf[playerIndex].grpNo != 0) {
            if (boxIndex < 3) {
                if (playerIndex == player) {
                    MgScoreBoxDispSet(lbl_1_bss_7C8[boxIndex].scoreBox, 1);
                    HuSprGrpScaleSet(lbl_1_bss_7C8[boxIndex].scoreBox,
                                     1.0f, 1.0f);
                    lbl_1_bss_7C8[boxIndex].active = 0;
                    lbl_1_bss_7C8[boxIndex].frames = 20;
                    fn_1_131BC(boxIndex, 1.0f);
                }
                boxIndex++;
            }
        }
        playerIndex++;
    }
}

/* Called by sequence callbacks each frame to apply score-box visibility and clamp transition
 * timers. */
void fn_1_12EA8(void)
{
    f32 scale;
    s16 index;

    index = 0;
    while (index < 3) {
        if (lbl_1_bss_7C8[index].active != 0) {
            scale = 1.0f;
        } else {
            scale = 0.0f;
        }
        fn_1_131BC(index, scale);
        lbl_1_bss_7C8[index].frames--;
        if (lbl_1_bss_7C8[index].frames < 0) {
            lbl_1_bss_7C8[index].frames = 0;
        }
        index++;
    }
}

/* Called during play to show the selected player's remaining time in whole seconds. */
void fn_1_12F98(s16 player, s16 frames)
{
    s16 framesPerSecond;
    s16 elapsedSeconds;
    s16 scoreBoxIndex;
    s16 playerIndex;

    scoreBoxIndex = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (GwPlayerConf[playerIndex].grpNo != 0 && scoreBoxIndex < 3) {
            if (playerIndex == player) {
                lbl_1_bss_7C8[scoreBoxIndex].remainingFrames = frames;
                if (lbl_1_bss_7C8[scoreBoxIndex].remainingFrames < 0) {
                    lbl_1_bss_7C8[scoreBoxIndex].remainingFrames = 0;
                }
                if (lbl_1_bss_7C8[scoreBoxIndex].remainingFrames > 600) {
                    lbl_1_bss_7C8[scoreBoxIndex].remainingFrames = 600;
                }
                if (lbl_1_bss_7C8[scoreBoxIndex].remainingFrames != 0) {
                    framesPerSecond = 60;
                    elapsedSeconds =
                        (framesPerSecond - 1 + lbl_1_bss_7C8[scoreBoxIndex].remainingFrames) /
                        framesPerSecond;
                    framesPerSecond = 10 - elapsedSeconds;
                    HuSprBankSet(
                        lbl_1_bss_7C8[scoreBoxIndex].scoreBox, 1,
                        framesPerSecond);
                    HuSprAttrReset(lbl_1_bss_7C8[scoreBoxIndex].scoreBox, 1, HUSPR_ATTR_DISPOFF);
                } else {
                    HuSprBankSet(lbl_1_bss_7C8[scoreBoxIndex].scoreBox, 1, 0);
                    HuSprAttrSet(lbl_1_bss_7C8[scoreBoxIndex].scoreBox, 1, HUSPR_ATTR_DISPOFF);
                }
            }
            scoreBoxIndex++;
        }
    }
}

/* Called by score-box transitions to apply clamped opacity to both score displays. */
void fn_1_131BC(s16 index, f32 opacity)
{
    if (index < 0 || index >= 3) {
        return;
    }
    if (opacity < 0.0f) {
        opacity = 0.0f;
    }
    if (opacity > 1.0f) {
        opacity = 1.0f;
    }
    HuSprTPLvlSet(lbl_1_bss_7C8[index].scoreBox, 1, opacity);
    if (opacity >= 1.0f) {
        HuSprAttrReset(lbl_1_bss_7C8[index].scoreBox, 2, HUSPR_ATTR_DISPOFF);
    } else {
        HuSprAttrSet(lbl_1_bss_7C8[index].scoreBox, 2, HUSPR_ATTR_DISPOFF);
    }
    opacity *= lbl_1_bss_7C8[index].opacity;
    MgScoreBoxTPLvlSet(lbl_1_bss_7C8[index].secondaryBox, opacity);
}
