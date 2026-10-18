/* Decathlon Park message windows, selection prompts and score announcements. */
#include "REL/mgmdecathlondll/module_types.h"
#include <string.h>
#include "messdir_enum.h"
#include "msm_se.h"
#include "game/window.h"
#include <math.h>
#include "dolphin/gx.h"
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
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"

#define DECATHLON_TEXT_TAB 12
#define DECATHLON_UNUSED_MESSAGE 4294967295U
#define DECATHLON_SCORE_MAX 9999
#define DECATHLON_TIME_SCORE_MAX 5999 /* 99 seconds and 59 frames at 60 frames per second. */

typedef struct {
    s8 first; /* first text byte; this two-byte view is unused */
    s8 second; /* following text byte; this two-byte view is unused */
} MessageText;

/* Result-list builders store an empty local string in an insertion slot. The window retains
 * its pointer after this helper returns. */
void fn_1_1D80(s16 windowId, s16 slotNo) {
    char messageText[2] = "";

    messageText[0] = messageText[1] = 0;
    HuWinInsertMesSet(windowId, (u32) &messageText, slotNo);
}

/* Called by result screens to clear all eight insertion slots in order. */
void fn_1_1DD8(s16 windowId) {
    s16 slotIndex;

    for (slotIndex = 0; slotIndex < 8; ++slotIndex) {
        s16 slotNo = (s16) lbl_1_data_30[slotIndex];

        fn_1_1D80(windowId, slotNo);
    }
}

/* Result-message construction clears the selected player's accumulated text. */
void fn_1_1E70(s16 playerNo) {
    memset(&lbl_1_bss_24[playerNo], 0, sizeof(lbl_1_bss_24[playerNo]));
}

/* Called while composing a player's result line to append message text bytes. */
void fn_1_1EB0(s16 playerNo, u32 messageId) {
    u8 *messageBytes = (u8 *)HuWinMesPtrGet(messageId);
    s16 textOffset;
    s16 sourceOffset;

    textOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (lbl_1_bss_24[playerNo].messageBytes[textOffset] == 0) {
            break;
        }
        textOffset++;
    }
    sourceOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (messageBytes[sourceOffset] == 0) {
            break;
        }
        /* A full buffer places its terminator in the adjacent status byte, also cleared below. */
        lbl_1_bss_24[playerNo].messageBytes[textOffset] = messageBytes[sourceOffset];
        lbl_1_bss_24[playerNo].messageBytes[textOffset + 1] = 0;
        textOffset++;
        sourceOffset++;
    }
    lbl_1_bss_24[playerNo].enabled = 0;
}

/* Result-message construction appends a tab and clears remaining text bytes. If the text is
 * full, the tab lands in the adjacent status byte and is cleared instead. */
void fn_1_1FD4(s16 playerNo) {
    s16 textOffset;

    textOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (lbl_1_bss_24[playerNo].messageBytes[textOffset] == 0) {
            break;
        }
        textOffset++;
    }
    lbl_1_bss_24[playerNo].messageBytes[textOffset] = DECATHLON_TEXT_TAB;
    textOffset++;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        lbl_1_bss_24[playerNo].messageBytes[textOffset] = 0;
        textOffset++;
    }
    lbl_1_bss_24[playerNo].enabled = 0;
}

MgmDecathlonPlayerRecord *fn_1_20A4(s16 index) {
    lbl_1_bss_24[index].enabled = 0;
    return &lbl_1_bss_24[index];
}

void fn_1_57F4(HUWINID windowId, u32 messageId, s16 characterCode);

/* Decathlon setup calls this to create the shared result and prompt window. */
void fn_1_20D8(void) {
    f32 windowY;

    HuWinInit(0);
    lbl_1_bss_18.width = 528.0f;
    lbl_1_bss_18.height = 80.0f;
    windowY = 440.0f - lbl_1_bss_18.height;
    lbl_1_bss_18.windowId = HuWinExCreateFrame(-10000.0f, windowY,
                                      (s16)lbl_1_bss_18.width,
                                      (s16)lbl_1_bss_18.height, -1, 3);
    HuWinCallbackSet(lbl_1_bss_18.windowId, fn_1_57F4);
    HuWinAttrSet(lbl_1_bss_18.windowId, HUWIN_ATTR_ALIGN_CENTER);
}

void fn_1_21DC(void) {
    HuWinExKill(lbl_1_bss_18.windowId);
    HuWinAllKill();
}

/* Window prompts call this to enable pads belonging to human players. */
void fn_1_220C(void) {
    s16 fn_1_9784(s16 player);
    s16 fn_1_97BC(s16 player);
    s16 padMask;
    s16 player;

    padMask = 0;
    {
    DecathlonPadMaskTable masks = {{1, 2, 4, 8}};
    player = 0;
    while (player < 4) {
        if (fn_1_9784(player) == 0) {
            padMask |= masks.signedValues[fn_1_97BC(player)];
        }
        player++;
    }
    HuWinPadMaskSet(lbl_1_bss_18.windowId, padMask);
}
}

void fn_1_22C0(void) {
    HuWinComKeyReset();
}

extern void fn_1_6E38(void);

