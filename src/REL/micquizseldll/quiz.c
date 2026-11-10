/* Player selection, quiz flow, windows, characters and result displays for the quiz menu. */
#include "REL/micquizseldll/module_types.h"
#include "game/window.h"
#include "math.h"
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/sprite.h"
#include "game/audio.h"
#include "game/gamemes.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/wipe.h"
#include "game/board/object.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "game/charman.h"
#include "game/hsfex.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/mg/actman.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "game/process.h"
#include "game/mgdata.h"
#include "game/board/player.h"
#include "game/hu3d.h"
#include "dolphin.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "string.h"
#include "humath.h"
#include "messdir_enum.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/init.h"

#define QUIZ_OBJECT_CAPACITY 200
#define QUIZ_OBJECT_HEAP_BYTES 8192
#define QUIZ_MENU_OBJECT_PRIORITY 100
#define QUIZ_MENU_HEADING_COLOR 13
#define QUIZ_MENU_SELECTED_COLOR 14
#define QUIZ_MENU_UNSELECTED_COLOR 15
#define QUIZ_MENU_OPTION_COLOR 12
#define QUIZ_MENU_OPTION_COUNT_COLOR 3

s32 HuMCInit(s16 mountResult);
void HuMCClose(void);
s16 HuMCContextCreate(char *path);
void HuMCListenerCreate(s16 context, MCResponseCallback callback, u8 flags);
void HuMCListenerKill(void);
int HuMCResponseGet(void);
void HuMCSelWinCreate(f32 x, f32 y);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s32 fontcolor;
s16 print8(s16 x, s16 y, f32 scale, char *format, ...);
s32 HuMCSelWinCheck(void);
s16 HuMCSelModeGet(void);
void HuMCSelWinContextKill(void);
void HuMCSessionClose(void);
s32 HuMCButtonGet(void);
s32 omPadErrChk(u8 playerIndex);

