/* Builds and runs the two-team minigame results display. */
#include "dolphin/math.h"
#include "game/hu3d.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
typedef struct ResultTeamDisplay {
    s32 sprites[3]; /* Team emblem, rank indicator and coin-award icon sprites. */
    s32 awardDigits[3]; /* Digit sprites for the team's coin award. */
    s32 starDigits[4]; /* Leading sprite and three star-total digits. */
    s32 coinDigits[4]; /* Leading sprite and three coin-total digits. */
    s32 nameSprite; /* Sprite showing the team name. */
    s32 characterModels[2]; /* The two character models shown for this team. */
    s32 characterMotions[2][3]; /* Idle, higher-rank and lower-rank motions per character. */
    s32 resultModels[3]; /* Award, star and coin models shown beside the team. */
} ResultTeamDisplay;
extern ResultTeamDisplay lbl_1_bss_AB8[2];

#include "dolphin/math.h"
#include "game/hu3d.h"
#include "game/window.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/wipe.h"

extern s32 lbl_1_bss_B88;
extern s32 lbl_1_bss_64C[2];
extern s32 lbl_1_bss_950[9][10];

extern void CharModelKill(s16 charNo);
extern s32 lbl_1_bss_664;
extern s16 lbl_1_bss_654;
extern float lbl_1_bss_65C;
extern float lbl_1_bss_658;
extern s16 lbl_1_bss_668[2];
extern s16 lbl_1_bss_66C[2];

extern s32 lbl_1_bss_638;
extern s32 lbl_1_bss_624;
extern float lbl_1_bss_B8C[2];
extern float lbl_1_bss_B94[2];
extern void fn_1_47A8(void);
extern void fn_1_523C(void);
extern void fn_1_4D48(void);
extern s32 fn_1_C45C(s32 dataNum);
extern void fn_1_62F0(void);
extern s16 fn_1_78B4(s32 team);
extern void fn_1_3698(void);
extern void fn_1_6B78(void);
extern s32 fn_1_356C(void);
extern void fn_1_3640(s32 readResult);
extern void fn_1_4694(void);
void fn_1_60F8(void);

extern f32 lbl_1_bss_7E8[9][10];
extern f32 lbl_1_bss_680[9][10];

extern s32 lbl_1_bss_640;
extern s32 lbl_1_bss_660;
extern Vec lbl_1_data_4F0[2];
void fn_1_71F0(void);

extern void fn_1_3FB0(s16 *coinAwards);
extern void fn_1_766C(s32 team, s32 coinAward);
extern void fn_1_73D4(s32 team, s32 coinTotal);
extern int fn_1_7840(int team, int occurrence);

#include "dolphin/math.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/window.h"

extern s32 lbl_1_bss_678[2];
extern s32 lbl_1_bss_670[2];

extern f32 lbl_1_data_3F8[4];
extern f32 lbl_1_data_408[4];
extern f32 lbl_1_data_418[4];
extern f32 lbl_1_data_428[4];
extern f32 lbl_1_data_438[4];
extern f32 lbl_1_data_448[4];
extern f32 lbl_1_data_458[4];
extern Vec lbl_1_data_468[4];
extern Vec lbl_1_data_498[2];
extern Vec lbl_1_data_4B0[2];
extern Vec lbl_1_data_4C8[2];
extern f32 lbl_1_data_4E0[4];

extern s32 lbl_1_data_50C[2];

s32 fn_1_C45C(s32 dataNum);
s16 fn_1_78B4(s32 team);
int fn_1_7840(int team, int occurrence);
void fn_1_73D4(s32 team, s32 coinTotal);
void fn_1_7520(s32 team, s32 starTotal);
void fn_1_766C(s32 team, s32 coinAward);
s32 fn_1_7750(s16 firstCharacter, s16 secondCharacter);

static inline s16 ResultNightModeGet(void)
{
    return GwMgNightF;
}

#include "dolphin/types.h"

typedef struct ResultTeamDisplayFields {
    s32 handles[26]; /* Packed fields follow ResultTeamDisplay order. */
} ResultTeamDisplayFields;

#include "dolphin/types.h"

#include "dolphin/os.h"

extern s8 lbl_1_data_514[110];

