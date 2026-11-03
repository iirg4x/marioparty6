/* Draws and updates each player's minigame results before returning to the board. */
#include "math.h"
#include "game/hu3d.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/charman.h"
#include "game/flag.h"
#include "game/audio.h"
#include "game/gamework.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/saveload.h"
#include "dolphin/os.h"
#include "game/object.h"
#include "game/data.h"
#include "game/frand.h"
#include "game/mgdata.h"
#include "game/window.h"
typedef struct ResultDisplay {
    s32 rankSprite[2]; /* Current and projected rank badges. */
    s32 placeSprite; /* The player's numeric place. */
    s32 bonusSprite; /* The bonus-coin icon. */
    s32 bonusDigits[3]; /* Leading sprite and two digits for the bonus-coin award. */
    s32 starDigits[4]; /* Leading sprite and three star-count digits. */
    s32 coinDigits[4]; /* Leading sprite and three coin-balance digits. */
    s32 characterModel; /* Player character shown beside the results. */
    s32 characterMotions[3]; /* Idle, rank-up, and rank-down motions. */
    s32 resultModels[3]; /* Award, star and coin models. */
} ResultDisplay;
extern ResultDisplay lbl_1_bss_488[4];

extern OM_CAMERA_VIEW lbl_1_data_0;
extern OM_CAMERA_VIEW lbl_1_data_1C;
extern s16 lbl_1_bss_61C;
extern s32 lbl_1_bss_624;
extern s16 lbl_1_bss_628;
extern OMOBJMAN *lbl_1_bss_62C;
extern OMOBJ *lbl_1_bss_630;
extern void fn_1_560(void);
extern void fn_1_49C4(void);
extern void fn_1_7A18(void);

#define RESULT_SPRITE_HIDDEN 4

extern s32 lbl_1_bss_2C;
extern s16 lbl_1_bss_1C;
extern f32 lbl_1_bss_24;
extern f32 lbl_1_bss_20;
extern f32 lbl_1_bss_18;
extern f32 lbl_1_bss_14;
extern s16 lbl_1_bss_5F4[4];
extern s16 lbl_1_bss_5EC[4];
extern f32 lbl_1_data_204[4];

extern s32 lbl_1_bss_8;
extern s32 lbl_1_bss_28;
extern Vec lbl_1_data_1D4[4];

void fn_1_2EBC(void);

/* Sprite calls retain default argument promotions for the console build. */
#ifdef __MWERKS__

#else
#include "game/esprite.h"
#endif

extern f32 lbl_1_bss_1B8[9][10];
extern f32 lbl_1_bss_50[9][10];
extern s32 lbl_1_bss_320[9][10];
extern f32 lbl_1_bss_4;
extern f32 lbl_1_bss_C;
extern f32 lbl_1_bss_10;
extern f32 lbl_1_data_23C;

extern s32 lbl_1_bss_0;

extern f32 lbl_1_bss_5FC[4];
extern f32 lbl_1_bss_60C[4];
extern s32 lbl_1_bss_620;

void fn_1_47A8(void);
void fn_1_1074(void);
void fn_1_940(void);
s32 fn_1_C45C(s32 dataNum);
void fn_1_2068(void);
s32 fn_1_45D0(s32 player);
void fn_1_3698(void);
void fn_1_2878(void);
s32 fn_1_356C(void);
void fn_1_3640(s32 readResult);
void fn_1_1E8C(void);
void fn_1_4694(void);
void fn_1_3FB0(s16 *awards);
void fn_1_3488(s32 player, s32 value);
void fn_1_31F0(s32 player, s32 value);

extern s32 lbl_1_bss_5E8;
extern s32 lbl_1_bss_30[4];
extern s32 lbl_1_bss_40[4];

extern f32 lbl_1_data_54[8];
extern f32 lbl_1_data_74[8];
extern f32 lbl_1_data_94[8];
extern f32 lbl_1_data_B4[8];
extern f32 lbl_1_data_D4[8];
extern f32 lbl_1_data_F4[8];
extern Point3d lbl_1_data_114[4];
extern Point3d lbl_1_data_144[4];
extern Point3d lbl_1_data_174[4];
extern Point3d lbl_1_data_1A4[4];

extern s32 lbl_1_data_240[2];

s32 fn_1_C45C(s32 dataNum);
s32 fn_1_45D0(s32 player);

void fn_1_31F0(s32 player, s32 value);
void fn_1_333C(s32 player, s32 value);
void fn_1_3488(s32 player, s32 value);

/* Returns the minigame presentation mode selected by the board. */
static inline s16 ResultNightModeGet(void)
{
    return lbl_802C0278;
}

extern u32 lbl_1_data_248[6][2];
extern void fn_1_3FB0(s16 *coinAwards);
void fn_1_3FB0(s16 *coinAwards);

