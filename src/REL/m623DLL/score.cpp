// Displays the two groups' scores.

extern "C" {
#include "game/mg/score.h"
#include "game/gamemes.h"
#include "humath.h"
#include "game/memory.h"
}

#include "datanum/mgconst.h"
#include "REL/m623DLL/score_owner.h"
#include "REL/framework/singleton.h"
#include "REL/framework/list_object.h"

// Screen coordinates for the two group score displays.
const HuVec2f lbl_1_rodata_378[2] = { { 150.0f, 418.0f }, { 426.0f, 418.0f } };

// Creates the two score displays and their sprite resources.
MGScoreOwner::MGScoreOwner()
{
    ANIMDATA *scoreBoxAnimation;
    ANIMDATA *digitAnimation;
    MGSCORE *score;
    int index;
    short spriteGroup;
    MGScoreEntry *entry;

    unk10 = 0;

    scoreBoxAnimation = HuSprAnimDataRead(DATANUM(0x520000, 50));
    digitAnimation = HuSprAnimDataRead(MGCONST_ANM_scoreSmall);

    for (index = 0; index < 2; ++index) {
        entry = &entries[index];
        entry->x = lbl_1_rodata_378[index].x;
        entry->y = lbl_1_rodata_378[index].y;
        entry->value = 0;
        entry->box = MgScoreBoxCreate(0x5A, 0x30);
        MgScoreBoxColorSet(entry->box, 0x40, 0x40, 0x40);
        MgScoreBoxTPLvlSet(entry->box, 0.8f);
        MgScoreBoxDispSet(entry->box, FALSE);

        spriteGroup = entry->spriteGroup = HuSprGrpCreate(2);
        score = entry->score = MgScoreCreate(DATANUM(0x9b0000, 49), -1, TRUE);
        MgScoreMaxDigitSet(score, 2);
        MgScoreDispOff(score);
        MgScorePosSet(score, 10.0f + lbl_1_rodata_378[index].x, lbl_1_rodata_378[index].y);
        MgScoreColorSet(score, 0xFF, 0xD8, 0x15);

        HuSprGrpMemberSet(spriteGroup, 0, HuSprCreate(scoreBoxAnimation, 0xC, 0));
        HuSprPosSet(spriteGroup, 0, -27.0f, 0.0f);
        HuSprAttrSet(spriteGroup, 0, HUSPR_ATTR_DISPOFF);
        HuSprGrpMemberSet(spriteGroup, 1, HuSprCreate(digitAnimation, 0xC, 0));
        HuSprPosSet(spriteGroup, 1, -8.0f, 0.0f);
        HuSprBankSet(spriteGroup, 1, 15);
        HuSprColorSet(spriteGroup, 1, 0xFF, 0xFF, 0xFF);
        HuSprAttrSet(spriteGroup, 1, HUSPR_ATTR_DISPOFF);
    }
}

// Updates one score entry's visible digits and box.
void MGScoreOwner::showEntry(MGScoreEntry *entry)
{
    float opacity = 1.0f;
    MgScoreBoxDispSet(entry->box, TRUE);
    MgScoreBoxTPLvlSet(entry->box, (float) (0.8 * (double) opacity));
    MgScoreBoxPosSet(entry->box, entry->x, entry->y);
    MgScoreValueSet(entry->score, entry->value);
    MgScoreDispOn(entry->score);
    MgScoreTPLvlSet(entry->score, opacity);
    HuSprGrpPosSet(entry->spriteGroup, entry->x, entry->y);
    HuSprAttrReset(entry->spriteGroup, 0, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(entry->spriteGroup, 1, HUSPR_ATTR_DISPOFF);
}

// Refreshes both score entries.
void MGScoreOwner::update()
{
    int index = 0;
    for (; index < 2; ++index) {
        MGScoreEntry *entry = &entries[index];
        showEntry(entry);
    }
}

// Clears the request bit and sets the request-2 flag.
void MGScoreOwner::flagEnabled(int request)
{
    flags &= ~(1UL << request);
    switch (request) {
    case 0:
    case 1:
    case 3:
    case 4:
        break;
    case 2:
        unk10 = 1;
        break;
    }
}

// Adds the supplied increment to one score entry.
void fn_1_8A10(unsigned long entryIndex, int increment)
{
    MGScoreOwner *self = &MGScoreOwner::getChecked();
    self->entries[entryIndex].value += increment;
}

// Returns the value of one score entry.
int fn_1_8AAC(unsigned long entryIndex)
{
    MGScoreOwner *self = &MGScoreOwner::getChecked();
    return self->entries[entryIndex].value;
}

// Destroys the score owner.
MGScoreOwner::~MGScoreOwner()
{
}
