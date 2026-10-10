/* Shows battle minigame standings and coin awards for all four players. */
#include "dolphin/math.h"
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "game/hu3d.h"
typedef struct ResultSceneHandles {
    s32 spriteId[2];
    s32 scoreSpriteId[3];
    s32 modelId;
    s32 characterModelId;
    s32 idleMotionId;
    s32 motionId[3];
    s32 arrivalMotionId;
} ResultSceneHandles;
typedef struct ResultParticleGroup {
    Vec pos;
    s16 modelId;
    s16 activeCount;
} ResultParticleGroup;
extern ResultSceneHandles lbl_1_bss_FAC[4];

#include "dolphin/math.h"
#include "game/gamework.h"
#include "game/board/player.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/process.h"
#include "game/window.h"
#include "game/wipe.h"
#include "string.h"
#include "game/charman.h"
#include "game/sprite.h"

#define RESULT_MOTION_IDLE 9633792
#define RESULT_MOTION_AWARD_REACTION 9633828
#define RESULT_MOTION_NO_AWARD_REACTION 9633829
#define RESULT_MOTION_ENTRY 9633795
#define RESULT_MOTION_ARRIVAL 9633796

extern s32 lbl_1_bss_1094[GW_PLAYER_MAX];
extern ResultParticleGroup lbl_1_bss_10A4[64];
extern s32 lbl_1_bss_BA0;
extern s32 lbl_1_bss_624;

extern void fn_1_AC54(void);
extern void fn_1_47A8(void);
extern void fn_1_7E0C(void);
extern void fn_1_7D34(void);
extern s32 fn_1_C45C(s32 dataNum);
extern void fn_1_8858(void);
extern void fn_1_8D4C(void);
extern void fn_1_924C(void);
extern void fn_1_95D8(void);
extern void fn_1_3698(void);
extern s32 fn_1_356C(void);
extern void fn_1_ACB8(void);
extern void fn_1_3640(s32 readResult);
extern void fn_1_86B0(void);
extern void fn_1_4694(void);

/* Runs the result presentation and waits for a player to continue unless all players are
 * computer-controlled. */

extern s32 lbl_1_bss_1070;
extern s32 lbl_1_bss_106C;
extern f32 lbl_1_bss_1074[4];
extern s32 lbl_1_bss_1084[4];
extern s32 lbl_1_bss_DAC[128];
extern ANIMDATA *lbl_1_bss_BA4;
extern f32 lbl_1_data_590[8];
extern f32 lbl_1_data_5B0[8];
extern f32 lbl_1_data_5D0[8];
extern HuVecF lbl_1_data_5F0[4];
extern HuVecF lbl_1_data_650[4];
extern HuVecF lbl_1_data_680[4];
extern HuVecF lbl_1_data_780;
extern f32 lbl_1_data_78C[4];
extern f32 lbl_1_data_79C[4];
extern void fn_1_9D18(void);

/* Builds the battle-result rows and the models used when their coins are awarded. */

#include "game/gamework.h"

#include "game/process.h"

typedef struct ResultCleanupRecord {
    s16 value[8];
} ResultCleanupRecord;

extern void fn_1_B39C(s16 modelId);
extern void CharModelKill(s16 charNo);

/* Clears result-screen sprites, models, and active per-player effects when the scene ends. */

extern HuVecF lbl_1_data_620[4];
extern HuVecF lbl_1_data_6B0[4];

/* Brings the character models into their result rows, then starts their arrival motions. */

#include "dolphin/math.h"
/* Animates the result models falling into the displayed player rows. */

#include "game/audio.h"
#include "game/process.h"

extern s32 lbl_1_bss_BA8;

extern s16 lbl_1_bss_61C;

extern f32 lbl_1_data_6F0[4];
extern void fn_1_A238(HuVecF *position);
extern void fn_1_3FB0(s16 *coinAwards);

/* Drops each rank's battle coin award and waits until all of the coins have landed. */

/* Drops each model after an eight-frame stagger and waits for the final impact. */

extern void fn_1_3FB0(s16 awards[5]);
extern void fn_1_9B90(s32 player, s32 value);

/* Reveals coin awards, plays each player's reaction, then hides the score resultHandles. */

#include "dolphin/types.h"
#include "game/gamework.h"

extern f32 lbl_1_data_7AC[8][4];

/* Updates the three score digits shown for one player in the results display. */

/* Sorts players by their coin bonuses and chooses row offsets for tied ranks. */

/* Updates the particle effects drawn over the result presentation. */
#include "dolphin/math.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/frand.h"

typedef struct ResultParticle {
    s16 time;
    s16 state[3];
    Vec velocity;
    float scaleSpeed;
    float verticalAccel;
    float speedDecay;
    float driftSpeed;
    float driftAngle;
    float spinSpeed;
    float scale;
    float rotationX;
    float rotationY;
    float zRot;
    Vec pos;
    u8 color[4];
    s16 animBank;
    u8 animationState[14];
} ResultParticle;

typedef struct ResultParticleSystem {
    s16 state;
    s16 group;
    Vec pos;
    Vec spawnCenter;
    void *work;
    s8 blendMode;
    u8 attributes;
    s16 sortEnabled;
    s16 modelId;
    s16 particleCount;
    u8 renderSettings[64];
    ResultParticle *particles;
} ResultParticleSystem;

