/* Option Mode models, sprites, and shared menu helpers. */
#include "dolphin.h"
#include "game/audio.h"
#include "game/esprite.h"
#include "game/hu3d.h"
#include "game/mgdata.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/saveload.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/window.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include <math.h>
#include <stdio.h>
#include "REL/optionDll/main.h"
#include "REL/optionDll/records.h"
#include "REL/optionDll/sound_test.h"

#define OPTION_MODEL_END ((u32)-1)

typedef void (*VoidFunc)(void);

typedef struct OptionAudioEntry {
    s8 type; /* Zero selects streamed music, one selects sound effects, and -1 ends a list. */
    s32 marker; /* -1 for audio rows; zero for the terminating row. */
    s32 message; /* Message displayed for this sound test entry. */
    s32 audioId; /* Stream or sound effect identifier. */
} OptionAudioEntry;

typedef struct OptionModelEntry {
    u32 dataNum;
    s16 jointModel;
    s16 pad;
    HuVecF pos;
    HuVecF rot;
    HuVecF scale;
    u32 attr1;
    u32 attr2;
    s16 cameraBit;
    u16 unk_32;
} OptionModelEntry;

typedef struct OptionAnimPair {
    s16 animId1;
    s16 animId2;
    s16 modelId;
} OptionAnimPair;

typedef struct OptionDecaRecord {
    s16 character;
    s16 finalScore;
    s16 score[10];
} OptionDecaRecord;

typedef struct OptionPrizeEntry {
    s16 flag;
    s16 tableEndMarker; /* -1 marks the final row in the prize table. */
    u32 name;
    u32 description;
    s16 id;
    s16 unkE;
} OptionPrizeEntry;

typedef struct OptionRecordDisplayEntry {
    s16 minigameId;
    u8 recordId;
    u8 displayType;
    s32 message;
} OptionRecordDisplayEntry;

typedef struct OptionSelectionData {
    s16 selectedIndex;
    s16 optionCount;
    s16 animationA[10];
    s16 choiceAnimationBank[10];
} OptionSelectionData;

typedef struct OptionWork {
    s16 rumbleEnabled; /* Saved controller vibration setting. */
    s16 micStatus;
    s16 soundMode;
    s16 boardRecord[6];
    s16 boardCharacterRecord[6][11];
    OptionDecaRecord decaRecord[10];
    s32 prizeUnlocked[42];
    s16 prizeId[42];
    u32 prizeName[42];
    u32 prizeDescription[42];
    s32 minigameUnlocked[81];
    u32 record[19];
    u8 consecutiveRecordCount;
    u8 consecutiveRecord[100];
} OptionWork;

OptionModelEntry lbl_1_data_1AB0[26] = {
    {DATANUM(0xbe0000, 0), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, ((1 << 30) | (1 << 0)), 2, 0},
    {DATANUM(0xbe0000, 1), -1, 0, {0.00000000f, 0.00000000f, -1000.0f}, {0.00000000f, 0.00000000f,
        0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 3), -1, 0, {0.00000000f, 0.00000000f, 50.0f}, {0.00000000f, 0.00000000f,
        0.00000000f}, {1.3f, 1.3f, 1.3f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 4), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f,
        0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 5), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f,
        0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 6), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f,
        0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 7), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {1.0f, 1.0f, 1.0f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(0xbe0000, 8), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f,
        0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U,
        ((1 << 30) | (1 << 0)), 2, 0},
    {OPTION_MODEL_END, -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0}
};

