// Builds the Treetop Bingo windows and presents its game instructions and results.
#include "REL/mgmbingodll/module_types.h"

#include "game/audio.h"
#include "game/pad.h"
#include "messdir_enum.h"
#include "msm_se.h"

#define MSM_SE_BINGO_NEW_COMPLETION 1267

typedef struct {
    u32 messages[5]; // Instruction message IDs for targets of one through five completed bingo
                     // lines.
} MessageList;

s16 fn_1_CF30(s16 player);
s16 fn_1_CF74(s16 player);

typedef struct MgmbingoResultMessages {
    u32 byPlayerCount[4]; // Result message IDs for one through four named players.
} MgmbingoResultMessages;
extern int MgModeWorkInt[16];
extern s16 MgModeScore[4];
s16 fn_1_8250(s16 playerIndex);
s16 fn_1_853C(s16 playerIndex);
s16 fn_1_7AD0(s16 playerIndex);
s16 fn_1_7E6C(s16 playerIndex);
s16 fn_1_CEF0(s16 player);

s16 fn_1_CF30(s16 player);
s16 fn_1_CF74(s16 player);
s16 fn_1_CEF0(s16 player);

s16 fn_1_CF30(s16 player);
s16 fn_1_CF74(s16 player);

s16 fn_1_CEF0(s16 player);

s16 fn_1_CF30(s16 player);
s16 fn_1_CF74(s16 player);

s32 lbl_1_data_90[8] = {0, 1, 2, 3, 4, 5, 6, 7};

u32 lbl_1_data_B0[10] = {
    MESSNUM(MESS_MGM_DECATHLON, 55), MESSNUM(MESS_MGM_DECATHLON, 45),
    MESSNUM(MESS_MGM_DECATHLON, 46), MESSNUM(MESS_MGM_DECATHLON, 47),
    MESSNUM(MESS_MGM_DECATHLON, 48), MESSNUM(MESS_MGM_DECATHLON, 49),
    MESSNUM(MESS_MGM_DECATHLON, 50), MESSNUM(MESS_MGM_DECATHLON, 51),
    MESSNUM(MESS_MGM_DECATHLON, 52), MESSNUM(MESS_MGM_DECATHLON, 53)
};

u8 lbl_1_data_D8[3] = {0};

u32 lbl_1_data_DC[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};

MgmbingoMessageSound lbl_1_data_114[33] = {
    {MESSNUM(MESS_MGM_BINGO, 0), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 0), 2, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 0), 3, 0, -1},
    {MESSNUM(MESS_MGM_BINGO, 0), 4, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 1), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 2), 1, 0, -1},
    {MESSNUM(MESS_MGM_BINGO, 3), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 3), 2, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 9), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 10), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 11), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 12), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 13), 1, 0, MSM_SE_GUIDE_19},
    {MESSNUM(MESS_MGM_BINGO, 14), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 14), 2, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 15), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 16), 1, 0, -1},
    {MESSNUM(MESS_MGM_BINGO, 17), 1, 0, MSM_SE_GUIDE_26},
    {MESSNUM(MESS_MGM_BINGO, 18), 1, 0, -1},
    {MESSNUM(MESS_MGM_BINGO, 19), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 19), 2, 0, -1},
    {MESSNUM(MESS_MGM_BINGO, 20), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 21), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 22), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 23), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 24), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 25), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 26), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 27), 1, 0, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_BINGO, 27), 2, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 28), 1, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 28), 2, 0, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_BINGO, 29), 1, 0, -1}
};

s16 lbl_1_data_2A0 = 64;

// Leaves an optional message line blank when a bingo prompt has no extra text.
void fn_1_14B8(s16 windowId, s16 insertionSlot) {
    char messageText[2] = "";

    messageText[0] = messageText[1] = 0;
    HuWinInsertMesSet(windowId, (u32) &messageText, insertionSlot);
}

// Leaves all optional message lines blank before the bingo menu displays its choices.
void fn_1_1510(s16 windowId) {
    s16 messageIndex;

    for (messageIndex = 0; messageIndex < 8; ++messageIndex) {
        s16 insertionSlot = (s16) lbl_1_data_90[messageIndex];

        fn_1_14B8(windowId, insertionSlot);
    }
}

// Clamps the value to 0-99 and returns its digit text in the shared bingo message buffer.
u8 *fn_1_15A8(s16 value)
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
        digitMessage = (u8 *)HuWinMesPtrGet(lbl_1_data_B0[quotient]);
        lbl_1_data_D8[outputIndex] = *digitMessage;
        outputIndex++;
    }
    digitMessage = (u8 *)HuWinMesPtrGet(lbl_1_data_B0[digit]);
    lbl_1_data_D8[outputIndex] = *digitMessage;
    outputIndex++;
    lbl_1_data_D8[outputIndex] = 0;
    lbl_1_data_D8[2] = 0;
    return lbl_1_data_D8;
}

