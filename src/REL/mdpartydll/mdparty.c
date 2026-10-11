/* Party Mode menus, player setup, board flow, and stage selection behavior. */
#include <string.h>

#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_stream.h"

#include "dolphin/os.h"

#include "REL/mdpartyDll.h"

#include "game/armem.h"
#include "game/board/main.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/printfunc.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

#define sind(x) sin((M_PI * (x)) / 180.0)
#define cosd(x) cos((M_PI * (x)) / 180.0)

#define MDPARTY_MSG_SETUP_FIRST ((int)MESSNUM(MESS_PARTY_SETTING, 0))
#define MDPARTY_MSG_SETUP_SECOND ((int)MESSNUM(MESS_PARTY_SETTING, 1))
#define MDPARTY_MSG_EXIT_CONFIRMATION ((int)MESSNUM(MESS_PARTY_SETTING, 8))
#define MDPARTY_MSG_PLAYER_COUNT_BASE ((int)MESSNUM(MESS_PARTY_SETTING, 9))
#define MDPARTY_MSG_HUMAN_CHARACTER_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 14))
#define MDPARTY_MSG_CPU_CHARACTER_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 15))
#define MDPARTY_MSG_CPU_SLOT_MESSAGE_BASE ((int)MESSNUM(MESS_PARTY_SETTING, 19))
#define MDPARTY_MSG_CPU_DIFFICULTY_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 20))
#define MDPARTY_MSG_TEAM_LAYOUT_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 21))
#define MDPARTY_MSG_NEW_GAME_SELECTED ((int)MESSNUM(MESS_PARTY_SETTING, 37))
#define MDPARTY_MSG_SAVED_GAME_SETUP_OPTION ((int)MESSNUM(MESS_PARTY_SETTING, 38))
#define MDPARTY_MSG_SAVED_GAME_CONTINUE_OPTION ((int)MESSNUM(MESS_PARTY_SETTING, 2))
#define MDPARTY_MSG_OPENING_BOARD_CHOICE ((int)MESSNUM(MESS_PARTY_SETTING, 4))
#define MDPARTY_MSG_NEXT_PARTY_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 5))
#define MDPARTY_MSG_BOARD_NAME_BASE ((int)MESSNUM(MESS_MAP_NAME, 9))
#define MDPARTY_MSG_LOCKED_BOARD_MESSAGE ((int)MESSNUM(MESS_MGM_MAIN_MENU, 23))
#define MDPARTY_MSG_GAME_MODE_OPTION_BASE ((int)MESSNUM(MESS_PARTY_SETTING, 6))
#define MDPARTY_MSG_TEAM_PAIRING_SUMMARY ((int)MESSNUM(MESS_PARTY_SETTING, 22))
#define MDPARTY_MSG_OPTION_GROUP_TWO_BASE ((int)MESSNUM(MESS_PARTY_SETTING, 23))
#define MDPARTY_MSG_OPTION_GROUP_THREE_BASE ((int)MESSNUM(MESS_MGPACK_NAME, 0))
#define MDPARTY_MSG_OPTION_GROUP_FOUR_BASE ((int)MESSNUM(MESS_PARTY_SETTING, 32))
#define MDPARTY_MSG_INDIVIDUAL_HANDICAP_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 34))
#define MDPARTY_MSG_NEW_GAME_CHOICE ((int)MESSNUM(MESS_PARTY_SETTING, 35))
#define MDPARTY_MSG_RESTART_SETUP_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 36))
#define MDPARTY_MSG_TEAM_HANDICAP_PROMPT ((int)MESSNUM(MESS_PARTY_SETTING, 39))
#define MDPARTY_SE_SETUP_CHARACTER_FIRST MSM_SE_MENU_16
#define MDPARTY_SE_SETUP_CHARACTER_SECOND MSM_SE_MENU_17
#define MDPARTY_STREAM_MODE MSM_STREAM_MODE
#define MDPARTY_MSG_CHARACTER_PAIR_BASE ((int)MESSNUM(MESS_TAG_NAME, 0))
#define MDPARTY_MSG_NO_CHARACTER_PAIR ((int)MESSNUM(MESS_TAG_NAME, 55))
#define MDPARTY_DATA_CHARACTER_SELECT_MODEL DATANUM(DATA_mdparty, 36)
#define MDPARTY_MSG_SELECT_PROMPT ((int)MESSNUM(MESS_SYS_GUIDE, 10))
#define MDPARTY_GUIDE_PAN_FIRST_GROUP 48
#define MDPARTY_GUIDE_PAN_SECOND_GROUP 80
#define MDPARTY_BANK_FLAG_SIXTH_BOARD 51

typedef void (*VoidFunc)(void);

typedef struct MdCameraWork_s MDCAMERA_WORK;
typedef void (*MDCAMERA_CALLBACK)(OMOBJ *obj, MDCAMERA_WORK *camera);

struct MdCameraWork_s {
    OMOBJ *obj; /* Object that owns the camera update callback. */
    HuVecF center; /* Current camera focus point in world coordinates. */
    HuVecF targetCenter; /* Camera focus position approached by center. */
    HuVecF rot; /* Current camera rotation in degrees. */
    HuVecF targetRot; /* Camera rotation approached by rot, in degrees. */
    float zoom; /* Current camera zoom value. */
    float targetZoom; /* Camera zoom approached by zoom. */
    MDCAMERA_CALLBACK callback; /* Per-frame camera behavior selected by the menu flow. */
    float transitionFrame; /* Elapsed frames for the active camera transition. */
    u8 unk_44[12]; /* Cleared during camera setup, never read. */
};

typedef struct MdSprInfo_s {
    s16 groupNo; /* Sprite group used to create this menu sprite. */
    s16 memberNo; /* Member within the sprite group. */
    s16 animNo; /* Initial animation index for the sprite. */
    s16 priority; /* Draw priority assigned to the sprite. */
    s16 bank; /* Animation bank used by the sprite. */
    HuVec2f pos; /* Screen position in sprite coordinates. */
    HuVec2f scale; /* Horizontal and vertical sprite scale. */
    float zRot; /* Sprite rotation around the screen axis, in degrees. */
} MDSPR_INFO;

typedef struct Lbl1DataE12Entry {
    s16 neighborCharacter[8]; /* Character choices in each stick direction; -1 means no neighbor. */
    s16 selectionState; /* -1 when available; 1 while a player is selecting or has selected this
                         * character. */
} LBL_1_DATA_E12_ENTRY;

typedef struct Lbl1Bss288Entry {
    HU3D_MODELID modelId; /* Menu display model for a character, board, or option. */
    HU3D_ANIMID animId[4]; /* Texture animation slots attached to the model. */
    s16 unk_A; /* Storage not read by this module. */
    HuVecF pos; /* Position captured at transition start, in world units. */
    HuVecF rot; /* Rotation captured at transition start, in degrees. */
    HuVecF scale; /* Scale factors captured at transition start. */
    s16 highlightSpinFrame; /* Highlight delay counter; clamped to 35 once the board model starts
                             * spinning. */
    u8 unk_32[6]; /* Cleared when the model set is created, never read. */
} LBL_1_BSS_288_ENTRY;

typedef struct Lbl1Bss1C {
    OMOBJ *obj; /* Object holding the four player marker models. */
    u8 unk_4[4]; /* Storage not read by this module. */
} LBL_1_BSS_1C;

typedef struct Lbl1Bss228Entry {
    s16 state; /* Storage not read by this module. */
    s16 pad; /* Storage not read by this module. */
    HuVecF pos; /* Captured model attachment position, in world units. */
    HuVecF controlPos; /* Storage not read by this module. */
    HuVecF endPos; /* Storage not read by this module. */
    float time; /* Elapsed frames of the attachment-position transition. */
    float duration; /* Storage not read by this module. */
} LBL_1_BSS_228_ENTRY;

typedef struct Lbl1Bss1C8Entry {
    s16 state; /* Phase or completion state of the active model transition. */
    s16 pad; /* Storage not read by this module. */
    HuVecF pos; /* Starting position of the current transition, in world units. */
    HuVecF controlPos; /* Middle control point for the model's quadratic path, in world units. */
    HuVecF endPos; /* Destination of the model's quadratic path, in world units. */
    float time; /* Elapsed frames of the current transition phase. */
    float duration; /* Length of the current transition phase, in frames. */
} LBL_1_BSS_1C8_ENTRY;

typedef struct Lbl1Bss9BCEntry {
    s16 characterChoiceState; /* 0 while choosing, 1 after choosing, and 2 when locked in. */
    s16 unusedSetupValue; /* Zeroed during setup and not read by this module. */
    s16 isComputer; /* 0 for a human-controlled slot and 1 for a computer slot. */
    s16 computerDifficulty; /* Difficulty passed to the game for a computer player. */
    s16 chrSel; /* Character selected for this player slot. */
    s16 padNo; /* Controller port used to control this player slot. */
    s16 handicap; /* Individual handicap; in Tag Mode, slots 0 and 1 store the two team
                   * handicaps. */
} LBL_1_BSS_9BC_ENTRY;

typedef struct Lbl1Bss9F4 {
    s16 tagMode; /* Whether the game uses tag teams. */
    s16 playerCount; /* Number of human players selected, from one to four. */
    s16 boardNo; /* Selected board index. */
    s16 turnCountChoice; /* Turn-count option index; the game uses 10 + index * 5 turns. */
    s16 minigamePack; /* Selected minigame pack. */
    s16 bonusStarChoice; /* Party Mode menu option index; its 0/1 value is inverted from the board
                          * bonus-star flag. */
    s16 selectedHandicapSlot; /* Individual player or team selected for handicap adjustment. */
    s16 teamLayout; /* Index into the player order table for the selected team layout. */
    s32 teamMessageNo[2]; /* Message IDs for the two team character pairings. */
    s16 selectedRuleIndex; /* Selected rule in the new game settings menu. */
    s16 unk_1A; /* Additional setup storage not referenced by this flow. */
} LBL_1_BSS_9F4;

#include "REL/mdpartyDll_globals.h"

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern int HuAudFXPlay(int seId);
extern int HuAudFXPlayPan(int seId, s16 pan);
extern void HuAudFXStop(int seNo);
extern int HuAudSStreamPlay(s16 streamId);

void fn_1_0(HUWINID winId, u32 messageId, s16 cueIndex);
void fn_1_1B4(void);
void fn_1_2B0(void);
void fn_1_400(void);
void fn_1_64C(void);
void fn_1_718(void);
void fn_1_E30(void);
void fn_1_353C(void);
void fn_1_6284(OMOBJ *obj);
void fn_1_72C4(OMOBJ *obj);
void fn_1_109C(s16 nextFlow);
void fn_1_1248(void);
void fn_1_12F8(void);
void fn_1_2D1D4(void);
void fn_1_2DD38(void);
void fn_1_2DFD4(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_2E338(OMOBJ *obj, MDCAMERA_WORK *camera);
void fn_1_2E560(OMOBJ *obj, MDCAMERA_WORK *camera);
s32 fn_1_2E5D4(void);
s16 fn_1_2F22C(void);
s16 fn_1_30B4C(void);
s16 fn_1_31AC0(s16 unused);
s16 fn_1_32904(void);
s16 fn_1_329A4(s16 returningFromPreviousStep);
s16 fn_1_335F0(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);
s16 fn_1_33724(s16 unused);
s16 fn_1_34FC4(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);
s16 fn_1_350F8(s16 unused);
s16 fn_1_36518(s16 returningFromPreviousStep);
void fn_1_37260(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot);
s16 fn_1_3736C(s16 returningFromPreviousStep);
s16 fn_1_38A3C(s16 returningToSelection);
s16 fn_1_39C60(s16 returningFromPreviousStep);
s16 fn_1_3AD84(s16 returningFromPreviousStep);
s16 fn_1_3CFE4(s16 returningFromPreviousStep);
s16 fn_1_3EAC8(s16 unused);
void ObjectSetup(void);
void fn_1_32D0(OMOBJ *obj);
void fn_1_467C(void);
void fn_1_4A84(OMOBJ *obj);
void fn_1_4EDC(OMOBJ *obj);
void fn_1_50EC(OMOBJ *obj);
void fn_1_5208(OMOBJ *obj);
void fn_1_5324(OMOBJ *obj);
void fn_1_5A18(OMOBJ *obj);
void fn_1_6324(OMOBJ *obj);
void fn_1_6A20(OMOBJ *obj);
void fn_1_737C(OMOBJ *obj);
void fn_1_605C(OMOBJ *obj);
void fn_1_7064(OMOBJ *obj);
void fn_1_9658(s16 winNo, u32 messNum);
void fn_1_A8D8(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);
void fn_1_A970(s16 memberNo, HuVecF *pos, float xOffset, float yOffset);
void fn_1_AA94(s16 memberNo);
void fn_1_AAF0(s16 memberNo);
void fn_1_AD14(s16 modelNo, HuVecF *pos);
void fn_1_7ED4(OMOBJ *obj);
void fn_1_861C(OMOBJ *obj);
void fn_1_9218(OMOBJ *obj);
void fn_1_9F50(OMOBJ *obj);
void fn_1_A5D8(OMOBJ *obj);
void fn_1_B008(OMOBJ *obj);
void fn_1_B4C8(OMOBJ *obj);
s16 fn_1_B748(s16 currentCharacterNo, s16 directionNo,
    s16 alternateDirectionNo, s16 fallbackDirectionNo,
    LBL_1_DATA_E12_ENTRY *characterChoices, s16 characterCount);
s16 fn_1_BBD8(
    s16 padNo, s16 *current, LBL_1_DATA_E12_ENTRY *entries, s16 count);
void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_C698(s16 modelNo, s16 time, s16 duration);
void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);
void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);
void fn_1_D748(void);
void fn_1_10518(OMOBJ *obj);
void fn_1_114C4(OMOBJ *obj);
void fn_1_114F4(OMOBJ *obj);
void fn_1_1158C(OMOBJ *obj);
void fn_1_11EAC(OMOBJ *obj);
void fn_1_12364(OMOBJ *obj);
void fn_1_12FD8(OMOBJ *obj);
void fn_1_12B18(OMOBJ *obj);
void fn_1_12D7C(OMOBJ *obj);
void fn_1_12EC8(OMOBJ *obj);
void fn_1_13EC0(OMOBJ *obj);
void fn_1_14344(OMOBJ *obj);
void fn_1_14510(OMOBJ *obj);
void fn_1_14CC0(OMOBJ *obj);
void fn_1_14F64(OMOBJ *obj);
void fn_1_150D0(OMOBJ *obj);
void fn_1_156B4(OMOBJ *obj);
void fn_1_160A0(OMOBJ *obj);
void fn_1_16A8C(OMOBJ *obj);
void fn_1_16E84(OMOBJ *obj);
void fn_1_1733C(s16 boardIndex);
void fn_1_17730(OMOBJ *obj);
void fn_1_17E10(OMOBJ *obj);
void fn_1_1807C(OMOBJ *obj);
void fn_1_18384(OMOBJ *obj);
void fn_1_18AA4(OMOBJ *obj);
void fn_1_1948C(OMOBJ *obj);
void fn_1_19860(OMOBJ *obj);
void fn_1_19D24(OMOBJ *obj);
void fn_1_1A20C(OMOBJ *obj);
void fn_1_1A8B0(OMOBJ *obj);
void fn_1_1AE60(OMOBJ *obj);
void fn_1_1B400(OMOBJ *obj);
void fn_1_1B9E8(OMOBJ *obj);
void fn_1_1D758(OMOBJ *obj);
void fn_1_1E7F8(OMOBJ *obj);
void fn_1_1F8B8(OMOBJ *obj);
void fn_1_21628(OMOBJ *obj);
void fn_1_219C8(OMOBJ *obj);
void fn_1_21C04(OMOBJ *obj);
void fn_1_21C9C(OMOBJ *obj);
void fn_1_22010(OMOBJ *obj);
void fn_1_221F4(OMOBJ *obj);
void fn_1_227E4(OMOBJ *obj);
void fn_1_228E8(OMOBJ *obj);
void fn_1_22EB8(OMOBJ *obj);
void fn_1_2318C(OMOBJ *obj);
void fn_1_23224(OMOBJ *obj);
void fn_1_232A0(OMOBJ *obj);
void fn_1_239DC(OMOBJ *obj);
void fn_1_23A00(OMOBJ *obj);
void fn_1_23AC0(OMOBJ *obj);
void fn_1_24090(OMOBJ *obj);
void fn_1_2457C(OMOBJ *obj);
void fn_1_26328(OMOBJ *obj);
void fn_1_2851C(OMOBJ *obj);
void fn_1_29598(OMOBJ *obj);
void fn_1_2A8C4(OMOBJ *obj);
void fn_1_2BBD8(OMOBJ *obj);
void fn_1_2C874(void);
void fn_1_2CE00(void);
s16 fn_1_2E638(void);
s16 fn_1_2EBEC(void);
s16 fn_1_2EF24(void);
s16 fn_1_31508(void);
s16 fn_1_38314(void);
s16 fn_1_3BEA8(void);
void fn_1_3CA70(void);
/* Party setup windows call this hook when message text requests a guide cue; repeating the same
 * message ID suppresses further cues. */
void fn_1_0(HUWINID winId, u32 messageId, s16 cueIndex)
{
    s32 pannedMessages[3] = { MDPARTY_MSG_SETUP_FIRST, MDPARTY_MSG_NEW_GAME_SELECTED, -1 };
    s32 choiceSounds[16] = {
        MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27, MSM_SE_GUIDE_28,
        MSM_SE_GUIDE_29, MSM_SE_GUIDE_30, MSM_SE_GUIDE_31, -1,
        MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19, MSM_SE_GUIDE_20,
        MSM_SE_GUIDE_21, MSM_SE_GUIDE_22, MSM_SE_GUIDE_23, -1,
    };
    s16 messageIndex;

    /* Message text supplies a one-based cue index into the two groups of guide sounds. */
    cueIndex--;
    OSReport(lbl_1_data_BCC, cueIndex);
    if (lbl_1_data_BC8 != messageId) {
        lbl_1_data_BC8 = messageId;
        for (messageIndex = 0;; messageIndex++) {
            if (pannedMessages[messageIndex] == -1) {
                HuAudFXPlay(choiceSounds[cueIndex]);
                break;
            }
            if (messageId == pannedMessages[messageIndex]) {
                if (cueIndex >= 8) {
                    HuAudFXPlayPan(choiceSounds[cueIndex], MDPARTY_GUIDE_PAN_SECOND_GROUP);
                } else {
                    HuAudFXPlayPan(choiceSounds[cueIndex], MDPARTY_GUIDE_PAN_FIRST_GROUP);
                }
                break;
            }
        }
    }
}

/* Loads Party Mode unlock flags and initializes board availability before setup. */
void fn_1_1B4(void)
{
    lbl_1_bss_2A = FALSE;
    lbl_1_bss_2C = FALSE;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_2A = TRUE;
    }
    if (GWBankFlagGet(3)) {
        lbl_1_bss_2C = TRUE;
    }
    lbl_1_data_BEE[0] = TRUE;
    lbl_1_data_BEE[1] = TRUE;
    lbl_1_data_BEE[2] = TRUE;
    lbl_1_data_BEE[3] = TRUE;
    lbl_1_data_BEE[4] = TRUE;
    lbl_1_data_BEE[5] = FALSE;
    if (GWBankFlagGet(MDPARTY_BANK_FLAG_SIXTH_BOARD)) {
        lbl_1_data_BEE[5] = TRUE;
    }
}

inline void fn_1_1B4(void);

/* Before board data loads, if any player lacks motion data, closes character data and reloads it
* for all four players. */
void fn_1_2B0(void)
{
    s16 i;
    s32 status;

    for (i = 0; i < 4; i++) {
        if ((void *)CharMotionAMemPGet(GwPlayer[i].charNo) == NULL) {
            break;
        }
    }
    if (i != 4) {
        CharDataClose(-1);
        for (i = 0; i < 4; i++) {
            status = HuDataDirReadAsync(
                CharDataDirTbl[GwPlayer[i].charNo][4]);
            if (status != -1) {
                while (!HuDataGetAsyncStat(status)) {
                    HuPrcVSleep();
                }
            }
            CharMotionInit(GwPlayer[i].charNo);
            HuDataDirClose(CharDataDirTbl[GwPlayer[i].charNo][4]);
        }
    }
}

inline void fn_1_2B0(void);

/* Loads common board assets into ARAM, then reads the selected board archive and waits for the read
 * before signaling ready. */
void fn_1_400(void)
{
    s32 dataDir[2] = { DATA_board_us, DATA_capsule };
    s16 i;
    s32 status;

    fn_1_2B0();
    for (i = 0; i < 2; i++) {
        status = HuDataDirReadAsync(dataDir[i]);
        if (status != -1) {
            while (!HuDataGetAsyncStat(status)) {
                HuPrcVSleep();
            }
        }
        HuAR_MRAMtoARAM(dataDir[i]);
        while (HuARDMACheck()) {
            HuPrcVSleep();
        }
        HuDataDirClose(dataDir[i]);
    }
    status = HuDataDirReadAsync(lbl_1_data_BFC[GwSystem.boardNo]);
    if (status != -1) {
        while (!HuDataGetAsyncStat(status)) {
            HuPrcVSleep();
        }
    }
    lbl_1_bss_2E = TRUE;
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Ends setup loading, closes Party Mode data, and starts the asynchronous board-asset loader. */
void fn_1_64C(void)
{
    lbl_1_bss_2E = FALSE;
    OSReport(lbl_1_data_C34);
    OSReport(lbl_1_data_C66, _effect);
    OSReport(lbl_1_data_C77, _gamemes);
    OSReport(lbl_1_data_C89, _mgconst);
    OSReport(lbl_1_data_C9B, _win);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
    HuDataDirClose(DATA_mdparty);
    HuPrcChildCreate(fn_1_400, 256, 16384, 0, lbl_1_bss_0);
}

inline void fn_1_64C(void);

/* Copies Party Mode rules and player settings into game state, inverts the bonus-star option, and
 * in Tag Mode clears handicaps before applying the two saved team handicaps. */
void fn_1_718(void)
{
    s16 i;

    lbl_1_bss_9F4.bonusStarChoice++;
    lbl_1_bss_9F4.bonusStarChoice %= 2;
    GwSystem.tagF = lbl_1_bss_9F4.tagMode;
    GwSystem.bonusStarF = lbl_1_bss_9F4.bonusStarChoice;
    GwSystem.mgPack = lbl_1_bss_9F4.minigamePack;
    GwSystem.turnNo = TRUE;
    GwSystem.turnMax = (lbl_1_bss_9F4.turnCountChoice * 5) + 10;
    GwSystem.boardNo = lbl_1_bss_9F4.boardNo;
    GwCommon.partyMgPack = GwSystem.mgPack;
    GwCommon.confTag = lbl_1_bss_9F4.tagMode;
    GwCommon.confTurnNum = (lbl_1_bss_9F4.turnCountChoice * 5) + 10;
    GwCommon.partyMgPack = lbl_1_bss_9F4.minigamePack;
    GwCommon.confBonusStar = lbl_1_bss_9F4.bonusStarChoice;
    GwCommon.lastBoard = lbl_1_bss_9F4.boardNo;

    for (i = 0; i < 4; i++) {
        GwPlayer[i].comF = lbl_1_bss_9BC[i].isComputer;
        GwPlayer[i].comDif = lbl_1_bss_9BC[i].computerDifficulty;
        GwPlayer[i].charNo = lbl_1_bss_9BC[i].chrSel;
        GwPlayer[i].padNo = lbl_1_bss_9BC[i].padNo;
        GwPlayer[i].handicap = lbl_1_bss_9BC[i].handicap;
        GwPlayer[i].team = FALSE;
    }
    if (GwSystem.tagF) {
        for (i = 0; i < 4; i++) {
            GwPlayer[i].handicap = 0;
        }
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][0]].handicap =
            lbl_1_bss_9BC[0].handicap;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][2]].handicap =
            lbl_1_bss_9BC[1].handicap;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][0]].team = FALSE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][1]].team = FALSE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][2]].team = TRUE;
        GwPlayer[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][3]].team = TRUE;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = FALSE;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    _ClearFlag(FLAG_BOARD_TUTORIAL);
    mbSavePartyInit(
        GwSystem.tagF, GwSystem.bonusStarF, GwSystem.mgPack,
        GwSystem.turnMax, GwPlayer[0].handicap, GwPlayer[1].handicap,
        GwPlayer[2].handicap, GwPlayer[3].handicap);
    mbSaveInit(GwSystem.boardNo);
}

/* Initializes four CPU players and selects board 6 when the opening board option is chosen. */
void fn_1_E30(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        GwPlayer[i].comF = TRUE;
        GwPlayer[i].comDif = 0;
        GwPlayer[i].charNo = i;
        GwPlayer[i].padNo = i;
        GwPlayer[i].handicap = 0;
        GwPlayer[i].team = FALSE;
    }
    for (i = 0; i < 4; i++) {
        GwPlayerConf[i].grpNo = FALSE;
        GwPlayerConf[i].type = GwPlayer[i].comF;
        GwPlayerConf[i].comDif = GwPlayer[i].comDif;
        GwPlayerConf[i].charNo = GwPlayer[i].charNo;
        GwPlayerConf[i].padNo = GwPlayer[i].padNo;
        GwPlayerConf[i].grpNo = GwPlayer[i].team;
    }
    mbSaveInit(9);
    GwSystem.boardNo = 6;
}

inline void fn_1_E30(void);

/* Waits for the loader-ready flag, then enters the board or minigame flow after the selected-board
 * read completes. */
void fn_1_109C(s16 nextFlow)
{
    OMOVL boardDll[6] = {
        DLL_w01dll,
        DLL_w02dll,
        DLL_w03dll,
        DLL_w04dll,
        DLL_w05dll,
        DLL_w06dll,
    };

    do {
        HuPrcVSleep();
    } while (!lbl_1_bss_2E);
    OSReport(lbl_1_data_CAB);
    OSReport(lbl_1_data_C66, _effect);
    OSReport(lbl_1_data_C77, _gamemes);
    OSReport(lbl_1_data_C89, _mgconst);
    OSReport(lbl_1_data_C9B, _win);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);

    switch (nextFlow) {
        case 0: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(
                0, boardDll[GwSystem.boardNo], history->evtno,
                history->stat);
            omOvlGotoEx(boardDll[GwSystem.boardNo], TRUE, 0, 0);
            break;
        }
        case 1: {
            OMOVLHIS *history = omOvlHisGet(0);

            omOvlHisChg(0, history->ovl, 1, 0);
            omOvlCallEx(DLL_w10dll, TRUE, 0, 0);
            break;
        }
    }
}

inline void fn_1_109C(s16 nextFlow);

/* On Party Mode entry, closes character data and frees board and capsule ARAM archives before
 * reporting memory use. */
void fn_1_1248(void)
{
    CharDataClose(-1);
    HuARDirFree(DATA_board);
    HuARDirFree(DATA_board_us);
    HuARDirFree(DATA_capsule);
    OSReport(lbl_1_data_CDC);
    OSReport(lbl_1_data_C66, _effect);
    OSReport(lbl_1_data_C77, _gamemes);
    OSReport(lbl_1_data_C89, _mgconst);
    OSReport(lbl_1_data_C9B, _win);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
}

/* Alternate Party Mode teardown closes character data and frees board and capsule archives before
* logging the overlay-return memory report. */
void fn_1_12F8(void)
{
    CharDataClose(-1);
    HuARDirFree(DATA_board);
    HuARDirFree(DATA_board_us);
    HuARDirFree(DATA_capsule);
    OSReport(lbl_1_data_D0C);
    OSReport(lbl_1_data_C66, _effect);
    OSReport(lbl_1_data_C77, _gamemes);
    OSReport(lbl_1_data_C89, _mgconst);
    OSReport(lbl_1_data_C9B, _win);
    HuAMemDump();
    OSReport(lbl_1_data_CA9);
}

