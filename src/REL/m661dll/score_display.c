/* Creates the two players' character boxes and two-digit score counters. */
#include <math.h>
#include "dolphin/types.h"

#include "game/gamemes.h"

#include "game/mg/score.h"
#include "REL/m661dll/player_state.h"
#include "REL/m661dll/score_display.h"

typedef struct WordQuad {
    struct {
        f32 x;
        f32 y;
    } values[2];
} WordQuad;

void fn_1_249A8(void)
{
    f32 characterBoxSize[2] = {96.0f, 48.0f};
    WordQuad characterPosition = { {{60.0f, 58.0f}, {514.0f, 58.0f}} };
    WordQuad scorePosition = { {{67.0f, 59.0f}, {521.0f, 59.0f}} };
    GXColor characterColor = { 0x40, 0x40, 0x40, 0x40 };
    GXColor scoreColor = { 0xFF, 0xD8, 0x15, 0xFF };
    f32 transparency;
    s16 playerIndex;

    transparency = 0.0f;
    playerIndex = 0;

    for (; playerIndex < 2; playerIndex++) {
        ScoreDisplay *display = &lbl_1_bss_0[playerIndex];
        PlayerResult *playerData = &lbl_1_bss_1C04[playerIndex];

        display->characterBox = MgScoreBoxCreateChar(
            (s16)characterBoxSize[0], (s16)characterBoxSize[1],
            (s16)playerData->character);
        MgScoreBoxPosSet(display->characterBox,
                         characterPosition.values[playerIndex].x,
                         characterPosition.values[playerIndex].y);
        MgScoreBoxColorSet(display->characterBox, characterColor.r, characterColor.g,
                           characterColor.b);
        MgScoreBoxTPLvlSet(display->characterBox, transparency);
        MgScoreBoxDispSet(display->characterBox, 1);
        display->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
        MgScoreMaxDigitSet(display->score, 2);
        MgScoreDispOff(display->score);
        MgScorePosSet(display->score, scorePosition.values[playerIndex].x,
                      scorePosition.values[playerIndex].y);
        MgScoreColorSet(display->score, scoreColor.r, scoreColor.g, scoreColor.b);
        MgScoreValueSet(display->score, 0);
    }
}