/* Quiz resource lists, choices and initialized sprite handles. */
s32 lbl_1_data_0[7] = {
    DATANUM(0xa40000, 122), DATANUM(0xa40000, 123),
    DATANUM(0xa40000, 124), DATANUM(0xa40000, 125),
    DATANUM(0xa40000, 126), DATANUM(0xa40000, 127),
    DATANUM(0xa40000, 128)
};
s32 lbl_1_data_1C[11] = {
    DATANUM(0x9b0000, 0), DATANUM(0x9b0000, 1),
    DATANUM(0x9b0000, 2), DATANUM(0x9b0000, 3),
    DATANUM(0x9b0000, 4), DATANUM(0x9b0000, 5),
    DATANUM(0x9b0000, 6), DATANUM(0x9b0000, 7),
    DATANUM(0x9b0000, 8), DATANUM(0x9b0000, 9),
    DATANUM(0x9b0000, 10)
};
s32 lbl_1_data_48[5] = {10, 20, 30, 40, 50};
u32 lbl_1_data_5C[17] = {
    DATANUM(0x210000, 12), DATANUM(0x210000, 13),
    DATANUM(0x210000, 14), DATANUM(0x210000, 15),
    DATANUM(0x210000, 16), DATANUM(0x210000, 17),
    0, 1, 2, 3, 4, 5, 6, 7, 9, 10,
    DATANUM(0x490000, 22)
};
u32 lbl_1_data_A0[18] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
    DATANUM(0x210000, 12), DATANUM(0x210000, 13),
    DATANUM(0x210000, 14), DATANUM(0x210000, 15),
    DATANUM(0x210000, 16), DATANUM(0x210000, 17),
    DATANUM(0x490000, 22)
};
u32 lbl_1_data_E8[17] = {
    0, 1, 2, 3, 4, 5, 6, 7, 9, 10,
    DATANUM(0x210000, 12), DATANUM(0x210000, 13),
    DATANUM(0x210000, 14), DATANUM(0x210000, 15),
    DATANUM(0x210000, 16), DATANUM(0x210000, 17),
    DATANUM(0x490000, 22)
};
u32 lbl_1_data_12C[] = {
    DATANUM(0x210000, 34), DATANUM(0x210000, 35),
    DATANUM(0x210000, 36), DATANUM(0x210000, 37),
    DATANUM(0x210000, 38), DATANUM(0x210000, 39),
    DATANUM(0x210000, 40), DATANUM(0x210000, 41),
    DATANUM(0x210000, 42), DATANUM(0x210000, 43),
    DATANUM(0x210000, 44), DATANUM(0x210000, 45),
    DATANUM(0x210000, 46), DATANUM(0x210000, 47),
    DATANUM(0x210000, 48), DATANUM(0x210000, 49),
    DATANUM(0x210000, 50), DATANUM(0x210000, 51),
    DATANUM(0x210000, 52), DATANUM(0x210000, 53)
};
u32 lbl_1_data_17C[] = {
    DATANUM(0x210000, 58), DATANUM(0x210000, 59),
    DATANUM(0x210000, 60), DATANUM(0x210000, 61),
    DATANUM(0x210000, 62), DATANUM(0x210000, 63),
    DATANUM(0x210000, 64), DATANUM(0x210000, 65),
    DATANUM(0x210000, 66), DATANUM(0x210000, 67),
    DATANUM(0x210000, 68), DATANUM(0x210000, 69),
    DATANUM(0x210000, 70), DATANUM(0x210000, 71),
    DATANUM(0x210000, 72), DATANUM(0x210000, 73),
    DATANUM(0x210000, 74), DATANUM(0x210000, 75),
    DATANUM(0x210000, 76), DATANUM(0x210000, 77)
};
u32 lbl_1_data_1CC[2] = {DATANUM(0x210000, 55), DATANUM(0x210000, 56)};
u32 lbl_1_data_1D4[8] = {
    DATANUM(0x210000, 25), DATANUM(0x210000, 26),
    DATANUM(0x210000, 27), DATANUM(0x210000, 28),
    DATANUM(0x210000, 29), DATANUM(0x210000, 30),
    DATANUM(0x210000, 31), DATANUM(0x210000, 32)
};
s32 lbl_1_data_1F4 = {-1};
s32 lbl_1_data_1F8 = {-1};
s32 lbl_1_data_1FC[3] = {-1, -1, -1};
s16 lbl_1_data_208 = {-1};
s16 lbl_1_data_20A = {-1};
s16 lbl_1_data_20C = {-1};
s16 lbl_1_data_20E = {-1};
s16 lbl_1_data_210[4] = {-1, -1, -1, -1};
s16 lbl_1_data_218 = {-1};
s16 lbl_1_data_21A = {-1};
struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[4] = {
    { 222.0f, 40.0f }, { 357.0f, 55.0f }, { 227.0f, 74.0f }, { 345.0f, 80.0f }
};
u32 lbl_1_data_23C[16] = {
    DATANUM(0x1d0000, 0), DATANUM(0x1d0000, 1),
    DATANUM(0x1d0000, 2), DATANUM(0x1d0000, 3),
    DATANUM(0x1d0000, 5), DATANUM(0x1d0000, 6),
    DATANUM(0x1d0000, 7), DATANUM(0x1d0000, 8),
    DATANUM(0x1d0000, 11), DATANUM(0x1d0000, 12),
    DATANUM(0x1d0000, 13), DATANUM(0x1d0000, 14),
    DATANUM(0x1d0000, 15), DATANUM(0x1d0000, 16),
    DATANUM(0x1d0000, 17), DATANUM(0x1d0000, 18)
};

