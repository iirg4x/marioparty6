#include "REL/m604DLL/score_owner.h"
#include "REL/m604DLL/player.h"
#include "dolphin/types.h"

extern "C" {
#include "game/mg/score.h"
#include "game/gamemes.h"
#include "game/esprite.h"

}

struct ScoreBoxPosition {
    s32 x;
    s32 y;
};

struct ScoreBoxPositions {
    ScoreBoxPosition values[4];
};

M604ScoreOwner::M604ScoreOwner()
{
    s32 index;
    s16 scoreBox;
    ScoreBoxPositions positions = {
        {{80, 54}, {200, 54}, {375, 54}, {495, 54}}
    };
    for (index = 0; index < 4; index++) {
        scoreBox = MgScoreBoxCreate(0x64, 0x20);
        MgScoreBoxPosSet(scoreBox, (f32)positions.values[index].x,
                         (f32)positions.values[index].y);
        scoreBoxIds[index] = scoreBox;
        fn_1_14E14(index, positions.values[index].x,
                   positions.values[index].y);
    }
    fn_1_15060();
}

M604ScoreOwner::~M604ScoreOwner()
{
    for (int scoreIndex = 0; scoreIndex < 4; scoreIndex++) {
        MgScoreKill(scoreDisplays[scoreIndex]);
    }
}

extern "C" { u32 lbl_1_data_850[15] = { 0x9B0020, 0x9B0021, 0x9B0022, 0x9B0023, 0x9B0024, 0x9B0025, 0x9B0026, 0x9B0027, 0x9B0028, 0x9B0029, 0x9B002A, 0x9B002B, 0x9B002C, 0x9B002D, 0x9B0029 }; }

void fn_1_14D30(M604ScoreOwner *owner, s32 entryIndex, s32 dataIndex, s32 x, s32 y)
{
    owner->spriteIds[entryIndex] = espEntry(lbl_1_data_850[dataIndex], 0, 0);
    espBankSet(owner->spriteIds[entryIndex], 0);
    espPosSet(owner->spriteIds[entryIndex], (f32)x, (f32)y);
}

extern "C" {
struct ScoreDisplayOffset {
    f32 x;
    f32 y;
};
f32 lbl_1_data_88C[3] = { 1.0f, 0.9f, 1.0f };
struct ScoreDisplayOffset lbl_1_data_898[3] = { { -26.0f, 0.0f }, { -34.0f, 1.0f }, { -32.0f, 0.0f } };

}

void M604ScoreOwner::fn_1_14E14(s32 index, s32 x, s32 y) {
    MGSCORE *display;
    s32 layoutIndex;

    layoutIndex = 0;
    layoutIndex = 1;
    display = MgScoreCreate(0x9B0032, 0x9B0035, 0);
    MgScoreMaxDigitSet(display, 3);
    MgScoreDigitScaleSet(display, lbl_1_data_88C[layoutIndex], lbl_1_data_88C[layoutIndex]);
    MgScoreScaleSet(display, 1.0f, 1.0f);
    MgScoreValueSet(display, 0);
    MgScorePosSet(display, (f32) x + lbl_1_data_898[layoutIndex].x, (f32) y + lbl_1_data_898[layoutIndex].y);
    MgScoreZRotSet(display, 0.0f);
    MgScoreTPLvlSet(display, 1.0f);
    MgScoreColorSet(display, 0xFFU, 0xFFU, 0xFFU);
    MgScorePriSet(display, 0xA);
    if (layoutIndex == 1) {
        MgScoreDigitWidthSet(display, 15.0f);
        display->unitOfs.x -= 6.0f;
    }
    scoreDisplays[index] = display;
}

extern "C" {

}
