/* Shared work, response, choice, and resource-table types for the Mic Quiz menu. */
#ifndef REL_MICQUIZSELDLL_MODULE_TYPES_H
#define REL_MICQUIZSELDLL_MODULE_TYPES_H

#include "math.h"
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

/* Shared player, quiz-round and menu selection state. */
typedef struct MicquizWork_s {
    /* Per-player flags identifying which configured players are human. */
    s32 isHumanPlayer[4];
    /* Number of human players participating in this selection round. */
    u8 humanPlayerCount;
    /* Value passed to the result panel's score display. */
    u8 resultPanelValue;
    /* Player slot used to select the current round's response text. */
    u8 currentPlayerIndex;
    /* Player slot saved with the answer-selection response. */
    s8 responsePlayerIndex;
    /* Result of the initial answer: 0 correct, 1 incorrect, 2 timed out. */
    u8 initialAnswerResult;
    /* Result of another player's attempt: 0 correct, 1 incorrect, 2 timed out. */
    u8 retryAnswerResult;
    /* Player slot captured before accepting the initial answer. */
    s8 initialAnswerPlayer;
    /* Player slot that buzzed in for another answer attempt. */
    s8 retryAnswerPlayer;
    /* Answer outcome: zero for failure and nonzero for success. */
    s32 answerSucceeded;
    /* Remaining countdown value saved when the answer flow ends. */
    s16 remainingCountdown;
    /* Difficulty index used to select the timer's initial value. */
    u8 difficultyIndex;
    /* Unread byte between the difficulty index and selected quiz mode. */
    u8 unreadMenuByteA;
    /* Selected quiz mode. */
    u8 selectedQuizMode;
    /* Unread byte following the selected mode. */
    u8 unreadMenuByteB;
    /* Reserved work bytes not accessed by this module's menu code. */
    u8 unusedMenuData[26];
    /* Accumulated score for each of the four player slots. */
    s16 playerScores[4];
    /* Per-player setup flags used by the quiz flow. */
    u8 playerSetupFlags[4];
    /* Character number selected for each player slot. */
    u8 playerCharacterNumbers[4];
    /* Quiz submode used to choose the response and display flow. */
    u16 quizSubmode;
    /* Reserved work bytes not accessed by this module's menu code. */
    u8 unusedWorkBytes[2];
    /* Selects the alternate prompt flow for the special quiz mode. */
    s32 useAlternatePrompt;
    /* Nonzero while the answer-flow process is pending; cleared to release waiting menu
     * processes. */
    s32 answerFlowPending;
    /* Selects the pause and overlay-return path. */
    s32 pauseOverlayMode;
    /* Set when the Start-button pause and quit prompt may run. */
    s32 startPromptEnabled;
    /* Nonzero selects the alternate sprite draw layer while the pause prompt is active. */
    s32 drawMode;
    /* Nonzero enables random quiz-option selection. */
    s32 randomSelectionEnabled;
    /* Current selection-screen mode. */
    s32 selectionMode;
    /* Difficulty index selected by the development menu, from 0 through 4. */
    s32 menuDifficultyIndex;
    /* Selected option for each quiz mode. */
    s32 mode1SelectionIndex;
    s32 mode2SelectionIndex;
    s32 mode3SelectionIndex;
    s32 mode4SelectionIndex;
    s32 mode0SelectionIndex;
} MicquizWork;

typedef void (*VoidFunc)(void);
typedef void (*MCResponseCallback)(u16 *response);
typedef struct MicquizResponse {
    /* Speech or pad-answer response status returned by HuMCCurResponseGet. */
    s16 result;
    /* Response bytes are not read by this module's menu code. */
    u8 unusedResponseData[10];
    /* First recognized word index or one-based pad-choice value; -1 when unavailable. */
    s32 selectedWord;
} MicquizResponse;
typedef struct MicquizChoiceRecord {
    /* Response identifier stored with this choice record. */
    int key;
    /* Nonzero selects the alternate reply path. */
    int useAlternateReply;
    /* Unread value retained in the choice table. */
    int unusedChoiceValue;
    /* Zero marks a slot as eligible for response selection. */
    int responseSelectionDisabled;
} MicquizChoiceRecord;
typedef struct QuizMessageIds { s32 values[20]; } QuizMessageIds;
typedef struct QuizSoundIds { s32 values[17]; } QuizSoundIds;
struct _struct_lbl_1_data_21C_0x8 { f32 positionX; f32 positionY; };