// Creates the shared instruction frame and small text window before bingo play.
void fn_1_1720(void) {
    HuWinInit(0);
    lbl_1_bss_C8.frameWidth = (528.0f);
    lbl_1_bss_C8.frameHeight = (80.0f);
    lbl_1_bss_C8.frameX = (-10000.0f);
    lbl_1_bss_C8.frameY = (14.0f) + ((440.0f) - lbl_1_bss_C8.frameHeight);
    lbl_1_bss_C8.frameWindow = HuWinExCreateFrame(
        lbl_1_bss_C8.frameX,
        lbl_1_bss_C8.frameY,
        (s16)lbl_1_bss_C8.frameWidth,
        (s16)lbl_1_bss_C8.frameHeight,
        -1,
        3);
    HuWinCallbackSet(lbl_1_bss_C8.frameWindow, fn_1_3A60);
    HuWinAttrSet(lbl_1_bss_C8.frameWindow, HUWIN_ATTR_ALIGN_CENTER);
    lbl_1_bss_C8.textWidth = (448.0f);
    lbl_1_bss_C8.textHeight = (32.0f);
    lbl_1_bss_C8.textX = (-10000.0f);
    lbl_1_bss_C8.textY = ((440.0f) - lbl_1_bss_C8.textHeight) - (26.0f);
    lbl_1_bss_C8.textWindow = HuWinExCreate(
        lbl_1_bss_C8.textX,
        lbl_1_bss_C8.textY,
        (s16)lbl_1_bss_C8.textWidth,
        (s16)lbl_1_bss_C8.textHeight,
        -1);
    HuWinMesSpeedSet(lbl_1_bss_C8.textWindow, 0);
    HuWinBGTPLvlSet(lbl_1_bss_C8.textWindow, (0.0f));
    HuWinAttrSet(lbl_1_bss_C8.textWindow, HUWIN_ATTR_ALIGN_CENTER);
}

// Removes both bingo windows when the minigame no longer needs its dialogue UI.
void fn_1_1970(void) {
    HuWinExKill(lbl_1_bss_C8.frameWindow);
    HuWinExKill(lbl_1_bss_C8.textWindow);
    HuWinAllKill();
}

// Allows the frame window to accept input from every human-controlled player.
void fn_1_19B0(void) {
    s16 padMask = 0;
    s16 masks[4] = {1, 2, 4, 8};
    s16 player;
    player = 0;
    while (player < 4) {
        if (fn_1_CF30(player) == 0) {
            padMask |= masks[fn_1_CF74(player)];
        }
        player++;
    }
    HuWinPadMaskSet(lbl_1_bss_C8.frameWindow, padMask);
}

// Restricts frame-window input to the selected player's controller port, including computer
// players.
void fn_1_1A64(s16 player) {
    s16 padMask = 0;
    s16 masks[4] = {1, 2, 4, 8};
    padMask |= masks[fn_1_CF74(player)];
    HuWinPadMaskSet(lbl_1_bss_C8.frameWindow, padMask);
}

void fn_1_1AE8(void) {
    HuWinComKeyReset();
}

// Opens the frame window and shows the first bingo instruction message.
void fn_1_1B08(void) {
    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 0));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Shows the follow-up instruction message before returning to bingo play.
void fn_1_1C14(void) {
    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 1));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
}

// Shows the control hint while the required bingo-line count is selected.
void fn_1_1D1C(void) {
    HuWinExOpen(lbl_1_bss_C8.textWindow);
    HuWinMesSet(lbl_1_bss_C8.textWindow, MESSNUM(MESS_SYS_GUIDE, 2));
    HuWinMesWait(lbl_1_bss_C8.textWindow);
}

// Closes the control hint when selection of the required bingo-line count ends.
void fn_1_1D70(void) {
    HuWinExClose(lbl_1_bss_C8.textWindow);
    HuPrcSleep(8);
}

// Shows the two-choice prompt and returns the choice made by the players.
s16 fn_1_1DA4(void) {
    s16 selection = 0;
    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 2));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    selection = HuWinChoiceGet(lbl_1_bss_C8.frameWindow, -1);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
    return selection;
}

