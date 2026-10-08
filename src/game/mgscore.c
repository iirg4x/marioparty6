// Creates and updates the numeric score displays used by minigames.
#define _MATH_H
#include "game/data.h"
#include "game/process.h"
#include "game/mg/score.h"
#include "game/sprite.h"

#define SCORE_UNIT_SPRNO (MGSCORE_DIGIT_MAX)
#define SCORE_SPRMAX (MGSCORE_DIGIT_MAX+1)
#define SCORE_MILLIONS_PLACE 1000000

static const float unitOfsTbl[3][4] = {
    8.0f, 28.0f, 28.0f, 8.0f,
    10.0f, 34.0f, 34.0f, 10.0f,
    12.0f, 40.0f, 40.0f, 12.0f
};

static void ScoreExec(MGSCORE *score);
static void ScoreMain();
static void ScoreDispUpdate(MGSCORE *score);

typedef void (*SCOREFUNC)(MGSCORE *score);

static SCOREFUNC modeTbl[1] = { ScoreExec };

// Creates a score display with digit art and optional unit art for a minigame.
MGSCORE *MgScoreCreate(int digitFile, int unitFile, BOOL showLeadingZeros) {
    MGSCORE *score;
    int unitStyle;
    int unitBank;
    unitStyle = -1;
    unitBank = 0;
    // Known unit files use bank 1; unrecognized unit files use bank 0.
    unitBank = 1;
    score = MgScoreInit(digitFile);
    if (score == NULL) {
        return NULL;
    }
    if (unitFile  != -1) {
        score->sprId[SCORE_UNIT_SPRNO] = (int)HuSprCreate(HuSprAnimDataRead(unitFile), 0, 0);
        HuSprGrpMemberSet(score->grpId, SCORE_UNIT_SPRNO, score->sprId[SCORE_UNIT_SPRNO]);
    }
    switch (unitFile) {
        case -1:
            break;

        case DATANUM(DATA_mgconst, 0x35):
        case DATANUM(DATA_mgconst, 0x36):
            // These two unit files share the first offset layout.
            unitStyle = 0;
            break;

        case DATANUM(DATA_mgconst, 0x37):
            unitStyle = 1;
            break;

        case DATANUM(DATA_mgconst, 0x38):
            unitStyle = 2;
            break;

        default:
            unitStyle = 3;
            break;
    }
    if (unitStyle == 3) {
        MgScoreUnitBankSet(score, 0);
        score->unitOfs.x = HuSprData[score->sprId[SCORE_UNIT_SPRNO]].data->pat->centerX;
        score->unitOfs.y = HuSprData[score->sprId[SCORE_UNIT_SPRNO]].data->pat->centerY;
    } else if (unitFile != -1) {
        MgScoreUnitBankSet(score, unitBank);
        score->unitOfs.x = unitOfsTbl[unitStyle][unitBank];
        score->unitOfs.y = unitOfsTbl[unitStyle][3];
    }
    score->dispLeadZeroF = showLeadingZeros;
    MgScorePriSet(score, 90);
    return score;
}

// Allocates the digit sprites and starts the child process that refreshes this display.
MGSCORE *MgScoreInit(int digitFile) {
    MGSCORE *score;
    int digitIndex;

    score = HuMemDirectMallocNum(HEAP_HEAP, sizeof(MGSCORE), HU_MEMNUM_OVL);
    if (score == NULL) {
        return NULL;
    }
    score->pos.x = 0.0f;
    score->pos.y = 0.0f;
    score->scale.x = 1.0f;
    score->scale.y = 1.0f;
    score->digitScale.x = 1.0f;
    score->digitScale.y = 1.0f;
    score->zRot = 0.0f;
    score->tpLvl = 1.0f;
    score->mode = 0;
    score->value = 0;
    score->maxDigit = 5;
    score->r = 255;
    score->g = 216;
    score->b = 21;
    score->dispF = 1;
    score->validF = 0;
    score->dispLeadZeroF = 0;
    score->unitOfs.x = 0.0f;
    score->unitOfs.y = 0.0f;
    score->grpId = HuSprGrpCreate(SCORE_SPRMAX);
    score->sprId = HuMemDirectMallocNum(HEAP_HEAP, SCORE_SPRMAX*sizeof(HUSPRID), HU_MEMNUM_OVL);
    score->digitAnim = HuSprAnimDataRead(digitFile);

    for (digitIndex = 0; digitIndex < MGSCORE_DIGIT_MAX; digitIndex++) {
        score->sprId[digitIndex] = (int)HuSprCreate(score->digitAnim, 0, 0);
        HuSprGrpMemberSet(score->grpId, digitIndex, score->sprId[digitIndex]);
    }
    score->sprId[MGSCORE_DIGIT_MAX] = HUSPR_NONE;
    score->digitW = HuSprData[score->sprId[0]].data->pat->sizeX;
    MgScorePriSet(score, 90);
    ScoreDispUpdate(score);
    score->proc = HuPrcChildCreate(ScoreMain, 0x1000, 0x1000, 0, HuPrcCurrentGet());
    score->proc->property = score;
    return score;
}

