/* Shows tournament prompts, player choices and message sound callbacks. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/window.h"
#include "game/process.h"
#include "game/audio.h"
#include "game/mgdata.h"
#include "dolphin/pad.h"
#include "messdir_enum.h"
#include "msm_se.h"

typedef struct TournamentWindow {
    HUWINID windowId; /* Shared framed dialogue window. */
    float width; /* Dialogue frame width in screen pixels. */
    float height; /* Dialogue frame height in screen pixels. */
} TournamentWindow;

typedef struct TournamentMessageSound {
    u32 messageId; /* Message containing the sound callback marker. */
    s16 callbackCode; /* Byte following a message's callback control code. */
    s32 soundEffectId; /* Guide sound effect, or -1 for a silent marker. */
} TournamentMessageSound;

TournamentWindow lbl_1_bss_C8;
extern int MgModeWorkInt[16];
extern s32 lbl_1_data_848;
void fn_1_5728(void);
s32 fn_1_6A24(void);
void fn_1_71E8(s16 *firstPlayer, s16 *secondPlayer);
s16 fn_1_7BC0(s16 player);
s16 fn_1_7C00(s16 player);
s16 fn_1_7C44(s16 player);
s16 fn_1_A42C(void);
void fn_1_2D1C(HUWINID windowId, u32 messageId, s16 callbackCode);
s32 lbl_1_data_140[8] = { 0, 1, 2, 3, 4, 5, 6, 7 };
s32 lbl_1_data_160[14] = {
    MESSNUM(MESS_CHARA_NAME, 0), MESSNUM(MESS_CHARA_NAME, 1),
    MESSNUM(MESS_CHARA_NAME, 2), MESSNUM(MESS_CHARA_NAME, 3),
    MESSNUM(MESS_CHARA_NAME, 4), MESSNUM(MESS_CHARA_NAME, 5),
    MESSNUM(MESS_CHARA_NAME, 6), MESSNUM(MESS_CHARA_NAME, 7),
    MESSNUM(MESS_CHARA_NAME, 8), MESSNUM(MESS_CHARA_NAME, 9),
    MESSNUM(MESS_CHARA_NAME, 10), MESSNUM(MESS_CHARA_NAME, 11),
    MESSNUM(MESS_CHARA_NAME, 12), MESSNUM(MESS_CHARA_NAME, 13)
};
TournamentMessageSound lbl_1_data_198[18] = {
    { MESSNUM(MESS_MGM_TOURNAMENT, 0), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 0), 2, -1 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 0), 3, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 0), 4, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 1), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 2), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 3), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 4), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 5), 1, -1 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 6), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 7), 1, MSM_SE_GUIDE_19 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 8), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 9), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 10), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 11), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 12), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 13), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_TOURNAMENT, 14), 1, MSM_SE_GUIDE_20 },
};
s16 lbl_1_data_270 = 64;

/* Dialogue setup clears one insertion slot by registering an empty string. */
void fn_1_199C(s16 windowId, s16 insertionSlot) {
	char messageText[2] = "";

	messageText[0] = messageText[1] = 0;
	/* The window keeps the local string's pointer rather than copying its text. */
	HuWinInsertMesSet(windowId, (u32) &messageText, insertionSlot);
}

/* Before the round options open, clear all eight dialogue insertion slots. */
void fn_1_19F4(s16 windowId)
{
    s16 messageIndex;
    for (messageIndex = 0; messageIndex < 8; ++messageIndex) {
        fn_1_199C(windowId, (s16)lbl_1_data_140[messageIndex]);
    }
}

/* Tournament scene setup creates the centered dialogue frame and its guide sound callback. */
void fn_1_1A8C(void)
{
    f32 windowY;

    HuWinInit(0);
    lbl_1_bss_C8.width = 528.0f;
    lbl_1_bss_C8.height = 80.0f;
    windowY = 16.0f + (440.0f - lbl_1_bss_C8.height);
    lbl_1_bss_C8.windowId = HuWinExCreateFrame(-10000.0f, windowY,
                                      (s16)lbl_1_bss_C8.width,
                                      (s16)lbl_1_bss_C8.height, -1, 3);
    HuWinAttrSet(lbl_1_bss_C8.windowId, HUWIN_ATTR_ALIGN_CENTER);
    HuWinCallbackSet(lbl_1_bss_C8.windowId, fn_1_2D1C);
}

/* Scene exit destroys the shared dialogue frame and clears the remaining windows. */
void fn_1_1BA0(void) {
    HuWinExKill(lbl_1_bss_C8.windowId);
    HuWinAllKill();
}