extern GXColor lbl_1_data_82C[4];
extern u8 lbl_1_data_83C[16];
extern u8 lbl_1_data_84C[32];
extern s16 fn_1_AD24(ANIMDATA *anim, s16 count);
extern void fn_1_C420(s16 modelId, void *callback);
void fn_1_A928(void *model, ResultParticleSystem *system, Mtx matrix);

/* Returns the distance magnitude used when grouping nearby coin landing effects. */
static inline float fabsf2(register float x)
{
    asm {
        fabs x, x
    }
    return x;
}
/* Adds a burst to a nearby result particle group, creating its model when needed. */

/* Advances a result effect's particles during drawing and records how many remain alive. */

/* Returns the requested member of a team, or -1 if that occurrence is absent. */
int fn_1_7840(int team, int occurrence)
{
    int foundCount = 0;
    int player;

    for (player = 0; player < GW_PLAYER_MAX; player++) {
        if (team == mbPlayerGrpGet(player)) {
            if (foundCount == occurrence) {
                return player;
            }
            foundCount++;
        }
    }
    return -1;
}

/* Returns the team's rank from the coin and star totals of each team's first player. */
s16 fn_1_78B4(s32 team)
{
    s32 teamScore[2];
    s32 teamIndex;
    s32 rank;

    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        teamScore[teamIndex] = GWPlayerCoinGet(fn_1_7840(teamIndex, 0))
            | (GWPlayerStarGet(fn_1_7840(teamIndex, 0)) << 11);
    }

    rank = 0;
    for (teamIndex = 0; teamIndex < 2; teamIndex++) {
        if ((teamIndex != team) && (teamScore[team] < teamScore[teamIndex])) {
            rank++;
        }
    }
    return rank;
}

/* Runs the battle-results presentation, waiting for a player unless all players are
 * computer-controlled. */