inline void fn_1_12F8(void);

/* Evaluates one coordinate of the quadratic Bezier path used by Party Mode model motion. */
float fn_1_13A8(float startValue, float controlValue, float endValue, float progress)
{
    float remaining = 1.0f - progress;

    return (endValue * (progress * progress))
        + ((startValue * (remaining * remaining))
            + ((controlValue * (remaining * progress) * 2.0f)));
}

/* The curved model-motion helper evaluates its quadratic path at the supplied 0-to-1 progress. */
void fn_1_1404(
    HuVecF *position, const HuVecF *startPos, const HuVecF *controlPos, const HuVecF *endPos,
    float progress)
{
    position->x = fn_1_13A8(startPos->x, controlPos->x, endPos->x, progress);
    position->y = fn_1_13A8(startPos->y, controlPos->y, endPos->y, progress);
    position->z = fn_1_13A8(startPos->z, controlPos->z, endPos->z, progress);
}

inline void fn_1_1404(
    HuVecF *position, const HuVecF *startPos, const HuVecF *controlPos, const HuVecF *endPos,
    float progress);

/* Moves a value toward its target using the same frame-based easing used by Party Mode motion. */
float fn_1_160C(float currentValue, float targetValue, float easingFactor)
{
    if (currentValue == targetValue) {
        return targetValue;
    }
    return (targetValue + (currentValue * (easingFactor - 1.0f))) / easingFactor;
}

inline float fn_1_160C(float currentValue, float targetValue, float easingFactor);

/* Per-frame menu and camera updates ease each vector component toward its target. */
void fn_1_163C(HuVecF *dst, const HuVecF *src, float weight)
{
    dst->x = fn_1_160C(dst->x, src->x, weight);
    dst->y = fn_1_160C(dst->y, src->y, weight);
    dst->z = fn_1_160C(dst->z, src->z, weight);
}

inline void fn_1_163C(HuVecF *dst, const HuVecF *src, float weight);

/* After setup is accepted, the effect callback uses this to ease an attached effect outward. */
float fn_1_1780(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / durationFrames) * elapsedFrames));
}

inline float fn_1_1780(
    float startValue, float endValue, float elapsedFrames, float durationFrames);

/* Menu and model callbacks use this to interpolate values across animation frames. */
float fn_1_1868(float startValue, float endValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f) {
        return startValue;
    }
    if (elapsedFrames >= durationFrames) {
        return endValue;
    }
    return startValue + ((elapsedFrames / durationFrames) * (endValue - startValue));
}

inline float fn_1_1868(float startValue, float endValue, float elapsedFrames, float durationFrames);

/* Model entrance and exit updates move along a straight path each frame and ease the heading toward
 * the movement direction while retaining the other rotation axes. */
void fn_1_18AC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration)
{
    HuVecF modelVector;
    HuVecF modelRot;
    HuVecF pos;
    HuVecF rot;

    Hu3DModelPosGet(modelId, &modelVector);
    Hu3DModelRotGet(modelId, &modelRot);
    pos.x = fn_1_1868(start->x, end->x, time, duration);
    pos.y = fn_1_1868(start->y, end->y, time, duration);
    pos.z = fn_1_1868(start->z, end->z, time, duration);
    modelVector.x -= pos.x;
    modelVector.z -= pos.z;
    rot.y = -(180.0 * (atan2(modelVector.x, -modelVector.z) / M_PI));
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_160C(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelPosSetV(modelId, &pos);
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_18AC(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration);

/* The entrance update uses this after travel ends to turn the model toward its final heading. */
void fn_1_1C14(HU3D_MODELID modelId, float rotY)
{
    HuVecF modelRot;
    HuVecF rot;

    Hu3DModelRotGet(modelId, &modelRot);
    rot.y = rotY;
    if (modelRot.y - rot.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - rot.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    rot.x = modelRot.x;
    rot.y = fn_1_160C(modelRot.y, rot.y, 10.0f);
    rot.z = modelRot.z;
    Hu3DModelRotSetV(modelId, &rot);
}

inline void fn_1_1C14(HU3D_MODELID modelId, float rotY);

/* Setup model callbacks follow the straight path through its final frame, then turn toward the
 * requested final heading on later frames. */
void fn_1_1D54(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY)
{
    if (time <= duration) {
        fn_1_18AC(modelId, start, end, time, duration);
    } else {
        fn_1_1C14(modelId, rotY);
    }
}

inline void fn_1_1D54(
    HU3D_MODELID modelId, HuVecF *start, HuVecF *end, float time,
    float duration, float rotY);

/* After setup is accepted, the effect callback uses this for the attached effect's vertical arc. */
float fn_1_21D4(float startValue, float peakValue, float elapsedFrames, float durationFrames)
{
    if (elapsedFrames <= 0.0f || elapsedFrames >= durationFrames) {
        return startValue;
    }
    return startValue +
           ((peakValue - startValue) * sind((180.0f / durationFrames) * elapsedFrames));
}

inline float fn_1_21D4(
    float startValue, float peakValue, float elapsedFrames, float durationFrames);

/* Setup model callbacks follow a quadratic path each frame and ease the heading toward travel;
 * pitch and roll are forced to zero. */
void fn_1_22A8(
    HU3D_MODELID modelId, const HuVecF *startPos, const HuVecF *controlPos, const HuVecF *endPos,
    float elapsedFrames, float durationFrames)
{
    HuVecF pos;
    HuVecF modelVector;
    HuVecF modelRot;
    float pathProgress;

    pathProgress = fn_1_1868(0.0f, 1.0f, elapsedFrames, durationFrames);
    fn_1_1404(&pos, startPos, controlPos, endPos, pathProgress);
    Hu3DModelPosGet(modelId, &modelVector);
    Hu3DModelRotGet(modelId, &modelRot);
    modelVector.x -= pos.x;
    modelVector.z -= pos.z;
    modelVector.y = -(180.0 * (atan2(modelVector.x, -modelVector.z) / M_PI));
    if (modelRot.y - modelVector.y > 180.0f) {
        modelRot.y -= 360.0f;
    } else if (modelRot.y - modelVector.y < -180.0f) {
        modelRot.y += 360.0f;
    }
    modelRot.y = fn_1_160C(modelRot.y, modelVector.y, 10.0f);
    Hu3DModelPosSet(modelId, pos.x, pos.y, pos.z);
    Hu3DModelRotSet(modelId, 0.0f, modelRot.y, 0.0f);
}

inline void fn_1_22A8(
    HU3D_MODELID modelId, const HuVecF *startPos, const HuVecF *controlPos, const HuVecF *endPos,
    float elapsedFrames, float durationFrames);

/* Party Mode setup resets each player slot before character selection begins. */
void fn_1_26FC(void)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        entry->characterChoiceState = 0;
        entry->unusedSetupValue = 0;
        entry->computerDifficulty = 0;
        entry->chrSel = i;
        entry->padNo = i;
    }
}

/* Party Mode setup always makes slot 0 human, assigns further human slots to connected pads,
 * and marks the remaining slots as CPUs. */
void fn_1_275C(void)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 playerCount = lbl_1_bss_9F4.playerCount;
    s16 i;

    lbl_1_bss_9BC[0].isComputer = 0;
    playerCount--;
    for (i = 1, entry = &lbl_1_bss_9BC[1]; i < 4; i++, entry++) {
        if (playerCount > 0 && HuPadStatGet(i) == 0) {
        entry->isComputer = 0;
            playerCount--;
        } else {
        entry->isComputer = 1;
        }
    }
}

inline void fn_1_275C(void);

/* Camera mode changes snap the live view to the camera's requested center, rotation, and zoom. */
void fn_1_2810(MDCAMERA_WORK *camera)
{
    memcpy(&camera->center, &camera->targetCenter, sizeof(HuVecF));
    memcpy(&camera->rot, &camera->targetRot, sizeof(HuVecF));
    camera->zoom = camera->targetZoom;
}

inline void fn_1_2810(MDCAMERA_WORK *camera);

/* Camera mode changes preserve the live view as the next transition's target. */
void fn_1_2860(MDCAMERA_WORK *camera)
{
    memcpy(&camera->targetCenter, &camera->center, sizeof(HuVecF));
    memcpy(&camera->targetRot, &camera->rot, sizeof(HuVecF));
    camera->targetZoom = camera->zoom;
}

inline void fn_1_2860(MDCAMERA_WORK *camera);

/* The camera update eases its center, rotation, and zoom toward their targets each frame. */
void fn_1_28B0(MDCAMERA_WORK *camera, float weight)
{
    fn_1_163C(&camera->center, &camera->targetCenter, weight);
    fn_1_163C(&camera->rot, &camera->targetRot, weight);
    camera->zoom = fn_1_160C(camera->zoom, camera->targetZoom, weight);
}

inline void fn_1_28B0(MDCAMERA_WORK *camera, float weight);

void fn_1_2B64(MDCAMERA_CALLBACK callback)
{
    lbl_1_bss_A10.callback = callback;
}

inline void fn_1_2B64(MDCAMERA_CALLBACK callback);

/* The free-camera object applies controller input to its targets and prints the live camera
 * values. */
void fn_1_2B74(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    float stickX = HuPadStkX[0];
    float stickY = -HuPadStkY[0];
    HuVec2f subStick;
    float moveX;
    float moveZ;

    subStick.y = -HuPadSubStkY[0];
    subStick.x = HuPadSubStkX[0];
    camera->targetRot.x += 0.02f * subStick.y;
    if (camera->targetRot.x > 85.0f) {
        camera->targetRot.x = 85.0f;
    }
    if (camera->targetRot.x < -85.0f) {
        camera->targetRot.x = -85.0f;
    }
    camera->targetRot.y += 0.02f * subStick.x;

    if (HuPadBtn[0] & PAD_TRIGGER_L) {
        camera->targetCenter.y -= 0.25f * stickY;
    } else if (HuPadBtn[0] & PAD_TRIGGER_R) {
        camera->targetZoom += 0.25f * stickY;
        if (camera->targetZoom < 5.0f) {
            camera->targetZoom = 5.0f;
        }
    } else {
        moveX = stickX * sind(90.0f + camera->rot.y)
            + stickY * sind(camera->rot.y);
        moveZ = stickX * cosd(90.0f + camera->rot.y)
            + stickY * cosd(camera->rot.y);
        camera->targetCenter.x += 0.25f * moveX;
        camera->targetCenter.z += 0.25f * moveZ;
    }
    fn_1_28B0(camera, 15.0f);

    print8(16, 110, 1.0f, lbl_1_data_D3E);
    print8(16, 120, 1.0f, lbl_1_data_D60,
        camera->center.x, camera->center.y, camera->center.z);
    print8(16, 130, 1.0f, lbl_1_data_D7A,
        camera->rot.x, camera->rot.y, camera->rot.z);
    print8(16, 140, 1.0f, lbl_1_data_D94, camera->zoom);
}

/* Each camera object update dispatches the callback selected by the current Party Mode flow. */
void fn_1_3284(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    if (camera->callback) {
        camera->callback(obj, camera);
    }
}

inline void fn_1_3284(OMOBJ *obj, MDCAMERA_WORK *camera);

/* The camera object's per-frame callback updates shared camera state and applies it to camera 0. */
void fn_1_32D0(OMOBJ *obj)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    fn_1_3284(obj, camera);
    Center.x = camera->center.x;
    Center.y = camera->center.y;
    Center.z = camera->center.z;
    CRot.x = camera->rot.x;
    CRot.y = camera->rot.y;
    CRot.z = camera->rot.z;
    CZoom = camera->zoom;
    omOutView(obj);
}

/* Party Mode camera setup creates camera 0, initializes its default view, and installs its update
 * object. */
void fn_1_33A0(MDCAMERA_CALLBACK callback)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 10.0f, 10000.0f, 1.2f);
    Hu3DCameraViewportSet(
        1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    memset(camera, 0, sizeof(MDCAMERA_WORK));
    camera->callback = callback;
    camera->center.x = 0.0f;
    camera->center.y = 65.0f;
    camera->center.z = -800.0f;
    camera->rot.x = -7.25f;
    camera->rot.y = 0.0f;
    camera->rot.z = 0.0f;
    if (lbl_1_bss_28 == 0) {
        camera->zoom = 2650.0f;
    } else {
        camera->zoom = 2150.0f;
    }
    camera->obj =
        omAddObjEx(lbl_1_bss_0, 32730, 0, 0, -1, fn_1_32D0);
}

inline void fn_1_33A0(MDCAMERA_CALLBACK callback);

/* Party Mode camera teardown kills camera 0 and deletes its update object. */
void fn_1_353C(void)
{
    MDCAMERA_WORK *camera = &lbl_1_bss_A10;

    Hu3DCameraKill(1);
    if (camera->obj) {
        omDelObjEx(lbl_1_bss_0, camera->obj);
    }
    camera->obj = NULL;
}

inline void fn_1_353C(void);

/* Party Mode lighting setup creates the two fixed directional lights used by its models. */
void fn_1_3598(void)
{
    lbl_1_bss_A68[0] = Hu3DGLightCreate(
        0.0f, 1.0f, 1.0f, 0.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A68[0]);
    Hu3DGLightStaticSet(lbl_1_bss_A68[0], TRUE);
    lbl_1_bss_A68[1] = Hu3DGLightCreate(
        -1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lbl_1_bss_A68[1]);
    Hu3DGLightStaticSet(lbl_1_bss_A68[1], TRUE);
}

inline void fn_1_3598(void);

void fn_1_36C4(void)
{
    Hu3DGLightKill(lbl_1_bss_A68[0]);
    Hu3DGLightKill(lbl_1_bss_A68[1]);
}

inline void fn_1_36C4(void);

/* The menu flow opens the selected message window, using the standard display path for window
 * zero. */
void fn_1_3700(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOn(lbl_1_bss_A60[winNo]);
    } else {
        HuWinExOpen(lbl_1_bss_A60[winNo]);
    }
}

inline void fn_1_3700(s16 winNo);

/* The menu flow closes the selected message window, using the standard display path for window
 * zero. */
void fn_1_3770(s16 winNo)
{
    if (winNo == 0) {
        HuWinDispOff(lbl_1_bss_A60[winNo]);
    } else {
        HuWinExClose(lbl_1_bss_A60[winNo]);
    }
}

inline void fn_1_3770(s16 winNo);

void fn_1_37E0(s16 winNo)
{
    HuWinMesWait(lbl_1_bss_A60[winNo]);
}

inline void fn_1_37E0(s16 winNo);

/* A menu step reads a choice; mode 2 turns a canceled choice into option 1. */
s16 fn_1_381C(s16 winNo, s16 mode)
{
    s16 choice = 0;

    if (mode == 1) {
        HuWinAttrSet(lbl_1_bss_A60[winNo], HUWIN_ATTR_NOCANCEL);
    } else {
        HuWinAttrReset(lbl_1_bss_A60[winNo], HUWIN_ATTR_NOCANCEL);
    }
    choice = HuWinChoiceGet(lbl_1_bss_A60[winNo], -1);
    if (mode == 2 && choice == -1) {
        choice = 1;
    }
    return choice;
}

inline s16 fn_1_381C(s16 winNo, s16 mode);

/* The menu flow centers a message, sets its text speed, and invalidates the cached message when it
 * changes. */
void fn_1_38F0(s16 winNo, s32 messNum, s16 speed)
{
    HuWinAttrSet(lbl_1_bss_A60[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_A60[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_A60[winNo], speed);
    if (lbl_1_data_BC8 != messNum) {
        lbl_1_data_BC8 = -1;
    }
}

inline void fn_1_38F0(s16 winNo, s32 messNum, s16 speed);

void fn_1_39AC(s16 winNo, s32 messNum, s16 insertPos)
{
    HuWinHomeClear(lbl_1_bss_A60[winNo]);
    HuWinInsertMesSet(lbl_1_bss_A60[winNo], messNum, insertPos);
}

inline void fn_1_39AC(s16 winNo, s32 messNum, s16 insertPos);

/* Party Mode window setup creates the four message windows used by its menus. */
void fn_1_3A1C(void)
{
    s16 i;

    HuWinInit(1);
    lbl_1_bss_A60[0] =
        HuWinExCreateFrame(16.0f, 337.0f, 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_A60[0]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[0], 0.0f);
    lbl_1_bss_A60[1] =
        HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 5);
    HuWinDispOff(lbl_1_bss_A60[1]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[1], 0.9f);
    lbl_1_bss_A60[2] =
        HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 3);
    HuWinDispOff(lbl_1_bss_A60[2]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[2], 0.9f);
    lbl_1_bss_A60[3] =
        HuWinExCreateFrame(16.0f, 372.0f, 544, 68, -1, 4);
    HuWinDispOff(lbl_1_bss_A60[3]);
    HuWinBGTPLvlSet(lbl_1_bss_A60[3], 0.9f);

    for (i = 0; i < 4; i++) {
        winData[lbl_1_bss_A60[i]].padMask = 1;
        HuWinCallbackSet(lbl_1_bss_A60[i], (HUWIN_CALLBACK)fn_1_0);
    }
}

inline void fn_1_3A1C(void);

/* Party Mode window teardown kills its four managed windows and clears any other open window. */
void fn_1_3C44(void)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        HuWinExKill(lbl_1_bss_A60[i]);
    }
    HuWinAllKill();
}

inline void fn_1_3C44(void);

/* Before displaying menu text, this selects its window and clears both cached message IDs when it
 * changes. */
void fn_1_3CA0(s16 winNo)
{
    if (lbl_1_data_DA2[0] != -1 && lbl_1_data_DA2[0] != winNo) {
        fn_1_3770(lbl_1_data_DA2[0]);
    }
    if (lbl_1_data_DA2[0] == -1 || lbl_1_data_DA2[0] != winNo) {
        lbl_1_data_DA2[0] = winNo;
        lbl_1_data_DA8[0] = -1;
        lbl_1_data_DA8[1] = -1;
        fn_1_3700(lbl_1_data_DA2[0]);
    }
}

inline void fn_1_3CA0(s16 winNo);

/* The active menu flow closes its message window and clears the selected-window and message
 * caches. */
void fn_1_3E0C(void)
{
    if (lbl_1_data_DA2[0] != -1) {
        fn_1_3770(lbl_1_data_DA2[0]);
    }
    lbl_1_data_DA2[0] = -1;
    lbl_1_data_DA8[0] = -1;
    lbl_1_data_DA8[1] = -1;
}

inline void fn_1_3E0C(void);

/* A menu step waits for the active message window's text to finish. */
void fn_1_3EC8(void)
{
    if (lbl_1_data_DA2[0] != -1) {
        fn_1_37E0(lbl_1_data_DA2[0]);
    }
}

inline void fn_1_3EC8(void);

/* The active menu flow reads its choice, returning zero when no message window is selected. */
s16 fn_1_3F28(s16 mode)
{
    if (lbl_1_data_DA2[0] != -1) {
        return fn_1_381C(lbl_1_data_DA2[0], mode);
    }
    return 0;
}

inline s16 fn_1_3F28(s16 mode);

/* Menu flows display a regular message only when its ID differs from the cached message. */
void fn_1_4020(s16 winNo, s32 messNum, s16 speed)
{
    fn_1_3CA0(winNo);
    if (lbl_1_data_DA8[0] != messNum) {
        lbl_1_data_DA8[0] = messNum;
        lbl_1_data_DA8[1] = -1;
        fn_1_38F0(lbl_1_data_DA2[0], lbl_1_data_DA8[0], speed);
    }
}

inline void fn_1_4020(s16 winNo, s32 messNum, s16 speed);

/* Menu flows prepare an inserted message only when its ID differs from the cached insertion,
* clearing the window's current page before storing data for the enclosing message to use. */
void fn_1_4258(s16 winNo, s32 messNum, s16 insertPos)
{
    fn_1_3CA0(winNo);
    if (lbl_1_data_DA8[1] != messNum) {
        lbl_1_data_DA8[0] = -1;
        lbl_1_data_DA8[1] = messNum;
        fn_1_39AC(lbl_1_data_DA2[0], lbl_1_data_DA8[1], insertPos);
    }
}

inline void fn_1_4258(s16 winNo, s32 messNum, s16 insertPos);

/* Party Mode displays a persistent message in window zero, opening that window on first use. */
void fn_1_444C(s32 messNum)
{
    if (lbl_1_data_DA2[1] == -1) {
        lbl_1_data_DA2[1] = 0;
        lbl_1_data_DA8[2] = -1;
        fn_1_3700(lbl_1_data_DA2[1]);
    }
    if (lbl_1_data_DA8[2] != messNum) {
        lbl_1_data_DA8[2] = messNum;
        fn_1_38F0(lbl_1_data_DA2[1], lbl_1_data_DA8[2], 0);
    }
}

inline void fn_1_444C(s32 messNum);

/* The persistent-message flow closes window zero and clears its cached message ID. */
void fn_1_45D0(void)
{
    if (lbl_1_data_DA2[1] != -1) {
        fn_1_3770(lbl_1_data_DA2[1]);
    }
    lbl_1_data_DA2[1] = -1;
    lbl_1_data_DA8[2] = -1;
}

inline void fn_1_45D0(void);

/* Party Mode shadow setup positions the directional shadow camera over the board scene. */
void fn_1_467C(void)
{
    HuVecF shadowPos = { 0.0f, 3000.0f, 600.0f };
    HuVecF shadowUp = { 0.0f, 1.0f, 0.0f };
    HuVecF shadowTarget = { 0.0f, 0.0f, 0.0f };

    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
}

inline void fn_1_467C(void);

void fn_1_4730(void)
{
}

/* Menu setup enables the supplied sprite attribute on every member of a sprite group. */
void fn_1_4734(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrSet(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_4734(HUSPR_GROUPID groupId, s32 attr);

/* Menu setup clears the supplied sprite attribute on every member of a sprite group. */
void fn_1_47B4(HUSPR_GROUPID groupId, s32 attr)
{
    s16 memberNo;
    HUSPR_GROUP *group = &HuSprGrpData[groupId];

    for (memberNo = 0; memberNo < group->sprNum; memberNo++) {
        HuSprAttrReset(groupId, memberNo, (u16)attr);
    }
}

inline void fn_1_47B4(HUSPR_GROUPID groupId, s32 attr);

/* Party Mode sprite setup loads the menu animations and creates and configures every sprite
 * group. */
void fn_1_4834(void)
{
    MDSPR_INFO *desc;
    s16 i;

    for (i = 0; i < 32; i++) {
        lbl_1_bss_93C[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_1C[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 17; i++) {
        lbl_1_bss_918[i] = HuSprGrpCreate(lbl_1_data_9C[i]);
    }
    for (i = 0, desc = lbl_1_data_C0; i < 58; i++, desc++) {
        lbl_1_bss_8A4[i] = HuSprCreate(
            lbl_1_bss_93C[desc->animNo], desc->priority, desc->bank);
        HuSprGrpMemberSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            lbl_1_bss_8A4[i]);
        HuSprPosSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->pos.x, desc->pos.y);
        HuSprScaleSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->scale.x, desc->scale.y);
        HuSprZRotSet(lbl_1_bss_918[desc->groupNo], desc->memberNo,
            desc->zRot);
    }
    for (i = 0; i < 17; i++) {
        fn_1_4734(lbl_1_bss_918[i], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_4834(void);

void fn_1_4A80(void)
{
}

/* After setup is accepted, moves the two setup characters' attached effects along outward arcs
 * and stops updating after both paths finish. */
void fn_1_4A84(OMOBJ *obj)
{
    s16 done = 0;
    float duration = 60.0f;
    OMOBJ *obj0 = lbl_1_bss_C;
    OMOBJ *obj1 = lbl_1_bss_10;
    LBL_1_BSS_228_ENTRY *work0 = &lbl_1_bss_228[0];
    LBL_1_BSS_228_ENTRY *work1 = &lbl_1_bss_228[1];
    HuVecF pos;

    if (obj0 != NULL) {
        pos.x = fn_1_1780(work0->pos.x, 500.0f, work0->time, duration);
        pos.y = fn_1_21D4(work0->pos.y, 400.0f, work0->time, duration);
        pos.z = work0->pos.z;
        if (++work0->time > duration) {
            done++;
            fn_1_43F3C(0, NULL, 0);
        } else {
            fn_1_43F3C(0, &pos, 1);
        }
    }
    if (obj1 != NULL) {
        pos.x = fn_1_1780(work1->pos.x, -500.0f, work1->time, duration);
        pos.y = fn_1_21D4(work1->pos.y, 400.0f, work1->time, duration);
        pos.z = work1->pos.z;
        if (++work1->time > duration) {
            done++;
            fn_1_43F3C(1, NULL, 0);
        } else {
            fn_1_43F3C(1, &pos, 1);
        }
    }
    if (done >= 2) {
        obj->objFunc = NULL;
    }
}

/* For 31 updates, captures each active character's attachment position and updates its effect,
 * then starts the effects' 60-frame arcs. */
void fn_1_4EDC(OMOBJ *obj)
{
    OMOBJ *obj0 = lbl_1_bss_C;
    OMOBJ *obj1 = lbl_1_bss_10;
    LBL_1_BSS_228_ENTRY *work0 = &lbl_1_bss_228[0];
    LBL_1_BSS_228_ENTRY *work1 = &lbl_1_bss_228[1];

    if (obj0 != NULL) {
        Hu3DModelObjPosGet(obj0->mdlId[0], lbl_1_data_DB4, &work0->pos);
        fn_1_43F3C(0, &work0->pos, 1);
        work0->time = 0.0f;
    }
    if (obj1 != NULL) {
        Hu3DModelObjPosGet(obj1->mdlId[0], lbl_1_data_DB4, &work1->pos);
        fn_1_43F3C(1, &work1->pos, 1);
        work1->time = 0.0f;
    }
    if (++obj->work[0] > 30) {
        obj->work[0] = 0;
        obj->objFunc = fn_1_4A84;
    }
}

/* Character setup starts both models' setup motions, schedules the position transition, and plays
 * two cues. */
void fn_1_4FE8(void)
{
    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[4],
        0.0f, 5.0f, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[4],
        0.0f, 5.0f, 0);
    lbl_1_bss_30 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_4EDC);
    lbl_1_bss_30->work[0] = 0;
    HuAudFXPlay(MDPARTY_SE_SETUP_CHARACTER_FIRST);
    HuAudFXPlay(MDPARTY_SE_SETUP_CHARACTER_SECOND);
}

inline void fn_1_4FE8(void);

/* Setup prompt callback: doubles the current motion's speed for 32 updates, then restores speed 1
 * and loops motion 0. */
void fn_1_50EC(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

/* Starts the first character's brief setup motion and installs its per-frame callback. */
void fn_1_5194(void)
{
    OMOBJ *obj = lbl_1_bss_C;

    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[3], 0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_50EC;
}

inline void fn_1_5194(void);

/* Setup prompt callback: doubles the current motion's speed for 32 updates, then restores speed 1
 * and loops motion 0. */
void fn_1_5208(OMOBJ *obj)
{
    Hu3DMotionSpeedSet(obj->mdlId[0], 2.0f);
    if (obj->work[3]++ > 30) {
        obj->objFunc = NULL;
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 15.0f,
            HU3D_MOTATTR_LOOP);
    }
}

/* Starts the second character's brief setup motion and installs its per-frame callback. */
void fn_1_52B0(void)
{
    OMOBJ *obj = lbl_1_bss_10;

    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[3], 0.0f, 10.0f, 0);
    obj->work[3] = 0;
    obj->objFunc = fn_1_5208;
}

inline void fn_1_52B0(void);

/* After a game is accepted or resumed, raises the first setup character, follows its curved
 * departure path, and stops its sound. */
void fn_1_5324(OMOBJ *obj)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[0];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->time += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->pos);
            work->state = 1;
            work->time = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_22A8(obj->mdlId[0], &work->pos, &work->controlPos,
            &work->endPos, work->time, work->duration);
        if ((work->time += 1.0f) > work->duration) {
            HuAudFXStop(lbl_1_bss_A6C[0]);
            work->state = 2;
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_464A0(4, &pos, 1, 5);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

/* When a game is accepted or resumed, sets fixed departure waypoints for the first setup
* character, overwriting all three position reads. The installed update raises the model and
* then captures the curve's starting position. */
void fn_1_58A0(void)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[0];
    OMOBJ *obj = lbl_1_bss_C;

    work->state = 0;
    work->time = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = -325.0f;
    work->pos.y = 0.0f;
    work->pos.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->controlPos);
    work->controlPos.x = 100.0f;
    work->controlPos.y = 600.0f;
    work->controlPos.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->endPos);
    work->endPos.x = 100.0f;
    work->endPos.y = 225.0f;
    work->endPos.z = -1000.0f;
    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[2], 0.0f, 0.0f, 0);
    lbl_1_bss_A6C[0] = HuAudFXPlay(MSM_SE_BRD00_146);
    obj->objFunc = fn_1_5324;
}