/* General tournament prompts accept input from every human player's controller. */
void fn_1_1BD0(void) {

    s16 padMask;
    s16 player;

    padMask = 0;
    {
    s16 controllerMasks[4] = { 1, 2, 4, 8 };
    player = 0;
    while (player < 4) {
        if (fn_1_7C00(player) == 0) {
            padMask |= controllerMasks[fn_1_7C44(player)];
        }
        player++;
    }
    HuWinPadMaskSet(lbl_1_bss_C8.windowId, padMask);
}
}

/* Round prompts limit human input to the two players selected by the bracket. */
void fn_1_1C84(void) {

    s16 firstPlayer;
    s16 secondPlayer;
    s16 padMask;

    padMask = 0;
    {
    s16 controllerMasks[4] = { 1, 2, 4, 8 };
    fn_1_71E8(&firstPlayer, &secondPlayer);
    if (fn_1_7C00(firstPlayer) == 0) {
        padMask |= controllerMasks[fn_1_7C44(firstPlayer)];
    }
    if (fn_1_7C00(secondPlayer) == 0) {
        padMask |= controllerMasks[fn_1_7C44(secondPlayer)];
    }
    HuWinPadMaskSet(lbl_1_bss_C8.windowId, padMask);
}
}

/* Clears queued automatic dialogue input between tournament prompts. */
void fn_1_1D54(void)
{
    HuWinComKeyReset();
}

/* The opening scene shows the first introduction prompt before arranging the tournament. */
void fn_1_1D74(void) {

    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    HuWinComKeyReset();
    fn_1_1BD0();
    HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 0));
    HuWinMesWait(lbl_1_bss_C8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
}

/* After the opening host movement, show the second tournament introduction prompt. */
void fn_1_1E80(void) {

    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    HuWinComKeyReset();
    fn_1_1BD0();
    HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 1));
    HuWinMesWait(lbl_1_bss_C8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
}

/* Before the day/night reveal, show the prompt for the current bracket matchup. */
void fn_1_1F90(void)
{

    s16 mode;
    s16 padMask;
    s16 player;

    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    HuWinComKeyReset();
    fn_1_1BD0();
    mode = fn_1_A42C();
    if (mode == 0) {
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 2));
    } else if (mode == 1) {
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 8));
    } else if (mode == 2) {
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 9));
    } else {
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 10));
    }
    HuWinMesWait(lbl_1_bss_C8.windowId);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
}

/* The scene asks how to play the selected game: -1 exits, 0 plays, 1 shows instructions,
 * and 2 advances without launching the game. A declined exit returns to the options. */
s16 fn_1_2128(void)
{
    s16 promptState;
    s16 result;
    s16 player;
    s16 messageIndex;
    HUWINID messageWindow;

    result = 0;
    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    fn_1_19F4(lbl_1_bss_C8.windowId);
    promptState = 1;
    while (promptState > 0) {
        switch (promptState) {
        case 1:
            HuWinComKeyReset();
            if (fn_1_6A24() != 0) {
                fn_1_1BD0();
            } else {
                fn_1_1C84();
            }
            HuWinInsertMesSet(lbl_1_bss_C8.windowId,
                              MgDataTbl[MgModeWork[MgModePlayNum]].nameMes, 0);
            HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 3));
            HuWinMesWait(lbl_1_bss_C8.windowId);
            promptState = 3;
            HuWinComKeyReset();
            break;
        case 2:
            HuWinComKeyReset();
            fn_1_1BD0();
            HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 5));
            HuWinMesWait(lbl_1_bss_C8.windowId);
            result = HuWinChoiceGet(lbl_1_bss_C8.windowId, -1);
            promptState = 0;
            if (result == 0) {
                result = -1;
            } else {
                promptState = 3;
            }
            HuWinComKeyReset();
            break;
        case 3:
            HuWinComKeyReset();
            if (fn_1_6A24() != 0) {
                fn_1_1BD0();
                HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 14));
                HuWinMesWait(lbl_1_bss_C8.windowId);
                result = HuWinChoiceGet(lbl_1_bss_C8.windowId, -1);
                promptState = 0;
                if (result == 0) {
                    result = 0;
                } else if (result == 1) {
                    result = 2;
                } else {
                    promptState = 2;
                }
            } else {
                fn_1_1C84();
                HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 4));
                HuWinMesWait(lbl_1_bss_C8.windowId);
                result = HuWinChoiceGet(lbl_1_bss_C8.windowId, -1);
                promptState = 0;
                if (result == 0) {
                    result = 0;
                } else if (result == 1) {
                    promptState = 2;
                } else if (result == 2) {
                    result = 1;
                } else {
                    promptState = 2;
                }
            }
            HuWinComKeyReset();
            break;
        }
    }
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
    return result;
}

