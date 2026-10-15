/* Option Mode setup, menus, records, prizes, and display animations. */
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
#include <math.h>
#include "game/card.h"
#include "game/gamework.h"
#include "game/frand.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/main.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"

#define OPTION_MOTION_BLEND_START 0.0f
#define OPTION_MOTION_RETURN_BLEND_FRAMES 8.0f
#define OPTION_MOTION_TURN_BLEND_FRAMES 10.0f
/* Heading and terminator rows have no prize description to display. */
#define OPTION_PRIZE_MESSAGE_NONE 4294967295U

void fn_1_C3FC(s16 selection);
s16 fn_1_C324(s16 flag, s16 type);

typedef void (*VoidFunc)(void);

typedef struct OptionMessageEntry OptionMessageEntry;

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
    s16 reserved; /* Unused trailing halfword. */
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

OptionPrizeEntry lbl_1_data_0[50] = {
    {-1, 10, MESSNUM(MESS_OPTION_SINGLE, 116), OPTION_PRIZE_MESSAGE_NONE, 10, 0},
    {-1, 11, MESSNUM(MESS_OPTION_SINGLE, 115), OPTION_PRIZE_MESSAGE_NONE, 0, 0},
    {-1, 1, MESSNUM(MESS_OPTION_SINGLE, 104), OPTION_PRIZE_MESSAGE_NONE, -1, 0},
    {1, 12, MESSNUM(MESS_OPTION_SINGLE, 0), MESSNUM(MESS_OPTION_SINGLE, 51), 10, 0},
    {2, 13, MESSNUM(MESS_OPTION_SINGLE, 1), MESSNUM(MESS_OPTION_SINGLE, 52), 15, 0},
    {3, 14, MESSNUM(MESS_OPTION_SINGLE, 2), MESSNUM(MESS_OPTION_SINGLE, 53), 20, 0},
    {4, 15, MESSNUM(MESS_OPTION_SINGLE, 3), MESSNUM(MESS_OPTION_SINGLE, 54), 25, 0},
    {5, 18, MESSNUM(MESS_OPTION_SINGLE, 4), MESSNUM(MESS_OPTION_SINGLE, 55), 30, 0},
    {7, 0, MESSNUM(MESS_OPTION_SINGLE, 6), MESSNUM(MESS_OPTION_SINGLE, 57), 50, 0},
    {8, 0, MESSNUM(MESS_OPTION_SINGLE, 7), MESSNUM(MESS_OPTION_SINGLE, 58), 100, 0},
    {9, 0, MESSNUM(MESS_OPTION_SINGLE, 8), MESSNUM(MESS_OPTION_SINGLE, 59), 100, 0},
    {10, 0, MESSNUM(MESS_OPTION_SINGLE, 9), MESSNUM(MESS_OPTION_SINGLE, 60), 100, 0},
    {-1, 1, MESSNUM(MESS_OPTION_SINGLE, 105), OPTION_PRIZE_MESSAGE_NONE, -1, 0},
    {11, 0, MESSNUM(MESS_OPTION_SINGLE, 10), MESSNUM(MESS_OPTION_SINGLE, 61), 20, 0},
    {12, 0, MESSNUM(MESS_OPTION_SINGLE, 11), MESSNUM(MESS_OPTION_SINGLE, 62), 30, 0},
    {13, 0, MESSNUM(MESS_OPTION_SINGLE, 12), MESSNUM(MESS_OPTION_SINGLE, 63), 10, 0},
    {14, 0, MESSNUM(MESS_OPTION_SINGLE, 13), MESSNUM(MESS_OPTION_SINGLE, 64), 10, 0},
    {15, 0, MESSNUM(MESS_OPTION_SINGLE, 14), MESSNUM(MESS_OPTION_SINGLE, 65), 30, 0},
    {16, 0, MESSNUM(MESS_OPTION_SINGLE, 15), MESSNUM(MESS_OPTION_SINGLE, 66), 30, 0},
    {17, 0, MESSNUM(MESS_OPTION_SINGLE, 16), MESSNUM(MESS_OPTION_SINGLE, 67), 10, 0},
    {18, 0, MESSNUM(MESS_OPTION_SINGLE, 17), MESSNUM(MESS_OPTION_SINGLE, 68), 5, 0},
    {19, 0, MESSNUM(MESS_OPTION_SINGLE, 18), MESSNUM(MESS_OPTION_SINGLE, 69), 50, 0},
    {20, 0, MESSNUM(MESS_OPTION_SINGLE, 19), MESSNUM(MESS_OPTION_SINGLE, 70), 100, 0},
    {-1, 1, MESSNUM(MESS_OPTION_SINGLE, 106), OPTION_PRIZE_MESSAGE_NONE, -1, 0},
    {21, 0, MESSNUM(MESS_OPTION_SINGLE, 20), MESSNUM(MESS_OPTION_SINGLE, 71), 10, 0},
    {22, 0, MESSNUM(MESS_OPTION_SINGLE, 21), MESSNUM(MESS_OPTION_SINGLE, 72), 30, 0},
    {23, 0, MESSNUM(MESS_OPTION_SINGLE, 22), MESSNUM(MESS_OPTION_SINGLE, 73), 10, 0},
    {24, 0, MESSNUM(MESS_OPTION_SINGLE, 23), MESSNUM(MESS_OPTION_SINGLE, 74), 30, 0},
    {25, 0, MESSNUM(MESS_OPTION_SINGLE, 24), MESSNUM(MESS_OPTION_SINGLE, 75), 20, 0},
    {26, 0, MESSNUM(MESS_OPTION_SINGLE, 25), MESSNUM(MESS_OPTION_SINGLE, 76), 30, 0},
    {27, 0, MESSNUM(MESS_OPTION_SINGLE, 26), MESSNUM(MESS_OPTION_SINGLE, 77), 20, 0},
    {28, 0, MESSNUM(MESS_OPTION_SINGLE, 27), MESSNUM(MESS_OPTION_SINGLE, 78), 10, 0},
    {29, 0, MESSNUM(MESS_OPTION_SINGLE, 28), MESSNUM(MESS_OPTION_SINGLE, 79), 20, 0},
    {30, 0, MESSNUM(MESS_OPTION_SINGLE, 29), MESSNUM(MESS_OPTION_SINGLE, 80), 20, 0},
    {-1, 1, MESSNUM(MESS_OPTION_SINGLE, 107), OPTION_PRIZE_MESSAGE_NONE, -1, 0},
    {31, 16, MESSNUM(MESS_OPTION_SINGLE, 30), MESSNUM(MESS_OPTION_SINGLE, 81), 10, 0},
    {32, 17, MESSNUM(MESS_OPTION_SINGLE, 31), MESSNUM(MESS_OPTION_SINGLE, 82), 10, 0},
    {33, 0, MESSNUM(MESS_OPTION_SINGLE, 32), MESSNUM(MESS_OPTION_SINGLE, 83), 15, 0},
    {34, 0, MESSNUM(MESS_OPTION_SINGLE, 33), MESSNUM(MESS_OPTION_SINGLE, 84), 15, 0},
    {35, 0, MESSNUM(MESS_OPTION_SINGLE, 34), MESSNUM(MESS_OPTION_SINGLE, 85), 15, 0},
    {36, 0, MESSNUM(MESS_OPTION_SINGLE, 35), MESSNUM(MESS_OPTION_SINGLE, 86), 30, 0},
    {37, 0, MESSNUM(MESS_OPTION_SINGLE, 36), MESSNUM(MESS_OPTION_SINGLE, 87), 30, 0},
    {38, 0, MESSNUM(MESS_OPTION_SINGLE, 37), MESSNUM(MESS_OPTION_SINGLE, 88), 50, 0},
    {39, 0, MESSNUM(MESS_OPTION_SINGLE, 38), MESSNUM(MESS_OPTION_SINGLE, 89), 50, 0},
    {40, 0, MESSNUM(MESS_OPTION_SINGLE, 39), MESSNUM(MESS_OPTION_SINGLE, 90), 300, 0},
    {-1, 1, MESSNUM(MESS_OPTION_SINGLE, 108), OPTION_PRIZE_MESSAGE_NONE, -1, 0},
    {48, 0, MESSNUM(MESS_OPTION_SINGLE, 47), MESSNUM(MESS_OPTION_SINGLE, 98), 50, 0},
    {49, 0, MESSNUM(MESS_OPTION_SINGLE, 48), MESSNUM(MESS_OPTION_SINGLE, 99), 100, 0},
    {50, 0, MESSNUM(MESS_OPTION_SINGLE, 49), MESSNUM(MESS_OPTION_SINGLE, 100), 300, 0},
    {-1, -1, OPTION_PRIZE_MESSAGE_NONE, OPTION_PRIZE_MESSAGE_NONE, -1, 0},
};