inline void fn_1_58A0(void);

/* Per-frame callback that moves the first character model into its menu position before looping its
 * idle motion. */
void fn_1_5A18(OMOBJ *obj)
{
    HuVecF path[2] = {
        { -100.0f, 0.0f, 0.0f },
        { -325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1D54(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, 15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Accepts the first character's exit only after its model reaches the menu boundary, then starts
 * its exit motion. */
s32 fn_1_5FB0(void)
{
    OMOBJ *obj = lbl_1_bss_C;
    HuVecF pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x < -100.0f) {
        return FALSE;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_5A18;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    return TRUE;
}

inline s32 fn_1_5FB0(void);

/* Creates the first character model and its motions when the character setup menu is
 * initialized. */
void fn_1_605C(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 79), HU_MEMNUM_OVL));
    obj->mtnId[0] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 80), HU_MEMNUM_OVL));
    obj->mtnId[1] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 81), HU_MEMNUM_OVL));
    obj->mtnId[2] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 82), HU_MEMNUM_OVL));
    obj->mtnId[3] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 83), HU_MEMNUM_OVL));
    obj->mtnId[4] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 84), HU_MEMNUM_OVL));
    obj->mtnId[5] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 85), HU_MEMNUM_OVL));
    Hu3DModelPosSet(obj->mdlId[0], -100.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

/* Releases the first character model and its six motions when the menu object is destroyed. */
void fn_1_6284(OMOBJ *obj)
{
    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DMotionKill(obj->mtnId[5]);
        Hu3DModelKill(obj->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj->objFunc = NULL;
}

inline void fn_1_6284(OMOBJ *obj);

/* After a game is accepted or resumed, raises the second setup character, follows its curved
 * departure path, and stops its sound. */
void fn_1_6324(OMOBJ *obj)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[1];
    HuVecF pos;

    if (work->state == 0) {
        Hu3DModelPosGet(obj->mdlId[0], &pos);
        pos.y += 5.0f;
        Hu3DModelPosSetV(obj->mdlId[0], &pos);
        if ((work->time += 1.0f) > work->duration) {
            Hu3DModelPosGet(obj->mdlId[0], &work->pos);
            work->state = 1;
            work->time = 0.0f;
            work->duration = 90.0f;
        }
    }
    if (work->state == 1) {
        fn_1_22A8(obj->mdlId[0], &work->pos, &work->controlPos,
            &work->endPos, work->time, work->duration);
        if ((work->time += 1.0f) > work->duration) {
            HuAudFXStop(lbl_1_bss_A6C[1]);
            work->state = 2;
        }
    }
    Hu3DModelPosGet(obj->mdlId[0], &pos);
    fn_1_464A0(5, &pos, 1, 6);
    if (work->state == 2) {
        obj->objFunc = NULL;
    }
}

/* When a game is accepted or resumed, sets fixed departure waypoints for the second setup
* character, overwriting all three position reads. The installed update raises the model and
* then captures the curve's starting position. */
void fn_1_68A4(void)
{
    LBL_1_BSS_1C8_ENTRY *work = &lbl_1_bss_1C8[1];
    OMOBJ *obj = lbl_1_bss_10;

    work->state = 0;
    work->time = 0.0f;
    work->duration = 20.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->pos);
    work->pos.x = 325.0f;
    work->pos.y = 0.0f;
    work->pos.z = 100.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->controlPos);
    work->controlPos.x = -100.0f;
    work->controlPos.y = 400.0f;
    work->controlPos.z = 200.0f;
    Hu3DModelPosGet(obj->mdlId[0], &work->endPos);
    work->endPos.x = -100.0f;
    work->endPos.y = 125.0f;
    work->endPos.z = -1000.0f;
    Hu3DMotionShiftSet(
        obj->mdlId[0], obj->mtnId[2], 0.0f, 0.0f, 0);
    lbl_1_bss_A6C[1] = HuAudFXPlay(MSM_SE_BRD00_144);
    obj->objFunc = fn_1_6324;
}

inline void fn_1_68A4(void);

/* Per-frame callback that moves the second character model into its menu position before looping
 * its idle motion. */
void fn_1_6A20(OMOBJ *obj)
{
    HuVecF path[2] = {
        { 100.0f, 0.0f, 0.0f },
        { 325.0f, 0.0f, 100.0f },
    };
    float time = obj->work[0];

    fn_1_1D54(
        obj->mdlId[0], &path[0], &path[1], time, 30.0f, -15.0f);
    Hu3DMotionSpeedSet(obj->mdlId[0], 1.5f);
    if (++obj->work[0] > 60) {
        Hu3DMotionSpeedSet(obj->mdlId[0], 1.0f);
        Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0],
            0.0f, 15.0f, HU3D_MOTATTR_LOOP);
        obj->objFunc = NULL;
    }
}

/* Accepts the second character's exit only after its model reaches the menu boundary, then starts
 * its exit motion. */
s32 fn_1_6FB8(void)
{
    OMOBJ *obj = lbl_1_bss_10;
    HuVecF pos;

    Hu3DModelPosGet(obj->mdlId[0], &pos);
    if (pos.x > 100.0f) {
        return FALSE;
    }
    obj->work[0] = 0;
    obj->objFunc = fn_1_6A20;
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[1], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    return TRUE;
}

inline s32 fn_1_6FB8(void);

/* Creates the second character and attached companion model when the character setup menu is
 * initialized. */
void fn_1_7064(OMOBJ *obj)
{
    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    obj->mdlId[0] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 86), HU_MEMNUM_OVL));
    obj->mdlId[1] = Hu3DModelCreate(
        HuDataReadNum(DATANUM(DATA_mdparty, 87), HU_MEMNUM_OVL));
    obj->mtnId[0] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 88), HU_MEMNUM_OVL));
    obj->mtnId[1] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 89), HU_MEMNUM_OVL));
    obj->mtnId[2] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 90), HU_MEMNUM_OVL));
    obj->mtnId[3] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 91), HU_MEMNUM_OVL));
    obj->mtnId[4] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 92), HU_MEMNUM_OVL));
    obj->mtnId[5] = Hu3DJointMotion(obj->mdlId[0],
        HuDataReadNum(DATANUM(DATA_mdparty, 93), HU_MEMNUM_OVL));
    Hu3DModelHookSet(obj->mdlId[0], lbl_1_data_DC6, obj->mdlId[1]);
    Hu3DModelPosSet(obj->mdlId[0], 100.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(obj->mdlId[0], 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(obj->mdlId[0], 1.25f, 1.25f, 1.25f);
    Hu3DModelLayerSet(obj->mdlId[0], 1);
    Hu3DMotionShiftSet(obj->mdlId[0], obj->mtnId[0], 0.0f, 0.0f,
        HU3D_MOTATTR_LOOP);
    Hu3DModelShadowSet(obj->mdlId[0]);
    obj->objFunc = NULL;
}

/* Releases the second character, its companion model, and motions when the menu object is
 * destroyed. */
void fn_1_72C4(OMOBJ *obj)
{
    if (obj) {
        Hu3DModelHookReset(obj->mdlId[1]);
        Hu3DMotionKill(obj->mtnId[0]);
        Hu3DMotionKill(obj->mtnId[1]);
        Hu3DMotionKill(obj->mtnId[2]);
        Hu3DMotionKill(obj->mtnId[3]);
        Hu3DMotionKill(obj->mtnId[4]);
        Hu3DMotionKill(obj->mtnId[5]);
        Hu3DModelKill(obj->mdlId[0]);
        Hu3DModelKill(obj->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj->objFunc = NULL;
}

inline void fn_1_72C4(OMOBJ *obj);

/* After setup is accepted, moves the four selected characters along their curved exit paths,
 * updates their effects, and hides each model when its path ends. */
void fn_1_737C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *desc;
    LBL_1_BSS_1C8_ENTRY *work;
    HuVecF pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        work = &lbl_1_bss_108[i];
        if (work->state == 1) {
            fn_1_22A8(obj->mdlId[desc->chrSel], &work->pos,
                &work->controlPos, &work->endPos, work->time,
                work->duration);
            if ((work->time += 1.0f) > work->duration) {
                work->state = 2;
                Hu3DModelAttrSet(
                    obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
            }
            Hu3DModelPosGet(obj->mdlId[desc->chrSel], &pos);
            if (desc->isComputer == 0) {
                fn_1_464A0(i, &pos, 1, desc->padNo);
            } else {
                fn_1_464A0(i, &pos, 1, 4);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        work = &lbl_1_bss_108[i];
        if (work->state != 2) {
            break;
        }
    }
    if (i == 4) {
        obj->objFunc = NULL;
    }
}

/* After setup is accepted, starts the four selected characters' curved exit paths and looping
 * movement motions. */
void fn_1_7900(void)
{
    LBL_1_BSS_9BC_ENTRY *desc;
    OMOBJ *obj = lbl_1_bss_14;
    LBL_1_BSS_1C8_ENTRY *work;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        work = &lbl_1_bss_108[i];
        work->state = 1;
        work->time = 0.0f;
        work->duration = 90.0f;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->pos);
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->controlPos);
        work->controlPos.x += 0.5f * work->controlPos.x;
        work->controlPos.y = 550.0f;
        work->controlPos.z = 100.0f;
        Hu3DModelPosGet(obj->mdlId[desc->chrSel], &work->endPos);
        work->endPos.y = 225.0f;
        work->endPos.z = -800.0f;
        Hu3DMotionShiftSet(obj->mdlId[desc->chrSel],
            obj->mtnId[(2 * desc->chrSel) + 1], 0.0f, 10.0f,
            HU3D_MOTATTR_LOOP);
    }
    HuAudFXPlay(MSM_SE_MENU_19);
    obj->objFunc = fn_1_737C;
}

inline void fn_1_7900(void);

/* Per-frame callback that scales the four selected characters into view after the menu transition
 * starts. */
void fn_1_7ABC(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *desc;
    float scale;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        scale = fn_1_1868(0.0f, 1.0f, obj->work[0], obj->work[1]);
        Hu3DModelScaleSet(
            obj->mdlId[desc->chrSel], scale, scale, scale);
    }
    if ((obj->work[0] += 1.0f) > obj->work[1]) {
        obj->objFunc = NULL;
    }
}

/* Restores the selected characters to their menu positions and starts their scale-in transition. */
void fn_1_7C88(void)
{
    OMOBJ *obj = lbl_1_bss_14;
    LBL_1_BSS_9BC_ENTRY *desc;
    HuVecF pos;
    s16 i;

    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        if (desc->isComputer != 0) {
            fn_1_45F3C(i, &pos, 4, 0);
        } else {
            fn_1_45F3C(i, &pos, desc->padNo, 0);
        }
    }
    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        pos.y -= 70.0f;
        Hu3DModelPosSetV(obj->mdlId[desc->chrSel], &pos);
        Hu3DModelScaleSet(obj->mdlId[desc->chrSel], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(obj->mdlId[desc->chrSel], HU3D_ATTR_DISPOFF);
    }
    obj->work[0] = 0;
    obj->work[1] = 15;
    obj->objFunc = fn_1_7ABC;
    HuPrcSleep(15);
    HuAudFXPlay(MSM_SE_MENU_18);
    for (i = 0, desc = &lbl_1_bss_9BC[i]; i < 4; i++, desc++) {
        Hu3DModelPosGet(lbl_1_bss_288[desc->chrSel].modelId, &pos);
        if (desc->isComputer != 0) {
            fn_1_45F3C(i, &pos, 4, 1);
        } else {
            fn_1_45F3C(i, &pos, desc->padNo, 1);
        }
    }
}

/* Creates the 11 character-selection models and attaches two motions to each. */
void fn_1_7ED4(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 11; i++) {
        obj->mdlId[i] =
            Hu3DModelCreateData(DATANUM(DATA_mdparty, 94) + i);
        obj->mtnId[2 * i] = Hu3DJointMotionData(
            obj->mdlId[i], DATANUM(DATA_mdparty, 105) + i);
        obj->mtnId[(2 * i) + 1] = Hu3DJointMotionData(
            obj->mdlId[i], DATANUM(DATA_mdparty, 116) + i);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_ATTR_DISPOFF);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[2 * i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowSet(obj->mdlId[i]);
    }
    obj->objFunc = NULL;
}

/* Releases all character selection models and motions when the selection display closes. */
void fn_1_8074(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 11; i++) {
            Hu3DMotionKill(obj->mtnId[2 * i]);
            Hu3DMotionKill(obj->mtnId[(2 * i) + 1]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_8074(OMOBJ *obj);

/* Shows current and maximum turns after a saved game loads for the continue prompt. */
void fn_1_8124(s16 currentTurn, s16 turnLimit)
{
    s16 digit = 0;

    if (currentTurn <= turnLimit) {
        HuSprGrpPosSet(lbl_1_bss_918[12], 288.0f, 280.0f);
        fn_1_47B4(lbl_1_bss_918[12], HUSPR_ATTR_DISPOFF);

        /* Sprite members 0-1 show the current turn; hide an unused tens digit. */
        digit = currentTurn / 10;
        HuSprBankSet(lbl_1_bss_918[12], 0, digit);
        if (digit == 0) {
            HuSprAttrSet(lbl_1_bss_918[12], 0, HUSPR_ATTR_DISPOFF);
        }
        digit = currentTurn % 10;
        HuSprBankSet(lbl_1_bss_918[12], 1, digit);

        /* Sprite members 2-3 show the turn limit, also omitting a leading zero. */
        digit = turnLimit / 10;
        HuSprBankSet(lbl_1_bss_918[12], 2, digit);
        if (digit == 0) {
            HuSprAttrSet(lbl_1_bss_918[12], 2, HUSPR_ATTR_DISPOFF);
        }
        digit = turnLimit % 10;
        HuSprBankSet(lbl_1_bss_918[12], 3, digit);
    }
}

void fn_1_8318(void)
{
    fn_1_4734(lbl_1_bss_918[12], HUSPR_ATTR_DISPOFF);
}

/* Per-frame callback that fades the menu sprite group out and hides it when the transition
 * finishes. */
void fn_1_8398(OMOBJ *obj)
{
    float alpha = fn_1_1868(1.0f, 0.0f, obj->work[0], 10.0f);

    HuSprGrpTPLvlSet(lbl_1_bss_918[0], alpha);
    if (++obj->work[0] > 10) {
        fn_1_4734(lbl_1_bss_918[0], HUSPR_ATTR_DISPOFF);
        obj->objFunc = NULL;
    }
}

/* Positions and reveals the menu sprite group when the overlay event is zero. */
void fn_1_852C(void)
{
    if (lbl_1_bss_28 == 0) {
        HuSprGrpPosSet(lbl_1_bss_918[0], 288.0f, 120.0f);
        fn_1_47B4(lbl_1_bss_918[0], HUSPR_ATTR_DISPOFF);
    }
}

inline void fn_1_852C(void);

/* Starts the menu sprite group's fade-out callback. */
void fn_1_85E8(void)
{
    OMOBJ *obj = lbl_1_bss_4;

    obj->work[0] = 0;
    obj->objFunc = fn_1_8398;
}

/* Creates the three menu scene models and starts their looping motions during menu setup. */
void fn_1_861C(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 3; i++) {
        obj->mdlId[i] = Hu3DModelCreateData(DATA_mdparty + i);
        obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[i]);
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[i],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelShadowMapSet(obj->mdlId[i]);
    }
    fn_1_852C();
    obj->objFunc = NULL;
}

/* Releases the three menu scene models and motions when their menu object is destroyed. */
void fn_1_87BC(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 3; i++) {
            Hu3DMotionKill(obj->mtnId[i]);
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_87BC(OMOBJ *obj);

/* Shows the selected board-name sprite and begins its fade-in. */
void fn_1_884C(s16 memberNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
    fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
    HuSprAttrReset(
        lbl_1_bss_918[1], memberNo, HUSPR_ATTR_DISPOFF);
    obj->work[2] = 1;
    obj->work[3] = 0;
}

inline void fn_1_884C(s16 memberNo);

/* Starts fading out the board-name sprite during a menu transition. */
void fn_1_8950(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[2] = 2;
    obj->work[3] = 0;
}

inline void fn_1_8950(void);

void fn_1_8980(void)
{
    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
}

inline void fn_1_8980(void);

/* Per-frame callback that fades the board-name sprite in or out when requested. */
void fn_1_89E0(OMOBJ *obj)
{
    float alpha;

    switch (obj->work[2]) {
        case 1:
            alpha = fn_1_1868(
                0.0f, 1.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
        case 2:
            alpha = fn_1_1868(
                1.0f, 0.0f, obj->work[3], 10.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], alpha);
            if (++obj->work[3] > 10) {
                obj->work[2] = 0;
            }
            break;
    }
}

/* Sets the rear model's two board-picture texture animations for the chosen board and starts
 * fading the front model out. */
void fn_1_8C28(s16 animNo)
{
    OMOBJ *obj = lbl_1_bss_8;

    Hu3DAnimAnimSet(
        obj->mtnId[2], lbl_1_bss_93C[(2 * animNo) + 3]);
    Hu3DAnimAnimSet(
        obj->mtnId[3], lbl_1_bss_93C[(2 * animNo) + 4]);
    obj->work[0] = 1;
    obj->work[1] = 0;
}

inline void fn_1_8C28(s16 animNo);

/* Starts fading the front board-picture model in and the board-name sprite out. */
void fn_1_8CCC(void)
{
    OMOBJ *obj = lbl_1_bss_8;

    obj->work[0] = 2;
    obj->work[1] = 0;
    fn_1_8950();
}

inline void fn_1_8CCC(void);

/* Rotates both board-picture models, fades the front model, and updates the board-name sprite
 * fade. */
void fn_1_8D20(OMOBJ *obj)
{
    HuVecF rot;
    float modelAlpha;
    s16 i;

    for (i = 0; i < 2; i++) {
        Hu3DModelRotGet(obj->mdlId[i], &rot);
        rot.z -= 0.1f;
        if (rot.z < 0.0f) {
            rot.z += 360.0f;
        }
        Hu3DModelRotSetV(obj->mdlId[i], &rot);
    }
    switch (obj->work[0]) {
        case 1:
            modelAlpha = fn_1_1868(
                1.0f, 0.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
        case 2:
            modelAlpha = fn_1_1868(
                0.0f, 1.0f, obj->work[1], 30.0f);
            Hu3DModelTPLvlSet(obj->mdlId[0], modelAlpha);
            if (++obj->work[1] > 30) {
                obj->work[0] = 0;
            }
            break;
    }
    fn_1_89E0(obj);
}

/* Creates both board-picture models and their texture animations during Party Mode menu setup. */
void fn_1_9218(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 2; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdparty, 3), HU_MEMNUM_OVL,
                    HEAP_MODEL));
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelPosSet(
            obj->mdlId[i], 0.0f, -100.0f, -850.0f - i);
        Hu3DModelRotSet(obj->mdlId[i], 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(obj->mdlId[i], HU3D_MOTATTR_PAUSE);
        Hu3DModelLayerSet(obj->mdlId[i], 0);
        obj->mtnId[2 * i] = Hu3DAnimCreate(
            lbl_1_bss_93C[1], obj->mdlId[i], lbl_1_data_DD8);
        obj->mtnId[(2 * i) + 1] = Hu3DAnimCreate(
            lbl_1_bss_93C[2], obj->mdlId[i], lbl_1_data_DDF);
    }
    HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
    HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
    obj->objFunc = fn_1_8D20;
}

/* Releases all four board-picture texture animations and both models during menu cleanup. */
void fn_1_945C(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        for (i = 0; i < 2; i++) {
            Hu3DAnimKill(obj->mtnId[2 * i]);
            Hu3DAnimKill(obj->mtnId[(2 * i) + 1]);
            obj->mtnId[2 * i] = -1;
            obj->mtnId[(2 * i) + 1] = -1;
        }
        for (i = 1; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_945C(OMOBJ *obj);

/* Returns the matchup message for two character choices, independent of their order. */
u32 fn_1_9558(s16 charNo1, s16 charNo2)
{
    s8 pair[55][2] = {
        { 0, 1 }, { 0, 2 }, { 0, 3 }, { 0, 4 }, { 0, 5 },
        { 0, 6 }, { 0, 7 }, { 0, 8 }, { 0, 9 }, { 0, 10 },
        { 1, 2 }, { 1, 3 }, { 1, 4 }, { 1, 5 }, { 1, 6 },
        { 1, 7 }, { 1, 8 }, { 1, 9 }, { 1, 10 }, { 2, 3 },
        { 2, 4 }, { 2, 5 }, { 2, 6 }, { 2, 7 }, { 2, 8 },
        { 2, 9 }, { 2, 10 }, { 3, 4 }, { 3, 5 }, { 3, 6 },
        { 3, 7 }, { 3, 8 }, { 3, 9 }, { 3, 10 }, { 4, 5 },
        { 4, 6 }, { 4, 7 }, { 4, 8 }, { 4, 9 }, { 4, 10 },
        { 5, 6 }, { 5, 7 }, { 5, 8 }, { 5, 9 }, { 5, 10 },
        { 6, 7 }, { 6, 8 }, { 6, 9 }, { 6, 10 }, { 7, 8 },
        { 7, 9 }, { 7, 10 }, { 8, 9 }, { 8, 10 }, { 9, 10 },
    };
    s8 i;

    for (i = 0; i < 55; i++) {
        if ((charNo1 == pair[i][0] && charNo2 == pair[i][1])
            || (charNo1 == pair[i][1] && charNo2 == pair[i][0])) {
            break;
        }
    }
    if (i == 55) {
        return MDPARTY_MSG_NO_CHARACTER_PAIR;
    }
    return MDPARTY_MSG_CHARACTER_PAIR_BASE + i;
}

inline u32 fn_1_9558(s16 charNo1, s16 charNo2);

/* Sets a message window to show its supplied centered text immediately. */
void fn_1_9658(s16 winNo, u32 messNum)
{
    HuWinAttrSet(lbl_1_bss_104[winNo], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_104[winNo], messNum);
    HuWinMesSpeedSet(lbl_1_bss_104[winNo], 0);
}

inline void fn_1_9658(s16 winNo, u32 messNum);

/* Displays both character message windows and shows the related prompt sprite. */
void fn_1_96E4(u32 messNum1, u32 messNum2)
{
    HuWinDispOn(lbl_1_bss_104[0]);
    HuWinDispOn(lbl_1_bss_104[1]);
    fn_1_9658(0, messNum1);
    fn_1_9658(1, messNum2);
    fn_1_47B4(lbl_1_bss_918[13], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_96E4(u32 messNum1, u32 messNum2);

/* Hides both character message windows and the related prompt sprite. */
void fn_1_9804(void)
{
    HuWinDispOff(lbl_1_bss_104[0]);
    HuWinDispOff(lbl_1_bss_104[1]);
    fn_1_4734(lbl_1_bss_918[13], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_9804(void);

/* Creates and positions the two message windows beside the character portraits. */
void fn_1_98A4(void)
{
    HuVecF pos[2] = {
        { 150.0f, 120.0f, 0.0f },
        { 426.0f, 120.0f, 0.0f },
    };
    s16 i;

    for (i = 0; i < 2; i++) {
        lbl_1_bss_104[i] = HuWinExCreateFrame(
            pos[i].x - 124.0f, 6.0f + pos[i].y, 240, 42, -1, 0);
        HuWinDispOff(lbl_1_bss_104[i]);
        HuWinBGTPLvlSet(lbl_1_bss_104[i], 0.0f);
        HuWinPriSet(lbl_1_bss_104[i], 0);
        HuSprPosSet(lbl_1_bss_918[13], i, pos[i].x, pos[i].y);
    }
}

inline void fn_1_98A4(void);

/* Destroys both character message windows when their menu display closes. */
void fn_1_9A24(void)
{
    s16 i;

    for (i = 0; i < 2; i++) {
        HuWinExKill(lbl_1_bss_104[i]);
    }
}

inline void fn_1_9A24(void);

/* Reveals a player's roster marker and selects its marker bank during character setup. */
void fn_1_9A7C(s16 playerNo, s16 bank)
{
    lbl_1_bss_D4[playerNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[playerNo + 4], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[playerNo + 4], 0, bank);
}

inline void fn_1_9A7C(s16 playerNo, s16 bank);

/* Hides a player's character marker after the player backs out of a choice. */
void fn_1_9B68(s16 playerNo)
{
    lbl_1_bss_D4[playerNo].x = 0.0f;
}

/* Changes the bank shown in a player's character marker during character selection. */
void fn_1_9B90(s16 playerNo, s16 bankNo)
{
    HuSprBankSet(lbl_1_bss_918[playerNo + 4], 0, bankNo);
}

inline void fn_1_9B90(s16 playerNo, s16 bankNo);

/* Reveals a player's handicap marker and selects its displayed value during handicap setup. */
void fn_1_9BDC(s16 playerNo, s16 bank)
{
    lbl_1_bss_A4[playerNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[playerNo + 8], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[playerNo + 8], 2, bank);
}

inline void fn_1_9BDC(s16 playerNo, s16 bank);

/* Hides the selected player's handicap marker when handicap setup closes. */
void fn_1_9CC8(s16 playerNo)
{
    lbl_1_bss_A4[playerNo].x = 0.0f;
}

/* Updates the handicap number displayed beside a player during handicap setup. */
void fn_1_9CF0(s16 playerNo, s16 handicap)
{
    HuSprBankSet(lbl_1_bss_918[playerNo + 8], 2, handicap);
}

/* Reveals a team's handicap marker and selects its displayed value during team setup. */
void fn_1_9D3C(s16 teamNo, s16 bank)
{
    lbl_1_bss_8C[teamNo].x = 0.85f;
    fn_1_47B4(
        lbl_1_bss_918[teamNo + 14], HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[teamNo + 14], 2, bank);
}

inline void fn_1_9D3C(s16 teamNo, s16 bank);

/* Hides a team's handicap marker when team handicap setup closes. */
void fn_1_9E28(s16 teamNo)
{
    lbl_1_bss_8C[teamNo].x = 0.0f;
}

/* Updates a team's displayed handicap during team handicap setup. */
void fn_1_9E50(s16 teamNo, s16 handicap)
{
    HuSprBankSet(lbl_1_bss_918[teamNo + 14], 2, handicap);
}

inline void fn_1_9E50(s16 teamNo, s16 handicap);

void fn_1_9E9C(void)
{
    lbl_1_bss_80.x = 0.5f;
    fn_1_47B4(lbl_1_bss_918[16], HUSPR_ATTR_DISPOFF);
}

inline void fn_1_9E9C(void);

void fn_1_9F34(void)
{
    lbl_1_bss_80.x = 0.0f;
}

inline void fn_1_9F34(void);

/* Per-frame callback tracks each selected character with roster and handicap markers. The pairing
 * indicator uses player slot 2's height and depth after the team loop, with world x forced to zero.
 */
void fn_1_9F50(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos3D;
    HuVecF pos2D;
    s16 i;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 4],
            pos2D.x - 18.0f, (s16)(pos2D.y - 23.0f));
        HuSprAttrReset(
            lbl_1_bss_918[i + 4], 0, HUSPR_ATTR_LINEAR);
        lbl_1_bss_D4[i].y = fn_1_160C(
            lbl_1_bss_D4[i].y, lbl_1_bss_D4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 4],
            lbl_1_bss_D4[i].y, lbl_1_bss_D4[i].y);

        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 8],
            pos2D.x + 18.0f, pos2D.y + 23.0f);
        lbl_1_bss_A4[i].y = fn_1_160C(
            lbl_1_bss_A4[i].y, lbl_1_bss_A4[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 8],
            lbl_1_bss_A4[i].y, lbl_1_bss_A4[i].y);
    }

    for (i = 0; i < 2; i++) {
        Hu3DModelPosGet(
            lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
            &pos3D);
        pos3D.x = lbl_1_data_8A4[i + 61].x;
        Hu3D3Dto2D(&pos3D, 1, &pos2D);
        HuSprGrpPosSet(lbl_1_bss_918[i + 14],
            pos2D.x, pos2D.y + 23.0f);
        lbl_1_bss_8C[i].y = fn_1_160C(
            lbl_1_bss_8C[i].y, lbl_1_bss_8C[i].x, 3.0f);
        HuSprGrpScaleSet(lbl_1_bss_918[i + 14],
            lbl_1_bss_8C[i].y, lbl_1_bss_8C[i].y);
    }

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[i].chrSel].modelId,
        &pos3D);
    pos3D.x = 0.0f;
    Hu3D3Dto2D(&pos3D, 1, &pos2D);
    HuSprGrpPosSet(lbl_1_bss_918[16], pos2D.x, pos2D.y);
    lbl_1_bss_80.y = fn_1_160C(
        lbl_1_bss_80.y, lbl_1_bss_80.x, 3.0f);
    HuSprGrpScaleSet(
        lbl_1_bss_918[16], lbl_1_bss_80.y, lbl_1_bss_80.y);
}