OM_CAMERA_VIEW lbl_1_data_0 = {{0.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1600.0f};
OM_CAMERA_VIEW lbl_1_data_1C = {{0.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1600.0f};
OM_CAMERA_VIEW lbl_1_data_38 = {{0.0f, 200.0f, 0.0f}, {0.0f, 0.0f, 0.0f}, 1600.0f};
f32 lbl_1_data_54[8] = {169.0f, 171.0f, 407.0f, 171.0f, 169.0f, 334.0f, 407.0f, 334.0f};
f32 lbl_1_data_74[8] = {229.0f, 141.0f, 467.0f, 141.0f, 229.0f, 304.0f, 467.0f, 304.0f};
f32 lbl_1_data_94[8] = {204.0f, 191.0f, 442.0f, 191.0f, 204.0f, 354.0f, 442.0f, 354.0f};
f32 lbl_1_data_B4[8] = {204.0f, 221.0f, 442.0f, 221.0f, 204.0f, 384.0f, 442.0f, 384.0f};
f32 lbl_1_data_D4[8] = {99.0f, 123.0f, 337.0f, 123.0f, 99.0f, 286.0f, 337.0f, 286.0f};
f32 lbl_1_data_F4[8] = {59.0f, 91.0f, 297.0f, 91.0f, 59.0f, 254.0f, 297.0f, 254.0f};
Point3d lbl_1_data_114[4] = { { 119.0f, 221.0f, 9000.0f },
                              { 357.0f, 221.0f, 9000.0f },
                              { 119.0f, 384.0f, 9000.0f },
                              { 357.0f, 384.0f, 9000.0f } };
Point3d lbl_1_data_144[4] = { { 219.0f, 106.0f, 9000.0f },
                              { 457.0f, 106.0f, 9000.0f },
                              { 219.0f, 269.0f, 9000.0f },
                              { 457.0f, 269.0f, 9000.0f } };
Point3d lbl_1_data_174[4] = { { 179.0f, 191.0f, 9000.0f },
                              { 417.0f, 191.0f, 9000.0f },
                              { 179.0f, 354.0f, 9000.0f },
                              { 417.0f, 354.0f, 9000.0f } };
Point3d lbl_1_data_1A4[4] = { { 179.0f, 221.0f, 9000.0f },
                              { 417.0f, 221.0f, 9000.0f },
                              { 179.0f, 384.0f, 9000.0f },
                              { 417.0f, 384.0f, 9000.0f } };
Vec lbl_1_data_1D4[4] = { { 1.0f, 1.0f, 1.0f },
                          { 0.75f, 0.75f, 0.75f },
                          { 0.5f, 0.5f, 0.5f },
                          { 0.3f, 0.3f, 0.3f } };
f32 lbl_1_data_204[4] = {1.0f, 0.98f, 0.96f, 0.94f};

/* Called by _prolog when this overlay opens; sets up result cameras and starts the corresponding
 * result
 * process. */
void fn_1_A0(void)
{
    s32 player;
    s32 lightId;
    s32 partyMode;
    GXColor color = {20, 0, 0, 0};

    OSReport("******* RESULT ObjectSetup *********\n");
    lbl_1_bss_62C = fn_8003A8BC(50, 8192);
    omSystemKeyCheckSetup(lbl_1_bss_62C);
    omSysPauseEnable(0);
    HuWinInit(0);
    lbl_1_bss_628 = lbl_80265B70.mgNo;
    for (player = 0; player < 4; player++) {
        lbl_80265728[player].grpNo = lbl_80265750[player].team;
    }
    fn_800097F4(MgDataTbl[lbl_1_bss_628].dataDir);
    partyMode = lbl_80265B70.partyF;
    if (partyMode == 0) {
        fn_8003A5E4(1, 1);
        return;
    }
    switch (MgDataTbl[lbl_80265B70.mgNo].type) {
    case MG_TYPE_KUPA:
    case MG_TYPE_KETTOU:
    case MG_TYPE_DONKEY:
        fn_8003A5E4(1, 1);
        return;
    }
    if (lbl_1_bss_628 <= -1) {
        lbl_1_bss_628 = 0;
    }
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(1, 5.0f, 1000.0f, 30000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 640, 480);
    Hu3DCameraPerspectiveSet(2, 5.0f, 10000.0f, 30000.0f, 1.2f);
    Hu3DCameraViewportSet(2, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraScissorSet(2, 0, 0, 640, 480);
    fn_8003E488(1, &lbl_1_data_0);
    fn_8003E488(2, &lbl_1_data_1C);
    lbl_1_bss_630 = fn_8003AB38(lbl_1_bss_62C, 32730, 100, 50, -1, fn_8003DAA4);
    lbl_1_bss_630->work[0] = 2;
    lightId = Hu3DGLightCreate(0.0f, 100.0f, 1000.0f, 0.0f, -0.5f, -1.0f, 255, 255, 255);
    Hu3DGLightInfinitytSet(lightId);
    lbl_1_bss_624 = 0;
    if (MgDataTbl[lbl_80265B70.mgNo].type != MG_TYPE_BATTLE) {
        if ((s32)lbl_80265B70.tagF != 0) {
            fn_8000DAC0(fn_1_49C4, 100, 12288, 0, lbl_1_bss_62C);
        } else {
            fn_8000DAC0(fn_1_560, 100, 12288, 0, lbl_1_bss_62C);
        }
    } else {
        fn_8000DAC0(fn_1_7A18, 100, 12288, 0, lbl_1_bss_62C);
    }
    /* The earlier window initialization makes this second initialization call return
     * immediately. */
    HuWinInit(1);
    lbl_1_bss_61C = frandmod(4);
}

f32 lbl_1_data_23C = 1.0f;
s32 lbl_1_data_240[2] = {-80, -80};
/* Board archives prefetched for the return, selected by board and day/night mode. */
#define RESULT_BG_BOARD_0_DAY 14090240
#define RESULT_BG_BOARD_0_NIGHT 14155776
#define RESULT_BG_BOARD_1_DAY 14221312
#define RESULT_BG_BOARD_1_NIGHT 14286848
#define RESULT_BG_BOARD_2_DAY 14352384
#define RESULT_BG_BOARD_2_NIGHT 14417920
#define RESULT_BG_BOARD_3_DAY 14483456
#define RESULT_BG_BOARD_3_NIGHT 14548992
#define RESULT_BG_BOARD_4_DAY 14614528
#define RESULT_BG_BOARD_4_NIGHT 14680064
#define RESULT_BG_BOARD_5_DAY 14745600
#define RESULT_BG_BOARD_5_NIGHT 14811136
u32 lbl_1_data_248[6][2] = {
    { RESULT_BG_BOARD_0_DAY, RESULT_BG_BOARD_0_NIGHT },
    { RESULT_BG_BOARD_1_DAY, RESULT_BG_BOARD_1_NIGHT },
    { RESULT_BG_BOARD_2_DAY, RESULT_BG_BOARD_2_NIGHT },
    { RESULT_BG_BOARD_3_DAY, RESULT_BG_BOARD_3_NIGHT },
    { RESULT_BG_BOARD_4_DAY, RESULT_BG_BOARD_4_NIGHT },
    { RESULT_BG_BOARD_5_DAY, RESULT_BG_BOARD_5_NIGHT }
};

/* Started by fn_1_A0 for ordinary minigames; presents player results and waits for confirmation
 * unless all players are computer-controlled before returning to the board. */
void fn_1_560(void)
{
    s16 readResult;
    HUPROCESS *currentProcess;
    s32 confirmed = 0;
    s16 comCount;
    HUWINID window;
    s16 player;

    currentProcess = fn_8000DD9C();
    fn_1_47A8();
    comCount = 0;
    for (player = comCount; player < 4; player++) {
        if (lbl_80265728[player].type != 0) {
            comCount++;
        }
    }
    if (comCount == 4) {
        lbl_1_bss_0 = 1;
    } else {
        lbl_1_bss_0 = 0;
    }
    lbl_1_bss_620 = 1;
    lbl_1_bss_8 = 0;
    for (player = 0; player < 4; player++) {
        lbl_1_bss_60C[player] = 1.0f;
        lbl_1_bss_5FC[player] = 1.0f;
    }
    fn_1_1074();
    fn_8000DAC0(fn_1_940, 100, 8192, 0, lbl_1_bss_62C);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    HuAudSStreamPlay(MSM_STREAM_PARTYRESULT_WAIT);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_800097F4(fn_1_C45C(DATANUM(0xc60000, 0)));
    HuPrcSleep(10);
    if (lbl_1_bss_624 == 0) {
        HuPrcSleep(20);
    }
    fn_1_2068();
    for (player = 0; player < 4; player++) {
        lbl_1_bss_5F4[player] = fn_1_45D0(player);
    }
    fn_1_3698();
    for (player = 0; player < 4; player++) {
        lbl_1_bss_5EC[player] = fn_1_45D0(player);
    }
    fn_1_2878();
    lbl_1_bss_8 = 1;
    window = HuWinCreate(HUWIN_POS_CENTER, 400.0f, 320, 40, 0);
    HuWinMesSpeedSet(window, 0);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinPriSet(window, 5);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    if (lbl_1_bss_0 == 0) {
        HuWinMesSet(window, MESSNUM(MESS_SYS_GUIDE, 0));
    }
    HuPrcSleep(4);
    readResult = fn_1_356C();
    for (;;) {
        if (lbl_1_bss_0 != 0) {
            confirmed = 1;
        } else {
            for (player = 0; player < 4; player++) {
                if (HuPadStatGet(player) == 0 && (lbl_802C0008[player] & PAD_BUTTON_A) != 0) {
                    HuAudFXPlay(2);
                    confirmed = 1;
                    break;
                }
            }
        }
        if (confirmed != 0) {
            break;
        }
        HuPrcVSleep();
    }
    HuWinKill(window);
    fn_1_3640(readResult);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    HuAudSStreamAllFadeOut(1000);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    lbl_1_bss_620 = 0;
    lbl_1_bss_8 = 0;
    fn_1_1E8C();
    fn_800097F4(fn_1_C45C(DATANUM(0xc60000, 0)));
    fn_1_4694();
    omSysPauseEnable(1);
    fn_8003A5E4(1, 1);
    for (;;) {
        HuPrcVSleep();
    }
}

/* Created by fn_1_560 under the object-manager process; updates result models, scrolling sprites,
* and rank pulses each frame. */
void fn_1_940(void)
{
    f32 starRotation[4];
    f32 coinRotation[4];
    f32 rotation;
    s16 player;
    s16 sprite;

    rotation = 0.0f;
    for (player = 0; player < 4; player++) {
        starRotation[player] = 0.0f;
        coinRotation[player] = 0.0f;
    }
    while (lbl_1_bss_620 != 0) {
        if (lbl_1_bss_0 != 0) {
            lbl_1_bss_624 = 1;
        } else {
            lbl_1_bss_624 = 0;
            for (player = 0; player < 4; player++) {
                if (HuPadStatGet(player) == 0 && (lbl_802C0010[player] & PAD_BUTTON_A) != 0) {
                    lbl_1_bss_624 = 1;
                }
            }
        }
        for (player = 0; player < 4; player++) {
            starRotation[player] += 1.2f * lbl_1_bss_5FC[player];
            if (starRotation[player] >= 360.0f) {
                starRotation[player] -= 360.0f;
            }
            fn_80026808((s16)lbl_1_bss_488[player].resultModels[1],
                0.0f, starRotation[player], 0.0f);
            coinRotation[player] += lbl_1_bss_60C[player];
            if (coinRotation[player] >= 360.0f) {
                coinRotation[player] -= 360.0f;
            }
            fn_80026808((s16)lbl_1_bss_488[player].resultModels[2],
                0.0f, coinRotation[player], 0.0f);
        }
        for (player = 0; player < 9; player++) {
            for (sprite = 0; sprite < 10; sprite++) {
                lbl_1_bss_1B8[player][sprite] += 1.0f;
                lbl_1_bss_50[player][sprite] += 1.0f;
                if (lbl_1_bss_1B8[player][sprite] > 616.0f) {
                    lbl_1_bss_1B8[player][sprite] = -80.0f;
                }
                if (lbl_1_bss_50[player][sprite] > 520.0f) {
                    lbl_1_bss_50[player][sprite] = -80.0f;
                }
                espPosSet(lbl_1_bss_320[player][sprite],
                    lbl_1_bss_1B8[player][sprite], lbl_1_bss_50[player][sprite]);
            }
        }
        if (lbl_1_bss_8 != 0) {
            lbl_1_bss_10 = sin((3.141592653589793 * lbl_1_bss_4) / 180.0);
            lbl_1_bss_4 += 5.0f;
            if (lbl_1_bss_4 > 180.0f) {
                lbl_1_bss_4 -= 180.0f;
            }
            lbl_1_bss_C = sin((3.141592653589793 * lbl_1_bss_4) / 180.0);
            lbl_1_data_23C += 0.1f * (lbl_1_bss_C - lbl_1_bss_10);
            for (player = 0; player < 4; player++) {
                if (lbl_1_bss_5EC[player] == 0) {
                    espScaleSet(lbl_1_bss_488[player].rankSprite[0],
                        0.95f * lbl_1_data_23C, 0.95f * lbl_1_data_23C);
                    espScaleSet(lbl_1_bss_488[player].rankSprite[1],
                        0.95f * lbl_1_data_23C, 0.95f * lbl_1_data_23C);
                    espScaleSet(lbl_1_bss_488[player].placeSprite,
                        lbl_1_data_23C, lbl_1_data_23C);
                }
            }
        }
        HuPrcVSleep();
    }
    HuPrcEnd();
}

/* Called by fn_1_560 before awards appear; creates the player panels, characters, and scrolling
 * backdrop. */
void fn_1_1074(void)
{
    int characters[4];
    int scores[4];
    int ranks[4];
    Point3d characterPosition;
    int characterOffset;
    int player;
    int digit;
    int rank;
    int positionIndex;
    int displayId;
    int language;

    language = GWLanguageGet();
    if (language == 1) {
        lbl_1_bss_5E8 = displayId = espEntry(fn_1_C45C(DATANUM(0xc60000, 5)), 180, 0);
        espPosSet(displayId, 288.0, 64.0);
        espDrawNoSet(displayId, 64);
    } else {
        lbl_1_bss_5E8 = displayId = espEntry(fn_1_C45C(DATANUM(0xc60000, 7)), 180, 0);
        espPosSet(displayId, 288.0, 64.0);
        espDrawNoSet(displayId, 64);
    }

    for (player = 0; player < 4; player++) {
        positionIndex = player * 2;
        rank = fn_1_45D0(player);
        lbl_1_bss_488[player].rankSprite[0] = displayId =
            espEntry(fn_1_C45C(DATANUM(0xc60000, 9) + rank), 200, 0);
        espPosSet(displayId, lbl_1_data_54[positionIndex], lbl_1_data_54[positionIndex + 1]);
        espDrawNoSet(displayId, 64);
        espScaleSet(displayId, 0.95f, 0.95f);

        lbl_1_bss_488[player].placeSprite = displayId =
            espEntry(fn_1_C45C(DATANUM(0xc60000, 2)), 150, rank);
        espPosSet(displayId, lbl_1_data_74[positionIndex], lbl_1_data_74[positionIndex + 1]);
        espBankSet(displayId, rank);
        espDrawNoSet(displayId, 64);
        espScaleSet(displayId, lbl_1_data_204[rank], lbl_1_data_204[rank]);

        lbl_1_bss_488[player].bonusSprite = displayId =
            espEntry(fn_1_C45C(DATANUM(0xc60000, 4)), 130, 0);
        espPosSet(displayId, lbl_1_data_D4[positionIndex], lbl_1_data_D4[positionIndex + 1]);
        espBankSet(displayId, 0);
        espDrawNoSet(displayId, 64);
        espScaleSet(displayId, 0.0, 0.0);
        espAttrSet(displayId, 4);

        for (digit = 0; digit < 3; digit++) {
            if (digit == 0) {
                lbl_1_bss_488[player].bonusDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 0)), 120, 10);
            } else {
                lbl_1_bss_488[player].bonusDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 0)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_F4[positionIndex] + (f32)(digit * 17),
                lbl_1_data_F4[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
            espScaleSet(displayId, 0.0, 0.0);
            espAttrSet(displayId, 4);
        }
        for (digit = 0; digit < 4; digit++) {
            if (digit == 0) {
                lbl_1_bss_488[player].starDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 1)), 120, 10);
            } else {
                lbl_1_bss_488[player].starDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 1)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_94[positionIndex] + (f32)(digit * 17),
                lbl_1_data_94[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        }
        for (digit = 0; digit < 4; digit++) {
            if (digit == 0) {
                lbl_1_bss_488[player].coinDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 1)), 120, 10);
            } else {
                lbl_1_bss_488[player].coinDigits[digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 1)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_B4[positionIndex] + (f32)(digit * 17),
                lbl_1_data_B4[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        }

        lbl_1_bss_40[player] = fn_8003CD14(player);
        lbl_1_bss_30[player] = GWPlayerStarGet(player);
        fn_1_31F0(player, lbl_1_bss_40[player]);
        fn_1_333C(player, lbl_1_bss_30[player]);
        fn_1_3488(player, 0);

        characters[player] = lbl_80265728[player].charNo;
        characterOffset = characters[player] * 4;
        lbl_1_bss_488[player].characterModel = displayId = fn_80057068(characters[player], 2);
        lbl_1_bss_488[player].characterMotions[0] =
            fn_8005B0B8(characters[player], DATANUM(0x930000, FILENUM(0)));
        lbl_1_bss_488[player].characterMotions[1] =
            fn_8005B0B8(characters[player], DATANUM(0x930000, FILENUM(36)));
        lbl_1_bss_488[player].characterMotions[2] =
            fn_8005B0B8(characters[player], DATANUM(0x930000, FILENUM(37)));
        fn_8005B4BC(characters[player]);
        fn_8002C95C(displayId, lbl_1_bss_488[player].characterMotions[0]);
        fn_800269E8(displayId, ((1 << 30) | (1 << 0)));
        fn_80026B64(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_114[player], 1, &characterPosition);
        fn_80026790(displayId, &characterPosition);
        fn_800268E0(displayId, &lbl_1_data_1D4[rank]);
        fn_80026B8C(displayId, 1);

        lbl_1_bss_488[player].resultModels[0] = displayId =
            fn_80025A04(fn_80008DD0(fn_1_C45C(DATANUM(0xc60000, 22)),
                0x10000000, 2));
        fn_80026B64(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_144[player], 1, &characterPosition);
        fn_80026790(displayId, &characterPosition);
        fn_80026B8C(displayId, 1);
        fn_800268B0(displayId, 0.0f, 0.0f, 0.0f);
        fn_800269E8(displayId, 1);
        fn_800269E8(displayId, ((1 << 30) | (1 << 0)));

        lbl_1_bss_488[player].resultModels[1] = displayId =
            fn_80025A04(fn_80008DD0(fn_1_C45C(DATANUM(0xc60000, 25)),
                0x10000000, 2));
        fn_80026B64(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_174[player], 1, &characterPosition);
        fn_800268B0(displayId, 0.6f, 0.6f, 0.6f);
        fn_80026790(displayId, &characterPosition);
        fn_80026B8C(displayId, 1);

        lbl_1_bss_488[player].resultModels[2] = displayId =
            fn_80025A04(fn_80008DD0(fn_1_C45C(DATANUM(0xc60000, 26)),
                0x10000000, 2));
        fn_80026B64(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_1A4[player], 1, &characterPosition);
        fn_800268B0(displayId, 0.5f, 0.5f, 0.5f);
        fn_80026790(displayId, &characterPosition);
        fn_80026B8C(displayId, 1);
    }

    for (player = 0; player < 9; player++) {
        for (digit = 0; digit < 10; digit++) {
            if (ResultNightModeGet() == 0) {
                lbl_1_bss_320[player][digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 19)), 256, 0);
            } else {
                lbl_1_bss_320[player][digit] = displayId =
                    espEntry(fn_1_C45C(DATANUM(0xc60000, 20)), 256, 0);
            }
            lbl_1_bss_1B8[player][digit] = lbl_1_data_240[0] + (9 - digit) * 80;
            lbl_1_bss_50[player][digit] = lbl_1_data_240[1] + player * 80;
            espPosSet(displayId, lbl_1_bss_1B8[player][digit], lbl_1_bss_50[player][digit]);
            espDrawNoSet(displayId, 64);
        }
    }
    HuSprExecLayerCameraSet(64, 1, 1);

    for (player = 0; player < 4; player++) {
        scores[player] = fn_8003CD14(player) | (GWPlayerStarGet(player) << 10);
        scores[player] += GWMgCoinBonusGet(player) + GWMgCoinGet(player);
    }
    for (player = 0; player < 4; player++) {
        rank = 0;
        for (digit = 0; digit < 4; digit++) {
            if (digit != player && scores[player] < scores[digit]) {
                rank++;
            }
        }
        ranks[player] = rank;
    }
    for (player = 0; player < 4; player++) {
        positionIndex = player * 2;
        lbl_1_bss_488[player].rankSprite[1] = displayId =
            espEntry(fn_1_C45C(DATANUM(0xc60000, 9) + ranks[player]), 200, 0);
        espPosSet(displayId, lbl_1_data_54[positionIndex], lbl_1_data_54[positionIndex + 1]);
        espDrawNoSet(displayId, 64);
        espAttrSet(displayId, 4);
        espScaleSet(displayId, 0.95f, 0.95f);
    }
}

