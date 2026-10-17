/* Shows the instructions, choices and results for the minigame streak mode. */
#include "dolphin/math.h"
#include "game/object.h"
#include "game/window.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/mgdata.h"
#include "messdir_enum.h"
#include "msm_se.h"

typedef struct {
    HUWINID windowId;
    f32 width;
    f32 height;
} MGRenshoWindowState;

typedef struct {
    u32 key;
    s16 variant;
    s32 soundEffect;
} MGRenshoSoundCue;

MGRenshoWindowState lbl_1_bss_B8;
s32 lbl_1_data_10[8] = {0, 1, 2, 3, 4, 5, 6, 7};
u32 lbl_1_data_30[10] = {
    MESSNUM(MESS_MGM_DECATHLON, 55), MESSNUM(MESS_MGM_DECATHLON, 45),
    MESSNUM(MESS_MGM_DECATHLON, 46), MESSNUM(MESS_MGM_DECATHLON, 47),
    MESSNUM(MESS_MGM_DECATHLON, 48), MESSNUM(MESS_MGM_DECATHLON, 49),
    MESSNUM(MESS_MGM_DECATHLON, 50), MESSNUM(MESS_MGM_DECATHLON, 51),
    MESSNUM(MESS_MGM_DECATHLON, 52), MESSNUM(MESS_MGM_DECATHLON, 53)
};
u8 lbl_1_data_58[3] = {0, 0, 0};
u32 lbl_1_data_5C[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
MGRenshoSoundCue lbl_1_data_94[25] = {
    {MESSNUM(MESS_MGM_RENSHOU, 0), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 0), 2, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 0), 3, -1},
    {MESSNUM(MESS_MGM_RENSHOU, 1), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 2), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 3), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 4), 1, -1},
    {MESSNUM(MESS_MGM_RENSHOU, 5), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 6), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 7), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 8), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 8), 2, -1},
    {MESSNUM(MESS_MGM_RENSHOU, 9), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 9), 2, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_RENSHOU, 10), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 10), 2, -1},
    {MESSNUM(MESS_MGM_RENSHOU, 11), 1, MSM_SE_GUIDE_19},
    {MESSNUM(MESS_MGM_RENSHOU, 12), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 13), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 14), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 15), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 17), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 18), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 18), 2, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_RENSHOU, 19), 1, MSM_SE_GUIDE_20},
};
s16 lbl_1_data_1C0 = 64;

extern int MgModeWorkInt[16];
s16 fn_1_A05C(s16 player);
s16 fn_1_A094(s16 player);
s16 fn_1_A00C(void);
s16 fn_1_A01C(s16 player);
void fn_1_7D7C(void);
void fn_1_2B7C(HUWINID windowId, u32 key, s16 variant);

/* Points an insertion slot at an empty local string for the choice and result pages.
* The window retains this pointer after the local buffer's lifetime ends. */
void fn_1_1250(s16 windowId, s16 insertionSlot)
{
    char messageText[2] = "";
    messageText[0] = messageText[1] = 0;
    HuWinInsertMesSet(windowId, (u32)messageText, insertionSlot);
}

/* Reserves the eight insertion slots used by the final streak-results page. */
void fn_1_12A8(s16 windowId)
{
    s16 messageIndex;
    for (messageIndex = 0; messageIndex < 8; ++messageIndex) {
        fn_1_1250(windowId, (s16)lbl_1_data_10[messageIndex]);
    }
}

/* Clamps the streak count to 0 through 99, then writes one or two digit glyphs into the
 * shared result-message buffer. */
u8 *fn_1_1340(s16 value)
{
    s16 digit;
    s16 quotient;
    s16 outputIndex;
    u8 *digitMessage;

    if (value < 0) {
        value = 0;
    }
    if (value > 99) {
        value = 99;
    }
    digit = value % 10;
    value = (value - digit) / 10;
    quotient = value % 10;
    outputIndex = 0;
    if (quotient > 0) {
        digitMessage = (u8 *)HuWinMesPtrGet(lbl_1_data_30[quotient]);
        lbl_1_data_58[outputIndex] = *digitMessage;
        outputIndex++;
    }
    digitMessage = (u8 *)HuWinMesPtrGet(lbl_1_data_30[digit]);
    lbl_1_data_58[outputIndex] = *digitMessage;
    outputIndex++;
    lbl_1_data_58[outputIndex] = 0;
    lbl_1_data_58[2] = 0;
    return lbl_1_data_58;
}

/* Creates the centered message window before the main minigame sequence starts. */
void fn_1_14B8(void)
{
    f32 windowY;

    HuWinInit(0);
    lbl_1_bss_B8.width = 528.0f;
    lbl_1_bss_B8.height = 80.0f;
    windowY = 14.0f + (440.0f - lbl_1_bss_B8.height);
    lbl_1_bss_B8.windowId = HuWinExCreateFrame(-10000.0f, windowY,
                                      (s16)lbl_1_bss_B8.width,
                                      (s16)lbl_1_bss_B8.height, -1, 3);
    HuWinAttrSet(lbl_1_bss_B8.windowId, HUWIN_ATTR_ALIGN_CENTER);
    HuWinCallbackSet(lbl_1_bss_B8.windowId, fn_1_2B7C);
}

