/* Option Mode audio test menu and its grouped message lists. */
#include "dolphin.h"
#include "game/audio.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include <math.h>
#include "REL/optionDll/sound_test.h"

/* The title-message list ends with an empty zero entry. */
#define OPTION_AUDIO_EMPTY_BIT_PATTERN 0x00000000U
#define OPTION_AUDIO_UNUSED_BIT_PATTERN 0xffffffffU
#define OPTION_AUDIO_EFFECT_TYPE_BIT_PATTERN 0x01000000U
#define OPTION_AUDIO_END_TYPE_BIT_PATTERN 0xff000000U
#define OPTION_AUDIO_STREAM 0
#define OPTION_AUDIO_EFFECT 1
#define OPTION_AUDIO_END (-1)

typedef void (*VoidFunc)(void);

typedef struct OptionAudioEntry {
    s8 type; /* Zero selects streamed music, one selects sound effects, and -1 ends a list. */
    s32 marker; /* -1 for audio rows; zero for the terminating row. */
    s32 message; /* Message displayed for this sound test entry. */
    s32 audioId; /* Stream or sound effect identifier. */
} OptionAudioEntry;

typedef struct OptionMessageEntry {
    s8 type; /* Zero selects streamed music; -1 terminates the list. */
    s32 marker; /* -1 for audio rows; zero for the terminating row. */
    s32 message; /* Message displayed for this sound test entry. */
    s32 audioId; /* Stream or sound effect identifier. */
} OptionMessageEntry;

typedef struct OptionAnimPair {
    s16 firstTextureAnimation; /* First texture animation slot attached to the model. */
    s16 secondTextureAnimation; /* Second texture animation slot attached to the model. */
    s16 modelId; /* Model that owns both texture animation slots. */
} OptionAnimPair;

typedef struct OptionDecaRecord {
    s16 character; /* Character whose Decathlon scores are stored here. */
    s16 finalScore; /* Total score across the Decathlon events. */
    s16 score[10]; /* Score for each of the ten Decathlon events. */
} OptionDecaRecord;

typedef struct OptionPrizeEntry {
    s16 flag; /* Saved prize unlock flag, or -1 for a heading entry. */
    s16 tableEndMarker; /* -1 marks the final row in the prize table. */
    u32 name; /* Message shown as the prize name. */
    u32 description; /* Message shown as the prize description. */
    s16 id; /* Prize value copied into the collection's prize ID list. */
    s16 reserved; /* Unused trailing halfword in the prize record. */
} OptionPrizeEntry;

typedef struct OptionRecordDisplayEntry {
    s16 minigameId; /* Minigame whose record is displayed. */
    u8 recordId; /* Record field read from the saved minigame data. */
    u8 displayType; /* Selects how the record value is formatted. */
    s32 message; /* Message used for the record label. */
} OptionRecordDisplayEntry;

typedef struct OptionSelectionData {
    s16 selectedOption; /* Initially highlighted choice when the submenu opens. */
    s16 choiceCount; /* Number of choices available on this screen. */
    s16 choiceAnimation[10]; /* Texture animation index used for each choice. */
    s16 choiceAnimationBank[10]; /* Texture animation bank used for each choice. */
} OptionSelectionData;

typedef struct OptionWork {
    s16 rumbleEnabled; /* Saved controller vibration setting. */
    s16 micStatus; /* Microphone mode selected by the player. */
    s16 soundMode; /* Output selection: 0 mono, 1 stereo, 2 surround. */
    s16 boardRecord[6]; /* Saved records for the six boards. */
    s16 boardCharacterRecord[6][11]; /* Board records grouped by character. */
    OptionDecaRecord decaRecord[10]; /* Decathlon results for saved players. */
    s32 prizeUnlocked[42]; /* Saved unlock state for each option prize. */
    s16 prizeId[42]; /* Numeric prize values displayed by the collection screen. */
    u32 prizeName[42]; /* Name messages for the collected prizes. */
    u32 prizeDescription[42]; /* Description messages for the collected prizes. */
    s32 minigameUnlocked[81]; /* Saved unlock state for minigames 601 through 681. */
    u32 record[19]; /* Not accessed by the Option Mode menus. */
    u8 consecutiveRecordCount; /* Number of consecutive minigame records saved. */
    u8 consecutiveRecord[100]; /* Zero-based MgDataTbl indices in the saved consecutive-play
                                * sequence. */
} OptionWork;