int _prolog(void);
void _epilog(void);
void ObjectSetup(void);
void fn_1_244(OMOBJ *obj);
void fn_1_680(void);
s32 fn_1_6B8(void);
void fn_1_8E8(void);
void fn_1_D00(void);
void fn_1_122C(s32 value);
void fn_1_123C(void);
void fn_1_1250(void);
s32 fn_1_1294(void);
void fn_1_12A4(void);
void fn_1_13F8(void);
s16 fn_1_1858(s32 windowNo);
void fn_1_18D4(void);
void fn_1_1A40(void);
void fn_1_1BAC(u32 messageId, s16 messageSpeed, u8 windowIndex);
void fn_1_1C80(void);
void fn_1_1CD0(s32 mode);
void fn_1_1DF0(void);
s16 fn_1_1E30(s16 choice);
void fn_1_1E64(s32 windowNo, u32 messageId, s16 insertNo);
void fn_1_1F18(void);
void fn_1_1F44(void);
void fn_1_1F70(s32 displayOn);
void fn_1_1FC0(s32 index);
void fn_1_20AC(s32 messageType);
void fn_1_2118(s16 layer);
void fn_1_261C(HUPROCESS *parentProcess);
void fn_1_2700(void);
void fn_1_2BF4(void);
void fn_1_2CDC(void);
void fn_1_2DA0(void);
void fn_1_48A4(void);
void fn_1_507C(void);
void fn_1_5294(void);
void fn_1_53F0(void);
void fn_1_5600(void);
void fn_1_571C(void);
void fn_1_5BB0(s32 mode);
void fn_1_5EF0(void);
void fn_1_6024(void);
void fn_1_627C(void);
void fn_1_638C(void);
void fn_1_6C90(void);
void fn_1_6DE4(void);
void fn_1_6F38(void);
void fn_1_707C(void);
void fn_1_7380(void);
void fn_1_7684(void);
void fn_1_7740(void);
void fn_1_77FC(s32 action);
void fn_1_7B1C(void);
void fn_1_7B78(void);
void fn_1_7BD0(void);
void fn_1_7C28(void);
void fn_1_7F34(void);
void fn_1_81CC(void);
void fn_1_8464(void);
void fn_1_84A0(void);
void fn_1_868C(s32 selectionState);
void fn_1_8728(void);
void fn_1_87B8(void);
void fn_1_888C(s32 selectionAccepted, s32 responseMode);
void fn_1_899C(void);
void fn_1_8BAC(void);
void fn_1_8E1C(void);
void fn_1_8F74(void);
void fn_1_90CC(void);
void fn_1_90D0(void);
u8 fn_1_90D4(void);
u8 fn_1_90E8(void);
void fn_1_90FC(s8 responsePlayerIndex, u16 remainingCountdown, s32 answerSucceeded);
s32 fn_1_9134(void);
s32 fn_1_9148(s32 playerIndex);
f32 fn_1_9160(void);
void fn_1_92D4(void);
void fn_1_92F8(void);
s32 fn_1_931C(void);
s32 fn_1_9330(void);
void fn_1_9344(void);
void fn_1_935C(void);
void fn_1_9374(void);
void fn_1_9620(u32 message);
void fn_1_9860(u32 message);
void fn_1_9AA0(void);
void fn_1_9CCC(void);
void fn_1_9EC8(void);
void fn_1_A0C4(s32 playerIndex);
void fn_1_A368(void);
void fn_1_A564(u32 message);
void fn_1_A7D8(void);
void fn_1_A9D4(void);
void fn_1_ABD0(void);
void fn_1_ADCC(void);
void fn_1_AFDC(void);
void fn_1_B1D8(s32 playerIndex);
void fn_1_B434(char *path, s32 mode);
void fn_1_B4A0(s32 contextKind);
void fn_1_B60C(void);
void fn_1_B638(void);
void fn_1_B664(s32 quizMode);
void fn_1_CD50(s32 mode);
s32 fn_1_DF28(s32 playerIndex, s32 responseMode);
s32 fn_1_E370(s32 firstResult, s32 secondResult);
void fn_1_E388(void);
void fn_1_E4A8(void);
void fn_1_E8A4(s32 displayState);
void fn_1_EAB8(void);
void fn_1_EB18(void);
void fn_1_EB68(void);
void fn_1_EBC8(void);
void fn_1_EC18(s32 select, s32 padNo);
void fn_1_ED58(s32 padNo, s32 numberMode);
void fn_1_EE48(void);
void fn_1_F40C(void);
void fn_1_F820(s32 animationState);
void fn_1_F984(void);
void fn_1_FBFC(void);
void fn_1_FD0C(void);
void fn_1_FE04(void);
void fn_1_100E0(s32 mode, s32 retryReply);
void fn_1_10688(u32 message);
void fn_1_108C8(s32 messageIndex);
void fn_1_10B40(s32 messageIndex);
void fn_1_10DB8(void);
void fn_1_10DBC(void);
void fn_1_10DFC(HUPROCESS* parent);
void fn_1_10E4C(void);
void fn_1_10E84(void);
void fn_1_10FD0(void);
s32 fn_1_11198(void);
void fn_1_11278(s32 value);
s32 fn_1_11308(s32 minimum, s32 maximum);
s32 fn_1_11344(s32 chancePercent);
void fn_1_113A4(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_117FC(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_11C68(s32 primaryMessageIndex, s32 secondaryMessageIndex);
void fn_1_11FB0(s32 messageIndex);
void fn_1_12120(void);
s32 fn_1_12238(void);
void fn_1_1224C(s32 success, s32 choice);
void fn_1_12724(void);
int fn_1_12800(int key);
int fn_1_12850(int key);
void fn_1_128BC(void);
void fn_1_129C8(s32 motionIndex, f32 blendFrames, u32 attributes);
void fn_1_12A6C(s32 promptReady);
void fn_1_12A80(int base);
void fn_1_12B68(s32 baseIndex);
void fn_1_12C7C(void);
void fn_1_12D78(void);
void fn_1_12E48(HUWINID windowId, u32 messageId, s16 soundIndex);

/* Runs overlay constructors and starts the selection menu during overlay load. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* Runs overlay destructors when the selection menu is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Initializes the selection overlay and its menu object; called by the overlay prolog. */
void ObjectSetup(void) {
    OMOVLHIS *overlayHistory;

    OSReport(lbl_1_data_4E4);
    overlayHistory = omOvlHisGet(0);
    lbl_1_bss_20[0] = fn_8003A8BC(QUIZ_OBJECT_CAPACITY, QUIZ_OBJECT_HEAP_BYTES);
    fn_80069F1C(lbl_1_bss_20[0]);
    Hu3DCameraCreate(1);
    lbl_1_bss_1E4 = (MicquizWork *)MgModeWork;
    if (lbl_802C0248 == 0) {
        omOvlHisChg(0, 106, 1, overlayHistory->stat);
        lbl_1_bss_1E4->selectionMode = 0;
        lbl_1_bss_1E4->menuDifficultyIndex = 0;
        lbl_1_bss_1E4->mode0SelectionIndex = 0;
        lbl_1_bss_1E4->mode1SelectionIndex = 0;
        lbl_1_bss_1E4->mode2SelectionIndex = 0;
        lbl_1_bss_1E4->mode3SelectionIndex = 0;
        lbl_1_bss_1E4->mode4SelectionIndex = 0;
        lbl_1_bss_1E4->randomSelectionEnabled = 0;
    }
    lbl_1_bss_1C = fn_8003AB38(lbl_1_bss_20[0], QUIZ_MENU_OBJECT_PRIORITY, 0U, 0U, -1, fn_1_244);
    lbl_1_bss_1C->work[0] = 0;
    WipeCreate(1, 0, 20);
}

/* Advances menu selection, confirmation, and return once per object-manager update. */
void fn_1_244(OMOBJ *obj) {
    s32 playerIndex;
    s32 choice;
    s32 selectionChoice;

    if (omSysExitReq != 0) {
        WipeCreate(2, 0, 60);
        obj->objFunc = (OMOBJ_FUNC)fn_1_680;
    }
    switch (obj->work[0]) {
    case 0:
        if (WipeCheck() == 0) {
            obj->work[0] = 1;
            break;
        }
        break;
    case 1:
        choice = fn_1_6B8();
        if (choice == -1) {
            obj->work[0] = 4;
            return;
        }
        if (choice == 1) {
            obj->work[0] = 2;
            return;
        }
        break;
    case 2:
        WipeCreate(2, 0, 60);
        obj->work[0] = 3;
        return;
    case 3:
        if (WipeCheck() == 0) {
            lbl_1_bss_1E4->difficultyIndex = (u8)lbl_1_bss_1E4->menuDifficultyIndex;
            lbl_1_bss_1E4->selectedQuizMode = (u8)lbl_1_bss_1E4->selectionMode;
            lbl_1_bss_1E4->currentPlayerIndex = 0;
            lbl_1_bss_1E4->quizSubmode = 8;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                lbl_1_bss_1E4->playerScores[playerIndex] = 0;
                lbl_1_bss_1E4->playerCharacterNumbers[playerIndex] =
                    (u8) lbl_80265728[playerIndex].charNo;
                lbl_1_bss_1E4->playerSetupFlags[playerIndex] = 1;
            }
            lbl_1_bss_1E4->humanPlayerCount = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_80265728[playerIndex].type != 0) {
                    lbl_1_bss_1E4->isHumanPlayer[playerIndex] = 0;
                } else {
                    lbl_1_bss_1E4->isHumanPlayer[playerIndex] = 1;
                    lbl_1_bss_1E4->humanPlayerCount++;
                }
            }
            GameMesPauseEnable(0);
            fn_8006A1C4(65547);
            if (lbl_1_bss_1E4->selectionMode == 5) {
                lbl_1_bss_1E4->useAlternatePrompt = 1;
            } else {
                lbl_1_bss_1E4->useAlternatePrompt = 0;
            }
            if (lbl_1_bss_1E4->selectionMode == 6) {
                lbl_1_bss_1E4->quizSubmode = 3;
            }
            if (lbl_1_bss_1E4->selectionMode == 7) {
                lbl_1_bss_1E4->quizSubmode = 8;
            }
            if (lbl_1_bss_1E4->selectionMode == 2) {
                if (lbl_1_bss_1E4->randomSelectionEnabled != 0) {
                    selectionChoice = frandmod(2);
                } else {
                    selectionChoice = lbl_1_bss_1E4->mode2SelectionIndex;
                }
                if (selectionChoice == 0) {
                    omOvlCallEx(104, 1, 0, 0);
                    return;
                }
                omOvlCallEx(103, 1, 0, 0);
                return;
            }
            omOvlCallEx((OMOVL)lbl_1_data_388[lbl_1_bss_1E4->selectionMode], 1, 0, 0);
            return;
        }
        break;
    case 4:
        WipeCreate(2, 0, 60);
        obj->work[0] = 5;
        return;
    case 5:
        if (WipeCheck() == 0) {
            fn_8003A5E4(1, 1);
        }
        break;
    }
}

