/* Option Mode screens for board records, prizes, minigames, and record lists. */
#include "dolphin.h"
#include "game/audio.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include <math.h>

#define OPTION_LAST_CHARACTER_BANK_FLAG 2
#define OPTION_LAST_BOARD_BANK_FLAG 51

void fn_1_C3FC(s16 selection);
s16 fn_1_C324(s16 flag, s16 type);

typedef void (*VoidFunc)(void);

typedef struct OptionMessageEntry OptionMessageEntry;

typedef struct OptionAnimPair {
    s16 animId1;
    s16 animId2;
    s16 modelId;
} OptionAnimPair;

typedef struct OptionDecaRecord {
    s16 character;
    s16 finalScore;
    s16 score[10];
} OptionDecaRecord;

typedef struct OptionRecordDisplayEntry {
    s16 minigameId;
    u8 recordId;
    u8 displayType;
    s32 message;
} OptionRecordDisplayEntry;

typedef struct OptionSelectionData {
    s16 selectedIndex;
    s16 optionCount;
    s16 animationA[10];
    s16 choiceAnimationBank[10];
} OptionSelectionData;

typedef struct OptionWork {
    s16 rumbleEnabled; /* Saved controller vibration setting. */
    s16 micStatus;
    s16 soundMode;
    s16 boardRecord[6];
    s16 boardCharacterRecord[6][11];
    OptionDecaRecord decaRecord[10];
    s32 prizeUnlocked[42];
    s16 prizeId[42];
    u32 prizeName[42];
    u32 prizeDescription[42];
    s32 minigameUnlocked[81];
    u32 record[19];
    u8 consecutiveRecordCount;
    u8 consecutiveRecord[100];
} OptionWork;

char lbl_1_data_450[18] = "gN01m1-itemhook_R";

char lbl_1_data_462[8] = "ys77060";

char lbl_1_data_46A[8] = "ys77061";

char lbl_1_data_472[7] = "dpic00";

char lbl_1_data_479[8] = "dpic01";

char lbl_1_data_481[5] = "%05d";

char lbl_1_data_486[6] = "%d";