OptionAudioEntry lbl_1_data_510[8] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 28), 0},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 29), MSM_STREAM_TEST},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 30), MSM_STREAM_TITLE},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 31), MSM_STREAM_MODESEL},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 32), MSM_STREAM_MODE},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 33), MSM_STREAM_W01},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 34), MSM_STREAM_W02},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_590[14] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 36), MSM_STREAM_MGDECA},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 51), MSM_STREAM_MGWARS_INIT},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 35), MSM_STREAM_W07},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 54), 33},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 37), MSM_STREAM_W10},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 52), MSM_STREAM_CARDOP},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 56), MSM_STREAM_MGTOUR},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 57), MSM_STREAM_MGCIRCUIT},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 58), MSM_STREAM_CARD},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 59), MSM_STREAM_SDROOM},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 44), MSM_STREAM_STORY_END},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 53), MSM_STREAM_STAFF},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 55), 34},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_670[13] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 38), MSM_STREAM_BOARD_OP},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 45), MSM_STREAM_BOARD_STAR},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 39), MSM_STREAM_BOARD_BATTLE},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 46), MSM_STREAM_CAPSULE_KOOPA},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 40), MSM_STREAM_BOARD_DONKEY},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 47), MSM_STREAM_CAPSULE_CHANCE},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 41), MSM_STREAM_CAPSULE_HANACHAN},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 48), MSM_STREAM_BOARD_LAST5},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 42), MSM_STREAM_PARTYRESULT},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 49), 22},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 43), 23},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 50), MSM_STREAM_BOARD_STORY_OP},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_740[7] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 60), MSM_STREAM_SD_FLAG},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 61), MSM_STREAM_SD_RABBIT},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 62), 47},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 63), MSM_STREAM_MGINST},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 64), MSM_STREAM_RESULT},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 65), MSM_STREAM_MGMUS_1},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_7B0[14] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 66), MSM_STREAM_MGMUS_2},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 67), MSM_STREAM_MGMUS_3},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 68), MSM_STREAM_MGMUS_4},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 69), MSM_STREAM_MGMUS_5},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 70), MSM_STREAM_MGMUS_6},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 71), MSM_STREAM_MGMUS_12},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 72), MSM_STREAM_MGMUS_KOOPA},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 73), MSM_STREAM_MGMUS_DONKEY},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 74), 56},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 75), MSM_STREAM_MGMUS_7},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 76), MSM_STREAM_MGMUS_8},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 77), MSM_STREAM_MGMUS_9},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 78), MSM_STREAM_MGMUS_10},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_890[14] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 79), MSM_STREAM_MGMUS_LAST},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 80), MSM_STREAM_MGTOUR_END},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 81), 73},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 82), MSM_STREAM_SD_TOUR_WIN},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 83), MSM_STREAM_MGMUS_19},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 84), MSM_STREAM_CAPSULE_WANWAN},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 85), MSM_STREAM_MGMUS_20},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 86), MSM_STREAM_MGMUS_21},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 87), MSM_STREAM_MGMUS_22},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 88), MSM_STREAM_MGMUS_24},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 89), MSM_STREAM_MGWARS},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 90), MSM_STREAM_MGMUS_25},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 91), MSM_STREAM_MGMUS_26},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_970[11] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 92), MSM_STREAM_MGMUS_27},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 93), MSM_STREAM_MGMUS_28},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 94), MSM_STREAM_STORY_BOWSER},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 95), MSM_STREAM_MGMUS_29},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 96), MSM_STREAM_MGMUS_30},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 99), MSM_STREAM_MGMUS_16},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 100), MSM_STREAM_MGMUS_17},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 101), MSM_STREAM_MGMUS_18},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 102), MSM_STREAM_FILESEL},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 103), MSM_STREAM_PARTYRESULT_WAIT},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_A20[8] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 97), MSM_STREAM_BOARD_RESULT_DONKEY},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 105), MSM_STREAM_MGMUS_23},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 104), MSM_STREAM_PARTYRESULT_WINNER},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 98), MSM_STREAM_BOARD_RESULT_KETTOU},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 106), MSM_STREAM_W06},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 107), MSM_STREAM_SD_TOUR_LAST},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 108), 42},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_AA0[12] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 109), 95},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 110), 96},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 111), 97},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 123), 93},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 112), 99},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 113), 100},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 114), 101},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 115), 102},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 116), 103},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 117), 104},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 118), 105},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_B60[5] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 120), 107},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 121), 108},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 122), 109},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 119), 106},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_BB0[5] = {
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 124), 92},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 125), 94},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 126), MSM_STREAM_W04},
    {OPTION_AUDIO_STREAM, -1, MESSNUM(MESS_OPTION_SOUND, 127), MSM_STREAM_W05},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_C00[13] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 142), 949},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 143), 950},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 144), 951},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 145), 952},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 147), 954},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 148), 955},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 149), 941},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 150), 942},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 151), 943},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 152), 944},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 154), 946},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 155), 947},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_CD0[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 573},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 573 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 573 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 573 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 573 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 573 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 573 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 573 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 573 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 573 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 573 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 573 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 573 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_DB0[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 549},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 549 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 549 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 549 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 549 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 549 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 549 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 549 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 549 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 549 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 549 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 549 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 549 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_E90[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 693},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 693 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 693 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 693 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 693 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 693 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 693 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 693 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 693 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 693 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 693 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 693 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 693 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_F70[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 789},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 789 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 789 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 789 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 789 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 789 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 789 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 789 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 789 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 789 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 789 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 789 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 789 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1050[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 741},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 741 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 741 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 741 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 741 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 741 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 741 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 741 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 741 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 741 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 741 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 741 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 741 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1130[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 477},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 477 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 477 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 477 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 477 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 477 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 477 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 477 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 477 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 477 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 477 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 477 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 477 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1210[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 765},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 765 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 765 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 765 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 765 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 765 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 765 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 765 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 765 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 765 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 765 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 765 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 765 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_12F0[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 501},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 501 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 501 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 501 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 501 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 501 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 501 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 501 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 501 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 501 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 501 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 501 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 501 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_13D0[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 717},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 717 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 717 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 717 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 717 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 717 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 717 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 717 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 717 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 717 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 717 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 717 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 717 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_14B0[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 669},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 669 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 669 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 669 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 669 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 669 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 669 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 669 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 669 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 669 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 669 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 669 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 669 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1590[14] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 128), 525},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 130), 525 + 2},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 132), 525 + 4},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 129), 525 + 1},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 133), 525 + 6},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 134), 525 + 7},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 135), 525 + 8},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 136), 525 + 11},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 137), 525 + 12},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 138), 525 + 13},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 140), 525 + 20},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 139), 525 + 15},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 131), 525 + 3},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1670[9] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 156), 971},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 157), 972},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 158), 973},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 159), 974},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 160), 975},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 161), 976},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 162), 977},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 163), 978},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1700[12] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 164), 935},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 165), 936},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 166), 937},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 170), 986},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 171), 987},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 172), 988},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 167), 981},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 168), 982},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 173), 960},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 174), 961},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 175), 962},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_17C0[13] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 176), 999},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 177), 1000},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 178), 956},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 179), 957},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 180), 965},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 181), 966},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 182), 967},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 183), 968},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 184), 931},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 185), 932},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 186), 925},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 187), 926},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1890[7] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 188), 928},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 189), 939},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 190), 929},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 191), 1001},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 192), 992},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 193), 1045},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry lbl_1_data_1900[5] = {
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 194), 0x19},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 195), 0x1A},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 196), 0x1B},
    {OPTION_AUDIO_EFFECT, -1, MESSNUM(MESS_OPTION_SOUND, 197), 0x1C},
    {OPTION_AUDIO_END, 0, 0, 0},
};