// Shows the current game's start instructions and the prompt to begin play.
void fn_1_1ED8(void) {
    MessageList messages = {{MESSNUM(MESS_MGM_BINGO, 4), MESSNUM(MESS_MGM_BINGO, 5),
                             MESSNUM(MESS_MGM_BINGO, 6), MESSNUM(MESS_MGM_BINGO, 7),
                             MESSNUM(MESS_MGM_BINGO, 8)}};
    s16 firstPlayer;
    s16 secondPlayer;
    s16 firstPadMask;
    s16 secondPadMask;

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinInsertMesSet(
        lbl_1_bss_C8.frameWindow,
        messages.messages[MgGameNo - 1],
        (s16)lbl_1_data_90[0]);
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 3));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 9));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Announces players newly able to reach the bingo-line target with one mark, then shows next-game
// prompts.
void fn_1_2114(void) {
    MgmbingoResultMessages resultMessages = {{MESSNUM(MESS_MGM_BINGO, 22),
                                               MESSNUM(MESS_MGM_BINGO, 23),
                                               MESSNUM(MESS_MGM_BINGO, 24),
                                               MESSNUM(MESS_MGM_BINGO, 25)}};
    s16 newReachCharacters[4];
    s16 candidateCount;
    s16 newReachCount;
    s16 previousReachCount;
    s16 playersMask;
    s16 inputPlayer;
    s16 resultMask;
    s16 player;
    s16 resultPlayer;
    s16 nextPlayer;
    s16 waitPlayer;
    s16 periodicPlayer;
    s16 nextMask;
    s16 waitMask;
    s16 periodicMask;
    s16 resultTotal;
    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    candidateCount = 0;
    previousReachCount = 0;
    newReachCount = 0;
    player = 0;
    while (player < 4)
    {
        newReachCharacters[player] = - 1;
        resultTotal = fn_1_8250(player);
        resultTotal += fn_1_7AD0(player);
        if (resultTotal >= MgGameNo)
        {
            candidateCount++;
            resultTotal = fn_1_853C(player);
            resultTotal += fn_1_7E6C(player);
            if (resultTotal >= MgGameNo)
            {
                previousReachCount++;
            }
            else
            {
                newReachCharacters[player] = fn_1_CEF0(player);
                newReachCount++;
            }
        }
        player++;
    }
    if (newReachCount)
    {
        HuAudFXPlay(MSM_SE_BINGO_NEW_COMPLETION);
        HuWinComKeyReset();
        fn_1_19B0();
        if (previousReachCount != 0)
        {
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 28));
        }
        else
        {
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 27));
        }
        HuWinMesWait(lbl_1_bss_C8.frameWindow);
        HuWinComKeyReset();
        fn_1_19B0();
        player = 0;
        resultTotal = 0;
        while (player < 4)
        {
            if (newReachCharacters[player] >= 0)
            {
                HuWinInsertMesSet(lbl_1_bss_C8.frameWindow,
                                  lbl_1_data_DC[newReachCharacters[player]],
                                  (s16) lbl_1_data_90[resultTotal]);
                resultTotal++;
            }
            player++;
        }
        HuWinMesSet(lbl_1_bss_C8.frameWindow, resultMessages.byPlayerCount[newReachCount - 1]);
        HuWinMesWait(lbl_1_bss_C8.frameWindow);
        HuWinComKeyReset();
        fn_1_19B0();
        if (newReachCount < 4)
        {
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 29));
            HuWinMesWait(lbl_1_bss_C8.frameWindow);
        }
    }
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 18));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    fn_1_19B0();
    if ((MgModeWorkInt[15] % 5) == 4)
    {
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 19));
        HuWinMesWait(lbl_1_bss_C8.frameWindow);
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Shows the selected game's name and handles its continue or exit choices.
s16 fn_1_26EC(void) {
    s16 result;
    s16 window;
    s16 menuState;
    s16 insertIndex;
    s16 insertionMessage;

    result = 0;
    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    window = lbl_1_bss_C8.frameWindow;
    insertIndex = 0;
    while (insertIndex < 8) {
        insertionMessage = (s16)lbl_1_data_90[insertIndex];
        fn_1_14B8(window, insertionMessage);
        insertIndex++;
    }

    menuState = 1;
    while (menuState > 0) {
        switch (menuState) {
        case 1: {
            HuWinComKeyReset();
            fn_1_19B0();
            HuWinInsertMesSet(lbl_1_bss_C8.frameWindow,
                              MgDataTbl[MgModeWork[MgModePlayNum]].nameMes,
                              (s16) lbl_1_data_90[0]);
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 10));
            HuWinMesWait(lbl_1_bss_C8.frameWindow);
            HuWinComKeyReset();
            menuState = 3;
            break;
        }
        case 2: {
            HuWinComKeyReset();
            fn_1_19B0();
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 2));
            HuWinMesWait(lbl_1_bss_C8.frameWindow);
            result = HuWinChoiceGet(lbl_1_bss_C8.frameWindow, -1);
            menuState = 0;
            if (result == 0) {
                result = -1;
            } else {
                menuState = 3;
            }
            HuWinComKeyReset();
            break;
        }
        case 3: {
            HuWinComKeyReset();
            fn_1_19B0();
            HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 11));
            HuWinMesWait(lbl_1_bss_C8.frameWindow);
            result = HuWinChoiceGet(lbl_1_bss_C8.frameWindow, -1);
            menuState = 0;
            if (result == 0) {
                result = 0;
            } else if (result == 1) {
                menuState = 2;
            } else if (result == 2) {
                result = 1;
            } else {
                menuState = 2;
            }
            HuWinComKeyReset();
            break;
        }
        }
    }
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
    return result;
}