/* Returns to the preceding overlay after the exit wipe finishes; installed by fn_1_244. */
void fn_1_680(void) {
    if (WipeCheck() == 0) {
        fn_8003A5E4(1, 1);
    }
}

/* Reads controller 1's menu input and returns confirm or cancel state to fn_1_244. */
s32 fn_1_6B8(void) {
    if (lbl_802BFFE0[0] & 8) {
        lbl_1_bss_1E4->selectionMode--;
        if (lbl_1_bss_1E4->selectionMode < 0) {
            lbl_1_bss_1E4->selectionMode = 7;
        }
    } else if (lbl_802BFFE0[0] & 4) {
        lbl_1_bss_1E4->selectionMode++;
        if (lbl_1_bss_1E4->selectionMode >= 8) {
            lbl_1_bss_1E4->selectionMode = 0;
        }
    } else if (lbl_802BFFE0[0] & 1) {
        lbl_1_bss_1E4->menuDifficultyIndex--;
        if (lbl_1_bss_1E4->menuDifficultyIndex < 0) {
            lbl_1_bss_1E4->menuDifficultyIndex = 4;
        }
    } else if (lbl_802BFFE0[0] & 2) {
        lbl_1_bss_1E4->menuDifficultyIndex++;
        if (lbl_1_bss_1E4->menuDifficultyIndex >= 5) {
            lbl_1_bss_1E4->menuDifficultyIndex = 0;
        }
    } else if (lbl_802C0008[0] & 0x1000) {
        lbl_1_bss_1E4->randomSelectionEnabled = 1;
        return 1;
    } else if (lbl_802C0008[0] & 0x100) {
        lbl_1_bss_1E4->randomSelectionEnabled = 0;
        return 1;
    } else if (lbl_802C0008[0] & 0x200) {
        return -1;
    } else {
        fn_1_8E8();
    }
    fn_1_D00();
    return 0;
}