void fn_1_7A18(void)
{
    s16 readResult;
    HUPROCESS *parent;
    s32 confirmed = 0;
    s16 comCount;
    HUWINID window;
    s16 player;

    parent = HuPrcCurrentGet();
    for (player = 0; player < GW_PLAYER_MAX; player++) {
        lbl_1_bss_1094[player] = 0;
    }
    fn_1_AC54();
    fn_1_47A8();
    memset(lbl_1_bss_10A4, 0, sizeof(lbl_1_bss_10A4));
    comCount = 0;
    for (player = comCount; player < GW_PLAYER_MAX; player++) {
        if (GwPlayerConf[player].type != 0) {
            comCount++;
        }
    }

    if (comCount == GW_PLAYER_MAX) {
        lbl_1_bss_BA0 = 1;
    } else {
        lbl_1_bss_BA0 = 0;
    }
    fn_1_7E0C();
    HuPrcChildCreate(fn_1_7D34, 100, 8192, 0, parent);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 20);
    HuAudSStreamPlay(MSM_STREAM_STORY_WIN);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    HuDataDirClose(fn_1_C45C(DATANUM(DATA_result, 0)));
    HuPrcSleep(10);
    if (lbl_1_bss_624 == 0) {
        HuPrcSleep(20);
    }
    fn_1_8858();
    fn_1_8D4C();
    fn_1_924C();
    fn_1_95D8();
    fn_1_3698();
    window = HuWinCreate(HUWIN_POS_CENTER, 400.0f, 320, 40, 0);
    HuWinMesSpeedSet(window, 0);
    HuWinBGTPLvlSet(window, 0.0f);
    HuWinPriSet(window, 5);
    HuWinAttrSet(window, HUWIN_ATTR_ALIGN_CENTER);
    if (lbl_1_bss_BA0 == 0) {
        HuWinMesSet(window, MESSNUM(MESS_SYS_GUIDE, 0));
    }
    HuPrcSleep(4);
    readResult = fn_1_356C();
    for (;;) {
        if (lbl_1_bss_BA0 != 0) {
            confirmed = 1;
        } else {
            for (player = 0; player < GW_PLAYER_MAX; player++) {
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
    fn_1_ACB8();
    fn_1_3640(readResult);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    HuAudSStreamAllFadeOut(1000);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_86B0();
    fn_1_4694();
    omSysPauseEnable(1);
    HuDataDirClose(fn_1_C45C(DATANUM(DATA_result, 0)));
    omOvlReturnEx(1, 1);
    HuPrcEnd();
}

/* Child of fn_1_7A18; polls held A to set the animation-speed flag every frame. All-computer
 * games force the fast setting. */
void fn_1_7D34(void) {
    f32 unusedX;
    f32 unusedY;
    f32 unusedZ;
    s32 player;

    unusedX = 0.0f;
    unusedY = 0.0f;
    unusedZ = 0.0f;
    for (;;) {
        if ((s32) lbl_1_bss_BA0 != 0) {
            lbl_1_bss_624 = 1;
        } else {
            lbl_1_bss_624 = 0;
            player = 0;
            while (player < 4) {
                if ((HuPadStatGet((s16) player) == 0) &&
                    ((s32) (HuPadBtn[player] & PAD_BUTTON_A) != 0)) {
                    lbl_1_bss_624 = 1;
                }
                player += 1;
            }
        }
        HuPrcVSleep();
    }
}

/* Creates the result row resultHandles, character models, and coin models before the show
 * begins. */
void fn_1_7E0C(void)
{
    HuVecF worldPosition;
    HuVecF screenPosition;
    s32 characterIndex[1];
    s32 handle;
    s32 player;
    s32 character;
    s32 digit;
    s32 positionIndex;
    s32 language;

    fn_1_9D18();
    language = GWLanguageGet();
    if (language == 1) {
        handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 6)), 180, 0);
        lbl_1_bss_1070 = handle;
        espPosSet(handle, 288.0f, 80.0f);
        espDrawNoSet(handle, 64);
    } else {
        handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 8)), 180, 0);
        lbl_1_bss_1070 = handle;
        espPosSet(handle, 288.0f, 80.0f);
        espDrawNoSet(handle, 64);
    }
    handle = Hu3DModelCreate(
        HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 24)), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_106C = handle;
    Hu3DModelCameraSet(handle, 1);
    Hu3D2Dto3D(&lbl_1_data_780, 1, &worldPosition);
    Hu3DModelPosSetV(handle, &worldPosition);
    Hu3DModelAttrSet(handle, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(handle, 0);
    for (player = 0; player < 4; player++) {
        positionIndex = player * 2;
        handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 2)), 150, 0);
        lbl_1_bss_FAC[player].spriteId[0] = handle;
        espPosSet(handle, lbl_1_data_590[positionIndex] + lbl_1_bss_1074[player],
            lbl_1_data_590[positionIndex + 1] + lbl_1_data_78C[lbl_1_bss_1094[player]]);
        espBankSet(handle, lbl_1_bss_1094[player]);
        espDrawNoSet(handle, 64);
        espScaleSet(handle, 0.0f, 0.0f);
        handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 4)), 130, 1);
        lbl_1_bss_FAC[player].spriteId[1] = handle;
        espPosSet(handle, lbl_1_data_5B0[positionIndex] + lbl_1_bss_1074[player],
            lbl_1_data_5B0[positionIndex + 1] + lbl_1_data_78C[lbl_1_bss_1094[player]]);
        espDrawNoSet(handle, 64);
        espScaleSet(handle, 0.0f, 0.0f);
        espAttrSet(handle, 4);
        for (digit = 0; digit < 3; digit++) {
            if (digit == 0) {
                handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 0)), 120, 10);
                lbl_1_bss_FAC[player].scoreSpriteId[digit] = handle;
            } else {
                handle = espEntry(fn_1_C45C(DATANUM(DATA_result, 0)), 120, 0);
                lbl_1_bss_FAC[player].scoreSpriteId[digit] = handle;
            }
            espPosSet(handle, lbl_1_bss_1074[player] +
                (lbl_1_data_5D0[positionIndex] + (f32)(digit * 18)),
                lbl_1_data_5D0[positionIndex + 1] + lbl_1_data_79C[lbl_1_bss_1094[player]]);
            espDrawNoSet(handle, 64);
            espScaleSet(handle, 0.0f, 0.0f);
            espAttrSet(handle, 4);
        }
        character = GwPlayerConf[lbl_1_bss_1084[player]].charNo;
        characterIndex[0] = character * 4;
        handle = CharModelCreate(character, 2);
        lbl_1_bss_FAC[player].characterModelId = handle;
        lbl_1_bss_FAC[player].idleMotionId = CharMotionCreate(character, RESULT_MOTION_IDLE);
        lbl_1_bss_FAC[player].motionId[0] =
            CharMotionCreate(character, RESULT_MOTION_AWARD_REACTION);
        lbl_1_bss_FAC[player].motionId[1] =
            CharMotionCreate(character, RESULT_MOTION_NO_AWARD_REACTION);
        lbl_1_bss_FAC[player].motionId[2] = CharMotionCreate(character, RESULT_MOTION_ENTRY);
        lbl_1_bss_FAC[player].arrivalMotionId = CharMotionCreate(character, RESULT_MOTION_ARRIVAL);
        CharMotionDataClose(character);
        Hu3DMotionShiftSet(handle, lbl_1_bss_FAC[player].motionId[2], 0.0f, 1.0f, 0);
        Hu3DModelCameraSet(handle, 1);
        screenPosition = lbl_1_data_5F0[player];
        screenPosition.x += lbl_1_bss_1074[player];
        Hu3D2Dto3D(&screenPosition, 1, &worldPosition);
        Hu3DModelPosSetV(handle, &worldPosition);
        Hu3DModelScaleSetV(handle, &lbl_1_data_680[lbl_1_bss_1094[player]]);
        Hu3DModelLayerSet(handle, 3);
        handle = Hu3DModelCreate(
            HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 23)), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_FAC[player].modelId = handle;
        Hu3DModelCameraSet(handle, 1);
        screenPosition = lbl_1_data_650[player];
        screenPosition.x += lbl_1_bss_1074[player];
        Hu3D2Dto3D(&screenPosition, 1, &worldPosition);
        Hu3DModelPosSetV(handle, &worldPosition);
        Hu3DModelScaleSet(handle, 0.0f, 0.0f, 0.0f);
        Hu3DModelLayerSet(handle, 1);
    }
    for (player = 0; player < 128; player++) {
        handle = Hu3DModelCreate(
            HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 26)), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_DAC[player] = handle;
        Hu3DModelScaleSet(handle, 0.5f, 0.5f, 0.5f);
        Hu3DModelAttrSet(handle, HU3D_ATTR_DISPOFF);
        Hu3DModelCameraSet(handle, 1);
        Hu3DModelLayerSet(handle, 3);
    }
    lbl_1_bss_BA4 = HuSprAnimRead(
        HuDataSelHeapReadNum(fn_1_C45C(DATANUM(DATA_result, 21)), HU_MEMNUM_OVL, HEAP_MODEL));
    HuSprExecLayerCameraSet(64, 1, 2);
}