// Shows a short message in the frame window, then closes it after the player reads it.
void fn_1_2B10(void) {
    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 14));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Announces the player left with a positive round score after the tie-breaking roulette.
void fn_1_2C20(void) {
    s16 messageIndex;
    s16 winningPlayer;
    s16 padMask;
    s16 player;
    s16 padPlayer;

    messageIndex = 0;
    player = 0;
    while (player < 4) {
        if (MgModeScore[player] > 0) {
            winningPlayer = player;
        }
        player++;
    }

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    HuWinComKeyReset();
    fn_1_19B0();
    messageIndex = fn_1_CEF0(winningPlayer);
    HuWinInsertMesSet(
        lbl_1_bss_C8.frameWindow,
        lbl_1_data_DC[messageIndex],
        (s16)lbl_1_data_90[0]);
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 15));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Displays the sole positive scorer's result or the no-winner result.
void fn_1_2DB4(void) {
    s16 messageIndex = 0;
    s16 positiveCount = 0;
    s16 player = 0;
    s16 padPlayer;
    s16 winningPlayer;

    while (player < 4) {
        if (MgModeScore[player] > 0) {
            positiveCount++;
            winningPlayer = player;
        }
        player++;
    }

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    if (positiveCount == 1) {
        HuWinComKeyReset();
        if (fn_1_CF30(winningPlayer) != 0) {
            fn_1_1A64(winningPlayer);
            HuWinComKeyWait(PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, 16);
        } else {
            fn_1_1A64(winningPlayer);
        }
        messageIndex = fn_1_CEF0(winningPlayer);
        HuWinInsertMesSet(lbl_1_bss_C8.frameWindow,
                          lbl_1_data_DC[messageIndex],
                          (s16)lbl_1_data_90[0]);
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 12));
        HuWinMesWait(lbl_1_bss_C8.frameWindow);
    } else {
        s16 padMask;
        HuWinComKeyReset();
        fn_1_19B0();
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 13));
        HuWinMesWait(lbl_1_bss_C8.frameWindow);
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
}

// Opens the text window and displays the standard short instruction message.
void fn_1_3074(void) {
    HuWinExOpen(lbl_1_bss_C8.textWindow);
    HuWinMesSet(lbl_1_bss_C8.textWindow, MESSNUM(MESS_SYS_GUIDE, 12));
    HuWinMesWait(lbl_1_bss_C8.textWindow);
}

// Opens the text window and displays the standard short instruction message.
void fn_1_30C8(void) {
    HuWinExOpen(lbl_1_bss_C8.textWindow);
    HuWinMesSet(lbl_1_bss_C8.textWindow, MESSNUM(MESS_SYS_GUIDE, 13));
    HuWinMesWait(lbl_1_bss_C8.textWindow);
}

// Shows the selected player's marking instruction, with scripted A input for a computer player.
void fn_1_311C(s16 player) {
    s16 delayedPadMask;
    s16 immediatePadMask;

    HuWinComKeyReset();
    if (fn_1_CF30(player) != 0) {
        fn_1_1A64(player);
            HuWinComKeyWait(PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, 16);
    } else {
        fn_1_1A64(player);
    }
    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 16));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
    HuWinExOpen(lbl_1_bss_C8.textWindow);
    HuWinMesSet(lbl_1_bss_C8.textWindow, MESSNUM(MESS_SYS_GUIDE, 12));
    HuWinMesWait(lbl_1_bss_C8.textWindow);
}

