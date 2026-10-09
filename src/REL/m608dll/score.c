/* Creates and updates player score displays and the course record display. */
#define _MATH_H
#include "dolphin/math.h"
#include "REL/m608dll.h"

/* Team-mode flag used to conditionally hide the shared record panel. */
#define M608_TEAM_MODE_FLAG 0x30002U

s32 lbl_1_data_410[15] = {
    DATANUM(DATA_mgconst, 0),  DATANUM(DATA_mgconst, 1),  DATANUM(DATA_mgconst, 2),
    DATANUM(DATA_mgconst, 3),  DATANUM(DATA_mgconst, 4),  DATANUM(DATA_mgconst, 5),
    DATANUM(DATA_mgconst, 6),  DATANUM(DATA_mgconst, 7),  DATANUM(DATA_mgconst, 8),
    DATANUM(DATA_mgconst, 9),  DATANUM(DATA_mgconst, 10), DATANUM(DATA_mgconst, 11),
    DATANUM(DATA_mgconst, 12), DATANUM(DATA_mgconst, 13), DATANUM(DATA_mgconst, 69)
};

struct _struct_lbl_1_data_44C_0x14 lbl_1_data_44C[5] = {
    { 90, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 220, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 350, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 480, 420, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 255, 255, 255, 0 },
    { 280, 58, 128, 40, 64, 64, 64, 0, {63, 51, 51, 51}, 66, 255, 122, 0 }
};

struct _struct_lbl_1_bss_3DF0_0x18 lbl_1_bss_3DF0[5];

/* Called during setup to create the four rider panels and shared course-record panel. */
void fn_1_6150(void)
{
    s32 recordSpriteDataIndex;
    MGSCORE *scoreDisplay;
    s16 spriteGroupId;
    s16 scoreBoxId;
    s32 panelIndex;

    panelIndex = 0;
    while (panelIndex < 5) {
        if (panelIndex < 4) {
            scoreBoxId = MgScoreBoxCreateChar(lbl_1_data_44C[panelIndex].boxWidth,
                                              lbl_1_data_44C[panelIndex].boxHeight,
                                              lbl_1_bss_3E80.characterNos[panelIndex]);
            lbl_1_bss_3DF0[panelIndex].scoreBoxId = scoreBoxId;
        } else {
            scoreBoxId = MgScoreBoxCreate(lbl_1_data_44C[panelIndex].boxWidth,
                                          lbl_1_data_44C[panelIndex].boxHeight);
            lbl_1_bss_3DF0[panelIndex].scoreBoxId = scoreBoxId;
        }
        MgScoreBoxPosSet(scoreBoxId, (f32) lbl_1_data_44C[panelIndex].screenX,
                         (f32) lbl_1_data_44C[panelIndex].screenY);
        MgScoreBoxColorSet(scoreBoxId, lbl_1_data_44C[panelIndex].boxRed,
                           lbl_1_data_44C[panelIndex].boxGreen, lbl_1_data_44C[panelIndex].boxBlue);
        scoreDisplay = MgScoreCreate(DATANUM(DATA_mgconst, 48), DATANUM(DATA_mgconst, 48), 0);
        lbl_1_bss_3DF0[panelIndex].scoreDisplay = scoreDisplay;
        MgScoreUnitBankSet(scoreDisplay, 17);
        MgScoreMaxDigitSet(scoreDisplay, 4);
        MgScorePosSet(scoreDisplay, (f32) (lbl_1_data_44C[panelIndex].screenX - 12),
                      (f32) lbl_1_data_44C[panelIndex].screenY);
        MgScoreColorSet(scoreDisplay, lbl_1_data_44C[panelIndex].digitRed,
                        lbl_1_data_44C[panelIndex].digitGreen,
                        lbl_1_data_44C[panelIndex].digitBlue);
        if (panelIndex >= 4) {
            recordSpriteDataIndex = 14;
            lbl_1_bss_3DF0[panelIndex].recordSpriteAnimation = HuSprAnimRead(HuDataSelHeapReadNum(
                lbl_1_data_410[recordSpriteDataIndex], HU_MEMNUM_OVL, HEAP_MODEL));
            spriteGroupId = HuSprGrpCreate(2);
            lbl_1_bss_3DF0[panelIndex].recordSpriteGroup = spriteGroupId;
            lbl_1_bss_3DF0[panelIndex].recordSpriteId =
                HuSprCreate(lbl_1_bss_3DF0[panelIndex].recordSpriteAnimation, 10, 0);
            HuSprGrpMemberSet(spriteGroupId, 0, lbl_1_bss_3DF0[panelIndex].recordSpriteId);
            HuSprGrpPosSet(spriteGroupId, (f32) (lbl_1_data_44C[panelIndex].screenX - 40),
                           (f32) lbl_1_data_44C[panelIndex].screenY);
        }
        panelIndex += 1;
    }
    fn_1_6604();
}