// Removes the score's sprite group and update process, then frees its storage.
void MgScoreKill(MGSCORE *score) {
    HuSprGrpKill(score->grpId);
    HuPrcKill(score->proc);
    HuMemDirectFree(score->sprId);
    HuMemDirectFree(score);
}

int MgScoreModeGet(MGSCORE *score) {
    return score->mode;
}

// Sets the displayed integer and marks the sprite layout for refresh.
void MgScoreValueSet(MGSCORE *score, int value) {
    score->value = value;
    score->validF = FALSE;
}

int MgScoreValueGet(MGSCORE *score) {
    return score->value;
}

// Sets the score group's screen position in pixels.
void MgScorePosSet(MGSCORE *score, float posX, float posY) {
    score->pos.x = posX;
    score->pos.y = posY;
    score->validF = FALSE;
}

void MgScorePosGet(MGSCORE *score, float *posX, float *posY) {
    *posX = score->pos.x;
    *posY = score->pos.y;
}

// Sets the scale applied to the whole score group.
void MgScoreScaleSet(MGSCORE *score, float scaleX, float scaleY) {
    score->scale.x = scaleX;
    score->scale.y = scaleY;
    score->validF = FALSE;
}

void MgScoreScaleGet(MGSCORE *score, float *scaleX, float *scaleY) {
    *scaleX = score->scale.x;
    *scaleY = score->scale.y;
}

// Sets the scale applied to each digit and the optional unit sprite.
void MgScoreDigitScaleSet(MGSCORE *score, float scaleX, float scaleY) {
    score->digitScale.x = scaleX;
    score->digitScale.y = scaleY;
    score->validF = FALSE;
}

// Sets the score group's rotation around the screen Z axis, in radians.
void MgScoreZRotSet(MGSCORE *score, float zRot) {
    score->zRot = zRot;
    score->validF = FALSE;
}

void MgScoreZRotGet(MGSCORE *score, float *zRot) {
    *zRot = score->zRot;
}

// Sets the transparency level passed to the score sprites (1.0 is opaque).
void MgScoreTPLvlSet(MGSCORE *score, float tpLvl) {
    score->tpLvl = tpLvl;
    score->validF = FALSE;
}

void MgScoreTPLvlGet(MGSCORE *score, float *tpLvl) {
    *tpLvl = score->tpLvl;
}

// Sets the RGB tint applied to the digits and optional unit sprite.
void MgScoreColorSet(MGSCORE *score, u8 red, u8 green, u8 blue) {
    score->r = red;
    score->g = green;
    score->b = blue;
    score->validF = FALSE;
}

// Sets the slot count used for alignment and, with leading zeros, limits visible positions to the
// rightmost slots.
void MgScoreMaxDigitSet(MGSCORE *score, int maxDigit) {
    score->maxDigit = maxDigit;
    score->validF = FALSE;
}

void MgScoreMaxDigitGet(MGSCORE *score, int *maxDigit) {
    *maxDigit = score->maxDigit;
}

// Sets the horizontal spacing between adjacent digit sprites, in pixels.
void MgScoreDigitWidthSet(MGSCORE *score, float digitWidth) {
    score->digitW = digitWidth;
    score->validF = FALSE;
}

void MgScoreDigitWidthGet(MGSCORE *score, float *digitW) {
    *digitW = score->digitW;
}

// Enables the score sprites on the next display refresh.
void MgScoreDispOn(MGSCORE *score) {
    score->dispF = TRUE;
    score->validF = FALSE;
}

// Hides the score sprites on the next display refresh.
void MgScoreDispOff(MGSCORE *score) {
    score->dispF = FALSE;
    score->validF = FALSE;
}

// Sets the draw priority of every digit and the optional unit sprite.
void MgScorePriSet(MGSCORE *score, s16 priority) {
    int spriteIndex;
    score->prio = priority;
    for (spriteIndex = 0; spriteIndex < SCORE_SPRMAX; spriteIndex++) {
        if (score->sprId[spriteIndex] != HUSPR_NONE) {
            HuSprPriSet(score->grpId, spriteIndex, priority);
        }
    }
}

// Selects the animation bank used by the optional unit sprite, when present.
void MgScoreUnitBankSet(MGSCORE *score, s16 bank) {
    if (score->sprId[SCORE_UNIT_SPRNO] != HUSPR_NONE) {
        HuSprBankSet(score->grpId, SCORE_UNIT_SPRNO, bank);
    }
}

// Runs as the score's child process and dispatches its selected update mode.
static void ScoreMain(void)
{
    MGSCORE *score;

    score = HuPrcCurrentGet()->property;
    while (TRUE) {
        modeTbl[score->mode](score);
    }
}

