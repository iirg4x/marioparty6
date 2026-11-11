extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "math.h"
}

extern "C" {
#include "dolphin/gx.h"
}

#include "dolphin/mtx.h"

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

extern "C" {
#include "game/mg/score.h"
}

#include "game/pad.h"

#include "game/frand.h"

extern "C" {
#include "game/sprite.h"
}

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"
}

#include "datadir_enum.h"
#include "REL/framework/linknode.h"

#include "REL/m622DLL/score_display_owner.h"

struct ScorePosition {
    f32 x;
    f32 y;
};

extern const ScorePosition lbl_1_rodata_2E8[4] = {
    {71.0f, 63.0f},
    {191.0f, 63.0f},
    {385.0f, 63.0f},
    {505.0f, 63.0f},
};

extern "C" {
extern ANIMDATA *lbl_1_bss_F0;
extern ANIMDATA *lbl_1_bss_F4;
}

/* Sets up the owner and clears the four displays' transparency. */
ScoreDisplayOwner::ScoreDisplayOwner()
{
    s32 index;
    ScoreDisplay *entry;

    active = 0;
    lbl_1_bss_F0 = HuSprAnimRead(
        fn_80008DD0(DATANUM(0x510000, 17), 0x10000000, 2));
    HuSprAnimLock(lbl_1_bss_F0);
    lbl_1_bss_F4 = HuSprAnimRead(
        fn_80008DD0(DATANUM(0x9b0000, 48), 0x10000000, 2));
    HuSprAnimLock(lbl_1_bss_F4);
    for (index = 0; index < 4; index++) {
        entry = &displays[index];
        entry->transparency = 0.0f;
    }
}

ScoreDisplayOwner::~ScoreDisplayOwner()
{
    HuSprAnimKill(lbl_1_bss_F0);
    HuSprAnimKill(lbl_1_bss_F4);
}

/* Takes a value for debug output and does nothing with it. */
static inline void DebugValue(f32 value) { }

/* Advances the score display and restores its visible score elements. */
static inline void updateScoreDisplay(ScoreDisplay *display)
{
    f32 fullTransparency;

    if ((display->transparency += 0.1) > 1.0f) {
        display->transparency = 1.0f;
    }

    fullTransparency = display->transparency;
    DebugValue(fullTransparency);
    fullTransparency = 1.0f;
    MgScoreBoxDispSet(display->scoreBox, 1);
    MgScoreBoxTPLvlSet(display->scoreBox, 1.0f);
    MgScoreValueSet(display->score, display->value);
    MgScoreDispOn(display->score);
    MgScoreTPLvlSet(display->score, fullTransparency);
    HuSprAttrReset(display->spriteGroup, 0, 4);
    HuSprAttrReset(display->spriteGroup, 1, 4);
}

ANIMDATA *lbl_1_bss_F0;

ANIMDATA *lbl_1_bss_F4;

void fn_1_8D74(ScoreDisplayOwner *owner, s32 index, s32 characterId)
{
    ScoreDisplay *display;
    s16 spriteGroup;
    MGSCORE *score;

    display = &owner->displays[index];
    display->scoreBox = MgScoreBoxCreateChar(0x72, 0x30, (s16)characterId);
    MgScoreBoxColorSet(display->scoreBox, 0x40, 0x40, 0x40);
    MgScoreBoxTPLvlSet(display->scoreBox, 0.0f);
    spriteGroup = display->spriteGroup = HuSprGrpCreate(3);
    score = display->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, 22.0f + lbl_1_rodata_2E8[index].x,
                  lbl_1_rodata_2E8[index].y);
    MgScoreColorSet(score, 0xFF, 0xD8, 0x15);
    HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(lbl_1_bss_F0, 0x32, 0));
    HuSprPosSet(spriteGroup, 0, -8.0f, 0.0f);
    HuSprAttrSet(spriteGroup, 0, 4);
    HuSprGrpMemberSet(spriteGroup, 1, HuSprCreate(lbl_1_bss_F4, 0x32, 0));
    HuSprPosSet(spriteGroup, 1, 8.0f, 0.0f);
    HuSprBankSet(spriteGroup, 1, 0x0F);
    HuSprColorSet(spriteGroup, 1, 0xFF, 0xFF, 0xFF);
    HuSprAttrSet(spriteGroup, 1, 4);
    display->characterId = characterId;
    display->value = 0;
    display->x = lbl_1_rodata_2E8[index].x;
    display->y = lbl_1_rodata_2E8[index].y;
    MgScoreBoxPosSet(display->scoreBox, display->x, display->y);
    HuSprGrpPosSet(display->spriteGroup, display->x, display->y);
}

void fn_1_8FC8(ScoreDisplayOwner *owner, s32 index, s32 value)
{
    owner->displays[index].value = value;
}

void fn_1_8FD8(void *context, ScoreDisplay *display)
{
    updateScoreDisplay(display);
}

void ScoreDisplayOwner::update()
{
    s32 index;
    ScoreDisplay *display;

    if (active != 0) {
        for (index = 0; index < 4; index++) {
            display = &displays[index];
            updateScoreDisplay(display);
        }
    }
}

void ScoreDisplayOwner::flagEnabled(int request)
{
    flags &= ~(1U << request);
    switch (request) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        active = 1;
        break;
    }
}