/* Moves the active quiz-option choice with the L/R shoulder buttons during fn_1_6B8. */
void fn_1_8E8(void) {
    s32 maxOptionIndex;

    maxOptionIndex = lbl_1_data_398[lbl_1_bss_1E4->selectionMode];
    if (maxOptionIndex != -1) {
        if ((lbl_802C0008[0] & 0x40) != 0) {
            if (lbl_1_bss_1E4->selectionMode == 0) {
                lbl_1_bss_1E4->mode0SelectionIndex--;
                if (lbl_1_bss_1E4->mode0SelectionIndex < 0) {
                    lbl_1_bss_1E4->mode0SelectionIndex = maxOptionIndex;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 1) {
                lbl_1_bss_1E4->mode1SelectionIndex--;
                if (lbl_1_bss_1E4->mode1SelectionIndex < 0) {
                    lbl_1_bss_1E4->mode1SelectionIndex = maxOptionIndex;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 2) {
                lbl_1_bss_1E4->mode2SelectionIndex--;
                if (lbl_1_bss_1E4->mode2SelectionIndex < 0) {
                    lbl_1_bss_1E4->mode2SelectionIndex = maxOptionIndex;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 3) {
                lbl_1_bss_1E4->mode3SelectionIndex--;
                if (lbl_1_bss_1E4->mode3SelectionIndex < 0) {
                    lbl_1_bss_1E4->mode3SelectionIndex = maxOptionIndex;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 4) {
                lbl_1_bss_1E4->mode4SelectionIndex--;
                if (lbl_1_bss_1E4->mode4SelectionIndex < 0) {
                    lbl_1_bss_1E4->mode4SelectionIndex = maxOptionIndex;
                }
            }
        } else if ((lbl_802C0008[0] & 0x20) != 0) {
            if (lbl_1_bss_1E4->selectionMode == 0) {
                lbl_1_bss_1E4->mode0SelectionIndex++;
                if (lbl_1_bss_1E4->mode0SelectionIndex > maxOptionIndex) {
                    lbl_1_bss_1E4->mode0SelectionIndex = 0;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 1) {
                lbl_1_bss_1E4->mode1SelectionIndex++;
                if (lbl_1_bss_1E4->mode1SelectionIndex > maxOptionIndex) {
                    lbl_1_bss_1E4->mode1SelectionIndex = 0;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 2) {
                lbl_1_bss_1E4->mode2SelectionIndex++;
                if (lbl_1_bss_1E4->mode2SelectionIndex > maxOptionIndex) {
                    lbl_1_bss_1E4->mode2SelectionIndex = 0;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 3) {
                lbl_1_bss_1E4->mode3SelectionIndex++;
                if (lbl_1_bss_1E4->mode3SelectionIndex > maxOptionIndex) {
                    lbl_1_bss_1E4->mode3SelectionIndex = 0;
                }
            } else if (lbl_1_bss_1E4->selectionMode == 4) {
                lbl_1_bss_1E4->mode4SelectionIndex++;
                if (lbl_1_bss_1E4->mode4SelectionIndex > maxOptionIndex) {
                    lbl_1_bss_1E4->mode4SelectionIndex = 0;
                }
            }
        }
    }
}

#include "REL/micquizseldll/window_functions.h"

/* Draws menu labels and stored option values when the development menu updates. */
void fn_1_D00(void) {
    fontcolor = QUIZ_MENU_HEADING_COLOR;
    print8(16, 40, 2, lbl_1_data_509);
    fontcolor = QUIZ_MENU_OPTION_COUNT_COLOR;
    print8(416, 40, 2, lbl_1_data_51D,
           lbl_1_bss_1E4->menuDifficultyIndex);
    if (lbl_1_bss_1E4->selectionMode == 0) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 104, 2, lbl_1_data_424[0]);
    if (lbl_1_bss_1E4->selectionMode == 1) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 136, 2, lbl_1_data_424[1]);
    if (lbl_1_bss_1E4->selectionMode == 2) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 168, 2, lbl_1_data_424[2]);
    if (lbl_1_bss_1E4->selectionMode == 3) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 200, 2, lbl_1_data_424[3]);
    if (lbl_1_bss_1E4->selectionMode == 4) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 232, 2, lbl_1_data_424[4]);
    if (lbl_1_bss_1E4->selectionMode == 5) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 264, 2, lbl_1_data_424[5]);
    if (lbl_1_bss_1E4->selectionMode == 6) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 296, 2, lbl_1_data_424[6]);
    if (lbl_1_bss_1E4->selectionMode == 7) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 328, 2, lbl_1_data_424[7]);

    fontcolor = QUIZ_MENU_OPTION_COLOR;
    print8(304, 104, 2, lbl_1_data_527,
           lbl_1_data_45C[lbl_1_bss_1E4->mode0SelectionIndex]);
    print8(304, 136, 2, lbl_1_data_527,
           lbl_1_data_47C[lbl_1_bss_1E4->mode1SelectionIndex]);
    print8(304, 168, 2, lbl_1_data_527,
           lbl_1_data_494[lbl_1_bss_1E4->mode2SelectionIndex]);
    print8(304, 200, 2, lbl_1_data_527,
           lbl_1_data_4B4[lbl_1_bss_1E4->mode3SelectionIndex]);
    print8(304, 232, 2, lbl_1_data_527,
           lbl_1_data_4D8[lbl_1_bss_1E4->mode4SelectionIndex]);
}

#include "REL/micquizseldll/quiz_functions.h"