s32 lbl_1_data_2060[60] = {
    DATANUM(0xbe0000, 9), DATANUM(0xbe0000, 10),
    DATANUM(0xbe0000, 11), DATANUM(0xbe0000, 12),
    DATANUM(0xbe0000, 13), DATANUM(0xbe0000, 14),
    DATANUM(0xbe0000, 15), DATANUM(0xbe0000, 16),
    DATANUM(0xbe0000, 17), DATANUM(0xbe0000, 18),
    DATANUM(0xbe0000, 19), DATANUM(0xbe0000, 20),
    DATANUM(0xbe0000, 21), DATANUM(0xbe0000, 22),
    DATANUM(0xbe0000, 23), DATANUM(0xbe0000, 24),
    DATANUM(0xbe0000, 25), DATANUM(0xbe0000, 26),
    DATANUM(0xbe0000, 27), DATANUM(0xbe0000, 28),
    DATANUM(0xbe0000, 29), DATANUM(0xbe0000, 30),
    DATANUM(0xbe0000, 31), DATANUM(0xbe0000, 33),
    DATANUM(0xbe0000, 32), DATANUM(0xbe0000, 36),
    DATANUM(0xbe0000, 37), DATANUM(0xbe0000, 38),
    DATANUM(0xbe0000, 39), DATANUM(0xbe0000, 40),
    DATANUM(0xbe0000, 41), DATANUM(0xbe0000, 42),
    DATANUM(0xbe0000, 43), DATANUM(0xbe0000, 44),
    DATANUM(0xbe0000, 45), DATANUM(0xbe0000, 46),
    DATANUM(0xbe0000, 47), DATANUM(0xbe0000, 48),
    DATANUM(0xbe0000, 49), DATANUM(0xbe0000, 50),
    DATANUM(0xbe0000, 51), DATANUM(0xbe0000, 52),
    DATANUM(0xbe0000, 53), DATANUM(0xbe0000, 54),
    DATANUM(0xbe0000, 55), DATANUM(0xbe0000, 56),
    DATANUM(0xbe0000, 57), DATANUM(0xbe0000, 58),
    DATANUM(0xbe0000, 60), DATANUM(0xbe0000, 61),
    DATANUM(0xbe0000, 62), DATANUM(0xbe0000, 63),
    DATANUM(0xbe0000, 64), DATANUM(0xbe0000, 65),
    DATANUM(0xbe0000, 66), DATANUM(0xbe0000, 67),
    DATANUM(0xbe0000, 68), DATANUM(0xbe0000, 69),
    DATANUM(0xbe0000, 70), DATANUM(0xbe0000, 71)
};

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
extern s16 lbl_1_bss_5D4;
extern OptionAnimPair lbl_1_bss_5D6[17];
extern s16 lbl_1_bss_63C[];
extern s16 lbl_1_bss_66E[];
extern s16 lbl_1_bss_6A2[];
extern s16 lbl_1_bss_6A0;
extern ANIMDATA *lbl_1_bss_BF8[];
extern s16 lbl_1_bss_BF0[4];

void fn_1_EA6C(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
float fn_1_F684(s16 groupId, s16 memberNo);

/* The settings list calls this when the player previews a sound entry. */
void fn_1_E688(OptionAudioEntry *entry)
{
    HuAudSStreamAllFadeOut(150);
    /* This call has no playback effect; the preceding call fades all streamed tracks. */
    HuAudStreamFadeOut(10);
    if (entry->type == 0) {
        HuPrcSleep(30);
        HuAudSStreamPlay((s16)entry->audioId);
    } else {
        HuAudFXPlay(entry->audioId);
        HuPrcSleep(20);
    }
}

/* Scene setup calls this to build the microphone test's volume meter sprites. */
void fn_1_E6F8(void)
{
    s16 spriteId;
    s16 i;

    lbl_1_bss_BF0[0] = HuSprGrpCreate(29);
    spriteId = HuSprCreate(lbl_1_bss_BF8[48], 100, 0);
    HuSprGrpMemberSet(lbl_1_bss_BF0[0], 0, spriteId);
    HuSprPosSet(lbl_1_bss_BF0[0], 0, -170.0f, 10.0f);
    for (i = 0; i < 4; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[49], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_BF0[0], i + 1, spriteId);
        HuSprPosSet(lbl_1_bss_BF0[0], i + 1, -161.0f,
            -10.0f);
    }
    for (i = 0; i < 8; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[50], 110, 0);
        HuSprGrpMemberSet(lbl_1_bss_BF0[0], i + 5, spriteId);
        HuSprPosSet(lbl_1_bss_BF0[0], i + 5, i * 38 - 50, 10.0f);
    }
    for (i = 0; i < 8; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[51], 105, 0);
        HuSprGrpMemberSet(lbl_1_bss_BF0[0], i + 13, spriteId);
        HuSprPosSet(lbl_1_bss_BF0[0], i + 13, i * 38 - 50,
            10.0f);
    }
    for (i = 0; i < 8; i++) {
        spriteId = HuSprCreate(lbl_1_bss_BF8[i + 52], 100, 0);
        HuSprGrpMemberSet(lbl_1_bss_BF0[0], i + 21, spriteId);
        HuSprPosSet(lbl_1_bss_BF0[0], i + 21, i * 38 - 50,
            10.0f);
    }
    HuSprGrpPosSet(lbl_1_bss_BF0[0], 288.0f, 200.0f);
    for (i = 0; i < 29; i++) {
        HuSprAttrSet(lbl_1_bss_BF0[0], i, HUSPR_ATTR_DISPOFF);
    }
}

