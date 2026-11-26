#ifndef OPTIONDLL_OPTION_H
#define OPTIONDLL_OPTION_H

#include "dolphin/types.h"

typedef struct OptionAudioEntry {
    s8 type; /* Zero selects streamed music, one selects sound effects, and -1 ends a list. */
    s32 marker; /* -1 for audio rows; zero for the terminating row. */
    s32 message; /* Message displayed for this sound test entry. */
    s32 audioId; /* Stream or sound effect identifier. */
} OptionAudioEntry;

void fn_1_EA6C(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_E688(struct OptionAudioEntry *entry);

typedef struct OptionAnimPair {
    s16 firstTextureAnimation; /* Animation slot for the first named texture. */
    s16 secondTextureAnimation; /* Animation slot for the second named texture. */
    s16 modelId; /* Model that owns both texture animation slots. */
} OptionAnimPair;

typedef struct OptionDecaRecord {
    /* Character whose 10-game score row is shown. */
    s16 character;
    /* Combined score saved for the row. */
    s16 finalScore;
    /* Scores for the ten minigames in the row. */
    s16 score[10];
} OptionDecaRecord;

typedef struct OptionPrizeEntry {
    s16 flag; /* Saved prize unlock flag, or -1 for a heading. */
    /* A value of -1 marks the final row in the prize table. */
    s16 tableEndMarker;
    u32 name; /* Message used as the prize name or section heading. */
    u32 description; /* Prize description message, or OPTION_PRIZE_MESSAGE_NONE. */
    s16 id; /* Prize value copied into the collection's prize ID list. */
    s16 unk0E; /* Initialized in the table; the game's code never reads it. */
} OptionPrizeEntry;

typedef struct OptionRecordDisplayEntry {
    s16 minigameId; /* Minigame whose saved record is displayed; -1 ends the list. */
    u8 recordId; /* Saved record field passed to GWRecordGet. */
    u8 displayType; /* Selects the record's numeric or time format. */
    s32 message; /* Message used as the record label. */
} OptionRecordDisplayEntry;

typedef struct OptionSelectionData {
    /* Initially highlighted choice and number of available choices. */
    s16 selectedOption;
    s16 choiceCount;
    /* Texture animation and bank to display for each choice. */
    s16 choiceAnimation[10];
    s16 choiceAnimationBank[10];
} OptionSelectionData;

typedef struct OptionWork {
    s16 rumbleEnabled; /* Saved controller vibration setting. */
    s16 micStatus; /* Microphone setting selected by the player. */
    s16 soundMode; /* Output selection; 0 mono, 1 or 2 use stereo console output. */
    s16 boardRecord[6]; /* Number of plays saved for each board. */
    s16 boardCharacterRecord[6][11]; /* Board play counts for each character. */
    OptionDecaRecord decaRecord[10]; /* Ten saved Decathlon score rows. */
    s32 prizeUnlocked[42]; /* Saved unlock state for each listed prize. */
    s16 prizeId[42]; /* Values copied from the prize table's ID column. */
    u32 prizeName[42]; /* Prize name messages in collection order. */
    u32 prizeDescription[42]; /* Prize description messages in collection order. */
    s32 minigameUnlocked[81]; /* Unlock state for minigames 601 through 681. */
    u32 record[19]; /* Not accessed by the Option Mode menus. */
    u8 consecutiveRecordCount; /* Number of saved consecutive minigame entries. */
    u8 consecutiveRecord[100]; /* Saved minigame sequence for the consecutive record. */
} OptionWork;

#endif