f32 lbl_1_data_3F8[4] = {318.0f, 165.0f, 258.0f, 340.0f};
f32 lbl_1_data_408[4] = {448.0f, 135.0f, 388.0f, 310.0f};
f32 lbl_1_data_418[4] = {428.0f, 185.0f, 368.0f, 360.0f};
f32 lbl_1_data_428[4] = {428.0f, 215.0f, 368.0f, 390.0f};
f32 lbl_1_data_438[4] = {138.0f, 115.0f, 78.0f, 290.0f};
f32 lbl_1_data_448[4] = {98.0f, 83.0f, 38.0f, 258.0f};
f32 lbl_1_data_458[4] = {258.0f, 193.0f, 198.0f, 368.0f};
Vec lbl_1_data_468[4] = { { 178.0f, 195.0f, 9000.0f },
                          { 338.0f, 195.0f, 9000.0f },
                          { 118.0f, 370.0f, 9000.0f },
                          { 278.0f, 370.0f, 9000.0f } };
Vec lbl_1_data_498[2] = {{258.0f, 130.0f, 9000.0f}, {198.0f, 305.0f, 9000.0f}};
Vec lbl_1_data_4B0[2] = {{403.0f, 185.0f, 9000.0f}, {343.0f, 360.0f, 9000.0f}};
Vec lbl_1_data_4C8[2] = {{403.0f, 215.0f, 9000.0f}, {343.0f, 390.0f, 9000.0f}};
f32 lbl_1_data_4E0[4] = {133.0f, 200.0f, 73.0f, 375.0f};
Vec lbl_1_data_4F0[2] = {{1.0f, 1.0f, 1.0f}, {0.75f, 0.75f, 0.75f}};
f32 lbl_1_data_508 = 1.0f;
s32 lbl_1_data_50C[2] = {-80, -80};
s8 lbl_1_data_514[110] = {
    -1, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, -1, -1, 10, 11, 12,
    13, 14, 15, 16, 17, 18, -1, -1, -1, 19, 20, 21, 22, 23, 24, 25,
    26, -1, -1, -1, -1, 27, 28, 29, 30, 31, 32, 33, -1, -1, -1, -1,
    -1, 34, 35, 36, 37, 38, 39, -1, -1, -1, -1, -1, -1, 40, 41, 42,
    43, 44, -1, -1, -1, -1, -1, -1, -1, 45, 46, 47, 48, -1, -1, -1,
    -1, -1, -1, -1, -1, 49, 50, 51, -1, -1, -1, -1, -1, -1, -1, -1,
    -1, 52, 53, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, 54
};

/* Runs team results, waiting for confirmation unless all players are computer-controlled, then
 * returns to the board. */