/* Menu object setup clears roster and handicap indicator scales, creates the pairing windows,
 * and installs the per-frame indicator tracking callback. */
void fn_1_A5D8(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        lbl_1_bss_D4[i].x = lbl_1_bss_D4[i].y = 0.0f;
        lbl_1_bss_A4[i].x = lbl_1_bss_A4[i].y = 0.0f;
    }
    for (i = 0; i < 2; i++) {
        lbl_1_bss_8C[i].x = lbl_1_bss_8C[i].y = 0.0f;
    }
    lbl_1_bss_80.x = lbl_1_bss_80.y = 0.0f;
    obj->objFunc = fn_1_9F50;
    fn_1_98A4();
}

void fn_1_A880(void)
{
    fn_1_9A24();
}

inline void fn_1_A880(void);

/* Selection callbacks position one cursor sprite beside a world point using camera 0; a null point
 * places it at the supplied screen offsets alone. */
void fn_1_A8D8(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset)
{
    HuVecF pos2D = { 0.0f, 0.0f, 0.0f };

    if (pos3D) {
        Hu3D3Dto2D(pos3D, 1, &pos2D);
    }
    HuSprPosSet(lbl_1_bss_918[2], memberNo,
        pos2D.x + xOffset, pos2D.y + yOffset);
}

inline void fn_1_A8D8(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

/* Selection steps show and activate a cursor at its model position, starting its scale at 2. */
void fn_1_A970(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset)
{
    fn_1_A8D8(memberNo, pos3D, xOffset, yOffset);
    HuSprScaleSet(lbl_1_bss_918[2], memberNo, 2.0f, 2.0f);
    HuSprAttrReset(lbl_1_bss_918[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_bss_38[memberNo] = 2.0f;
    lbl_1_data_DE6[memberNo] = 1;
}

inline void fn_1_A970(
    s16 memberNo, HuVecF *pos3D, float xOffset, float yOffset);

void fn_1_AA94(s16 memberNo)
{
    HuSprAttrSet(lbl_1_bss_918[2], memberNo, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[memberNo] = 0;
}

inline void fn_1_AA94(s16 memberNo);

/* Resets the cursor's scale target to 2 even when inactive; plays the move sound only for an
 * active member. */
void fn_1_AAF0(s16 memberNo)
{
    lbl_1_bss_38[memberNo] = 2.0f;
    if (lbl_1_data_DE6[memberNo] == 1) {
        HuAudFXPlay(MSM_SE_CMN_01);
    }
}

inline void fn_1_AAF0(s16 memberNo);

/* Eases the six character-select cursor sprites toward their normal size each menu frame. */
void fn_1_AB64(void)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        lbl_1_bss_38[i] =
            fn_1_160C(lbl_1_bss_38[i], 1.0f, 5.0f);
        HuSprScaleSet(lbl_1_bss_918[2], i,
            lbl_1_bss_38[i], lbl_1_bss_38[i]);
    }
}

inline void fn_1_AB64(void);

/* Hides all six selection cursors and clears their active markers during menu reset. */
void fn_1_AC88(void)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        fn_1_AA94(i);
        lbl_1_data_DE6[i] = 0;
    }
}

/* Stores the offset target used to keep a player's character cursor beside its model. */
void fn_1_AD14(s16 modelNo, HuVecF *pos)
{
    lbl_1_bss_50[modelNo].x = pos->x - 50.0f;
    lbl_1_bss_50[modelNo].y = 50.0f + pos->y;
    lbl_1_bss_50[modelNo].z = 50.0f + pos->z;
}

inline void fn_1_AD14(s16 modelNo, HuVecF *pos);

/* Positions and reveals a player's marker model and sprite at the supplied character position. */
void fn_1_AD9C(s32 modelNo, HuVecF *pos, s16 bank)
{
    OMOBJ *obj = lbl_1_bss_1C.obj;

    Hu3DModelPosSet(
        obj->mdlId[(s16)modelNo], pos->x, pos->y, pos->z);
    Hu3DModelRotSet(
        obj->mdlId[(s16)modelNo], 0.0f, 0.0f, 180.0f);
    Hu3DModelScaleSet(
        obj->mdlId[(s16)modelNo], 1.0f, 1.0f, 1.0f);
    fn_1_AD14(modelNo, pos);
    Hu3DModelAttrReset(
        obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_918[3], modelNo, 0.8f, 0.8f);
    HuSprAttrReset(lbl_1_bss_918[3], modelNo, HUSPR_ATTR_DISPOFF);
    HuSprBankSet(lbl_1_bss_918[3], modelNo, bank);
}

inline void fn_1_AD9C(s32 modelNo, HuVecF *pos, s16 bank);

/* Hides a player's marker model and marker sprite while a choice is not active. */
void fn_1_AF70(s32 modelNo)
{
    OMOBJ *obj = lbl_1_bss_1C.obj;

    Hu3DModelAttrSet(obj->mdlId[(s16)modelNo], HU3D_ATTR_DISPOFF);
    HuSprScaleSet(lbl_1_bss_918[3], modelNo, 0.8f, 0.8f);
    HuSprAttrSet(lbl_1_bss_918[3], modelNo, HUSPR_ATTR_DISPOFF);
}

inline void fn_1_AF70(s32 modelNo);

/* Updates the four player marker models and their sprites during selection. */
void fn_1_B008(OMOBJ *obj)
{
    HuVecF modelVector;
    HuVecF pos2D;
    s16 i;

    for (i = 0; i < 4; i++) {
        Hu3DModelPosGet(obj->mdlId[i], &modelVector);
        fn_1_163C(&modelVector, &lbl_1_bss_50[i], 3.0f);
        Hu3DModelPosSetV(obj->mdlId[i], &modelVector);
        Hu3D3Dto2D(&modelVector, 1, &pos2D);
        HuSprPosSet(lbl_1_bss_918[3], (s16)i, pos2D.x, pos2D.y);
        HuSprScaleSet(lbl_1_bss_918[3], (s16)i, 0.8f, 0.8f);

        Hu3DModelRotGet(obj->mdlId[i], &modelVector);
        modelVector.z = fn_1_160C(modelVector.z, 0.0f, 3.0f);
        Hu3DModelRotSetV(obj->mdlId[i], &modelVector);

        Hu3DModelScaleGet(obj->mdlId[i], &modelVector);
        modelVector.x = modelVector.y = modelVector.z =
            fn_1_160C(modelVector.x, 0.4f, 3.0f);
        Hu3DModelScaleSetV(obj->mdlId[i], &modelVector);
    }
    fn_1_AB64();
}

/* Creates the shared player marker models and starts their per-frame position update. */
void fn_1_B4C8(OMOBJ *obj)
{
    s16 i;

    omSetStatBit(obj, OM_STAT_MODELPAUSE);
    for (i = 0; i < 4; i++) {
        if (i == 0) {
            obj->mdlId[i] = Hu3DModelCreate(
                HuDataSelHeapReadNum(
                    DATANUM(DATA_mdparty, 26), HU_MEMNUM_OVL,
                    HEAP_MODEL));
            obj->mtnId[i] = Hu3DMotionIDGet(obj->mdlId[0]);
        } else {
            obj->mdlId[i] = Hu3DModelLink(obj->mdlId[0]);
        }
        Hu3DModelLayerSet(obj->mdlId[i], 1);
        Hu3DMotionShiftSet(obj->mdlId[i], obj->mtnId[0],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        fn_1_AF70(i);
    }
    fn_1_AC88();
    obj->objFunc = fn_1_B008;
}

/* Releases the shared marker motion and models when the player markers are removed. */
void fn_1_B6C0(OMOBJ *obj)
{
    s16 i;

    if (obj) {
        Hu3DMotionKill(obj->mtnId[0]);
        for (i = 3; i >= 0; i--) {
            Hu3DModelKill(obj->mdlId[i]);
        }
        omDelObjEx(lbl_1_bss_0, obj);
    }
    obj = NULL;
}

inline void fn_1_B6C0(OMOBJ *obj);

/* Routes character-selection navigation around characters already chosen by another player. */
s16 fn_1_B748(s16 currentCharacterNo, s16 directionNo,
    s16 alternateDirectionNo, s16 fallbackDirectionNo,
    LBL_1_DATA_E12_ENTRY *characterChoices, s16 characterCount)
{
    s16 result;
    s16 j;
    s16 i;
    s16 k;
    s16 l;

    if (directionNo == -1) {
        return -1;
    }
    result = characterChoices[currentCharacterNo].neighborCharacter[directionNo];
    if (result == -1) {
        s16 routed;

        if (alternateDirectionNo == -1) {
            routed = -1;
        } else {
            s16 next;

            next = characterChoices[currentCharacterNo].neighborCharacter[alternateDirectionNo];
            if (next == -1) {
                next = fn_1_B748(
                    currentCharacterNo, fallbackDirectionNo, -1, -1,
                    characterChoices, characterCount);
            }
            for (i = 0; i < characterCount; i++) {
                if (currentCharacterNo != i
                    && characterChoices[i].selectionState != -1
                    && next == i) {
                    next = fn_1_B748(
                        next, alternateDirectionNo,
                        fallbackDirectionNo, -1, characterChoices,
                        characterCount);
                    if (next == -1) {
                        next = fn_1_B748(
                            currentCharacterNo, fallbackDirectionNo,
                            -1, -1, characterChoices, characterCount);
                    }
                }
            }
            routed = next;
        }
        result = routed;
    }

    for (j = 0; j < characterCount; j++) {
        if (currentCharacterNo != j
            && characterChoices[j].selectionState != -1 && result == j) {
            s16 routed;

            /* The entry check already excludes -1, so this repeated guard cannot fire. */
            if (directionNo == -1) {
                routed = -1;
            } else {
                s16 next;

                next = characterChoices[result].neighborCharacter[directionNo];
                if (next == -1) {
                    next = fn_1_B748(
                        result, alternateDirectionNo,
                        fallbackDirectionNo, -1, characterChoices,
                        characterCount);
                }
                for (k = 0; k < characterCount; k++) {
                    if (result != k
                        && characterChoices[k].selectionState != -1
                        && next == k) {
                        next = fn_1_B748(
                            next, directionNo, alternateDirectionNo,
                            fallbackDirectionNo, characterChoices,
                            characterCount);
                        if (next == -1) {
                            next = fn_1_B748(
                                result, alternateDirectionNo,
                                fallbackDirectionNo, -1, characterChoices,
                                characterCount);
                        }
                    }
                }
                routed = next;
            }
            result = routed;
            if (result == -1) {
                s16 fallback;

                if (alternateDirectionNo == -1) {
                    fallback = -1;
                } else {
                    s16 next;

                    next = characterChoices[currentCharacterNo]
                               .neighborCharacter[alternateDirectionNo];
                    if (next == -1) {
                        next = fn_1_B748(
                            currentCharacterNo, fallbackDirectionNo,
                            -1, -1, characterChoices, characterCount);
                    }
                    for (l = 0; l < characterCount; l++) {
                        if (currentCharacterNo != l
                            && characterChoices[l].selectionState != -1
                            && next == l) {
                            next = fn_1_B748(
                                next, alternateDirectionNo,
                                fallbackDirectionNo, -1, characterChoices,
                                characterCount);
                            if (next == -1) {
                                next = fn_1_B748(
                                    currentCharacterNo,
                                    fallbackDirectionNo, -1, -1,
                                    characterChoices, characterCount);
                            }
                        }
                    }
                    fallback = next;
                }
                result = fallback;
            }
        }
    }
    return result;
}

#pragma auto_inline off
/* Converts a twelve-way stick sector into an ordered set of neighboring-character checks. */
s16 fn_1_BB30(s16 currentCharacterNo,
    LBL_1_DATA_E12_ENTRY *characterChoices, s16 stickDirectionNo,
    s16 characterCount)
{
    s16 order[12][3] = {
        { 2, 1, 3 },
        { 6, 7, 5 },
        { 0, 7, 1 },
        { 4, 5, 3 },
        { 1, 2, 0 },
        { 3, 2, 4 },
        { 7, 6, 0 },
        { 5, 6, 4 },
        { 1, 0, 2 },
        { 7, 0, 6 },
        { 3, 4, 2 },
        { 5, 4, 6 },
    };

    return fn_1_B748(currentCharacterNo, order[stickDirectionNo][0],
        order[stickDirectionNo][1], order[stickDirectionNo][2],
        characterChoices, characterCount);
}
#pragma auto_inline reset

/* Reads a player's repeated stick input and advances the highlighted character choice. */
s16 fn_1_BBD8(s16 padNo, s16 *current,
    LBL_1_DATA_E12_ENTRY *entries, s16 count)
{
    s16 next = 0;
    s16 directionNo = 0;
    HuVecF direction = { 0.0f, 0.0f, 0.0f };

    if (HuPadDStkRep[padNo] & PAD_BUTTON_UP) {
        direction.y = 1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_DOWN) {
        direction.y = -1.0f;
    }
    if (HuPadDStkRep[padNo] & PAD_BUTTON_LEFT) {
        direction.x = -1.0f;
    } else if (HuPadDStkRep[padNo] & PAD_BUTTON_RIGHT) {
        direction.x = 1.0f;
    }
    if (HuAbs(direction.x) < 0.25f
        && HuAbs(direction.y) < 0.25f) {
        return FALSE;
    }

    direction.z = sqrtf(
        (direction.x * direction.x) + (direction.y * direction.y));
    direction.x /= direction.z;
    direction.y /= direction.z;
    if (HuAbs(direction.y) < 0.25f) {
        if (direction.x > 0.0f) {
            directionNo = 0;
        } else {
            directionNo = 1;
        }
    } else if (HuAbs(direction.x) < 0.25f) {
        if (direction.y > 0.0f) {
            directionNo = 2;
        } else {
            directionNo = 3;
        }
    } else if (HuAbs(direction.x) > HuAbs(direction.y)) {
        if (direction.x > 0.0f) {
            if (direction.y > 0.0f) {
                directionNo = 4;
            } else {
                directionNo = 5;
            }
        } else if (direction.y > 0.0f) {
            directionNo = 6;
        } else {
            directionNo = 7;
        }
    } else if (direction.y > 0.0f) {
        if (direction.x > 0.0f) {
            directionNo = 8;
        } else {
            directionNo = 9;
        }
    } else if (direction.x > 0.0f) {
        directionNo = 10;
    } else {
        directionNo = 11;
    }

    if ((next = fn_1_BB30(*current, entries, directionNo, count)) != -1) {
        entries[*current].selectionState = -1;
        *current = next;
        entries[*current].selectionState = 1;
        HuAudFXPlay(MSM_SE_CMN_01);
        return TRUE;
    }
    return FALSE;
}

/* Assigns animation data to one display-model slot and resets its bank to 0; a bank other than
 * -1 overrides that reset. */
void fn_1_C158(s16 modelNo, s16 animNo, s16 dataNo, s16 bank)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];

    Hu3DAnimAnimSet(entry->animId[animNo], lbl_1_bss_800[dataNo]);
    if (bank != -1) {
        Hu3DAnimBankSet(entry->animId[animNo], bank);
    }
}

inline void fn_1_C158(s16 modelNo, s16 animNo, s16 dataNo, s16 bank);

/* Assigns the same animation data to all four slots of one Party Mode display model. */
void fn_1_C200(s16 modelNo, s16 dataNo)
{
    s16 i;

    for (i = 0; i < 4; i++) {
        fn_1_C158(modelNo, i, dataNo, -1);
    }
}

/* Menu transition callbacks reveal a display model at the supplied point, spinning it into view
 * while growing its horizontal and vertical scale; depth scale stays 1 after initialization. */
void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(MSM_SE_CMN_53);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosSet(entry->modelId, pos->x, pos->y, pos->z);
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    } else {
        lbl_1_bss_34 = 0;
    }
    value = fn_1_1868(-540.0f, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_1868(0.0f, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

inline void fn_1_C28C(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

/* Spins a display model away and shrinks it during a menu transition. */
void fn_1_C698(s16 modelNo, s16 time, s16 duration)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float value;

    if (time == 0) {
        if (lbl_1_bss_34 == 0) {
            HuAudFXPlay(MSM_SE_CMN_54);
            lbl_1_bss_34 = 1;
        }
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    } else {
        lbl_1_bss_34 = 0;
    }
    value = fn_1_1868(
        entry->rot.y, entry->rot.y - 540.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
    value = fn_1_1868(entry->scale.x, 0.0f, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    if (time <= duration) {
        Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
    }
}

inline void fn_1_C698(s16 modelNo, s16 time, s16 duration);

/* Character, board, and settings callbacks interpolate a display model's position and x/y scale
 * toward the supplied targets; both scale axes start from captured x scale and z scale stays 1. */
void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    HuVecF modelPos;
    float value;

    if (time == 0) {
        Hu3DModelPosGet(entry->modelId, &entry->pos);
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        Hu3DModelScaleGet(entry->modelId, &entry->scale);
    }
    modelPos.x = fn_1_1868(entry->pos.x, pos->x, time, duration);
    modelPos.y = fn_1_1868(entry->pos.y, pos->y, time, duration);
    modelPos.z = fn_1_1868(entry->pos.z, pos->z, time, duration);
    Hu3DModelPosSet(entry->modelId, modelPos.x, modelPos.y, modelPos.z);
    value = fn_1_1868(entry->scale.x, scale, time, duration);
    Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
}

inline void fn_1_CA68(
    s16 modelNo, s16 time, s16 duration, HuVecF *pos, float scale);

/* Player-count and settings callbacks turn a display card by half a revolution, replace its texture
* when it reaches edge-on, and restore zero rotation at the transition endpoints. */
void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    HuVecF rot;

    if (time == 0 || time >= duration) {
        Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    }
    if (time >= duration) {
        fn_1_C200(modelNo, dataOffset + dataNo);
    } else {
        Hu3DModelRotGet(entry->modelId, &rot);
        rot.y = fn_1_1868(
            0.0f, 180.0f * direction, time, duration);
        if (((rot.y < 0.0f) ? -rot.y : rot.y) >= 90.0f) {
            fn_1_C200(modelNo, dataOffset + dataNo);
        }
        Hu3DModelRotSetV(entry->modelId, &rot);
    }
}

inline void fn_1_D058(s16 modelNo, s16 dataNo, s16 dataOffset, s16 time,
    s16 duration, s16 direction);

/* Turns a display model to zero yaw over the supplied frames, clearing pitch and roll. The angle
 * argument only chooses how the captured starting yaw wraps. */
void fn_1_D3E0(s16 modelNo, float angle, s16 time, s16 duration)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];
    float rotY;

    if (time == 0) {
        Hu3DModelRotGet(entry->modelId, &entry->rot);
        if (entry->rot.y - angle > 180.0f) {
            entry->rot.y -= 360.0f;
        } else if (entry->rot.y - angle < -180.0f) {
            entry->rot.y += 360.0f;
        }
    }
    rotY = fn_1_1868(entry->rot.y, 0.0f, time, duration);
    Hu3DModelRotSet(entry->modelId, 0.0f, rotY, 0.0f);
}

inline void fn_1_D3E0(
    s16 modelNo, float angle, s16 time, s16 duration);

/* Reports the current Party Mode menu transition state to its waiting process. */
void fn_1_D648(s16 transitionState)
{
    lbl_1_bss_24->mtnId[0] = transitionState;
}

s16 fn_1_D660(void)
{
    return lbl_1_bss_24->mtnId[0];
}

/* Reports whether the current Party Mode menu update has reached its ready frame. */
s16 fn_1_D678(void)
{
    OMOBJ *obj = lbl_1_bss_24;

    if (obj->work[0] >= 10) {
        return TRUE;
    }
    return FALSE;
}

inline s16 fn_1_D678(void);

/* Runs a menu callback and sleeps until that callback signals its transition is complete. */
void fn_1_D6B0(OMOBJ_FUNC callback)
{
    OMOBJ *obj = lbl_1_bss_24;

    lbl_1_bss_24->mtnId[0] = 0;
    obj->work[0] = 0;
    obj->work[1] = 0;
    obj->work[2] = 0;
    obj->work[3] = 0;
    obj->objFunc = callback;
    while (lbl_1_bss_24->mtnId[0] == 0) {
        HuPrcVSleep();
    }
}

inline void fn_1_D6B0(OMOBJ_FUNC callback);

/* Loads character-select animations, creates the model set, and starts the menu object. */
void fn_1_D748(void)
{
    char *bitmapName[4] = {
        lbl_1_data_DF2,
        lbl_1_data_DFA,
        lbl_1_data_E02,
        lbl_1_data_E0A,
    };
    s16 i;
    s16 j;

    for (i = 0; i < 41; i++) {
        lbl_1_bss_800[i] = HuSprAnimRead(
            HuDataSelHeapReadNum(
                lbl_1_data_800[i], HU_MEMNUM_OVL, HEAP_MODEL));
    }
    for (i = 0; i < 25; i++) {
        memset(&lbl_1_bss_288[i], 0, sizeof(LBL_1_BSS_288_ENTRY));
        if (i == 0) {
            lbl_1_bss_288[i].modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
                MDPARTY_DATA_CHARACTER_SELECT_MODEL, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            lbl_1_bss_288[i].modelId =
                Hu3DModelLink(lbl_1_bss_288[0].modelId);
        }
        for (j = 0; j < 4; j++) {
            lbl_1_bss_288[i].animId[j] = Hu3DAnimCreate(
                lbl_1_bss_800[0], lbl_1_bss_288[i].modelId,
                bitmapName[j]);
        }
        Hu3DModelLayerSet(lbl_1_bss_288[i].modelId, 1);
        Hu3DModelAttrSet(
            lbl_1_bss_288[i].modelId, HU3D_ATTR_DISPOFF);
    }
    lbl_1_bss_24 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
}

inline void fn_1_D748(void);

/* Marks the menu transition aborted, releases all 25 display-card models and their four texture
 * animations, and clears the saved menu object pointer. */
void fn_1_D978(void)
{
    s16 i;
    s16 j;

    if (lbl_1_bss_24) {
        lbl_1_bss_24->mtnId[0] = -1;
        for (i = 24; i >= 0; i--) {
            for (j = 0; j < 4; j++) {
                Hu3DAnimKill(lbl_1_bss_288[i].animId[j]);
            }
            Hu3DModelKill(lbl_1_bss_288[i].modelId);
        }
    }
    lbl_1_bss_24 = NULL;
}

inline void fn_1_D978(void);

/* Shows the saved board's name and four rule cards for the continue prompt, then updates its
 * board picture and signals that the entrance is complete. */