// Shows the selected player's extra-mark instruction, with scripted A input for a computer player.
void fn_1_32CC(s16 player) {
    s16 delayedPadMask;
    s16 immediatePadMask;

    HuWinComKeyReset();
    if (fn_1_CF30(player) != 0) {
        fn_1_1A64(player);
            HuWinComKeyWait(PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, PAD_BUTTON_A, 16);
    } else {
        fn_1_1A64(player);
    }
    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 17));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
    HuWinExOpen(lbl_1_bss_C8.textWindow);
    HuWinMesSet(lbl_1_bss_C8.textWindow, MESSNUM(MESS_SYS_GUIDE, 12));
    HuWinMesWait(lbl_1_bss_C8.textWindow);
}

// Closes the short instruction window after the message sequence is complete.
void fn_1_347C(void) {
    HuWinExClose(lbl_1_bss_C8.textWindow);
    HuPrcSleep(8);
}

// Names players who reached the required bingo-line count and rumbles those controlled by humans.
void fn_1_34B0(void) {
    s16 qualifyingCharacterIndex;
    s16 playersMask;
    s16 availableMask;
    s16 vibrationMask;
    s16 player;
    s16 availablePlayer;
    s16 vibrationPlayer;
    s16 gamePlayer;
    s16 qualifyingPlayerCount;
    s16 qualifyingCharacters[4];

    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    fn_1_109E4();
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 20));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();

    fn_1_19B0();
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 21));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();

    fn_1_19B0();

    gamePlayer = 0;
    qualifyingPlayerCount = 0;
    qualifyingCharacterIndex = 0;
    while (gamePlayer < 4) {
        if (MgModeWorkInt[gamePlayer] >= MgGameNo) {
            qualifyingPlayerCount++;
            qualifyingCharacters[qualifyingCharacterIndex] = fn_1_CEF0(gamePlayer);
            qualifyingCharacterIndex++;
        }
        gamePlayer++;
    }
    gamePlayer = 0;
    while (gamePlayer < qualifyingPlayerCount) {
        HuWinInsertMesSet(lbl_1_bss_C8.frameWindow,
                          lbl_1_data_DC[qualifyingCharacters[gamePlayer]],
                          (s16) lbl_1_data_90[gamePlayer]);
        gamePlayer++;
    }
    if (qualifyingPlayerCount <= 1) {
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 22));
    } else if (qualifyingPlayerCount <= 2) {
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 23));
    } else if (qualifyingPlayerCount <= 3) {
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 24));
    } else {
        HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 25));
    }

    gamePlayer = 0;
    while (gamePlayer < 4) {
        if ((MgModeWorkInt[gamePlayer] >= MgGameNo) && (fn_1_CF30(gamePlayer) == 0)) {
                omVibrate(gamePlayer, 20, 20, 0);
        }
        gamePlayer++;
    }
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(10);
}

// Asks whether to replay and returns one when the players choose yes.
s16 fn_1_38E4(void)
{
    s16 result;
    s16 padMask;
    s16 player;
    s16 choice;

    result = 0;
    HuWinExOpen(lbl_1_bss_C8.frameWindow);
    HuWinComKeyReset();
    fn_1_19B0();
    HuWinAttrSet(lbl_1_bss_C8.frameWindow, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(lbl_1_bss_C8.frameWindow, MESSNUM(MESS_MGM_BINGO, 26));
    HuWinMesWait(lbl_1_bss_C8.frameWindow);
    choice = HuWinChoiceGet(lbl_1_bss_C8.frameWindow, -1);
    HuWinAttrReset(lbl_1_bss_C8.frameWindow, HUWIN_ATTR_NOCANCEL);
    if (choice != 0) {
        result = 0;
    } else {
        result = 1;
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.frameWindow);
    HuPrcSleep(8);
    return result;
}

MgmbingoWindowWork lbl_1_bss_C8;

// A message cue with an enabled sound plays it at the current pan and requests a board-side
// gesture.
void fn_1_3A60(HUWINID winId, u32 messageId, s16 cueId) {
    s16 entryIndex;

    for (entryIndex = 0; entryIndex < 33; entryIndex++) {
        if ((messageId == lbl_1_data_114[entryIndex].messageId) &&
            (cueId == lbl_1_data_114[entryIndex].cueId) &&
            (lbl_1_data_114[entryIndex].soundEffectId >= 0)) {
            HuAudFXPlayPan(lbl_1_data_114[entryIndex].soundEffectId, lbl_1_data_2A0);
            fn_1_109E4();
        }
    }
}

// Limits character voice pan to the range centered on the bingo window.
void fn_1_3B3C(s16 pan) {
    if (pan < 32) {
        pan = 32;
    }
    if (pan > 96) {
        pan = 96;
    }
    lbl_1_data_2A0 = pan;
}
