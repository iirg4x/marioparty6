/* The development selection menu of the microphone quizzes: picks a quiz, its difficulty
 * and options, and starts the quiz module. The quiz code all five modules share is compiled
 * in from quiz_common.h. */
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
#include "game/mic.h"
#include "game/printfunc.h"
#include "REL/micquiz/quiz_common.h"

#define QUIZ_MENU_HEADING_COLOR 13
#define QUIZ_MENU_SELECTED_COLOR 14
#define QUIZ_MENU_UNSELECTED_COLOR 15
#define QUIZ_MENU_OPTION_COLOR 12
#define QUIZ_MENU_OPTION_COUNT_COLOR 3

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

int _prolog(void);
void _epilog(void);
void ObjectSetup(void);
void fn_1_244(OMOBJ *obj);
void fn_1_680(void);
s32 fn_1_6B8(void);
void fn_1_8E8(void);
void fn_1_D00(void);

/* Per menu entry: the quiz module it starts, its last option index (-1: no options) and its label;
 * then each entry's option labels. */
u16 lbl_1_data_388[] = {
    DLL_micquizohdedll, DLL_micquizishidll, DLL_micquizishidll, DLL_micquizishidll,
    DLL_micquizishidll, DLL_micquizishidll, DLL_micquizishidll, DLL_micquizishidll
};
s32 lbl_1_data_398[] = {1, 1, 1, 1, 2, -1, -1, -1};
char *lbl_1_data_424[] = {
    "Picture Quiz", "Counting Quiz", "Memory Quiz", "Comparison Quiz",
    "Variety Quiz", "Unreasonable Quiz", "Tutorial", "Introduction"
};
char *lbl_1_data_45C[] = {"1:Spin", "2:Screen effect"};
char *lbl_1_data_47C[] = {"1:Scroll", "2:Spotlight"};
char *lbl_1_data_494[] = {"1:Walk", "2:Hide"};
char *lbl_1_data_4B4[] = {"1:Characters", "2:Stars"};
char *lbl_1_data_4D8[] = {"1:Slit", "2:Blcoks", "3:Shadow"};
HUPROCESS * lbl_1_bss_20[11];
OMOBJ * lbl_1_bss_1C;
f32 micQuizPauseBgScroll;
ANIMDATA * micQuizPauseBgAnim;
HUPROCESS * micQuizMicPulseProc;
HUPROCESS * micQuizIndicatorSwingProc;
MicquizResponse * micQuizResponse;
s32 micQuizTimeoutHandled;
HUPROCESS * micQuizRingProc;

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

    OSReport("******* MQSel ObjectSetup *********\n");
    overlayHistory = omOvlHisGet(0);
    lbl_1_bss_20[0] = fn_8003A8BC(200, 8192);
    fn_80069F1C(lbl_1_bss_20[0]);
    Hu3DCameraCreate(1);
    micQuizModeWork = (MicquizModeWork *)MgModeWork;
    if (lbl_802C0248 == 0) {
        omOvlHisChg(0, DLL_micquizseldll, 1, overlayHistory->stat);
        micQuizModeWork->selectionMode = 0;
        micQuizModeWork->menuDifficultyIndex = 0;
        micQuizModeWork->mode0Option = 0;
        micQuizModeWork->modeOption[0] = 0;
        micQuizModeWork->modeOption[1] = 0;
        micQuizModeWork->modeOption[2] = 0;
        micQuizModeWork->modeOption[3] = 0;
        micQuizModeWork->randomEnabled = 0;
    }
    lbl_1_bss_1C = fn_8003AB38(lbl_1_bss_20[0], 100, 0U, 0U, -1, fn_1_244);
    lbl_1_bss_1C->work[0] = 0;
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
}