OptionRecordDisplayEntry lbl_1_data_48C[9] = {
    {606, 0, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {608, 1, 1, MESSNUM(MESS_SAF_TEST, 25)},
    {618, 2, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {638, 3, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {648, 4, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {650, 5, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {652, 6, 0, MESSNUM(MESS_SAF_TEST, 23)},
    {678, 7, 1, MESSNUM(MESS_SAF_TEST, 24)},
    {-1, 0, 0, 0},
};

char lbl_1_data_4D4[4] = "%2d";

char lbl_1_data_4D8[5] = "%02d";

char lbl_1_data_4DD[5] = "%8d";

s16 lbl_1_data_4E2[7] = {0, 1, 2, 3, 6, -1, 0};

u32 lbl_1_data_4F0[8] = {
    MESSNUM(MESS_OPTION_MESS, 52), MESSNUM(MESS_OPTION_MESS, 53),
    MESSNUM(MESS_OPTION_MESS, 54), MESSNUM(MESS_OPTION_MESS, 55),
    MESSNUM(MESS_OPTION_MESS, 56), MESSNUM(MESS_OPTION_MESS, 57),
    MESSNUM(MESS_OPTION_MESS, 58), 0
};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern OptionWork lbl_1_bss_8;
extern s16 lbl_1_bss_5D4;
extern s16 lbl_1_bss_66E[];
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_6A0;
extern u32 lbl_1_bss_848[];
extern char lbl_1_bss_9D8[5][4];
extern s16 lbl_1_bss_9EC[7][2];
/* Scratch space for formatting seven record rows in 30-byte segments. */
extern char lbl_1_bss_A08[216];
/* Each visible row uses a 30-byte segment of the shared formatting buffer. */
#define OPTION_RECORD_TEXT(row) (((char (*)[30])lbl_1_bss_A08)[row])

void fn_1_A0(void);
void fn_1_804(void);
void fn_1_FDC(void);
void fn_1_1044(void);
void fn_1_EA6C(void);
void fn_1_1084(void);
void fn_1_3AE0(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
s16 fn_1_3318(s16 modelId, OptionSelectionData *selection, u32 *messages);
void fn_1_18B8(s16 cameraNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
void fn_1_3BD8(OMOBJ *obj);
s16 fn_1_1DF0(s16 currentOption);

void fn_1_B1A0(s16 mode, s16 argument);
void fn_1_4738(void);
void fn_1_6898(void);
void fn_1_5620(void);
void fn_1_6504(s16 prizeNo);
void fn_1_6590(s16 firstPrize);
void fn_1_7728(s16 firstEntry);
void fn_1_7A74(void);
void fn_1_85E4(s16 item, float progress);
void fn_1_879C(void);
void fn_1_8CA4(s16 recordIndex);
void fn_1_930C(u32 *messages, s16 entryCount, u32 titleMessage);
void fn_1_C3FC(s16 selection);
float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_B02C(s16 first, s16 count);
void fn_1_9F74(void);
void fn_1_C484(void);
void fn_1_E538(s16 page);
extern int HuMCMicGet(void);
int sprintf(char *buffer, const char *format, int value);

/* Called by the Option Mode menu to show board and character record totals. */
void fn_1_4738(void)
{
    s16 board;
    s16 item;
    s16 digit;
    s16 amount;
    s16 divisor;
    s16 digitValue;
    s16 delta;
    s16 titleWindow;
    s16 countWindow;
    char countText[8];
    float progress;
    float x;
    float y;
    float scale;

    board = 0;
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 5, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 5, -205.0f, -110.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 6, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 6, 205.0f, -110.0f);
    for (item = 0; item < 11; item++) {
        HuSprAttrReset(lbl_1_bss_5D4, item + 8, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_5D4, item + 8, 1.0f, 1.0f);
        if (item == 10 && GWBankFlagGet(OPTION_LAST_CHARACTER_BANK_FLAG) == FALSE)
            HuSprBankSet(lbl_1_bss_5D4, item + 8, 11);
        else HuSprBankSet(lbl_1_bss_5D4, item + 8, item);
        if (item < 6) {
            x = item * 80 - 200;
            y = -30.0f;
            HuSprPosSet(lbl_1_bss_5D4, item + 8, x, y);
        } else {
            x = (item - 6) * 80 - 160;
            y = 70.0f;
            HuSprPosSet(lbl_1_bss_5D4, item + 8, x, y);
        }
        amount = lbl_1_bss_8.boardCharacterRecord[board][item];
        for (digit = 0, divisor = 100; digit < 3; digit++, divisor /= 10) {
            digitValue = amount / divisor;
            if (digitValue > 9) digitValue = 9;
            HuSprAttrReset(lbl_1_bss_5D4, item * 3 + 19 + digit, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(lbl_1_bss_5D4, item * 3 + 19 + digit,
                8.0f + (x - 24.0f) + digit * 16,
                45.0f + y);
            HuSprBankSet(lbl_1_bss_5D4, item * 3 + 19 + digit, digitValue);
            amount -= divisor * (amount / divisor);
        }
    }
    titleWindow = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(titleWindow, 10);
    HuWinBGTPLvlSet(titleWindow, 0.0f);
    HuWinAttrSet(titleWindow, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(titleWindow, 0);
    HuWinMesSet(titleWindow, MESSNUM(MESS_OPTION_MESS, 68) + board);
    countWindow = HuWinCreate(HUWIN_POS_CENTER, 95.0f, 400, 40, 0);
    HuWinPriSet(countWindow, 10);
    HuWinBGTPLvlSet(countWindow, 0.0f);
    HuWinAttrSet(countWindow, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(countWindow, 0);
    sprintf(countText, lbl_1_data_481, lbl_1_bss_8.boardRecord[board]);
    HuWinInsertMesSet(countWindow, (u32)countText, 0);
    HuWinMesSet(countWindow, MESSNUM(MESS_SAF_TEST, 22));
    HuAudFXPlay(MSM_SE_MENU_38);
    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(titleWindow, 288.0f - 200.0f * progress, 200.0f - 147.0f * progress);
        HuWinScaleSet(titleWindow, progress, progress);
        HuWinPosSet(countWindow, 288.0f - 200.0f * progress, 200.0f - 105.0f * progress);
        HuWinScaleSet(countWindow, progress, progress);
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 21));
    HuWinDispOn(lbl_1_bss_6A0);
    for (;;) {
        delta = 0;
        if (HuPadBtnDown[0] & 0x4000) delta = -1;
        if (HuPadBtnDown[0] & 0x2000) delta = 1;
        if (delta != 0) {
            board += delta;
            if (board == 5 && GWBankFlagGet(OPTION_LAST_BOARD_BANK_FLAG) == FALSE) board += delta;
            if (board < 0) {
                board = 6;
                /* The totals page is 6, so the board-5 branch below is never taken. */
                if (board == 5 && GWBankFlagGet(OPTION_LAST_BOARD_BANK_FLAG) == FALSE) board = 4;
            }
            if (board >= 7) board = 0;
            if (delta < 0) HuSprBankSet(lbl_1_bss_5D4, 5, 1);
            else HuSprBankSet(lbl_1_bss_5D4, 6, 1);
            HuAudFXPlay(MSM_SE_MENU_40);
            HuPrcSleep(3);
            if (board < 6) {
                for (item = 19; item < 69; item++) {
                    HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
                }
                for (item = 0; item < 11; item++) {
                    if (item < 6) {
                        x = item * 80 - 200;
                        y = -30.0f;
                    } else {
                        x = (item - 6) * 80 - 160;
                        y = 70.0f;
                    }
                    amount = lbl_1_bss_8.boardCharacterRecord[board][item];
                    for (digit = 0, divisor = 100; digit < 3;
                         digit++, divisor /= 10) {
                        digitValue = amount / divisor;
                        if (digitValue > 9) digitValue = 9;
                        HuSprAttrReset(lbl_1_bss_5D4,
                            item * 3 + 19 + digit, HUSPR_ATTR_DISPOFF);
                        HuSprPosSet(lbl_1_bss_5D4,
                            item * 3 + 19 + digit,
                            8.0f +
                                (x - 24.0f) + digit * 16,
                            45.0f + y);
                        HuSprBankSet(lbl_1_bss_5D4,
                            item * 3 + 19 + digit, digitValue);
                        amount -= divisor * (amount / divisor);
                    }
                }
                HuWinMesSet(titleWindow, MESSNUM(MESS_OPTION_MESS, 68) + board);
                sprintf(countText, lbl_1_data_481,
                    lbl_1_bss_8.boardRecord[board]);
            } else {
                for (item = 0; item < 11; item++) {
                    if (item < 6) {
                        x = item * 80 - 200;
                        y = -30.0f;
                    } else {
                        x = (item - 6) * 80 - 160;
                        y = 70.0f;
                    }
                    for (digit = amount = 0; digit < 6; digit++) {
                        amount +=
                            lbl_1_bss_8.boardCharacterRecord[digit][item];
                    }
                    for (digit = 0, divisor = 1000; digit < 4;
                         digit++, divisor /= 10) {
                        digitValue = amount / divisor;
                        if (digitValue > 9) digitValue = 9;
                        HuSprAttrReset(lbl_1_bss_5D4,
                            item * 4 + 19 + digit, HUSPR_ATTR_DISPOFF);
                        HuSprPosSet(lbl_1_bss_5D4,
                            item * 4 + 19 + digit,
                            8.0f +
                                (x - 32.0f) + digit * 16,
                            45.0f + y);
                        HuSprBankSet(lbl_1_bss_5D4,
                            item * 4 + 19 + digit, digitValue);
                        amount -= divisor * (amount / divisor);
                    }
                }
                HuWinMesSet(titleWindow, MESSNUM(MESS_OPTION_MESS, 51));
                for (digit = amount = 0; digit < 6; digit++) {
                    amount += lbl_1_bss_8.boardRecord[digit];
                }
                sprintf(countText, lbl_1_data_481, amount);
            }
            HuWinInsertMesSet(countWindow, (u32)countText, 0);
            HuWinMesSet(countWindow, MESSNUM(MESS_SAF_TEST, 22));
            HuPrcSleep(3);
            HuSprBankSet(lbl_1_bss_5D4, 5, 0);
            HuSprBankSet(lbl_1_bss_5D4, 6, 0);
        }
        if (HuPadBtnDown[0] & 0x200) { HuAudFXPlay(MSM_SE_CMN_04); break; }
        HuPrcVSleep();
    }
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        scale = 1.0 - progress;
        HuSprGrpScaleSet(lbl_1_bss_5D4, scale, scale);
        HuWinPosSet(titleWindow, 288.0f - 200.0f * scale, 200.0f - 147.0f * scale);
        HuWinScaleSet(titleWindow, scale, scale);
        HuWinPosSet(countWindow, 288.0f - 200.0f * scale, 200.0f - 105.0f * scale);
        HuWinScaleSet(countWindow, scale, scale);
        HuPrcVSleep();
    }
    HuWinKill(titleWindow);
    HuWinKill(countWindow);
    for (item = 0; item < 105; item++) HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Opens the prize collection, showing names and values for unlocked prizes and placeholders for
 * locked ones. */
void fn_1_5620(void)
{
    s16 count;
    s16 item;
    s16 frame;
    s16 selected;
    s16 pageStart;
    s16 window;
    s16 delta;
    s16 repeatCount;
    s16 animationFrames;
    float progress;
    float lowerArrowScale;
    float upperArrowScale;
    float scale;

    pageStart = 0;
    repeatCount = 0;
    count = 42;

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -90.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);

    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, MESSNUM(MESS_OPTION_MESS, 45));

    for (item = 0; item < 7; item++) {
        lbl_1_bss_9EC[item][0] = HuWinCreate(80.0f,
            item * 27 + 120, 400, 40, 0);
        lbl_1_bss_9EC[item][1] = HuWinCreate(425.0f,
            item * 27 + 120, 177, 40, 0);
        HuWinMesSet(lbl_1_bss_9EC[item][0], MESSNUM(MESS_MG_NAME, 0) + item);
        HuWinMesSet(lbl_1_bss_9EC[item][1], MESSNUM(MESS_SAF_TEST, 23));
        for (frame = 0; frame < 2; frame++) {
            HuWinPriSet(lbl_1_bss_9EC[item][frame], 10);
            HuWinBGTPLvlSet(lbl_1_bss_9EC[item][frame],
                0.0f);
            HuWinMesSpeedSet(lbl_1_bss_9EC[item][frame], 0);
        }
    }
    fn_1_6590(0);

    if (count > 7) {
        HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f,
            1.0f);
    }
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    HuAudFXPlay(MSM_SE_MENU_38);

    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 228.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - (-137.0f) * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 18));
    HuWinDispOn(lbl_1_bss_6A0);
    selected = 0;
    fn_1_C3FC(selected);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 17.0f,
        1.0f);
    fn_1_6504(selected + pageStart);
    fn_1_19B8(1);
    for (;;) {
        delta = 0;
        if (HuPadDStkRep[0] & 8) {
            if (pageStart + selected > 0) {
                delta = -1;
            }
        }
        if (HuPadDStkRep[0] & 4) {
            if (pageStart + selected < count - 1) {
                delta = 1;
            }
        }
        if ((HuPadDStk[0] & 0xC) == 0) {
            repeatCount = 0;
        }
        if (delta != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            selected += delta;
            if (selected < 0 || selected >= 7) {
                if (selected < 0) {
                    selected = 0;
                } else {
                    selected = 6;
                }
                pageStart += delta;
                repeatCount++;
                if (repeatCount < 5) {
                    animationFrames = 10;
                } else {
                    animationFrames = 4;
                    repeatCount = 8;
                }
                upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);
                for (item = 1; item <= animationFrames; item++) {
                    progress = item / (float)animationFrames;
                    if (delta < 0) {
                        if (pageStart != 0) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        if (pageStart < count - 7 &&
                            lowerArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        }
                    } else {
                        if (pageStart < count - 7) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        if (pageStart != 0 &&
                            upperArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        }
                    }
                    if (item == animationFrames / 2) {
                        fn_1_6590(pageStart);
                    }
                    HuPrcVSleep();
                }
            } else {
                fn_1_C3FC(selected);
            }
            fn_1_6504(selected + pageStart);
            fn_1_19B8(1);
        }
        if (HuPadBtnDown[0] & 0x200) {
            HuAudFXPlay(MSM_SE_CMN_04);
            break;
        }
        HuPrcVSleep();
    }

    HuSprAttrSet(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = 1.0f - item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 208.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - (-22.0f) * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (item = 0; item < 7; item++) {
        for (frame = 0; frame < 2; frame++) {
            HuWinKill(lbl_1_bss_9EC[item][frame]);
        }
    }
    for (item = 0; item < 105; item++) {
        HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called by the prize browser when the highlighted prize changes. */
void fn_1_6504(s16 prizeNo)
{
    if (lbl_1_bss_8.prizeUnlocked[prizeNo] == FALSE) {
        HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_SINGLE, 101));
    } else {
        HuWinMesSet(lbl_1_bss_6A2[0],
            lbl_1_bss_8.prizeDescription[prizeNo]);
    }
}

/* Called by the prize browser to refresh the visible prize rows. */
void fn_1_6590(s16 firstPrize)
{
    s16 i;

    for (i = 0; i < 7; i++) {
        if (lbl_1_bss_8.prizeUnlocked[firstPrize + i] == FALSE) {
            HuWinMesSet(lbl_1_bss_9EC[i][0], MESSNUM(MESS_OPTION_SINGLE, 50));
            OPTION_RECORD_TEXT(i)[0] = 14;
            OPTION_RECORD_TEXT(i)[1] = 19;
            OPTION_RECORD_TEXT(i)[2] = 16;
            OPTION_RECORD_TEXT(i)[3] = 195;
            OPTION_RECORD_TEXT(i)[4] = 195;
            OPTION_RECORD_TEXT(i)[5] = 195;
            OPTION_RECORD_TEXT(i)[6] = 0;
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)OPTION_RECORD_TEXT(i));
        } else {
            HuWinMesSet(lbl_1_bss_9EC[i][0],
                lbl_1_bss_8.prizeName[firstPrize + i]);
            OPTION_RECORD_TEXT(i)[0] = 14;
            OPTION_RECORD_TEXT(i)[1] = 19;
            OPTION_RECORD_TEXT(i)[2] = 16;
            sprintf(&OPTION_RECORD_TEXT(i)[3], lbl_1_data_486,
                lbl_1_bss_8.prizeId[firstPrize + i]);
            /* Byte 2 is the fixed 0x10 spacing character, so this leading-zero cleanup never
             * runs. */
            if (OPTION_RECORD_TEXT(i)[2] == '0') {
                OPTION_RECORD_TEXT(i)[2] = ' ';
                if (OPTION_RECORD_TEXT(i)[3] == '0') {
                    OPTION_RECORD_TEXT(i)[3] = ' ';
                }
            }
            HuWinMesSet(lbl_1_bss_9EC[i][1], (u32)OPTION_RECORD_TEXT(i));
        }
    }
}

