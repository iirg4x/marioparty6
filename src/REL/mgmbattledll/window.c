/* Message windows, player labels, and battle mode dialogue choices. */
#include "dolphin/math.h"
#include "dolphin.h"
#include "REL/mgmbattledll/module_types.h"
#include "game/window.h"
#include "messdir_enum.h"
#include "msm_se.h"

typedef struct {
    s16 values[4];
} PadMaskTable;
typedef struct { s16 values[4]; } MessageIndices;

void fn_1_3070(s16 winId, u32 messageId, s16 characterCode);
s16 fn_1_BA94(s16 player);
s16 fn_1_BAEC(s16 player);
s16 fn_1_BB40(s16 player);
s16 fn_1_BB60(s16 player);
void fn_1_45BC(void);

u32 lbl_1_data_0[14] = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13 };

/* Called during battle setup to create the shared dialogue and player-name windows. */
void fn_1_E9C(void) {
    s16 player;
    f32 windowY;

    HuWinInit(0);
    lbl_1_bss_D8[0].width = 528.0f;
    lbl_1_bss_D8[0].height = 80.0f;
    windowY = 440.0f - lbl_1_bss_D8[0].height;
    lbl_1_bss_D8[0].windowId = HuWinExCreateFrame(
        -10000.0f,
        windowY,
        (s16)lbl_1_bss_D8[0].width,
        (s16)lbl_1_bss_D8[0].height,
        -1,
        3);
    HuWinAttrSet(lbl_1_bss_D8[0].windowId, 1 << 11);
    HuWinCallbackSet(lbl_1_bss_D8[0].windowId, fn_1_3070);

    lbl_1_bss_D8[1].width = 448.0f;
    lbl_1_bss_D8[1].height = 32.0f;
    windowY = (440.0f - lbl_1_bss_D8[0].height) - lbl_1_bss_D8[1].height;
    lbl_1_bss_D8[1].windowId = HuWinExCreate(
        -10000.0f,
        windowY,
        (s16)lbl_1_bss_D8[1].width,
        (s16)lbl_1_bss_D8[1].height,
        -1);
    HuWinMesSpeedSet(lbl_1_bss_D8[1].windowId, 0);
    HuWinBGTPLvlSet(lbl_1_bss_D8[1].windowId, 0.0f);
    HuWinAttrSet(lbl_1_bss_D8[1].windowId, 1 << 11);

    lbl_1_bss_D8[2].width = 448.0f;
    lbl_1_bss_D8[2].height = 32.0f;
    windowY = 440.0f - lbl_1_bss_D8[2].height;
    lbl_1_bss_D8[2].windowId = HuWinExCreate(
        -10000.0f,
        windowY,
        (s16)lbl_1_bss_D8[2].width,
        (s16)lbl_1_bss_D8[2].height,
        -1);
    HuWinMesSpeedSet(lbl_1_bss_D8[2].windowId, 0);
    HuWinBGTPLvlSet(lbl_1_bss_D8[2].windowId, 0.0f);
    HuWinAttrSet(lbl_1_bss_D8[2].windowId, 1 << 11);

    for (player = 0; player < 4; player++) {
        lbl_1_bss_A8[player].width = 240.0f;
        lbl_1_bss_A8[player].height = 32.0f;
        lbl_1_bss_A8[player].windowId = HuWinExCreate(
            0.0f,
            0.0f,
            (s16)lbl_1_bss_A8[player].width,
            (s16)lbl_1_bss_A8[player].height,
            -1);
        HuWinMesSpeedSet(lbl_1_bss_A8[player].windowId, 0);
        HuWinBGTPLvlSet(lbl_1_bss_A8[player].windowId, 0.0f);
    }
}

/* Called during battle teardown to kill the shared dialogue and player-name windows. */
void fn_1_12A8(void) {
    s16 i;

    HuWinExKill(lbl_1_bss_D8[0].windowId);
    HuWinExKill(lbl_1_bss_D8[1].windowId);
    HuWinExKill(lbl_1_bss_D8[2].windowId);
    i = 0;
    while (i < 4) {
        HuWinExKill(lbl_1_bss_A8[i].windowId);
        i++;
    }
    HuWinAllKill();
}

/* Called before a battle message to let only active human-player pads control the window. */
void fn_1_1334(void) {
    s16 player;
    s16 padMask = 0;
    PadMaskTable masks = {{ 1, 2, 4, 8 }};
    player = 0;
    while (player < 4) {
        if (fn_1_BB40(player) == 0) {
            padMask |= masks.values[fn_1_BB60(player)];
        }
        player++;
    }
    HuWinPadMaskSet(lbl_1_bss_D8[0].windowId, padMask);
}