void fn_1_15CC(void) {
    HuWinExKill(lbl_1_bss_B8.windowId);
    HuWinAllKill();
}

s16 fn_1_A05C(s16 player);

/* Restricts window input to the controller ports assigned to human players. */
void fn_1_15FC(void) {
    s16 padMask = 0;
    s16 masks[4] = {1, 2, 4, 8};
    s16 player;

    player = 0;
    while (player < 4) {
        if (fn_1_A05C(player) == 0) {
            padMask |= masks[fn_1_A094(player)];
        }
        player++;
    }
    HuWinPadMaskSet(lbl_1_bss_B8.windowId, padMask);
}

void fn_1_16B0(void) {
    HuWinComKeyReset();
}

/* Shows the opening streak-mode instructions when the overlay enters its first prompt state. */
void fn_1_16D0(void)
{

    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    HuWinComKeyReset();
    fn_1_15FC();
    HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 0));
    HuWinMesWait(lbl_1_bss_B8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
}

/* Shows the active character's name after the entrance walk in submode 2. */
void fn_1_17DC(void)
{
    s16 character;

    character = fn_1_A01C(fn_1_A00C());
    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    HuWinComKeyReset();
    fn_1_15FC();
    HuWinInsertMesSet(lbl_1_bss_B8.windowId, lbl_1_data_5C[character], lbl_1_data_10[0]);
    HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 3));
    HuWinMesWait(lbl_1_bss_B8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
}

/* Shows the first-round or later-round prompt when submode 3 begins. */
void fn_1_1938(void)
{

    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    HuWinComKeyReset();
    fn_1_15FC();
    if (MgModePlayNum != 0) {
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 17));
    } else {
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 4));
    }
    HuWinMesWait(lbl_1_bss_B8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
}

/* Presents the current minigame name and asks whether to continue or leave the streak. */
s16 fn_1_1A78(void)
{
    HUWINID insertionWindow;
    s16 result;
    s16 state;
    s16 insertionIndex;
    s16 insertionSlot;

    result = 0;
    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    insertionWindow = lbl_1_bss_B8.windowId;
    insertionIndex = 0;
    while (insertionIndex < 8) {
        insertionSlot = (s16)lbl_1_data_10[insertionIndex];
        fn_1_1250(insertionWindow, insertionSlot);
        insertionIndex++;
    }

    state = 1;
    while (state > 0) {
        switch (state) {
        case 1: {
            HuWinComKeyReset();
            fn_1_15FC();
            HuWinInsertMesSet(lbl_1_bss_B8.windowId,
                              MgDataTbl[MgModeWork[MgModePlayNum]].nameMes,
                              (s16)lbl_1_data_10[0]);
            HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 5));
            HuWinMesWait(lbl_1_bss_B8.windowId);
            state = 3;
            HuWinComKeyReset();
            break;
        }
        case 2: {
            HuWinComKeyReset();
            fn_1_15FC();
            HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 2));
            HuWinMesWait(lbl_1_bss_B8.windowId);
            result = HuWinChoiceGet(lbl_1_bss_B8.windowId, -1);
            state = 0;
            if (result == 0) {
                result = -1;
            } else {
                state = 3;
            }
            HuWinComKeyReset();
            break;
        }
        case 3: {
            HuWinComKeyReset();
            fn_1_15FC();
            HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 6));
            HuWinMesWait(lbl_1_bss_B8.windowId);
            result = HuWinChoiceGet(lbl_1_bss_B8.windowId, -1);
            state = 0;
            if (result == 0) {
                result = 0;
            } else if (result == 1) {
                state = 2;
            } else if (result == 2) {
                result = 1;
            } else {
                state = 2;
            }
            HuWinComKeyReset();
            break;
        }
        }
    }
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
    return result;
}

/* Reports the completed minigame result as the overlay enters its post-game state. */
void fn_1_1E9C(void)
{
    s16 character;

    character = fn_1_A01C(fn_1_A00C());
    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    HuWinComKeyReset();
    fn_1_15FC();
    if (MgModeWorkInt[0] != 0) {
        HuWinInsertMesSet(lbl_1_bss_B8.windowId, lbl_1_data_5C[character], lbl_1_data_10[0]);
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 7));
        HuWinMesWait(lbl_1_bss_B8.windowId);
    } else {
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 11));
        HuWinMesWait(lbl_1_bss_B8.windowId);
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
}