void fn_1_DA54(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_884C(lbl_1_bss_9F4.boardNo);
        fn_1_C200(21, lbl_1_bss_9F4.tagMode + 20);
        fn_1_C200(22, lbl_1_bss_9F4.turnCountChoice + 22);
        fn_1_C200(23, lbl_1_bss_9F4.minigamePack + 31);
        fn_1_C200(24, lbl_1_bss_9F4.bonusStarChoice + 36);
    }
    fn_1_C28C(21, obj->work[0], 10, &lbl_1_data_8A4[41], 1.5f);
    fn_1_C28C(22, obj->work[0], 10, &lbl_1_data_8A4[42], 1.5f);
    fn_1_C28C(23, obj->work[0], 10, &lbl_1_data_8A4[43], 1.5f);
    fn_1_C28C(24, obj->work[0], 10, &lbl_1_data_8A4[44], 1.5f);
    if (obj->work[0]++ > 10) {
        fn_1_8C28(lbl_1_bss_9F4.boardNo);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Closes the saved-game rule cards and fades its board name after continuing the saved game is
 * chosen. */
void fn_1_EB74(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_8950();
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Closes the saved-game rule cards and fades the front board-picture model in when new setup
 * is chosen. */
void fn_1_F838(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        fn_1_8CCC();
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handles the player-count selector and animates its four player markers each frame. */
void fn_1_10518(OMOBJ *obj)
{
    s16 minimum = 1;
    s16 count = 0;
    s16 i;

    for (i = 0; i < 4; i++) {
        if (HuPadStatGet(i) == 0) {
            count++;
        }
    }
    if (count < minimum) {
        count = minimum;
    }
    if (count >= lbl_1_bss_9F4.playerCount) {
        if (obj->work[0]++ > 10) {
            if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                lbl_1_bss_9F4.playerCount--;
                if (lbl_1_bss_9F4.playerCount < minimum) {
                    lbl_1_bss_9F4.playerCount = count;
                }
                obj->work[0] = 0;
                obj->work[1] = 0;
                fn_1_AAF0(4);
            } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                lbl_1_bss_9F4.playerCount++;
                if (lbl_1_bss_9F4.playerCount >= count + 1) {
                    lbl_1_bss_9F4.playerCount = minimum;
                }
                obj->work[0] = 0;
                obj->work[1] = 1;
                fn_1_AAF0(5);
            }
            if (count == minimum) {
                obj->work[0] = 12;
                if (obj->work[3] == 1) {
                    obj->work[3] = 0;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                }
            } else if (obj->work[3] == 0) {
                obj->work[3] = 1;
                fn_1_A970(4, &lbl_1_data_8A4[0], -180.0f, 0.0f);
                fn_1_A970(5, &lbl_1_data_8A4[0], 180.0f, 0.0f);
            }
        }
    } else {
        lbl_1_bss_9F4.playerCount = count;
        obj->work[0] = 0;
        obj->work[1] = 0;
    }
    if (obj->work[0] <= 10) {
        for (i = 0; i < 4; i++) {
            if (obj->work[1]) {
                if (i < lbl_1_bss_9F4.playerCount) {
                    fn_1_D058(i + 17, 18, 1,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(i + 17, 19, -1,
                        obj->work[0], 10, 1);
                }
            } else if (i < lbl_1_bss_9F4.playerCount) {
                fn_1_D058(i + 17, 18, 1,
                    obj->work[0], 10, -1);
            } else {
                fn_1_D058(i + 17, 19, -1,
                    obj->work[0], 10, -1);
            }
        }
    }
}

/* Resumes the player-count selector after the preceding menu transition completes. */
void fn_1_114C4(OMOBJ *obj)
{
    obj->work[0] = 10;
    obj->objFunc = fn_1_10518;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the two player-count arrows and ends their cursor update after selection. */
void fn_1_114F4(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Enters the player-count screen and initializes the human count from connected controllers,
* forcing at least one human slot even if no controller is connected. */
void fn_1_1158C(OMOBJ *obj)
{
    s16 minimum = 1;
    s16 i;

    /* Each update reveals the four models twice; first-frame texture selection runs between the two
     * passes. */
    for (i = 0; i < 4; i++) {
        fn_1_C28C(i + 17, obj->work[0], 10,
            &lbl_1_data_8A4[i + 33], 1.25f);
    }
    if (obj->work[0] == 0) {
        for (i = 0, lbl_1_bss_9F4.playerCount = 0; i < 4; i++) {
            if (HuPadStatGet(i) == 0) {
                lbl_1_bss_9F4.playerCount++;
            }
        }
        if (lbl_1_bss_9F4.playerCount < minimum) {
            lbl_1_bss_9F4.playerCount = minimum;
        }
        for (i = 0; i < 4; i++) {
            if (i < lbl_1_bss_9F4.playerCount) {
                fn_1_C200(i + 17, 19);
            } else {
                fn_1_C200(i + 17, 18);
            }
        }
    }
    for (i = 0; i < 4; i++) {
        fn_1_C28C(i + 17, obj->work[0], 10,
            &lbl_1_data_8A4[i + 33], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Spins and shrinks the four player-count models as the selector screen closes. */
void fn_1_11EAC(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    float value;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[(s16)(i + 17)];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(entry->scale.x, 0.0f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
        if (time <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Character-selection callbacks rotate the card highlighted by an unconfirmed player,
* including the locked character card when it is highlighted. */
void fn_1_12260(s16 modelNo)
{
    HuVecF rot;
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];

    Hu3DModelRotGet(entry->modelId, &rot);
    rot.y += 6.0f;
    if (rot.y > 360.0f) {
        rot.y -= 360.0f;
    }
    Hu3DModelRotSetV(entry->modelId, &rot);
}

/* Character-selection updates reset cards that are not highlighted by an unconfirmed player. */
void fn_1_122FC(s16 modelNo)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[modelNo];

    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
}

inline void fn_1_122FC(s16 modelNo);

/* Reads human players' character choices and confirms or cancels each choice per frame. */
void fn_1_12364(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 0) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[i]++ > 10) {
                    if (fn_1_BBD8(entry->padNo, &entry->chrSel,
                            lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[i] = 0;
                    } else if (HuPadBtnDown[entry->padNo]
                        & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            continue;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, entry->padNo);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14],
                            entry->padNo);
                    }
                }
            } else if (HuPadBtnDown[entry->padNo] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                entry->characterChoiceState = 0;
                fn_1_AD9C(i, &lbl_1_data_8A4[entry->chrSel + 14],
                    entry->padNo);
                fn_1_9B68(i);
                fn_1_C158((s16)entry->chrSel, 0,
                    (s16)entry->chrSel, 0);
                fn_1_C158((s16)entry->chrSel, 2,
                    (s16)entry->chrSel, 0);
            }
        }
    }
    for (i = 0; i < 11; i++) {
        for (j = 0, entry = lbl_1_bss_9BC; j < 4;
             j++, entry++) {
            if (entry->isComputer == 0 && entry->chrSel == i
                && entry->characterChoiceState == 0) {
                break;
            }
        }
        if (j == 4) {
            fn_1_122FC(i);
        } else {
            fn_1_12260(i);
        }
    }
}

/* Restores the human players' markers and resumes character selection after a prompt. */
void fn_1_12B18(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 0 && entry->characterChoiceState == 0) {
            lbl_1_data_E12[entry->chrSel].selectionState = 1;
            fn_1_AD9C(
                i, &lbl_1_data_8A4[entry->chrSel + 14], entry->padNo);
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_12364;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides unconfirmed human markers and finishes the current character-select prompt. */
void fn_1_12D7C(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 0 && entry->characterChoiceState == 0) {
            fn_1_AF70(i);
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Cancels the first player's choice and returns the menu process to its waiting state. */
void fn_1_12EC8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry = &lbl_1_bss_9BC[0];

    entry->characterChoiceState = 0;
    lbl_1_bss_D4[0].x = 0.0f;
    fn_1_C158((s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
    fn_1_C158((s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Handles computer-player character and difficulty choices from the menu's shared pad. */
void fn_1_12FD8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState != 2) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_BBD8(
                            0, &entry->chrSel, lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            break;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        /* Confirming a CPU character resets its difficulty to 1, including after
                         * back navigation. */
                        entry->computerDifficulty = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, entry->computerDifficulty + 5);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        Hu3DModelPosGet(
                            lbl_1_bss_288[entry->chrSel].modelId, &pos);
                        fn_1_A970(4, &pos, -40.0f, -45.0f);
                        fn_1_A970(5, &pos, 40.0f, -45.0f);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14], 4);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        lbl_1_data_E12[entry->chrSel].selectionState = -1;
                        fn_1_AF70(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9BC[j].isComputer == 1
                                && lbl_1_bss_9BC[j].characterChoiceState == 2) {
                                lbl_1_bss_9BC[j].characterChoiceState = 0;
                                lbl_1_data_E12[
                                    lbl_1_bss_9BC[j].chrSel]
                                    .selectionState = -1;
                                fn_1_9B68(j);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 0,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 2,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_13EC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            } else if (entry->characterChoiceState == 1) {
                if (obj->work[0]++ > 10) {
                    if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                        entry->computerDifficulty--;
                        if (lbl_1_bss_2C == 0) {
                            if (entry->computerDifficulty < 0) {
                                entry->computerDifficulty += 3;
                            }
                        } else {
                            if (entry->computerDifficulty < 0) {
                                entry->computerDifficulty += 4;
                            }
                        }
                        obj->work[0] = 0;
                        fn_1_AAF0(4);
                    } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                        entry->computerDifficulty++;
                        if (lbl_1_bss_2C == 0) {
                            if (entry->computerDifficulty > 2) {
                                entry->computerDifficulty -= 3;
                            }
                        } else if (entry->computerDifficulty > 3) {
                            entry->computerDifficulty -= 4;
                        }
                        obj->work[0] = 0;
                        fn_1_AAF0(5);
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        HuAudFXPlay(MSM_SE_CMN_02);
                        entry->characterChoiceState = 2;
                        fn_1_AA94(4);
                        fn_1_AA94(5);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9BC[j].isComputer == 1
                                && lbl_1_bss_9BC[j].characterChoiceState == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_13EC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        entry->characterChoiceState = 0;
                        fn_1_AA94(4);
                        fn_1_AA94(5);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 0);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 0);
                        fn_1_AD9C(
                            i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
                        fn_1_9B68(i);
                    }
                }
                fn_1_9B90(i, entry->computerDifficulty + 5);
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->characterChoiceState == 0) {
            fn_1_12260(i);
        } else {
            fn_1_122FC(i);
        }
    }
}