/* Removes battle display sprites, models and particle groups after the outgoing wipe, before
 * returning to the board. */
void fn_1_86B0(void)
{
    s32 index;
    s32 subIndex;
    s32 character;

    espKill(lbl_1_bss_1070);
    Hu3DModelKill((s16)lbl_1_bss_106C);

    for (index = 0; index < GW_PLAYER_MAX; index++) {
        espKill(lbl_1_bss_FAC[index].spriteId[0]);
        espKill(lbl_1_bss_FAC[index].spriteId[1]);

        for (subIndex = 0; subIndex < 3; subIndex++) {
            espKill(lbl_1_bss_FAC[index].scoreSpriteId[subIndex]);
        }

        character = GwPlayerConf[lbl_1_bss_1084[index]].charNo;
        Hu3DModelKill((s16)lbl_1_bss_FAC[index].modelId);
    }

    for (index = 0; index < 128; index++) {
        Hu3DModelKill((s16)lbl_1_bss_DAC[index]);
    }

    for (index = 0; index < 64; index++) {
        if (lbl_1_bss_10A4[index].modelId > 0) {
            fn_1_B39C(lbl_1_bss_10A4[index].modelId);
            lbl_1_bss_10A4[index].modelId = 0;
        }
    }

    CharModelKill(-1);
}