/* Advances menu selection, confirmation, and return once per object-manager update. */
void fn_1_244(OMOBJ *obj) {
    s32 playerIndex;
    s32 choice;
    s32 selectionChoice;

    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
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
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->work[0] = 3;
        return;
    case 3:
        if (WipeCheck() == 0) {
            micQuizModeWork->questionLevel = (u8)micQuizModeWork->menuDifficultyIndex;
            micQuizModeWork->questionColumn = (u8)micQuizModeWork->selectionMode;
            micQuizModeWork->currentPlayer = 0;
            micQuizModeWork->questionEvent = 8;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                micQuizModeWork->playerScore[playerIndex] = 0;
                micQuizModeWork->playerCharacter[playerIndex] =
                    (u8) lbl_80265728[playerIndex].charNo;
                micQuizModeWork->playerRank[playerIndex] = 1;
            }
            micQuizModeWork->playerCount = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_80265728[playerIndex].type != 0) {
                    micQuizModeWork->playerActive[playerIndex] = 0;
                } else {
                    micQuizModeWork->playerActive[playerIndex] = 1;
                    micQuizModeWork->playerCount++;
                }
            }
            GameMesPauseEnable(0);
            fn_8006A1C4(65547);
            if (micQuizModeWork->selectionMode == 5) {
                micQuizModeWork->presenterLayout = 1;
            } else {
                micQuizModeWork->presenterLayout = 0;
            }
            if (micQuizModeWork->selectionMode == 6) {
                micQuizModeWork->questionEvent = 3;
            }
            if (micQuizModeWork->selectionMode == 7) {
                micQuizModeWork->questionEvent = 8;
            }
            if (micQuizModeWork->selectionMode == 2) {
                if (micQuizModeWork->randomEnabled != 0) {
                    selectionChoice = frandmod(2);
                } else {
                    selectionChoice = micQuizModeWork->modeOption[1];
                }
                if (selectionChoice == 0) {
                    omOvlCallEx(DLL_micquizmyokodll, 1, 0, 0);
                    return;
                }
                omOvlCallEx(DLL_micquizishidll, 1, 0, 0);
                return;
            }
            omOvlCallEx((OMOVL)lbl_1_data_388[micQuizModeWork->selectionMode], 1, 0, 0);
            return;
        }
        break;
    case 4:
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
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
    if (lbl_802BFFE0[0] & PAD_BUTTON_UP) {
        micQuizModeWork->selectionMode--;
        if (micQuizModeWork->selectionMode < 0) {
            micQuizModeWork->selectionMode = 7;
        }
    } else if (lbl_802BFFE0[0] & PAD_BUTTON_DOWN) {
        micQuizModeWork->selectionMode++;
        if (micQuizModeWork->selectionMode >= 8) {
            micQuizModeWork->selectionMode = 0;
        }
    } else if (lbl_802BFFE0[0] & PAD_BUTTON_LEFT) {
        micQuizModeWork->menuDifficultyIndex--;
        if (micQuizModeWork->menuDifficultyIndex < 0) {
            micQuizModeWork->menuDifficultyIndex = 4;
        }
    } else if (lbl_802BFFE0[0] & PAD_BUTTON_RIGHT) {
        micQuizModeWork->menuDifficultyIndex++;
        if (micQuizModeWork->menuDifficultyIndex >= 5) {
            micQuizModeWork->menuDifficultyIndex = 0;
        }
    } else if (lbl_802C0008[0] & PAD_BUTTON_START) {
        micQuizModeWork->randomEnabled = 1;
        return 1;
    } else if (lbl_802C0008[0] & PAD_BUTTON_A) {
        micQuizModeWork->randomEnabled = 0;
        return 1;
    } else if (lbl_802C0008[0] & PAD_BUTTON_B) {
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

    maxOptionIndex = lbl_1_data_398[micQuizModeWork->selectionMode];
    if (maxOptionIndex != -1) {
        if ((lbl_802C0008[0] & PAD_TRIGGER_L) != 0) {
            if (micQuizModeWork->selectionMode == 0) {
                micQuizModeWork->mode0Option--;
                if (micQuizModeWork->mode0Option < 0) {
                    micQuizModeWork->mode0Option = maxOptionIndex;
                }
            } else if (micQuizModeWork->selectionMode == 1) {
                micQuizModeWork->modeOption[0]--;
                if (micQuizModeWork->modeOption[0] < 0) {
                    micQuizModeWork->modeOption[0] = maxOptionIndex;
                }
            } else if (micQuizModeWork->selectionMode == 2) {
                micQuizModeWork->modeOption[1]--;
                if (micQuizModeWork->modeOption[1] < 0) {
                    micQuizModeWork->modeOption[1] = maxOptionIndex;
                }
            } else if (micQuizModeWork->selectionMode == 3) {
                micQuizModeWork->modeOption[2]--;
                if (micQuizModeWork->modeOption[2] < 0) {
                    micQuizModeWork->modeOption[2] = maxOptionIndex;
                }
            } else if (micQuizModeWork->selectionMode == 4) {
                micQuizModeWork->modeOption[3]--;
                if (micQuizModeWork->modeOption[3] < 0) {
                    micQuizModeWork->modeOption[3] = maxOptionIndex;
                }
            }
        } else if ((lbl_802C0008[0] & PAD_TRIGGER_R) != 0) {
            if (micQuizModeWork->selectionMode == 0) {
                micQuizModeWork->mode0Option++;
                if (micQuizModeWork->mode0Option > maxOptionIndex) {
                    micQuizModeWork->mode0Option = 0;
                }
            } else if (micQuizModeWork->selectionMode == 1) {
                micQuizModeWork->modeOption[0]++;
                if (micQuizModeWork->modeOption[0] > maxOptionIndex) {
                    micQuizModeWork->modeOption[0] = 0;
                }
            } else if (micQuizModeWork->selectionMode == 2) {
                micQuizModeWork->modeOption[1]++;
                if (micQuizModeWork->modeOption[1] > maxOptionIndex) {
                    micQuizModeWork->modeOption[1] = 0;
                }
            } else if (micQuizModeWork->selectionMode == 3) {
                micQuizModeWork->modeOption[2]++;
                if (micQuizModeWork->modeOption[2] > maxOptionIndex) {
                    micQuizModeWork->modeOption[2] = 0;
                }
            } else if (micQuizModeWork->selectionMode == 4) {
                micQuizModeWork->modeOption[3]++;
                if (micQuizModeWork->modeOption[3] > maxOptionIndex) {
                    micQuizModeWork->modeOption[3] = 0;
                }
            }
        }
    }
}