/* Prepares the first unfinished CPU slot's character cursor or difficulty arrows and resumes
* CPU input on initial entry, after CPU choices, and after back navigation. */
void fn_1_13EC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1) {
            if (entry->characterChoiceState == 0) {
                for (j = 0; j < 11; j++) {
                    if (lbl_1_data_E12[j].selectionState == -1) {
                        entry->chrSel = j;
                        lbl_1_data_E12[j].selectionState = 1;
                        break;
                    }
                }
                fn_1_AD9C(
                    i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
                break;
            } else if (entry->characterChoiceState == 1) {
                Hu3DModelPosGet(
                    lbl_1_bss_288[entry->chrSel].modelId, &pos);
                fn_1_A970(4, &pos, -40.0f, -45.0f);
                fn_1_A970(5, &pos, 40.0f, -45.0f);
                break;
            }
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_12FD8;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the current computer-player marker and completes its selection prompt. */
void fn_1_14344(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1) {
            if (entry->characterChoiceState == 0) {
                lbl_1_data_E12[entry->chrSel].selectionState = -1;
                fn_1_AF70(i);
                break;
            } else if (entry->characterChoiceState == 1) {
                fn_1_AA94(4);
                fn_1_AA94(5);
                break;
            }
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Handles computer-player character choices on the shared pad, forcing confirmed CPU difficulty
 * to 1. */
void fn_1_14510(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState != 1) {
            if (entry->characterChoiceState == 0) {
                if (obj->work[0]++ > 10) {
                    if (fn_1_BBD8(
                            0, &entry->chrSel, lbl_1_data_E12, 11)) {
                        fn_1_AD14(
                            i, &lbl_1_data_8A4[entry->chrSel + 14]);
                        obj->work[0] = 0;
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                        if (lbl_1_bss_2A == 0 && entry->chrSel == 10) {
                            HuAudFXPlay(MSM_SE_CMN_05);
                            break;
                        }
                        CharFXPlay(entry->chrSel, CHARVOICEID(14));
                        entry->characterChoiceState = 1;
                        entry->computerDifficulty = 1;
                        fn_1_AF70(i);
                        fn_1_9A7C(i, 4);
                        fn_1_C158(
                            (s16)entry->chrSel, 0, (s16)entry->chrSel, 1);
                        fn_1_C158(
                            (s16)entry->chrSel, 2, (s16)entry->chrSel, 1);
                        fn_1_45C40(i,
                            &lbl_1_data_8A4[entry->chrSel + 14], 4);
                        for (j = 0; j < 4; j++) {
                            if (lbl_1_bss_9BC[j].isComputer == 1
                                && lbl_1_bss_9BC[j].characterChoiceState == 0) {
                                break;
                            }
                        }
                        if (j != 4) {
                            obj->objFunc = fn_1_14CC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                        HuAudFXPlay(MSM_SE_CMN_04);
                        lbl_1_data_E12[entry->chrSel].selectionState = -1;
                        fn_1_AF70(i);
                        for (j = 3; j >= 0; j--) {
                            if (lbl_1_bss_9BC[j].isComputer == 1
                                && lbl_1_bss_9BC[j].characterChoiceState == 1) {
                                lbl_1_bss_9BC[j].characterChoiceState = 0;
                                lbl_1_data_E12[
                                    lbl_1_bss_9BC[j].chrSel]
                                    .selectionState = -1;
                                fn_1_9B68(j);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 0,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                fn_1_C158((s16)lbl_1_bss_9BC[j].chrSel, 2,
                                    (s16)lbl_1_bss_9BC[j].chrSel, 0);
                                break;
                            }
                        }
                        if (j != -1) {
                            obj->objFunc = fn_1_14CC0;
                        } else {
                            obj->objFunc = NULL;
                        }
                    }
                }
            }
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        if (entry->chrSel == i && entry->characterChoiceState == 0) {
            fn_1_12260(i);
        } else {
            fn_1_122FC(i);
        }
    }
}

/* Selects the next available character and resumes the computer-player prompt. */
void fn_1_14CC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState == 0) {
            for (j = 0; j < 11; j++) {
                if (lbl_1_data_E12[j].selectionState == -1) {
                    entry->chrSel = j;
                    lbl_1_data_E12[j].selectionState = 1;
                    break;
                }
            }
            fn_1_AD9C(
                i, &lbl_1_data_8A4[entry->chrSel + 14], 4);
            break;
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_14510;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the current computer-player cursor and ends that selection prompt. */
void fn_1_14F64(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0, entry = lbl_1_bss_9BC; i < 4; i++, entry++) {
        if (entry->isComputer == 1 && entry->characterChoiceState == 0) {
            lbl_1_data_E12[entry->chrSel].selectionState = -1;
            fn_1_AF70(i);
            break;
        }
    }
    for (i = 0; i < 11; i++) {
        fn_1_122FC(i);
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Resets player choices and brings the character models onto the selection screen. */
void fn_1_150D0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    OMOBJ *workObj;
    float value;
    s16 i;
    s16 j;
    s16 time;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[2] = 2;
        workObj->work[3] = 0;
        for (i = 0; i < 11; i++) {
            fn_1_C200(i, i);
            fn_1_C158(i, 1, i, 2);
            fn_1_C158(i, 3, i, 2);
            lbl_1_data_E12[i].selectionState = -1;
        }
        if (lbl_1_bss_2A == 0) {
            fn_1_C200(10, 11);
        }
        for (j = 0, player = lbl_1_bss_9BC; j < 4;
             j++, player++) {
            player->characterChoiceState = 0;
            player->unusedSetupValue = 0;
            player->computerDifficulty = 0;
            player->chrSel = j;
            player->padNo = j;
        }
    }
    for (i = 0; i < 11; i++) {
        time = obj->work[0];
        entry = &lbl_1_bss_288[i];
        if (time == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_53);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosSet(entry->modelId,
                lbl_1_data_8A4[i + 14].x,
                lbl_1_data_8A4[i + 14].y,
                lbl_1_data_8A4[i + 14].z);
            Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelScaleSet(entry->modelId, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        } else {
            lbl_1_bss_34 = 0;
        }
        value = fn_1_1868(-540.0f, 0.0f, time, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, value, 0.0f);
        value = fn_1_1868(0.0f, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Hides unchosen character cards, moves chosen ones to their player positions, then reveals the
 * selected board's name. */
void fn_1_156B4(OMOBJ *obj)
{
    s16 i;
    s16 j;

    for (i = 0; i < 11; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_1_bss_9BC[j].chrSel) {
                break;
            }
        }
        if (j == 4) {
            fn_1_C698(i, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        for (i = 0; i < 4; i++) {
            fn_1_CA68((s16)lbl_1_bss_9BC[i].chrSel,
                obj->work[0] - 10, 10, &lbl_1_data_8A4[i + 45], 1.25f);
        }
    }
    if (obj->work[0]++ > 20) {
        fn_1_884C(lbl_1_bss_9F4.boardNo);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Hides unchosen character cards, reveals the selected board's name, and moves chosen cards to
 * their next menu positions. */
void fn_1_160A0(OMOBJ *obj)
{
    s16 i;
    s16 j;

    for (i = 0; i < 11; i++) {
        for (j = 0; j < 4; j++) {
            if (i == lbl_1_bss_9BC[j].chrSel) {
                break;
            }
        }
        if (j == 4) {
            fn_1_C698(i, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        fn_1_884C(lbl_1_bss_9F4.boardNo);
        for (i = 0; i < 4; i++) {
            fn_1_CA68((s16)lbl_1_bss_9BC[i].chrSel,
                obj->work[0] - 10, 10, &lbl_1_data_8A4[i + 49], 1.25f);
        }
    }
    if (obj->work[0]++ > 20) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* fn_1_34D10 and fn_1_36264 run this transition through fn_1_D6B0 when returning to character
 * setup. */
void fn_1_16A8C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    float modelValue;
    s32 characterModelIndex;
    s16 i;
    s16 frame;

    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
        }
    }
    for (i = 0, player = lbl_1_bss_9BC; i < 4; i++, player++) {
        frame = obj->work[0];
        characterModelIndex = (s16)player->chrSel;
        entry = &lbl_1_bss_288[characterModelIndex];
        if (frame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        modelValue = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
        modelValue = fn_1_1868(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* On back navigation from character selection to player count, fades the board name in, clears
 * roster marker scales, and spins and shrinks all 11 character cards. */
void fn_1_16E84(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    OMOBJ *workObj;
    float modelValue;
    s16 boardIndex;
    s16 i;
    s16 frame;

    if (obj->work[0] == 0) {
        boardIndex = lbl_1_bss_9F4.boardNo;
        workObj = lbl_1_bss_8;
        HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
        fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
        HuSprAttrReset(
            lbl_1_bss_918[1], boardIndex, HUSPR_ATTR_DISPOFF);
        workObj->work[2] = 1;
        workObj->work[3] = 0;
        for (i = 0; i < 4; i++) {
            lbl_1_bss_D4[i].x = 0.0f;
        }
    }
    for (i = 0; i < 11; i++) {
        frame = obj->work[0];
        entry = &lbl_1_bss_288[i];
        if (frame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        modelValue = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
        modelValue = fn_1_1868(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Animates the highlighted board model while the Party Mode board picker is updated. */
void fn_1_1733C(s16 boardIndex)
{
    HuVecF modelVector;
    LBL_1_BSS_288_ENTRY *modelEntry = &lbl_1_bss_288[boardIndex + 11];

    Hu3DModelPosGet(modelEntry->modelId, &modelVector);
    fn_1_163C(&modelVector, &lbl_1_data_8A4[boardIndex + 7], 5.0f);
    Hu3DModelPosSetV(modelEntry->modelId, &modelVector);
    if (modelEntry->highlightSpinFrame++ > 30) {
        modelEntry->highlightSpinFrame = 35;
        Hu3DModelRotGet(modelEntry->modelId, &modelVector);
        modelVector.y += 3.0f;
        if (modelVector.y >= 360.0f) {
            modelVector.y -= 360.0f;
        }
        Hu3DModelRotSetV(modelEntry->modelId, &modelVector);
    }
    Hu3DModelScaleGet(modelEntry->modelId, &modelVector);
    modelVector.y = modelVector.x = fn_1_160C(modelVector.x, 2.25f, 5.0f);
    Hu3DModelScaleSetV(modelEntry->modelId, &modelVector);
    Hu3DModelLayerSet(modelEntry->modelId, 2);
}

/* Places a nonhighlighted board model in its selection slot and resets its transform. */
void fn_1_1765C(s16 boardIndex)
{
    LBL_1_BSS_288_ENTRY *entry = &lbl_1_bss_288[boardIndex + 11];

    Hu3DModelPosSetV(entry->modelId, &lbl_1_data_8A4[boardIndex + 1]);
    Hu3DModelRotSet(entry->modelId, 0.0f, 0.0f, 0.0f);
    Hu3DModelScaleSet(entry->modelId, 1.25f, 1.25f, 1.0f);
    entry->highlightSpinFrame = 0;
    Hu3DModelLayerSet(entry->modelId, 1);
}

/* Installed by fn_1_17E10; the menu object calls it each frame to move the Party Mode board
 * cursor. */
void fn_1_17730(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            OMOBJ *workObj;
            s16 boardIndex;

            lbl_1_bss_9F4.boardNo--;
            if (lbl_1_bss_9F4.boardNo < 0) {
                lbl_1_bss_9F4.boardNo += 6;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            boardIndex = lbl_1_bss_9F4.boardNo;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
            fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_918[1], boardIndex, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_38[0] = 2.0f;
            if (lbl_1_data_DE6[0] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            OMOBJ *workObj;
            s16 boardIndex;

            lbl_1_bss_9F4.boardNo++;
            if (lbl_1_bss_9F4.boardNo >= 6) {
                lbl_1_bss_9F4.boardNo -= 6;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            boardIndex = lbl_1_bss_9F4.boardNo;
            workObj = lbl_1_bss_8;
            HuSprGrpPosSet(lbl_1_bss_918[1], 288.0f, 60.0f);
            HuSprGrpTPLvlSet(lbl_1_bss_918[1], 0.0f);
            fn_1_4734(lbl_1_bss_918[1], HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(
                lbl_1_bss_918[1], boardIndex, HUSPR_ATTR_DISPOFF);
            workObj->work[2] = 1;
            workObj->work[3] = 0;
            lbl_1_bss_38[1] = 2.0f;
            if (lbl_1_data_DE6[1] == 1) {
                HuAudFXPlay(MSM_SE_CMN_01);
            }
        }
    }
    for (i = 0; i < 6; i++) {
        if (i == lbl_1_bss_9F4.boardNo) {
            fn_1_1733C(i);
        } else {
            fn_1_1765C(i);
        }
    }
}

/* fn_1_38A3C runs this callback through fn_1_D6B0 before accepting board input. */
void fn_1_17E10(OMOBJ *obj)
{
    s16 i;

    for (i = 0; i < 6; i++) {
        lbl_1_bss_288[i + 11].highlightSpinFrame = 0;
    }
    fn_1_A970(0, &lbl_1_data_8A4[0], -240.0f, 0.0f);
    fn_1_A970(1, &lbl_1_data_8A4[0], 240.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_17730;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* fn_1_38A3C runs this callback through fn_1_D6B0 after leaving board choice. */
void fn_1_1807C(OMOBJ *obj)
{
    s16 i;

    if (obj->work[0] == 0) {
        fn_1_AA94(0);
        fn_1_AA94(1);
    }
    for (i = 0; i < 6; i++) {
        lbl_1_bss_288[i + 11].highlightSpinFrame = 0;
        fn_1_D3E0(i + 11, 0.0f, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* fn_1_38A3C runs this callback through fn_1_D6B0 when entering board choice. */
void fn_1_18384(OMOBJ *obj)
{
    s16 i;

    /* The board picker always starts at board 0, including when returning from a later step. */
    lbl_1_bss_9F4.boardNo = 0;
    if (obj->work[0] == 0) {
        for (i = 0; i < 6; i++) {
            fn_1_C200(i + 11, i + 12);
            fn_1_C158(i + 11, 1, i + 12, 1);
            fn_1_C158(i + 11, 3, i + 12, 1);
            if (lbl_1_data_BEE[i] == 0) {
                fn_1_C158(i + 11, 2, 40, -1);
                fn_1_C158(i + 11, 3, 40, -1);
            }
        }
    }
    for (i = 0; i < 6; i++) {
        fn_1_C28C(
            i + 11, obj->work[0], 10, &lbl_1_data_8A4[i + 1], 1.25f);
    }
    if (obj->work[0]++ > 10) {
        Hu3DZClearLayerSet(2);
        fn_1_884C(lbl_1_bss_9F4.boardNo);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* fn_1_38A3C runs this callback through fn_1_D6B0 when returning to board choice. */
void fn_1_18A10(OMOBJ *obj)
{
    OMOBJ *workObj;
    OMOBJ *workObj2;

    if (obj->work[0] == 0) {
        workObj = lbl_1_bss_8;
        workObj->work[0] = 2;
        workObj->work[1] = 0;
        workObj2 = lbl_1_bss_8;
        workObj2->work[2] = 2;
        workObj2->work[3] = 0;
    }
    if (obj->work[0]++ > 30) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* fn_1_38A3C runs this callback through fn_1_D6B0 after the player accepts a board. */
void fn_1_18AA4(OMOBJ *obj)
{
    HuVecF rot;
    s16 i;
    s16 frame;

    for (i = 0; i < 6; i++) {
        if (i != lbl_1_bss_9F4.boardNo) {
            fn_1_C698(i + 11, obj->work[0], 10);
        }
    }
    if (obj->work[0] >= 10) {
        frame = obj->work[0] - 10;
        fn_1_CA68(lbl_1_bss_9F4.boardNo + 11, frame, 10,
            &lbl_1_data_8A4[13], 1.5f);
        Hu3DModelRotGet(
            lbl_1_bss_288[lbl_1_bss_9F4.boardNo + 11].modelId, &rot);
        rot.y += 54.0f;
        Hu3DModelRotSetV(
            lbl_1_bss_288[lbl_1_bss_9F4.boardNo + 11].modelId, &rot);
    }
    if (obj->work[0] == 20) {
        Hu3DLayerHookReset(2);
        fn_1_45A48(&lbl_1_data_8A4[13]);
        HuAudFXPlay(MSM_SE_MENU_15);
        fn_1_8C28(lbl_1_bss_9F4.boardNo);
        Hu3DModelAttrSet(
            lbl_1_bss_288[lbl_1_bss_9F4.boardNo + 11].modelId,
            HU3D_ATTR_DISPOFF);
    }
    if (obj->work[0]++ > 90) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Fades the board name out and shrinks all six board cards. The board picker references this
 * callback only in its unreachable result-1 branch. */
void fn_1_1948C(OMOBJ *obj)
{
    LBL_1_BSS_288_ENTRY *entry;
    s16 i;
    OMOBJ *selectionObj;
    float modelValue;
    s16 frame;

    if (obj->work[0] == 0) {
        selectionObj = lbl_1_bss_8;
        selectionObj->work[2] = 2;
        selectionObj->work[3] = 0;
    }
    for (i = 0; i < 6; i++) {
        frame = obj->work[0];
        entry = &lbl_1_bss_288[(s16)(i + 11)];
        if (frame == 0) {
            if (lbl_1_bss_34 == 0) {
                HuAudFXPlay(MSM_SE_CMN_54);
                lbl_1_bss_34 = 1;
            }
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        } else {
            lbl_1_bss_34 = 0;
        }
        modelValue = fn_1_1868(
            entry->rot.y, entry->rot.y - 540.0f, frame, 10.0f);
        Hu3DModelRotSet(entry->modelId, 0.0f, modelValue, 0.0f);
        modelValue = fn_1_1868(entry->scale.x, 0.0f, frame, 10.0f);
        Hu3DModelScaleSet(entry->modelId, modelValue, modelValue, 1.0f);
        if (frame <= 10) {
            Hu3DModelAttrReset(entry->modelId, HU3D_ATTR_DISPOFF);
        }
    }
    if (obj->work[0]++ > 10) {
        Hu3DLayerHookReset(2);
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Installed by fn_1_19D24; the menu object calls it each frame for player focus and handicap
 * input. */
void fn_1_19860(OMOBJ *obj)
{
    HuVecF characterPosition;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.selectedHandicapSlot--;
            if (lbl_1_bss_9F4.selectedHandicapSlot < 0) {
                lbl_1_bss_9F4.selectedHandicapSlot += 4;
            }
            obj->work[0] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.selectedHandicapSlot++;
            if (lbl_1_bss_9F4.selectedHandicapSlot > 3) {
                lbl_1_bss_9F4.selectedHandicapSlot -= 4;
            }
            obj->work[0] = 0;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap--;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap < 0) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap += 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(3);
            fn_1_9CF0(lbl_1_bss_9F4.selectedHandicapSlot,
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap++;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap > 9) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap -= 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(2);
            fn_1_9CF0(lbl_1_bss_9F4.selectedHandicapSlot,
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap);
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel]
            .modelId,
            &characterPosition);
    fn_1_A8D8(2, &characterPosition, 0.0f, -80.0f);
    fn_1_A8D8(3, &characterPosition, 0.0f, 80.0f);
}

/* Positions handicap arrows at the active character card and player-selection arrows around world
* x = 0, then resumes handicap input. fn_1_39C60 installs this during individual handicap setup. */
void fn_1_19D24(OMOBJ *obj)
{
    HuVecF selectedCharacterPosition;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel]
            .modelId,
        &selectedCharacterPosition);
    fn_1_A970(2, &selectedCharacterPosition, 0.0f, -80.0f);
    fn_1_A970(3, &selectedCharacterPosition, 0.0f, 80.0f);
    selectedCharacterPosition.x = 0.0f;
    fn_1_A970(4, &selectedCharacterPosition, -180.0f, 0.0f);
    fn_1_A970(5, &selectedCharacterPosition, 180.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_19860;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the four handicap and player-selection arrows when fn_1_39C60 ends the handicap step,
 * then lets that menu transition continue. */
void fn_1_1A124(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* On first entry to individual handicap setup, moves the four character cards into position and
 * resets each handicap to zero; fn_1_39C60 waits for the transition before taking input. */
void fn_1_1A20C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    LBL_1_BSS_288_ENTRY *characterModel;
    HuVecF targetPosition;
    float targetScale;
    s32 characterModelIndex;
    s16 handicapValue;
    s16 playerIndex;
    s16 animationFrame;

    lbl_1_bss_9F4.selectedHandicapSlot = 0;
    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        animationFrame = obj->work[0];
        characterModelIndex = (s16)playerSetup->chrSel;
        characterModel = &lbl_1_bss_288[characterModelIndex];
        if (animationFrame == 0) {
            Hu3DModelPosGet(characterModel->modelId, &characterModel->pos);
            Hu3DModelRotGet(characterModel->modelId, &characterModel->rot);
            Hu3DModelScaleGet(characterModel->modelId, &characterModel->scale);
        }
        targetPosition.x = fn_1_1868(characterModel->pos.x,
            lbl_1_data_8A4[playerIndex + 45].x, animationFrame, 10.0f);
        targetPosition.y = fn_1_1868(characterModel->pos.y,
            lbl_1_data_8A4[playerIndex + 45].y, animationFrame, 10.0f);
        targetPosition.z = fn_1_1868(characterModel->pos.z,
            lbl_1_data_8A4[playerIndex + 45].z, animationFrame, 10.0f);
        Hu3DModelPosSet(characterModel->modelId, targetPosition.x, targetPosition.y,
            targetPosition.z);
        targetScale = fn_1_1868(characterModel->scale.x, 1.25f, animationFrame, 10.0f);
        Hu3DModelScaleSet(characterModel->modelId, targetScale, targetScale, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_9BC[playerIndex].handicap = 0;
            handicapValue = lbl_1_bss_9BC[playerIndex].handicap;
            lbl_1_bss_A4[playerIndex].x = 0.85f;
            fn_1_47B4(
                lbl_1_bss_918[playerIndex + 8], HUSPR_ATTR_DISPOFF);
            HuSprBankSet(lbl_1_bss_918[playerIndex + 8], 2, handicapValue);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* When fn_1_39C60 returns to individual handicap setup, moves the four character cards back into
 * position without resetting the handicap displays. */
void fn_1_1A8B0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    LBL_1_BSS_288_ENTRY *characterModel;
    HuVecF targetPosition;
    float targetScale;
    s32 characterModelIndex;
    s16 playerIndex;
    s16 animationFrame;

    lbl_1_bss_9F4.selectedHandicapSlot = 0;
    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        animationFrame = obj->work[0];
        characterModelIndex = (s16)playerSetup->chrSel;
        characterModel = &lbl_1_bss_288[characterModelIndex];
        if (animationFrame == 0) {
            Hu3DModelPosGet(characterModel->modelId, &characterModel->pos);
            Hu3DModelRotGet(characterModel->modelId, &characterModel->rot);
            Hu3DModelScaleGet(characterModel->modelId, &characterModel->scale);
        }
        targetPosition.x = fn_1_1868(characterModel->pos.x,
            lbl_1_data_8A4[playerIndex + 45].x, animationFrame, 10.0f);
        targetPosition.y = fn_1_1868(characterModel->pos.y,
            lbl_1_data_8A4[playerIndex + 45].y, animationFrame, 10.0f);
        targetPosition.z = fn_1_1868(characterModel->pos.z,
            lbl_1_data_8A4[playerIndex + 45].z, animationFrame, 10.0f);
        Hu3DModelPosSet(characterModel->modelId, targetPosition.x, targetPosition.y,
            targetPosition.z);
        targetScale = fn_1_1868(characterModel->scale.x, 1.25f, animationFrame, 10.0f);
        Hu3DModelScaleSet(characterModel->modelId, targetScale, targetScale, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After individual handicaps are confirmed, moves the character cards into the next setup layout;
 * fn_1_39C60 waits for this frame callback to finish before continuing. */
void fn_1_1AE60(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    LBL_1_BSS_288_ENTRY *characterModel;
    HuVecF targetPosition;
    float targetScale;
    s32 characterModelIndex;
    s16 playerIndex;
    s16 animationFrame;

    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        animationFrame = obj->work[0];
        characterModelIndex = (s16)playerSetup->chrSel;
        characterModel = &lbl_1_bss_288[characterModelIndex];
        if (animationFrame == 0) {
            Hu3DModelPosGet(characterModel->modelId, &characterModel->pos);
            Hu3DModelRotGet(characterModel->modelId, &characterModel->rot);
            Hu3DModelScaleGet(characterModel->modelId, &characterModel->scale);
        }
        targetPosition.x = fn_1_1868(characterModel->pos.x,
            lbl_1_data_8A4[playerIndex + 29].x, animationFrame, 10.0f);
        targetPosition.y = fn_1_1868(characterModel->pos.y,
            lbl_1_data_8A4[playerIndex + 29].y, animationFrame, 10.0f);
        targetPosition.z = fn_1_1868(characterModel->pos.z,
            lbl_1_data_8A4[playerIndex + 29].z, animationFrame, 10.0f);
        Hu3DModelPosSet(characterModel->modelId, targetPosition.x, targetPosition.y,
            targetPosition.z);
        targetScale = fn_1_1868(characterModel->scale.x, 1.25f, animationFrame, 10.0f);
        Hu3DModelScaleSet(characterModel->modelId, targetScale, targetScale, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After individual handicap setup is canceled, clears the handicap display state and moves the
 * character cards into the return layout before fn_1_39C60 continues. */
void fn_1_1B400(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    LBL_1_BSS_288_ENTRY *characterModel;
    HuVecF targetPosition;
    float targetScale;
    s16 playerIndex;
    s16 animationFrame;
    s32 characterModelIndex;

    if (obj->work[0] == 0) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_A4[playerIndex].x = 0.0f;
        }
    }
    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        animationFrame = obj->work[0];
        characterModelIndex = (s16)playerSetup->chrSel;
        characterModel = &lbl_1_bss_288[characterModelIndex];
        if (animationFrame == 0) {
            Hu3DModelPosGet(characterModel->modelId, &characterModel->pos);
            Hu3DModelRotGet(characterModel->modelId, &characterModel->rot);
            Hu3DModelScaleGet(characterModel->modelId, &characterModel->scale);
        }
        targetPosition.x = fn_1_1868(characterModel->pos.x,
            lbl_1_data_8A4[playerIndex + 45].x, animationFrame, 10.0f);
        targetPosition.y = fn_1_1868(characterModel->pos.y,
            lbl_1_data_8A4[playerIndex + 45].y, animationFrame, 10.0f);
        targetPosition.z = fn_1_1868(characterModel->pos.z,
            lbl_1_data_8A4[playerIndex + 45].z, animationFrame, 10.0f);
        Hu3DModelPosSet(characterModel->modelId, targetPosition.x, targetPosition.y,
            targetPosition.z);
        targetScale = fn_1_1868(characterModel->scale.x, 1.25f, animationFrame, 10.0f);
        Hu3DModelScaleSet(characterModel->modelId, targetScale, targetScale, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* While fn_1_3BEA8 opens final setup confirmation, moves the rule and character cards into its
 * layout, using the team ordering when Tag Mode is active. */
void fn_1_1B9E8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    s16 playerIndex;

    fn_1_CA68(21, obj->work[0], 10,
        &lbl_1_data_8A4[41], 1.5f);
    fn_1_CA68(22, obj->work[0], 10,
        &lbl_1_data_8A4[42], 1.5f);
    fn_1_CA68(23, obj->work[0], 10,
        &lbl_1_data_8A4[43], 1.5f);
    fn_1_CA68(24, obj->work[0], 10,
        &lbl_1_data_8A4[44], 1.5f);
    if (lbl_1_bss_9F4.tagMode == 0) {
        for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
             playerIndex < 4; playerIndex++, playerSetup++) {
            fn_1_CA68((s16)playerSetup->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[playerIndex + 29], 1.25f);
        }
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerSetup = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][playerIndex]];
            fn_1_CA68((s16)playerSetup->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[playerIndex + 57], 1.25f);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After setup is confirmed, fades the board name out, clears roster and handicap markers, hides
* the rule and character cards, and restores the animated characters; fn_1_3BEA8 waits for
* completion. */
void fn_1_1D758(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    OMOBJ *animationObject;
    s16 playerIndex;

    if (obj->work[0] == 0) {
        animationObject = lbl_1_bss_8;
        animationObject->work[2] = 2;
        animationObject->work[3] = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_D4[playerIndex].x = 0.0f;
            lbl_1_bss_A4[playerIndex].x = 0.0f;
        }
        lbl_1_bss_8C[0].x = 0.0f;
        lbl_1_bss_8C[1].x = 0.0f;
        lbl_1_bss_80.x = 0.0f;
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        fn_1_C698((s16)playerSetup->chrSel, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        fn_1_7C88();
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* After restarting setup is chosen, fades the front board picture in and the board name out,
* clears roster and handicap markers, and hides the rule and character cards while fn_1_3BEA8
* waits for completion. */
void fn_1_1E7F8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    OMOBJ *animationObject;
    OMOBJ *secondaryAnimationObject;
    s16 playerIndex;

    if (obj->work[0] == 0) {
        animationObject = lbl_1_bss_8;
        animationObject->work[0] = 2;
        animationObject->work[1] = 0;
        secondaryAnimationObject = lbl_1_bss_8;
        secondaryAnimationObject->work[2] = 2;
        secondaryAnimationObject->work[3] = 0;
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            lbl_1_bss_D4[playerIndex].x = 0.0f;
            lbl_1_bss_A4[playerIndex].x = 0.0f;
        }
        lbl_1_bss_8C[0].x = 0.0f;
        lbl_1_bss_8C[1].x = 0.0f;
        lbl_1_bss_80.x = 0.0f;
    }
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
         playerIndex < 4; playerIndex++, playerSetup++) {
        fn_1_C698((s16)playerSetup->chrSel, obj->work[0], 10);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* When final confirmation returns to handicap setup, moves rule and character cards into the
 * return layout; fn_1_3BEA8 waits for this transition to finish. */
void fn_1_1F8B8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    s16 playerIndex;

    fn_1_CA68(21, obj->work[0], 10,
        &lbl_1_data_8A4[37], 1.0f);
    fn_1_CA68(22, obj->work[0], 10,
        &lbl_1_data_8A4[38], 1.0f);
    fn_1_CA68(23, obj->work[0], 10,
        &lbl_1_data_8A4[39], 1.0f);
    fn_1_CA68(24, obj->work[0], 10,
        &lbl_1_data_8A4[40], 1.0f);
    if (lbl_1_bss_9F4.tagMode == 0) {
        for (playerIndex = 0, playerSetup = lbl_1_bss_9BC;
             playerIndex < 4; playerIndex++, playerSetup++) {
            fn_1_CA68((s16)playerSetup->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[playerIndex + 45], 1.25f);
        }
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerSetup = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][playerIndex]];
            fn_1_CA68((s16)playerSetup->chrSel, obj->work[0], 10,
                &lbl_1_data_8A4[playerIndex + 57], 1.25f);
        }
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handles left and right changes to the Party Mode team layout, rotating the four character models
 * during the transition. fn_1_219C8 installs this after positioning the layout markers. */
void fn_1_21628(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *playerSetup;
    HuVecF characterRotation;
    s16 teamSlot;

    if (obj->work[0]++ > 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.teamLayout--;
            if (lbl_1_bss_9F4.teamLayout < 0) {
                lbl_1_bss_9F4.teamLayout += 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.teamLayout++;
            if (lbl_1_bss_9F4.teamLayout > 2) {
                lbl_1_bss_9F4.teamLayout -= 3;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            fn_1_AAF0(5);
        }
    }
    if (obj->work[0] < 10) {
        for (teamSlot = 0; teamSlot < 4; teamSlot++) {
            playerSetup = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][teamSlot]];
            Hu3DModelRotGet(
                lbl_1_bss_288[playerSetup->chrSel].modelId, &characterRotation);
            if (obj->work[1]) {
                characterRotation.y += 18.0f;
                if (characterRotation.y > 90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_288[playerSetup->chrSel].modelId,
                        &lbl_1_data_8A4[teamSlot + 49]);
                }
            } else {
                characterRotation.y -= 18.0f;
                if (characterRotation.y < -90.0f) {
                    Hu3DModelPosSetV(
                        lbl_1_bss_288[playerSetup->chrSel].modelId,
                        &lbl_1_data_8A4[teamSlot + 49]);
                }
            }
            Hu3DModelRotSetV(
                lbl_1_bss_288[playerSetup->chrSel].modelId, &characterRotation);
        }
    } else {
        for (teamSlot = 0; teamSlot < 4; teamSlot++) {
            playerSetup = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][teamSlot]];
            Hu3DModelRotGet(
                lbl_1_bss_288[playerSetup->chrSel].modelId, &characterRotation);
            characterRotation.y = 0.0f;
            Hu3DModelRotSetV(
                lbl_1_bss_288[playerSetup->chrSel].modelId, &characterRotation);
        }
    }
}

/* Places the character-pair arrows around world x = 0 at the first player's y/z position as the
 * prompt opens. */
void fn_1_219C8(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[0].chrSel].modelId, &pos);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -210.0f, 0.0f);
    fn_1_A970(5, &pos, 210.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_21628;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the character-pair arrows and ends their object callback when the prompt closes. */
void fn_1_21C04(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Resets character-pair messages and previews during the team-layout setup callback. */
void fn_1_21C9C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    lbl_1_bss_9F4.teamLayout = 0;
    lbl_1_bss_9F4.teamMessageNo[0] = MDPARTY_MSG_NO_CHARACTER_PAIR;
    lbl_1_bss_9F4.teamMessageNo[1] = MDPARTY_MSG_NO_CHARACTER_PAIR;
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[i];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 1, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 2, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 3, entry->chrSel, 1);
        }
        fn_1_96E4(
            lbl_1_bss_9F4.teamMessageNo[0], lbl_1_bss_9F4.teamMessageNo[1]);
        fn_1_9E9C();
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Clears the paired-character previews while the team-layout setup callback settles. */
void fn_1_22010(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    lbl_1_bss_9F4.teamMessageNo[0] = MDPARTY_MSG_NO_CHARACTER_PAIR;
    lbl_1_bss_9F4.teamMessageNo[1] = MDPARTY_MSG_NO_CHARACTER_PAIR;
    if (obj->work[0] == 0) {
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[i];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 1);
            fn_1_C158(entry->chrSel, 2, entry->chrSel, 1);
        }
        fn_1_9658(0, lbl_1_bss_9F4.teamMessageNo[0]);
        fn_1_9658(1, lbl_1_bss_9F4.teamMessageNo[1]);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Shows each selected team pairing and its team marker during the layout preview. */
void fn_1_221F4(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.teamMessageNo[0] = fn_1_9558(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][0]].chrSel,
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][1]].chrSel);
        lbl_1_bss_9F4.teamMessageNo[1] = fn_1_9558(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][2]].chrSel,
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][3]].chrSel);
        fn_1_9658(0, lbl_1_bss_9F4.teamMessageNo[0]);
        fn_1_9658(1, lbl_1_bss_9F4.teamMessageNo[1]);
        for (i = 0; i < 4; i++) {
            entry = &lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
            fn_1_C158(entry->chrSel, 0, entry->chrSel, 3);
        }
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][0]].chrSel,
            2, 38, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][1]].chrSel,
            2, 38, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][2]].chrSel,
            2, 39, -1);
        fn_1_C158(
            lbl_1_bss_9BC[
                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][3]].chrSel,
            2, 39, -1);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Hides the team-layout windows and sprites after that menu stage is dismissed. */
void fn_1_227E4(OMOBJ *obj)
{
    HUSPR_GROUP *group;
    s16 groupId;
    s16 i;

    if (obj->work[0] == 0) {
        HuWinDispOff(lbl_1_bss_104[0]);
        HuWinDispOff(lbl_1_bss_104[1]);
        groupId = lbl_1_bss_918[13];
        group = &HuSprGrpData[groupId];
        for (i = 0; i < group->sprNum; i++) {
            HuSprAttrSet(groupId, i, HUSPR_ATTR_DISPOFF);
        }
        lbl_1_bss_80.x = 0.0f;
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handles directional and A/B input for CPU difficulty while this setup callback is active. */
void fn_1_228E8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;
    s16 j;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        if (entry->isComputer == 1 && entry->characterChoiceState != 2) {
            if (entry->characterChoiceState == 1 && obj->work[0]++ > 10) {
                if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                    entry->computerDifficulty--;
                    if (lbl_1_bss_2C == 0) {
                        if (entry->computerDifficulty < 0) {
                            entry->computerDifficulty += 3;
                        }
                    } else if (entry->computerDifficulty < 0) {
                        entry->computerDifficulty += 4;
                    }
                    obj->work[0] = 0;
                    fn_1_AAF0(4);
                    fn_1_9B90(entry->padNo, entry->computerDifficulty + 5);
                } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                    entry->computerDifficulty++;
                    if (lbl_1_bss_2C == 0) {
                        if (entry->computerDifficulty > 2) {
                            entry->computerDifficulty -= 3;
                        }
                    } else if (entry->computerDifficulty > 3) {
                        entry->computerDifficulty -= 4;
                    }
                    obj->work[0] = 0;
                    fn_1_AAF0(5);
                    fn_1_9B90(entry->padNo, entry->computerDifficulty + 5);
                } else if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                    HuAudFXPlay(MSM_SE_CMN_02);
                    entry->characterChoiceState = 2;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                    for (j = 0; j < 4; j++) {
                        if (lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .isComputer == 1
                            && lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .characterChoiceState == 1) {
                            /* Advancing resets the next unconfirmed CPU's difficulty to 1. */
                            lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .computerDifficulty = 1;
                            break;
                        }
                    }
                    if (j != 4) {
                        obj->objFunc = fn_1_22EB8;
                    } else {
                        obj->objFunc = NULL;
                    }
                } else if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                    HuAudFXPlay(MSM_SE_CMN_04);
                    entry->characterChoiceState = 1;
                    fn_1_AA94(4);
                    fn_1_AA94(5);
                    fn_1_9B90(entry->padNo, 4);
                    for (j = 3; j >= 0; j--) {
                        if (lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .isComputer == 1
                            && lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .characterChoiceState == 2) {
                            lbl_1_bss_9BC[
                                lbl_1_data_4[lbl_1_bss_9F4.teamLayout][j]]
                                    .characterChoiceState = 1;
                            break;
                        }
                    }
                    if (j != -1) {
                        obj->objFunc = fn_1_22EB8;
                    } else {
                        obj->objFunc = NULL;
                    }
                }
            }
            break;
        }
    }
}

/* Repositions the CPU difficulty arrows and resumes the difficulty input callback. */
void fn_1_22EB8(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    HuVecF pos;
    s16 i;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        if (entry->isComputer == 1 && entry->characterChoiceState == 1) {
            Hu3DModelPosGet(
                lbl_1_bss_288[entry->chrSel].modelId, &pos);
            fn_1_A970(4, &pos, -40.0f, -45.0f);
            fn_1_A970(5, &pos, 40.0f, -45.0f);
            fn_1_9B90(entry->padNo, entry->computerDifficulty + 5);
            break;
        }
    }
    obj->work[0] = 10;
    obj->objFunc = fn_1_228E8;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the CPU difficulty arrows and ends their object callback after the choice. */
void fn_1_2318C(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* On first entry to Tag Mode CPU difficulty setup, resets every CPU's difficulty to 1 and
 * signals that the setup transition is ready. */
void fn_1_23224(OMOBJ *obj)
{
    s16 i;
    LBL_1_BSS_9BC_ENTRY *entry;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->isComputer == 1) {
            entry->computerDifficulty = 1;
        }
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* On return to Tag Mode CPU difficulty setup, moves the characters into team positions, resets
 * every CPU's difficulty to 1, and restores its unconfirmed CPU marker. */
void fn_1_232A0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ >= 10) {
        fn_1_96E4(
            lbl_1_bss_9F4.teamMessageNo[0], lbl_1_bss_9F4.teamMessageNo[1]);
        for (i = 0; i < 4; i++) {
            player = &lbl_1_bss_9BC[i];
            if (player->isComputer == 1) {
                player->characterChoiceState = 1;
                player->computerDifficulty = 1;
                HuSprBankSet(lbl_1_bss_918[i + 4], 0, 4);
            }
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

void fn_1_239DC(OMOBJ *obj)
{
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* On back navigation from Tag Mode CPU difficulty setup, resets CPUs to difficulty 1 and restores
 * the CPU icon in their roster markers. */
void fn_1_23A00(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[i];
        if (entry->isComputer == 1) {
            entry->computerDifficulty = 1;
            fn_1_9B90(i, 4);
        }
    }
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Moves the selected characters into their team positions during the lineup transition. */
void fn_1_23AC0(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handles team selection and shared handicap changes while team handicap setup is active. */
void fn_1_24090(OMOBJ *obj)
{
    HuVecF pos;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.selectedHandicapSlot--;
            if (lbl_1_bss_9F4.selectedHandicapSlot < 0) {
                lbl_1_bss_9F4.selectedHandicapSlot += 2;
            }
            obj->work[0] = 0;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.selectedHandicapSlot++;
            if (lbl_1_bss_9F4.selectedHandicapSlot > 1) {
                lbl_1_bss_9F4.selectedHandicapSlot -= 2;
            }
            obj->work[0] = 0;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap--;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap < 0) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap += 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(3);
            fn_1_9E50(lbl_1_bss_9F4.selectedHandicapSlot,
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap++;
            if (lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap > 9) {
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap -= 10;
            }
            obj->work[0] = 0;
            fn_1_AAF0(2);
            fn_1_9E50(lbl_1_bss_9F4.selectedHandicapSlot,
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].handicap);
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel].modelId,
        &pos);
    pos.x = lbl_1_data_8A4[lbl_1_bss_9F4.selectedHandicapSlot + 61].x;
    fn_1_A8D8(2, &pos, 0.0f, -80.0f);
    fn_1_A8D8(3, &pos, 0.0f, 80.0f);
}

/* Places the handicap arrows around the selected team before team handicap input begins. */
void fn_1_2457C(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel].modelId,
        &pos);
    pos.x = lbl_1_data_8A4[lbl_1_bss_9F4.selectedHandicapSlot + 61].x;
    fn_1_A970(2, &pos, 0.0f, -80.0f);
    fn_1_A970(3, &pos, 0.0f, 80.0f);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -210.0f, 0.0f);
    fn_1_A970(5, &pos, 210.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_24090;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the team-selection and handicap arrows when team handicap setup ends. */
void fn_1_249A4(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Brings the player models into the handicap layout and initializes both handicaps. */
void fn_1_24A8C(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 bank;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.selectedHandicapSlot = 0;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        for (i = 0; i < 2; i++) {
            lbl_1_bss_9BC[i].handicap = 0;
            bank = lbl_1_bss_9BC[i].handicap;
            lbl_1_bss_8C[i].x = 0.85f;
            fn_1_47B4(lbl_1_bss_918[i + 14], HUSPR_ATTR_DISPOFF);
            fn_1_9E50(i, bank);
        }
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Brings the player models into the handicap layout before its menu stage settles. */
void fn_1_25160(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    lbl_1_bss_9F4.selectedHandicapSlot = 0;
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Moves the four player models to the alternate team-layout positions over ten frames. */
void fn_1_25740(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 57].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 57].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 57].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Hides handicap markers and returns the four player models to their team positions. */
void fn_1_25D10(OMOBJ *obj)
{
    LBL_1_BSS_9BC_ENTRY *player;
    LBL_1_BSS_288_ENTRY *entry;
    HuVecF pos;
    float value;
    s32 modelNo;
    s16 i;
    s16 time;

    if (obj->work[0] == 0) {
        for (i = 0; i < 2; i++) {
            lbl_1_bss_8C[i].x = 0.0f;
        }
    }
    for (i = 0; i < 4; i++) {
        player = &lbl_1_bss_9BC[
            lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        time = obj->work[0];
        modelNo = (s16)player->chrSel;
        entry = &lbl_1_bss_288[modelNo];
        if (time == 0) {
            Hu3DModelPosGet(entry->modelId, &entry->pos);
            Hu3DModelRotGet(entry->modelId, &entry->rot);
            Hu3DModelScaleGet(entry->modelId, &entry->scale);
        }
        pos.x = fn_1_1868(entry->pos.x,
            lbl_1_data_8A4[i + 49].x, time, 10.0f);
        pos.y = fn_1_1868(entry->pos.y,
            lbl_1_data_8A4[i + 49].y, time, 10.0f);
        pos.z = fn_1_1868(entry->pos.z,
            lbl_1_data_8A4[i + 49].z, time, 10.0f);
        Hu3DModelPosSet(entry->modelId, pos.x, pos.y, pos.z);
        value = fn_1_1868(entry->scale.x, 1.25f, time, 10.0f);
        Hu3DModelScaleSet(entry->modelId, value, value, 1.0f);
    }
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Handles Party Mode rule selection and animates the highlighted rule during each update. */
void fn_1_26328(OMOBJ *obj)
{
    HuVecF pos;

    if (obj->work[0]++ >= 10) {
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            lbl_1_bss_9F4.selectedRuleIndex--;
            if (lbl_1_bss_9F4.selectedRuleIndex < 0) {
                lbl_1_bss_9F4.selectedRuleIndex += 4;
            }
            obj->work[0] = 0;
            obj->work[1] = 2;
            fn_1_AAF0(4);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            lbl_1_bss_9F4.selectedRuleIndex++;
            if (lbl_1_bss_9F4.selectedRuleIndex > 3) {
                lbl_1_bss_9F4.selectedRuleIndex -= 4;
            }
            obj->work[0] = 0;
            obj->work[1] = 2;
            fn_1_AAF0(5);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_DOWN) {
            switch (lbl_1_bss_9F4.selectedRuleIndex) {
                case 0:
                    lbl_1_bss_9F4.tagMode--;
                    if (lbl_1_bss_9F4.tagMode < 0) {
                        lbl_1_bss_9F4.tagMode += 2;
                    }
                    break;
                case 1:
                    lbl_1_bss_9F4.turnCountChoice--;
                    if (lbl_1_bss_9F4.turnCountChoice < 0) {
                        lbl_1_bss_9F4.turnCountChoice += 9;
                    }
                    break;
                case 2:
                    lbl_1_bss_9F4.minigamePack--;
                    if (lbl_1_bss_9F4.minigamePack < 0) {
                        lbl_1_bss_9F4.minigamePack += 5;
                    }
                    break;
                case 3:
                    lbl_1_bss_9F4.bonusStarChoice--;
                    if (lbl_1_bss_9F4.bonusStarChoice < 0) {
                        lbl_1_bss_9F4.bonusStarChoice += 2;
                    }
                    break;
            }
            obj->work[0] = 0;
            obj->work[1] = 0;
            fn_1_AAF0(3);
        } else if (HuPadDStkRep[0] & PAD_BUTTON_UP) {
            switch (lbl_1_bss_9F4.selectedRuleIndex) {
                case 0:
                    lbl_1_bss_9F4.tagMode++;
                    if (lbl_1_bss_9F4.tagMode >= 2) {
                        lbl_1_bss_9F4.tagMode -= 2;
                    }
                    break;
                case 1:
                    lbl_1_bss_9F4.turnCountChoice++;
                    if (lbl_1_bss_9F4.turnCountChoice >= 9) {
                        lbl_1_bss_9F4.turnCountChoice -= 9;
                    }
                    break;
                case 2:
                    lbl_1_bss_9F4.minigamePack++;
                    if (lbl_1_bss_9F4.minigamePack >= 5) {
                        lbl_1_bss_9F4.minigamePack -= 5;
                    }
                    break;
                case 3:
                    lbl_1_bss_9F4.bonusStarChoice++;
                    if (lbl_1_bss_9F4.bonusStarChoice >= 2) {
                        lbl_1_bss_9F4.bonusStarChoice -= 2;
                    }
                    break;
            }
            obj->work[0] = 0;
            obj->work[1] = 1;
            fn_1_AAF0(2);
        }
    } else if (obj->work[1] != 2) {
        switch (lbl_1_bss_9F4.selectedRuleIndex) {
            case 0:
                if (obj->work[1] != 0) {
                    fn_1_D058(21, lbl_1_bss_9F4.tagMode, 20,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(21, lbl_1_bss_9F4.tagMode, 20,
                        obj->work[0], 10, -1);
                }
                break;
            case 1:
                if (obj->work[1] != 0) {
                    fn_1_D058(22, lbl_1_bss_9F4.turnCountChoice, 22,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(22, lbl_1_bss_9F4.turnCountChoice, 22,
                        obj->work[0], 10, -1);
                }
                break;
            case 2:
                if (obj->work[1] != 0) {
                    fn_1_D058(23, lbl_1_bss_9F4.minigamePack, 31,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(23, lbl_1_bss_9F4.minigamePack, 31,
                        obj->work[0], 10, -1);
                }
                break;
            case 3:
                if (obj->work[1] != 0) {
                    fn_1_D058(24, lbl_1_bss_9F4.bonusStarChoice, 36,
                        obj->work[0], 10, 1);
                } else {
                    fn_1_D058(24, lbl_1_bss_9F4.bonusStarChoice, 36,
                        obj->work[0], 10, -1);
                }
                break;
        }
    }
    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9F4.selectedRuleIndex + 21].modelId, &pos);
    fn_1_A8D8(2, &pos, 0.0f, -65.0f);
    fn_1_A8D8(3, &pos, 0.0f, 65.0f);
}

/* Places the rule-selection arrows and starts the rule input callback. */
void fn_1_28044(OMOBJ *obj)
{
    HuVecF pos;

    Hu3DModelPosGet(
        lbl_1_bss_288[lbl_1_bss_9F4.selectedRuleIndex + 21].modelId, &pos);
    fn_1_A970(2, &pos, 0.0f, -65.0f);
    fn_1_A970(3, &pos, 0.0f, 65.0f);
    pos.x = 0.0f;
    fn_1_A970(4, &pos, -220.0f, 0.0f);
    fn_1_A970(5, &pos, 220.0f, 0.0f);
    obj->work[0] = 10;
    obj->objFunc = fn_1_26328;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Hides the rule-selection arrows and ends their object callback after the choice. */
void fn_1_28434(OMOBJ *obj)
{
    HuSprAttrSet(lbl_1_bss_918[2], 2, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[2] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 3, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[3] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 4, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[4] = 0;
    HuSprAttrSet(lbl_1_bss_918[2], 5, HUSPR_ATTR_DISPOFF);
    lbl_1_data_DE6[5] = 0;
    obj->objFunc = NULL;
    lbl_1_bss_24->mtnId[0] = 1;
}

/* Loads saved Party Mode rules into the menu and animates their values into view. */
void fn_1_2851C(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.selectedRuleIndex = 0;
        lbl_1_bss_9F4.tagMode = GwCommon.confTag;
        lbl_1_bss_9F4.turnCountChoice = (GwCommon.confTurnNum / 5) - 2;
        lbl_1_bss_9F4.minigamePack = GwCommon.partyMgPack;
        lbl_1_bss_9F4.bonusStarChoice = GwCommon.confBonusStar;
        lbl_1_bss_9F4.bonusStarChoice++;
        lbl_1_bss_9F4.bonusStarChoice %= 2;
        fn_1_C200(21, lbl_1_bss_9F4.tagMode + 20);
        fn_1_C200(22, lbl_1_bss_9F4.turnCountChoice + 22);
        fn_1_C200(23, lbl_1_bss_9F4.minigamePack + 31);
        fn_1_C200(24, lbl_1_bss_9F4.bonusStarChoice + 36);
    }
    fn_1_C28C(21, obj->work[0], 10, &lbl_1_data_8A4[63], 1.5f);
    fn_1_C28C(22, obj->work[0], 10, &lbl_1_data_8A4[64], 1.5f);
    fn_1_C28C(23, obj->work[0], 10, &lbl_1_data_8A4[65], 1.5f);
    fn_1_C28C(24, obj->work[0], 10, &lbl_1_data_8A4[66], 1.5f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Resets the rule highlight and moves the four rule cards back into the rule-selection layout. */
void fn_1_29598(OMOBJ *obj)
{
    if (obj->work[0] == 0) {
        lbl_1_bss_9F4.selectedRuleIndex = 0;
    }
    fn_1_CA68(21, obj->work[0], 10, &lbl_1_data_8A4[63], 1.5f);
    fn_1_CA68(22, obj->work[0], 10, &lbl_1_data_8A4[64], 1.5f);
    fn_1_CA68(23, obj->work[0], 10, &lbl_1_data_8A4[65], 1.5f);
    fn_1_CA68(24, obj->work[0], 10, &lbl_1_data_8A4[66], 1.5f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Animates the four rule values into their summary positions during the menu transition. */
void fn_1_2A8C4(OMOBJ *obj)
{
    fn_1_CA68(21, obj->work[0], 10, &lbl_1_data_8A4[37], 1.0f);
    fn_1_CA68(22, obj->work[0], 10, &lbl_1_data_8A4[38], 1.0f);
    fn_1_CA68(23, obj->work[0], 10, &lbl_1_data_8A4[39], 1.0f);
    fn_1_CA68(24, obj->work[0], 10, &lbl_1_data_8A4[40], 1.0f);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Rule setup uses this callback when returning to board selection: it spins and shrinks the four
* rule-value displays, then signals completion. */
void fn_1_2BBD8(OMOBJ *obj)
{
    fn_1_C698(21, obj->work[0], 10);
    fn_1_C698(22, obj->work[0], 10);
    fn_1_C698(23, obj->work[0], 10);
    fn_1_C698(24, obj->work[0], 10);
    if (obj->work[0]++ > 10) {
        obj->objFunc = NULL;
        lbl_1_bss_24->mtnId[0] = 1;
    }
}

/* Releases Party Mode menu objects and resets its setup subsystems during exit. */
void fn_1_2C874(void)
{
    fn_1_87BC(lbl_1_bss_4);
    fn_1_945C(lbl_1_bss_8);
    fn_1_6284(lbl_1_bss_C);
    fn_1_72C4(lbl_1_bss_10);
    fn_1_8074(lbl_1_bss_14);
    fn_1_A880();
    fn_1_B6C0(lbl_1_bss_1C.obj);
    fn_1_4581C();
    fn_1_D978();
    fn_1_3C44();
    fn_1_36C4();
    fn_1_353C();
}

/* Waits for the exit wipe to finish, then cleans up and returns to the calling overlay. */
void fn_1_2CD6C(OMOBJ *obj)
{
    if (!WipeCheck()) {
        fn_1_2C874();
        omOvlReturnEx(1, 1);
    }
}

/* Starts the exit wipe when the object manager receives a system-exit request. */
void fn_1_2CDA8(OMOBJ *obj)
{
    if (omSysExitReq != 0) {
        WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        obj->objFunc = fn_1_2CD6C;
    }
}

/* Runs the Party Mode exit process, fades music, waits for the wipe, and selects the next flow. */
void fn_1_2CE00(void)
{
    s16 result = 0;

    result = fn_1_3EAC8(0);
    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, 1000);
        lbl_1_data_0 = -1;
    }
    switch (result) {
        case 0:
        case 1:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck()) {
                HuPrcVSleep();
            }
            break;
        case 2:
        case 3:
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
            while (WipeCheck()) {
                HuPrcVSleep();
            }
            break;
    }
    fn_1_2C874();
    switch (result) {
        case 0:
            fn_1_12F8();
            omOvlReturnEx(1, 1);
            break;
        case 1:
            fn_1_109C(1);
            break;
        case 2:
        case 3:
            fn_1_109C(0);
            break;
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Creates Party Mode's object manager, registers setup objects, and starts its exit process. */
void fn_1_2D1D4(void)
{
    lbl_1_bss_0 = omInitObjMan(27, 8192);
    omGameSysInit(lbl_1_bss_0);
    fn_1_33A0(fn_1_2DFD4);
    fn_1_3598();
    fn_1_3A1C();
    fn_1_467C();
    fn_1_4834();
    fn_1_D748();
    fn_1_44DCC(lbl_1_bss_0);
    lbl_1_bss_4 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_861C);
    lbl_1_bss_8 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_9218);
    lbl_1_bss_C =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_605C);
    lbl_1_bss_10 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_7064);
    lbl_1_bss_14 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 32, -1, fn_1_7ED4);
    lbl_1_bss_18 =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_A5D8);
    lbl_1_bss_1C.obj =
        omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, fn_1_B4C8);
    HuPrcChildCreate(fn_1_2CE00, 12288, 12288, 0, lbl_1_bss_0);
}

void fn_1_2DD38(void)
{
    HuDataDirCloseAll();
}

/* Initializes Party Mode state and starts its object manager when the overlay is entered. */
void ObjectSetup(void)
{
    OSReport(lbl_1_data_ED8);
    fn_1_2DD38();
    fn_1_1248();
    _ClearFlag(FLAG_BOARD_INIT);
    _ClearFlag(FLAGNUM(FLAG_GROUP_COMMON, 35));
    lbl_1_bss_2A = FALSE;
    lbl_1_bss_2C = FALSE;
    if (GWBankFlagGet(2)) {
        lbl_1_bss_2A = TRUE;
    }
    if (GWBankFlagGet(3)) {
        lbl_1_bss_2C = TRUE;
    }
    lbl_1_data_BEE[0] = TRUE;
    lbl_1_data_BEE[1] = TRUE;
    lbl_1_data_BEE[2] = TRUE;
    lbl_1_data_BEE[3] = TRUE;
    lbl_1_data_BEE[4] = TRUE;
    lbl_1_data_BEE[5] = FALSE;
    if (GWBankFlagGet(MDPARTY_BANK_FLAG_SIXTH_BOARD)) {
        lbl_1_data_BEE[5] = TRUE;
    }
    lbl_1_bss_28 = omovlevtno;
    fn_1_2D1D4();
}

/* Runs overlay constructors before initializing the Party Mode objects. */
int _prolog(void)
{
    const VoidFunc *ctors = _ctors;

    while (*ctors) {
        (**ctors)();
        ctors++;
    }
    ObjectSetup();
    return 0;
}

/* Runs overlay destructors when Party Mode is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtors = _dtors;

    while (*dtors) {
        (**dtors)();
        dtors++;
    }
}

/* Sets the initial camera target for the Party Mode setup view. */
void fn_1_2DFD4(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->targetCenter.x = 0.0f;
    camera->targetCenter.y = 65.0f;
    camera->targetCenter.z = -800.0f;
    camera->targetRot.x = -7.25f;
    camera->targetRot.y = 0.0f;
    camera->targetRot.z = 0.0f;
    camera->targetZoom = 2150.0f;
    fn_1_28B0(camera, 15.0f);
}

/* Moves the setup camera toward the wider view over its 60-frame transition callback. */
void fn_1_2E338(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->center.y =
        fn_1_1868(camera->targetCenter.y, 275.0f, camera->transitionFrame, 60.0f);
    camera->center.z =
        fn_1_1868(camera->targetCenter.z, -2900.0f, camera->transitionFrame, 60.0f);
    camera->rot.x =
        fn_1_1868(camera->targetRot.x, 0.0f, camera->transitionFrame, 60.0f);
    camera->transitionFrame += 1.0f;
}

/* Starts the camera transition back to the wider Party Mode setup view. */
void fn_1_2E560(OMOBJ *obj, MDCAMERA_WORK *camera)
{
    camera->transitionFrame = 0.0f;
    fn_1_2810(camera);
    fn_1_2B64(fn_1_2E338);
}

/* Starts Party Mode's stream and entrance wipe after the setup process pauses briefly. */
s32 fn_1_2E5D4(void)
{
    HuPrcSleep(5);
    lbl_1_data_0 = HuAudSStreamPlay(MDPARTY_STREAM_MODE);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    return TRUE;
}

inline s32 fn_1_2E5D4(void);

/* Opens the first Party Mode setup prompts when this menu stage begins. */
s16 fn_1_2E638(void)
{
    OMOBJ *obj;

    fn_1_5194();
    fn_1_52B0();
    fn_1_4020(1, MDPARTY_MSG_SETUP_FIRST, 1);
    fn_1_3EC8();
    fn_1_5194();
    fn_1_4020(3, MDPARTY_MSG_SETUP_SECOND, 1);
    fn_1_3EC8();
    obj = lbl_1_bss_4;
    obj->work[0] = 0;
    obj->objFunc = fn_1_8398;
    return TRUE;
}

/* Before saved-game or new-game prompts, closes the message window and requests both setup
 * characters' exit motions when the first character is near the center. */
s16 fn_1_2EBEC(void)
{
    HuVecF pos;

    Hu3DModelPosGet(lbl_1_bss_C->mdlId[0], &pos);
    if (pos.x > -200) {
        fn_1_3E0C();
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
            lbl_1_bss_C->mtnId[5], 0.0f, 30.0f, 0);
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0],
            lbl_1_bss_10->mtnId[5], 0.0f, 30.0f, 0);
        HuPrcSleep(60);
        /* Each exit helper may decline based on its model position; their results are ignored. */
        fn_1_5FB0();
        fn_1_6FB8();
        fn_1_3E0C();
        HuPrcSleep(90);
    }
}