OptionAudioEntry *lbl_1_data_1950[29] = {
    lbl_1_data_510, lbl_1_data_590, lbl_1_data_670, lbl_1_data_740,
    lbl_1_data_7B0, lbl_1_data_890, lbl_1_data_970, lbl_1_data_A20,
    lbl_1_data_AA0, lbl_1_data_B60, lbl_1_data_BB0, lbl_1_data_C00,
    lbl_1_data_CD0, lbl_1_data_DB0, lbl_1_data_E90, lbl_1_data_F70,
    lbl_1_data_1050, lbl_1_data_1130, lbl_1_data_1210, lbl_1_data_12F0,
    lbl_1_data_13D0, lbl_1_data_14B0, lbl_1_data_1590, lbl_1_data_1670,
    lbl_1_data_1700, lbl_1_data_17C0, lbl_1_data_1890, lbl_1_data_1900,
    NULL
};

u32 lbl_1_data_19C4[29] = {
    MESSNUM(MESS_OPTION_SOUND, 0), MESSNUM(MESS_OPTION_SOUND, 1),
    MESSNUM(MESS_OPTION_SOUND, 2), MESSNUM(MESS_OPTION_SOUND, 3),
    MESSNUM(MESS_OPTION_SOUND, 4), MESSNUM(MESS_OPTION_SOUND, 5),
    MESSNUM(MESS_OPTION_SOUND, 6), MESSNUM(MESS_OPTION_SOUND, 26),
    MESSNUM(MESS_OPTION_SOUND, 7), MESSNUM(MESS_OPTION_SOUND, 8),
    MESSNUM(MESS_OPTION_SOUND, 27), MESSNUM(MESS_OPTION_SOUND, 20),
    MESSNUM(MESS_OPTION_SOUND, 9), MESSNUM(MESS_OPTION_SOUND, 10),
    MESSNUM(MESS_OPTION_SOUND, 11), MESSNUM(MESS_OPTION_SOUND, 12),
    MESSNUM(MESS_OPTION_SOUND, 13), MESSNUM(MESS_OPTION_SOUND, 14),
    MESSNUM(MESS_OPTION_SOUND, 15), MESSNUM(MESS_OPTION_SOUND, 16),
    MESSNUM(MESS_OPTION_SOUND, 17), MESSNUM(MESS_OPTION_SOUND, 18),
    MESSNUM(MESS_OPTION_SOUND, 19), MESSNUM(MESS_OPTION_SOUND, 21),
    MESSNUM(MESS_OPTION_SOUND, 22), MESSNUM(MESS_OPTION_SOUND, 23),
    MESSNUM(MESS_OPTION_SOUND, 24), MESSNUM(MESS_OPTION_SOUND, 25),
    OPTION_AUDIO_EMPTY_BIT_PATTERN
};