void fn_1_49C4(void)
{
    s32 readResult;
    HUPROCESS *parent;
    s32 confirmed = 0;
    s16 comCount;
    HUWINID window;
    s16 player;

    parent = HuPrcCurrentGet();
    fn_1_47A8();
    comCount = 0;
    for (player = comCount; player < 4; player++) {
        if (GwPlayerConf[player].type != 0) {
            comCount++;
        }
    }
    if (comCount == 4) {
        lbl_1_bss_638 = 1;
    } else {
        lbl_1_bss_638 = 0;
    }
    for (player = 0; player < 2; player++) {
        lbl_1_bss_B8C[player] = 1.0f;
        lbl_1_bss_B94[player] = 1.0f;
    }
    fn_1_523C();
    HuPrcChildCreate(fn_1_4D48, 100, 8192, 0, parent);
    WipeCreate(1, 0, 20);
    HuAudSStreamPlay(MSM_STREAM_PARTYRESULT_WAIT);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuDataDirClose(fn_1_C45C(DATANUM(DATA_result, 0)));
    HuPrcSleep(10);
    if (lbl_1_bss_624 == 0) {
        HuPrcSleep(20);
    }
    fn_1_62F0();
    for (player = 0; player < 2; player++) {
        lbl_1_bss_66C[player] = fn_1_78B4(player);
    }
    fn_1_3698();
    for (player = 0; player < 2; player++) {
        lbl_1_bss_668[player] = fn_1_78B4(player);
    }
    fn_1_6B78();
    window = HuWinCreate(-10000.0f, 405.0f, 320, 40, 0);
    HuWinMesSpeedSet(window, 0);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinPriSet(window, 5);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    if (lbl_1_bss_638 == 0) {
        HuWinMesSet(window, MESSNUM(MESS_SYS_GUIDE, 0));
    }
    HuPrcSleep(4);
    readResult = fn_1_356C();
    for (;;) {
        if (lbl_1_bss_638 != 0) {
            confirmed = 1;
        } else {
            for (player = 0; player < 4; player++) {
                if (HuPadStatGet(player) == 0 && (HuPadBtnDown[player] & PAD_BUTTON_A) != 0) {
                    HuAudFXPlay(MSM_SE_CMN_03);
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
    WipeCreate(2, 0, 60);
    HuAudSStreamAllFadeOut(1000);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_4694();
    omSysPauseEnable(1);
    fn_1_60F8();
    HuDataDirClose(fn_1_C45C(DATANUM(DATA_result, 0)));
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    for (;;) {
        HuPrcVSleep();
    }
}

/* Runs as a child of the team-results process, updating models and background sprites each
 * frame. */
void fn_1_4D48(void)
{
    f32 modelAngle[2];
    f32 coinAngle[2];
    f32 rotationX;
    f32 rotationY;
    f32 rotationZ;
    s16 row;
    s16 column;

    rotationX = 0.0f;
    rotationY = 0.0f;
    rotationZ = 0.0f;
    for (row = 0; row < 2; row++) {
        modelAngle[row] = 0.0f;
        coinAngle[row] = 0.0f;
    }
    while (1) {
        if (lbl_1_bss_638 != 0) {
            lbl_1_bss_624 = 1;
        } else {
            lbl_1_bss_624 = 0;
            for (row = 0; row < 4; row++) {
                if (HuPadStatGet(row) == 0 && (HuPadBtn[row] & PAD_BUTTON_A) != 0) {
                    lbl_1_bss_624 = 1;
                }
            }
        }
        for (row = 0; row < 2; row++) {
            modelAngle[row] += 1.2f * lbl_1_bss_B8C[row];
            if (modelAngle[row] >= 360.0f) {
                modelAngle[row] -= 360.0f;
            }
            coinAngle[row] += lbl_1_bss_B94[row];
            if (coinAngle[row] >= 360.0f) {
                coinAngle[row] -= 360.0f;
            }
            /* Both teams' rotations are set on each loop iteration, twice per model each frame. */
            Hu3DModelRotSet((s16)lbl_1_bss_AB8[0].resultModels[1], 0.0f, modelAngle[0], 0.0f);
            Hu3DModelRotSet((s16)lbl_1_bss_AB8[1].resultModels[1], 0.0f, modelAngle[1], 0.0f);
            Hu3DModelRotSet((s16)lbl_1_bss_AB8[0].resultModels[2], 0.0f, coinAngle[0], 0.0f);
            Hu3DModelRotSet((s16)lbl_1_bss_AB8[1].resultModels[2], 0.0f, coinAngle[1], 0.0f);
        }
        for (row = 0; row < 9; row++) {
            for (column = 0; column < 10; column++) {
                lbl_1_bss_7E8[row][column] += 1.0f;
                lbl_1_bss_680[row][column] += 1.0f;
                if (lbl_1_bss_7E8[row][column] > 616.0f) {
                    lbl_1_bss_7E8[row][column] = -80.0f;
                }
                if (lbl_1_bss_680[row][column] > 520.0f) {
                    lbl_1_bss_680[row][column] = -80.0f;
                }
                espPosSet(lbl_1_bss_950[row][column], lbl_1_bss_7E8[row][column],
                    lbl_1_bss_680[row][column]);
            }
        }
        HuPrcVSleep();
    }
}

/* Creates the team panels, character models, score digits and drifting background sprites before
 * display. */
void fn_1_523C(void)
{
    Vec position;
    int characters[2];
    int characterOffset;
    int team;
    int slot;
    int positionIndex;
    int rank;
    int character;
    int displayId;
    int language;
    s32 message;

    language = GWLanguageGet();
    if (language == 1) {
        lbl_1_bss_B88 = displayId = espEntry(fn_1_C45C(DATANUM(DATA_result, 5)), 180, 0);
        espPosSet(displayId, 288.0, 60.0);
        espDrawNoSet(displayId, 64);
    } else {
        lbl_1_bss_B88 = displayId = espEntry(fn_1_C45C(DATANUM(DATA_result, 7)), 180, 0);
        espPosSet(displayId, 288.0, 60.0);
        espDrawNoSet(displayId, 64);
    }
    for (team = 0; team < 2; team++) {
        positionIndex = team * 2;
        rank = fn_1_78B4(team);
        lbl_1_bss_AB8[team].sprites[0] = displayId =
            espEntry(fn_1_C45C(DATANUM(DATA_result, 13) + team), 180, 0);
        espPosSet(displayId, lbl_1_data_3F8[positionIndex], lbl_1_data_3F8[positionIndex + 1]);
        espScaleSet(displayId, 0.93f, 0.93f);
        espDrawNoSet(displayId, 64);

        lbl_1_bss_AB8[team].sprites[1] = displayId =
            espEntry(fn_1_C45C(DATANUM(DATA_result, 3)), 150, 0);
        espPosSet(displayId, lbl_1_data_408[positionIndex], lbl_1_data_408[positionIndex + 1]);
        espBankSet(displayId, positionIndex + rank);
        espDrawNoSet(displayId, 64);

        lbl_1_bss_AB8[team].sprites[2] = displayId =
            espEntry(fn_1_C45C(DATANUM(DATA_result, 4)), 130, 0);
        espPosSet(displayId, lbl_1_data_438[positionIndex], lbl_1_data_438[positionIndex + 1]);
        espDrawNoSet(displayId, 64);
        espScaleSet(displayId, 0.0, 0.0);
        espAttrSet(displayId, HUSPR_ATTR_DISPOFF);

        if (language == 1) {
            lbl_1_bss_AB8[team].nameSprite = displayId =
                espEntry(fn_1_C45C(DATANUM(DATA_result, 15) + team), 130, 0);
            espPosSet(displayId, lbl_1_data_458[positionIndex], lbl_1_data_458[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        } else {
            lbl_1_bss_AB8[team].nameSprite = displayId =
                espEntry(fn_1_C45C(DATANUM(DATA_result, 17) + team), 130, 0);
            espPosSet(displayId, lbl_1_data_458[positionIndex], lbl_1_data_458[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        }
        for (slot = 0; slot < 3; slot++) {
            if (slot == 0) {
                lbl_1_bss_AB8[team].awardDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 0)), 120, 10);
            } else {
                lbl_1_bss_AB8[team].awardDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 0)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_448[positionIndex] + (f32)(slot * 18),
                lbl_1_data_448[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
            espScaleSet(displayId, 0.0, 0.0);
        espAttrSet(displayId, HUSPR_ATTR_DISPOFF);
        }
        for (slot = 0; slot < 4; slot++) {
            if (slot == 0) {
                lbl_1_bss_AB8[team].starDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 1)), 120, 10);
            } else {
                lbl_1_bss_AB8[team].starDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 1)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_418[positionIndex] + (f32)(slot * 17),
                lbl_1_data_418[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        }
        for (slot = 0; slot < 4; slot++) {
            if (slot == 0) {
                lbl_1_bss_AB8[team].coinDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 1)), 120, 10);
            } else {
                lbl_1_bss_AB8[team].coinDigits[slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 1)), 120, 0);
            }
            espPosSet(displayId, lbl_1_data_428[positionIndex] + (f32)(slot * 17),
                lbl_1_data_428[positionIndex + 1]);
            espDrawNoSet(displayId, 64);
        }
        lbl_1_bss_678[team] = GWPlayerCoinGet(fn_1_7840(team, 0));
        lbl_1_bss_670[team] = GWPlayerStarGet(fn_1_7840(team, 0));
        fn_1_73D4(team, lbl_1_bss_678[team]);
        fn_1_7520(team, lbl_1_bss_670[team]);
        fn_1_766C(team, 0);

        for (slot = 0; slot < 2; slot++) {
            character = GwPlayerConf[fn_1_7840(team, slot)].charNo;
            characterOffset = character * 4;
            characters[slot] = GwPlayerConf[fn_1_7840(team, slot)].charNo;
            lbl_1_bss_AB8[team].characterModels[slot] = displayId = CharModelCreate(character, 2);
            lbl_1_bss_AB8[team].characterMotions[slot][0] =
                CharMotionCreate(character, CHARMOT_DATANUM(0, mario));
            lbl_1_bss_AB8[team].characterMotions[slot][1] =
                CharMotionCreate(character, CHARMOT_DATANUM(36, mario));
            lbl_1_bss_AB8[team].characterMotions[slot][2] =
                CharMotionCreate(character, CHARMOT_DATANUM(37, mario));
            CharMotionDataClose(character);
            Hu3DMotionSet(displayId, lbl_1_bss_AB8[team].characterMotions[slot][0]);
            Hu3DModelAttrSet(displayId, HU3D_MOTATTR_LOOP);
            Hu3DModelCameraSet(displayId, 1);
            Hu3D2Dto3D(&lbl_1_data_468[slot + team * 2], 1, &position);
            Hu3DModelPosSetV(displayId, &position);
            Hu3DModelScaleSetV(displayId, &lbl_1_data_4F0[rank]);
            Hu3DModelLayerSet(displayId, 1);
        }
        lbl_1_bss_AB8[team].resultModels[0] = displayId = Hu3DModelCreate(
            HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 22)), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_498[team], 1, &position);
        Hu3DModelPosSetV(displayId, &position);
        Hu3DModelLayerSet(displayId, 1);
        Hu3DModelScaleSet(displayId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrSet(displayId, HU3D_ATTR_DISPOFF);
        Hu3DModelAttrSet(displayId, HU3D_MOTATTR_LOOP);

        lbl_1_bss_AB8[team].resultModels[1] = displayId = Hu3DModelCreate(
            HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 25)), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_4B0[team], 1, &position);
        Hu3DModelScaleSet(displayId, 0.6f, 0.6f, 0.6f);
        Hu3DModelPosSetV(displayId, &position);
        Hu3DModelLayerSet(displayId, 1);

        lbl_1_bss_AB8[team].resultModels[2] = displayId = Hu3DModelCreate(
            HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 26)), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(displayId, 1);
        Hu3D2Dto3D(&lbl_1_data_4C8[team], 1, &position);
        Hu3DModelScaleSet(displayId, 0.5f, 0.5f, 0.5f);
        Hu3DModelPosSetV(displayId, &position);
        Hu3DModelLayerSet(displayId, 1);

        message = fn_1_7750(characters[0], characters[1]);
        lbl_1_bss_64C[team] = HuWinCreate(lbl_1_data_4E0[positionIndex],
            lbl_1_data_4E0[positionIndex + 1], 240, 32, 0);
        /* The requested opacity is 255.0f; the sprite setter multiplies it by 255 before
        * storing it in the byte alpha field. */
        HuWinBGTPLvlSet(lbl_1_bss_64C[team], 255.0f);
        HuWinPriSet(lbl_1_bss_64C[team], 0);
        HuWinAttrSet(lbl_1_bss_64C[team], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSpeedSet(lbl_1_bss_64C[team], 0);
        HuWinMesSet(lbl_1_bss_64C[team], message);
    }
    for (team = 0; team < 9; team++) {
        for (slot = 0; slot < 10; slot++) {
            if (ResultNightModeGet() == 0) {
                lbl_1_bss_950[team][slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 19)), 256, 0);
            } else {
                lbl_1_bss_950[team][slot] = displayId =
                    espEntry(fn_1_C45C(DATANUM(DATA_result, 20)), 256, 0);
            }
            lbl_1_bss_7E8[team][slot] = lbl_1_data_50C[0] + (9 - slot) * 80;
            lbl_1_bss_680[team][slot] = lbl_1_data_50C[1] + team * 80;
            espPosSet(displayId, lbl_1_bss_7E8[team][slot], lbl_1_bss_680[team][slot]);
            espDrawNoSet(displayId, 64);
        }
    }
    HuSprExecLayerCameraSet(64, 1, 1);
}

