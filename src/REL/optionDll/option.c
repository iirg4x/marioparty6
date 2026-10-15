/* Option Mode models, sprites, and shared menu helpers. */
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
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include <math.h>

#define OPTION_MODEL_END ((u32)-1)

void fn_1_C3FC(s16 selection);
s16 fn_1_C324(s16 flag, s16 type);

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
    {DATANUM(DATA_option, 0), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, HU3D_MOTATTR_LOOP, 2, 0},
    {DATANUM(DATA_option, 1), -1, 0, {0.00000000f, 0.00000000f, -1000.00000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 3), -1, 0, {0.00000000f, 0.00000000f, 50.0000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.29999995f, 1.29999995f, 1.29999995f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 4), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 5), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 6), 2, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 7), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {1.00000000f, 1.00000000f, 1.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 2), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0},
    {DATANUM(DATA_option, 8), -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, HU3D_MOTATTR_LOOP, 2, 0},
    {OPTION_MODEL_END, -1, 0, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, {0.00000000f, 0.00000000f, 0.00000000f}, 0U, 0U, 2, 0}
};

s32 lbl_1_data_2060[60] = {
    DATANUM(DATA_option, 9), DATANUM(DATA_option, 10),
    DATANUM(DATA_option, 11), DATANUM(DATA_option, 12),
    DATANUM(DATA_option, 13), DATANUM(DATA_option, 14),
    DATANUM(DATA_option, 15), DATANUM(DATA_option, 16),
    DATANUM(DATA_option, 17), DATANUM(DATA_option, 18),
    DATANUM(DATA_option, 19), DATANUM(DATA_option, 20),
    DATANUM(DATA_option, 21), DATANUM(DATA_option, 22),
    DATANUM(DATA_option, 23), DATANUM(DATA_option, 24),
    DATANUM(DATA_option, 25), DATANUM(DATA_option, 26),
    DATANUM(DATA_option, 27), DATANUM(DATA_option, 28),
    DATANUM(DATA_option, 29), DATANUM(DATA_option, 30),
    DATANUM(DATA_option, 31), DATANUM(DATA_option, 33),
    DATANUM(DATA_option, 32), DATANUM(DATA_option, 36),
    DATANUM(DATA_option, 37), DATANUM(DATA_option, 38),
    DATANUM(DATA_option, 39), DATANUM(DATA_option, 40),
    DATANUM(DATA_option, 41), DATANUM(DATA_option, 42),
    DATANUM(DATA_option, 43), DATANUM(DATA_option, 44),
    DATANUM(DATA_option, 45), DATANUM(DATA_option, 46),
    DATANUM(DATA_option, 47), DATANUM(DATA_option, 48),
    DATANUM(DATA_option, 49), DATANUM(DATA_option, 50),
    DATANUM(DATA_option, 51), DATANUM(DATA_option, 52),
    DATANUM(DATA_option, 53), DATANUM(DATA_option, 54),
    DATANUM(DATA_option, 55), DATANUM(DATA_option, 56),
    DATANUM(DATA_option, 57), DATANUM(DATA_option, 58),
    DATANUM(DATA_option, 60), DATANUM(DATA_option, 61),
    DATANUM(DATA_option, 62), DATANUM(DATA_option, 63),
    DATANUM(DATA_option, 64), DATANUM(DATA_option, 65),
    DATANUM(DATA_option, 66), DATANUM(DATA_option, 67),
    DATANUM(DATA_option, 68), DATANUM(DATA_option, 69),
    DATANUM(DATA_option, 70), DATANUM(DATA_option, 71)
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

void fn_1_A0(void);
void fn_1_804(void);
void fn_1_FDC(void);
void fn_1_1044(void);
void fn_1_EA6C(void);
void fn_1_1084(void);
void fn_1_3AE0(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
s16 fn_1_3318(s16 modelId, OptionSelectionData *selection, u32 *messages);
void fn_1_18B8(s16 cameraNo);
void fn_1_18D8(void);
void fn_1_19B8(s32 value);
void fn_1_19E0(void);
void fn_1_E6F8(void);
void fn_1_F1D8(void);
void fn_1_F394(void);
void fn_1_F0B0(void);
void fn_1_F410(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
BOOL fn_1_F544(s16 modelId, s16 animNo1, s16 bank1, s16 animNo2, s16 bank2);
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
void fn_1_C3FC(s16 selection);
float fn_1_F684(s16 groupId, s16 memberNo);
void fn_1_B02C(s16 first, s16 count);
void fn_1_9F74(void);
void fn_1_C484(void);
void fn_1_E538(s16 page);
extern int HuMCMicGet(void);
int sprintf(char *buffer, const char *format, int value);

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
        HuAudFXPlay(MSM_SE_MENU_38);
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
            if ((HuPadBtnDown[0] & 0x200) != 0) {
                break;
            }
            for (index = 0; index < 8; index++) {
                HuSprAttrSet(lbl_1_bss_BF0[0], index + 21, 4);
            }
            if (HuMCButtonGet() != 0) {
                if ((inputCount++ & 15) == 0) {
                    child = HuPrcChildCreate(fn_1_F0B0, 1000, 8192, 0,
                        HuPrcCurrentGet());
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
        HuAudFXPlay(MSM_SE_MENU_39);
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

    process = HuPrcCurrentGet();
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
            lbl_1_bss_66E[i] = Hu3DModelCreateData(entry->dataNum);
            Hu3DModelPosSetV(lbl_1_bss_66E[i], &entry->pos);
            Hu3DModelRotSetV(lbl_1_bss_66E[i], &entry->rot);
            Hu3DModelScaleSetV(lbl_1_bss_66E[i], &entry->scale);
            Hu3DModelAttrSet(lbl_1_bss_66E[i], entry->attr1);
            Hu3DModelAttrSet(lbl_1_bss_66E[i], entry->attr2);
            Hu3DModelCameraSet(lbl_1_bss_66E[i], entry->cameraBit);
            Hu3DModelLayerSet(lbl_1_bss_66E[i], 1);
        } else {
            lbl_1_bss_63C[i] = Hu3DJointMotionData(
                lbl_1_bss_66E[entry->jointModel], entry->dataNum);
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
        Hu3DAnimAnimSet(lbl_1_bss_5D6[i].animId1, lbl_1_bss_BF8[animNo1]);
        Hu3DAnimBankSet(lbl_1_bss_5D6[i].animId1, bank1);
        Hu3DAnimAnimSet(lbl_1_bss_5D6[i].animId2, lbl_1_bss_BF8[animNo2]);
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