HuVec2f lbl_1_data_1A38[14] = {
    {-120.6f, -60.0f},
    {120.6f, -60.0f},
    {-120.6f, -33.0f},
    {120.6f, -33.0f},
    {-120.6f, -6.0f},
    {120.6f, -6.0f},
    {-120.6f, 21.0f},
    {120.6f, 21.0f},
    {-120.6f, 48.0f},
    {120.6f, 48.0f},
    {-120.6f, 75.0f},
    {120.6f, 75.0f},
    {-120.6f, 102.0f},
    {120.6f, 102.0f}
};

char lbl_1_data_1AA8[1] = {0};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s16 lbl_1_bss_5D4;
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_6A0;
extern s16 lbl_1_bss_AE0[7][2];
extern OptionMessageEntry *lbl_1_bss_AFC;
extern float lbl_1_rodata_10;
extern float lbl_1_rodata_14;
extern float lbl_1_rodata_18;
extern float lbl_1_rodata_1C;
extern float lbl_1_rodata_20;
extern float lbl_1_rodata_24;
extern float lbl_1_rodata_28;
extern float lbl_1_rodata_2C;
extern float lbl_1_rodata_30;
extern float lbl_1_rodata_34;
extern float lbl_1_rodata_38;
extern float lbl_1_rodata_3C;
extern float lbl_1_rodata_40;
extern float lbl_1_rodata_44;
extern float lbl_1_rodata_48;
extern float lbl_1_rodata_4C;
extern float lbl_1_rodata_50;
extern float lbl_1_rodata_54;
extern float lbl_1_rodata_58;
extern float lbl_1_rodata_5C;
extern float lbl_1_rodata_60;
extern float lbl_1_rodata_64;
extern float lbl_1_rodata_68;
extern float lbl_1_rodata_6C;
extern float lbl_1_rodata_70;
extern float lbl_1_rodata_74;
extern float lbl_1_rodata_78;
extern float lbl_1_rodata_7C;
extern float lbl_1_rodata_80;
extern double lbl_1_rodata_88;
extern double lbl_1_rodata_90;
extern float lbl_1_rodata_98;
extern double lbl_1_rodata_A0;
extern double lbl_1_rodata_A8;
extern float lbl_1_rodata_14C;
extern double lbl_1_rodata_308;
extern float lbl_1_rodata_310;
extern float lbl_1_rodata_328;
extern double lbl_1_rodata_330;
extern float lbl_1_rodata_338;
extern double lbl_1_rodata_340;
extern float lbl_1_rodata_118;
extern float lbl_1_rodata_11C;
extern float lbl_1_rodata_120;
extern float lbl_1_rodata_12C;
extern float lbl_1_rodata_124;
extern float lbl_1_rodata_128;
extern float lbl_1_rodata_130;
extern float lbl_1_rodata_134;
extern float lbl_1_rodata_138;
extern float lbl_1_rodata_13C;
extern float lbl_1_rodata_140;
extern float lbl_1_rodata_144;
extern float lbl_1_rodata_148;
extern float lbl_1_rodata_150;
extern float lbl_1_rodata_15C;
extern float lbl_1_rodata_160;
extern float lbl_1_rodata_184;
extern float lbl_1_rodata_170;
extern double lbl_1_rodata_178;
extern float lbl_1_rodata_188;
extern float lbl_1_rodata_198;
extern float lbl_1_rodata_194;
extern float lbl_1_rodata_190;
extern float lbl_1_rodata_18C;
extern float lbl_1_rodata_1F0;
extern float lbl_1_rodata_1F4;
extern float lbl_1_rodata_1F8;
extern float lbl_1_rodata_220;
extern float lbl_1_rodata_224;
extern float lbl_1_rodata_218;
extern float lbl_1_rodata_21C;
extern float lbl_1_rodata_C0;
extern float lbl_1_rodata_110;
extern float lbl_1_rodata_114;
extern float lbl_1_rodata_F8;
extern double lbl_1_rodata_F0;
extern float lbl_1_rodata_1E4;
extern double lbl_1_rodata_1E8;
extern float lbl_1_rodata_2F0;
extern float lbl_1_rodata_2F4;
extern float lbl_1_rodata_2F8;
extern float lbl_1_rodata_2FC;
extern float lbl_1_rodata_300;
extern float lbl_1_rodata_304;
extern float lbl_1_rodata_314;
extern float lbl_1_rodata_318;
extern float lbl_1_rodata_31C;
extern float lbl_1_rodata_320;
extern float lbl_1_rodata_324;
extern float lbl_1_rodata_228;
extern float lbl_1_rodata_22C;
extern float lbl_1_rodata_230;
extern float lbl_1_rodata_234;
extern float lbl_1_rodata_238;
extern float lbl_1_rodata_23C;
extern float lbl_1_rodata_154;
extern float lbl_1_rodata_158;
extern double lbl_1_rodata_200;
extern float lbl_1_rodata_208;
extern float lbl_1_rodata_20C;
extern float lbl_1_rodata_210;
extern float lbl_1_rodata_214;
extern float lbl_1_rodata_180;
extern float lbl_1_rodata_1D0;
extern float lbl_1_rodata_1D4;
extern float lbl_1_rodata_1D8;
extern float lbl_1_rodata_1DC;
extern float lbl_1_rodata_1E0;
extern float lbl_1_rodata_240;
extern float lbl_1_rodata_244;
extern float lbl_1_rodata_248;
extern float lbl_1_rodata_254;
extern double lbl_1_rodata_1A0;
extern double lbl_1_rodata_1A8;
extern float lbl_1_rodata_1B0;
extern double lbl_1_rodata_1B8;
extern float lbl_1_rodata_1C0;
extern float lbl_1_rodata_24C;
extern float lbl_1_rodata_250;
extern float lbl_1_rodata_258[];
extern float lbl_1_rodata_1C4;
extern double lbl_1_rodata_168;