/* Runs after fn_1_4694 has cleared model and sprite state, issuing cleanup calls with the
 * retained team handles and windows. */
void fn_1_60F8(void)
{
    s32 team;
    s32 spriteIndex;

    espKill(lbl_1_bss_B88);
    for (team = 0; team < 2; team++) {
        espKill(lbl_1_bss_AB8[team].sprites[0]);
        espKill(lbl_1_bss_AB8[team].sprites[1]);
        espKill(lbl_1_bss_AB8[team].sprites[2]);
        espKill(lbl_1_bss_AB8[team].nameSprite);
        for (spriteIndex = 0; spriteIndex < 3; spriteIndex++) {
            espKill(lbl_1_bss_AB8[team].awardDigits[spriteIndex]);
        }
        for (spriteIndex = 0; spriteIndex < 4; spriteIndex++) {
            espKill(lbl_1_bss_AB8[team].starDigits[spriteIndex]);
            espKill(lbl_1_bss_AB8[team].coinDigits[spriteIndex]);
        }
        Hu3DModelKill((s16)lbl_1_bss_AB8[team].resultModels[0]);
        Hu3DModelKill((s16)lbl_1_bss_AB8[team].resultModels[1]);
        Hu3DModelKill((s16)lbl_1_bss_AB8[team].resultModels[2]);
        HuWinKill((s16)lbl_1_bss_64C[team]);
    }
    for (team = 0; team < 9; team++) {
        for (spriteIndex = 0; spriteIndex < 10; spriteIndex++) {
            espKill(lbl_1_bss_950[team][spriteIndex]);
        }
    }
    CharModelKill(-1);
}