inline s16 fn_1_2EBEC(void);

/* Shows exit confirmation after a canceled setup step; zero leaves Party Mode, while canceling
 * selects option 1 so setup resumes. */
s16 fn_1_2EF24(void)
{
    fn_1_52B0();
    fn_1_4020(2, MDPARTY_MSG_EXIT_CONFIRMATION, 1);
    return fn_1_3F28(2);
}

inline s16 fn_1_2EF24(void);

/* Initial Party setup loads an available saved board and offers resume or new setup; returns 0 to
 * resume, 1 to begin setup, and -1 to leave Party Mode. */
s16 fn_1_2F22C(void)
{
    s16 menuChoice = 0;
    s16 savedTurn = 0;

    if (GwCommon.saveEnableF == TRUE) {
        SLBoardLoad();
        lbl_1_bss_9F4.tagMode = GwSystem.tagF;
        lbl_1_bss_9F4.boardNo = GwSystem.boardNo;
        lbl_1_bss_9F4.turnCountChoice = (GwSystem.turnMax / 5) - 2;
        lbl_1_bss_9F4.minigamePack = GwSystem.mgPack;
        lbl_1_bss_9F4.bonusStarChoice = GwSystem.bonusStarF;
        /* The setup menu stores the bonus-star option in the opposite order to the board state. */
        lbl_1_bss_9F4.bonusStarChoice++;
        lbl_1_bss_9F4.bonusStarChoice %= 2;
        savedTurn = GwSystem.turnNo;
        fn_1_2EBEC();
        fn_1_D6B0(fn_1_DA54);
        fn_1_8124(savedTurn, (lbl_1_bss_9F4.turnCountChoice + 2) * 5);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_52B0();
            fn_1_4020(2, MDPARTY_MSG_SAVED_GAME_CONTINUE_OPTION, 1);
            menuChoice = fn_1_3F28(0);
            if (menuChoice == 0) {
                _SetFlag(FLAGNUM(FLAG_GROUP_COMMON, 35));
                fn_1_8318();
                fn_1_D6B0(fn_1_EB74);
                fn_1_3E0C();
                fn_1_64C();
                HuAudFXPlay(MSM_SE_MENU_19);
                fn_1_58A0();
                fn_1_68A4();
                HuPrcSleep(10);
                fn_1_463E0();
                if (lbl_1_data_0 != -1) {
                    HuAudSStreamFadeOut(lbl_1_data_0, 3000);
                    lbl_1_data_0 = -1;
                }
                HuAudFXPlay(MSM_SE_MENU_20);
                HuPrcSleep(90);
                fn_1_2B64(fn_1_2E338);
                HuPrcSleep(60);
                return 0;
            }
            if (menuChoice == -1) {
                if (fn_1_2EF24()) {
                    continue;
                }
                return -1;
            }
            while (TRUE) {
                HuPrcVSleep();
                fn_1_4020(2, MDPARTY_MSG_SAVED_GAME_SETUP_OPTION, 1);
                menuChoice = fn_1_3F28(0);
                if (menuChoice != -1) {
                    break;
                }
                if (fn_1_2EF24()) {
                    continue;
                }
                return -1;
            }
            if (menuChoice != 0) {
                continue;
            }
            fn_1_8318();
            fn_1_D6B0(fn_1_F838);
            break;
        }
    }
    return 1;
}

/* The initial setup step offers the opening board choice; choice 0 prepares four CPU players on
 * board 6 and starts loading assets. Returns the chosen option, or -1 after canceling. */
s16 fn_1_30B4C(void)
{
    s16 menuChoice = 0;
    while (TRUE) {
        HuPrcVSleep();
        fn_1_52B0();
        fn_1_4020(2, MDPARTY_MSG_OPENING_BOARD_CHOICE, 1);
        menuChoice = fn_1_3F28(0);
        if (menuChoice != -1) {
            if (menuChoice == 0) {
                fn_1_3E0C();
                fn_1_E30();
                fn_1_64C();
            }
            break;
        }
        if (!fn_1_2EF24()) {
            menuChoice = -1;
            break;
        }
    }
    return menuChoice;
}

/* Presents the next player setup prompt and returns success to the caller. */
s16 fn_1_31508(void)
{
    fn_1_2EBEC();
    fn_1_5194();
    fn_1_4020(3, MDPARTY_MSG_NEXT_PARTY_PROMPT, 1);
    fn_1_3EC8();
    return TRUE;
}

/* Waits for a Party Mode setup choice and returns the selected result. */
s16 fn_1_31AC0(s16 unused)
{
    s16 result = 0;
    fn_1_D6B0(fn_1_1158C);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_114C4);
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_52B0();
        fn_1_4020(2, MDPARTY_MSG_PLAYER_COUNT_BASE + lbl_1_bss_9F4.playerCount, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
            fn_1_4020(2, MDPARTY_MSG_PLAYER_COUNT_BASE + lbl_1_bss_9F4.playerCount, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                fn_1_275C();
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_114F4);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result != -1) {
        fn_1_D6B0(fn_1_11EAC);
    }
    return result;
}

/* The setup confirmation loop polls character choices: 0 means enough slots have chosen, 1 means
 * player 0 is still choosing, and -1 means selection is still incomplete. */
s16 fn_1_32904(void)
{
    s16 chosenPlayerCount;
    s16 playerSlot;

    for (playerSlot = 0, chosenPlayerCount = 0; playerSlot < 4; playerSlot++) {
        if (lbl_1_bss_9BC[playerSlot].characterChoiceState == 1) {
            chosenPlayerCount++;
        }
    }
    if (lbl_1_bss_9F4.playerCount <= chosenPlayerCount) {
        return 0;
    }
    if (lbl_1_bss_9BC[0].characterChoiceState == 0) {
        return 1;
    }
    return -1;
}

inline s16 fn_1_32904(void);

/* Runs human character selection, waiting for all human slots to confirm and allowing back
 * navigation or exit. */
s16 fn_1_329A4(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_D6B0(fn_1_12EC8);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_5194();
        fn_1_4020(3, MDPARTY_MSG_HUMAN_CHARACTER_PROMPT, 0);
        fn_1_D6B0(fn_1_12B18);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, MDPARTY_MSG_HUMAN_CHARACTER_PROMPT, 0);
            result = fn_1_32904();
            if (!fn_1_D678()) {
                continue;
            }
            if (result == 0) {
                result = 0;
                break;
            }
            if (result == 1 && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_12D7C);
        if (result != -1) {
            break;
        }
        if (!fn_1_2EF24()) {
            result = -1;
            break;
        }
    }
    return result;
}

/* Returns the number of unfinished CPU choices and reports the current and first CPU slots;
 * the last slot is always 3. */
s16 fn_1_335F0(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot)
{
    s16 i;
    s16 count = 0;

    *activePlayer = 0;
    *firstActivePlayer = 0;
    *lastPlayerSlot = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1) {
            *firstActivePlayer = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1
            && lbl_1_bss_9BC[i].characterChoiceState != 2) {
            break;
        }
    }
    *activePlayer = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1
            && lbl_1_bss_9BC[i].characterChoiceState != 2) {
            count++;
        }
    }
    return count;
}

/* Runs CPU character and difficulty selection, allowing back navigation or exit. */
s16 fn_1_33724(s16 unused)
{
    s16 result = 0;
    s16 activePlayer = 0;
    s16 firstActivePlayer = 0;
    s16 lastPlayerSlot = 0;
    s16 count = 0;

    count = fn_1_335F0(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_5194();
        if (lbl_1_bss_9BC[activePlayer].characterChoiceState == 0) {
            fn_1_4258(3, MDPARTY_MSG_CPU_SLOT_MESSAGE_BASE - (count - 1), 0);
            fn_1_4020(3, MDPARTY_MSG_CPU_CHARACTER_PROMPT, 0);
        } else {
            fn_1_4020(3, MDPARTY_MSG_CPU_DIFFICULTY_PROMPT, 0);
        }
        fn_1_D6B0(fn_1_13EC0);
        while (TRUE) {
            HuPrcVSleep();
            if (lbl_1_bss_9BC[activePlayer].characterChoiceState == 0) {
                fn_1_4258(3, MDPARTY_MSG_CPU_SLOT_MESSAGE_BASE - (count - 1), 0);
                fn_1_4020(3, MDPARTY_MSG_CPU_CHARACTER_PROMPT, 0);
            } else {
                fn_1_4020(3, MDPARTY_MSG_CPU_DIFFICULTY_PROMPT, 0);
            }
            if (!fn_1_D678()) {
                continue;
            }
            count = fn_1_335F0(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
            if (activePlayer > lastPlayerSlot) {
                result = 0;
                break;
            }
            if (firstActivePlayer == activePlayer
                && lbl_1_bss_9BC[activePlayer].characterChoiceState == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_14344);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

/* Runs the Party Mode character-selection step and waits for confirm or cancel. */
s16 fn_1_34D10(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_D6B0(fn_1_16A8C);
    }
    fn_1_D6B0(fn_1_150D0);
    switch (lbl_1_bss_9F4.playerCount) {
        case 0:
            result = fn_1_33724(0);
            break;
        case 4:
            result = fn_1_329A4(0);
            break;
        default:
            while (TRUE) {
                HuPrcVSleep();
                result = fn_1_329A4(previousStepPending);
                if (result != 0) {
                    break;
                }
                result = fn_1_33724(0);
                if (result != 1) {
                    break;
                }
                previousStepPending = 1;
            }
            break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_156B4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_16E84);
    }
    return result;
}

inline s16 fn_1_34D10(s16 returningFromPreviousStep);

/* Returns the number of unconfirmed CPU characters for Tag Mode and reports current and first
 * CPU slots; the last slot is always 3. */
s16 fn_1_34FC4(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot)
{
    s16 i;
    s16 count = 0;

    *activePlayer = 0;
    *firstActivePlayer = 0;
    *lastPlayerSlot = 3;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1) {
            *firstActivePlayer = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1
            && lbl_1_bss_9BC[i].characterChoiceState != 1) {
            break;
        }
    }
    *activePlayer = i;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_9BC[i].isComputer == 1
            && lbl_1_bss_9BC[i].characterChoiceState != 1) {
            count++;
        }
    }
    return count;
}

/* Runs Tag Mode CPU character selection and waits for each CPU slot to confirm its character. */
s16 fn_1_350F8(s16 unused)
{
    s16 result = 0;
    s16 activePlayer = 0;
    s16 firstActivePlayer = 0;
    s16 lastPlayerSlot = 0;
    s16 count = 0;

    /* The initial prompt uses the ordinary CPU counter, which counts states other than 2; later
     * polling uses the Tag Mode counter, which counts states other than 1. */
    count = fn_1_335F0(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_5194();
        fn_1_4258(3, MDPARTY_MSG_CPU_SLOT_MESSAGE_BASE - (count - 1), 0);
        fn_1_4020(3, MDPARTY_MSG_CPU_CHARACTER_PROMPT, 0);
        fn_1_D6B0(fn_1_14CC0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(3, MDPARTY_MSG_CPU_SLOT_MESSAGE_BASE - (count - 1), 0);
            fn_1_4020(3, MDPARTY_MSG_CPU_CHARACTER_PROMPT, 0);
            if (!fn_1_D678()) {
                continue;
            }
            count = fn_1_34FC4(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
            if (activePlayer > lastPlayerSlot) {
                result = 0;
                break;
            }
            if (firstActivePlayer == activePlayer
                && lbl_1_bss_9BC[activePlayer].characterChoiceState == 0
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_14F64);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    return result;
}

/* Runs the alternate player-selection step used by the tag-team setup flow. */
s16 fn_1_36264(s16 returningFromPreviousStep)
{
    s16 result = 0;
    s16 previousStepPending = 0;

    if (returningFromPreviousStep != 0) {
        fn_1_D6B0(fn_1_16A8C);
    }
    fn_1_D6B0(fn_1_150D0);
    switch (lbl_1_bss_9F4.playerCount) {
        case 0:
            result = fn_1_350F8(0);
            break;
        case 4:
            result = fn_1_329A4(0);
            break;
        default:
            while (TRUE) {
                HuPrcVSleep();
                result = fn_1_329A4(previousStepPending);
                if (result != 0) {
                    break;
                }
                result = fn_1_350F8(0);
                if (result != 1) {
                    break;
                }
                previousStepPending = 1;
            }
            break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_160A0);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_16E84);
    }
    return result;
}

inline s16 fn_1_36264(s16 returningFromPreviousStep);

/* Runs the Tag Mode team-pairing selection and waits for confirmation, back navigation, or exit. */
s16 fn_1_36518(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep == 0) {
        fn_1_D6B0(fn_1_21C9C);
    } else {
        fn_1_D6B0(fn_1_22010);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_219C8);
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_5194();
    fn_1_4020(3, MDPARTY_MSG_TEAM_LAYOUT_PROMPT, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, MDPARTY_MSG_TEAM_LAYOUT_PROMPT, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_21C04);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_221F4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_227E4);
    }
    return result;
}

/* Reports the first CPU and current unfinished CPU positions in team order; the last position
 * is always 3. */
void fn_1_37260(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayerSlot)
{
    LBL_1_BSS_9BC_ENTRY *entry;
    s16 i;

    *activePlayer = 0;
    *firstActivePlayer = 0;
    *lastPlayerSlot = 3;
    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        if (entry->isComputer == 1) {
            *firstActivePlayer = i;
            break;
        }
    }
    for (i = 0; i < 4; i++) {
        entry = &lbl_1_bss_9BC[lbl_1_data_4[lbl_1_bss_9F4.teamLayout][i]];
        if (entry->isComputer == 1 && entry->characterChoiceState != 2) {
            break;
        }
    }
    *activePlayer = i;
}

inline void fn_1_37260(s16 *activePlayer, s16 *firstActivePlayer, s16 *lastPlayer);