/* The mode sequence calls this for the opening rules and play-or-exit choices. */
s16 fn_1_22E0(void) {
    s16 state;
    s16 choice;
    s16 firstPadMask;
    s16 secondPadMask;
    s16 thirdPadMask;
    s16 fourthPadMask;
    state = 1;
    choice = 0;
    HuWinExOpen(lbl_1_bss_18.windowId);
    fn_1_6E38();
    while (state > 0) {
        switch (state) {
        case 1:
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 0));
            HuWinMesWait(lbl_1_bss_18.windowId);
            choice = HuWinChoiceGet(lbl_1_bss_18.windowId, -1);
            if (choice == 0) {
                state = 2;
            } else if (choice == 1) {
                state = 3;
            } else {
                state = 4;
            }
            break;
        case 2:
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 58));
            HuWinMesWait(lbl_1_bss_18.windowId);
            state = 3;
            break;
        case 3:
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 1));
            HuWinMesWait(lbl_1_bss_18.windowId);
            state = 0;
            break;
        case 4:
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 6));
            HuWinMesWait(lbl_1_bss_18.windowId);
            choice = HuWinChoiceGet(lbl_1_bss_18.windowId, -1);
            if (choice == 0) {
                state = -1;
            } else if (choice == 1) {
                state = 1;
            } else {
                state = 1;
            }
            break;
        default:
            state = 0;
            break;
        }
    }
    HuWinExClose(lbl_1_bss_18.windowId);
    for (choice = 0; choice < 10; choice++) {
        HuPrcVSleep();
    }
    return state;
}

/* Before the next event, the sequence announces its place in the ten-event schedule. */
void fn_1_2704(void) {
    s16 fn_1_9784(s16 playerNo);
    s16 fn_1_97BC(s16 playerNo);
    s16 waitFrame;
    s16 padMask;
    s16 playerNo;

    waitFrame = 0;
    {
        DecathlonMessageIds messageIds = {
            { MESSNUM(MESS_MGM_DECATHLON, 45), MESSNUM(MESS_MGM_DECATHLON, 46),
              MESSNUM(MESS_MGM_DECATHLON, 47), MESSNUM(MESS_MGM_DECATHLON, 48),
              MESSNUM(MESS_MGM_DECATHLON, 49), MESSNUM(MESS_MGM_DECATHLON, 50),
              MESSNUM(MESS_MGM_DECATHLON, 51), MESSNUM(MESS_MGM_DECATHLON, 52),
              MESSNUM(MESS_MGM_DECATHLON, 53), MESSNUM(MESS_MGM_DECATHLON, 54) }
        };
        HuWinExOpen(lbl_1_bss_18.windowId);
        fn_1_6E38();
        HuWinComKeyReset();
        fn_1_220C();
        if ((s32) (MgModePlayNum + 1) < 10) {
            HuWinInsertMesSet(lbl_1_bss_18.windowId, messageIds.messageIds[MgModePlayNum], 0);
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 2));
    } else {
        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 4));
    }
    HuWinMesWait(lbl_1_bss_18.windowId);
    HuWinExClose(lbl_1_bss_18.windowId);
    for (waitFrame = 0; waitFrame < 10; waitFrame++) {
        HuPrcVSleep();
    }
}
}

void fn_1_2CF0(HUWINID window, s16 mode);
extern void fn_1_6E38(void);

/* The mode sequence calls this to show the game name and collect the next-game choice. */
s16 fn_1_28E8(void) {
    HUWINID window;
    s16 choice;
    s16 result;
    s16 state;
    s16 messageIndex;
    s16 insertPosition;

    result = 0;
    HuWinExOpen(lbl_1_bss_18.windowId);
    fn_1_6E38();
    window = lbl_1_bss_18.windowId;
    for (messageIndex = 0; messageIndex < 8; messageIndex++) {
        insertPosition = (s16)lbl_1_data_30[messageIndex];
        {
            fn_1_1D80(window, insertPosition);
        }
    }
    HuWinComKeyReset();
    fn_1_220C();
    fn_1_2CF0(lbl_1_bss_18.windowId, MgModePlayNum);
    HuWinInsertMesSet(lbl_1_bss_18.windowId,
                      lbl_1_data_CD0[MgModePlayNum].data->nameMes, 5);
    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 3));
    HuWinMesWait(lbl_1_bss_18.windowId);
    state = 1;
    while (state > 0) {
        switch (state) {
        case 1:
        {
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 5));
            HuWinMesWait(lbl_1_bss_18.windowId);
            result = HuWinChoiceGet(lbl_1_bss_18.windowId, -1);
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
            break;
        }
        case 2:
        {
            HuWinComKeyReset();
            fn_1_220C();
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 6));
            HuWinMesWait(lbl_1_bss_18.windowId);
            result = HuWinChoiceGet(lbl_1_bss_18.windowId, -1);
            state = 0;
            if (result == 0) {
                result = -1;
            } else {
                state = 1;
            }
            break;
        }
        }
    }
    HuWinExClose(lbl_1_bss_18.windowId);
    HuPrcSleep(10);
    return result;
}

/* The game-choice prompt formats the event record, using seconds and hundredths for timed
 * events. */