/* Runs after the result panels appear, counting each team's minigame coin award into its total. */
void fn_1_62F0(void)
{
    s16 coinAwards[4];
    s16 teamAwards[2] = {0, 0};
    s16 coinTotals[2];
    s16 team;
    s16 digit;
    s16 finished;
    s16 frame;
    s16 bounceFrame;
    f32 scale;
    f32 elapsed;
    f32 increment;

    fn_1_3FB0(coinAwards);
    for (team = 0; team < 4; team++) {
        teamAwards[GwPlayer[team].team] += coinAwards[team];
    }
    for (team = 0; team < 2; team++) {
        for (digit = 0; digit < 3; digit++) {
            espAttrReset(lbl_1_bss_AB8[team].awardDigits[digit], HUSPR_ATTR_DISPOFF);
        }
        fn_1_766C(team, teamAwards[team]);
        espAttrReset(lbl_1_bss_AB8[team].sprites[2], HUSPR_ATTR_DISPOFF);
    }
    finished = lbl_1_bss_624 != 0 ? 3 : 10;
    for (frame = 0; frame <= finished; frame++) {
        scale = HuSin((90.0 / finished) * frame);
        for (team = 0; team < 2; team++) {
            espScaleSet(lbl_1_bss_AB8[team].sprites[2], scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_AB8[team].awardDigits[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (team = 0; team < 2; team++) {
        espScaleSet(lbl_1_bss_AB8[team].sprites[2], 1.0, 1.0);
    }
    for (bounceFrame = 0; bounceFrame <= 14;) {
        for (team = 0; team < 2; team++) {
            if (teamAwards[team] >= 10) {
                Hu3DModelAttrReset((s16)lbl_1_bss_AB8[team].resultModels[0], HU3D_ATTR_DISPOFF);
            }
            scale = HuSin((120.0f / 14.0f) * bounceFrame) *
                (0.8 * (1.0 / sin((M_PI * 2) / 3))) + 0.1f;
            Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].resultModels[0], scale, scale, scale);
        }
        bounceFrame += lbl_1_bss_624 != 0 ? 2 : 1;
        HuPrcVSleep();
    }
    if (lbl_1_bss_624 == 0) {
        HuPrcSleep(15);
    }
    finished = 0;
    elapsed = finished;
    for (team = 0; team < 2; team++) {
        coinTotals[team] = GWPlayerCoinGet(fn_1_7840(team, 0));
    }
    for (;;) {
        if (lbl_1_bss_624 != 0) {
            increment = 1.0f;
        } else {
            increment = 0.2f;
        }
        elapsed += increment;
        if (elapsed >= 1.0f) {
            for (team = finished = 0; team < 2; team++) {
                if (teamAwards[team] == 0) {
                    fn_1_766C(team, 0);
                    lbl_1_bss_B94[team] = 1.0f;
                    finished++;
                } else {
                    coinTotals[team]++;
                    teamAwards[team]--;
                    HuAudFXPlay(MSM_SE_CMN_08);
                    lbl_1_bss_B94[team] = 3.0f;
                }
                fn_1_766C(team, teamAwards[team]);
                fn_1_73D4(team, coinTotals[team]);
            }
            elapsed = 0.0f;
        }
        if (finished >= 2) {
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(30);
    finished = lbl_1_bss_624 != 0 ? 3 : 10;
    for (frame = 0; frame <= finished; frame++) {
        scale = HuCos((90.0 / finished) * frame);
        for (team = 0; team < 2; team++) {
            espScaleSet(lbl_1_bss_AB8[team].sprites[2], scale, scale);
            Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].resultModels[0], scale, scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_AB8[team].awardDigits[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (team = 0; team < 2; team++) {
        espAttrSet(lbl_1_bss_AB8[team].sprites[2], HUSPR_ATTR_DISPOFF);
        Hu3DModelAttrSet(lbl_1_bss_AB8[team].resultModels[0], HU3D_ATTR_DISPOFF);
        for (digit = 0; digit < 3; digit++) {
            espAttrSet(lbl_1_bss_AB8[team].awardDigits[digit], HUSPR_ATTR_DISPOFF);
        }
    }
}

/* Runs after coin awards to animate characters and update each team's rank indicator. */
void fn_1_6B78(void)
{
    s32 team;
    s32 frame;
    s32 unchangedCount;
    s32 cycle;
    f32 scale[2];
    f32 targetScale[2];
    f32 scaleStep[2];
    s32 changed[2];
    s32 transitionFrame;

    unchangedCount = 0;
    transitionFrame = 0;
    for (team = 0; team < 2; team++) {
        if ((s16)lbl_1_bss_66C[team] > lbl_1_bss_668[team]) {
            changed[team] = 1;
            Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[0],
                (s16)lbl_1_bss_AB8[team].characterMotions[0][1], 0.0f, 8.0f, HU3D_MOTATTR_NONE);
            Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[1],
                (s16)lbl_1_bss_AB8[team].characterMotions[1][1], 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        } else if ((s16)lbl_1_bss_66C[team] < lbl_1_bss_668[team]) {
            changed[team] = 1;
            Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[0],
                (s16)lbl_1_bss_AB8[team].characterMotions[0][2], 0.0f, 8.0f, HU3D_MOTATTR_NONE);
            Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[1],
                (s16)lbl_1_bss_AB8[team].characterMotions[1][2], 0.0f, 8.0f, HU3D_MOTATTR_NONE);
        } else {
            changed[team] = 0;
            unchangedCount++;
        }
        if (changed[team] != 0) {
            scale[team] = lbl_1_data_4F0[lbl_1_bss_66C[team]].x;
            targetScale[team] = lbl_1_data_4F0[lbl_1_bss_668[team]].x;
            scaleStep[team] = (targetScale[team] - scale[team]) / 30.0f;
        }
    }
    if (unchangedCount >= 2) {
        lbl_1_bss_640 = 1;
        return;
    }
    lbl_1_bss_664 = 1;
    lbl_1_bss_660 = 0;
    lbl_1_bss_65C = 1.0f;
    lbl_1_bss_658 = 1.0f / 35.0f;
    lbl_1_bss_654 = 0;
    for (cycle = 0; cycle < 3; cycle++) {
        for (frame = 0; frame < 20; frame++) {
            for (team = 0; team < 2; team++) {
                if (changed[team] != 0) {
                    scale[team] += scaleStep[team];
                    Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].characterModels[0],
                        scale[team], scale[team], scale[team]);
                    Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].characterModels[1],
                        scale[team], scale[team], scale[team]);
                }
            }
            fn_1_71F0();
            HuPrcVSleep();
        }
        for (frame = 0; frame < 10.0f; frame++) {
            for (team = 0; team < 2; team++) {
                if (changed[team] != 0) {
                    scale[team] -= scaleStep[team];
                    Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].characterModels[0],
                        scale[team], scale[team], scale[team]);
                    Hu3DModelScaleSet((s16)lbl_1_bss_AB8[team].characterModels[1],
                        scale[team], scale[team], scale[team]);
                }
            }
            fn_1_71F0();
            HuPrcVSleep();
        }
    }
    for (team = 0; team < 2; team++) {
        espScaleSet(lbl_1_bss_AB8[team].sprites[1], 1.0, 1.0);
        Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[0],
            (s16)lbl_1_bss_AB8[team].characterMotions[0][0], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        Hu3DMotionShiftSet((s16)lbl_1_bss_AB8[team].characterModels[1],
            (s16)lbl_1_bss_AB8[team].characterMotions[1][0], 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    }
    lbl_1_bss_640 = 1;
}

