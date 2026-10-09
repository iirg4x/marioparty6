/* Message Checker REL browser for message directories and entries. */
#include "dolphin.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/window.h"
#include "game/wipe.h"
#include "messdir_enum.h"

#define MESSAGE_WINDOW_DIMENSION_MASK 0xFFF0
#define MESSAGE_KEY_WAIT_BIT_PATTERN 0xFF

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

static void fn_1_110(void);
static void fn_1_188(void);
static void fn_1_828(u32 packedMessageId, HuVec2f *messageSize);

/* Object manager passed to the browser child task when the overlay starts. */
static OMOBJMAN *objman;

void ObjectSetup(void);

/* REL loader entry: runs registered constructors, then starts the checker via ObjectSetup. */
int _prolog(void)
{
    const VoidFunc *constructorEntry = _ctors;

    while (*constructorEntry != 0) {
        (**constructorEntry)();
        constructorEntry++;
    }
    ObjectSetup();
    return 0;
}

/* REL loader entry: runs registered destructors when the checker overlay unloads. */
void _epilog(void)
{
    const VoidFunc *destructorEntry = _dtors;

    while (*destructorEntry != 0) {
        (**destructorEntry)();
        destructorEntry++;
    }
}

/* Called by _prolog after constructors; initializes windows and starts the checker child task. */
void ObjectSetup(void)
{
    OSReport("******* Message Checker *********\n");
    objman = omInitObjMan(50, 8192);
    HuWinInit(0);
    HuPrcChildCreate(fn_1_110, 1000, 12288, 0, objman);
}

/* Message-directory lookup names in display order; fn_1_188 changes '_' to '=' and truncates at
 * '.'. */
char *lbl_1_data_494[] = {
    "001_chara_name", "002_sys_guide", "003_map_name", "004_tag_name",
    "005_mgpack_name", "006_mg_name", "007_staff_name", "008_sbank_item",
    "009_mbook_page", "010_file_select", "012_all_main_menu", "013_opening",
    "014_ending", "020_party_setting", "021_party_results", "030_single_setting",
    "031_single_result", "040_mgm_main_menu", "041_mgm_free", "042_mgm_katinuki",
    "043_mgm_tournament", "044_mgm_decathlon", "045_mgm_renshou", "046_mgm_bingo",
    "060_sbank_main_menu", "061_option_single", "062_option_sound", "070_mic_setting",
    "071_micquiz", "072_micquiz_i", "073_micquiz_q", "074_micquiz_tutorial",
    "075_micquiz_moriage", "076_quiz_chara", "077_micquiz_intro", "079_quiz_lv",
    "090_micgo", "094_micgosuport", "200_Board_ope", "201_Board_star",
    "204_Board_pause", "206_Board_gate", "208_Board_tutorial", "209_Board_single",
    "210_Board_opening", "211_Board_snpc", "215_Board_blast5", "220_Board_w01",
    "221_Board_w02", "222_Board_w03", "223_Board_w04", "224_Board_w05",
    "225_Board_w06", "230_Capsule_Ex01", "231_Capsule_Ex02", "234_Capsule_Ex99",
    "235_Capsule_Ex98", "241_ShopEvent", "242_CapsuleMasu", "243_TeresaMasu",
    "244_MiracleMasu", "245_KettouMasu", "246_DonkeyMasu", "247_KoopaMasu",
    "248_mgc_battle", "500_mg_inst", "598_mg_inst_sys", "600_option_mess",
    "647_mess", "665_mic_min", "677_message", "679_mg_mess",
    "LANGUAGE", "mic_retyping", "saf_test", NULL,
};

/* Language label message IDs, indexed by the current GwLanguage value. */
u32 lbl_1_data_5C4[] = {
    MESSNUM(MESS_LANGUAGE, 1),
    MESSNUM(MESS_LANGUAGE, 1),
    MESSNUM(MESS_LANGUAGE, 4),
    MESSNUM(MESS_LANGUAGE, 7),
    MESSNUM(MESS_LANGUAGE, 10),
    MESSNUM(MESS_LANGUAGE, 13),
};

/* Child task started by ObjectSetup: opens the checker after a wipe, then returns this REL after
 * the browser exits. */
static void fn_1_110(void)
{
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    fn_1_188();

    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 30);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Called by fn_1_110 after the opening wipe; displays directory entries and handles language,
 * navigation, and exit input. */