/* Shows a milestone message after the 10th, 50th, or 90th minigame. */
void fn_1_2038(void)
{

    HuWinComKeyReset();
    fn_1_15FC();
    if (MgModePlayNum == 10) {
        HuWinExOpen(lbl_1_bss_B8.windowId);
        fn_1_7D7C();
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 8));
        HuWinMesWait(lbl_1_bss_B8.windowId);
        HuWinExClose(lbl_1_bss_B8.windowId);
    }
    if (MgModePlayNum == 50) {
        HuWinExOpen(lbl_1_bss_B8.windowId);
        fn_1_7D7C();
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 9));
        HuWinMesWait(lbl_1_bss_B8.windowId);
        HuWinExClose(lbl_1_bss_B8.windowId);
    }
    if (MgModePlayNum == 90) {
        HuWinExOpen(lbl_1_bss_B8.windowId);
        fn_1_7D7C();
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 10));
        HuWinMesWait(lbl_1_bss_B8.windowId);
        HuWinExClose(lbl_1_bss_B8.windowId);
    }
    HuWinComKeyReset();
    HuPrcSleep(8);
}

/* Shows the final streak summary or unlock result when the overlay finishes. */
void fn_1_221C(void)
{
    s16 character;

    character = fn_1_A01C(fn_1_A00C());
    HuWinExOpen(lbl_1_bss_B8.windowId);
    fn_1_7D7C();
    HuWinComKeyReset();
    fn_1_15FC();
    if ((s32)MgModePlayNum >= MgGameNo && MgModeWorkInt[0] != 0) {
        HuWinInsertMesSet(lbl_1_bss_B8.windowId, lbl_1_data_5C[character], (s16)lbl_1_data_10[0]);
        HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 18));
        HuWinMesWait(lbl_1_bss_B8.windowId);
        HuAudSStreamAllFadeOut(1000);
    } else {
        fn_1_12A8(lbl_1_bss_B8.windowId);
        if ((s32)(MgModePlayNum - 1) == 0) {
            HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 14));
            HuWinMesWait(lbl_1_bss_B8.windowId);
        } else if ((s32)(MgModePlayNum - 1) == 1) {
            HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 16));
            HuWinMesWait(lbl_1_bss_B8.windowId);
        } else {
            if (GwCommon.renshoMgRecordNum == 0) {

                HuWinInsertMesSet(lbl_1_bss_B8.windowId, (u32) fn_1_1340(MgModePlayNum - 1),
                                  (s16) lbl_1_data_10[0]);
                HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 15));
                HuWinMesWait(lbl_1_bss_B8.windowId);
            } else if ((s32)GwCommon.renshoMgRecordNum < (s32)(MgModePlayNum - 1)) {

                HuWinInsertMesSet(lbl_1_bss_B8.windowId, (u32) fn_1_1340(MgModePlayNum - 1),
                                  (s16) lbl_1_data_10[0]);
                HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 12));
                HuWinMesWait(lbl_1_bss_B8.windowId);
            } else {

                HuWinInsertMesSet(lbl_1_bss_B8.windowId, (u32) fn_1_1340(MgModePlayNum - 1),
                                  (s16) lbl_1_data_10[0]);
                HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 13));
                HuWinMesWait(lbl_1_bss_B8.windowId);
            }
        }
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(10);
}

/* Shows the final Endurance Alley choice prompt and returns 1 only when choice 0 is selected. */

s16 fn_1_2A00(void)
{
    s16 result;
    s16 choice;

    result = 0;
    HuWinExOpen(lbl_1_bss_B8.windowId);
    HuWinComKeyReset();
    fn_1_15FC();
    HuWinAttrSet(lbl_1_bss_B8.windowId, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(lbl_1_bss_B8.windowId, MESSNUM(MESS_MGM_RENSHOU, 19));
    HuWinMesWait(lbl_1_bss_B8.windowId);
    choice = HuWinChoiceGet(lbl_1_bss_B8.windowId, -1);
    HuWinAttrReset(lbl_1_bss_B8.windowId, HUWIN_ATTR_NOCANCEL);
    if (choice != 0) {
        result = 0;
    } else {
        result = 1;
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_B8.windowId);
    HuPrcSleep(8);
    return result;
}

/* Window message controls call this to play the assigned guide cue and request the
 * sign's speaking motion. */
void fn_1_2B7C(HUWINID windowId, u32 key, s16 variant)
{
    s16 index;

    for (index = 0; index < 25; index++) {
        if ((key == lbl_1_data_94[index].key) &&
            (variant == lbl_1_data_94[index].variant) &&
            (lbl_1_data_94[index].soundEffect >= 0)) {
            HuAudFXPlayPan(lbl_1_data_94[index].soundEffect, lbl_1_data_1C0);
            fn_1_7D7C();
        }
    }
}

/* Clamps the guide sound's pan to the range 32 through 96. */
void fn_1_2C58(short value) {
    if(value<32) value=32;
    if(value>96) value=96;
    lbl_1_data_1C0=value;
}