/* Updates the rank indicator during the shrink, bank-change and grow animation in fn_1_6B78. */
void fn_1_71F0(void)
{
    s32 team;
    s32 bankChanged = 0;

    if (lbl_1_bss_664 != 0) {
        if (lbl_1_bss_654 == 0) {
            lbl_1_bss_65C -= lbl_1_bss_658;
            if (lbl_1_bss_65C <= 0.0f) {
                lbl_1_bss_65C = 0.0f;
                lbl_1_bss_654 = 1;
                bankChanged = 1;
            }
        } else {
            lbl_1_bss_65C += lbl_1_bss_658;
            if (lbl_1_bss_65C >= 1.0f) {
                lbl_1_bss_664 = 0;
            }
        }
        for (team = 0; team < 2; team++) {
            if (lbl_1_bss_66C[team] != lbl_1_bss_668[team]) {
                if (bankChanged != 0) {
                    espBankSet(lbl_1_bss_AB8[team].sprites[1], lbl_1_bss_668[team] + team * 2);
                }
                espScaleSet(lbl_1_bss_AB8[team].sprites[1], lbl_1_bss_65C, lbl_1_bss_65C);
            }
        }
    }
}

/* Updates the three coin-total digits when the team panel is built and as awards count up. */
void fn_1_73D4(s32 team, s32 coinTotal)
{
    ResultTeamDisplayFields displayFields = ((ResultTeamDisplayFields *)lbl_1_bss_AB8)[team];
    s32 digits[3];
    s32 remainder;

    if (coinTotal > 999) {
        coinTotal = 999;
    }
    remainder = coinTotal;
    digits[0] = coinTotal / 100;
    if (digits[0] == 0) {
        espAttrSet(displayFields.handles[11], HUSPR_ATTR_DISPOFF);
    } else {
        espAttrReset(displayFields.handles[11], HUSPR_ATTR_DISPOFF);
        espBankSet(displayFields.handles[11], digits[0]);
    }
    remainder = coinTotal - (digits[0] * 100);
    digits[1] = remainder / 10;
    if ((digits[1] == 0) && (digits[0] == 0)) {
        espAttrSet(displayFields.handles[12], HUSPR_ATTR_DISPOFF);
    } else {
        espAttrReset(displayFields.handles[12], HUSPR_ATTR_DISPOFF);
        espBankSet(displayFields.handles[12], digits[1]);
    }
    remainder -= digits[1] * 10;
    espBankSet(displayFields.handles[13], remainder);
}