/* Result handling announces the winner's character, or the no-winner result for an invalid slot. */
void fn_1_2728(s16 player)
{

    s16 unusedSecondPlayer;
    s16 unusedFirstPlayer;
    s16 padMask;
    s16 winnerCharacter;

    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    if (fn_1_6A24() != 0) {
        HuWinComKeyReset();
        HuWinPadMaskSet(lbl_1_bss_C8.windowId, 1);
        /* Queue two automatic A presses, each after sixteen controller polls. */
        HuWinComKeyWait(PAD_BUTTON_A, 0, 0, 0, 16);
        HuWinComKeyWait(PAD_BUTTON_A, 0, 0, 0, 16);
    } else {
        HuWinComKeyReset();
        fn_1_1C84();
    }
    if ((player >= 0) && (player < 4)) {
        winnerCharacter = fn_1_7BC0(player);
        HuWinInsertMesSet(lbl_1_bss_C8.windowId, lbl_1_data_160[winnerCharacter],
                          (s16)lbl_1_data_140[0]);
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 6));
        HuWinMesWait(lbl_1_bss_C8.windowId);
    } else {
        HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 7));
        HuWinMesWait(lbl_1_bss_C8.windowId);
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
}

/* The final scene shows completion dialogue with the character in the remaining bracket slot. */
void fn_1_2948(void)
{

    s16 remainingCharacter;
    s16 unusedFirstPadMask;
    s16 unusedSecondPadMask;
    s16 player;
    s16 unusedFirstPlayer;
    s16 unusedSecondPlayer;

    for (player = 0; player < 4; player++) {
        if (MgModeWorkInt[player + 4] == 0) {
            remainingCharacter = fn_1_7BC0(player);
        }
    }

    HuWinExOpen(lbl_1_bss_C8.windowId);
    fn_1_5728();
    HuWinComKeyReset();
    fn_1_1BD0();
    HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 11));
    HuWinMesWait(lbl_1_bss_C8.windowId);
    HuWinComKeyReset();
    fn_1_1BD0();
    HuWinInsertMesSet(lbl_1_bss_C8.windowId, lbl_1_data_160[remainingCharacter],
                      (s16)lbl_1_data_140[0]);
    HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 12));
    HuWinMesWait(lbl_1_bss_C8.windowId);
    HuAudSStreamFadeOut(lbl_1_data_848, 200);
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
}

/* At tournament completion, ask whether to restart; the first choice returns one. */
s16 fn_1_2BA0(void) {

    s16 result;
    s16 padMask;
    s16 player;
    s16 choice;

    result = 0;
    HuWinExOpen(lbl_1_bss_C8.windowId);
    HuWinComKeyReset();
    fn_1_1BD0();
    HuWinAttrSet(lbl_1_bss_C8.windowId, HUWIN_ATTR_NOCANCEL);
    HuWinMesSet(lbl_1_bss_C8.windowId, MESSNUM(MESS_MGM_TOURNAMENT, 13));
    HuWinMesWait(lbl_1_bss_C8.windowId);
    choice = HuWinChoiceGet(lbl_1_bss_C8.windowId, -1);
    HuWinAttrReset(lbl_1_bss_C8.windowId, HUWIN_ATTR_NOCANCEL);
    if (choice != 0) {
        result = 0;
    } else {
        result = 1;
    }
    HuWinComKeyReset();
    HuWinExClose(lbl_1_bss_C8.windowId);
    HuPrcSleep(10);
    return result;
}

/* The window's embedded message markers trigger guide sounds and queue the host's brief
 * dialogue animation. */
void fn_1_2D1C(HUWINID windowId, u32 messageId, s16 callbackCode) {
    s16 entryIndex;

    for (entryIndex = 0; entryIndex < 18; entryIndex++) {
        if ((messageId == lbl_1_data_198[entryIndex].messageId) &&
            (callbackCode == lbl_1_data_198[entryIndex].callbackCode) &&
            (lbl_1_data_198[entryIndex].soundEffectId >= 0)) {
            HuAudFXPlayPan(lbl_1_data_198[entryIndex].soundEffectId, lbl_1_data_270);
            fn_1_5728();
        }
    }
}

/* Sets the guide dialogue's stereo pan, clamped to the middle half of the pan range. */
void fn_1_2DF8(s16 pan)
{
    if (pan < 32) pan = 32;
    if (pan > 96) pan = 96;
    lbl_1_data_270 = pan;
}