static void fn_1_188(void)
{
    char entryNumberText[8];
    HuVec2f messageWindowSize;
    char *textCursor;
    s16 messageIndex;
    s16 directoryIndex;
    HUWINID messageWin;
    s16 pressedButtons;
    HUWINID languageWin;
    HUWINID entryNumberWin;
    u32 packedMessageId;
    HUWIN *messageWinData;
    HUWINID directoryWin;
    s16 storedDimension;
    s16 firstMessageIndex;
    BOOL revisitPreviousDirectory;
    s16 entryCount;
    BOOL messageHasKeyWait;

    for (messageIndex = 0; lbl_1_data_494[messageIndex]; messageIndex++) {
        textCursor = lbl_1_data_494[messageIndex];
        while (*textCursor != '\0') {
            if (*textCursor == '_') {
                *textCursor = '=';
            }
            if (*textCursor == '.') {
                *textCursor = '\0';
            }
            textCursor++;
        }
    }

    entryNumberWin = HuWinCreate(478.0f, 32.0f, 82, 42, HUWIN_FRAME_DEFAULT);
    HuWinAttrSet(entryNumberWin, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(entryNumberWin, 0);
    HuWinScaleSet(entryNumberWin, 0.8f, 0.8f);

    directoryWin = HuWinCreate(24.0f, 32.0f, 522, 42, HUWIN_FRAME_DEFAULT);
    HuWinMesSpeedSet(directoryWin, 0);
    HuWinScaleSet(directoryWin, 0.8f, 0.8f);

    languageWin = HuWinCreate(350.0f, 32.0f, 346, 42, HUWIN_FRAME_DEFAULT);
    HuWinMesSpeedSet(languageWin, 0);
    HuWinScaleSet(languageWin, 0.8f, 0.8f);
    HuWinBGTPLvlSet(languageWin, 0.0f);
    HuWinMesSet(languageWin, lbl_1_data_5C4[GwLanguage]);

    directoryIndex = firstMessageIndex = 0;
    revisitPreviousDirectory = FALSE;
    while (lbl_1_data_494[directoryIndex]) {
        HuWinMesSet(directoryWin, MESSNUM_PTR(lbl_1_data_494[directoryIndex]));
        /* The message API takes a directory index in the high half of this packed value. */
        entryCount = HuWinMesMaxNumGet((u32)directoryIndex << 16);
        if (!revisitPreviousDirectory) {
            firstMessageIndex = 0;
        } else {
            revisitPreviousDirectory = FALSE;
            /* Revisit the previous directory at its last message so backing out from entry zero
             * selects the preceding entry across the directory boundary. At directory index 0,
             * the index wraps back to directory 0, whose last message is selected. */
            firstMessageIndex = entryCount - 1;
        }

        for (messageIndex = firstMessageIndex; messageIndex < entryCount; messageIndex++) {
            sprintf(entryNumberText, "%d", messageIndex);
            HuWinMesSet(entryNumberWin, MESSNUM_PTR(entryNumberText));

            packedMessageId = ((u32)directoryIndex << 16) | messageIndex;
            fn_1_828(packedMessageId, &messageWindowSize);
            if (messageWindowSize.x == 255.0f) {
                /* This directory entry has no stored width, so ask the message engine to measure
                 * it. */
                HuWinMesMaxSizeGet(1, &messageWindowSize, packedMessageId);
            } else {
                storedDimension = messageWindowSize.x;
                /* Scale stored dimensions by 21 pixels per width unit and 26 per height unit, then
                 * add a 16-pixel margin and align each dimension to 16 pixels. */
                messageWindowSize.x = (storedDimension * 21 + 31) & MESSAGE_WINDOW_DIMENSION_MASK;
                storedDimension = messageWindowSize.y;
                messageWindowSize.y = (storedDimension * 26 + 31) & MESSAGE_WINDOW_DIMENSION_MASK;
            }

            messageWin = HuWinCreate(HUWIN_POS_CENTER, 200.0f, messageWindowSize.x,
                                     messageWindowSize.y, HUWIN_FRAME_DEFAULT);
            HuWinMesSpeedSet(messageWin, 0);

            messageHasKeyWait = FALSE;
            textCursor = HuWinMesPtrGet(packedMessageId);
            while (*textCursor != '\0') {
                if (*textCursor == MESSAGE_KEY_WAIT_BIT_PATTERN) {
                    messageHasKeyWait = TRUE;
                }
                textCursor++;
            }

            messageWinData = &winData[messageWin];
            /* These keys dismiss an in-message wait and become the browser's navigation input. */
            messageWinData->pushKey = PAD_BUTTON_START | PAD_BUTTON_X | PAD_BUTTON_Y |
                PAD_BUTTON_A | PAD_BUTTON_B | PAD_TRIGGER_L | PAD_TRIGGER_R;
            HuWinMesSet(messageWin, packedMessageId);
            while (messageWinData->stat != HUWIN_STAT_NONE) {
                /* Convert held-button repeat state into a fresh press while the message is being
                 * drawn. */
                HuPadBtnDown[0] = HuPadBtnRep[0];
                HuPrcVSleep();
            }

            /* Only messages containing the key-wait marker use the key stored by the message
             * window; other entries wait below for browser input. */
            if (!messageHasKeyWait) {
                while (!(HuPadBtnRep[0] &
                    (PAD_BUTTON_START | PAD_BUTTON_X | PAD_BUTTON_Y | PAD_BUTTON_A |
                        PAD_BUTTON_B | PAD_TRIGGER_L | PAD_TRIGGER_R))) {
                    HuPrcVSleep();
                }
                messageWinData->activePadKey = HuPadBtnRep[0];
            }

            pressedButtons = messageWinData->activePadKey;
            HuWinKill(messageWin);
            if (pressedButtons & PAD_BUTTON_START) {
                return;
            }
            if (pressedButtons & PAD_TRIGGER_R) {
                break;
            }
            if (pressedButtons & PAD_TRIGGER_L) {
                /* Compensate for the outer increment below so it lands on the preceding
                 * directory. */
                directoryIndex -= 2;
                if (directoryIndex < 0) {
                    directoryIndex = -1;
                }
                break;
            }
            if (pressedButtons & (PAD_BUTTON_X | PAD_BUTTON_Y)) {
                /* X advances English, German, French, Italian, Spanish, then English; Y moves
                 * backward through that cycle. From Japanese, X selects English and Y selects
                 * Spanish. */
                if (pressedButtons & PAD_BUTTON_X) {
                    GwLanguage++;
                    if (GwLanguage > HUWIN_LANG_SPAIN) {
                        GwLanguage = HUWIN_LANG_ENGLISH;
                    }
                } else {
                    GwLanguage--;
                    if (GwLanguage <= HUWIN_LANG_JAPAN) {
                        GwLanguage = HUWIN_LANG_SPAIN;
                    }
                }
                GWLanguageSet(GwLanguage);
                GwCommonOrig.languageNo = GwLanguage;
                /* Apply the new language to game settings and message lookup, reload its text, and
                 * update the label. */
                HuWinMesLanguageSet(GwLanguage);
                HuWinMesRead();
                HuWinMesSet(languageWin, lbl_1_data_5C4[GwLanguage]);
                /* Revisit this entry after the for-loop increment so it appears in the new
                 * language. */
                messageIndex--;
            } else if (pressedButtons & PAD_BUTTON_B) {
                /* Back up one entry; the for-loop increment returns to the preceding message. */
                messageIndex -= 2;
                if (messageIndex < 0) {
                    messageIndex = -1;
                    directoryIndex -= 2;
                    if (directoryIndex < 0) {
                        directoryIndex = -1;
                    }
                    revisitPreviousDirectory = TRUE;
                    break;
                }
            }
            HuPrcVSleep();
        }

        directoryIndex++;
        if (!lbl_1_data_494[directoryIndex]) {
            /* Wrap from the final directory to the first, then yield before browsing continues. */
            directoryIndex = 0;
        }
        HuPrcVSleep();
    }
}

/* Called by fn_1_188 before each entry window; logs out-of-range directory or entry indices, then
 * continues following packed offsets to read the stored dimensions. */
static void fn_1_828(u32 packedMessageId, HuVec2f *messageSize)
{
    u32 directoryIndex = packedMessageId >> 16;
    u32 entryIndex = packedMessageId & 0xFFFF;
    u32 *directoryData = messDataPtr;

    if (directoryIndex >= directoryData[0]) {
        OSReport("Error: Message Dir Over\n");
    }
    directoryData++;
    directoryData += directoryData[directoryIndex] >> 2;

    if (entryIndex >= directoryData[0]) {
        OSReport("Error: Message Number Over\n");
    }
    directoryData++;
    directoryData += directoryData[entryIndex] >> 2;

    messageSize->x = (float)(directoryData[0] >> 16);
    messageSize->y = (float)(directoryData[0] & 0xFFFF);
}