/* Runs the microphone volume test with mode 1 forced temporarily.
 * Restores the previous microphone setting after the test or a mount failure. */
void fn_1_EA6C(void)
{
    s16 index;
    s16 window;
    s16 inputCount;
    s16 mode = 0;
    s32 oldMicSetting;
    HUPROCESS *child;
    s32 micVolume;
    float progress;
    float scale;

    oldMicSetting = HuMCMicGet();
    HuMCMicSet(1);
    /* The initialization status is ignored; the explicit mount result controls the test screen. */
    HuMCInit(0);
    if (HuMCMount(1) != 0) {
        HuMCClose();
        HuMCMicSet(oldMicSetting);
        HuWinDispOff(lbl_1_bss_6A0);
        HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 67));
        HuWinMesWait(lbl_1_bss_6A2[0]);
        HuWinKeyWaitEntry(lbl_1_bss_6A2[0]);
        HuWinMesWait(lbl_1_bss_6A2[0]);
    } else {
        HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f, 200.0f);
        HuSprAttrReset(lbl_1_bss_5D4, 4, 4);
        for (index = 0; index < 29; index++) {
            HuSprAttrReset(lbl_1_bss_BF0[0], index, 4);
        }
        for (index = 0; index < 4; index++) {
            HuSprScaleSet(lbl_1_bss_BF0[0], index + 1,
                0.0f, 0.0f);
        }
        for (index = 0; index < 8; index++) {
            HuSprAttrSet(lbl_1_bss_BF0[0], index + 21, 4);
        }
        window = HuWinCreate(-10000.0f, 53.0f, 400, 40, 0);
        HuWinPriSet(window, 10);
        HuWinBGTPLvlSet(window, 0.0f);
        HuWinAttrSet(window, 2048);
        HuWinMesSpeedSet(window, 0);
        HuWinMesSet(window, MESSNUM(MESS_OPTION_MESS, 65));
        HuAudFXPlay(1205);
        for (index = 1; index <= 10; index++) {
            progress = (float)index / 10.0f;
            HuWinPosSet(window,
                288.0f - 200.0f * progress,
                200.0f - 147.0f * progress);
            HuWinScaleSet(window, progress, progress);
            HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
            HuSprGrpScaleSet(lbl_1_bss_BF0[0], progress, progress);
            HuPrcVSleep();
        }
        HuMCVolSampleCreate();
        HuMCShiftsSet(0);
        HuWinMesSet(lbl_1_bss_6A2[0], MESSNUM(MESS_OPTION_MESS, 66));
        HuWinMesSet(lbl_1_bss_6A0, MESSNUM(MESS_SYS_GUIDE, 22));
        inputCount = 0;
        for (;;) {
            HuPrcVSleep();
            if ((lbl_802C0008[0] & 0x200) != 0) {
                break;
            }
            for (index = 0; index < 8; index++) {
                HuSprAttrSet(lbl_1_bss_BF0[0], index + 21, 4);
            }
            if (HuMCButtonGet() != 0) {
                if ((inputCount++ & 15) == 0) {
                    child = fn_8000DAC0(fn_1_F0B0, 1000, 8192, 0,
                        fn_8000DD9C());
                    child->property = (void *)(mode++ & 3);
                }
                micVolume = HuMCVolGet(-50, 8);
                for (index = 0; index < micVolume; index++) {
                    HuSprAttrReset(lbl_1_bss_BF0[0], index + 21, 4);
                }
            }
        }
        for (index = 0; index < 8; index++) {
            HuSprAttrSet(lbl_1_bss_BF0[0], index + 21, 4);
        }
        HuAudFXPlay(1206);
        HuWinDispOff(lbl_1_bss_6A0);
        for (index = 1; index <= 10; index++) {
            progress = 1.0f -
                (float)index / 10.0f;
            HuWinPosSet(window,
                288.0f - 200.0f * progress,
                200.0f - 147.0f * progress);
            HuWinScaleSet(window, progress, progress);
            HuSprGrpScaleSet(lbl_1_bss_5D4, progress, progress);
            HuSprGrpScaleSet(lbl_1_bss_BF0[0], progress, progress);
            HuPrcVSleep();
        }
        HuWinKill(window);
        for (index = 0; index < 105; index++) {
            HuSprAttrSet(lbl_1_bss_5D4, index, 4);
        }
        for (index = 0; index < 29; index++) {
            HuSprAttrSet(lbl_1_bss_BF0[0], index, 4);
        }
        HuSprGrpScaleSet(lbl_1_bss_5D4, 1.0f,
            1.0f);
        HuSprGrpPosSet(lbl_1_bss_5D4, 288.0f,
            240.0f);
        HuMCClose();
        HuMCMicSet(oldMicSetting);
    }
}