/* Clears queued window inputs when the battle dialogue flow is reset. */
void fn_1_13E8(void) {
    HuWinComKeyReset();
}

/* Opens the dialogue windows selected by the current message group before it is shown. */
void fn_1_1408(s16 windowCount) {
    HuWinComKeyReset();
    switch (windowCount) {
    case 1:
    case 2:
        HuWinExOpen(lbl_1_bss_D8[0].windowId);
        break;
    case 3:
    case 4:
    case 5:
        HuWinExOpen(lbl_1_bss_D8[0].windowId);
        HuWinExOpen(lbl_1_bss_D8[1].windowId);
        break;
    case 6:
    case 7:
        HuWinExOpen(lbl_1_bss_D8[0].windowId);
        break;
    case 8:
    case 9:
    case 10:
        HuWinExOpen(lbl_1_bss_D8[0].windowId);
        HuWinExOpen(lbl_1_bss_D8[1].windowId);
        break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
        HuWinExOpen(lbl_1_bss_D8[0].windowId);
        break;
    case 100:
    case 101:
        HuWinExOpen(lbl_1_bss_D8[2].windowId);
        break;
    case 102:
        HuWinExOpen(lbl_1_bss_D8[1].windowId);
        break;
    }
}

/* Closes the dialogue windows selected by the current message group after it is shown. */
void fn_1_1528(s16 windowCount) {
    switch (windowCount) {
    case 1:
    case 2:
        HuWinExClose(lbl_1_bss_D8[0].windowId);
        break;
    case 3:
    case 4:
    case 5:
        HuWinExClose(lbl_1_bss_D8[0].windowId);
        HuWinExClose(lbl_1_bss_D8[1].windowId);
        break;
    case 6:
    case 7:
        HuWinExClose(lbl_1_bss_D8[0].windowId);
        break;
    case 8:
    case 9:
    case 10:
        HuWinExClose(lbl_1_bss_D8[0].windowId);
        HuWinExClose(lbl_1_bss_D8[1].windowId);
        break;
    case 11:
    case 12:
    case 13:
    case 14:
    case 15:
    case 16:
    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
        HuWinExClose(lbl_1_bss_D8[0].windowId);
        break;
    case 100:
    case 101:
        HuWinExClose(lbl_1_bss_D8[2].windowId);
        break;
    case 102:
        HuWinExClose(lbl_1_bss_D8[1].windowId);
        break;
    }
    HuWinComKeyReset();
}