/* Draws menu labels and stored option values when the development menu updates. */
void fn_1_D00(void) {
    fontcolor = QUIZ_MENU_HEADING_COLOR;
    print8(16, 40, 2, "MICQUIZ SELECT MENU");
    fontcolor = QUIZ_MENU_OPTION_COUNT_COLOR;
    print8(416, 40, 2, "<DIFF:%d>",
           micQuizModeWork->menuDifficultyIndex);
    if (micQuizModeWork->selectionMode == 0) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 104, 2, lbl_1_data_424[0]);
    if (micQuizModeWork->selectionMode == 1) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 136, 2, lbl_1_data_424[1]);
    if (micQuizModeWork->selectionMode == 2) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 168, 2, lbl_1_data_424[2]);
    if (micQuizModeWork->selectionMode == 3) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 200, 2, lbl_1_data_424[3]);
    if (micQuizModeWork->selectionMode == 4) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 232, 2, lbl_1_data_424[4]);
    if (micQuizModeWork->selectionMode == 5) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 264, 2, lbl_1_data_424[5]);
    if (micQuizModeWork->selectionMode == 6) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 296, 2, lbl_1_data_424[6]);
    if (micQuizModeWork->selectionMode == 7) {
        fontcolor = QUIZ_MENU_SELECTED_COLOR;
    } else {
        fontcolor = QUIZ_MENU_UNSELECTED_COLOR;
    }
    print8(16, 328, 2, lbl_1_data_424[7]);

    fontcolor = QUIZ_MENU_OPTION_COLOR;
    print8(304, 104, 2, "%s",
           lbl_1_data_45C[micQuizModeWork->mode0Option]);
    print8(304, 136, 2, "%s",
           lbl_1_data_47C[micQuizModeWork->modeOption[0]]);
    print8(304, 168, 2, "%s",
           lbl_1_data_494[micQuizModeWork->modeOption[1]]);
    print8(304, 200, 2, "%s",
           lbl_1_data_4B4[micQuizModeWork->modeOption[2]]);
    print8(304, 232, 2, "%s",
           lbl_1_data_4D8[micQuizModeWork->modeOption[3]]);
}