/* The microphone test starts this child once per 16 updates with the talk button held, cycling
* four overlapping feedback sprites that shrink and fade in. */
void fn_1_F0B0(void)
{
    HUPROCESS *process;
    s32 memberNo;
    s32 frame;
    float progress;
    float scale;

    process = fn_8000DD9C();
    memberNo = (s32)process->property;
    memberNo++;
    HuSprScaleSet(lbl_1_bss_BF0[0], memberNo, 0.0f,
        0.0f);
    for (frame = 1; frame <= 55; frame++) {
        progress = (float)frame / 55.0f;
        scale = cos((3.141592653589793 *
            (90.0f * progress)) / 180.0);
        HuSprScaleSet(lbl_1_bss_BF0[0], memberNo, scale, scale);
        HuSprTPLvlSet(lbl_1_bss_BF0[0], memberNo, progress);
        HuPrcVSleep();
    }
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Option Mode resource setup calls this to load models and joint motions. */
void fn_1_F1D8(void)
{
    OptionModelEntry *entry = lbl_1_data_1AB0;
    s16 i;

    for (i = 0; entry->dataNum != OPTION_MODEL_END; entry++, i++) {
        if (entry->jointModel == -1) {
            lbl_1_bss_66E[i] = fn_80025A04(fn_80008DD0(entry->dataNum, 0x10000000, 2));
            fn_80026790(lbl_1_bss_66E[i], &entry->pos);
            fn_80026838(lbl_1_bss_66E[i], &entry->rot);
            fn_800268E0(lbl_1_bss_66E[i], &entry->scale);
            fn_800269E8(lbl_1_bss_66E[i], entry->attr1);
            fn_800269E8(lbl_1_bss_66E[i], entry->attr2);
            fn_80026B64(lbl_1_bss_66E[i], entry->cameraBit);
            fn_80026B8C(lbl_1_bss_66E[i], 1);
        } else {
            lbl_1_bss_63C[i] = fn_80030458((lbl_1_bss_66E[entry->jointModel]),
                fn_80008DD0(entry->dataNum, 0x10000000, 2));
        }
    }
}

/* Option Mode resource setup calls this to load its sprite animations. */
void fn_1_F394(void)
{
    s16 i;

    for (i = 0; i < 60; i++) {
        lbl_1_bss_BF8[i] = HuSprAnimDataRead(lbl_1_data_2060[i]);
    }
}

/* Menu callbacks call this to select both texture animations on a model. */
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2)
{
    s16 i;

    for (i = 0; i < 17; i++) {
        if (modelId == lbl_1_bss_5D6[i].modelId) {
            break;
        }
    }
    if (i != 17) {
        fn_80030D50(lbl_1_bss_5D6[i].animId1, lbl_1_bss_BF8[animNo1]);
        Hu3DAnimBankSet(lbl_1_bss_5D6[i].animId1, bank1);
        fn_80030D50(lbl_1_bss_5D6[i].animId2, lbl_1_bss_BF8[animNo2]);
        Hu3DAnimBankSet(lbl_1_bss_5D6[i].animId2, bank2);
    }
}

/* Choice screens call this before deciding whether a model needs an animation change. */
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2)
{
    HU3D_TEXANIM *anim;
    s16 i;

    for (i = 0; i < 17; i++) {
        if (modelId == lbl_1_bss_5D6[i].modelId) {
            break;
        }
    }
    if (i == 17) {
        return FALSE;
    }
    anim = &Hu3DTexAnimData[lbl_1_bss_5D6[i].animId1];
    if (anim->anim != lbl_1_bss_BF8[animNo1] || anim->bank != bank1) {
        return FALSE;
    }
    anim = &Hu3DTexAnimData[lbl_1_bss_5D6[i].animId2];
    if (anim->anim != lbl_1_bss_BF8[animNo2] || anim->bank != bank2) {
        return FALSE;
    }
    return TRUE;
}

/* Option screens call this to read a control sprite's current horizontal scale. */
float fn_1_F684(s16 groupId, s16 memberNo)
{
    HUSPRITE *sprite = &HuSprData[HuSprGrpData[groupId].sprId[memberNo]];

    return sprite->scale.x;
}