// Selects the standard mode, which refreshes the score once per frame.
void MgScoreModeDefaultSet(MGSCORE *score) {
    score->mode = 0;
}

// Runs from modeTbl in the score child process, refreshing until the selected mode changes.
static void ScoreExec(MGSCORE *score)
{
    s32 framePass;
    s16 mode;
    s32 passLimit;
    s16 prevMode;

    while (1) {
        // The mode-table callback refreshes once per frame; it reads the active process property.
        for(passLimit=1, framePass=0; framePass<passLimit; framePass++) {
            ScoreDispUpdate(HuPrcCurrentGet()->property);
            HuPrcVSleep();

            mode = ((MGSCORE *)HuPrcCurrentGet()->property)->mode;
            prevMode = ((MGSCORE *)HuPrcCurrentGet()->property)->prevMode;
            ((MGSCORE *)HuPrcCurrentGet()->property)->prevMode = mode;

            if (mode != prevMode) {
                return;
            }
        }
    }
}

// Rebuilds digit banks and sprite transforms after a score setting changes.
static void ScoreDispUpdate(MGSCORE *score)
{
    int digitIndex;
    int digitCount;
    int placeValue;
    int remainingValue;
    int currentDigit;
    BOOL significantDigitSeen;
    float digitPosX;
    int digitBank[MGSCORE_DIGIT_MAX];

    if (score->validF == 0) {
        digitIndex = 0;
        remainingValue = score->value;
        // Seven digit scores begin at the millions place.
        placeValue = SCORE_MILLIONS_PLACE;
        digitCount = 0;
        significantDigitSeen = 0;
        if (remainingValue == 0) {
            if (score->dispLeadZeroF) {
                for (digitIndex = 0; digitIndex < MGSCORE_DIGIT_MAX; digitIndex++) {
                    digitBank[digitIndex] = 0;
                    digitCount++;
                }
            } else {
                digitBank[digitIndex++] = 0;
                digitCount++;
            }
        } else {
            do {
                currentDigit = remainingValue / placeValue;
                // Remove the extracted place so the next pass reads the following decimal digit.
                remainingValue = remainingValue - (currentDigit * placeValue);
                if ((currentDigit > 0) || (significantDigitSeen) || (score->dispLeadZeroF)) {
                    significantDigitSeen = 1;
                    digitBank[digitIndex++] = currentDigit;
                    digitCount++;
                }
                placeValue = placeValue / 10;
            } while (placeValue > 0);
        }
        while (digitIndex < MGSCORE_DIGIT_MAX) {
            // -1 marks unused sprite slots; those sprites are hidden during layout below.
            digitBank[digitIndex] = -1;
            digitIndex++;
        }
        digitPosX = score->digitW * (score->maxDigit - digitCount);

        for (digitIndex = 0; digitIndex < MGSCORE_DIGIT_MAX; digitIndex++) {
            HuSprPosSet(score->grpId, digitIndex, digitPosX, 0.0f);
            digitPosX += score->digitW;
            // With leading zeros, only the configured rightmost digit positions are shown.
            if ((digitBank[digitIndex] != -1) &&
                ((score->dispLeadZeroF == 0) ||
                 (digitIndex >= (MGSCORE_DIGIT_MAX - score->maxDigit)))) {
                HuSprBankSet(score->grpId, digitIndex, digitBank[digitIndex]);
                if (score->dispF) {
                    HuSprDispOn(score->grpId, digitIndex);
                } else {
                    HuSprDispOff(score->grpId, digitIndex);
                }
                HuSprScaleSet(score->grpId, digitIndex, score->digitScale.x, score->digitScale.y);
                HuSprColorSet(score->grpId, digitIndex, score->r, score->g, score->b);
            } else {
                HuSprDispOff(score->grpId, digitIndex);
            }
        }
        if (score->sprId[SCORE_UNIT_SPRNO] != HUSPR_NONE) {
            // The unit's horizontal position uses unitOfs.y as the trailing adjustment.
            HuSprPosSet(score->grpId, SCORE_UNIT_SPRNO,
                        (score->unitOfs.x + (score->maxDigit * score->digitW)) - score->unitOfs.y,
                        0.0f);
            HuSprColorSet(score->grpId, SCORE_UNIT_SPRNO, score->r, score->g, score->b);
            HuSprScaleSet(score->grpId, SCORE_UNIT_SPRNO, score->digitScale.x, score->digitScale.y);
            if (score->dispF) {
                HuSprDispOn(score->grpId, SCORE_UNIT_SPRNO);
            } else {
                HuSprDispOff(score->grpId, SCORE_UNIT_SPRNO);
            }
        }
        HuSprGrpPosSet(score->grpId, score->pos.x, score->pos.y);
        HuSprGrpZRotSet(score->grpId, score->zRot);
        HuSprGrpScaleSet(score->grpId, score->scale.x, score->scale.y);
        HuSprGrpTPLvlSet(score->grpId, score->tpLvl);
        score->validF = 1;
    }
}