void fn_1_2CF0(HUWINID window, s16 mode) {
    s32 record;
    s16 digitIndex;
    s32 digit;

    if (mode == 1) {
        record = GwCommon.decaMgRecord[mode];
        if (record > DECATHLON_SCORE_MAX) {
            record = DECATHLON_SCORE_MAX;
        }
        if (record < 0) {
            record = 0;
        }
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[3]);
        digitIndex = 0;
        while ((digitIndex < 4) && (record > 0)) {
            digit = record % 10;
            record = (record - digit) / 10;
            HuWinInsertMesSet(window, lbl_1_data_88[digit],
                              (s16)lbl_1_data_30[3 - digitIndex]);
            digitIndex++;
        }
        HuWinInsertMesSet(window, MESSNUM(MESS_MGM_DECATHLON, 35), (s16)lbl_1_data_30[4]);
    } else if ((mode == 2) || (mode == 7)) {
        record = GwCommon.decaMgRecord[mode];
        if (record > DECATHLON_TIME_SCORE_MAX) {
            record = DECATHLON_TIME_SCORE_MAX;
        }
        if (record < 0) {
            record = 0;
        }
        record = (record - (record % 60)) / 60;
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[0]);
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[1]);
        digitIndex = 0;
        while ((digitIndex < 2) && (record > 0)) {
            digit = record % 10;
            record = (record - digit) / 10;
            HuWinInsertMesSet(window, lbl_1_data_88[digit],
                              (s16)lbl_1_data_30[1 - digitIndex]);
            digitIndex++;
        }
        HuWinInsertMesSet(window, MESSNUM(MESS_MGM_DECATHLON, 34), (s16)lbl_1_data_30[2]);
        record = GwCommon.decaMgRecord[mode];
        if (record > DECATHLON_TIME_SCORE_MAX) {
            record = DECATHLON_TIME_SCORE_MAX;
        }
        if (record < 0) {
            record = 0;
        }
        record = record % 60;
        record = (s32)(100.0f * (0.016666668f * (f32)record));
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[3]);
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[4]);
        digitIndex = 0;
        while ((digitIndex < 2) && (record > 0)) {
            digit = record % 10;
            record = (record - digit) / 10;
            HuWinInsertMesSet(window, lbl_1_data_88[digit],
                              (s16)lbl_1_data_30[4 - digitIndex]);
            digitIndex++;
        }
    } else if (mode == 3) {
        record = GwCommon.decaMgRecord[mode];
        if (record > DECATHLON_SCORE_MAX) {
            record = DECATHLON_SCORE_MAX;
        }
        if (record < 0) {
            record = 0;
        }
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[3]);
        digitIndex = 0;
        while ((digitIndex < 4) && (record > 0)) {
            digit = record % 10;
            record = (record - digit) / 10;
            HuWinInsertMesSet(window, lbl_1_data_88[digit],
                              (s16)lbl_1_data_30[3 - digitIndex]);
            digitIndex++;
        }
        HuWinInsertMesSet(window, MESSNUM(MESS_MGM_DECATHLON, 37), (s16)lbl_1_data_30[4]);
    } else {
        record = GwCommon.decaMgRecord[mode];
        if (record > DECATHLON_SCORE_MAX) {
            record = DECATHLON_SCORE_MAX;
        }
        if (record < 0) {
            record = 0;
        }
        HuWinInsertMesSet(window, lbl_1_data_88[0], (s16)lbl_1_data_30[3]);
        digitIndex = 0;
        while ((digitIndex < 4) && (record > 0)) {
            digit = record % 10;
            record = (record - digit) / 10;
            HuWinInsertMesSet(window, lbl_1_data_88[digit],
                              (s16)lbl_1_data_30[3 - digitIndex]);
            digitIndex++;
        }
        HuWinInsertMesSet(window, MESSNUM(MESS_MGM_DECATHLON, 36), (s16)lbl_1_data_30[4]);
    }
}

extern s16 fn_1_9784(s16 player);
extern s16 fn_1_97BC(s16 player);
extern s16 fn_1_9744(s16 player);
extern s32 fn_1_50E4(s32 mode, s32 record, s32 score);
extern void fn_1_E058(s16 *playerOrder, s16 *scores);
extern void fn_1_D484(s16 *values);
extern s32 frandmod(s32 modulus);
extern void fn_1_3878(HUWINID window, s16 mode, s16 row, s16 visible);
extern void fn_1_6E38(void);

/* The player-score formatter clears the player's insertion text before appending its digits. */
static inline void decathlonResultEntryClear(s16 index) {
    memset(&lbl_1_bss_24[index], 0, sizeof(lbl_1_bss_24[index]));
}

/* Score formatting appends a tab and clears unused text bytes. If the text is full, the tab
 * lands in the adjacent status byte and is cleared instead. */
static inline void decathlonResultEntryAppendTab(s16 playerNo) {
    s16 textOffset;

    textOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (lbl_1_bss_24[playerNo].messageBytes[textOffset] == 0) {
            break;
        }
        textOffset++;
    }
    lbl_1_bss_24[playerNo].messageBytes[textOffset] = DECATHLON_TEXT_TAB;
    textOffset++;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        lbl_1_bss_24[playerNo].messageBytes[textOffset] = 0;
        textOffset++;
    }
    lbl_1_bss_24[playerNo].enabled = 0;
}

static inline MgmDecathlonPlayerRecord *decathlonResultEntryGet(s16 playerNo) {
    lbl_1_bss_24[playerNo].enabled = 0;
    return &lbl_1_bss_24[playerNo];
}

/* Score formatting appends each message's bytes to the selected player's result text. */
static inline void decathlonResultMessageSet(s16 playerNo, u32 messageId) {
    u8 *messageBytes = (u8 *)HuWinMesPtrGet(messageId);
    s16 textOffset;
    s16 sourceOffset;

    textOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (lbl_1_bss_24[playerNo].messageBytes[textOffset] == 0) {
            break;
        }
        textOffset++;
    }
    sourceOffset = 0;
    while (textOffset < DECATHLON_RESULT_TEXT_CAPACITY) {
        if (messageBytes[sourceOffset] == 0) {
            break;
        }
        /* A full buffer puts its terminator in the adjacent status byte, also cleared below. */
        lbl_1_bss_24[playerNo].messageBytes[textOffset] = messageBytes[sourceOffset];
        lbl_1_bss_24[playerNo].messageBytes[textOffset + 1] = 0;
        textOffset++;
        sourceOffset++;
    }
    lbl_1_bss_24[playerNo].enabled = 0;
}