/* Updates the three star-total digits while the team result panel is built. */
void fn_1_7520(s32 team, s32 starTotal)
{
    ResultTeamDisplayFields displayFields = ((ResultTeamDisplayFields *)lbl_1_bss_AB8)[team];
    s32 digits[3];
    s32 remainder;

    if (starTotal > 999) {
        starTotal = 999;
    }
    remainder = starTotal;
    digits[0] = starTotal / 100;
    if (digits[0] == 0) {
        espAttrSet(displayFields.handles[7], HUSPR_ATTR_DISPOFF);
    } else {
        espAttrReset(displayFields.handles[7], HUSPR_ATTR_DISPOFF);
        espBankSet(displayFields.handles[7], digits[0]);
    }
    remainder = starTotal - (digits[0] * 100);
    digits[1] = remainder / 10;
    if ((digits[1] == 0) && (digits[0] == 0)) {
        espAttrSet(displayFields.handles[8], HUSPR_ATTR_DISPOFF);
    } else {
        espAttrReset(displayFields.handles[8], HUSPR_ATTR_DISPOFF);
        espBankSet(displayFields.handles[8], digits[1]);
    }
    remainder -= digits[1] * 10;
    espBankSet(displayFields.handles[9], remainder);
}

/* Updates the two visible digits of a team's coin award during the count-up. */
void fn_1_766C(s32 team, s32 coinAward)
{
    ResultTeamDisplayFields displayFields = ((ResultTeamDisplayFields *)lbl_1_bss_AB8)[team];
    s32 digits[3];
    s32 remainder;

    if (coinAward > 99) {
        coinAward = 99;
    }
    remainder = coinAward;
    digits[0] = coinAward / 10;
    if (digits[0] == 0) {
        espAttrSet(displayFields.handles[4], HUSPR_ATTR_DISPOFF);
    } else {
        espAttrReset(displayFields.handles[4], HUSPR_ATTR_DISPOFF);
        espBankSet(displayFields.handles[4], digits[0]);
    }
    remainder -= digits[0] * 10;
    espBankSet(displayFields.handles[5], remainder);
}