/* Sets the selected rider or record-panel score display; setup initializes the record panel and
 * gameplay updates rider scores. */
void fn_1_6534(s16 scoreIndex, s16 scoreValue)
{
    MgScoreValueSet(lbl_1_bss_3DF0[scoreIndex].scoreDisplay, (s32) scoreValue);
}

/* Called when a rider's rolling turn begins to show the four player score panels. */
void fn_1_6578(void)
{
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[playerIndex].scoreBoxId, 1);
        MgScoreDispOn(lbl_1_bss_3DF0[playerIndex].scoreDisplay);
        /* Only panel 4 creates a sprite group; zero-initialized player IDs still reach this
         * reset. */
        HuSprAttrReset(lbl_1_bss_3DF0[playerIndex].recordSpriteGroup, 0, HUSPR_ATTR_DISPOFF);
        playerIndex += 1;
    }
}

/* Called after panel creation to hide the four rider panels; in team mode, also hides the record
 * panel. */
void fn_1_6604(void)
{
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 4) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[playerIndex].scoreBoxId, 0);
        MgScoreDispOff(lbl_1_bss_3DF0[playerIndex].scoreDisplay);
        playerIndex += 1;
    }
    if (_CheckFlag(M608_TEAM_MODE_FLAG) != 0) {
        MgScoreBoxDispSet(lbl_1_bss_3DF0[4].scoreBoxId, 0);
        MgScoreDispOff(lbl_1_bss_3DF0[4].scoreDisplay);
        HuSprAttrSet(lbl_1_bss_3DF0[4].recordSpriteGroup, 0, HUSPR_ATTR_DISPOFF);
    }
}

/* Called by the record-result process; pulses the new record and saves it at frame 150. */
void fn_1_66C0(OMOBJ *recordObject)
{
    f32 recordScale;
    u32 elapsedFrames;

    recordScale = ((float *) recordObject->work)[0];
    elapsedFrames = recordObject->work[2];
    if ((s32) elapsedFrames >= 150) {
        if ((u32) recordObject->work[1] != 0) {
            recordScale += 0.02f;
            if (recordScale >= 1.2f) {
                recordScale = 1.2f;
                recordObject->work[1] = 0;
            }
        } else {
            recordScale -= 0.04f;
            if (recordScale <= 1.0f) {
                recordScale = 1.0f;
                recordObject->work[1] = 1;
            }
        }
        MgScoreDigitScaleSet(lbl_1_bss_3DF0[4].scoreDisplay, recordScale, recordScale);
        HuSprScaleSet(lbl_1_bss_3DF0[4].recordSpriteGroup, 0, recordScale, recordScale);
        if ((s32) elapsedFrames == 150) {
            /* The panel switches from its pulse to the saved record at this exact frame. */
            GWRecordSet(GW_RECORD_M608, ((s32) lbl_1_bss_3E80.record) * 90);
            {
                s16 score = ((s32) lbl_1_bss_3E80.record) * 90;
                MgScoreValueSet(lbl_1_bss_3DF0[4].scoreDisplay, score);
            }
        }
    }
    ((float *) recordObject->work)[0] = recordScale;
    recordObject->work[2] = elapsedFrames + 1;
}