/* The result sequence calls this to display scores and any newly earned record. */
void fn_1_337C(void) {
    HUWIN *window;
    s16 scorePlayer;
    s16 bestPlayer;
    s16 leadPlayer;
    s16 modeIndex;
    s32 record;
    s32 score;
    s32 scoreCopy;

    window = &winData[lbl_1_bss_18.windowId];
    HuWinExOpen(lbl_1_bss_18.windowId);
    fn_1_6E38();
    HuWinComKeyReset();
    fn_1_220C();
    HuWinAttrReset(lbl_1_bss_18.windowId, HUWIN_ATTR_ALIGN_CENTER);
    /* Tabs place the paired player scores in separate columns. */
    window->tabW = 252;
    fn_1_3878(lbl_1_bss_18.windowId, MgModePlayNum, 0, 1);
    fn_1_3878(lbl_1_bss_18.windowId, MgModePlayNum, 1, 0);
    fn_1_3878(lbl_1_bss_18.windowId, MgModePlayNum, 2, 1);
    fn_1_3878(lbl_1_bss_18.windowId, MgModePlayNum, 3, 0);
    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 8));
    HuWinMesWait(lbl_1_bss_18.windowId);
    /* Later prompts use the window's normal tab spacing and centered text. */
    window->tabW = 24;
    HuWinAttrSet(lbl_1_bss_18.windowId, HUWIN_ATTR_ALIGN_CENTER);
    if (MgModePlayNum < 10) {
        HuWinComKeyReset();
        fn_1_220C();
        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 9));
        HuWinMesWait(lbl_1_bss_18.windowId);
    } else {
        record = GwCommon.decaMgRecord[MgModePlayNum - 1];
        bestPlayer = -1;
        for (scorePlayer = 0; scorePlayer < 4; scorePlayer++) {
            score = GwPlayer[scorePlayer].mgScore;
            scoreCopy = score;
            if ((s16)fn_1_50E4(MgModePlayNum, record, scoreCopy) != 0) {
                record = scoreCopy;
                bestPlayer = scorePlayer;
            }
        }
        if (bestPlayer >= 0) {
            HuWinComKeyReset();
            fn_1_220C();
            bestPlayer = fn_1_9744(bestPlayer);
            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                              lbl_1_data_50[bestPlayer], lbl_1_data_30[0]);
            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 26));
            HuWinMesWait(lbl_1_bss_18.windowId);
        }
        HuWinComKeyReset();
        fn_1_220C();
        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 10));
        HuWinMesWait(lbl_1_bss_18.windowId);
    }
    HuWinExClose(lbl_1_bss_18.windowId);
    HuPrcSleep(10);
}

/* The result panel formats one player's score, subtracting one from the supplied completed-event
 * count to select the event's format. */
void fn_1_3878(HUWINID window, s16 mode, s16 playerNo, s16 appendControlCode) {
    s32 messageScore;
    u32 messageId;
    u32 messageIds[8];
    s32 score;
    s32 playerScore;
    s32 timeScore;
    s32 digit;
    s16 messageIndex;
    s16 textPosition;
    s16 characterIndex;

    for (messageIndex = 0; messageIndex < 8; messageIndex++) {
        messageIds[messageIndex] = DECATHLON_UNUSED_MESSAGE;
    }
    mode--;
    if (mode == 1) {
        playerScore = GwPlayer[playerNo].mgScore;
        score = playerScore;
        if (score > DECATHLON_SCORE_MAX) score = DECATHLON_SCORE_MAX;
        if (score < 0) score = 0;
        messageIds[3] = lbl_1_data_88[0];
        for (messageIndex = 0; messageIndex < 4 && score > 0; messageIndex++) {
            digit = score % 10; score = (score - digit) / 10;
            messageIds[3 - messageIndex] = lbl_1_data_88[digit];
        }
        messageIds[4] = MESSNUM(MESS_MGM_DECATHLON, 35);
    } else if ((mode == 2) || (mode == 7)) {
        s32 timeInputScore;

        timeScore = GwPlayer[playerNo].mgScore;
        score = timeScore;
        if (score > DECATHLON_TIME_SCORE_MAX) score = DECATHLON_TIME_SCORE_MAX;
        if (score < 0) score = 0;
        score = (score - score % 60) / 60;
        messageIds[0] = lbl_1_data_88[0]; messageIds[1] = lbl_1_data_88[0];
        for (messageIndex = 0; messageIndex < 2 && score > 0; messageIndex++) {
            digit = score % 10; score = (score - digit) / 10;
            messageIds[1 - messageIndex] = lbl_1_data_88[digit];
        }
        messageIds[2] = MESSNUM(MESS_MGM_DECATHLON, 34);
        timeInputScore = GwPlayer[playerNo].mgScore;
        score = timeInputScore;
        if (score > DECATHLON_TIME_SCORE_MAX) score = DECATHLON_TIME_SCORE_MAX;
        if (score < 0) score = 0;
        score = score % 60;
        score = (s32)(100.0f * (0.016666668f * (f32)score));
        messageIds[3] = lbl_1_data_88[0]; messageIds[4] = lbl_1_data_88[0];
        for (messageIndex = 0; messageIndex < 2 && score > 0; messageIndex++) {
            digit = score % 10; score = (score - digit) / 10;
            messageIds[4 - messageIndex] = lbl_1_data_88[digit];
        }
        /* Replace all four digits of a zero time with the event's placeholder message. */
        if (messageIds[0] == lbl_1_data_88[0] &&
            messageIds[1] == lbl_1_data_88[0] &&
            messageIds[3] == lbl_1_data_88[0] &&
            messageIds[4] == lbl_1_data_88[0]) {
            messageIds[0] = MESSNUM(MESS_MGM_DECATHLON, 56);
            messageIds[1] = MESSNUM(MESS_MGM_DECATHLON, 56);
            messageIds[3] = MESSNUM(MESS_MGM_DECATHLON, 56);
            messageIds[4] = MESSNUM(MESS_MGM_DECATHLON, 56);
        }
    } else if (mode == 3) {
        s32 gameInputScore;

        gameInputScore = GwPlayer[playerNo].mgScore;
        score = gameInputScore;
        if (score > DECATHLON_SCORE_MAX) score = DECATHLON_SCORE_MAX;
        if (score < 0) score = 0;
        messageIds[3] = lbl_1_data_88[0];
        for (messageIndex = 0; messageIndex < 4 && score > 0; messageIndex++) {
            digit = score % 10; score = (score - digit) / 10;
            messageIds[3 - messageIndex] = lbl_1_data_88[digit];
        }
        messageIds[4] = MESSNUM(MESS_MGM_DECATHLON, 37);
    } else {
        s32 rawScore;

        rawScore = GwPlayer[playerNo].mgScore;
        score = rawScore;
        if (score > DECATHLON_SCORE_MAX) score = DECATHLON_SCORE_MAX;
        if (score < 0) score = 0;
        messageScore = score;
        messageIds[3] = lbl_1_data_88[0];
        for (messageIndex = 0; messageIndex < 4 && score > 0; messageIndex++) {
            digit = score % 10; score = (score - digit) / 10;
            messageIds[3 - messageIndex] = lbl_1_data_88[digit];
        }
        if (messageScore == 1) messageIds[4] = MESSNUM(MESS_MGM_DECATHLON, 59);
        else messageIds[4] = MESSNUM(MESS_MGM_DECATHLON, 36);
    }

    decathlonResultEntryClear(playerNo);
    for (messageIndex = 0; messageIndex < 8; messageIndex++) {
        if (messageIds[messageIndex] != DECATHLON_UNUSED_MESSAGE) {
            messageId = messageIds[messageIndex];
            decathlonResultMessageSet(playerNo, messageId);
        }
    }
    if (appendControlCode != 0) {
        decathlonResultEntryAppendTab(playerNo);
    }
    HuWinInsertMesSet(window, (u32)decathlonResultEntryGet(playerNo),
                      (s16)lbl_1_data_30[playerNo]);
}