/* Opens the fixed list of eight minigame records, hiding locked minigame names and results. */
void fn_1_6898(void)
{
    s16 count;
    s16 item;
    s16 frame;
    s16 selected;
    s16 pageStart;
    s16 window;
    s16 delta;
    float progress;
    float lowerArrowScale;
    float upperArrowScale;
    float scale;

    pageStart = 0;
    count = 0;
    while (lbl_1_data_48C[count].minigameId != -1) {
        count++;
    }

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -90.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);

    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, MESSNUM(MESS_OPTION_MESS, 43));

    for (item = 0; item < 7; item++) {
        lbl_1_bss_9EC[item][0] = HuWinCreate(75.0f,
            item * 27 + 120, 400, 40, 0);
        lbl_1_bss_9EC[item][1] = HuWinCreate(305.0f,
            item * 27 + 120, 177, 40, 0);
        HuWinMesSet(lbl_1_bss_9EC[item][0], MESSNUM(MESS_MG_NAME, 0) + item);
        HuWinAttrSet(lbl_1_bss_9EC[item][1], HUWIN_ATTR_ALIGN_RIGHT);
        HuWinMesSet(lbl_1_bss_9EC[item][1], MESSNUM(MESS_SAF_TEST, 23));
        for (frame = 0; frame < 2; frame++) {
            HuWinPriSet(lbl_1_bss_9EC[item][frame], 10);
            HuWinBGTPLvlSet(lbl_1_bss_9EC[item][frame],
                0.0f);
            HuWinMesSpeedSet(lbl_1_bss_9EC[item][frame], 0);
        }
    }
    fn_1_7728(0);

    if (count > 7) {
        HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f,
            1.0f);
    }
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    HuAudFXPlay(MSM_SE_MENU_38);

    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 216.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - -17.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 18));
    HuWinDispOn(lbl_1_bss_6A0);
    selected = 0;
    fn_1_C3FC(selected);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 16.2f,
        1.0f);
    for (;;) {
        delta = 0;
        if (HuPadDStkRep[0] & 8) {
            if (pageStart + selected > 0) {
                delta = -1;
            }
        }
        if (HuPadDStkRep[0] & 4) {
            if (pageStart + selected < count - 1) {
                delta = 1;
            }
        }
        if (delta != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            selected += delta;
            if (selected < 0 || selected >= 7) {
                if (selected < 0) {
                    selected = 0;
                } else {
                    selected = 6;
                }
                pageStart += delta;
                upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);
        for (item = 1; item <= 10; item++) {
            progress = item / 10.0f;
                    if (delta < 0) {
                        if (pageStart != 0) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        if (pageStart < count - 7 &&
                            lowerArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        }
                    } else {
                        if (pageStart < count - 7) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        if (pageStart != 0 &&
                            upperArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        }
                    }
                    if (item == 5) {
                        fn_1_7728(pageStart);
                    }
                    HuPrcVSleep();
                }
            } else {
                fn_1_C3FC(selected);
            }
        }
        if (HuPadBtnDown[0] & 0x200) {
            HuAudFXPlay(MSM_SE_CMN_04);
            break;
        }
        HuPrcVSleep();
    }

    HuSprAttrSet(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = 1.0f - item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 216.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - -17.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (item = 0; item < 7; item++) {
        for (frame = 0; frame < 2; frame++) {
            HuWinKill(lbl_1_bss_9EC[item][frame]);
        }
    }
    for (item = 0; item < 105; item++) {
        HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called while the record browser refreshes a page of minigame results. */
void fn_1_7728(s16 firstEntry)
{
    s16 row;
    OptionRecordDisplayEntry *entry = &lbl_1_data_48C[firstEntry];
    u32 recordValue;
    s16 timePart;

    for (row = 0; row < 7; row++, entry++) {
        if (lbl_1_bss_8.minigameUnlocked[entry->minigameId - 601] == FALSE) {
            HuWinMesSet(lbl_1_bss_9EC[row][0], MESSNUM(MESS_OPTION_MESS, 31));
            HuWinMesSet(lbl_1_bss_9EC[row][1], MESSNUM(MESS_OPTION_MESS, 74));
        } else {
            HuWinMesSet(lbl_1_bss_9EC[row][0],
                MgDataTbl[entry->minigameId - 601].nameMes);
            recordValue = GWRecordGet(entry->recordId);
            switch (entry->displayType) {
            case 0:
                /* Format 60 Hz frame counts as minutes, seconds, and truncated hundredths. */
                timePart = recordValue / 3600;
                sprintf(OPTION_RECORD_TEXT(row), lbl_1_data_4D4, timePart);
                recordValue -= timePart * 3600;
                HuWinInsertMesSet(lbl_1_bss_9EC[row][1],
                    (u32)OPTION_RECORD_TEXT(row), 0);
                timePart = recordValue / 60;
                sprintf(&OPTION_RECORD_TEXT(row)[10], lbl_1_data_4D8, timePart);
                recordValue -= timePart * 60;
                HuWinInsertMesSet(lbl_1_bss_9EC[row][1],
                    (u32)&OPTION_RECORD_TEXT(row)[10], 1);
                recordValue = (u32)(1.6666666f * recordValue);
                sprintf(&OPTION_RECORD_TEXT(row)[20], lbl_1_data_4D8,
                    recordValue);
                HuWinInsertMesSet(lbl_1_bss_9EC[row][1],
                    (u32)&OPTION_RECORD_TEXT(row)[20], 2);
                break;
            case 1:
                sprintf(OPTION_RECORD_TEXT(row), lbl_1_data_4DD, recordValue);
                HuWinInsertMesSet(lbl_1_bss_9EC[row][1],
                    (u32)OPTION_RECORD_TEXT(row), 0);
                break;
            }
            HuWinMesSet(lbl_1_bss_9EC[row][1], entry->message);
        }
    }
}

/* Called by Option Mode to browse the ten saved Decathlon records. */
void fn_1_7A74(void)
{
    s16 frame;
    s16 item;
    s16 selectedRecord;
    s16 horizontalDelta;
    s16 verticalDelta;
    s16 window;
    OptionDecaRecord *record;
    float progress;
    float rotation;
    float x;
    float y;
    float rotationStep;
    HuVecF screenPos;
    HuVecF worldPos;

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    fn_1_879C();
    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, MESSNUM(MESS_OPTION_MESS, 44));
    HuAudFXPlay(MSM_SE_MENU_38);

    for (frame = 1; frame <= 10; frame++) {
        progress = frame / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        for (item = 0; item < 10; item++) {
            if (item < 5) {
                x = item * 90 - 180;
                y = -40.0f;
            } else {
                x = (item - 5) * 90 - 180;
                y = 80.0f;
            }
            screenPos.x = 288.0f + x * progress;
            screenPos.y = 240.0f + y * progress - 40.0f;
            screenPos.z = 1200.0f;
            Hu3D2Dto3D(&screenPos, 2, &worldPos);
            Hu3DModelPosSetV(lbl_1_bss_66E[item + 14], &worldPos);
            Hu3DModelScaleSet(lbl_1_bss_66E[item + 14], progress, progress,
                0.5 * progress);
        }
        fn_1_85E4(0, progress);
        HuPrcVSleep();
    }

    selectedRecord = 0;
    HuSprScaleSet(lbl_1_bss_5D4, (s16)(selectedRecord + 8),
        1.2f, 1.2f);
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 2));
    HuWinDispOn(lbl_1_bss_6A0);
    HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 48));
    rotation = rotationStep = 0.0f;
    for (;;) {
        horizontalDelta = verticalDelta = 0;
        if (HuPadDStkRep[0] & 2) {
            horizontalDelta = 1;
        } else if (HuPadDStkRep[0] & 1) {
            horizontalDelta = -1;
        } else if (HuPadDStkRep[0] & 8) {
            verticalDelta = -5;
        } else if (HuPadDStkRep[0] & 4) {
            verticalDelta = 5;
        }
        if (selectedRecord + horizontalDelta + verticalDelta < 0 ||
            selectedRecord + horizontalDelta + verticalDelta >= 10) {
            horizontalDelta = verticalDelta = 0;
        }
        if (horizontalDelta != 0 || verticalDelta != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            rotation = 0.0f;
            Hu3DModelRotSet(lbl_1_bss_66E[selectedRecord + 14],
                0.0f, 0.0f, 0.0f);
            HuSprScaleSet(lbl_1_bss_5D4, selectedRecord + 8,
                1.0f, 1.0f);
            selectedRecord += horizontalDelta + verticalDelta;
            HuSprScaleSet(lbl_1_bss_5D4, selectedRecord + 8,
                1.2f, 1.2f);
            fn_1_85E4(selectedRecord, 1.0f);
        }

        if (HuPadBtnDown[0] & 0x100) {
            record = &lbl_1_bss_8.decaRecord[selectedRecord];
            if (record->character != -1) {
                HuAudFXPlay(MSM_SE_CMN_02);
                rotation = 0.0f;
                Hu3DModelRotSet(lbl_1_bss_66E[selectedRecord + 14],
                    0.0f, 0.0f, 0.0f);
                for (frame = 0; frame < 10; frame++) {
                    HuSprAttrSet(lbl_1_bss_5D4, frame + 8, HUSPR_ATTR_DISPOFF);
                    HuSprAttrSet(lbl_1_bss_5D4, frame + 69, HUSPR_ATTR_DISPOFF);
                    for (item = 0; item < 5; item++) {
                        HuSprAttrSet(lbl_1_bss_5D4,
                            (frame * 5 + 19) + item, HUSPR_ATTR_DISPOFF);
                    }
                }
                for (item = 0; item < 10; item++) {
                    Hu3DModelScaleSet(lbl_1_bss_66E[item + 14],
                        0.0f, 0.0f,
                        0.0f);
                }
                Hu3DModelScaleSet(lbl_1_bss_66E[24], 0.0f,
                    0.0f, 0.0f);
                fn_1_8CA4(selectedRecord);
                fn_1_879C();
                fn_1_85E4(selectedRecord, 1.0f);
                HuPrcVSleep();
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 2));
    HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 48));
                HuWinDispOn(lbl_1_bss_6A0);
            } else {
                HuAudFXPlay(MSM_SE_CMN_05);
            }
        }
        if (HuPadBtnDown[0] & 0x200) {
            HuAudFXPlay(MSM_SE_CMN_04);
            break;
        }
        rotation += rotationStep;
        if (rotation > 360.0f) {
            rotation -= 360.0f;
        }
        rotationStep += 0.05f;
        if (rotationStep > 3.0f) {
            rotationStep = 3.0f;
        }
        Hu3DModelRotSet(lbl_1_bss_66E[selectedRecord + 14],
            0.0f, rotation, 0.0f);
        HuPrcVSleep();
    }

    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (frame = 1; frame <= 10; frame++) {
        progress = 1.0f - frame / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        for (item = 0; item < 10; item++) {
            if (item < 5) {
                x = item * 90 - 180;
                y = -40.0f;
            } else {
                x = (item - 5) * 90 - 180;
                y = 80.0f;
            }
            screenPos.x = 288.0f + x * progress;
            screenPos.y = 240.0f + y * progress - 40.0f;
            screenPos.z = 1200.0f;
            Hu3D2Dto3D(&screenPos, 2, &worldPos);
            Hu3DModelPosSetV(lbl_1_bss_66E[item + 14], &worldPos);
            Hu3DModelScaleSet(lbl_1_bss_66E[item + 14], progress, progress,
                0.5 * progress);
        }
        fn_1_85E4(selectedRecord, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (frame = 0; frame < 105; frame++) {
        HuSprAttrSet(lbl_1_bss_5D4, frame, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called by the Decathlon screen to position its trophy during transitions. */
void fn_1_85E4(s16 item, float progress)
{
    HuVecF screenPos;
    HuVecF worldPos;
    float scale;

    if (item < 5) {
        screenPos.x = item * 90 - 180;
        screenPos.y = -40.0f;
    } else {
        screenPos.x = (item - 5) * 90 - 180;
        screenPos.y = 80.0f;
    }
    screenPos.x = 288.0f + screenPos.x * progress -
        40.0f * progress;
    screenPos.y = 240.0f + screenPos.y * progress -
        40.0f - 40.0f * progress;
    screenPos.z = 1100.0f;
    Hu3D2Dto3D(&screenPos, 2, &worldPos);
    Hu3DModelPosSetV(lbl_1_bss_66E[24], &worldPos);
    scale = 0.3f * progress;
    Hu3DModelScaleSet(lbl_1_bss_66E[24], scale, scale, scale);
}

/* Called by the Decathlon screen to place saved player models and scores. */
void fn_1_879C(void)
{
    s16 item;
    s16 digit;
    s16 rankIndex;
    s16 previousScore;
    s16 score;
    s16 bank;
    s16 divisor;
    float x;
    float y;
    HuVecF screenPos;
    HuVecF worldPos;
    OptionDecaRecord *record = &lbl_1_bss_8.decaRecord[0];

    previousScore = -1;
    for (item = rankIndex = 0; item < 10;) {
        if (item < 5) {
            x = item * 90 - 180;
            y = -40.0f;
        } else {
            x = (item - 5) * 90 - 180;
            y = 80.0f;
        }
        screenPos.x = 288.0f + x;
        screenPos.y = 240.0f + y - 40.0f;
        screenPos.z = 1200.0f;
        Hu3D2Dto3D(&screenPos, 2, &worldPos);
        Hu3DModelPosSetV(lbl_1_bss_66E[item + 14], &worldPos);
        Hu3DModelScaleSet(lbl_1_bss_66E[item + 14], 1.0f, 1.0f,
            1.0f);
        Hu3DModelRotSet(lbl_1_bss_66E[item + 14], 0.0f, 0.0f,
            0.0f);
        if (record->character != -1) {
            fn_1_F410(lbl_1_bss_66E[item + 14], record->character + 25,
                0, record->character + 25, 2);
        } else {
            fn_1_F410(lbl_1_bss_66E[item + 14], 36, 0, 36, 0);
        }

        if (previousScore != record->finalScore || record->character == -1) {
            rankIndex = item;
        }
        previousScore = record->finalScore;
        HuSprAttrReset(lbl_1_bss_5D4, item + 69, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_5D4, item + 69, x, y - 43.0f);
        HuSprScaleSet(lbl_1_bss_5D4, item + 69, 1.0f,
            1.0f);
        HuSprBankSet(lbl_1_bss_5D4, item + 69, rankIndex);
        score = record->finalScore;
        for (digit = 0, divisor = 10000; digit < 5;
             digit++, divisor /= 10) {
            HuSprAttrReset(lbl_1_bss_5D4, item * 5 + 19 + digit, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(lbl_1_bss_5D4, item * 5 + 19 + digit,
                8.0f + (x - 40.0f) + digit * 16,
                43.0f + y);
            bank = score / divisor;
            if (bank > 9) {
                bank = 9;
            }
            HuSprBankSet(lbl_1_bss_5D4, item * 5 + 19 + digit, bank);
            score -= divisor * (score / divisor);
        }
        item++;
        record++;
    }

    screenPos.x = 78.0f;
    screenPos.y = 130.0f;
    screenPos.z = 1100.0f;
    Hu3D2Dto3D(&screenPos, 2, &worldPos);
    Hu3DModelPosSetV(lbl_1_bss_66E[24], &worldPos);
    Hu3DModelScaleSet(lbl_1_bss_66E[24], 0.3f,
        0.3f, 0.3f);
}

/* Called when the player selects a record entry to show its details. */
void fn_1_8CA4(s16 recordIndex)
{
    s16 i;
    s16 j;
    s16 rankIndex;
    s16 previousScore;
    s16 score;
    s16 bank;
    s16 divisor;
    float x;
    float y;
    HuVecF screenPos;
    HuVecF worldPos;
    OptionDecaRecord *record;
    s16 selectedCharacter;

    for (i = 0; i < 50; i++) {
        HuSprAttrSet(lbl_1_bss_5D4, i + 19, HUSPR_ATTR_DISPOFF);
    }
    record = &lbl_1_bss_8.decaRecord[recordIndex];
    selectedCharacter = record->character;
    HuSprAttrReset(lbl_1_bss_5D4, selectedCharacter + 94, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, selectedCharacter + 94,
        -45.0f, -88.0f);
    HuSprScaleSet(lbl_1_bss_5D4, selectedCharacter + 94,
        1.0f, 1.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 69, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 69, -110.0f, -88.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 69, 1.5f, 1.5f);

    /* The first comparison reads an uninitialized previousScore; the rank already starts at
     * zero. */
    j = rankIndex = 0;
    for (; j <= recordIndex; j++) {
        if (previousScore != lbl_1_bss_8.decaRecord[j].finalScore ||
            lbl_1_bss_8.decaRecord[j].character == -1) {
            rankIndex = j;
        }
        previousScore = lbl_1_bss_8.decaRecord[j].finalScore;
    }
    HuSprBankSet(lbl_1_bss_5D4, 69, rankIndex);

    score = record->finalScore;
    for (i = 0, divisor = 10000; i < 5; i++, divisor /= 10) {
        HuSprAttrReset(lbl_1_bss_5D4, i + 89, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_5D4, i + 89, i * 26 - 2, -88.0f);
        bank = score / divisor;
        if (bank > 9) {
            bank = 9;
        }
        HuSprBankSet(lbl_1_bss_5D4, i + 89, bank);
        score -= divisor * (score / divisor);
    }

    for (j = 0; j < 10; j++) {
        if (j < 5) {
            x = j * 90 - 180;
            y = -25.0f;
        } else {
            x = (j - 5) * 90 - 180;
            y = 85.0f;
        }
        HuSprAttrReset(lbl_1_bss_5D4, j + 79, HUSPR_ATTR_DISPOFF);
        HuSprPosSet(lbl_1_bss_5D4, j + 79, x, y);
        HuSprBankSet(lbl_1_bss_5D4, j + 79, j);
        score = record->score[j];
        for (i = 0, divisor = 1000; i < 4; i++, divisor /= 10) {
            HuSprAttrReset(lbl_1_bss_5D4, j * 4 + 19 + i, HUSPR_ATTR_DISPOFF);
            HuSprPosSet(lbl_1_bss_5D4, j * 4 + 19 + i,
                8.0f + (x - 32.0f) + i * 16,
                52.0f + y);
            bank = score / divisor;
            if (bank > 9) {
                bank = 9;
            }
            HuSprBankSet(lbl_1_bss_5D4, j * 4 + 19 + i, bank);
            score -= divisor * (score / divisor);
        }
    }
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 20));
    HuWinDispOn(lbl_1_bss_6A0);
    HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 48));
    HuPrcVSleep();
    while ((HuPadBtnDown[0] & 0x200) == 0) {
        HuPrcVSleep();
    }
    HuAudFXPlay(MSM_SE_CMN_04);
    HuWinDispOff(lbl_1_bss_6A0);
    HuSprAttrSet(lbl_1_bss_5D4, selectedCharacter + 94, HUSPR_ATTR_DISPOFF);
    for (i = 0; i < 5; i++) {
        HuSprAttrSet(lbl_1_bss_5D4, i + 89, HUSPR_ATTR_DISPOFF);
    }
    for (i = 0; i < 10; i++) {
        HuSprAttrSet(lbl_1_bss_5D4, i + 79, HUSPR_ATTR_DISPOFF);
    }
}

/* Displays seven supplied message rows until B is pressed.
 * The entry count limits cursor movement and scrolling; all seven windows are created. */
void fn_1_930C(u32 *messages, s16 entryCount, u32 titleMessage)
{
    s32 windows[7];
    s16 item;
    s16 frame;
    s16 selected;
    s16 pageStart;
    s16 window;
    s16 delta;
    float progress;
    float upperArrowScale;
    float lowerArrowScale;
    float scale;

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -90.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 2, 1.0f, 1.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f, 1.0f);

    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, titleMessage);

    for (item = 0; item < 7; item++) {
        windows[item] = HuWinCreate(HUWIN_POS_CENTER,
            item * 27 + 120, 400, 40, 0);
        HuWinAttrSet(windows[item], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSet(windows[item], messages[item]);
        HuWinPriSet(windows[item], 10);
        HuWinBGTPLvlSet(windows[item], 0.0f);
        HuWinMesSpeedSet(windows[item], 0);
    }

    pageStart = 0;
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    if (entryCount <= 7) {
        HuSprScaleSet(lbl_1_bss_5D4, 3, 0.0f,
            0.0f);
    }
    HuAudFXPlay(MSM_SE_MENU_38);

    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(windows[frame],
                288.0f - 200.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(windows[frame], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 18));
    HuWinDispOn(lbl_1_bss_6A0);
    selected = 0;
    fn_1_C3FC(selected);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 11.0f,
        1.0f);
    for (;;) {
        delta = 0;
        if (HuPadDStkRep[0] & 8) {
            if (pageStart + selected > 0) {
                delta = -1;
            }
        }
        if (HuPadDStkRep[0] & 4) {
            if (pageStart + selected < entryCount - 1) {
                delta = 1;
            }
        }
        if (delta != 0) {
            selected += delta;
            if (selected < 0 || selected >= 7) {
                if (selected < 0) {
                    selected = 0;
                } else {
                    selected = 6;
                }
                pageStart += delta;
                upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);
                for (item = 1; item <= 10; item++) {
                    progress = item / 10.0f;
                    if (delta < 0) {
                        if (pageStart != 0) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        if (pageStart < entryCount - 7 &&
                            lowerArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        }
                    } else {
                        if (pageStart < entryCount - 7) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        if (pageStart != 0 &&
                            upperArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        }
                    }
                    if (item == 5) {
                        for (frame = 0; frame < 7; frame++) {
                            HuWinMesSet(windows[frame],
                                messages[pageStart + frame]);
                        }
                    }
                    HuPrcVSleep();
                }
            } else {
                fn_1_C3FC(selected);
            }
        }
        if (HuPadBtnDown[0] & 0x200) {
            HuAudFXPlay(MSM_SE_CMN_04);
            break;
        }
        HuPrcVSleep();
    }

    HuSprAttrSet(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = 1.0f - item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(windows[frame],
                288.0f - 200.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(windows[frame], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (item = 0; item < 7; item++) {
        HuWinKill(windows[item]);
    }
    for (item = 0; item < 105; item++) {
        HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called by Option Mode to browse consecutive minigame-play records. */
void fn_1_9F74(void)
{
    s16 count;
    s16 item;
    s16 frame;
    s16 selected;
    s16 pageStart;
    s16 window;
    s16 summaryWindow;
    char countText[8];
    s16 delta;
    s16 repeatCount;
    s16 animationFrames;
    float progress;
    float lowerArrowScale;
    float upperArrowScale;
    float scale;

    pageStart = 0;
    repeatCount = 0;
    count = lbl_1_bss_8.consecutiveRecordCount;

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -36.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 3, 0.0f, 0.0f);

    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, MESSNUM(MESS_OPTION_MESS, 59));
    summaryWindow = HuWinCreate(HUWIN_POS_CENTER, 110.0f, 400, 40, 0);
    HuWinPriSet(summaryWindow, 10);
    HuWinBGTPLvlSet(summaryWindow, 0.0f);
    HuWinAttrSet(summaryWindow, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(summaryWindow, 0);
    sprintf(countText, lbl_1_data_486, count);
    HuWinInsertMesSet(summaryWindow, (u32)countText, 0);
    HuWinMesSet(summaryWindow, MESSNUM(MESS_OPTION_MESS, 60));

    for (item = 0; item < 5; item++) {
        lbl_1_bss_9EC[item][0] = HuWinCreate(75.0f,
            item * 27 + 174, 180, 40, 0);
        lbl_1_bss_9EC[item][1] = HuWinCreate(228.0f,
            item * 27 + 174, 400, 40, 0);
        HuWinMesSet(lbl_1_bss_9EC[item][0], MESSNUM(MESS_MG_NAME, 0) + item);
        HuWinMesSet(lbl_1_bss_9EC[item][1], MESSNUM(MESS_SAF_TEST, 23));
        for (frame = 0; frame < 2; frame++) {
            HuWinPriSet(lbl_1_bss_9EC[item][frame], 10);
            HuWinBGTPLvlSet(lbl_1_bss_9EC[item][frame],
                0.0f);
            HuWinMesSpeedSet(lbl_1_bss_9EC[item][frame], 0);
        }
    }
    fn_1_B02C(0, count);

    if (count > 5) {
        HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
        HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f,
            1.0f);
    }
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    HuAudFXPlay(MSM_SE_MENU_38);

    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        HuWinPosSet(summaryWindow,
            288.0f - 200.0f * progress,
            200.0f - 90.0f * progress);
        HuWinScaleSet(summaryWindow, progress, progress);
        for (frame = 0; frame < 5; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 213.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 174)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - 60.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 174)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 18));
    HuWinDispOn(lbl_1_bss_6A0);
    if (count == 0) {
        while (!(HuPadBtnDown[0] & 0x200)) {
            HuPrcVSleep();
        }
        HuAudFXPlay(MSM_SE_CMN_04);
    } else {
        selected = 0;
        fn_1_C3FC(selected + 2);
        HuSprScaleSet(lbl_1_bss_5D4, 7, 16.0f,
            1.0f);
        for (;;) {
            delta = 0;
            if (HuPadDStkRep[0] & 8) {
                if (pageStart + selected > 0) {
                    delta = -1;
                }
            }
            if (HuPadDStkRep[0] & 4) {
                if (pageStart + selected < count - 1) {
                    delta = 1;
                }
            }
            if ((HuPadDStk[0] & 0xC) == 0) {
                repeatCount = 0;
            }
            if (delta != 0) {
                HuAudFXPlay(MSM_SE_CMN_01);
                selected += delta;
                if (selected < 0 || selected >= 5) {
                    if (selected < 0) {
                        selected = 0;
                    } else {
                        selected = 4;
                    }
                    pageStart += delta;
                    repeatCount++;
                    if (repeatCount < 5) {
                        animationFrames = 10;
                    } else {
                        animationFrames = 4;
                        repeatCount = 8;
                    }
                    upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                    lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);
                    for (item = 1; item <= animationFrames; item++) {
                        progress = item / (float) animationFrames;
                        if (delta < 0) {
                            if (pageStart != 0) {
                                scale = 1.0 +
                                    0.30000001192092896 * sin(3.141592653589793 *
                                        (180.0f * progress) /
                                        180.0);
                            } else {
                                scale = cos(3.141592653589793 *
                                    (90.0f * progress) /
                                    180.0);
                            }
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                            if (pageStart < count - 5 &&
                                lowerArrowScale < 0.1f) {
                                scale = sin(3.141592653589793 *
                                    (90.0f * progress) /
                                    180.0);
                                HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                            }
                        } else {
                            if (pageStart < count - 5) {
                                scale = 1.0 +
                                    0.30000001192092896 * sin(3.141592653589793 *
                                        (180.0f * progress) /
                                        180.0);
                            } else {
                                scale = cos(3.141592653589793 *
                                    (90.0f * progress) /
                                    180.0);
                            }
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                            if (pageStart != 0 &&
                                upperArrowScale < 0.1f) {
                                scale = sin(3.141592653589793 *
                                    (90.0f * progress) /
                                    180.0);
                                HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                            }
                        }
                        if (item == animationFrames / 2) {
                            fn_1_B02C(pageStart, count);
                        }
                        HuPrcVSleep();
                    }
                } else {
                    fn_1_C3FC(selected + 2);
                    repeatCount = 0;
                }
            }
            if (HuPadBtnDown[0] & 0x200) {
                HuAudFXPlay(MSM_SE_CMN_04);
                break;
            }
            HuPrcVSleep();
        }

    }

    HuSprAttrSet(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = 1.0f - item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        HuWinPosSet(summaryWindow,
            288.0f - 200.0f * progress,
            200.0f - 90.0f * progress);
        HuWinScaleSet(summaryWindow, progress, progress);
        for (frame = 0; frame < 5; frame++) {
            HuWinPosSet(lbl_1_bss_9EC[frame][0],
                288.0f - 213.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 174)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][0], progress, progress);
            HuWinPosSet(lbl_1_bss_9EC[frame][1],
                288.0f - 60.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 174)) * progress);
            HuWinScaleSet(lbl_1_bss_9EC[frame][1], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (item = 0; item < 5; item++) {
        for (frame = 0; frame < 2; frame++) {
            HuWinKill(lbl_1_bss_9EC[item][frame]);
        }
    }
    for (item = 0; item < 105; item++) {
        HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called by the consecutive-record browser to fill its five visible rows. */
void fn_1_B02C(s16 first, s16 count)
{
    s16 i;

    for (i = 0; i < 5; i++) {
        if (first + i >= count) {
            HuWinHomeClear(lbl_1_bss_9EC[i][0]);
            HuWinHomeClear(lbl_1_bss_9EC[i][1]);
        } else {
            sprintf(lbl_1_bss_9D8[i], lbl_1_data_486, first + i + 1);
            HuWinInsertMesSet(lbl_1_bss_9EC[i][0],
                (u32)lbl_1_bss_9D8[i], 0);
            HuWinMesSet(lbl_1_bss_9EC[i][0], MESSNUM(MESS_OPTION_MESS, 61));
            HuWinMesSet(lbl_1_bss_9EC[i][1],
                MgDataTbl[lbl_1_bss_8.consecutiveRecord[first + i]].nameMes);
        }
    }
}

/* Browses minigame names and unlock status, filtered by mode flags and the selected category. */
void fn_1_B1A0(s16 mode, s16 argument)
{
    s32 windows[7];
    s16 item;
    s16 frame;
    s16 selected;
    s16 pageStart;
    s16 typeIndex;
    s16 typeCount;
    s16 minigameCount;
    s16 window;
    s16 delta;
    s16 repeatCount;
    s16 animationFrames;
    float progress;
    float upperArrowScale;
    float lowerArrowScale;
    float scale;

    repeatCount = 0;
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(lbl_1_bss_5D4, 2, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -90.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 2, 1.0f, 1.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);
    HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f, 1.0f);

    HuSprAttrReset(lbl_1_bss_5D4, 5, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 5, -205.0f, -110.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 6, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 6, 205.0f, -110.0f);
    item = typeCount = typeIndex = 0;
    while (lbl_1_data_4E2[item] != -1) {
        if (argument == lbl_1_data_4E2[item]) {
            typeIndex = item;
        }
        item++;
        typeCount++;
    }
    minigameCount = fn_1_C324(mode, lbl_1_data_4E2[typeIndex]);

    window = HuWinCreate(HUWIN_POS_CENTER, 53.0f, 400, 40, 0);
    HuWinPriSet(window, 10);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(window, 0);
    HuWinMesSet(window, lbl_1_data_4F0[typeIndex]);

    for (item = 0; item < 7; item++) {
        windows[item] = HuWinCreate(HUWIN_POS_CENTER,
            item * 27 + 120, 400, 40, 0);
        HuWinAttrSet(windows[item], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSet(windows[item], lbl_1_bss_848[item]);
        HuWinPriSet(windows[item], 10);
        HuWinBGTPLvlSet(windows[item], 0.0f);
        HuWinMesSpeedSet(windows[item], 0);
        if (item >= minigameCount) {
            HuWinDispOff(windows[item]);
        }
    }

    pageStart = 0;
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    if (minigameCount <= 7) {
        HuSprScaleSet(lbl_1_bss_5D4, 3, 0.0f,
            0.0f);
    }
    HuAudFXPlay(MSM_SE_MENU_38);

    for (item = 1; item <= 10; item++) {
        progress = item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(windows[frame],
                288.0f - 200.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(windows[frame], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 17));
    HuWinDispOn(lbl_1_bss_6A0);
    selected = 0;
    fn_1_C3FC(selected);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 11.0f,
        1.0f);
    for (;;) {
        delta = 0;
        if (HuPadBtnDown[0] & 0x4000) {
            delta = -1;
        }
        if (HuPadBtnDown[0] & 0x2000) {
            delta = 1;
        }
        if (delta != 0) {
            typeIndex += delta;
            if (typeIndex < 0) {
                typeIndex = typeCount - 1;
            }
            if (typeIndex >= typeCount) {
                typeIndex = 0;
            }
            HuAudFXPlay(MSM_SE_MENU_40);
            if (delta < 0) {
                HuSprBankSet(lbl_1_bss_5D4, 5, 1);
            } else {
                HuSprBankSet(lbl_1_bss_5D4, 6, 1);
            }
            for (item = 1; item <= 5; item++) {
                progress = item / 5.0f;
                if (pageStart != 0) {
                    HuSprScaleSet(lbl_1_bss_5D4, 2,
                        1.0f - progress,
                        1.0f - progress);
                }
                if (pageStart + 7 < minigameCount
                    && fn_1_C324(mode, lbl_1_data_4E2[typeIndex]) <= 7) {
                    HuSprScaleSet(lbl_1_bss_5D4, 3,
                        1.0f - progress,
                        1.0f - progress);
                } else if (pageStart + 7 >= minigameCount
                    && fn_1_C324(mode, lbl_1_data_4E2[typeIndex]) > 7) {
                    HuSprScaleSet(lbl_1_bss_5D4, 3, progress, progress);
                }
                HuPrcVSleep();
            }
            minigameCount = fn_1_C324(mode, lbl_1_data_4E2[typeIndex]);
            HuWinMesSet(window, lbl_1_data_4F0[typeIndex]);
            pageStart = 0;
            selected = pageStart;
            for (frame = 0; frame < 7; frame++) {
                if (frame >= minigameCount) {
                    HuWinDispOff(windows[frame]);
                } else {
                    HuWinDispOn(windows[frame]);
                    HuWinMesSet(windows[frame],
                        lbl_1_bss_848[pageStart + frame]);
                }
            }
            fn_1_C3FC(selected);
            HuSprBankSet(lbl_1_bss_5D4, 5, 0);
            HuSprBankSet(lbl_1_bss_5D4, 6, 0);
        }
        delta = 0;
        if (HuPadDStkRep[0] & 8) {
            if (pageStart + selected > 0) {
                delta = -1;
            }
        }
        if (HuPadDStkRep[0] & 4) {
            if (pageStart + selected < minigameCount - 1) {
                delta = 1;
            }
        }
        if ((HuPadDStk[0] & 0xC) == 0) {
            repeatCount = 0;
        }
        if (delta != 0) {
            HuAudFXPlay(MSM_SE_CMN_01);
            selected += delta;
            repeatCount++;
            if (repeatCount < 5) {
                animationFrames = 10;
            } else {
                animationFrames = 4;
                repeatCount = 8;
            }
            if (selected < 0 || selected >= 7) {
                if (selected < 0) {
                    selected = 0;
                } else {
                    selected = 6;
                }
                pageStart += delta;
                upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);
                for (item = 1; item <= animationFrames; item++) {
                    progress = item / (float)animationFrames;
                    if (delta < 0) {
                        if (pageStart != 0) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        if (pageStart < minigameCount - 7 &&
                            lowerArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        }
                    } else {
                        if (pageStart < minigameCount - 7) {
                            scale = 1.0 +
                                0.30000001192092896 * sin(3.141592653589793 *
                                    (180.0f * progress) /
                                    180.0);
                        } else {
                            scale = cos(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        if (pageStart != 0 &&
                            upperArrowScale < 0.1f) {
                            scale = sin(3.141592653589793 *
                                (90.0f * progress) /
                                180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        }
                    }
                    if (item == animationFrames / 2) {
                        for (frame = 0; frame < 7; frame++) {
                            HuWinMesSet(windows[frame],
                                lbl_1_bss_848[pageStart + frame]);
                        }
                    }
                    HuPrcVSleep();
                }
            } else {
                fn_1_C3FC(selected);
                repeatCount = 0;
            }
        }
        if (HuPadBtnDown[0] & 0x200) {
            HuAudFXPlay(MSM_SE_CMN_04);
            break;
        }
        HuPrcVSleep();
    }

    HuSprAttrSet(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuWinDispOff(lbl_1_bss_6A0);
    HuAudFXPlay(MSM_SE_MENU_39);
    for (item = 1; item <= 10; item++) {
        progress = 1.0f - item / 10.0f;
        HuWinPosSet(window,
            288.0f - 200.0f * progress,
            200.0f - 147.0f * progress);
        HuWinScaleSet(window, progress, progress);
        for (frame = 0; frame < 7; frame++) {
            HuWinPosSet(windows[frame],
                288.0f - 200.0f * progress,
                200.0f -
                    (200.0f - (frame * 27 + 120)) * progress);
            HuWinScaleSet(windows[frame], progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }
    HuWinKill(window);
    for (item = 0; item < 7; item++) {
        HuWinKill(windows[item]);
    }
    for (item = 0; item < 105; item++) {
        HuSprAttrSet(lbl_1_bss_5D4, item, HUSPR_ATTR_DISPOFF);
    }
    HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
}

/* Called by the category browser to list minigames with the requested flag and type. */
s16 fn_1_C324(s16 flag, s16 type)
{
    MGDATA *data = MgDataTbl;
    s16 count = 0;
    s16 index = count;

    while (data->ovl != 65535) {
        if ((data->flag & flag) && data->type == type) {
            if (lbl_1_bss_8.minigameUnlocked[index] == FALSE) {
                lbl_1_bss_848[count] = MESSNUM(MESS_OPTION_MESS, 31);
            } else {
                lbl_1_bss_848[count] = data->nameMes;
            }
            count++;
        }
        index++;
        data++;
    }
    return count;
}

/* Called by list screens to move the selection marker to a visible row. */
void fn_1_C3FC(s16 row)
{
    HuSprAttrReset(lbl_1_bss_5D4, 7, HUSPR_ATTR_DISPOFF);
    HuSprPosSet(lbl_1_bss_5D4, 7, 0.0f, row * 27 - 60);
}