/* Displays a battle message or choice and waits for its scripted window interaction. */
s16 fn_1_1648(s16 messageId) {
    s16 result = 0;
    MessageIndices messageIndices = {{ 0, 1, 2, 3 }};
    s16 player;
    s16 insertedCount;

    fn_1_45BC();

    switch (messageId) {
    default:
        break;
    case 1:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 0));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 2:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 1));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 3:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 2));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 4:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 3));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 5:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 4));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 6:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 5));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        result = HuWinChoiceGet(lbl_1_bss_D8[0].windowId, -1);
        return result;
    case 7:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 6));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 8:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 7));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 9:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 8));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 10:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 9));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        HuWinMesSpeedSet(lbl_1_bss_D8[0].windowId, 1);
        break;
    case 11:
        HuWinComKeyReset();
        fn_1_1334();
        if (MgGameNo == 3) {
            HuWinInsertMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 11), 0);
        } else if (MgGameNo == 5) {
            HuWinInsertMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 12), 0);
        } else {
            HuWinInsertMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 13), 0);
        }
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 10));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 12:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinInsertMesSet(lbl_1_bss_D8[0].windowId,
            MgDataTbl[MgModeWork[MgModePlayNum]].nameMes, 0);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 14));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 13:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 15));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        result = HuWinChoiceGet(lbl_1_bss_D8[0].windowId, -1);
        return result;
    case 14:
    case 15:
        HuWinComKeyReset();
        fn_1_1334();
        player = 0;
        insertedCount = 0;
        for (; player < 4; player++) {
            result = fn_1_BA94(player);
            if (result >= 0) {
                HuWinInsertMesSet(lbl_1_bss_D8[0].windowId,
                    lbl_1_data_0[result],
                    messageIndices.values[insertedCount]);
                insertedCount++;
            }
        }
        if (insertedCount == 0) {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 19));
        } else if (insertedCount == 1) {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 16));
        } else if (insertedCount == 2) {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 17));
        } else {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 18));
        }
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 16:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 20));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 17:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 21));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 171:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 23));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 172:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 22));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 18:
        HuWinComKeyReset();
        fn_1_1334();
        player = 0;
        insertedCount = 0;
        for (; player < 4; player++) {
            result = fn_1_BAEC(player);
            if (result >= 0) {
                HuWinInsertMesSet(lbl_1_bss_D8[0].windowId,
                    lbl_1_data_0[result],
                    messageIndices.values[insertedCount]);
                insertedCount++;
            }
        }
        if (insertedCount != 0) {
            if (insertedCount == 1) {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 24));
            } else if (insertedCount == 2) {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 25));
            } else {
                HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 26));
            }
        }
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 19:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinAttrSet(lbl_1_bss_D8[0].windowId, 1 << 4);
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 27));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        result = HuWinChoiceGet(lbl_1_bss_D8[0].windowId, -1);
        HuWinAttrReset(lbl_1_bss_D8[0].windowId, 1 << 4);
        return result;
    case 20:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 28));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 21:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 29));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 22:
        HuWinComKeyReset();
        fn_1_1334();
        HuWinMesSet(lbl_1_bss_D8[0].windowId, MESSNUM(MESS_MGM_KATINUKI, 30));
        HuWinMesWait(lbl_1_bss_D8[0].windowId);
        break;
    case 100:
        HuWinMesSet(lbl_1_bss_D8[2].windowId, MESSNUM(MESS_SYS_GUIDE, 3));
        HuWinMesWait(lbl_1_bss_D8[2].windowId);
        break;
    case 101:
        HuWinMesSet(lbl_1_bss_D8[2].windowId, MESSNUM(MESS_SYS_GUIDE, 0));
        HuWinMesWait(lbl_1_bss_D8[2].windowId);
        break;
    case 102:
        HuWinMesSet(lbl_1_bss_D8[1].windowId, MESSNUM(MESS_SYS_GUIDE, 2));
        HuWinMesWait(lbl_1_bss_D8[1].windowId);
        break;
    }

    return 0;
}

/* Opens a player label and inserts that player's character name. */
void fn_1_2F10(s16 windowIndex, s16 playerIndex) {
    HuWinExOpen(lbl_1_bss_A8[windowIndex].windowId);
    HuWinMesSet(lbl_1_bss_A8[windowIndex].windowId,
                lbl_1_data_0[GwPlayerConf[playerIndex].charNo]);
}

/* Positions a player label at the supplied screen coordinates. */
void fn_1_2FA0(s16 windowIndex, Point3d *position) {
    HuWinPriSet(lbl_1_bss_A8[windowIndex].windowId, 0);
    HuWinPosSet(lbl_1_bss_A8[windowIndex].windowId, position->x, position->y);
}

/* Closes all player labels when the battle dialogue sequence ends. */
void fn_1_3018(void) {
    s16 index;

    for (index = 0; index < 4; index++) {
        /* HUWINID is the leading s16 in each 12-byte target window record. */
        HuWinExClose(lbl_1_bss_A8[index].windowId);
    }
}

MgBattleWindow lbl_1_bss_D8[3];
MgBattleWindow lbl_1_bss_A8[4];

MgBattleMessageSound lbl_1_data_38[38] = {
    { MESSNUM(MESS_MGM_KATINUKI, 0), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 0), 2, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 0), 3, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 0), 4, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 1), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 2), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 3), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 4), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 5), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 6), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 7), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 8), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 9), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 10), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 10), 2, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 10), 3, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 11), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 12), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 13), 1, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 14), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 15), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 16), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 17), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 18), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 19), 1, MSM_SE_GUIDE_19 },
    { MESSNUM(MESS_MGM_KATINUKI, 20), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 21), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 22), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 23), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 24), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 25), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 26), 1, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 27), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 28), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 28), 2, -1 },
    { MESSNUM(MESS_MGM_KATINUKI, 28), 3, MSM_SE_GUIDE_18 },
    { MESSNUM(MESS_MGM_KATINUKI, 29), 1, MSM_SE_GUIDE_20 },
    { MESSNUM(MESS_MGM_KATINUKI, 30), 1, MSM_SE_GUIDE_18 }
};
s16 lbl_1_data_200 = 64;