OM_CAMERA_VIEW lbl_1_data_320 = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 7000.0f};

OM_CAMERA_VIEW lbl_1_data_33C = {{0.0f, 65.0f, -800.0f}, {-7.25f, 0.0f, 0.0f}, 2650.0f};

OM_CAMERA_VIEW lbl_1_data_358[2] = { { { 0.0f, 65.0f, -800.0f }, { -7.25f, 0.0f, 0.0f }, 2150.0f },
                                     { { 170.0f, 105.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 100.0f } };

char lbl_1_data_390[] = "";

u32 lbl_1_data_394[7] = {
    MESSNUM(MESS_OPTION_MESS, 24), MESSNUM(MESS_OPTION_MESS, 3),
    MESSNUM(MESS_OPTION_MESS, 11), MESSNUM(MESS_OPTION_MESS, 6),
    MESSNUM(MESS_OPTION_MESS, 5), MESSNUM(MESS_OPTION_MESS, 12),
    MESSNUM(MESS_OPTION_MESS, 64)
};

u32 lbl_1_data_3B0[2] = {MESSNUM(MESS_OPTION_MESS, 8), MESSNUM(MESS_OPTION_MESS, 7)};

u32 lbl_1_data_3B8[5] = {MESSNUM(MESS_OPTION_MESS, 37), MESSNUM(MESS_OPTION_MESS, 38),
    MESSNUM(MESS_OPTION_MESS, 39), MESSNUM(MESS_OPTION_MESS, 40), MESSNUM(MESS_OPTION_MESS, 41)};

u32 lbl_1_data_3CC[3] = {MESSNUM(MESS_OPTION_MESS, 26), MESSNUM(MESS_OPTION_MESS, 25),
    MESSNUM(MESS_OPTION_MESS, 27)};

u32 lbl_1_data_3D8[5] = {MESSNUM(MESS_OPTION_MESS, 29), MESSNUM(MESS_OPTION_MESS, 28),
    MESSNUM(MESS_OPTION_MESS, 30), MESSNUM(MESS_OPTION_MESS, 11),
    MESSNUM(MESS_OPTION_MESS, 12)};

u32 lbl_1_data_3EC[3] = {MESSNUM(MESS_OPTION_MESS, 13), MESSNUM(MESS_OPTION_MESS, 14),
    MESSNUM(MESS_OPTION_MESS, 15)};

u32 lbl_1_data_3F8[5] = {MESSNUM(MESS_OPTION_MESS, 46), MESSNUM(MESS_OPTION_MESS, 47),
    MESSNUM(MESS_OPTION_MESS, 49), MESSNUM(MESS_OPTION_MESS, 48),
    MESSNUM(MESS_OPTION_MESS, 62)};

char lbl_1_data_40C[40] = "******* Option ObjectSetup *********\n";

ANIMDATA *lbl_1_bss_BF8[60];

s16 lbl_1_bss_BF0[4];

u8 lbl_1_bss_B78[120];

u32 lbl_1_bss_B00[30];

OptionMessageEntry *lbl_1_bss_AFC;

s16 lbl_1_bss_AE0[7][2];

/* Scratch space for formatting seven record rows in 30-byte segments. */
char lbl_1_bss_A08[216];

s16 lbl_1_bss_9EC[7][2];

char lbl_1_bss_9D8[5][4];

u32 lbl_1_bss_848[100];

OMOBJ *lbl_1_bss_840[2];

HUPROCESS *lbl_1_bss_83C;

HUPROCESS *lbl_1_bss_838;

s16 lbl_1_bss_6A2[203];

s16 lbl_1_bss_6A0;

s16 lbl_1_bss_66E[25];

s16 lbl_1_bss_63C[25];

OptionAnimPair lbl_1_bss_5D6[17];

s16 lbl_1_bss_5D4;

OptionWork lbl_1_bss_8;

s16 lbl_1_bss_6;

/* A nonzero word in the first four bytes enables pad control of the Option Mode camera. */
u8 lbl_1_bss_0[6];

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

extern char lbl_1_data_472[];
extern char lbl_1_data_479[];
extern char lbl_1_data_450[];
extern char lbl_1_data_462[];
extern char lbl_1_data_46A[];

#define OPTION_SUBSTICK_STEP_MASK 0xF8
void fn_1_A0(void);
void fn_1_804(void);
void fn_1_FDC(void);
void fn_1_1044(void);
void fn_1_EA6C(void);
void fn_1_1084(void);
void fn_1_3AE0(s16 modelId, s16 firstAnimation, s16 firstBank, s16 secondAnimation, s16 secondBank);
s16 fn_1_3318(s16 modelId, OptionSelectionData *selection, u32 *messages);
void fn_1_18B8(s16 layerNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 firstAnimation, s16 firstBank, s16 secondAnimation, s16 secondBank);
BOOL fn_1_F544(s16 modelId, s16 firstAnimation, s16 firstBank, s16 secondAnimation, s16 secondBank);
void fn_1_3BD8(OMOBJ *obj);
s16 fn_1_1DF0(s16 currentOption);

void fn_1_B1A0(s16 mode, s16 argument);
void fn_1_4738(void);
void fn_1_6898(void);
void fn_1_5620(void);
void fn_1_6504(s16 prizeNo);
void fn_1_6590(s16 firstPrize);
void fn_1_7728(s16 firstEntry);
void fn_1_7A74(void);
void fn_1_85E4(s16 item, float progress);
void fn_1_879C(void);
void fn_1_8CA4(s16 recordIndex);
void fn_1_930C(u32 *messages, s16 entryCount, u32 titleMessage);
float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_B02C(s16 first, s16 count);
void fn_1_9F74(void);
void fn_1_C484(void);
void fn_1_E538(s16 page);
extern int HuMCMicGet(void);
int sprintf(char *buffer, const char *format, int value);

/* Runs the overlay constructors, then enters the Option Mode setup routine. */
int _prolog(void) {
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

/* Runs the overlay destructors when Option Mode is unloaded. */
void _epilog(void) {
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* The overlay entry calls this to initialize Option Mode's scene and saved records. */
void fn_1_A0(void)
{
    HuVecF shadowPos;
    HuVecF shadowTarget;
    HuVecF shadowUp;
    s16 lightId;
    s32 recordIndex;
    s32 entryIndex;

    OSReport(lbl_1_data_40C);
    lbl_1_bss_83C = omInitObjMan(50, 8192);
    omSysPauseEnable(FALSE);
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 5.0f, 20.0f,
        15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    Hu3DCameraPerspectiveSet(2, 30.0f, 20.0f,
        15000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    omCameraViewSetMulti(1, &lbl_1_data_320);
    omCameraViewSetMulti(2, &lbl_1_data_33C);
    lightId = (int)Hu3DGLightCreate(0.0f, 100.0f,
        1000.0f, 0.0f, -0.5f,
        -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_840[0] = omAddObjEx(lbl_1_bss_83C, 32730, 0, 0, -1,
        omOutViewMulti);
    lbl_1_bss_840[0]->work[0] = 2;
    omAddObjEx(lbl_1_bss_83C, 0, 32, 32, -1, fn_1_3BD8);
    HuWinInit(0);
    HuPrcChildCreate(fn_1_804, 1000, 12288, 0, lbl_1_bss_83C);
    Hu3DShadowCreate(15.0f, 20.0f,
        5000.0f);
    shadowPos.x = 0.0f;
    shadowPos.y = 1000.0f;
    shadowPos.z = 500.0f;
    shadowUp.y = 1.0f;
    shadowUp.x = shadowUp.z = 0.0f;
    shadowTarget.x = shadowTarget.y = shadowTarget.z = 0.0f;
    Hu3DShadowPosSet(&shadowPos, &shadowUp, &shadowTarget);
    Hu3DShadowTPLvlSet(0.3f);

    lbl_1_bss_8.rumbleEnabled = GwCommon.vibrateF ? TRUE : FALSE;
    lbl_1_bss_8.micStatus = HuMCMicGet();
    /* Console mono forces mono; console stereo preserves a nonzero saved mode and changes saved
     * mono to stereo. */
    if (OSGetSoundMode() == OS_SOUND_MODE_MONO) {
        lbl_1_bss_8.soundMode = GwCommon.outputMode = 0;
    } else if (GwCommon.outputMode != 0) {
        lbl_1_bss_8.soundMode = GwCommon.outputMode;
    } else {
        lbl_1_bss_8.soundMode = GwCommon.outputMode = 1;
    }

    for (recordIndex = 0; recordIndex < 6; recordIndex++) {
        for (entryIndex = 0; entryIndex < 11; entryIndex++) {
            lbl_1_bss_8.boardCharacterRecord[recordIndex][entryIndex] =
                GwCommon.charPlayNum[recordIndex][entryIndex];
        }
        lbl_1_bss_8.boardRecord[recordIndex] = GwCommon.boardPlayNum[recordIndex];
    }
    for (recordIndex = 0; recordIndex < 10; recordIndex++) {
        lbl_1_bss_8.decaRecord[recordIndex].character = GwCommon.decaScore[recordIndex].charNo;
        for (entryIndex = 0; entryIndex < 10; entryIndex++) {
            lbl_1_bss_8.decaRecord[recordIndex].score[entryIndex] =
                GwCommon.decaScore[recordIndex].mgScore[entryIndex];
        }
        lbl_1_bss_8.decaRecord[recordIndex].finalScore =
            GwCommon.decaScore[recordIndex].finalScore;
    }

    for (entryIndex = 2, lbl_1_bss_6 = 0; lbl_1_data_0[entryIndex].tableEndMarker != -1;
         entryIndex++) {
        if (lbl_1_data_0[entryIndex].flag != -1) {
            lbl_1_bss_8.prizeId[lbl_1_bss_6] = lbl_1_data_0[entryIndex].id;
            lbl_1_bss_8.prizeUnlocked[lbl_1_bss_6] =
                GWSinglePrizeSaveFlagGet(lbl_1_data_0[entryIndex].flag);
            lbl_1_bss_8.prizeName[lbl_1_bss_6] = lbl_1_data_0[entryIndex].name;
            lbl_1_bss_8.prizeDescription[lbl_1_bss_6] =
                lbl_1_data_0[entryIndex].description;
            lbl_1_bss_6++;
        }
    }
    for (entryIndex = 0; entryIndex < 81; entryIndex++) {
        lbl_1_bss_8.minigameUnlocked[entryIndex] = GWMgUnlockGet(entryIndex + 601);
    }
    lbl_1_bss_8.consecutiveRecordCount = GwCommon.renshoMgRecordNum;
    for (entryIndex = 0; entryIndex < 100; entryIndex++) {
        lbl_1_bss_8.consecutiveRecord[entryIndex] = GwCommon.renshoMgRecord[entryIndex];
    }
    HuAudBGMPlay(MSM_STREAM_W03);
}

/* The setup child opens the settings hub and dispatches menus until the player exits. */
void fn_1_804(void)
{
    s16 choiceIndex;
    s16 choice;
    s16 soundMode;
    s32 mode;
    s16 currentOption;
    OptionSelectionData selection;

    fn_1_1084();
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
    omCameraViewMoveMulti(2, &lbl_1_data_358[0], 60, 0);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    HuWinExOpen(lbl_1_bss_6A2[0]);
    HuWinDispOff(lbl_1_bss_6A0);
    HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 0));
    HuAudFXPlay(MSM_SE_GUIDE_17);
    fn_1_19B8(1);
    HuWinMesWait(lbl_1_bss_6A2[0]);
    fn_1_19B8(2);
    HuPrcSleep(40);

    currentOption = -1;
    for (;;) {
        currentOption = fn_1_1DF0(currentOption);
        if (currentOption < 0) {
            break;
        }
        switch (currentOption) {
        case 1:
            selection.choiceCount = 2;
            selection.selectedOption = lbl_1_bss_8.rumbleEnabled;
            selection.choiceAnimation[0] = 1;
            selection.choiceAnimation[1] = 0;
            selection.choiceAnimationBank[0] = selection.choiceAnimationBank[1] = 0;
            choice = fn_1_3318(lbl_1_bss_66E[8], &selection,
                lbl_1_data_3B0);
            if (choice != -1) {
                lbl_1_bss_8.rumbleEnabled = choice;
                GwCommon.vibrateF = lbl_1_bss_8.rumbleEnabled ? TRUE : FALSE;
                if (lbl_1_bss_8.rumbleEnabled != 0) {
                    HuPadRumbleSet(0, 20, 20, 0);
                }
                HuWinMesSet(lbl_1_bss_6A2[0],
                    choice != 0 ? MESSNUM(MESS_OPTION_MESS, 9) :
                    MESSNUM(MESS_OPTION_MESS, 10));
                fn_1_19B8(1);
                HuWinMesWait(lbl_1_bss_6A2[0]);
            }
            fn_1_3AE0(lbl_1_bss_66E[8], 0, 0, 0, 1);
            break;
        case 3:
            choice = 0;
            for (;;) {
                selection.choiceCount = 5;
                selection.selectedOption = choice;
                for (choiceIndex = 0; choiceIndex < 5; choiceIndex++) {
                    selection.choiceAnimation[choiceIndex] = 3;
                    selection.choiceAnimationBank[choiceIndex] = choiceIndex;
                }
                if ((choice = fn_1_3318(lbl_1_bss_66E[10], &selection,
                    lbl_1_data_3B8)) == -1) {
                    break;
                }
                switch (choice) {
                case 0:
                    mode = 15;
                    break;
                case 1:
                    mode = 8;
                    break;
                case 2:
                    mode = 1;
                    break;
                case 3:
                    mode = 2;
                    break;
                case 4:
                    mode = 4;
                    break;
                default:
                    break;
                }
                fn_1_B1A0(mode, 0);
            }
            fn_1_3AE0(lbl_1_bss_66E[10], 2, 0, 2, 1);
            break;
        case 0:
            selection.choiceCount = 3;
            selection.selectedOption = lbl_1_bss_8.micStatus;
            selection.choiceAnimation[0] = 6;
            selection.choiceAnimationBank[0] = 0;
            selection.choiceAnimation[1] = 4;
            selection.choiceAnimationBank[1] = 0;
            selection.choiceAnimation[2] = 5;
            selection.choiceAnimationBank[2] = 0;
            choice = fn_1_3318(lbl_1_bss_66E[7], &selection,
                lbl_1_data_3CC);
            if (choice != -1) {
                lbl_1_bss_8.micStatus = choice;
                HuWinMesSet(lbl_1_bss_6A2[0], lbl_1_data_3D8[choice]);
                fn_1_19B8(1);
                HuWinMesWait(lbl_1_bss_6A2[0]);
                HuMCMicSet(lbl_1_bss_8.micStatus);
            }
            if (lbl_1_bss_8.micStatus == 0) {
                fn_1_3AE0(lbl_1_bss_66E[7], 6, 0, 6, 1);
            } else if (lbl_1_bss_8.micStatus == 1) {
                fn_1_3AE0(lbl_1_bss_66E[7], 4, 0, 4, 1);
            } else {
                fn_1_3AE0(lbl_1_bss_66E[7], 5, 0, 5, 1);
            }
            break;
        case 2:
            selection.choiceCount = 3;
            selection.selectedOption = lbl_1_bss_8.soundMode;
            for (choiceIndex = 0; choiceIndex < 3; choiceIndex++) {
                selection.choiceAnimation[choiceIndex] = choiceIndex + 10;
                selection.choiceAnimationBank[choiceIndex] = 0;
            }
            if ((soundMode = fn_1_3318(lbl_1_bss_66E[9], &selection,
                lbl_1_data_3EC)) != -1) {
                if (soundMode == 0) {
                    soundMode = 0;
                    OSSetSoundMode(OS_SOUND_MODE_MONO);
                } else if (soundMode == 1) {
                    soundMode = 1;
                    OSSetSoundMode(OS_SOUND_MODE_STEREO);
                } else {
                    soundMode = 2;
                    OSSetSoundMode(OS_SOUND_MODE_STEREO);
                }
                lbl_1_bss_8.soundMode = soundMode;
                GwCommon.outputMode = soundMode;
                msmSysSetOutputMode(GwCommon.outputMode);
                HuAudFXPlay(MSM_SE_CMN_40);
                HuWinMesSet(lbl_1_bss_6A2[0], (s32)MESSNUM(MESS_OPTION_MESS, 16) + soundMode);
                fn_1_19B8(1);
                HuWinMesWait(lbl_1_bss_6A2[0]);
            }
            if (lbl_1_bss_8.soundMode == 0) {
                fn_1_3AE0(lbl_1_bss_66E[9], 10, 0, 10, 1);
            } else if (lbl_1_bss_8.soundMode == 1) {
                fn_1_3AE0(lbl_1_bss_66E[9], 11, 0, 11, 1);
            } else {
                fn_1_3AE0(lbl_1_bss_66E[9], 12, 0, 12, 1);
            }
            break;
        case 4:
            choice = 0;
            for (;;) {
                if (GWBankFlagGet(7)) {
                    selection.choiceCount = 5;
                } else {
                    selection.choiceCount = 4;
                }
                selection.selectedOption = choice;
                for (choiceIndex = 0; choiceIndex < selection.choiceCount; choiceIndex++) {
                    selection.choiceAnimation[choiceIndex] = 14;
                    selection.choiceAnimationBank[choiceIndex] = choiceIndex;
                }
                choice = fn_1_3318(lbl_1_bss_66E[11], &selection,
                    lbl_1_data_3F8);
                if (choice == -1) {
                    break;
                }
                switch (choice) {
                case 0:
                    fn_1_4738();
                    break;
                case 1:
                    fn_1_6898();
                    break;
                case 2:
                    fn_1_5620();
                    break;
                case 3:
                    fn_1_7A74();
                    break;
                case 4:
                    fn_1_9F74();
                    break;
                }
            }
            fn_1_3AE0(lbl_1_bss_66E[11], 13, 0, 13, 1);
            break;
        case 5:
            fn_1_C484();
            /* The return turn targets the sound-output model with the sound-test texture
             * animations. */
            fn_1_3AE0(lbl_1_bss_66E[9], 8, 0, 8, 1);
            break;
        case 6:
            fn_1_EA6C();
            break;
        default:
            continue;
        }
    }
    fn_1_1044();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Exit children call this to wipe out, save changed settings, and leave Option Mode. */
void fn_1_FDC(void)
{
    HuAudSStreamAllFadeOut(1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 40);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    Hu3DAllKill();
    HuSprClose();
    HuSprInit();
    espInit();
    SLSaveModeExec(FALSE);
    omOvlReturnEx(1, TRUE);
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* The settings loop calls this to start the child process that exits Option Mode. */
void fn_1_1044(void)
{
    HuPrcChildCreate(fn_1_FDC, 1000, 4096, 0, lbl_1_bss_83C);
}

/* The settings child calls this to build its models, sprites, windows, and processes. */
void fn_1_1084(void)
{
    s16 spriteId;
    s16 slotIndex;

    fn_1_F1D8();
    Hu3DModelShadowMapSet(lbl_1_bss_66E[0]);
    Hu3DModelShadowSet(lbl_1_bss_66E[2]);
    Hu3DModelShadowSet(lbl_1_bss_66E[6]);
    Hu3DModelHookSet(lbl_1_bss_66E[2], lbl_1_data_450,
        lbl_1_bss_66E[6]);
    Hu3DMotionSet(lbl_1_bss_66E[2], lbl_1_bss_63C[3]);
    Hu3DModelAttrSet(lbl_1_bss_66E[2], HU3D_MOTATTR_LOOP);
    lbl_1_bss_838 = HuPrcChildCreate(fn_1_19E0, 1000, 4096, 0,
        lbl_1_bss_83C);
    fn_1_19B8(0);
    HuPrcChildCreate(fn_1_18D8, 1000, 4096, 0, lbl_1_bss_83C);
    fn_1_F394();
    for (slotIndex = 0; slotIndex < 17; slotIndex++) {
        Hu3DModelScaleSet(lbl_1_bss_66E[slotIndex + 7], 0.0f,
            0.0f, 0.0f);
        lbl_1_bss_5D6[slotIndex].firstTextureAnimation = Hu3DAnimCreate(lbl_1_bss_BF8[0],
            lbl_1_bss_66E[slotIndex + 7], lbl_1_data_462);
        lbl_1_bss_5D6[slotIndex].secondTextureAnimation = Hu3DAnimCreate(lbl_1_bss_BF8[0],
            lbl_1_bss_66E[slotIndex + 7], lbl_1_data_46A);
        lbl_1_bss_5D6[slotIndex].modelId = lbl_1_bss_66E[slotIndex + 7];
        if (slotIndex >= 7) {
            Hu3DModelLayerSet(lbl_1_bss_66E[slotIndex + 7], 3);
        }
    }
    for (slotIndex = 14; slotIndex <= 23; slotIndex++) {
        Hu3DModelLayerSet(lbl_1_bss_66E[slotIndex], 4);
    }
    Hu3DModelLayerSet(lbl_1_bss_66E[24], 4);
    HuSprExecLayerCameraSet(64, 2, 2);
    lbl_1_bss_5D4 = HuSprGrpCreate(105);

    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 0, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 2);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 1, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 5);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 2, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[23], 100, 7);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 3, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[16], 110, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 4, spriteId);
    HuSprDrawNoSet(lbl_1_bss_5D4, 4, 64);
    spriteId = HuSprCreate(lbl_1_bss_BF8[21], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 5, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[22], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 6, spriteId);
    spriteId = HuSprCreate(lbl_1_bss_BF8[24], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_5D4, 7, spriteId);
    HuSprScaleSet(lbl_1_bss_5D4, 7, 8.7f,
        1.0f);

    for (slotIndex = 0; slotIndex < 11; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[15], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 8, spriteId);
    }
    for (slotIndex = 0; slotIndex < 50; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[19], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 19, spriteId);
    }
    for (slotIndex = 0; slotIndex < 10; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[18], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 69, spriteId);
    }
    for (slotIndex = 0; slotIndex < 10; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[17], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 79, spriteId);
    }
    for (slotIndex = 0; slotIndex < 5; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[20], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 89, spriteId);
    }
    for (slotIndex = 0; slotIndex < 11; slotIndex++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[slotIndex + 37], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_5D4, slotIndex + 94, spriteId);
    }
    HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 240.0f);
    for (slotIndex = 0; slotIndex < 105; slotIndex++) {
        HuSprAttrSet(lbl_1_bss_5D4, slotIndex, HUSPR_ATTR_DISPOFF);
    }
    fn_1_E6F8();

    lbl_1_bss_6A2[0] = HuWinExCreateFrame(HUWIN_POS_CENTER,
        376.0f, 544, 80, -1, 3);
    HuWinAttrSet(lbl_1_bss_6A2[0], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(lbl_1_bss_6A2[0], 0);
    HuWinPadMaskSet(lbl_1_bss_6A2[0], 1);
    lbl_1_bss_6A0 = HuWinCreate(HUWIN_POS_CENTER, 344.0f,
        544, 40, 0);
    HuWinBGTPLvlSet(lbl_1_bss_6A0, 0.0f);
    HuWinAttrSet(lbl_1_bss_6A0, HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSpeedSet(lbl_1_bss_6A0, 0);
    HuWinDispOff(lbl_1_bss_6A0);
    HuWinPriSet(lbl_1_bss_6A0, 10);
    Hu3DCameraLayerHookSet(2, 4, fn_1_18B8);
}

/* Camera 2 calls this before layer 4 to draw that layer with a fresh depth buffer. */
void fn_1_18B8(s16 layerNo)
{
    Hu3DZClear();
}

/* A child created during scene setup rotates the display model while the menu is open. */
void fn_1_18D8(void)
{
    float displayRotationDegrees;

    Hu3DAnimCreate(HuDataSelHeapReadNum(DATANUM(DATA_option, 34), HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_472);
    Hu3DAnimCreate(HuDataSelHeapReadNum(DATANUM(DATA_option, 35), HU_MEMNUM_OVL,
        HEAP_MODEL), lbl_1_bss_66E[1], lbl_1_data_479);
    displayRotationDegrees = 0.0f;
    while (TRUE) {
        Hu3DModelRotSet(lbl_1_bss_66E[1], 0.0f,
            0.0f, displayRotationDegrees);
        displayRotationDegrees -= 0.1f;
        if (displayRotationDegrees < 0.0f) {
            displayRotationDegrees += 360.0f;
        }
        HuPrcVSleep();
    }
}

/* The menu process writes a motion request to the character-animation child's property. */
void fn_1_19B8(s32 motionRequest)
{
    void **property = &lbl_1_bss_838->property;

    *property = (void *)motionRequest;
}

/* A child created during scene setup runs the character motions requested by the menu. */
void fn_1_19E0(void)
{
    HUPROCESS *process = HuPrcCurrentGet();
    u32 *motionRequest = (u32 *)&process->property;
    HuVecF stepPos;
    HuVecF startPos;
    HuVecF targetPos;
    s16 frame;
    float rotation;
    float progress;

    while (TRUE) {
        switch ((s32)*motionRequest) {
        case 1:
            /* Play motion 4, then motion 3, unless a new request replaces this one. */
            Hu3DMotionShiftSet(lbl_1_bss_66E[2], lbl_1_bss_63C[4],
                OPTION_MOTION_BLEND_START, OPTION_MOTION_RETURN_BLEND_FRAMES,
                HU3D_MOTATTR_LOOP);
            for (frame = 0; frame < 40 && *motionRequest == 1U; frame++) {
                HuPrcVSleep();
            }
            if (frame == 40) {
                Hu3DMotionShiftSet(lbl_1_bss_66E[2], lbl_1_bss_63C[3],
                    OPTION_MOTION_BLEND_START, OPTION_MOTION_TURN_BLEND_FRAMES,
                    HU3D_MOTATTR_LOOP);
                for (frame = 0; frame < 10 && *motionRequest == 1; frame++) {
                    HuPrcVSleep();
                }
                if (frame == 10 && *motionRequest == 1) {
                    *motionRequest = 0;
                }
            }
            break;
        case 2:
            /* Move 300 units along X and 50 along Z, turning at both ends of the move. */
            Hu3DModelPosGet(lbl_1_bss_66E[2], &startPos);
            Hu3DMotionShiftSet(lbl_1_bss_66E[2], lbl_1_bss_63C[5],
                OPTION_MOTION_BLEND_START, OPTION_MOTION_RETURN_BLEND_FRAMES,
                HU3D_MOTATTR_LOOP);
            targetPos = startPos;
            targetPos.x += 300.0f;
            targetPos.z += 50.0f;
            rotation = 180.0 *
                (atan2(targetPos.x - startPos.x,
                    targetPos.z - startPos.z) / 3.141592653589793);
            for (frame = 1; frame <= 40; frame++) {
                progress = (float)frame / 40.0f;
                PSVECSubtract((Vec *)&targetPos, (Vec *)&startPos,
                    (Vec *)&stepPos);
                PSVECScale((Vec *)&stepPos, (Vec *)&stepPos, progress);
                PSVECAdd((Vec *)&startPos, (Vec *)&stepPos, (Vec *)&stepPos);
                Hu3DModelPosSetV(lbl_1_bss_66E[2], &stepPos);
                if (frame <= 10) {
                    progress = (float)frame / 10.0f;
                    Hu3DModelRotSet(lbl_1_bss_66E[2], 0.0f,
                        rotation * progress, 0.0f);
                }
                if (frame > 30) {
                    progress = 1.0 -
                        (float)(frame - 30) / 10.0f;
                    Hu3DModelRotSet(lbl_1_bss_66E[2], 0.0f,
                        rotation * progress, 0.0f);
                }
                HuPrcVSleep();
            }
            if (*motionRequest == 2) {
                *motionRequest = 0;
                Hu3DMotionShiftSet(lbl_1_bss_66E[2], lbl_1_bss_63C[3],
                    OPTION_MOTION_BLEND_START, OPTION_MOTION_RETURN_BLEND_FRAMES,
                    HU3D_MOTATTR_LOOP);
            }
            break;
        case 0:
            break;
        }
        HuPrcVSleep();
    }
}

/* The settings loop calls this to display the seven choices and handle player input. */
s16 fn_1_1DF0(s16 selectedOption)
{
    HuVecF screenPositions[7];
    HuVecF position;
    HuVecF worldPosition;
    HUWIN *optionWindow;
    s32 exiting;
    s16 frame;
    s16 index;
    s16 previousOption;
    s16 delta;
    float rotation;
    float rotationSpeed;
    float rotationSum;
    float progress;
    float scale;

    rotation = 0.0f;
    rotationSpeed = 0.0f;
    exiting = 0;
    optionWindow = &winData[lbl_1_bss_6A2[0]];

    for (frame = 0; frame < 7; frame++) {
        screenPositions[frame].x = 123.0f + frame * 55;
        screenPositions[frame].y = 150.0f + ((frame & 1) ? 48 : -48);
        screenPositions[frame].z = 1000.0f;
        Hu3D2Dto3D(&screenPositions[frame], 2, &position);
        if (selectedOption != frame) {
            Hu3DModelPosSetV(lbl_1_bss_66E[frame + 7], &position);
            Hu3DModelScaleSet(lbl_1_bss_66E[frame + 7], 1.2f, 1.2f, 0.5f);
        }
    }

    if (lbl_1_bss_8.rumbleEnabled == 0) {
        fn_1_F410(lbl_1_bss_66E[8], 1, 0, 1, 1);
    } else {
        fn_1_F410(lbl_1_bss_66E[8], 0, 0, 0, 1);
    }
    fn_1_F410(lbl_1_bss_66E[10], 2, 0, 2, 1);
    if (lbl_1_bss_8.micStatus == 0) {
        fn_1_F410(lbl_1_bss_66E[7], 6, 0, 6, 1);
    } else if (lbl_1_bss_8.micStatus == 1) {
        fn_1_F410(lbl_1_bss_66E[7], 4, 0, 4, 1);
    } else {
        fn_1_F410(lbl_1_bss_66E[7], 5, 0, 5, 1);
    }
    if (lbl_1_bss_8.soundMode == 0) {
        fn_1_F410(lbl_1_bss_66E[9], 10, 0, 10, 1);
    } else if (lbl_1_bss_8.soundMode == 1) {
        fn_1_F410(lbl_1_bss_66E[9], 11, 0, 11, 1);
    } else {
        fn_1_F410(lbl_1_bss_66E[9], 12, 0, 12, 1);
    }
    fn_1_F410(lbl_1_bss_66E[11], 13, 0, 13, 1);
    fn_1_F410(lbl_1_bss_66E[12], 8, 0, 8, 1);
    fn_1_F410(lbl_1_bss_66E[13], 7, 0, 7, 1);

    HuSprAttrReset(lbl_1_bss_5D4, 0, HUSPR_ATTR_DISPOFF);
    position.x = -256.0f;
    position.y = -90.0f;
    HuSprPosSet(lbl_1_bss_5D4, 0, position.x, position.y);
    HuSprScaleSet(lbl_1_bss_5D4, 0, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 1, HUSPR_ATTR_DISPOFF);
    position.x = 256.0f;
    HuSprPosSet(lbl_1_bss_5D4, 1, position.x, position.y);
    HuSprScaleSet(lbl_1_bss_5D4, 1, 0.0f, 0.0f);
    HuWinMesSet(lbl_1_bss_6A2[0], (u32)lbl_1_data_390);
    HuAudFXPlay(MSM_SE_CMN_53);
    for (frame = 1; frame <= 10; frame++) {
        progress = (float)frame / 10.0f;
        scale = 1.2f * sin(3.141592653589793 * (90.0 * progress) / 180.0);
        for (index = 0; index < 7; index++) {
            if (selectedOption != index) {
                Hu3DModelScaleSet(lbl_1_bss_66E[index + 7], scale, scale, scale);
                Hu3DModelRotSet(lbl_1_bss_66E[index + 7], 0.0f, 720.0f * progress, 0.0f);
            }
        }
        HuPrcVSleep();
    }

    if (selectedOption == -1) {
        HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 1));
        HuAudFXPlay(MSM_SE_GUIDE_18);
        fn_1_19B8(1);
        HuWinMesWait(lbl_1_bss_6A2[0]);
        selectedOption = 0;
        for (frame = 1; frame <= 10; frame++) {
            progress = (float)frame / 10.0f;
            /* This sine value is overwritten below; model scale grows linearly during the
             * entrance. */
            scale = sin(3.141592653589793 * (90.0 * progress) / 180.0);
            position = screenPositions[selectedOption];
            position.y += progress * ((selectedOption & 1) ? -48 : 48);
            position.z += -200.0 * sin(3.141592653589793 * (90.0 * progress) / 180.0);
            Hu3D2Dto3D(&position, 2, &worldPosition);
            Hu3DModelPosSetV(lbl_1_bss_66E[selectedOption + 7], &worldPosition);
            scale = 0.3f * progress;
            Hu3DModelScaleSet(lbl_1_bss_66E[selectedOption + 7], 1.2f + scale, 1.2f + scale,
                              0.5f + scale);
            HuSprScaleSet(lbl_1_bss_5D4, 0, progress, progress);
            HuSprScaleSet(lbl_1_bss_5D4, 1, progress, progress);
            HuPrcVSleep();
        }
    } else {
        for (frame = 1; frame <= 10; frame++) {
            progress = (float)frame / 10.0f;
            position = screenPositions[selectedOption];
            position.x += (1.0f - progress) * (288.0f - screenPositions[selectedOption].x);
            position.y += (selectedOption & 1) ? -48 : 48;
            position.z += -200.0f;
            Hu3D2Dto3D(&position, 2, &worldPosition);
            Hu3DModelPosSetV(lbl_1_bss_66E[selectedOption + 7], &worldPosition);
            HuSprScaleSet(lbl_1_bss_5D4, 0, progress, progress);
            HuSprScaleSet(lbl_1_bss_5D4, 1, progress, progress);
            HuPrcVSleep();
        }
    }
    HuWinMesSet(lbl_1_bss_6A2[0], lbl_1_data_394[selectedOption]);
    fn_1_19B8(1);
    HuWinDispOn(lbl_1_bss_6A0);
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 10));
    previousOption = selectedOption;

    while (TRUE) {
        if (exiting == 0) {
            delta = 0;
            if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
                delta = -1;
            }
            if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
                delta = 1;
            }
            if (delta != 0) {
                selectedOption += delta;
                if (selectedOption < 0) {
                    selectedOption = 6;
                }
                if (selectedOption >= 7) {
                    selectedOption = 0;
                }
                HuAudFXPlay(MSM_SE_CMN_01);
                position = screenPositions[previousOption];
                Hu3D2Dto3D(&position, 2, &worldPosition);
                Hu3DModelPosSetV(lbl_1_bss_66E[previousOption + 7], &worldPosition);
                Hu3DModelScaleSet(lbl_1_bss_66E[previousOption + 7], 1.2f, 1.2f, 0.5f);
                Hu3DModelRotSet(lbl_1_bss_66E[previousOption + 7], 0.0f, 0.0f, 0.0f);
                for (frame = 1; frame <= 10; frame++) {
                    progress = (float)frame / 10.0f;
                    position = screenPositions[selectedOption];
                    position.y += ((selectedOption & 1) ? -48 : 48) *
                                  sin(3.141592653589793 * (90.0 * progress) / 180.0);
                    position.z += -200.0 * sin(3.141592653589793 * (90.0 * progress) / 180.0);
                    Hu3D2Dto3D(&position, 2, &worldPosition);
                    Hu3DModelPosSetV(lbl_1_bss_66E[selectedOption + 7], &worldPosition);
                    scale = 0.3f * progress;
                    Hu3DModelScaleSet(lbl_1_bss_66E[selectedOption + 7],
                        1.2f + scale, 1.2f + scale, 0.5f + scale);
                    scale = 1.0 + 0.3f * sin(3.141592653589793 * (180.0f * progress) / 180.0);
                    if (delta < 0) {
                        HuSprScaleSet(lbl_1_bss_5D4, 0, scale, scale);
                    } else {
                        HuSprScaleSet(lbl_1_bss_5D4, 1, scale, scale);
                    }
                    HuPrcVSleep();
                }
                HuWinMesSet(lbl_1_bss_6A2[0], lbl_1_data_394[selectedOption]);
                fn_1_19B8(1);
                rotation = 0.0f;
                previousOption = selectedOption;
            }

            if (HuPadBtnDown[0] & PAD_BUTTON_A) {
                HuAudFXPlay(MSM_SE_CMN_02);
                break;
            }
            if ((HuPadBtnDown[0] & PAD_BUTTON_B) || (HuPadBtnDown[0] & PAD_BUTTON_X)) {
                HuAudFXPlay(MSM_SE_CMN_04);
                HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 50));
                HuAudFXPlay(MSM_SE_GUIDE_17);
                HuWinMesWait(lbl_1_bss_6A2[0]);
                HuWinChoiceSet(lbl_1_bss_6A2[0], -1);
                while (optionWindow->stat != 0) {
                    Hu3DModelRotSet(lbl_1_bss_66E[selectedOption + 7],
                        0.0f, rotation, 0.0f);
                    rotationSpeed += 0.05f;
                    if (rotationSpeed > 3.0f) {
                        rotationSpeed = 3.0f;
                    }
                    rotation += rotationSpeed;
                    if (rotation > 360.0f) {
                        rotation -= 360.0f;
                    }
                    HuPrcVSleep();
                }
                if (optionWindow->choice != 0) {
                    HuWinMesSet(lbl_1_bss_6A2[0], lbl_1_data_394[selectedOption]);
                } else {
                    HuPrcChildCreate(fn_1_FDC, 1000, 4096, 0, lbl_1_bss_83C);
                    exiting = 1;
                }
            }
        }
        Hu3DModelRotSet(lbl_1_bss_66E[selectedOption + 7], 0.0f, rotation, 0.0f);
        rotationSpeed += 0.05f;
        if (rotationSpeed > 3.0f) {
            rotationSpeed = 3.0f;
        }
        rotation += rotationSpeed;
        if (rotation > 360.0f) {
            rotation -= 360.0f;
        }
        HuPrcVSleep();
    }

    HuWinMesSet(lbl_1_bss_6A2[0], (u32)lbl_1_data_390);
    rotationSum = 0.0f;
    HuAudFXPlay(MSM_SE_CMN_54);
    for (frame = 1; frame <= 10; frame++) {
        progress = (float)frame / 10.0f;
        for (index = 0; index < 7; index++) {
            if (index == selectedOption) {
                if (rotation != 0.0f) {
                    rotation += (rotation > 180.0f) ? 20.0f : -20.0f;
                }
                if (rotation < 0.0f || rotation > 360.0f) {
                    rotation = 0.0f;
                }
                Hu3DModelRotSet(lbl_1_bss_66E[index + 7], 0.0f, rotation, 0.0f);
                position = screenPositions[index];
                position.x += progress * (288.0f - screenPositions[index].x);
                position.y += (index & 1) ? -48 : 48;
                position.z += -200.0f;
                Hu3D2Dto3D(&position, 2, &worldPosition);
                Hu3DModelPosSetV(lbl_1_bss_66E[index + 7], &worldPosition);
            } else {
                rotationSum += 72.0f;
                Hu3DModelRotSet(lbl_1_bss_66E[index + 7], 0.0f, rotationSum, 0.0f);
                scale = cos(3.141592653589793 * (90.0f * progress) / 180.0);
                Hu3DModelScaleSet(lbl_1_bss_66E[index + 7],
                    1.2f * scale, 1.2f * scale, 0.5f * scale);
            }
        }
        scale = 1.0f - progress;
        HuSprScaleSet(lbl_1_bss_5D4, 0, scale, scale);
        HuSprScaleSet(lbl_1_bss_5D4, 1, scale, scale);
        HuPrcVSleep();
    }
    for (frame = 0; frame < 105; frame++) {
        HuSprAttrSet(lbl_1_bss_5D4, frame, HUSPR_ATTR_DISPOFF);
    }
    return selectedOption;
}