/* Returns the player's current minigame score. */
static inline u32 GWMgScoreGet(s32 playerNo) {
    return GwPlayer[playerNo].mgScore;
}

/* After a round, the sequence announces records or ranking changes and avoids repeating
 * commentary. */
s32 fn_1_4130(void) {
    s32 allEventsComplete = 0;
    s32 record;
    s32 scoreCopy;
    s16 player;
    s16 playerOrder[4];
    s16 currentRanks[4];
    s16 previousRanks[4];
    s16 announcementValue; /* score gap, tie count, random choice, or selected character as
                            * needed */
    s16 bestPlayer;
    s16 runnerUpValue; /* count of second-place players, then the runner-up's character */
    fn_1_E058(playerOrder, currentRanks);
    fn_1_D484(previousRanks);
    if (MgModePlayNum >= 10)
    {
        allEventsComplete = 1;
    }
    else
    {
        HuWinExOpen(lbl_1_bss_18.windowId);
        fn_1_6E38();
        HuWinComKeyReset();
        fn_1_220C();
        record = GwCommon.decaMgRecord[MgModePlayNum - 1];
        bestPlayer = - 1;
        for (player = 0; player < 4; player++)
        {
            scoreCopy = GWMgScoreGet(playerOrder[player]);
            if ((s16) fn_1_50E4(MgModePlayNum, record, scoreCopy) != 0)
            {
                record = scoreCopy;
                bestPlayer = player;
            }
        }
        if (bestPlayer >= 0)
        {
            if (currentRanks[bestPlayer] <= 0)
            {
                announcementValue = fn_1_9744(playerOrder[bestPlayer]);
                HuWinInsertMesSet(lbl_1_bss_18.windowId, lbl_1_data_50[announcementValue],
                                  (s16) lbl_1_data_30[0]);
                HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 20));
                HuWinMesWait(lbl_1_bss_18.windowId);
                goto resultMessageDone;
            }
            else if (currentRanks[bestPlayer] > 0)
            {
                announcementValue = fn_1_9744(playerOrder[bestPlayer]);
                HuWinInsertMesSet(lbl_1_bss_18.windowId, lbl_1_data_50[announcementValue],
                                  (s16) lbl_1_data_30[0]);
                HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 21));
                HuWinMesWait(lbl_1_bss_18.windowId);
                goto resultMessageDone;
            }
        }
        {
            /* A low leading total takes priority over changes in the ranking. */
            if (MgModePlayNum >= 3 && (announcementValue = MgModeScore[playerOrder[0]]) <= 500)
            {
                if (MgModeWorkInt[15] != 11)
                {
                    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 19));
                    HuWinMesWait(lbl_1_bss_18.windowId);
                    MgModeWorkInt[15] = 11;
                }
                else { goto resultFallback; }
                goto resultMessageDone;
            }
            /* Check whether every player's tied rank changed, using early or late commentary. */
            if (MgModePlayNum <= 5)
            {
                for (player = 0; player < 4; player++)
                {
                    if (previousRanks[player] == currentRanks[player])
                    {
                        break;
                    }
                }
                if (player >= 4)
                {
                    if (MgModeWorkInt[15] != 1)
                    {
                        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 22));
                        HuWinMesWait(lbl_1_bss_18.windowId);
                        MgModeWorkInt[15] = 1;
                    }
                    else { goto resultFallback; }
                    goto resultMessageDone;
                }
            }
            {
                if (MgModePlayNum >= 6)
                {
                    for (player = 0; player < 4; player++)
                    {
                        if (previousRanks[player] == currentRanks[player])
                        {
                            break;
                        }
                    }
                    if (player >= 4)
                    {
                        if (MgModeWorkInt[15] != 2)
                        {
                            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 23));
                            HuWinMesWait(lbl_1_bss_18.windowId);
                            MgModeWorkInt[15] = 2;
                        }
                        else { goto resultFallback; }
                        goto resultMessageDone;
                    }
                }
            }
            if (MgModePlayNum >= 4 && 7 >= MgModePlayNum)
            {
                for (player = 0; player < 4; player++)
                {
                    if (previousRanks[player] > currentRanks[player])
                    {
                        if (previousRanks[player] - currentRanks[player] >= 2)
                        {
                        if (MgModeWorkInt[15] != 3)
                        {
                            announcementValue = fn_1_9744(playerOrder[player]);
                            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                              lbl_1_data_50[announcementValue],
                                              (s16) lbl_1_data_30[0]);
                            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 24));
                            HuWinMesWait(lbl_1_bss_18.windowId);
                            MgModeWorkInt[15] = 3;
                        }
                        else
                        {
                            goto resultFallback;
                        }
                        goto resultMessageDone;
                        }
                    }
                }
            }
            if (MgModePlayNum >= 8 && MgModePlayNum <= 9)
            {
                    for (player = 0; player < 4; player++)
                    {
                        if (previousRanks[player] > currentRanks[player] &&
                            previousRanks[player] - currentRanks[player] >= 2) {
                            if (MgModeWorkInt[15] != 4)
                            {
                                announcementValue = fn_1_9744(playerOrder[player]);
                                HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                                  lbl_1_data_50[announcementValue],
                                                  (s16) lbl_1_data_30[0]);
                                HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 25));
                                HuWinMesWait(lbl_1_bss_18.windowId);
                                MgModeWorkInt[15] = 4;
                            }
                            else
                            {
                                goto resultFallback;
                            }
                            goto resultMessageDone;
                        }
                    }
            }
            {
                /* Score gaps distinguish a runaway leader, a close field, and a trailing player. */
                announcementValue = MgModeScore[playerOrder[0]] - MgModeScore[playerOrder[1]];
                if (announcementValue >= 1000)
                {
                    if (MgModeWorkInt[15] == 5) { goto resultFallback; }
                    announcementValue = fn_1_9744(playerOrder[0]);
                    HuWinInsertMesSet(lbl_1_bss_18.windowId, lbl_1_data_50[announcementValue],
                                      (s16) lbl_1_data_30[0]);
                    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 11));
                    HuWinMesWait(lbl_1_bss_18.windowId);
                    MgModeWorkInt[15] = 5;
                }
                else
                {
                    if ((announcementValue =
                             MgModeScore[playerOrder[0]] - MgModeScore[playerOrder[3]]) < 100) {
                        if (MgModeWorkInt[15] == 6) { goto resultFallback; }
                        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 12));
                        HuWinMesWait(lbl_1_bss_18.windowId);
                        MgModeWorkInt[15] = 6;
                    } else {
                        if ((announcementValue = MgModeScore[playerOrder[2]] -
                                              MgModeScore[playerOrder[3]]) >= 500) {
                            if (MgModeWorkInt[15] == 7) { goto resultFallback; }
                            announcementValue = fn_1_9744(playerOrder[3]);
                            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                              lbl_1_data_50[announcementValue],
                                              (s16) lbl_1_data_30[0]);
                            HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 13));
                            HuWinMesWait(lbl_1_bss_18.windowId);
                            MgModeWorkInt[15] = 7;
                        } else {
                            if ((announcementValue = MgModeScore[playerOrder[0]] -
                                                  MgModeScore[playerOrder[3]]) < 500) {
                                if (MgModeWorkInt[15] != 8)
                                {
                                    if (! (! (frandmod(2) != 0)))
                                    {
                                        HuWinMesSet(lbl_1_bss_18.windowId,
                                                    MESSNUM(MESS_MGM_DECATHLON, 14));
                                    }
                                    else
                                    {
                                        HuWinMesSet(lbl_1_bss_18.windowId,
                                                    MESSNUM(MESS_MGM_DECATHLON, 15));
                                    }
                                    HuWinMesWait(lbl_1_bss_18.windowId);
                                    MgModeWorkInt[15] = 8;
                                }
                                else
                                {
                                    goto resultFallback;
                                }
                            } else {
                                if ((announcementValue = MgModeScore[playerOrder[0]] -
                                                      MgModeScore[playerOrder[3]]) >= 1000) {
                                    if (MgModeWorkInt[15] != 9)
                                    {
                                        HuWinMesSet(lbl_1_bss_18.windowId,
                                                    MESSNUM(MESS_MGM_DECATHLON, 16));
                                        HuWinMesWait(lbl_1_bss_18.windowId);
                                        MgModeWorkInt[15] = 9;
                                    }
                                    else
                                    {
                                        goto resultFallback;
                                    }
                                } else {
                                    announcementValue = runnerUpValue = 0;
                                    for (player = 0; player < 4; ++player)
                                    {
                                        if (currentRanks[player] == 0)
                                        {
                                            announcementValue++;
                                        }
                                        if (currentRanks[player] == 1)
                                        {
                                            runnerUpValue++;
                                        }
                                    }
                                    if (((announcementValue == 1 && runnerUpValue == 1) ||
                                         announcementValue == 2) &&
                                        MgModeScore[playerOrder[0]] - MgModeScore[playerOrder[1]] <
                                            100) {
                                        if (MgModeWorkInt[15] != 10)
                                        {
                                            announcementValue = fn_1_9744(playerOrder[0]);
                                            runnerUpValue = fn_1_9744(playerOrder[1]);
                                            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                                              lbl_1_data_50[announcementValue],
                                                              (s16) lbl_1_data_30[0]);
                                            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                                              lbl_1_data_50[runnerUpValue],
                                                              (s16) lbl_1_data_30[1]);
                                            HuWinMesSet(lbl_1_bss_18.windowId,
                                                        MESSNUM(MESS_MGM_DECATHLON, 18));
                                            HuWinMesWait(lbl_1_bss_18.windowId);
                                            MgModeWorkInt[15] = 10;
                                        }
                                        else
                                        {
                                            goto resultFallback;
                                        }
                                    } else {
                                        if (MgModePlayNum <= 2 &&
                                            (announcementValue = MgModeScore[playerOrder[0]]) <=
                                                500 &&
                                            MgModeWorkInt[15] != 11) {
                                            HuWinMesSet(lbl_1_bss_18.windowId,
                                                        MESSNUM(MESS_MGM_DECATHLON, 19));
                                            HuWinMesWait(lbl_1_bss_18.windowId);
                                            MgModeWorkInt[15] = 11;
                                        } else {
                                        resultFallback:
                                            /* Choose a player and one of seven messages if no fresh
                                             * condition applies. */
                                            announcementValue = fn_1_9744(frandmod(4));
                                            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                                              lbl_1_data_50[announcementValue],
                                                              (s16) lbl_1_data_30[0]);
                                            announcementValue = frandmod(70);
                                            if (announcementValue < 10)
                                            {
                                                HuWinInsertMesSet(lbl_1_bss_18.windowId,
                                                                  MESSNUM(MESS_MGM_DECATHLON, 38),
                                                                  (s16) lbl_1_data_30[1]);
                                            }
                                            else
                                            {
                                                if (announcementValue < 20)
                                                {
                                                    HuWinInsertMesSet(
                                                        lbl_1_bss_18.windowId,
                                                        MESSNUM(MESS_MGM_DECATHLON, 39),
                                                        (s16) lbl_1_data_30[1]);
                                                }
                                                else
                                                {
                                                    if (announcementValue < 30)
                                                    {
                                                        HuWinInsertMesSet(
                                                            lbl_1_bss_18.windowId,
                                                            MESSNUM(MESS_MGM_DECATHLON, 40),
                                                            (s16) lbl_1_data_30[1]);
                                                    }
                                                    else
                                                    {
                                                        if (announcementValue < 40)
                                                        {
                                                            HuWinInsertMesSet(
                                                                lbl_1_bss_18.windowId,
                                                                MESSNUM(MESS_MGM_DECATHLON, 41),
                                                                (s16) lbl_1_data_30[1]);
                                                        }
                                                        else
                                                        {
                                                            if (announcementValue < 50)
                                                            {
                                                                HuWinInsertMesSet(
                                                                    lbl_1_bss_18.windowId,
                                                                    MESSNUM(MESS_MGM_DECATHLON, 42),
                                                                    (s16) lbl_1_data_30[1]);
                                                            }
                                                            else
                                                            {
                                                                if (announcementValue < 60)
                                                                {
                                                                    HuWinInsertMesSet(
                                                                        lbl_1_bss_18.windowId,
                                                                        MESSNUM(MESS_MGM_DECATHLON,
                                                                                43),
                                                                        (s16) lbl_1_data_30[1]);
                                                                }
                                                                else
                                                                {
                                                                    HuWinInsertMesSet(
                                                                        lbl_1_bss_18.windowId,
                                                                        MESSNUM(MESS_MGM_DECATHLON,
                                                                                44),
                                                                        (s16) lbl_1_data_30[1]);
                                                                }
                                                            }
                                                        }
                                                    }
                                                }
                                            }
                                            HuWinMesSet(lbl_1_bss_18.windowId,
                                                        MESSNUM(MESS_MGM_DECATHLON, 17));
                                            HuWinMesWait(lbl_1_bss_18.windowId);
                                            MgModeWorkInt[15] = 0;
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
        resultMessageDone : HuWinComKeyReset();
        fn_1_220C();
        HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 7));
        HuWinMesWait(lbl_1_bss_18.windowId);
        HuWinExClose(lbl_1_bss_18.windowId);
        allEventsComplete = 0;
    }
    HuPrcSleep(10);
    return allEventsComplete;
}