extern MicquizChoiceRecord lbl_1_bss_258[8];
extern MicquizWork * lbl_1_bss_1E4;
extern s16 lbl_1_bss_86;
extern s16 lbl_1_bss_8C;
extern s16 lbl_1_bss_8E;
extern s16 lbl_1_bss_90;
extern s16 lbl_1_bss_92[4];
extern u32 lbl_1_data_12C[];
extern u32 lbl_1_data_17C[];
extern s16 lbl_1_bss_E0[4][3];
extern ANIMDATA * lbl_1_bss_F8[4][3];
extern u16 lbl_1_data_388[];
extern s32 lbl_1_data_398[];
extern char lbl_1_data_509[];
extern char lbl_1_data_51D[];
extern char lbl_1_data_527[];
extern char * lbl_1_data_424[];
extern char *lbl_1_data_45C[];
extern char *lbl_1_data_47C[];
extern char *lbl_1_data_494[];
extern char *lbl_1_data_4B4[];
extern char *lbl_1_data_4D8[];
extern s32 lbl_1_bss_220;
extern s32 lbl_1_bss_208;
extern s16 lbl_1_bss_8A;
extern s16 lbl_1_bss_B4;
extern s16 lbl_1_bss_B6;
extern s16 lbl_1_bss_178;
extern s32 lbl_1_bss_1E8;
extern f32 lbl_1_bss_1EC;
extern f32 lbl_1_bss_1F0;
extern s16 lbl_1_bss_88;
extern s32 lbl_1_data_1F4;
extern HUPROCESS * lbl_1_bss_B0;
extern s32 lbl_1_bss_148;
extern HUPROCESS * lbl_1_bss_254;
extern s32 lbl_1_bss_A8;
extern s32 lbl_1_bss_AC;
extern ANIMDATA * lbl_1_bss_150;
extern ANIMDATA * lbl_1_bss_154;
extern ANIMDATA * lbl_1_bss_158;
extern ANIMDATA * lbl_1_bss_15C[4];
extern s16 lbl_1_bss_2D8[6];
extern s32 lbl_1_data_1C[11];
extern s32 lbl_1_bss_1F4;
extern ANIMDATA * lbl_1_bss_1BC;
extern ANIMDATA * lbl_1_bss_14C;
extern s16 lbl_1_bss_16C[3];
extern s16 lbl_1_bss_172;
extern s16 lbl_1_bss_174;
extern s16 lbl_1_bss_176;
extern s16 lbl_1_bss_17A;
extern s16 lbl_1_bss_17C;
extern s16 lbl_1_bss_17E;
extern s16 lbl_1_bss_180[5];
extern s16 lbl_1_bss_18A;
extern ANIMDATA * lbl_1_bss_18C[2];
extern ANIMDATA * lbl_1_bss_194;
extern ANIMDATA * lbl_1_bss_198;
extern ANIMDATA * lbl_1_bss_19C;
extern ANIMDATA * lbl_1_bss_1A0;
extern ANIMDATA * lbl_1_bss_1A4;
extern ANIMDATA * lbl_1_bss_1A8;
extern ANIMDATA * lbl_1_bss_1AC;
extern ANIMDATA * lbl_1_bss_1B0;
extern s16 lbl_1_bss_1B4;
extern ANIMDATA * lbl_1_bss_1B8;
extern ANIMDATA * lbl_1_bss_1C0;
extern ANIMDATA * lbl_1_bss_1C4;
extern ANIMDATA * lbl_1_bss_1C8;
extern ANIMDATA * lbl_1_bss_1CC;
extern ANIMDATA * lbl_1_bss_1D0;
extern ANIMDATA * lbl_1_bss_1D4;
extern ANIMDATA * lbl_1_bss_1D8[2];
extern ANIMDATA * lbl_1_bss_1E0;
extern HUPROCESS * lbl_1_bss_4C[10];
extern s16 lbl_1_bss_9A;
extern s16 lbl_1_bss_9C;
extern ANIMDATA * lbl_1_bss_A0;
extern ANIMDATA * lbl_1_bss_A4;
extern s16 lbl_1_data_20A;
extern s16 lbl_1_data_20C;
extern s16 lbl_1_data_20E;
extern s16 lbl_1_data_210[4];
extern s16 lbl_1_data_218;
extern s16 lbl_1_data_21A;
extern struct _struct_lbl_1_data_21C_0x8 lbl_1_data_21C[4];
extern MGSCORE * lbl_1_bss_B8[2];
extern s16 lbl_1_bss_C0[4];
extern s16 lbl_1_bss_D8;
extern s16 lbl_1_bss_128[4];
extern s16 lbl_1_bss_140;
extern ANIMDATA * lbl_1_bss_14;
extern ANIMDATA * lbl_1_bss_130[4];
extern ANIMDATA * lbl_1_bss_144;
extern ANIMDATA * lbl_1_bss_DC;
extern ANIMDATA * lbl_1_bss_C8[4];
extern s16 lbl_1_data_208;
extern s16 lbl_1_bss_1F8[6];
extern HUPROCESS * lbl_1_bss_C;
extern s16 lbl_1_bss_84;
extern s32 lbl_1_bss_224;
extern s32 lbl_1_bss_228;
extern s32 lbl_1_bss_22C;
extern s32 lbl_1_data_48[5];
extern u32 lbl_1_data_A0[18];
extern s32 lbl_1_bss_230;
extern s32 lbl_1_bss_204;
extern s16 lbl_1_bss_218;
extern s32 lbl_1_bss_214;
extern s32 lbl_1_bss_234;
extern s32 lbl_1_bss_238;
extern s32 lbl_1_bss_23C;
extern s32 lbl_1_bss_240;
extern s32 lbl_1_bss_244;
extern s32 lbl_1_bss_248;
extern s32 lbl_1_bss_24C;
extern s32 lbl_1_bss_250;
extern s32 lbl_1_bss_4;
extern u32 lbl_1_data_1CC[2];
extern u32 lbl_1_data_1D4[8];
extern u32 lbl_1_data_5C[17];
extern u32 lbl_1_data_E8[17];
extern s32 lbl_1_bss_20C;
extern s32 lbl_1_bss_21C;
extern s16 lbl_1_bss_210;
extern s32 lbl_1_data_1FC[3];
extern HUPROCESS * lbl_1_bss_10;
extern s16 lbl_1_bss_74[7];
extern s16 lbl_1_bss_82;
extern OMOBJ * lbl_1_bss_1C;
extern HUPROCESS * lbl_1_bss_20[11];
extern char lbl_1_data_4E4[];
extern f32 lbl_1_bss_18;
extern MicquizResponse * lbl_1_bss_8;
extern s32 lbl_1_data_1F8;
extern HUPROCESS * lbl_1_bss_0;
extern s32 lbl_1_data_0[7];

#endif