/* The settings loop opens a model's choices, returning the chosen index or -1 on cancel. */
s16 fn_1_3318(s16 modelId, OptionSelectionData *selection, u32 *messages)
{
    s16 previousChoiceIndex;
    s16 selectedIndex;
    s16 frame;
    s16 direction;
    BOOL animationsAlreadySelected;
    HuVecF position;
    float progress;
    float scale;

    HuSprAttrReset(lbl_1_bss_5D4, 0, HUSPR_ATTR_DISPOFF);
    position.x = -93.0f;
    position.y = -90.0f;
    HuSprPosSet(lbl_1_bss_5D4, 0, position.x, position.y);
    HuSprScaleSet(lbl_1_bss_5D4, 0, 0.0f, 0.0f);
    HuSprAttrReset(lbl_1_bss_5D4, 1, HUSPR_ATTR_DISPOFF);
    position.x = 93.0f;
    HuSprPosSet(lbl_1_bss_5D4, 1, position.x, position.y);
    HuSprScaleSet(lbl_1_bss_5D4, 1, 0.0f, 0.0f);
    HuWinDispOff(lbl_1_bss_6A0);

    selectedIndex = previousChoiceIndex = selection->selectedOption;
    animationsAlreadySelected = fn_1_F544(modelId,
        selection->choiceAnimation[selectedIndex],
        selection->choiceAnimationBank[selectedIndex],
        selection->choiceAnimation[selectedIndex],
        selection->choiceAnimationBank[selectedIndex]);
    for (frame = 1; frame <= 10; frame++) {
        progress = (float)frame / 10.0f;
        if (animationsAlreadySelected == 0) {
            Hu3DModelRotSet(modelId, 0.0f,
                180.0f * progress, 0.0f);
            if (frame == 5) {
                fn_1_F410(modelId,
                    selection->choiceAnimation[selectedIndex],
                    selection->choiceAnimationBank[selectedIndex],
                    selection->choiceAnimation[selectedIndex],
                    selection->choiceAnimationBank[selectedIndex]);
            }
        }
        HuSprScaleSet(lbl_1_bss_5D4, 0, progress, progress);
        HuSprScaleSet(lbl_1_bss_5D4, 1, progress, progress);
        HuPrcVSleep();
    }
    HuWinMesSet(lbl_1_bss_6A2[0], messages[selectedIndex]);
    fn_1_19B8(1);
    HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 10));
    HuWinDispOn(lbl_1_bss_6A0);

    for (;;) {
        direction = 0;
        if (HuPadDStkRep[0] & PAD_BUTTON_LEFT) {
            direction = -1;
        }
        if (HuPadDStkRep[0] & PAD_BUTTON_RIGHT) {
            direction = 1;
        }
        if (direction != 0) {
            selectedIndex += direction;
            if (selectedIndex < 0) {
                selectedIndex = selection->choiceCount - 1;
            }
            if (selectedIndex >= selection->choiceCount) {
                selectedIndex = 0;
            }
            HuAudFXPlay(MSM_SE_CMN_01);
            for (frame = 1; frame <= 10; frame++) {
                progress = (float)frame / 10.0f;
                Hu3DModelRotSet(modelId, 0.0f,
                    180.0f * progress * (direction < 0 ? -1 : 1),
                    0.0f);
                if (frame == 5) {
                    fn_1_F410(modelId, selection->choiceAnimation[selectedIndex],
                              selection->choiceAnimationBank[selectedIndex],
                              selection->choiceAnimation[selectedIndex],
                              selection->choiceAnimationBank[selectedIndex]);
                }
                scale = sin(3.141592653589793 *
                    (180.0f * progress) / 180.0) *
                    0.30000001192092896 + 1.0;
                if (direction < 0) {
                    HuSprScaleSet(lbl_1_bss_5D4, 0, scale, scale);
                } else {
                    HuSprScaleSet(lbl_1_bss_5D4, 1, scale, scale);
                }
                HuPrcVSleep();
            }
            Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
            HuWinMesSet(lbl_1_bss_6A2[0], messages[selectedIndex]);
            fn_1_19B8(1);
            previousChoiceIndex = selectedIndex;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_A) {
            HuAudFXPlay(MSM_SE_CMN_03);
            break;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_B) {
            HuAudFXPlay(MSM_SE_CMN_04);
            selectedIndex = -1;
            break;
        }
        if (HuPadBtnDown[0] & PAD_BUTTON_X) {
            HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 50));
            HuWinMesWait(lbl_1_bss_6A2[0]);
            frame = HuWinChoiceGet(lbl_1_bss_6A2[0], -1);
            if (frame == 0) {
                HuPrcChildCreate(fn_1_FDC, 1000, 4096, 0, lbl_1_bss_83C);
                for (;;) {
                    HuPrcVSleep();
                }
            }
            HuWinMesSet(lbl_1_bss_6A2[0], messages[selectedIndex]);
        }
        HuPrcVSleep();
    }

    HuWinDispOff(lbl_1_bss_6A0);
    for (frame = 1; frame <= 10; frame++) {
        progress = (float)frame / 10.0f;
        scale = 1.0f - progress;
        HuSprScaleSet(lbl_1_bss_5D4, 0, scale, scale);
        HuSprScaleSet(lbl_1_bss_5D4, 1, scale, scale);
        HuPrcVSleep();
    }
    Hu3DModelRotSet(modelId, 0.0f, 0.0f, 0.0f);
    for (frame = 0; frame < 105; frame++) {
        HuSprAttrSet(lbl_1_bss_5D4, frame, HUSPR_ATTR_DISPOFF);
    }
    return selectedIndex;
}