/* Moves each character into its result row, then plays its arrival motion. */
void fn_1_8858(void)
{
    HuVecF positions[4];
    HuVecF scales[4];
    s32 delays[4] = { 0, 30, 60, 90 };
    s32 arrived[4];
    f32 scaleSteps[4];
    HuVecF worldPosition;
    HuVecF finalPosition;
    f32 positionStep;
    s32 player;
    s32 arrivalCount;

    for (player = 0; player < 4; player++) {
        positions[player] = lbl_1_data_5F0[player];
        positions[player].x += lbl_1_bss_1074[player];
        arrived[player] = 0;
        scales[player] = lbl_1_data_6B0[lbl_1_bss_1094[player]];
        scales[player].x = 0.0f;
        scaleSteps[player] = lbl_1_data_6B0[lbl_1_bss_1094[player]].x / 27.5f;
    }
    positionStep = (lbl_1_data_620[0].y - lbl_1_data_5F0[0].y) / 27.5f;
    for (;;) {
        for (player = arrivalCount = 0; player < 4; player++) {
            if (arrived[player] != 0 &&
                Hu3DMotionEndCheck((s16)lbl_1_bss_FAC[player].characterModelId) != 0) {
                Hu3DMotionShiftSet((s16)lbl_1_bss_FAC[player].characterModelId,
                    (s16)lbl_1_bss_FAC[player].idleMotionId, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            }
            arrivalCount += arrived[player];
            if (delays[player]-- <= 0 && arrived[player] == 0) {
                positions[player].y += positionStep;
                Hu3D2Dto3D(&positions[player], 1, &worldPosition);
                Hu3DModelPosSetV((s16)lbl_1_bss_FAC[player].characterModelId, &worldPosition);
                scales[player].x += scaleSteps[player];
                if (scales[player].x >= lbl_1_data_6B0[lbl_1_bss_1094[player]].x) {
                    scales[player].x = lbl_1_data_6B0[lbl_1_bss_1094[player]].x;
                }
                espScaleSet(lbl_1_bss_FAC[player].spriteId[0], scales[player].x,
                    scales[player].x);
                Hu3DModelScaleSetV((s16)lbl_1_bss_FAC[player].modelId, &scales[player]);
                if (positions[player].y >= lbl_1_data_620[player].y) {
                    finalPosition = lbl_1_data_620[player];
                    finalPosition.x += lbl_1_bss_1074[player];
                    Hu3D2Dto3D(&finalPosition, 1, &worldPosition);
                    Hu3DModelPosSetV((s16)lbl_1_bss_FAC[player].characterModelId, &worldPosition);
                    Hu3DMotionShiftSet((s16)lbl_1_bss_FAC[player].characterModelId,
                        (s16)lbl_1_bss_FAC[player].arrivalMotionId, 0.0f, 8.0f, 0);
                    arrived[player] = 1;
                }
            }
        }
        if (arrivalCount >= 4) {
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(20);
}

/* Drops each ranked player's battle coins and waits for the coins to land. */
void fn_1_8D4C(void)
{
    HuVecF positions[128];
    HuVecF rotations[128];
    s32 ranks[128];
    f32 speeds[128];
    s16 coinAwards[5];
    s32 emitted[4];
    s32 delays[4];
    HuVecF worldPosition;
    s32 index;
    s32 model;
    s32 remaining;
    f32 speedScale;

    remaining = 0;
    fn_1_3FB0(coinAwards);
    lbl_1_bss_BA8 = coinAwards[4];
    for (index = 0; index < 4; index++) {
        delays[index] = 0;
        emitted[index] = 0;
        remaining += coinAwards[index];
    }
    for (index = 0; index < 128; index++) {
        speeds[index] = 0.0f;
        ranks[index] = -1;
        Hu3DModelAttrSet((s16)lbl_1_bss_DAC[index], 1);
    }
    for (;;) {
        speedScale = lbl_1_bss_624 != 0 ? 1.5 : 1.0;
        for (index = 0; index < 4; index++) {
            if (--delays[index] <= 0 && emitted[index] < coinAwards[index]) {
                for (model = 0; model < 128; model++) {
                    if ((Hu3DModelAttrGet((s16)lbl_1_bss_DAC[model]) & 1) != 0) {
                        Hu3DModelAttrReset((s16)lbl_1_bss_DAC[model], 1);
                        positions[model] = lbl_1_data_5F0[index];
                        positions[model].x += lbl_1_bss_1074[index];
                        /* The starting rotation is read into the rank's slot rather than the
                         * emitted model's. Higher pooled model slots are later updated and applied
                         * without initialization. */
                        Hu3DModelRotGet((s16)lbl_1_bss_DAC[index], &rotations[index]);
                        Hu3D2Dto3D(&positions[model], 1, &worldPosition);
                        Hu3DModelPosSetV((s16)lbl_1_bss_DAC[model], &worldPosition);
                        speeds[model] = 10.0f;
                        delays[index] = 6;
                        emitted[index]++;
                        ranks[model] = index;
                        break;
                    }
                }
            }
        }
        for (index = 0; index < 128; index++) {
            if ((Hu3DModelAttrGet((s16)lbl_1_bss_DAC[index]) & 1) == 0) {
                positions[index].y += speedScale * speeds[index];
                speeds[index] += 0.002f * speedScale;
                positions[index].z = 9000.0f;
                rotations[index].y += 5.0f;
                Hu3D2Dto3D(&positions[index], 1, &worldPosition);
                Hu3DModelPosSetV((s16)lbl_1_bss_DAC[index], &worldPosition);
                Hu3DModelRotSetV((s16)lbl_1_bss_DAC[index], &rotations[index]);
                if (positions[index].y >= lbl_1_data_6F0[lbl_1_bss_1094[ranks[index]]]) {
                    HuAudFXPlay(MSM_SE_CMN_08);
                    fn_1_A238(&worldPosition);
                    Hu3DModelAttrSet((s16)lbl_1_bss_DAC[index], 1);
                    remaining--;
                }
            }
        }
        if (remaining <= 0) {
            break;
        }
        HuPrcVSleep();
    }
    for (index = 0; index < 128; index++) {
        Hu3DModelAttrSet((s16)lbl_1_bss_DAC[index], 1);
    }
    HuPrcSleep(20);
}

/* Drops the remaining battle coins at the selected player's result row. */
void fn_1_924C(void)
{
    HuVecF positions[32];
    HuVecF rotations[32];
    s32 delays[32];
    f32 speeds[32];
    HuVecF worldPosition;
    s32 model;
    s32 remaining;
    f32 speedScale;

    remaining = lbl_1_bss_BA8;
    for (model = 0; model < lbl_1_bss_BA8; model++) {
        delays[model] = model * 8;
        Hu3DModelAttrReset((s16)lbl_1_bss_DAC[model], 1);
        positions[model] = lbl_1_data_5F0[lbl_1_bss_61C];
        positions[model].x += lbl_1_bss_1074[lbl_1_bss_61C];
        Hu3D2Dto3D(&positions[model], 1, &worldPosition);
        Hu3DModelPosSetV((s16)lbl_1_bss_DAC[model], &worldPosition);
        speeds[model] = 10.0f;
    }

    for (;;) {
        speedScale = lbl_1_bss_624 != 0 ? 1.5 : 1.0;
        for (model = 0; model < lbl_1_bss_BA8; model++) {
            if (delays[model]-- <= 0 &&
                (Hu3DModelAttrGet((s16)lbl_1_bss_DAC[model]) & 1) == 0) {
                positions[model].y += speedScale * speeds[model];
                speeds[model] += 0.002f * speedScale;
                positions[model].z = 9000.0f;
                /* This path does not initialize the rotation vectors before updating and applying
                 * them. */
                rotations[model].y += 5.0f;
                Hu3D2Dto3D(&positions[model], 1, &worldPosition);
                Hu3DModelPosSetV((s16)lbl_1_bss_DAC[model], &worldPosition);
                Hu3DModelRotSetV((s16)lbl_1_bss_DAC[model], &rotations[model]);
                if (positions[model].y >= lbl_1_data_6F0[lbl_1_bss_1094[lbl_1_bss_61C]]) {
                    HuAudFXPlay(MSM_SE_CMN_08);
                    fn_1_A238(&worldPosition);
                    Hu3DModelAttrSet((s16)lbl_1_bss_DAC[model], 1);
                    remaining--;
                }
            }
        }
        if (remaining <= 0) {
            break;
        }
        HuPrcVSleep();
    }
    HuPrcSleep(20);
}

/* Reveals coin awards, plays each player's reaction, then hides the score resultHandles. */
void fn_1_95D8(void)
{
    s16 awards[5];
    s16 player;
    s16 digit;
    s16 frame;
    s16 duration;
    f32 scale;

    fn_1_3FB0(awards);
    for (player = 0; player < 4; player++) {
        if (player == lbl_1_bss_61C) {
            awards[player] += awards[4];
        }
    }
    for (player = 0; player < 4; player++) {
        for (digit = 0; digit < 3; digit++) {
            espAttrReset(lbl_1_bss_FAC[player].scoreSpriteId[digit], 4);
        }
        fn_1_9B90(player, awards[player]);
        espAttrReset(lbl_1_bss_FAC[player].spriteId[1], 4);
    }
    duration = lbl_1_bss_624 != 0 ? 3 : 10;
    for (frame = 0; frame <= duration; frame++) {
        scale = sin(M_PI * ((90.0 / duration) * frame) / 180.0);
        for (player = 0; player < 4; player++) {
            espScaleSet(lbl_1_bss_FAC[player].spriteId[1], scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_FAC[player].scoreSpriteId[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (player = 0; player < 4; player++) {
        espScaleSet(lbl_1_bss_FAC[player].spriteId[1], 1.0, 1.0);
        if (awards[player] != 0) {
            Hu3DMotionShiftSet((s16)lbl_1_bss_FAC[player].characterModelId,
                (s16)lbl_1_bss_FAC[player].motionId[0], 0.0f, 8.0f, 0);
        } else {
            Hu3DMotionShiftSet((s16)lbl_1_bss_FAC[player].characterModelId,
                (s16)lbl_1_bss_FAC[player].motionId[1], 0.0f, 8.0f, 0);
        }
    }
    HuPrcSleep(100);
    for (player = 0; player < 4; player++) {
        Hu3DMotionShiftSet((s16)lbl_1_bss_FAC[player].characterModelId,
            (s16)lbl_1_bss_FAC[player].idleMotionId, 0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    }
    for (frame = 0; frame <= duration; frame++) {
        scale = cos(M_PI * ((90.0 / duration) * frame) / 180.0);
        for (player = 0; player < 4; player++) {
            espScaleSet(lbl_1_bss_FAC[player].spriteId[1], scale, scale);
            for (digit = 0; digit < 3; digit++) {
                espScaleSet(lbl_1_bss_FAC[player].scoreSpriteId[digit], scale, scale);
            }
        }
        HuPrcVSleep();
    }
    for (player = 0; player < 4; player++) {
        espAttrSet(lbl_1_bss_FAC[player].spriteId[1], 4);
        for (digit = 0; digit < 3; digit++) {
            espAttrSet(lbl_1_bss_FAC[player].scoreSpriteId[digit], 4);
        }
    }
}

/* Updates the three score digits shown for one player in the results display. */
void fn_1_9B90(s32 player, s32 value)
{
    ResultSceneHandles resultHandles = lbl_1_bss_FAC[player];
    s32 digit[3];
    s32 remainder;

    if (value > 999) {
        value = 999;
    }

    remainder = value;
    digit[0] = value / 100;
    if (digit[0] == 0) {
        espBankSet(resultHandles.scoreSpriteId[0], 10);
    } else {
        espAttrReset(resultHandles.scoreSpriteId[0], 4);
        espBankSet(resultHandles.scoreSpriteId[0], digit[0]);
    }

    remainder = value - digit[0] * 100;
    digit[1] = remainder / 10;
    if ((digit[1] == 0) && (digit[0] == 0)) {
        espAttrSet(resultHandles.scoreSpriteId[1], 4);
    } else {
        espAttrReset(resultHandles.scoreSpriteId[1], 4);
        espBankSet(resultHandles.scoreSpriteId[1], digit[1]);
    }

    remainder -= digit[1] * 10;
    espBankSet(resultHandles.scoreSpriteId[2], remainder);
}

/* Sorts players by battle coin bonus and assigns offsets for tied ranks. */
void fn_1_9D18(void)
{
    s16 scores[4];
    s16 groups[4];
    s16 player;
    s16 pattern;
    s16 opponent;
    s16 swap;
    s16 pass;
    s16 rank;
    s16 group;
    s16 bonus;

    for (player = 0; player < 4; player++) {
        bonus = GwPlayer[player].mgCoinBonus;
        scores[player] = bonus;
        lbl_1_bss_1084[player] = player;
    }
    for (player = 0; player < 4; player++) {
        rank = 0;
        for (opponent = rank; opponent < 4; opponent++) {
            if (player != opponent && scores[player] > scores[opponent]) {
                rank++;
            }
        }
        lbl_1_bss_1094[player] = rank;
    }
    for (pass = 1; pass < 4; pass++) {
        for (player = 0; player < 4 - pass; player++) {
            if (scores[player] > scores[player + 1]) {
                swap = scores[player];
                scores[player] = scores[player + 1];
                scores[player + 1] = swap;
                swap = lbl_1_bss_1094[player];
                lbl_1_bss_1094[player] = lbl_1_bss_1094[player + 1];
                lbl_1_bss_1094[player + 1] = swap;
                swap = lbl_1_bss_1084[player];
                lbl_1_bss_1084[player] = lbl_1_bss_1084[player + 1];
                lbl_1_bss_1084[player + 1] = swap;
            }
        }
    }
    groups[0] = 0;
    group = 0;
    for (player = 1; player < 4; player++) {
        if (scores[player] != scores[player - 1]) {
            group++;
        }
        groups[player] = group;
    }
    if (groups[0] == 0 && groups[1] == 1 && groups[2] == 2 && groups[3] == 3) {
        pattern = 0;
    } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 1 && groups[3] == 2) {
        pattern = 1;
    } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 1 && groups[3] == 1) {
        pattern = 2;
    } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 1) {
        pattern = 3;
    } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 1 && groups[3] == 2) {
        pattern = 4;
    } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 1 && groups[3] == 1) {
        pattern = 5;
    } else if (groups[0] == 0 && groups[1] == 1 && groups[2] == 2 && groups[3] == 2) {
        pattern = 6;
    } else if (groups[0] == 0 && groups[1] == 0 && groups[2] == 0 && groups[3] == 0) {
        pattern = 7;
    }
    for (player = 0; player < 4; player++) {
        lbl_1_bss_1074[player] = lbl_1_data_7AC[pattern][player];
    }
}

/* Emits coin landing particles, reusing nearby particle groups when possible. */
void fn_1_A238(Vec *position)
{
    Vec offset;
    s32 effectState;
    ResultParticleGroup *group;
    s32 index;
    ResultParticleSystem *system;
    ResultParticle *particle;
    s32 groupIndex;
    s32 colorIndex;
    s32 remaining;

    groupIndex = -1;
    group = lbl_1_bss_10A4;
    for (index = 0; index < 64; index++, group++) {
        if (group->modelId != 0) {
            if (group->activeCount <= 40 && fabsf2(position->x - group->pos.x) < 300.0f &&
                fabsf2(position->y - group->pos.y) < 300.0f &&
                fabsf2(position->z - group->pos.z) < 300.0f) {
                groupIndex = index;
                break;
            }
        } else if (groupIndex < 0) {
            groupIndex = index;
        }
    }
    if (groupIndex >= 0) {
        group = &lbl_1_bss_10A4[groupIndex];
        if (group->modelId <= 0) {
            group->modelId = fn_1_AD24(lbl_1_bss_BA4, 60);
            fn_1_C420(group->modelId, fn_1_A928);
            Hu3DModelLayerSet(group->modelId, 3);
            group->pos.x = position->x;
            group->pos.y = position->y;
            group->pos.z = position->z;
            Hu3DModelPosSetV(group->modelId, position);
            group->activeCount = 0;
            system = Hu3DData[group->modelId].hookData;
            particle = system->particles;
            for (index = 0; index < system->particleCount; index++, particle++) {
                particle->scale = 0.0f;
                particle->color[3] = 0;
                particle->time = 0;
            }
            system->blendMode = 1;
            system->group = groupIndex;
        }
        effectState = 0;
        system = Hu3DData[group->modelId].hookData;
        PSVECSubtract(position, &group->pos, &offset);
        remaining = 20;
        group->activeCount += remaining;
        particle = system->particles;
        for (index = 0; index < system->particleCount && remaining != 0; index++, particle++) {
            if (particle->time == 0) {
                f32 angle;
                f32 elevation;

                colorIndex = lbl_1_data_83C[frandmod(16)];
                particle->color[0] = lbl_1_data_82C[colorIndex].r + frandmod(20);
                particle->color[1] = lbl_1_data_82C[colorIndex].g + frandmod(20);
                particle->color[2] = lbl_1_data_82C[colorIndex].b + frandmod(20);
                particle->color[3] = lbl_1_data_82C[colorIndex].a + frandmod(20);
                particle->animBank = lbl_1_data_84C[frandmod(32)];
                particle->pos.x = particle->pos.y = particle->pos.z = 0.0f;
                angle = 360.0f * frandf();
                elevation = 1.7f * frandf() - 0.7f;
                elevation = 90.0f * (elevation * fabsf2(elevation));
                particle->velocity.x = sin((M_PI * angle) / 180.0) *
                    cos((M_PI * elevation) / 180.0);
                particle->velocity.y = sin((M_PI * elevation) / 180.0);
                particle->velocity.z = cos((M_PI * angle) / 180.0) *
                    cos((M_PI * elevation) / 180.0);
                PSVECScale(&particle->velocity, &particle->pos, 100.0f * (0.5f * frandf()));
                PSVECScale(&particle->velocity, &particle->velocity,
                    (1.0f / 60.0f) * (300.0f + 100.0f * (4.0f * frandf())));
                PSVECAdd(&particle->pos, &offset, &particle->pos);
                particle->driftSpeed = (1.0f / 60.0f) *
                    (100.0f * (0.7f * (0.2f + 0.3f * frandf())));
                particle->driftAngle = 90.0f + angle;
                particle->scale = 70.0f * (0.5f + 0.5f * frandf());
                particle->scaleSpeed = -1.3f;
                particle->zRot = 360.0f * frandf();
                particle->spinSpeed = 20.0f * frandf() - 10.0f;
                particle->time = frandmod(10) + 20;
                particle->verticalAccel = 0.44444448f;
                particle->speedDecay = 0.92f;
                remaining--;
            }
        }
    }
}

/* Updates the coin landing particles while the particle model is drawn. */
void fn_1_A928(void *model, ResultParticleSystem *system, Mtx matrix)
{
    s32 index;
    s16 activeCount;
    ResultParticle *particle;

    activeCount = 0;
    particle = system->particles;
    for (index = 0; index < system->particleCount; index++, particle++) {
        if (particle->time != 0) {
            activeCount++;
            PSVECScale(&particle->velocity, &particle->velocity, particle->speedDecay);
            PSVECAdd(&particle->pos, &particle->velocity, &particle->pos);
            particle->velocity.y += particle->verticalAccel;
            particle->velocity.x += particle->driftSpeed *
                sin((3.141592653589793 * particle->driftAngle) / 180.0);
            particle->velocity.z += particle->driftSpeed *
                cos((3.141592653589793 * particle->driftAngle) / 180.0);
            particle->driftAngle += 5.0f;
            particle->driftSpeed += 0.02777778f;
            particle->scale += particle->scaleSpeed;
            particle->zRot += particle->spinSpeed;
            particle->time--;
            if (particle->time < 10) {
                particle->color[3] *= 0.8f;
                if (particle->time == 0) {
                    particle->color[3] = 0;
                    particle->scale = 0.0f;
                    activeCount--;
                }
            }
        }
    }
    lbl_1_bss_10A4[system->group].activeCount = (s16)activeCount;
}

f32 lbl_1_data_590[8] = {
    90.2000046f, 148.0f, 220.400009f, 148.0f, 355.600006f, 148.0f, 485.800018f, 148.0f
};
f32 lbl_1_data_5B0[8] = {
    90.2000046f, 238.0f, 219.400009f, 238.0f, 354.600006f, 238.0f, 483.800018f, 238.0f
};
f32 lbl_1_data_5D0[8] = {
    70.2000046f, 204.0f, 200.400009f, 204.0f, 335.600006f, 204.0f, 465.800018f, 204.0f
};
HuVecF lbl_1_data_5F0[4] = { { 90.2000046f, -12.0f, 9000.0f },
                             { 220.400009f, -12.0f, 9000.0f },
                             { 355.600006f, -12.0f, 9000.0f },
                             { 485.800018f, -12.0f, 9000.0f } };
HuVecF lbl_1_data_620[4] = { { 90.2000046f, 368.0f, 9000.0f },
                             { 220.400009f, 368.0f, 9000.0f },
                             { 355.600006f, 368.0f, 9000.0f },
                             { 485.800018f, 368.0f, 9000.0f } };
HuVecF lbl_1_data_650[4] = { { 90.2000046f, 388.0f, 4500.0f },
                             { 220.400009f, 388.0f, 4500.0f },
                             { 355.600006f, 388.0f, 4500.0f },
                             { 485.800018f, 388.0f, 4500.0f } };
HuVecF lbl_1_data_680[4] = { { 1.0f, 1.0f, 1.0f },
                             { 0.75f, 0.75f, 0.75f },
                             { 0.5f, 0.5f, 0.5f },
                             { 0.300000012f, 0.300000012f, 0.300000012f } };
HuVecF lbl_1_data_6B0[4] = { { 1.0f, 1.10000002f, 1.0f },
                             { 0.899999976f, 1.0f, 0.899999976f },
                             { 0.800000012f, 0.899999976f, 0.800000012f },
                             { 0.699999988f, 0.800000012f, 0.699999988f } };
f32 lbl_1_data_6E0[4] = {1.0f, 0.899999976f, 0.800000012f, 0.699999988f};
f32 lbl_1_data_6F0[4] = {288.0f, 298.0f, 308.0f, 318.0f};
s32 lbl_1_data_700[8][4] = { { 0, 1, 2, 3 }, { 0, 0, 2, 3 }, { 0, 0, 2, 2 }, { 0, 0, 0, 3 },
                             { 0, 1, 1, 3 }, { 0, 1, 1, 1 }, { 0, 1, 2, 2 }, { 0, 0, 0, 0 } };
HuVecF lbl_1_data_780 = {288.0f, 470.0f, 14000.0f};
f32 lbl_1_data_78C[4] = {0.0f, 25.0f, 45.0f, 66.0f};
f32 lbl_1_data_79C[4] = {0.0f, 24.0f, 44.0f, 66.0f};
f32 lbl_1_data_7AC[8][4] = { { 12.0f, 22.0f, 19.0f, 5.0f }, { 10.0f, 24.0f, 24.0f, 8.0f },
                             { 10.0f, 21.0f, 17.0f, 4.0f }, { 5.0f, 14.0f, 21.0f, 12.0f },
                             { 8.0f, 17.0f, 18.0f, 9.0f },  { 4.0f, 8.0f, 4.0f, 2.0f },
                             { 10.0f, 18.0f, 12.0f, 2.0f }, { 0.0f, 0.0f, 0.0f, 0.0f } };
GXColor lbl_1_data_82C[4] = {
    { 220, 220, 64, 120 }, { 220, 220, 220, 120 }, { 64, 160, 220, 140 }, { 220, 140, 140, 140 }
};
u8 lbl_1_data_83C[16] = {0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 2, 3, 2, 3, 2, 3};
u8 lbl_1_data_84C[32] = {
    2, 2, 2, 2, 2, 3, 3, 3, 2, 2, 2, 2, 2, 3, 3, 3,
    2, 2, 2, 2, 2, 3, 3, 3, 2, 3, 2, 3, 2, 3, 0, 1
};

ResultParticleGroup lbl_1_bss_10A4[64];
s32 lbl_1_bss_1094[4];
s32 lbl_1_bss_1084[4];
f32 lbl_1_bss_1074[4];
s32 lbl_1_bss_1070;
s32 lbl_1_bss_106C;
ResultSceneHandles lbl_1_bss_FAC[4];
s32 lbl_1_bss_DAC[128];
s32 lbl_1_bss_BAC[128];
s32 lbl_1_bss_BA8;
ANIMDATA *lbl_1_bss_BA4;
s32 lbl_1_bss_BA0;