float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_E688(OptionAudioEntry *entry);

extern OptionAudioEntry *lbl_1_bss_B78[30];
extern u32 lbl_1_bss_B00[30];

/* Walks the current group's entry list (lbl_1_bss_AFC) to its -1 terminator, counting the
 * entries. */
#define COUNT_GROUP_ENTRIES(entry, count)            \
    do {                                             \
        (entry) = (OptionAudioEntry *)lbl_1_bss_AFC; \
        (count) = 0;                                 \
        while ((entry)->type != -1) {                \
            (count)++;                               \
            (entry)++;                               \
        }                                            \
    } while (0)

/* Runs when Option Mode selects the sound test; displays grouped audio entries and plays the
 * highlighted choice. */
void fn_1_C484(void)
{
    s16 mainWindow;
    s16 frame;
    s16 column;
    s16 enabledGroupCount;
    s16 itemCount;
    s16 currentGroup;
    s16 previousGroup;
    s16 listOffset;
    s16 selectedCell;
    OptionAudioEntry *entry;
    s16 columnDelta;
    s16 rowDelta;
    float progress;
    float scale;
    float upperArrowScale;
    float lowerArrowScale;

    /* Keep the resource and title lists aligned for each unlocked sound-test group. */
    listOffset = 0;
    frame = 0;
    lbl_1_bss_B78[frame] = lbl_1_data_1950[0];
    lbl_1_bss_B00[frame] = lbl_1_data_19C4[0];
    frame++;
    if (fn_8003D2F0(31)) {
        lbl_1_bss_B78[frame] = lbl_1_data_1950[1];
        lbl_1_bss_B00[frame] = lbl_1_data_19C4[1];
        frame++;
        lbl_1_bss_B78[frame] = lbl_1_data_1950[2];
        lbl_1_bss_B00[frame] = lbl_1_data_19C4[2];
        frame++;
    }
    if (fn_8003D2F0(32)) {
        lbl_1_bss_B78[frame] = lbl_1_data_1950[3];
        lbl_1_bss_B00[frame] = lbl_1_data_19C4[3];
        frame++;
    }
    if (fn_8003D2F0(33)) {
        lbl_1_bss_B78[frame] = lbl_1_data_1950[4];
        lbl_1_bss_B00[frame] = lbl_1_data_19C4[4];
        lbl_1_bss_B78[frame + 1] = lbl_1_data_1950[5];
        lbl_1_bss_B00[frame + 1] = lbl_1_data_19C4[5];
        lbl_1_bss_B78[frame + 2] = lbl_1_data_1950[6];
        lbl_1_bss_B00[frame + 2] = lbl_1_data_19C4[6];
        lbl_1_bss_B78[frame + 3] = lbl_1_data_1950[7];
        lbl_1_bss_B00[frame + 3] = lbl_1_data_19C4[7];
        frame += 4;
    }
    if (fn_8003D2F0(34)) {
        lbl_1_bss_B78[frame] = lbl_1_data_1950[8];
        lbl_1_bss_B00[frame] = lbl_1_data_19C4[8];
        lbl_1_bss_B78[frame + 1] = lbl_1_data_1950[9];
        lbl_1_bss_B00[frame + 1] = lbl_1_data_19C4[9];
        lbl_1_bss_B78[frame + 2] = lbl_1_data_1950[10];
        lbl_1_bss_B00[frame + 2] = lbl_1_data_19C4[10];
        frame += 3;
    }
    if (fn_8003D2F0(35)) {
        lbl_1_bss_B78[frame] = lbl_1_data_1950[12];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[12];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[13];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[13];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[14];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[14];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[15];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[15];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[16];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[16];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[17];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[17];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[18];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[18];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[19];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[19];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[20];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[20];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[21];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[21];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[22];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[22];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[23];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[23];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[24];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[24];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[25];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[25];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[26];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[26];
        lbl_1_bss_B78[frame] = lbl_1_data_1950[27];
        lbl_1_bss_B00[frame++] = lbl_1_data_19C4[27];
    }
    lbl_1_bss_B78[frame] = NULL;
    lbl_1_bss_B00[frame] = 0;
    enabledGroupCount = frame;

    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 4, 4);
    HuSprAttrReset(lbl_1_bss_5D4, 2, 4);
    HuSprBankSet(lbl_1_bss_5D4, 2, 5);
    HuSprPosSet(lbl_1_bss_5D4, 2, 0.0f, -90.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 3, 4);
    HuSprBankSet(lbl_1_bss_5D4, 3, 7);
    HuSprPosSet(lbl_1_bss_5D4, 3, 0.0f, 130.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 5, 4);
    HuSprPosSet(lbl_1_bss_5D4, 5, -205.0f, -110.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 6, 4);
    HuSprPosSet(lbl_1_bss_5D4, 6, 205.0f, -110.0f);
    if (enabledGroupCount <= 1) {
        HuSprColorSet(lbl_1_bss_5D4, 6, 128, 128, 128);
        HuSprColorSet(lbl_1_bss_5D4, 5, 128, 128, 128);
    }

    /* The list shows seven rows in two columns, with one window for each half. */
    mainWindow = HuWinCreate(-10000.0f, 53.0f, 400, 40, 0);
    HuWinPriSet(mainWindow, 10);
    HuWinBGTPLvlSet(mainWindow, 0.0f);
    HuWinAttrSet(mainWindow, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(mainWindow, 0);
    HuWinMesSet(mainWindow, lbl_1_bss_B00[0]);
    for (frame = 0; frame < 7; frame++) {
        lbl_1_bss_AE0[frame][0] = HuWinCreate(46.800003f,
            (float)((frame * 27) + 120), 267, 40, 0);
        lbl_1_bss_AE0[frame][1] = HuWinCreate(288.0f,
            (float)((frame * 27) + 120), 267, 40, 0);
        for (column = 0; column < 2; column++) {
            HuWinPriSet(lbl_1_bss_AE0[frame][column], 10);
            HuWinBGTPLvlSet(lbl_1_bss_AE0[frame][column], 0.0f);
            HuWinMesSpeedSet(lbl_1_bss_AE0[frame][column], 0);
        }
    }

    previousGroup = currentGroup = 0;
    lbl_1_bss_AFC = (OptionMessageEntry *)lbl_1_bss_B78[currentGroup];
    COUNT_GROUP_ENTRIES(entry, itemCount);
    OSReport("%d\n", itemCount);
    HuSprAttrSet(lbl_1_bss_5D4, 2, 4);
    HuSprAttrSet(lbl_1_bss_5D4, 3, 4);
    HuSprScaleSet(lbl_1_bss_5D4, 2, 0.0f, 0.0f);
    if (itemCount > 14) {
        HuSprAttrReset(lbl_1_bss_5D4, 3, 4);
        HuSprScaleSet(lbl_1_bss_5D4, 3, 1.0f, 1.0f);
    }
    fn_1_E538(0);
    HuAudSStreamAllFadeOut(500);
    HuAudFXPlay(1205);
    for (frame = 1; frame <= 10; frame++) {
        progress = (float)frame / 10.0f;
        HuWinPosSet(mainWindow, 288.0f - (200.0f * progress),
            200.0f - (147.0f * progress));
        HuWinScaleSet(mainWindow, progress, progress);
        for (column = 0; column < 7; column++) {
            HuWinPosSet(lbl_1_bss_AE0[column][0],
            288.0f - (241.2f * progress),
            200.0f - (progress * (200.0f - (float)((column * 27) + 120))));
            HuWinScaleSet(lbl_1_bss_AE0[column][0], 0.9f * progress, progress);
            HuWinPosSet(lbl_1_bss_AE0[column][1], 288.0f,
                200.0f - (progress * (200.0f - (float)((column * 27) + 120))));
            HuWinScaleSet(lbl_1_bss_AE0[column][1], 0.9f * progress, progress);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
        HuPrcVSleep();
    }

    selectedCell = 0;
    HuSprAttrReset(lbl_1_bss_5D4, 7, 4);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 8.7f, 1.0f);
    HuSprPosSet(lbl_1_bss_5D4, 7, lbl_1_data_1A38[selectedCell].x, lbl_1_data_1A38[selectedCell].y);
    HuWinDispOn(lbl_1_bss_6A0);
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 16));
    HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 19));

    for (;;) {
        if (enabledGroupCount > 1) {
            if (lbl_802C0008[0] & 0x4000) {
                currentGroup--;
            }
            if (lbl_802C0008[0] & 0x2000) {
                currentGroup++;
            }
        }
        /* Tab selection wraps around and resets the cursor to the first audio entry. */
        if (currentGroup != previousGroup) {
            HuAudFXPlay(1207);
            if (currentGroup < previousGroup) {
                HuSprBankSet(lbl_1_bss_5D4, 5, 1);
            } else {
                HuSprBankSet(lbl_1_bss_5D4, 6, 1);
            }
            if (currentGroup < 0) {
                currentGroup = enabledGroupCount - 1;
            }
            if (currentGroup >= enabledGroupCount) {
                currentGroup = 0;
            }
            lbl_1_bss_AFC = (OptionMessageEntry *)lbl_1_bss_B78[currentGroup];
            HuSprAttrSet(lbl_1_bss_5D4, 7, 4);
            for (frame = 1; frame <= 5; frame++) {
                progress = 1.0 - (float)frame / 5.0f;
                HuSprScaleSet(lbl_1_bss_5D4, 2, progress, progress);
                HuSprScaleSet(lbl_1_bss_5D4, 3, progress, progress);
                HuPrcVSleep();
            }
            selectedCell = 0;
            listOffset = selectedCell;
            HuWinMesSet(mainWindow, lbl_1_bss_B00[currentGroup]);
            fn_1_E538(listOffset);
            HuSprPosSet(lbl_1_bss_5D4, 7,
                lbl_1_data_1A38[selectedCell].x, lbl_1_data_1A38[selectedCell].y);
            COUNT_GROUP_ENTRIES(entry, itemCount);
            HuSprAttrSet(lbl_1_bss_5D4, 2, 4);
            HuSprAttrSet(lbl_1_bss_5D4, 3, 4);
            for (frame = 1; frame <= 5; frame++) {
                progress = (float)frame / 5.0f;
                if (itemCount > 14) {
                    HuSprAttrReset(lbl_1_bss_5D4, 3, 4);
                    HuSprScaleSet(lbl_1_bss_5D4, 3, progress, progress);
                }
                HuPrcVSleep();
            }
            HuSprAttrReset(lbl_1_bss_5D4, 7, 4);
            HuSprBankSet(lbl_1_bss_5D4, 5, 0);
            HuSprBankSet(lbl_1_bss_5D4, 6, 0);
            previousGroup = currentGroup;
        }

        /* Convert repeated stick directions into grid moves. When the final row has only a left
         * cell, right from it selects the preceding right cell, and down from that cell selects the
         * final left cell. */
        rowDelta = 0;
        columnDelta = rowDelta;
        if (lbl_802BFFE0[0] & 1) {
            if (selectedCell & 1) {
                columnDelta = -1;
            }
        } else if (lbl_802BFFE0[0] & 2) {
            if ((selectedCell & 1) == 0) {
                columnDelta = 1;
            }
        } else if (lbl_802BFFE0[0] & 8) {
            if (selectedCell + (listOffset * 2) >= 2) {
                rowDelta = -1;
            }
        } else if ((lbl_802BFFE0[0] & 4) &&
            (selectedCell + (listOffset * 2) < ((itemCount - 1) & 0xFFFE))) {
            rowDelta = 1;
        }
        if ((itemCount == (listOffset * 2) +
            (columnDelta + (selectedCell + (rowDelta * 2)))) && (itemCount & 1)) {
            if (columnDelta == 1) {
                rowDelta = -1;
            } else if (rowDelta == 1) {
                columnDelta = -1;
            } else {
                columnDelta = rowDelta = 0;
            }
        }
        /* Vertical moves shift the visible page when the cursor crosses either edge. */
        if (columnDelta != 0 || rowDelta != 0) {
        HuAudFXPlay(0);
            selectedCell = columnDelta + (rowDelta * 2) + selectedCell;
            if (rowDelta != 0) {
                upperArrowScale = fn_1_F684(lbl_1_bss_5D4, 2);
                lowerArrowScale = fn_1_F684(lbl_1_bss_5D4, 3);

                if (selectedCell < 0) {
                    listOffset--;
                    selectedCell += 2;
                    for (frame = 1; frame <= 5; frame++) {
                        if (listOffset != 0) {
                            scale = 1.0 + 0.3f *
                                sin(3.141592653589793 * (180.0f * ((float)frame / 5.0f)) / 180.0);
                        } else {
                            scale = (float)cos((3.141592653589793 *
                                (double)(90.0f * ((float)frame / 5.0f))) / 180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        if (((listOffset * 2) < (itemCount - 14)) &&
                            (lowerArrowScale < 0.1f)) {
                            scale = (float)sin((3.141592653589793 *
                                (double)(90.0f * ((float)frame / 5.0f))) / 180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                            HuSprAttrReset(lbl_1_bss_5D4, 3, 4);
                        }
                        if (frame == 3) {
                            fn_1_E538(listOffset);
                        }
                        HuPrcVSleep();
                    }
                    if (listOffset == 0) {
                        HuSprAttrSet(lbl_1_bss_5D4, 2, 4);
                    }
                } else if (selectedCell >= 14) {
                    listOffset++;
                    selectedCell -= 2;
                    for (frame = 1; frame <= 5; frame++) {
                    if ((listOffset * 2) < (itemCount - 14)) {
                            scale = 1.0 + 0.3f *
                                sin(3.141592653589793 * (180.0f * ((float)frame / 5.0f)) / 180.0);
                        } else {
                            scale = (float)cos((3.141592653589793 *
                                (double)(90.0f * ((float)frame / 5.0f))) / 180.0);
                        }
                        HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        if ((listOffset != 0) && (upperArrowScale < 0.1f)) {
                            scale = (float)sin((3.141592653589793 *
                                (double)(90.0f * ((float)frame / 5.0f))) / 180.0);
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                            HuSprAttrReset(lbl_1_bss_5D4, 2, 4);
                        }
                        /* Replace the row messages on frame 3 of the five-frame arrow animation. */
                        if (frame == 3) {
                            fn_1_E538(listOffset);
                        }
                        HuPrcVSleep();
                    }
                    if ((listOffset * 2) >= (itemCount - 14)) {
                        HuSprAttrSet(lbl_1_bss_5D4, 3, 4);
                    }
                } else {
                    for (frame = 1; frame <= 5; frame++) {
                        progress = (float)frame / 5.0f;
                        scale = 1.0 + 0.3f *
                            sin(3.141592653589793 * (180.0f * progress) / 180.0);
                        if (rowDelta < 0) {
                            HuSprScaleSet(lbl_1_bss_5D4, 2, scale, scale);
                        } else {
                            HuSprScaleSet(lbl_1_bss_5D4, 3, scale, scale);
                        }
                        if (frame == 3) {
                            fn_1_E538(listOffset);
                        }
                        HuPrcVSleep();
                    }
                }
            }
            HuSprPosSet(lbl_1_bss_5D4, 7,
                lbl_1_data_1A38[selectedCell].x,
                lbl_1_data_1A38[selectedCell].y);
        }

        /* A confirms the highlighted audio entry; B starts the return transition. */
        if (lbl_802C0008[0] & 0x100) {
            fn_1_E688((OptionAudioEntry *)((u8 *)lbl_1_bss_AFC +
            ((selectedCell + (listOffset * 2)) * 16)));
        }
        if ((lbl_802C0008[0] & 0x200) == 0) {
            HuPrcVSleep();
            continue;
        }
        HuAudFXPlay(1206);
        HuWinDispOff(lbl_1_bss_6A0);
        HuAudSStreamAllFadeOut(150);
        for (frame = 1; frame <= 10; frame++) {
            progress = 1.0f - (float)frame / 10.0f;
            HuWinPosSet(mainWindow,
                288.0f - (200.0f * progress), 200.0f - (147.0f * progress));
            HuWinScaleSet(mainWindow, progress, progress);
            for (column = 0; column < 7; column++) {
                HuWinPosSet(lbl_1_bss_AE0[column][0],
                    288.0f - (241.2f * progress),
                    200.0f - (progress * (200.0f - (float)((column * 27) + 120))));
                HuWinScaleSet(lbl_1_bss_AE0[column][0], 0.9f * progress, progress);
                HuWinPosSet(lbl_1_bss_AE0[column][1], 288.0f,
                    200.0f - (progress * (200.0f - (float)((column * 27) + 120))));
                HuWinScaleSet(lbl_1_bss_AE0[column][1], 0.9f * progress, progress);
            }
            HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
            HuPrcVSleep();
        }
        HuAudBGMPlay(MSM_STREAM_W03);
        HuWinKill(mainWindow);
        for (frame = 0; frame < 7; frame++) {
            for (column = 0; column < 2; column++) {
                HuWinKill(lbl_1_bss_AE0[frame][column]);
            }
        }
        for (frame = 0; frame < 105; frame++) {
            HuSprAttrSet(lbl_1_bss_5D4, frame, 4);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f, 1.0f);
        HuSprColorSet(lbl_1_bss_5D4, 6, 255, 255, 255);
        HuSprColorSet(lbl_1_bss_5D4, 5, 255, 255, 255);
        HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
        return;
    }
}

/* Called by the sound test menu to refresh 14 display cells, blanking cells beyond the list. */
void fn_1_E538(s16 page)
{
    OptionMessageEntry *messageEntry = lbl_1_bss_AFC;
    s16 entryCount;
    s16 rowCount;
    s16 row;
    s16 column;

    for (entryCount = 0; messageEntry->type != -1; entryCount++, messageEntry++) {
    }
    rowCount = (entryCount + 1) / 2;
    messageEntry = &lbl_1_bss_AFC[page * 2];
    for (row = 0; row < 7; row++) {
        for (column = 0; column < 2; column++, messageEntry++) {
            if (column + (row * 2) + (page * 2) >= entryCount) {
                HuWinMesSet(lbl_1_bss_AE0[row][column],
                    (s32)lbl_1_data_1AA8);
            } else {
                HuWinMesSet(lbl_1_bss_AE0[row][column], messageEntry->message);
            }
        }
    }
}
