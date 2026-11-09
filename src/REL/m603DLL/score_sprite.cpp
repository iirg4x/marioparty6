#include "REL/m603DLL/score_owner.h"
#include "REL/m603DLL/sprite_fade.h"
#include "datadir_enum.h"
#include "msm_se.h"
#include "REL/m603DLL/singleton_32930.h"
extern "C" {
#include "game/data.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/mg/score.h"
#include "game/audio.h"
#include "game/gamemes.h"
}













struct ScorePosition { f32 x, y; };
extern "C" const ScorePosition lbl_1_rodata_310[4] = {
    {66.0f, 206.0f}, {511.0f, 206.0f}, {66.0f, 408.0f}, {511.0f, 408.0f}
};

ScoreDisplay::ScoreDisplay()
{
    s32 index;
    PlayerScoreEntry *entry;
    active = 0;
    soundPending = 0;
    for (index = 0; index < 4; index++) {
        entry = &entries[index];
        entry->initializedFloat = 0.0f;
    }
}

void ScoreDisplay::fn_1_8268(s32 index, s32 character)
{
    PlayerScoreEntry *entry = &entries[index];
    MGSCORE *score;
    entry->boxId = MgScoreBoxCreateChar(0x5C, 0x30, (s16)character);
    MgScoreBoxColorSet(entry->boxId, 0x40, 0x40, 0x40);
    MgScoreBoxTPLvlSet(entry->boxId, 0.8f);
    MgScoreBoxDispSet(entry->boxId, 0);
    score = entry->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, 1);
    MgScoreMaxDigitSet(score, 2);
    MgScoreDispOff(score);
    MgScorePosSet(score, 7.0f + lbl_1_rodata_310[index].x, lbl_1_rodata_310[index].y);
    MgScoreColorSet(score, 0xFF, 0xD8, 0x15);
    entry->character = character;
    entry->value = 0;
    entry->x = lbl_1_rodata_310[index].x;
    entry->y = lbl_1_rodata_310[index].y;
    MgScoreBoxPosSet(entry->boxId, entry->x, entry->y);
    active = 1;
}

void ScoreDisplay::fn_1_83CC(int index, int value)
{
    entries[index].value += value;
}

unsigned int ScoreDisplay::fn_1_83E4(unsigned int index)
{
    return entries[index].value;
}

void ScoreDisplay::fn_1_83F4()
{
    s32 index;
    for (index = 0; index < 4; index++) {
        PlayerScoreEntry *box = &entries[index];
        box->x = (f32)(288.0 + (120.0f * (f32)index - 180.0));
        box->y = 408.0f;
        MgScoreBoxPosSet(box->boxId, box->x, box->y);
        MgScorePosSet(box->score, 7.0f + box->x, box->y);
    }
}

void fn_1_84E4(void *, PlayerScoreEntry *entry)
{
    MgScoreBoxDispSet(entry->boxId, 1);
    MgScoreBoxTPLvlSet(entry->boxId, 1.0f);
    MgScoreValueSet(entry->score, entry->value);
    MgScoreDispOn(entry->score);
    MgScoreTPLvlSet(entry->score, 1.0f);
}

void ScoreDisplay::extra()
{
    int index;
    if (soundPending != 0) {
        HuAudFXPlay(MSM_SE_MG603_SCORE_EFFECT);
        soundPending = 0;
    }
    if (active != 0) {
        for (index = 0; index < 4; index++) {
            PlayerScoreEntry *entry = &entries[index];
            MgScoreBoxDispSet(entry->boxId, 1);
            MgScoreBoxTPLvlSet(entry->boxId, 1.0f);
            MgScoreValueSet(entry->score, entry->value);
            MgScoreDispOn(entry->score);
            MgScoreTPLvlSet(entry->score, 1.0f);
        }
    }
}

void ScoreDisplay::update(unsigned long request)
{
    state &= ~(1UL << request);
    switch (request) {
    case 0: case 1: case 2: case 3: case 4: break;
    }
}

extern "C" s32 lbl_1_data_340[14] = {
    DATANUM(0x3e0000, 38), DATANUM(0x3e0000, 39), DATANUM(0x3e0000, 40),
    DATANUM(0x3e0000, 41), DATANUM(0x3e0000, 42), DATANUM(0x3e0000, 43),
    DATANUM(0x3e0000, 44), DATANUM(0x3e0000, 45), DATANUM(0x3e0000, 46),
    DATANUM(0x3e0000, 47), DATANUM(0x3e0000, 48), DATANUM(0x3e0000, 49),
    DATANUM(0x3e0000, 50), DATANUM(0x3e0000, 51)
};
extern "C" const ScorePosition lbl_1_rodata_36C[4] = {
    {49.0f, 208.0f}, {528.0f, 208.0f}, {49.0f, 410.0f}, {528.0f, 410.0f}
};

SpriteFade::SpriteFade(const s32 *animationIds)
{
    ANIMDATA *animations[4];
    ANIMDATA *baseAnimation;
    s32 index;
    s32 index2;
    reverse = 0;
    transitionState = 1;
    initializedValue = 0;
    level = 0.0f;
    baseAnimation = HuSprAnimRead(fn_80008DD0(DATANUM(0x3e0000, 37), 0x10000000, 2));
    for (index = 0; index < 4; index++) {
        animations[index] = HuSprAnimRead(fn_80008DD0(
            lbl_1_data_340[animationIds[index]], 0x10000000, 2));
    }
    for (index2 = 0; index2 < 4; index2++) {
        FadeSpriteGroup *group = &groups[index2];
        HUSPR_GROUPID groupId;
        s32 member;
        group->x = lbl_1_rodata_36C[index2].x;
        group->y = lbl_1_rodata_36C[index2].y;
        group->groupId = groupId = HuSprGrpCreate(2);
        HuSprGrpMemberSet(groupId, 1, HuSprCreate(animations[index2], 0x64, 0));
        HuSprGrpMemberSet(groupId, 0, HuSprCreate(baseAnimation, 0xC8, 0));
        HuSprGrpPosSet(group->groupId, group->x, group->y);
        for (member = 0; member < 2; member++) {
            HuSprAttrSet(groupId, (s16)member, 1);
            HuSprAttrSet(groupId, (s16)member, 4);
        }
    }
}

SpriteFade::~SpriteFade()
{
    FadeSpriteGroup *sprite;
    int spriteIndex;
    for (spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
        sprite = &groups[spriteIndex];
        HuSprGrpKill(sprite->groupId);
    }
}

void SpriteFade::extra()
{
    if (transitionState != 0) {
        hold();
        return;
    }
    advance();
}

void SpriteFade::hold() {}

void SpriteFade::advance()
{
    s32 group;
    s16 *sprite;
    s32 member;
    if (reverse == 0) {
        level += 0.033333335f;
        if (level > 1.0) level = 1.0f;
    } else {
        level -= 0.033333335f;
        if (level < 0.0f) {
            delete this;
            return;
        }
    }
    for (group = 0; group < 4; group++) {
        sprite = &groups[group].groupId;
        for (member = 0; member < 2; member++) {
            HuSprTPLvlSet(sprite[0], (s16)member, level);
            HuSprAttrReset(sprite[0], (s16)member, 4);
        }
    }
}

void SpriteFade::update(unsigned long request)
{
    state &= ~(1UL << request);
    switch (request) {
    case 0: case 1: case 2: case 4: break;
    case 3: reverse = 1; break;
    }
}

ScoreDisplay::~ScoreDisplay() {}