/* The settings loop turns a menu model and restores its texture animations on frame 8. */
void fn_1_3AE0(s16 modelId, s16 firstAnimation, s16 firstBank, s16 secondAnimation, s16 secondBank)
{
    s16 frame;
    float rotation;

    for (frame = 1; frame <= 10; frame++) {
        rotation = (float)frame / 10.0f;
        Hu3DModelRotSet(modelId, 0.0f, 360.0f * rotation, 0.0f);
        if (frame == 8) {
            fn_1_F410(modelId, firstAnimation, firstBank, secondAnimation, secondBank);
        }
        HuPrcVSleep();
    }
}

/* When camera controls are enabled, the camera object orbits, zooms, and pans using pad 0. */
void fn_1_3BD8(OMOBJ *object)
{
    HuVecF eye;
    HuVecF delta;
    HuVecF up;
    HuVecF roll;
    s8 subStick;
    float cameraRollDegrees;

    if (!(*(s32 *)lbl_1_bss_0)) {
        return;
    }

    CRotM[1].y += 0.1f * (float)HuPadStkX[0];
    CRotM[1].x += 0.1f * (float)HuPadStkY[0];
    CZoomM[1] += (float)(HuPadTrigL[0] / 2);
    CZoomM[1] -= (float)(HuPadTrigR[0] / 2);
    if (CZoomM[1] < 100.0f) {
        CZoomM[1] = 100.0f;
    }

    eye.x = CenterM[1].x + (CZoomM[1] *
        (sin(3.141592653589793 * CRotM[1].y / 180.0) *
         cos(3.141592653589793 * CRotM[1].x / 180.0)));
    eye.y = CenterM[1].y + (CZoomM[1] *
        -sin(3.141592653589793 * CRotM[1].x / 180.0));
    eye.z = CenterM[1].z + (CZoomM[1] *
        (cos(3.141592653589793 * CRotM[1].y / 180.0) *
         cos(3.141592653589793 * CRotM[1].x / 180.0)));

    delta.x = CenterM[1].x - eye.x;
    delta.y = CenterM[1].y - eye.y;
    delta.z = CenterM[1].z - eye.z;

    up.x = sin(3.141592653589793 * CRotM[1].y / 180.0) *
           sin(3.141592653589793 * CRotM[1].x / 180.0);
    up.y = cos(3.141592653589793 * CRotM[1].x / 180.0);
    up.z = cos(3.141592653589793 * CRotM[1].y / 180.0) *
           sin(3.141592653589793 * CRotM[1].x / 180.0);

    cameraRollDegrees = CRotM[1].z;

    roll.x = (up.x * ((delta.x * delta.x) +
               ((1.0f - (delta.x * delta.x)) *
                cos(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.y * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) -
               (delta.z * sin(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.z * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) +
               (delta.y * sin(3.141592653589793 * cameraRollDegrees / 180.0))));

    roll.y = (up.x * (((delta.x * delta.y) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) +
               (delta.z * sin(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.y * ((delta.y * delta.y) +
               ((1.0f - (delta.y * delta.y)) *
                cos(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.z * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) -
               (delta.x * sin(3.141592653589793 * cameraRollDegrees / 180.0))));

    roll.z = (up.x * (((delta.x * delta.z) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) -
               (delta.y * sin(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.y * (((delta.y * delta.z) *
               (1.0 - cos(3.141592653589793 * cameraRollDegrees / 180.0))) +
               (delta.x * sin(3.141592653589793 * cameraRollDegrees / 180.0)))) +
             (up.z * ((delta.z * delta.z) +
               ((1.0f - (delta.z * delta.z)) *
                cos(3.141592653589793 * cameraRollDegrees / 180.0))));

    PSVECCrossProduct(&up, &delta, &delta);
    PSVECNormalize(&delta, &delta);

    subStick = HuPadSubStkX[0] & OPTION_SUBSTICK_STEP_MASK;
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }

    PSVECNormalize(&roll, &delta);

    subStick = -(HuPadSubStkY[0] & OPTION_SUBSTICK_STEP_MASK);
    if (subStick != 0) {
        CenterM[1].x += (delta.x * (float)subStick) * 0.05f;
        CenterM[1].y += (delta.y * (float)subStick) * 0.05f;
        CenterM[1].z += (delta.z * (float)subStick) * 0.05f;
    }
}