typedef struct DecathlonMessageText {
    s8 first; /* first text byte; this two-byte view is unused */
    s8 second; /* following text byte; this two-byte view is unused */
} DecathlonMessageText;

extern s16 MgModeScore[4];
extern s16 fn_1_9784(s16 playerNo);
extern s16 fn_1_97BC(s16 playerNo);
extern void fn_1_6E38(void);

/* Result screens test whether the new score beats a record; timed events require a positive
 * time. */
s32 fn_1_50E4(s32 mode, s32 firstScore, s32 secondScore) {
    mode -= 1;

    if (((s16)mode == 2) || ((s16)mode == 7)) {
        if ((firstScore > secondScore) && (secondScore > 0)) {
            return 1;
        }
    } else if (firstScore < secondScore) {
        return 1;
    }
    return 0;
}

/* The final result sequence calls this to announce the leading Decathlon score and players. */
void fn_1_5130(void) {
    s16 playerCharacters[4];
    s16 insertWindow;
    s16 scoreMax;
    s16 playerNo;
    s16 scoreDigits;
    s16 characterIndex;
    s16 digitOrLeaderIndex; /* score digit, leader count, then count-minus-one message index */
    s16 insertionSlot;

    {
        DecathlonScoreMessages scoreMessages = {
            { MESSNUM(MESS_MGM_DECATHLON, 29), MESSNUM(MESS_MGM_DECATHLON, 30),
              MESSNUM(MESS_MGM_DECATHLON, 31), MESSNUM(MESS_MGM_DECATHLON, 32) }
        };
        scoreMax = MgModeScore[0];
        playerNo = 0;
        while (playerNo < 4) {
            if (scoreMax < MgModeScore[playerNo]) {
                scoreMax = MgModeScore[playerNo];
            }
            playerNo++;
    }
    playerNo = 0;
    while (playerNo < 4) {
        playerCharacters[playerNo] = -1;
        if (scoreMax <= MgModeScore[playerNo]) {
            playerCharacters[playerNo] = GwPlayerConf[playerNo].charNo;
        }
        playerNo++;
    }
    HuWinExOpen(lbl_1_bss_18.windowId);
    fn_1_6E38();
    HuWinComKeyReset();
    fn_1_220C();
    HuWinComKeyReset();
    fn_1_220C();
    insertWindow = lbl_1_bss_18.windowId;
    for (characterIndex = 0; characterIndex < 8; characterIndex++) {
        insertionSlot = lbl_1_data_30[characterIndex];
        {
            fn_1_1D80(insertWindow, insertionSlot);
        }
    }
    HuWinInsertMesSet(lbl_1_bss_18.windowId, lbl_1_data_88[0], (s16)lbl_1_data_30[4]);
    scoreDigits = scoreMax;
    playerNo = 0;
    while ((playerNo < 5) && (scoreDigits > 0)) {
        digitOrLeaderIndex = scoreDigits % 10;
        HuWinInsertMesSet(lbl_1_bss_18.windowId, lbl_1_data_88[digitOrLeaderIndex],
                          (s16)lbl_1_data_30[4 - playerNo]);
        scoreDigits = scoreDigits / 10;
        playerNo++;
    }
    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 28));
    HuWinMesWait(lbl_1_bss_18.windowId);
    HuWinComKeyReset();
    fn_1_220C();
    playerNo = 0;
    digitOrLeaderIndex = 0;
    while (playerNo < 4) {
        if (playerCharacters[playerNo] >= 0) {
            HuWinInsertMesSet(lbl_1_bss_18.windowId,
                              lbl_1_data_50[playerCharacters[playerNo]],
                              (s16)lbl_1_data_30[digitOrLeaderIndex]);
            digitOrLeaderIndex++;
        }
        playerNo++;
    }
    digitOrLeaderIndex--;
    HuWinMesSet(lbl_1_bss_18.windowId, scoreMessages.messageIds[digitOrLeaderIndex]);
    HuWinMesWait(lbl_1_bss_18.windowId);
    HuAudSStreamFadeOut(lbl_1_data_C70, 200);
    HuWinExClose(lbl_1_bss_18.windowId);
    HuPrcSleep(10);
}
}