/* Frees every sprite and model created for the result overlay before it closes. */
void fn_1_1E8C(void)
{
    s32 player;
    s32 handle;

    espKill(lbl_1_bss_5E8);
    for (player = 0; player < 4; player++) {
        espKill(lbl_1_bss_488[player].rankSprite[0]);
        espKill(lbl_1_bss_488[player].rankSprite[1]);
        espKill(lbl_1_bss_488[player].placeSprite);
        espKill(lbl_1_bss_488[player].bonusSprite);
        for (handle = 0; handle < 3; handle++) {
            espKill(lbl_1_bss_488[player].bonusDigits[handle]);
        }
        for (handle = 0; handle < 4; handle++) {
            espKill(lbl_1_bss_488[player].starDigits[handle]);
            espKill(lbl_1_bss_488[player].coinDigits[handle]);
        }
        fn_800262D0((s16)lbl_1_bss_488[player].resultModels[0]);
        fn_800262D0((s16)lbl_1_bss_488[player].resultModels[1]);
        fn_800262D0((s16)lbl_1_bss_488[player].resultModels[2]);
    }
    for (player = 0; player < 9; player++) {
        for (handle = 0; handle < 10; handle++) {
            espKill(lbl_1_bss_320[player][handle]);
        }
    }
    fn_8005BB8C(-1);
}