/* Selects the team introduction message for its two characters while panels are built. */
s32 fn_1_7750(s16 firstCharacter, s16 secondCharacter)
{
    s32 messageIndex;

    if ((firstCharacter > 10) || (secondCharacter > 10)) {
        return MESSNUM(MESS_TAG_NAME, 55);
    }
/* The lookup stores one result for each unordered character pair. */
    if (firstCharacter < secondCharacter) {
        messageIndex = lbl_1_data_514[secondCharacter + (firstCharacter * 11)];
    } else {
        messageIndex = lbl_1_data_514[firstCharacter + (secondCharacter * 11)];
    }
    OSReport("%d:%d->%d\n", firstCharacter, secondCharacter, messageIndex);
    if (messageIndex == -1) {
        return MESSNUM(MESS_TAG_NAME, 55);
    }
    return MESSNUM(MESS_TAG_NAME, 0) + messageIndex;
}

f32 lbl_1_bss_B94[2];
f32 lbl_1_bss_B8C[2];
s32 lbl_1_bss_B88;
ResultTeamDisplay lbl_1_bss_AB8[2];
s32 lbl_1_bss_950[9][10];
f32 lbl_1_bss_7E8[9][10];
f32 lbl_1_bss_680[9][10];
s32 lbl_1_bss_678[2];
s32 lbl_1_bss_670[2];
s16 lbl_1_bss_66C[2];
s16 lbl_1_bss_668[2];
s32 lbl_1_bss_664;
s32 lbl_1_bss_660;
f32 lbl_1_bss_65C;
f32 lbl_1_bss_658;
s16 lbl_1_bss_654;
s32 lbl_1_bss_64C[2];
s32 lbl_1_bss_644[2];
s32 lbl_1_bss_640;
s32 lbl_1_bss_63C;
s32 lbl_1_bss_638;