extern s16 fn_1_9784(s16 player);
extern s16 fn_1_97BC(s16 player);
extern void fn_1_6E38(void);

/* The end-of-mode sequence calls this to ask whether to return to the board. */
s32 fn_1_567C(void) {
    s32 result;
    s16 padMask;
    s16 player;
    s16 choice;

    result = 0;
    HuWinExOpen(lbl_1_bss_18.windowId);
    HuWinComKeyReset();
    fn_1_220C();
    HuWinAttrSet(lbl_1_bss_18.windowId, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(lbl_1_bss_18.windowId, MESSNUM(MESS_MGM_DECATHLON, 33));
    HuWinMesWait(lbl_1_bss_18.windowId);
    choice = HuWinChoiceGet(lbl_1_bss_18.windowId, -1);
    HuWinAttrReset(lbl_1_bss_18.windowId, HUWIN_ATTR_NOCANCEL);
    if (choice != 0) {
        result = 0;
    } else {
        result = 1;
    }
    HuWinExClose(lbl_1_bss_18.windowId);
    HuPrcSleep(10);
    return result;
}

/* The shared window callback plays the cue paired with the current message and callback code. */
void fn_1_57F4(HUWINID winId, u32 messageId, s16 characterCode) {
    s16 entryIndex;

    for (entryIndex = 0; entryIndex < 42; entryIndex++) {
        if ((messageId == lbl_1_data_B0[entryIndex].messageId) &&
            (characterCode == lbl_1_data_B0[entryIndex].characterCode) &&
            (lbl_1_data_B0[entryIndex].soundEffectId >= 0)) {
            HuAudFXPlayPan(lbl_1_data_B0[entryIndex].soundEffectId,
                           lbl_1_data_2A8);
            fn_1_6E38();
        }
    }
}

MgmDecathlonPlayerRecord lbl_1_bss_24[4];
DecathlonWindowState lbl_1_bss_18;

s32 lbl_1_data_30[8] = {0, 1, 2, 3, 4, 5, 6, 7};
u32 lbl_1_data_50[14] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13};
u32 lbl_1_data_88[10] = {
    MESSNUM(MESS_MGM_DECATHLON, 55), MESSNUM(MESS_MGM_DECATHLON, 45),
    MESSNUM(MESS_MGM_DECATHLON, 46), MESSNUM(MESS_MGM_DECATHLON, 47),
    MESSNUM(MESS_MGM_DECATHLON, 48), MESSNUM(MESS_MGM_DECATHLON, 49),
    MESSNUM(MESS_MGM_DECATHLON, 50), MESSNUM(MESS_MGM_DECATHLON, 51),
    MESSNUM(MESS_MGM_DECATHLON, 52), MESSNUM(MESS_MGM_DECATHLON, 53)
};
DecathlonMessageSound lbl_1_data_B0[42] = {
    {MESSNUM(MESS_MGM_DECATHLON, 0), 1, MSM_SE_GUIDE_26},
    {MESSNUM(MESS_MGM_DECATHLON, 0), 2, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 1), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 2), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 3), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 4), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 5), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 6), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 7), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 8), 1, -1},
    {MESSNUM(MESS_MGM_DECATHLON, 9), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 10), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 11), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 11), 2, -1},
    {MESSNUM(MESS_MGM_DECATHLON, 12), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 13), 1, MSM_SE_GUIDE_19},
    {MESSNUM(MESS_MGM_DECATHLON, 14), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 15), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 16), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 16), 2, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 17), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 17), 2, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 18), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 19), 1, MSM_SE_GUIDE_19},
    {MESSNUM(MESS_MGM_DECATHLON, 19), 2, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 20), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 21), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 22), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 23), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 24), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 25), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 27), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 28), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 29), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 30), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 31), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 32), 1, MSM_SE_GUIDE_18},
    {MESSNUM(MESS_MGM_DECATHLON, 33), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 58), 1, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 58), 2, -1},
    {MESSNUM(MESS_MGM_DECATHLON, 58), 3, MSM_SE_GUIDE_20},
    {MESSNUM(MESS_MGM_DECATHLON, 58), 4, -1},
};
s16 lbl_1_data_2A8 = 64;