/* Animates awarded coins into each balance; called before the rank update. */
void fn_1_2068(void)
{
    extern void espAttrSet();
    extern void espAttrReset();
    s16 awards[5]; /* The fifth entry holds the battle coin remainder. */
    s16 coins[4];
    s16 player;
    s16 digit;
    s16 phaseCount;
    s16 frame;
    s16 bounceFrame;
    s32 appearDuration;
    int bounceStep;
    s32 disappearDuration;
    f32 scale;
    f32 elapsed;
    f32 rate;

    fn_1_3FB0(awards);
    for (player = 0; player < 4; player++) {
        for (digit = 0; digit < 3; digit++) {
            espAttrReset(lbl_1_bss_488[player].bonusDigits[digit], HUSPR_ATTR_DISPOFF);
        }
        fn_1_3488(player, awards[player]);
        espAttrReset(lbl_1_bss_488[player].bonusSprite, HUSPR_ATTR_DISPOFF);
    }
    if (lbl_1_bss_624 != 0) {
        appearDuration = 3;
    } else {
        appearDuration = 10;
    }
    phaseCount = appearDuration;
    for (frame = 0; frame <= phaseCount; frame++) {
        scale = sin((3.141592653589793 * ((90.0 / phaseCount) * frame)) / 180.0);
        for (player = 0; player < 4; player++) {
            espScaleSet(lbl_1_bss_488[player].bonusSprite, scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_488[player].bonusDigits[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (player = 0; player < 4; player++) {
        espScaleSet(lbl_1_bss_488[player].bonusSprite, 1.0, 1.0);
    }
    for (bounceFrame = 0; bounceFrame <= 14;) {
        for (player = 0; player < 4; player++) {
            if (awards[player] >= 10) {
                fn_80026A44((s16)lbl_1_bss_488[player].resultModels[0], (1 << 0));
            }
            scale = 0.1f + sin((3.141592653589793 * ((120.0f / 14.0f) * bounceFrame)) / 180.0) *
                (0.8 * (1.0 / sin((3.141592653589793 * 120.0) / 180.0)));
            fn_800268B0((s16)lbl_1_bss_488[player].resultModels[0], scale, scale, scale);
        }
        if (lbl_1_bss_624 != 0) {
            bounceStep = 2;
        } else {
            bounceStep = 1;
        }
        bounceFrame = bounceFrame + bounceStep;
        HuPrcVSleep();
    }
    if (lbl_1_bss_624 == 0) {
        HuPrcSleep(15);
    }
    elapsed = phaseCount = 0;
    for (player = 0; player < 4; player++) {
        coins[player] = fn_8003CD14((s16)player);
    }
    for (;;) {
        if (lbl_1_bss_624 != 0) {
            rate = 1.0f;
        } else {
            rate = 0.2f;
        }
        elapsed += rate;
        if (elapsed >= 1.0f) {
            phaseCount = 0;
            for (player = phaseCount; player < 4; player++) {
                if (awards[player] == 0) {
                    fn_1_3488((s16)player, 0);
                    lbl_1_bss_60C[player] = 1.0f;
                    phaseCount++;
                } else {
                    coins[player]++;
                    awards[player]--;
                    HuAudFXPlay(7);
                    lbl_1_bss_60C[player] = 3.0f;
                }
                fn_1_3488((s16)player, awards[player]);
                fn_1_31F0((s16)player, coins[player]);
            }
            elapsed = 0.0f;
        }
        if (phaseCount >= 4) {
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    if (lbl_1_bss_624 != 0) {
        disappearDuration = 3;
    } else {
        disappearDuration = 10;
    }
    phaseCount = disappearDuration;
    for (frame = 0; frame <= phaseCount; frame++) {
        scale = cos((3.141592653589793 * ((90.0 / phaseCount) * frame)) / 180.0);
        for (player = 0; player < 4; player++) {
            espScaleSet(lbl_1_bss_488[player].bonusSprite, scale, scale);
            fn_800268B0((s16)lbl_1_bss_488[player].resultModels[0], scale, scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_488[player].bonusDigits[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (player = 0; player < 4; player++) {
        espAttrSet(lbl_1_bss_488[player].bonusSprite, HUSPR_ATTR_DISPOFF);
        fn_800269E8((s16)lbl_1_bss_488[player].resultModels[0], (1 << 0));
        for (digit = 0; digit < 3; digit++) {
            espAttrSet(lbl_1_bss_488[player].bonusDigits[digit], HUSPR_ATTR_DISPOFF);
        }
    }
}

/* Changes character poses and rank panels after the new standings are calculated. */
void fn_1_2878(void)
{
    s32 player;
    s32 frame;
    s32 unchangedCount;
    s32 cycle;
    f32 scale[4];
    f32 targetScale[4];
    f32 scaleStep[4];
    f32 transitionStep[4];
    s32 changed[4];
    s32 transitionFrame;

    unchangedCount = 0;
    transitionFrame = 0;
    for (player = 0; player < 4; player++) {
        if ((s16)lbl_1_bss_5F4[player] > lbl_1_bss_5EC[player]) {
            changed[player] = 1;
            fn_8002CCC0((s16)lbl_1_bss_488[player].characterModel,
                (s16)lbl_1_bss_488[player].characterMotions[1], 0.0f, 8.0f, 0);
            if (lbl_1_bss_5EC[player] != 0) {
                CharFXStop(lbl_80265728[player].charNo, 575);
                CharFXPlay(lbl_80265728[player].charNo, 579);
            }
        } else if ((s16)lbl_1_bss_5F4[player] < lbl_1_bss_5EC[player]) {
            changed[player] = 1;
            fn_8002CCC0((s16)lbl_1_bss_488[player].characterModel,
                (s16)lbl_1_bss_488[player].characterMotions[2], 0.0f, 8.0f, 0);
        } else {
            changed[player] = 0;
            unchangedCount++;
        }
        if (changed[player] != 0) {
            scale[player] = lbl_1_data_1D4[lbl_1_bss_5F4[player]].x;
            targetScale[player] = lbl_1_data_1D4[lbl_1_bss_5EC[player]].x;
            transitionStep[player] = (targetScale[player] - scale[player]) / 20.0f;
            scaleStep[player] = (targetScale[player] - scale[player]) / 30.0f;
        }
    }
    if (unchangedCount >= 4) {
        for (player = 0; player < 4; player++) {
            espAttrSet(lbl_1_bss_488[player].rankSprite[0], RESULT_SPRITE_HIDDEN);
            espAttrReset(lbl_1_bss_488[player].rankSprite[1], RESULT_SPRITE_HIDDEN);
        }
        lbl_1_bss_8 = 1;
        return;
    }
    lbl_1_bss_2C = 1;
    lbl_1_bss_28 = 0;
    lbl_1_bss_24 = 1.0f;
    lbl_1_bss_20 = 1.0f / 35.0f;
    lbl_1_bss_1C = 0;
    lbl_1_bss_18 = 1.0f;
    lbl_1_bss_14 = 1.0f / 35.0f;
    for (cycle = 0; cycle < 3; cycle++) {
        for (frame = 0; frame < 20; frame++) {
            for (player = 0; player < 4; player++) {
                if (changed[player] != 0) {
                    scale[player] += scaleStep[player];
                    fn_800268B0((s16)lbl_1_bss_488[player].characterModel,
                        scale[player], scale[player], scale[player]);
                }
            }
            fn_1_2EBC();
            HuPrcVSleep();
        }
        for (frame = 0; frame < 10.0f; frame++) {
            for (player = 0; player < 4; player++) {
                if (changed[player] != 0) {
                    scale[player] -= scaleStep[player];
                    fn_800268B0((s16)lbl_1_bss_488[player].characterModel,
                        scale[player], scale[player], scale[player]);
                }
            }
            fn_1_2EBC();
            HuPrcVSleep();
        }
    }
    for (player = 0; player < 4; player++) {
        espScaleSet(lbl_1_bss_488[player].placeSprite,
            lbl_1_data_204[lbl_1_bss_5EC[player]], lbl_1_data_204[lbl_1_bss_5EC[player]]);
        fn_8002CCC0((s16)lbl_1_bss_488[player].characterModel,
            (s16)lbl_1_bss_488[player].characterMotions[0], 0.0f, 8.0f,
            ((1 << 30) | (1 << 0)));
    }
    lbl_1_bss_8 = 1;
}

/* Advances the rank badge swap during fn_1_2878's transition frames. */
void fn_1_2EBC(void)
{
    s32 switchRank;
    s32 player;

    switchRank = 0;
    if (lbl_1_bss_2C != 0) {
        if (lbl_1_bss_1C == 0) {
            lbl_1_bss_24 -= lbl_1_bss_20;
            lbl_1_bss_18 -= lbl_1_bss_14;
            if (lbl_1_bss_24 <= 0.0f) {
                lbl_1_bss_24 = 0.0f;
                lbl_1_bss_18 = 0.0f;
                lbl_1_bss_1C = 1;
                switchRank = 1;
            }
        } else {
            lbl_1_bss_24 += lbl_1_bss_20;
            lbl_1_bss_18 += lbl_1_bss_14;
            if (lbl_1_bss_24 >= 1.0f) {
                lbl_1_bss_2C = 0;
            }
        }
        for (player = 0; player < 4; player++) {
            if (lbl_1_bss_5F4[player] != lbl_1_bss_5EC[player]) {
                if (switchRank != 0) {
                    espBankSet(lbl_1_bss_488[player].placeSprite, lbl_1_bss_5EC[player]);
                    espAttrSet(lbl_1_bss_488[player].rankSprite[0], RESULT_SPRITE_HIDDEN);
                    espAttrReset(lbl_1_bss_488[player].rankSprite[1], RESULT_SPRITE_HIDDEN);
                }
                espScaleSet(lbl_1_bss_488[player].placeSprite,
                            lbl_1_bss_24 * lbl_1_data_204[lbl_1_bss_5EC[player]],
                            lbl_1_bss_24 * lbl_1_data_204[lbl_1_bss_5EC[player]]);
                if (lbl_1_bss_1C == 0) {
                    espTPLvlSet(lbl_1_bss_488[player].rankSprite[0], lbl_1_bss_18);
                } else {
                    espTPLvlSet(lbl_1_bss_488[player].rankSprite[1], lbl_1_bss_18);
                }
            }
        }
    }
}

/* Updates the three displayed coin digits when awards are counted into a balance. */
void fn_1_31F0(s32 player, s32 coinCount)
{
    ResultDisplay displayFields = lbl_1_bss_488[player];
    s32 digit[3];
    s32 remainder;

    if (coinCount > 999) {
        coinCount = 999;
    }
    remainder = coinCount;
    digit[0] = coinCount / 100;
    if (digit[0] == 0) {
        espAttrSet(displayFields.coinDigits[1], 4);
    } else {
        espAttrReset(displayFields.coinDigits[1], 4);
        espBankSet(displayFields.coinDigits[1], digit[0]);
    }
    remainder = coinCount - (digit[0] * 100);
    digit[1] = remainder / 10;
    if ((digit[1] == 0) && (digit[0] == 0)) {
        espAttrSet(displayFields.coinDigits[2], 4);
    } else {
        espAttrReset(displayFields.coinDigits[2], 4);
        espBankSet(displayFields.coinDigits[2], digit[1]);
    }
    remainder -= digit[1] * 10;
    espBankSet(displayFields.coinDigits[3], remainder);
}

/* Updates the three displayed star digits when the player result panel is built. */
void fn_1_333C(s32 player, s32 starCount)
{
    ResultDisplay displayFields = lbl_1_bss_488[player];
    s32 digit[3];
    s32 remainder;

    if (starCount > 999) {
        starCount = 999;
    }
    remainder = starCount;
    digit[0] = starCount / 100;
    if (digit[0] == 0) {
        espAttrSet(displayFields.starDigits[1], 4);
    } else {
        espAttrReset(displayFields.starDigits[1], 4);
        espBankSet(displayFields.starDigits[1], digit[0]);
    }
    remainder = starCount - (digit[0] * 100);
    digit[1] = remainder / 10;
    if ((digit[1] == 0) && (digit[0] == 0)) {
        espAttrSet(displayFields.starDigits[2], 4);
    } else {
        espAttrReset(displayFields.starDigits[2], 4);
        espBankSet(displayFields.starDigits[2], digit[1]);
    }
    remainder -= digit[1] * 10;
    espBankSet(displayFields.starDigits[3], remainder);
}

/* Updates the displayed two-digit bonus-coin count. */
void fn_1_3488(s32 player, s32 bonusCoins)
{
    ResultDisplay displayFields = lbl_1_bss_488[player];
    s32 digit[3];
    s32 remainder;

    if (bonusCoins > 99) {
        bonusCoins = 99;
    }
    remainder = bonusCoins;
    digit[0] = bonusCoins / 10;
    if (digit[0] == 0) {
        espAttrSet(displayFields.bonusDigits[1], 4);
    } else {
        espAttrReset(displayFields.bonusDigits[1], 4);
        espBankSet(displayFields.bonusDigits[1], digit[0]);
    }
    remainder -= digit[0] * 10;
    espBankSet(displayFields.bonusDigits[2], remainder);
}

/* Called by result processes after the awards and before confirmation; starts an asynchronous
 * read of the board's day/night archive when saving is initialized. */
s32 fn_1_356C(void)
{
    s32 result;
    s32 timeMode;

    result = -1;
    if (lbl_80265B70.curTime == 0) {
        timeMode = 0;
    } else {
        timeMode = 1;
    }
    if (fn_8006A124(65536) == 0) {
        return -1;
    }
    if (lbl_80265B70.boardNo <= 5) {
        result = fn_80007F20(lbl_1_data_248[lbl_80265B70.boardNo][timeMode]);
        OSReport("/----------------Result_BGRead----------------/\n");
    }
    return result;
}

/* Called by fn_1_560 before closing the result screen; waits for the background read started by
 * fn_1_356C. */
void fn_1_3640(s32 backgroundReadHandle) {
    OSReport("/----------------Result_BGReadWait----------------/\n");
    if (backgroundReadHandle != -1) {
        while (fn_80008270(backgroundReadHandle) == 0) {
            HuPrcVSleep();
        }
    }
}

/* Applies awards to players or teams and updates earned-coin statistics. Practice suppresses
 * balance and bonus-counter writes; minigame coin counters are still cleared. */
void fn_1_3698(void)
{
    s16 player;
    s16 opponent;
    s16 award = 0;
    s16 rank;
    s16 pass;
    s16 swap;
    s16 remainderPlayer;
    s16 opponentBonus;
    s16 playerBonus;
    s16 coinAwards[5];
    s16 ranks[4];
    s16 sortedRanks[4];
    s16 playerOrder[4];
    s16 teamPlayers[2] = {-1, -1};
    s16 teamAwards[2] = {0, 0};

    for (player = 0; player < 5; player++) {
        coinAwards[player] = 0;
    }
    if (MgDataTbl[lbl_80265B70.mgNo].type == MG_TYPE_BATTLE) {
        for (player = 0; player < 4; player++) {
            for (opponent = rank = 0; opponent < 4; opponent++) {
                if (player != opponent) {
                    opponentBonus = lbl_80265750[opponent].mgCoinBonus;
                    playerBonus = lbl_80265750[player].mgCoinBonus;
                    if (playerBonus > opponentBonus) {
                        rank++;
                    }
                }
            }
            sortedRanks[player] = ranks[player] = rank;
            playerOrder[player] = player;
        }
        for (pass = 1; pass < 4; pass++) {
            for (player = 0; player < 4 - pass; player++) {
                if (sortedRanks[player] > sortedRanks[player + 1]) {
                    swap = sortedRanks[player];
                    sortedRanks[player] = sortedRanks[player + 1];
                    sortedRanks[player + 1] = swap;
                    swap = playerOrder[player];
                    playerOrder[player] = playerOrder[player + 1];
                    playerOrder[player + 1] = swap;
                }
            }
        }
        for (player = 0; player < 4; player++) {
            if (lbl_1_bss_61C == player) {
                remainderPlayer = playerOrder[player];
            }
        }
        fn_1_3FB0(coinAwards);
        if ((s32)lbl_80265B70.tagF != 0) {
            for (player = 0; player < 4; player++) {
                award = coinAwards[ranks[player]];
                if (remainderPlayer == player) {
                    award += coinAwards[4];
                }
                teamAwards[lbl_80265750[player].team] += award;
            }
            for (player = 0; player < 2; player++) {
                if (teamPlayers[player] == -1) {
                    for (opponent = 0; opponent < 4; opponent++) {
                        if (player == lbl_80265750[opponent].team) {
                            teamPlayers[player] = opponent;
                            break;
                        }
                    }
                }
            }
            for (player = 0; player < 2; player++) {
                fn_8003CDDC(teamPlayers[player], teamAwards[player]);
                lbl_80265750[teamPlayers[player]].coinTotalMg += teamAwards[player];
                if (lbl_80265750[teamPlayers[player]].coinTotalMg > 9999) {
                    lbl_80265750[teamPlayers[player]].coinTotalMg = 9999;
                }
            }
            for (player = 0; player < 4; player++) {
                GWMgCoinBonusSet(player, 0);
                GWMgCoinSet(player, 0);
            }
            return;
        }
        for (player = 0; player < 4; player++) {
            award = coinAwards[ranks[player]];
            if (remainderPlayer == player) {
                award += coinAwards[4];
            }
            fn_8003CDDC(player, award);
            GWMgCoinBonusSet(player, 0);
            GWMgCoinSet(player, 0);
            lbl_80265750[player].coinTotalMg += award;
            if (lbl_80265750[player].coinTotalMg > 9999) {
                lbl_80265750[player].coinTotalMg = 9999;
            }
        }
        return;
    }
    fn_1_3FB0(coinAwards);
    if ((s32)lbl_80265B70.tagF != 0) {
        for (player = 0; player < 4; player++) {
            teamAwards[lbl_80265750[player].team] += coinAwards[player];
        }
        for (player = 0; player < 2; player++) {
            if (teamPlayers[player] == -1) {
                for (opponent = 0; opponent < 4; opponent++) {
                    if (player == lbl_80265750[opponent].team) {
                        teamPlayers[player] = opponent;
                        break;
                    }
                }
            }
        }
        for (player = 0; player < 2; player++) {
            fn_8003CDDC(teamPlayers[player], teamAwards[player]);
            lbl_80265750[teamPlayers[player]].coinTotalMg += teamAwards[player];
            if (lbl_80265750[teamPlayers[player]].coinTotalMg > 9999) {
                lbl_80265750[teamPlayers[player]].coinTotalMg = 9999;
            }
        }
        for (player = 0; player < 4; player++) {
            GWMgCoinBonusSet(player, 0);
            GWMgCoinSet(player, 0);
        }
        return;
    }
    for (player = 0; player < 4; player++) {
        fn_8003CDDC(player, coinAwards[player]);
        GWMgCoinBonusSet(player, 0);
        GWMgCoinSet(player, 0);
        lbl_80265750[player].coinTotalMg += coinAwards[player];
        if (lbl_80265750[player].coinTotalMg > 9999) {
            lbl_80265750[player].coinTotalMg = 9999;
        }
    }
}

/* Splits battle coins by rank or copies each player's ordinary minigame award. */
void fn_1_3FB0(s16 *coinAwards)
{
    f32 shares[4];
    s16 ranks[4];
    s16 groups[4];
    s16 player;
    s16 other;
    u32 allocated;
    s16 swapRank;
    u32 total;
    u32 award;
    s16 minigameType;
    s16 contribution;
    s16 rank;

    minigameType = MgDataTbl[lbl_80265B70.mgNo].type;
    switch (minigameType) {
    case MG_TYPE_BATTLE:
        for (player = total = 0; player < 4; player++) {
            contribution = lbl_80265750[player].coinBattle;
            total += contribution;
        }
        ranks[0] = ranks[1] = ranks[2] = ranks[3] = 0;
        groups[0] = groups[1] = groups[2] = groups[3] = 0;
        for (player = 0; player < 4; player++) {
            rank = lbl_80265750[player].mgCoinBonus;
            ranks[player] = rank;
        }
        for (player = 0; player < 4; player++) {
            for (other = player + 1; other < 4; other++) {
                if (ranks[player] > ranks[other]) {
                    swapRank = ranks[player];
                    ranks[player] = ranks[other];
                    ranks[other] = swapRank;
                }
            }
        }
        swapRank = 0;
        for (player = 1; player < 4; player++) {
            if (ranks[player] != ranks[player - 1]) {
                swapRank++;
            }
            groups[player] = swapRank;
        }
        shares[0] = shares[1] = shares[2] = shares[3] = 0.0f;
        if (groups[0] == 0 && groups[1] == 1 && groups[2] == 2 && groups[3] == 3) {
            shares[0] = 0.7f;
            shares[1] = 0.25f;
            shares[2] = 0.05f;
        } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 1 && groups[3] == 2) {
            shares[0] = 0.5f;
            shares[1] = 0.5f;
        } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 1 && groups[3] == 1) {
            shares[0] = 0.5f;
            shares[1] = 0.5f;
        } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 1) {
            shares[0] = 0.3f;
            shares[1] = 0.3f;
            shares[2] = 0.3f;
        } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 1 && groups[3] == 2) {
            shares[0] = 0.6f;
            shares[1] = 0.2f;
            shares[2] = 0.2f;
        } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 1 && groups[3] == 1) {
            shares[0] = 0.7f;
            shares[1] = 0.1f;
            shares[2] = 0.1f;
            shares[3] = 0.1f;
        } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 2 && groups[3] == 2) {
            shares[0] = 0.7f;
            shares[1] = 0.3f;
        } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 0) {
            shares[0] = 0.25f;
            shares[1] = 0.25f;
            shares[2] = 0.25f;
            shares[3] = 0.25f;
        }
        for (player = allocated = 0; player < 4; player++) {
            award = (u32)((f32)total * shares[player]);
            coinAwards[player] = award;
            allocated += award;
        }
        if (allocated < total) {
            coinAwards[4] = total - allocated;
        } else {
            coinAwards[4] = 0;
        }
        break;
    default: {
        s16 minigameCoins;
        s16 bonusCoins;

        for (player = 0; player < 4; player++) {
            minigameCoins = lbl_80265750[player].mgCoin;
            bonusCoins = lbl_80265750[player].mgCoinBonus;
            coinAwards[player] = allocated = bonusCoins + minigameCoins;
        }
        break;
    }
    }
}

/* Returns how many players have a higher combined star-and-coin score. */
s32 fn_1_45D0(s32 playerId)
{
    s32 playerScore[4];
    s32 player;
    s32 rank;

    for (player = 0; player < 4; player++) {
        playerScore[player] = fn_8003CD14(player) | (GWPlayerStarGet(player) << 10);
    }
    rank = 0;
    for (player = 0; player < 4; player++) {
        if (player != playerId && playerScore[playerId] < playerScore[player]) {
            rank++;
        }
    }
    return rank;
}

/* Records star/coin history for turns below 52, then runs board-turn saving. The save routine
 * always clears models and resets sprites, even when saving is disabled. */
void fn_1_4694(void)
{
    s16 player;

    if (lbl_80265B70.turnNo < 52) {
        s16 turnNo = lbl_80265B70.turnNo;

        for (player = 0; player < 4; player++) {
            lbl_80265750[player].starGraph[turnNo - 1] = lbl_80265750[player].star;
            lbl_80265750[player].coinGraph[turnNo - 1] = lbl_80265750[player].coin;
        }
        if (SLSaveFlagGet() == 0) {
            lbl_80265B70.saveMode = 1;
        }
    }
    SLSaveBoardTurnExec();
}

/* Prints award totals and battle ranks when the result process starts. */
void fn_1_47A8(void)
{
    s16 coinAwards[5];
    s16 playerRanks[4];
    s32 player;
    s16 totalCoins;

    if (MgDataTbl[lbl_80265B70.mgNo].type != MG_TYPE_BATTLE) {
        OSReport("\n");
        OSReport("--------------Result BonusCoin-------------- \n");
        for (player = 0; player < 4; player++) {
            s16 minigameCoins = lbl_80265750[player].mgCoin;
            s16 bonusCoins = lbl_80265750[player].mgCoinBonus;

            coinAwards[player] = bonusCoins + minigameCoins;
            OSReport("Player=%d  BonusCoin=%d \n", player, coinAwards[player]);
        }
        OSReport("-------------------------------------------- \n");
        OSReport("\n");
    } else {
        OSReport("\n");
        OSReport("--------------BATTLE_MINIGAME------------- \n");
        OSReport("--------------Result GetCoin-------------- \n");
        fn_1_3FB0(coinAwards);
        for (player = totalCoins = 0; player < 5; player++) {
            totalCoins += coinAwards[player];
        }
        OSReport("TotalCoin=%d \n", totalCoins);
        for (player = 0; player < 4; player++) {
            s16 rank = lbl_80265750[player].mgCoinBonus;

            playerRanks[player] = rank;
            OSReport("Player=%d  Rank=%d  GetCoin=%d \n", player, playerRanks[player],
                coinAwards[playerRanks[player]]);
        }
        OSReport("RandomCoin=%d \n", coinAwards[4]);
        OSReport("-------------------------------------------- \n");
        OSReport("\n");
    }
}

OMOBJ *lbl_1_bss_634;
OMOBJ *lbl_1_bss_630;
OMOBJMAN *lbl_1_bss_62C;
s16 lbl_1_bss_628;
s32 lbl_1_bss_624;
s32 lbl_1_bss_620;
s16 lbl_1_bss_61C;
f32 lbl_1_bss_60C[4];
f32 lbl_1_bss_5FC[4];
s16 lbl_1_bss_5F4[4];
s16 lbl_1_bss_5EC[4];
s32 lbl_1_bss_5E8;
ResultDisplay lbl_1_bss_488[4];
s32 lbl_1_bss_320[9][10];
f32 lbl_1_bss_1B8[9][10];
f32 lbl_1_bss_50[9][10];
s32 lbl_1_bss_40[4];
s32 lbl_1_bss_30[4];
s32 lbl_1_bss_2C;
s32 lbl_1_bss_28;
f32 lbl_1_bss_24;
f32 lbl_1_bss_20;
s16 lbl_1_bss_1C;
f32 lbl_1_bss_18;
f32 lbl_1_bss_14;
f32 lbl_1_bss_10;
f32 lbl_1_bss_C;
s32 lbl_1_bss_8;
f32 lbl_1_bss_4;
s32 lbl_1_bss_0;