/* Runs CPU difficulty selection in team order; with four human players it skips forward or
 * restores the team display when returning. */
s16 fn_1_3736C(s16 returningFromPreviousStep)
{
    s16 activePlayer;
    s16 firstActivePlayer;
    s16 lastPlayerSlot;
    s16 result = 0;

    if (lbl_1_bss_9F4.playerCount == 4) {
        if (returningFromPreviousStep == 0) {
            return 0;
        }
        fn_1_96E4(lbl_1_bss_9F4.teamMessageNo[0], lbl_1_bss_9F4.teamMessageNo[1]);
        return 1;
    }
    if (returningFromPreviousStep == 0) {
        fn_1_D6B0(fn_1_23224);
    } else {
        fn_1_D6B0(fn_1_232A0);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_5194();
        fn_1_4020(3, MDPARTY_MSG_CPU_DIFFICULTY_PROMPT, 0);
        fn_1_D6B0(fn_1_22EB8);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4020(3, MDPARTY_MSG_CPU_DIFFICULTY_PROMPT, 0);
            if (!fn_1_D678()) {
                continue;
            }
            fn_1_37260(&activePlayer, &firstActivePlayer, &lastPlayerSlot);
            if (activePlayer > lastPlayerSlot) {
                result = 0;
                break;
            }
            /* This back check uses the team-order position directly as a player-slot index. */
            if (activePlayer == firstActivePlayer
                && lbl_1_bss_9BC[activePlayer].characterChoiceState == 1
                && (HuPadBtnDown[0] & PAD_BUTTON_B)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_2318C);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_239DC);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_23A00);
    }
    return result;
}

/* Displays both selected team pairings, closes the pairing windows, and moves the character
 * cards into the next setup layout. */
s16 fn_1_38314(void)
{
    fn_1_5194();
    fn_1_4258(3, lbl_1_bss_9F4.teamMessageNo[0], 0);
    fn_1_4258(3, lbl_1_bss_9F4.teamMessageNo[1], 1);
    fn_1_4020(3, MDPARTY_MSG_TEAM_PAIRING_SUMMARY, 1);
    fn_1_3EC8();
    fn_1_9804();
    fn_1_D6B0(fn_1_23AC0);
}

/* Asks whether to accept the current Party Mode selection; the flow calls this on entry and when
 * returning from a later step. */
s16 fn_1_38A3C(s16 returningToSelection)
{
    s16 result = 0;

    if (returningToSelection != 0) {
        fn_1_D6B0(fn_1_18A10);
    }
    fn_1_D6B0(fn_1_18384);
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_17E10);
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_52B0();
        if (lbl_1_data_BEE[lbl_1_bss_9F4.boardNo] == 1) {
            fn_1_4020(2, lbl_1_bss_9F4.boardNo + MDPARTY_MSG_BOARD_NAME_BASE, 0);
        } else {
            fn_1_4020(2, MDPARTY_MSG_LOCKED_BOARD_MESSAGE, 0);
        }
        while (TRUE) {
            HuPrcVSleep();
            if (lbl_1_data_BEE[lbl_1_bss_9F4.boardNo] == 1) {
                fn_1_4020(2, lbl_1_bss_9F4.boardNo + MDPARTY_MSG_BOARD_NAME_BASE, 0);
            } else {
                fn_1_4020(2, MDPARTY_MSG_LOCKED_BOARD_MESSAGE, 0);
            }
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                if (lbl_1_data_BEE[lbl_1_bss_9F4.boardNo] == 0) {
                    HuAudFXPlay(MSM_SE_CMN_05);
                    continue;
                }
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_1807C);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_18AA4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_1948C);
    }
    return result;
}

/* Runs individual player handicap setup and handles confirmation, back navigation, or exit. */
s16 fn_1_39C60(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep == 0) {
        fn_1_D6B0(fn_1_1A20C);
    } else {
        fn_1_D6B0(fn_1_1A8B0);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_19D24);
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_52B0();
        fn_1_4258(2,
            lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel, 0);
        fn_1_4020(2, MDPARTY_MSG_INDIVIDUAL_HANDICAP_PROMPT, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(2,
                lbl_1_bss_9BC[lbl_1_bss_9F4.selectedHandicapSlot].chrSel, 0);
            fn_1_4020(2, MDPARTY_MSG_INDIVIDUAL_HANDICAP_PROMPT, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_1A124);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_1AE60);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_1B400);
    }
    return result;
}

/* Runs the two teams' handicap setup, allowing confirmation, back navigation, or exit. */
s16 fn_1_3AD84(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep == 0) {
        fn_1_D6B0(fn_1_24A8C);
    } else {
        fn_1_D6B0(fn_1_25160);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_2457C);
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        fn_1_52B0();
        fn_1_4258(2,
            lbl_1_bss_9F4.teamMessageNo[lbl_1_bss_9F4.selectedHandicapSlot], 0);
        fn_1_4020(2, MDPARTY_MSG_TEAM_HANDICAP_PROMPT, 0);
        while (TRUE) {
            HuPrcVSleep();
            fn_1_4258(2,
                lbl_1_bss_9F4.teamMessageNo[lbl_1_bss_9F4.selectedHandicapSlot], 0);
            fn_1_4020(2, MDPARTY_MSG_TEAM_HANDICAP_PROMPT, 0);
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_249A4);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_25740);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_25D10);
    }
    return result;
}

/* Runs final confirmation after handicap setup: accepts the game, restarts setup, or returns
 * to handicap setup. */
s16 fn_1_3BEA8(void)
{
    s16 result = 0;

    fn_1_D6B0(fn_1_1B9E8);
    HuPrcVSleep();
    fn_1_5194();
    fn_1_4020(3, MDPARTY_MSG_NEW_GAME_CHOICE, 1);
    if ((result = fn_1_3F28(2)) == 0) {
        fn_1_4020(1, MDPARTY_MSG_NEW_GAME_SELECTED, 1);
        HuPrcSleep(60);
        fn_1_4FE8();
        HuPrcSleep(60);
        fn_1_3E0C();
        fn_1_D6B0(fn_1_1D758);
        result = 0;
    } else {
        fn_1_5194();
        fn_1_4020(3, MDPARTY_MSG_RESTART_SETUP_PROMPT, 1);
        if ((result = fn_1_3F28(2)) == 0) {
            fn_1_D6B0(fn_1_1E7F8);
            result = 1;
        } else {
            fn_1_D6B0(fn_1_1F8B8);
            result = 2;
        }
    }
    return result;
}

/* Commits the accepted rules and players, starts loading board assets, and moves the characters
 * and camera through the departure from setup. */
void fn_1_3CA70(void)
{
    fn_1_718();
    fn_1_64C();
    HuPrcSleep(10);
    fn_1_7900();
    fn_1_58A0();
    fn_1_68A4();
    HuPrcSleep(10);
    fn_1_463E0();
    if (lbl_1_data_0 != -1) {
        HuAudSStreamFadeOut(lbl_1_data_0, 3000);
        lbl_1_data_0 = -1;
    }
    HuAudFXPlay(MSM_SE_MENU_20);
    HuPrcSleep(90);
    lbl_1_bss_A10.callback = fn_1_2E338;
    HuPrcSleep(60);
}

/* Confirms the selected Party Mode rules and handles return navigation to its caller. */
s16 fn_1_3CFE4(s16 returningFromPreviousStep)
{
    s16 result = 0;

    if (returningFromPreviousStep == 0) {
        fn_1_D6B0(fn_1_2851C);
    } else {
        fn_1_D6B0(fn_1_29598);
    }
    while (TRUE) {
        HuPrcVSleep();
        fn_1_D6B0(fn_1_28044);
        fn_1_5194();
        fn_1_444C(MDPARTY_MSG_SELECT_PROMPT);
        switch (lbl_1_bss_9F4.selectedRuleIndex) {
            case 0:
                fn_1_4020(3, lbl_1_bss_9F4.tagMode + MDPARTY_MSG_GAME_MODE_OPTION_BASE, 0);
                break;
            case 1:
                fn_1_4020(3, lbl_1_bss_9F4.turnCountChoice + MDPARTY_MSG_OPTION_GROUP_TWO_BASE, 0);
                break;
            case 2:
                fn_1_4020(3, lbl_1_bss_9F4.minigamePack + MDPARTY_MSG_OPTION_GROUP_THREE_BASE, 0);
                break;
            case 3:
                fn_1_4020(3, lbl_1_bss_9F4.bonusStarChoice + MDPARTY_MSG_OPTION_GROUP_FOUR_BASE, 0);
                break;
        }
        while (TRUE) {
            HuPrcVSleep();
            switch (lbl_1_bss_9F4.selectedRuleIndex) {
                case 0:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.tagMode + MDPARTY_MSG_GAME_MODE_OPTION_BASE, 0);
                    break;
                case 1:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.turnCountChoice + MDPARTY_MSG_OPTION_GROUP_TWO_BASE, 0);
                    break;
                case 2:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.minigamePack + MDPARTY_MSG_OPTION_GROUP_THREE_BASE, 0);
                    break;
                case 3:
                    fn_1_4020(3,
                        lbl_1_bss_9F4.bonusStarChoice + MDPARTY_MSG_OPTION_GROUP_FOUR_BASE, 0);
                    break;
            }
            if (!fn_1_D678()) {
                continue;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_03);
                result = 0;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_B) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = 1;
                break;
            }
            if (HuPadBtnDown[0] & PAD_BUTTON_X) {
                HuAudFXPlay(MSM_SE_CMN_04);
                result = -1;
                break;
            }
        }
        fn_1_45D0();
        fn_1_D6B0(fn_1_28434);
        if (result == -1) {
            if (fn_1_2EF24()) {
                continue;
            }
            result = -1;
        }
        break;
    }
    if (result == 0) {
        fn_1_D6B0(fn_1_2A8C4);
    } else if (result == 1) {
        fn_1_D6B0(fn_1_2BBD8);
    }
    return result;
}

/* Runs the Party Mode setup state machine until the player exits or confirms a game. */
s16 fn_1_3EAC8(s16 unused)
{
    s16 state = 0;
    s16 result = 0;
    s16 back = 0;

    fn_1_2E5D4();
    if (lbl_1_bss_28 == 0) {
        fn_1_2E638();
        result = fn_1_2F22C();
        if (result == 0) {
            return 3;
        }
        if (result == -1) {
            return 0;
        }
    } else {
        state = 1;
        back = 0;
    }
    while (TRUE) {
        HuPrcVSleep();
        switch (state) {
            case 0:
                result = fn_1_30B4C();
                if (result == 0) {
                    return 1;
                }
                if (result == 1) {
                    state = 1;
                    back = 0;
                }
                break;
            case 1:
                fn_1_31508();
                state = 2;
                back = 0;
                break;
            case 2:
                result = fn_1_38A3C(back);
                if (result == 0) {
                    state = 3;
                    back = 0;
                } else if (result == 1) {
                    state = 0;
                    back = 1;
                }
                break;
            case 3:
                result = fn_1_3CFE4(back);
                if (result == 0) {
                    state = 4;
                    back = 0;
                } else if (result == 1) {
                    state = 2;
                    back = 1;
                }
                break;
            case 4:
                result = fn_1_31AC0(back);
                if (result == 0) {
                    if (lbl_1_bss_9F4.tagMode == 0) {
                        state = 5;
                        back = 0;
                    } else {
                        state = 100;
                        back = 0;
                    }
                } else if (result == 1) {
                    state = 3;
                    back = 1;
                }
                break;
            case 5:
                result = fn_1_34D10(back);
                if (result == 0) {
                    state = 6;
                    back = 0;
                } else if (result == 1) {
                    state = 4;
                    back = 1;
                }
                break;
            case 6:
                result = fn_1_39C60(back);
                if (result == 0) {
                    state = 7;
                    back = 0;
                } else if (result == 1) {
                    state = 5;
                    back = 1;
                }
                break;
            case 7:
                result = fn_1_3BEA8();
                if (result == 0) {
                    state = 8;
                    back = 0;
                } else if (result == 1) {
                    state = 1;
                    back = 0;
                } else if (result == 2) {
                    if (lbl_1_bss_9F4.tagMode == 0) {
                        state = 6;
                        back = 1;
                    } else {
                        state = 104;
                        back = 1;
                    }
                }
                break;
            case 8:
                fn_1_3CA70();
                return 2;
            case 100:
                result = fn_1_36264(back);
                if (result == 0) {
                    state = 101;
                    back = 0;
                } else if (result == 1) {
                    state = 4;
                    back = 1;
                }
                break;
            case 101:
                result = fn_1_36518(back);
                if (result == 0) {
                    state = 102;
                    back = 0;
                } else if (result == 1) {
                    state = 100;
                    back = 1;
                }
                break;
            case 102:
                result = fn_1_3736C(back);
                if (result == 0) {
                    state = 103;
                    back = 0;
                } else if (result == 1) {
                    state = 101;
                    back = 1;
                }
                break;
            case 103:
                fn_1_38314();
                state = 104;
                back = 0;
                break;
            case 104:
                result = fn_1_3AD84(back);
                if (result == 0) {
                    state = 7;
                    back = 0;
                } else if (result == 1) {
                    state = 102;
                    back = 1;
                }
                break;
        }
        if (result == -1) {
            return 0;
        }
    }
}

s32 lbl_1_data_0 = -1;
s16 lbl_1_data_4[3][4] = {
    { 0, 1, 2, 3 },
    { 0, 2, 1, 3 },
    { 0, 3, 1, 2 },
};
u32 lbl_1_data_1C[32] = {
    DATANUM(DATA_mdparty, 28), DATANUM(DATA_mdparty, 4),
    DATANUM(DATA_mdparty, 5), DATANUM(DATA_mdparty, 6),
    DATANUM(DATA_mdparty, 7), DATANUM(DATA_mdparty, 8),
    DATANUM(DATA_mdparty, 9), DATANUM(DATA_mdparty, 10),
    DATANUM(DATA_mdparty, 11), DATANUM(DATA_mdparty, 12),
    DATANUM(DATA_mdparty, 13), DATANUM(DATA_mdparty, 14),
    DATANUM(DATA_mdparty, 15), DATANUM(DATA_mdparty, 16),
    DATANUM(DATA_mdparty, 17), DATANUM(DATA_mdparty, 30),
    DATANUM(DATA_mdparty, 31), DATANUM(DATA_mdparty, 32),
    DATANUM(DATA_mdparty, 33), DATANUM(DATA_mdparty, 34),
    DATANUM(DATA_mdparty, 35), DATANUM(DATA_mdparty, 27),
    DATANUM(DATA_mdparty, 21), DATANUM(DATA_mdparty, 20),
    DATANUM(DATA_mdparty, 22), DATANUM(DATA_mdparty, 23),
    DATANUM(DATA_mdparty, 18), DATANUM(DATA_mdparty, 19),
    DATANUM(DATA_mdparty, 24), DATANUM(DATA_mdparty, 25),
    DATANUM(DATA_mdparty, 78), DATANUM(DATA_mdparty, 29),
};
s16 lbl_1_data_9C[17] = {
    2, 6, 6, 4, 2, 2, 2, 2, 4, 4, 4, 4, 5, 2, 4, 4, 1,
};
MDSPR_INFO lbl_1_data_C0[58] = {
    { 0, 0, 0, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 0, 15, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 1, 16, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 2, 17, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 3, 18, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 4, 19, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 1, 5, 20, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 0, 21, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 1, 21, 0, 2, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 2, 21, 0, 5, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 3, 21, 0, 7, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 4, 21, 0, 4, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 2, 5, 21, 0, 6, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 3, 0, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 1, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 2, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 3, 3, 22, 0, 0, { 0.0f, 0.0f }, { 0.8f, 0.8f }, 0.0f },
    { 4, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 4, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 5, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 6, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 0, 22, 9, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 7, 1, 23, 10, 0, { 22.0f, -24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 8, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 8, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 8, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 8, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 9, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 9, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 9, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 9, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 10, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 10, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 10, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 10, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 11, 0, 25, 9, 0, { -44.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 11, 1, 24, 9, 10, { -25.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 11, 2, 24, 9, 0, { -4.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 11, 3, 23, 10, 1, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 0, 27, 9, 0, { -35.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 1, 27, 9, 0, { 5.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 2, 27, 9, 0, { 85.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 3, 27, 9, 0, { 125.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 12, 4, 26, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 13, 0, 28, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 13, 1, 29, 10, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 14, 0, 25, 9, 0, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 14, 1, 24, 9, 10, { -3.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 14, 2, 24, 9, 0, { 18.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 14, 3, 23, 10, 2, { 0.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 15, 0, 25, 9, 0, { -22.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 15, 1, 24, 9, 10, { -3.0f, 25.0f }, { 0.8f, 0.8f }, 0.0f },
    { 15, 2, 24, 9, 0, { 18.0f, 24.0f }, { 0.9f, 0.9f }, 0.0f },
    { 15, 3, 23, 10, 2, { 0.0f, 24.0f }, { 1.0f, 1.0f }, 0.0f },
    { 16, 0, 30, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
    { 0, 1, 31, 0, 0, { 0.0f, 0.0f }, { 1.0f, 1.0f }, 0.0f },
};
u32 lbl_1_data_800[41] = {
    DATANUM(DATA_mdparty, 39), DATANUM(DATA_mdparty, 40),
    DATANUM(DATA_mdparty, 41), DATANUM(DATA_mdparty, 42),
    DATANUM(DATA_mdparty, 43), DATANUM(DATA_mdparty, 44),
    DATANUM(DATA_mdparty, 45), DATANUM(DATA_mdparty, 46),
    DATANUM(DATA_mdparty, 47), DATANUM(DATA_mdparty, 48),
    DATANUM(DATA_mdparty, 49), DATANUM(DATA_mdparty, 50),
    DATANUM(DATA_mdparty, 51), DATANUM(DATA_mdparty, 52),
    DATANUM(DATA_mdparty, 53), DATANUM(DATA_mdparty, 54),
    DATANUM(DATA_mdparty, 55), DATANUM(DATA_mdparty, 56),
    DATANUM(DATA_mdparty, 37), DATANUM(DATA_mdparty, 38),
    DATANUM(DATA_mdparty, 57), DATANUM(DATA_mdparty, 58),
    DATANUM(DATA_mdparty, 59), DATANUM(DATA_mdparty, 60),
    DATANUM(DATA_mdparty, 61), DATANUM(DATA_mdparty, 62),
    DATANUM(DATA_mdparty, 63), DATANUM(DATA_mdparty, 64),
    DATANUM(DATA_mdparty, 65), DATANUM(DATA_mdparty, 66),
    DATANUM(DATA_mdparty, 67), DATANUM(DATA_mdparty, 68),
    DATANUM(DATA_mdparty, 69), DATANUM(DATA_mdparty, 70),
    DATANUM(DATA_mdparty, 71), DATANUM(DATA_mdparty, 72),
    DATANUM(DATA_mdparty, 73), DATANUM(DATA_mdparty, 74),
    DATANUM(DATA_mdparty, 75), DATANUM(DATA_mdparty, 76),
    DATANUM(DATA_mdparty, 77),
};
HuVecF lbl_1_data_8A4[67] = {
    { 0.0f, 275.0f, 0.0f },
    { -200.0f, 325.0f, 0.0f },
    { -120.0f, 225.0f, 0.0f },
    { -40.0f, 325.0f, 0.0f },
    { 40.0f, 225.0f, 0.0f },
    { 120.0f, 325.0f, 0.0f },
    { 200.0f, 225.0f, 0.0f },
    { -200.0f, 275.0f, 0.0f },
    { -120.0f, 275.0f, 0.0f },
    { -40.0f, 275.0f, 0.0f },
    { 40.0f, 275.0f, 0.0f },
    { 120.0f, 275.0f, 0.0f },
    { 200.0f, 275.0f, 0.0f },
    { 0.0f, 275.0f, -800.0f },
    { -195.0f, 380.0f, 0.0f },
    { -65.0f, 380.0f, 0.0f },
    { 65.0f, 380.0f, 0.0f },
    { 195.0f, 380.0f, 0.0f },
    { -130.0f, 240.0f, 0.0f },
    { 0.0f, 240.0f, 0.0f },
    { 130.0f, 240.0f, 0.0f },
    { -195.0f, 100.0f, 0.0f },
    { -65.0f, 100.0f, 0.0f },
    { 65.0f, 100.0f, 0.0f },
    { 195.0f, 100.0f, 0.0f },
    { -180.0f, 65.0f, 0.0f },
    { -60.0f, 65.0f, 0.0f },
    { 60.0f, 65.0f, 0.0f },
    { 180.0f, 65.0f, 0.0f },
    { -180.0f, 125.0f, 0.0f },
    { -60.0f, 125.0f, 0.0f },
    { 60.0f, 125.0f, 0.0f },
    { 180.0f, 125.0f, 0.0f },
    { -180.0f, 275.0f, 0.0f },
    { -60.0f, 275.0f, 0.0f },
    { 60.0f, 275.0f, 0.0f },
    { 180.0f, 275.0f, 0.0f },
    { -340.0f, 375.0f, 0.0f },
    { -340.0f, 240.0f, 0.0f },
    { 340.0f, 375.0f, 0.0f },
    { 340.0f, 240.0f, 0.0f },
    { -225.0f, 300.0f, 0.0f },
    { -75.0f, 300.0f, 0.0f },
    { 75.0f, 300.0f, 0.0f },
    { 225.0f, 300.0f, 0.0f },
    { -180.0f, 175.0f, 0.0f },
    { -60.0f, 175.0f, 0.0f },
    { 60.0f, 175.0f, 0.0f },
    { 180.0f, 175.0f, 0.0f },
    { -220.0f, 175.0f, 0.0f },
    { -100.0f, 175.0f, 0.0f },
    { 100.0f, 175.0f, 0.0f },
    { 220.0f, 175.0f, 0.0f },
    { -220.0f, 65.0f, 0.0f },
    { -100.0f, 65.0f, 0.0f },
    { 100.0f, 65.0f, 0.0f },
    { 220.0f, 65.0f, 0.0f },
    { -220.0f, 125.0f, 0.0f },
    { -100.0f, 125.0f, 0.0f },
    { 100.0f, 125.0f, 0.0f },
    { 220.0f, 125.0f, 0.0f },
    { -160.0f, 0.0f, 0.0f },
    { 160.0f, 0.0f, 0.0f },
    { -225.0f, 275.0f, 0.0f },
    { -75.0f, 275.0f, 0.0f },
    { 75.0f, 275.0f, 0.0f },
    { 225.0f, 275.0f, 0.0f },
};
s32 lbl_1_data_BC8 = -1;
char lbl_1_data_BCC[] = "# ========== win callback :: %d\n";
s16 lbl_1_data_BEE[7] = { 0, 0, 0, 0, 0, 0, 0 };
u32 lbl_1_data_BFC[14] = {
    DATA_w01, DATA_w02, DATA_w03, DATA_w04, DATA_w05, DATA_w06, DATA_w10, DATA_w01n,
    DATA_w02n, DATA_w03n, DATA_w04n, DATA_w05n, DATA_w06n, DATA_w10n,
};
char lbl_1_data_C34[] =
    "\n>>>>>>>>>> mdpartydll :: READ INIT!! <<<<<<<<<<\n";
char lbl_1_data_C66[] = "0x%x :: _effect\n";
char lbl_1_data_C77[] = "0x%x :: _gamemes\n";
char lbl_1_data_C89[] = "0x%x :: _mgconst\n";
char lbl_1_data_C9B[] = "0x%x :: _win\n";
char lbl_1_data_CA9[] = "\n";
char lbl_1_data_CAB[] =
    "\n>>>>>>>>>> mdpartydll :: READ END!! <<<<<<<<<<\n";
char lbl_1_data_CDC[] = ">>>>>>>>>> mdpartydll :: objsetup!! <<<<<<<<<<\n";
char lbl_1_data_D0C[] = "\n>>>>>>>>>> mdpartydll :: ovlreturn!! <<<<<<<<<<\n";
char lbl_1_data_D3E[] = "-----===== CAMERA TEST =====-----";
char lbl_1_data_D60[] = "CENTER : %.2f, %.2f, %.2f";
char lbl_1_data_D7A[] = "ROT    : %.2f, %.2f, %.2f";
char lbl_1_data_D94[] = "ZOOM   : %.2f";
s16 lbl_1_data_DA2[3] = { -1, -1, 0 };
s32 lbl_1_data_DA8[3] = { -1, -1, -1 };
char lbl_1_data_DB4[] = "gN00m1-itemhook_R";
char lbl_1_data_DC6[] = "gN01m1-itemhook_R";
char lbl_1_data_DD8[] = "dpic00";
char lbl_1_data_DDF[] = "dpic01";
s16 lbl_1_data_DE6[6] = { 0, 0, 0, 0, 0, 0 };
char lbl_1_data_DF2[] = "ys77120";
char lbl_1_data_DFA[] = "ys77121";
char lbl_1_data_E02[] = "ys77060";
char lbl_1_data_E0A[] = "ys77061";
LBL_1_DATA_E12_ENTRY lbl_1_data_E12[11] = {
    { { -1, -1, 1, 5, 4, -1, -1, -1 }, -1 },
    { { -1, -1, 2, 6, 5, 4, 0, -1 }, -1 },
    { { -1, -1, 3, -1, 6, 5, 1, -1 }, -1 },
    { { -1, -1, -1, -1, -1, 6, 2, -1 }, -1 },
    { { 0, 1, 5, 9, 8, 7, -1, -1 }, -1 },
    { { 1, 2, 6, 10, 9, 8, 4, 0 }, -1 },
    { { 2, 3, 10, 10, 10, 9, 5, 1 }, -1 },
    { { -1, 4, 8, -1, -1, -1, -1, -1 }, -1 },
    { { 4, 5, 9, -1, -1, -1, 7, -1 }, -1 },
    { { 5, 6, 10, -1, -1, -1, 8, 4 }, -1 },
    { { 6, -1, -1, -1, -1, -1, 9, 5 }, -1 },
};
char lbl_1_data_ED8[] = "\n-----===== MARIO PARTY 6 :: PARTY MODE =====-----\n\n";

s32 lbl_1_bss_A6C[5];
HU3D_LIGHTID lbl_1_bss_A68[2];
HUWINID lbl_1_bss_A60[4];
MDCAMERA_WORK lbl_1_bss_A10;
LBL_1_BSS_9F4 lbl_1_bss_9F4;
LBL_1_BSS_9BC_ENTRY lbl_1_bss_9BC[4];
ANIMDATA *lbl_1_bss_93C[32];
HUSPR_GROUPID lbl_1_bss_918[17];
HUSPRID lbl_1_bss_8A4[58];
ANIMDATA *lbl_1_bss_800[41];
LBL_1_BSS_288_ENTRY lbl_1_bss_288[25];
LBL_1_BSS_228_ENTRY lbl_1_bss_228[2];
LBL_1_BSS_1C8_ENTRY lbl_1_bss_1C8[2];
LBL_1_BSS_1C8_ENTRY lbl_1_bss_108[4];
HUWINID lbl_1_bss_104[2];
HuVecF lbl_1_bss_D4[4];
HuVecF lbl_1_bss_A4[4];
HuVecF lbl_1_bss_8C[2];
HuVecF lbl_1_bss_80;
HuVecF lbl_1_bss_50[4];
float lbl_1_bss_38[6];
s16 lbl_1_bss_34;
OMOBJ *lbl_1_bss_30;
s16 lbl_1_bss_2E;
s16 lbl_1_bss_2C;
s16 lbl_1_bss_2A;
s16 lbl_1_bss_28;
OMOBJ *lbl_1_bss_24;
LBL_1_BSS_1C lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
