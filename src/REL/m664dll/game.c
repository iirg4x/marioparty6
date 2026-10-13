/* Wrasslin' Rapids sequence setup, play, camera work, and presentation. */
#include "dolphin/math.h"

#include "REL/m664dll/m664dll.h"

#define MSM_SE_M664_OPENING_CUE 2136
#define MSM_SE_M664_PLAYER_ACTION 2126
#define MSM_SE_M664_OPENING_TRANSITION 2127
#define MSM_SE_M664_SCENE_TRANSITION 2135
#define MSM_SE_M664_COURSE_REVEAL 2129
#define MSM_SE_M664_LATE_SCENE_CUE 2137
#define MSM_SE_M664_LATE_SCENE_TRANSITION 2133
#define MSM_SE_M664_PROP_RELEASE 2131
#define MSM_SE_M664_PROP_LANDING 2132
#define MSM_SE_M664_NUT_DROP 2138
#define MSM_SE_M664_HOOK_IMPACT 2139
#define MSM_SE_M664_PROP_MISS 2140
#define MSM_SE_M664_NUT_IMPACT 2130
#define MSM_SE_M664_SCENE_SOUND 2134
#define MSM_SE_M664_SCENE_EFFECT 2141
#define MSM_SE_M664_PLAYER_STUN 2142
#define M664_SOUND_PAN_MIN 32
#define M664_SOUND_PAN_MAX 96
/* Byte offsets of the saved stick and button slots in player input settings. */
#define M664_INPUT_STICK_X_OFFSET 24
#define M664_INPUT_STICK_Y_OFFSET 28
#define M664_INPUT_BUTTONS_DOWN_OFFSET 32
#define M664_INPUT_BUTTONS_HELD_OFFSET 36
#define M664_SCENE_FRAME_250 250
#define M664_SCENE_FRAME_1000 1000
#define M664_SCENE_FRAME_1350 1350
#define M664_SCENE_FRAME_1550 1550
#define M664_SCENE_FRAME_1700 1700
#define M664_SCENE_FRAME_2000 2000
#define M664_SCENE_FRAME_2500 2500
#define M664_SCENE_FRAME_3200 3200
#define M664_SCENE_FRAME_3300 3300
#define M664_SCENE_FRAME_3730 3730
#define M664_SCENE_FRAME_4000 4000
#define M664_SCENE_FRAME_3950 3950
#define M664_SCENE_FRAME_2828 2828
#define M664_SCENE_FRAME_2853 2853
#define M664_SCENE_FRAME_2978 2978
#define M664_SCENE_FRAME_3003 3003
#define M664_SCENE_FRAME_3648 3648
#define M664_SCENE_FRAME_3653 3653
#define M664_SCENE_FRAME_3673 3673
#define M664_SCENE_FRAME_3678 3678
#define M664_SCENE_FRAME_4378 4378
#define M664_SCENE_FRAME_4403 4403
#define M664_SCENE_FRAME_4508 4508
#define M664_SCENE_FRAME_4533 4533
#define M664_SCENE_FRAME_2850 2850
#define M664_SCENE_FRAME_3000 3000
#define M664_SCENE_FRAME_3670 3670
#define M664_SCENE_FRAME_3675 3675
#define M664_SCENE_FRAME_4400 4400
#define M664_SCENE_FRAME_4530 4530
u32 lbl_1_data_28[7] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 21),
    DATANUM(DATA_mario, 184), DATANUM(DATA_mariomot, 34),
    DATANUM(DATA_mario, 120), DATANUM(DATA_mariomot, 38),
    DATANUM(DATA_mariomot, 40)
};
const u8 lbl_1_rodata_60[2][3] = { {5, 4, 5}, {5, 4, 5} };
const u8 lbl_1_rodata_66[2][3] = { {255, 255, 255}, {255, 255, 255} };
const Point3d lbl_1_rodata_6C[2] = {
    { 0.8500000238418579f, 0.800000011920929f, 0.8999999761581421f },
    { 0.8500000238418579f, 0.800000011920929f, 0.8999999761581421f }
};

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/audio.h"

#include "game/charman.h"

#include "game/gamemes.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/gamework.h"

#include "game/memory.h"

#include "game/mg/seqman.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "msm_stream.h"

s16 lbl_1_bss_94C;

Point3d lbl_1_bss_940;

Point3d lbl_1_bss_934;

GXColor lbl_1_bss_930;

s32 lbl_1_bss_924[3];

s16 lbl_1_bss_920;

Point3d lbl_1_bss_914;

Point3d lbl_1_bss_908;

GXColor lbl_1_bss_904;

s32 lbl_1_bss_8F8[3];

f32 lbl_1_bss_8F4;

Point3d lbl_1_bss_8E8;

s32 lbl_1_bss_8E4;

OM_CAMERA_VIEW lbl_1_bss_8C8;

s32 lbl_1_bss_8C4;

s32 lbl_1_bss_8C0;

s32 lbl_1_bss_8BC;

s32 lbl_1_bss_8AC[4];

s32 lbl_1_bss_8A8;

s16 lbl_1_bss_8A0[4];

s16 lbl_1_bss_89E;

s16 lbl_1_bss_89C;

s16 lbl_1_bss_894[4];

s16 lbl_1_bss_874[16];

s16 lbl_1_bss_870[2];

s16 lbl_1_bss_86E;

s16 lbl_1_bss_86C;

/* Each player raft and hook model IDs. */
M664ModelPair lbl_1_bss_85C[4];

s16 lbl_1_bss_85A;

s16 lbl_1_bss_858;

f32 lbl_1_bss_850[2];

s16 lbl_1_bss_84E;

s16 lbl_1_bss_844[5];

/* Model IDs for the four loose nut props on each of the five course hooks. */
s16 lbl_1_bss_81C[5][4];

/* Model IDs for the two nut reaction animations at each course hook. */
s16 lbl_1_bss_7CC[5][4][2];

s16 lbl_1_bss_7CA;

/* Hook object names selected for the five course hook assemblies. */
char lbl_1_bss_798[5][10];

f32 lbl_1_bss_794;

/* Random nut-drop delays for each hook; zero suppresses that nut's scripted drop. */
s32 lbl_1_bss_744[5][4];

/* Drop and landing state for each nut at the five course hooks. */
s32 lbl_1_bss_6F4[5][4];

/* Vertical offsets applied to each hook's nut positions during play. */
f32 lbl_1_bss_6A4[5][4];

s16 lbl_1_bss_69E[3];

/* Model IDs for seven attached course parts in each of three timed scene groups. */
s16 lbl_1_bss_674[3][7];

/* World positions of the seven attached course parts for each scene group. */
Point3d lbl_1_bss_578[3][7];

/* World rotations of the seven attached course parts for each scene group. */
Point3d lbl_1_bss_47C[3][7];

s16 lbl_1_bss_46C[8];

/* Joint-motion IDs attached to the two side course models. */
s16 lbl_1_bss_468[2];

s32 lbl_1_bss_464;

f32 lbl_1_bss_460;

s16 lbl_1_bss_458[4] ATTRIBUTE_ALIGN(8);

s16 lbl_1_bss_454[2];

M664MotionState lbl_1_bss_41C[1];

s16 lbl_1_bss_418;

Point3d lbl_1_bss_40C;

f32 lbl_1_bss_404[2];

s32 lbl_1_bss_3FC[2];

s32 lbl_1_bss_3F8;

/* Falling course props and their splash-effect models for both sides and three events. */
M664EffectModel lbl_1_bss_248[2][3];

M664PlayerRecord lbl_1_bss_C8[4];

M664PlayerSettings lbl_1_bss_18[4];

M664SoundHandles lbl_1_bss_10;

s32 lbl_1_bss_C;

s32 lbl_1_bss_8;

void fn_1_A08(void);

s32 fn_1_B74(void);

void fn_1_13DC(void);

void fn_1_14C4(void);

void fn_1_1584(void);

void fn_1_20B0(void);

void fn_1_3E74(void);

void fn_1_2848(void);

void fn_1_2188(void);

void fn_1_2E08(void);

void fn_1_4060(void);

void fn_1_A428();

void fn_1_640C(void);

void fn_1_640C(void);

#include "game/hu3d.h"

extern s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask);

void fn_1_640C(void);

void fn_1_2188(void);

void fn_1_2E08(void);

void fn_1_4060(void);

void fn_1_A428();

extern const Point3d lbl_1_rodata_90;

extern const Point3d lbl_1_rodata_9C;

extern const Point3d lbl_1_rodata_A8;

extern const Point3d lbl_1_rodata_B4;

void fn_1_2188(void);

void fn_1_2E08(void);

void fn_1_4060(void);

void fn_1_640C(void);

void fn_1_A8F0(void);

void fn_1_A428(void);

s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask);

/* Unused lineup helper: bobs the raft and places all four player models in a row. */
static inline void fn_1_14C4_inlined(void) {
    s32 playerIndex;

    fn_1_640C();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelPosSet(lbl_1_bss_C8[playerIndex].modelId,
                        (-300.0f) + (200.0f) * (f32)playerIndex,
                        (232.0f) + lbl_1_bss_41C[0].position.y,
                        0.0f);
    }
}

extern const Point3d lbl_1_rodata_E8;

extern const Point3d lbl_1_rodata_F4;

extern const Point3d lbl_1_rodata_100;

extern const Point3d lbl_1_rodata_10C;

void Hu3DFogClear(void);

extern const s32 lbl_1_rodata_128[3];

extern const s32 lbl_1_rodata_134[3];

#include "dolphin/types.h"

extern const HuVecF lbl_1_rodata_144;

extern const HuVecF lbl_1_rodata_150;

extern const HuVecF lbl_1_rodata_15C;

extern const HuVecF lbl_1_rodata_168;

extern const HuVecF lbl_1_rodata_174;

extern const HuVecF lbl_1_rodata_180;

extern const HuVecF lbl_1_rodata_18C;

extern const HuVecF lbl_1_rodata_198;

extern const HuVecF lbl_1_rodata_1A4;

extern const HuVecF lbl_1_rodata_1B0;

extern const HuVecF lbl_1_rodata_1BC;

extern const HuVecF lbl_1_rodata_1C8;

extern const HuVecF lbl_1_rodata_1D4;

extern const HuVecF lbl_1_rodata_1E0;

extern const HuVecF lbl_1_rodata_1EC;

extern const HuVecF lbl_1_rodata_1F8;

extern const HuVecF lbl_1_rodata_204;

/* Unused camera helper: sets the saved center, rotation and zoom, then starts a view transition. */
static inline void moveCameraView(
    const HuVecF *centerSource,
    const HuVecF *rotationSource,
    const f32 *zoomSource,
    s32 duration
) {
    HuVecF rotation;
    HuVecF center;
    HuVecF *rotationPointer;
    HuVecF *centerPointer;

    center = *centerSource;
    centerPointer = &center;
    lbl_1_bss_8C8.center = *centerPointer;
    rotation = *rotationSource;
    rotationPointer = &rotation;
    lbl_1_bss_8C8.rot = *rotationPointer;
    lbl_1_bss_8C8.zoom = *zoomSource;
    omCameraViewMove(&lbl_1_bss_8C8, duration, 3);
}

/* Unused camera helper: sets the saved center and rotation, then transitions with the saved
 * zoom. */
static inline void moveCameraViewWithoutZoom(
    const HuVecF *centerSource,
    const HuVecF *rotationSource,
    s32 duration
) {
    HuVecF rotation;
    HuVecF center;
    HuVecF *rotationPointer;
    HuVecF *centerPointer;

    center = *centerSource;
    centerPointer = &center;
    lbl_1_bss_8C8.center = *centerPointer;
    rotation = *rotationSource;
    rotationPointer = &rotation;
    lbl_1_bss_8C8.rot = *rotationPointer;
    omCameraViewMove(&lbl_1_bss_8C8, duration, 3);
}

/* Unused camera helper: transitions to a new saved center, keeping the saved rotation and zoom. */
static inline void moveCameraCenter(const HuVecF *centerSource, s32 duration) {
    HuVecF *centerPointer;
    HuVecF center;

    center = *centerSource;
    centerPointer = &center;
    lbl_1_bss_8C8.center = *centerPointer;
    omCameraViewMove(&lbl_1_bss_8C8, duration, 3);
}

/* Unused camera helper: transitions to a new saved rotation, keeping the saved center and zoom. */
static inline void moveCameraRotation(const HuVecF *rotationSource, s32 duration) {
    HuVecF *rotationPointer;
    HuVecF rotation;

    rotation = *rotationSource;
    rotationPointer = &rotation;
    lbl_1_bss_8C8.rot = *rotationPointer;
    omCameraViewMove(&lbl_1_bss_8C8, duration, 3);
}

#include "dolphin/mtx.h"

extern void fn_1_AC58(s32 playerIndex);

extern void fn_1_3868(s32 playerIndex);

extern s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask);

void MgPlayerDespawn(MGPLAYER *player);

extern s32 frandmod(s32 limit);

extern u32 MgSeqModeGet(void);

s32 fn_1_420C(s16 *collisionMap);

s32 fn_1_4848(s16 *collisionMap);

void fn_1_4DD8(void);

void fn_1_5770(void);

s32 fn_1_58E8(s16 *collisionMap);

void fn_1_6704(void);

void fn_1_7E24(void);

void fn_1_8E44(void);

extern const Point3d lbl_1_rodata_278;

void fn_1_476C(void);

void fn_1_4CC8(void);

void fn_1_5108(void);

void fn_1_5860(void);

void fn_1_6D7C(void);

void fn_1_8244(void);

void fn_1_9704(void);

void fn_1_5D3C(void);

extern s32 frandmod(s32 limit);

extern s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask);

void fn_1_1A70(void);

extern const Point3d lbl_1_rodata_2AC;

extern const Point3d lbl_1_rodata_2B8;

void fn_1_5D90(void);

void fn_1_640C(void);

extern Point3d lbl_1_rodata_2EC;

extern s16 fn_1_B910(s16, s16, s16);

extern s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask);

void fn_1_B8FC(void *inputSettings, s32 stickX, s32 stickY, s32 buttonsDown, s32 buttonsHeld);

#include "dolphin/mtx/GeoTypes.h"

#include "game/process.h"

/* Finish-message callback: switches players to scripted control, advances the course and fades
 * audio. */
void fn_1_2F8(s16 mode, s16 frameNo)
{
    fn_1_A08();
}

/* Called by the sequence manager during results; waits for the reveal to finish before
 * advancing. */
void fn_1_318(s16 mode, s16 frameNo)
{
    /* Sequence callback: hold this phase until the results transition is ready. */
    if (fn_1_B74() != 0) {
        MgSeqModeNext();
        return;
    }
    MgSeqModeChangeOff();
}

void fn_1_34C(s16 mode, s16 frameNo)
{
    /* Sequence callback: place the players and start the results jingle. */
    fn_1_13DC();
}

void fn_1_36C(s16 mode, s16 frameNo)
{
    /* Sequence callback: keep the players in their results positions. */
    fn_1_14C4();
}

/* Sequence callback: stop this minigame's audio and models before returning. */
void fn_1_38C(s16 mode, s16 frameNo)
{
    HuAudAllStop();
    Hu3DModelAllKill();
    Hu3DLightAllKill();
    omOvlReturnEx(1, 1);
}

/* Called by setup.c when the opening scene begins; initializes its day or night scene and sequence
 * state. */
void fn_1_3C0(void)
{
    s16 isNightMode;
    s32 nightFlag;

    /* Initializes the day or night scene and clears the sequence's running state. */
    isNightMode = GwMgNightF;
    if (isNightMode == 0) {
        nightFlag = 0;
    } else {
        nightFlag = 1;
    }
    lbl_1_bss_8E4 = nightFlag;
    fn_1_1584();
    fn_1_20B0();
    fn_1_3E74();
    fn_1_2848();
    lbl_1_bss_8C4 = lbl_1_bss_8C0 = 0;
    lbl_1_bss_8BC = 4;
    lbl_1_bss_8AC[0] = lbl_1_bss_8AC[1] = lbl_1_bss_8AC[2] = lbl_1_bss_8AC[3] = 0;
    lbl_1_bss_10.soundHandle[0] = lbl_1_bss_10.soundHandle[1] = -1;
    lbl_1_bss_C = -1;
    HuDataDirClose(DATA_m664);
}

/* Called each active-play frame by setup.c; updates players and the course, then reports when play
 * ends. */
s32 fn_1_4C0(void) {
    s32 sequenceFinished;

    /* Update timed scene phases and actors before checking the final motion. */
    sequenceFinished = 0;
    fn_1_2E08();
    fn_1_4060();
    fn_1_2188();
    MgActorExec();
    fn_1_A428();
    if (Hu3DMotionEndCheck(lbl_1_bss_86E) == 0) {
        lbl_1_bss_8C0 += 1;
    }
    if (((s32) lbl_1_bss_8BC < 2) || ((s32) lbl_1_bss_8C0 > 4888)) {
        sequenceFinished = 1;
    }
    return sequenceFinished;
}

/* Called each opening frame by setup.c to animate the lineup and guide the players onto the
 * raft. */
void fn_1_560(void) {
    Point3d playerPosition;
    s32 playerIndex;
    s32 stickX;
    s32 stickY;
    s32 padStickX;
    s32 padStickY;

    lbl_1_bss_8C4 += 1;
    if (lbl_1_bss_8C4 > 10) {
        if (lbl_1_bss_8C4 == 11) {
            lbl_1_bss_10.soundHandle[0] = HuAudFXPlay(MSM_SE_M664_OPENING_CUE);
            HuAudFXPlay(MSM_SE_M664_PLAYER_ACTION);
        }
        if (lbl_1_bss_8C4 == 110) {
            HuAudFXPlay(MSM_SE_M664_OPENING_TRANSITION);
        }
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            Hu3DMotionSpeedSet(lbl_1_bss_85C[playerIndex].modelId, (1.0f));
            Hu3DModelObjPosGet(lbl_1_bss_85C[playerIndex].modelId, "fook00",
                               &playerPosition);
            Hu3DModelPosSetV(lbl_1_bss_85C[playerIndex].objectModelId, &playerPosition);
            if (lbl_1_bss_8C4 == 65) {
                Hu3DModelShadowMapSet(lbl_1_bss_85C[playerIndex].modelId);
            }
            MgPlayerAttrSet(lbl_1_bss_C8[playerIndex].player, 1);
            if (lbl_1_bss_8C4 >= 125) {
                if (lbl_1_bss_8C4 == 125) {
                    MgPlayerAttrReset(lbl_1_bss_C8[playerIndex].player, 1);
                    MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, 0, 0, PAD_BUTTON_A, 0);
                } else {
                    if ((lbl_1_bss_C8[playerIndex].player->actor->colGroundAttr & 0x407F) == 0) {
                        MgPlayerAttrReset(lbl_1_bss_C8[playerIndex].player, 1);
                        if (playerIndex % 2 != 0) {
                            stickX = -50;
                        } else {
                            stickX = 50;
                        }
                        padStickX = stickX;
                        if (playerIndex / 2 != 0) {
                            stickY = 50;
                        } else {
                            stickY = -50;
                        }
                        padStickY = stickY;
                        MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, padStickX, padStickY, 0,
                                       PAD_BUTTON_A);
                    }
                    if (lbl_1_bss_8AC[playerIndex] == 185) {
                        CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                           lbl_1_bss_C8[playerIndex].motionIds[4],
                                           (0.0f),
                                           (8.0f), 0);
                        Hu3DModelAttrSet(lbl_1_bss_85C[playerIndex].modelId, HU3D_MOTATTR_REV);
                        if (playerIndex == 0) {
                            HuAudFXPlay(MSM_SE_M664_PLAYER_ACTION);
                        }
                    }
                }
            }
            lbl_1_bss_8AC[playerIndex] += 1;
        }
    }
    if (lbl_1_bss_8C4 > 135) {
        lbl_1_bss_8C4 = 135;
    }
    lbl_1_bss_8A8 = 118;
    fn_1_640C();
    MgActorExec();
}

/* Called each Start-message frame by setup.c to settle the lineup and start the gameplay track. */
void fn_1_8DC(void) {
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_8A8 == 90) {
            CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                               lbl_1_bss_C8[playerIndex].motionIds[0],
                               (0.0f), (8.0f), 0);
        } else if (lbl_1_bss_8A8 == 0) {
            MgPlayerAttrReset(lbl_1_bss_C8[playerIndex].player, 1);
        }
    }
    if ((lbl_1_bss_8A8 -= 1) < 0) {
        lbl_1_bss_8A8 = 0;
    }
    fn_1_640C();
    MgActorExec();
    if (lbl_1_bss_C == -1) {
        lbl_1_bss_C = HuAudBGMPlay(MSM_STREAM_MGMUS_26);
    }
}

/* Called during the Finish message to disable AI input, advance the course and fade gameplay
 * audio. */
void fn_1_A08(void) {
    s32 playerIndex;

    /* Disable AI input and bypass pad-driven movement while the Finish scene continues. */
    playerIndex = 0;
    while (playerIndex < 4) {
        MgPlayerAttrSet(lbl_1_bss_C8[playerIndex].player, 1U);
        lbl_1_bss_18[playerIndex].type = 0;
        playerIndex += 1;
    }
    fn_1_2E08();
    fn_1_4060();
    fn_1_2188();
    MgActorExec();
    fn_1_A428();
    if (Hu3DMotionEndCheck(lbl_1_bss_86E) == 0) {
        lbl_1_bss_8C0 += 1;
    }
    lbl_1_bss_8C4 = 0;
    if ((s32) lbl_1_bss_C != -1) {
        HuAudSStreamFadeOut(lbl_1_bss_C, 100);
        lbl_1_bss_C = -1;
    }
    if (lbl_1_bss_10.soundHandle[1] > -1) {
        HuAudFXFadeOut(lbl_1_bss_10.soundHandle[1], 300);
        lbl_1_bss_10.soundHandle[1] = -2;
    }
    if (lbl_1_bss_10.soundHandle[0] != -1) {
        HuAudFXFadeOut(lbl_1_bss_10.soundHandle[0], 300);
        lbl_1_bss_10.soundHandle[0] = -1;
    }
}

/* Called by fn_1_318 during results to finish the wipe, prepare the lineup, and award coin
 * bonuses. */
s32 fn_1_B74(void) {
    s32 result = 0;
    s32 playerIndex;
    s16 motionId;
    s32 coinBonus;

    Point3d *scenePositionPointer;
    Point3d *sceneRotationPointer;
    Point3d *cameraCenterPointer;
    Point3d *cameraRotationPointer;

    if (lbl_1_bss_10.soundHandle[1] > -1) {
        HuAudFXFadeOut(lbl_1_bss_10.soundHandle[1], 300);
        lbl_1_bss_10.soundHandle[1] = -2;
    }
    if (lbl_1_bss_8C4 == 0) {
        WipeColorSet(255, 255, 255);
        WipeCreate(2, 0, 60);
        lbl_1_bss_8C4 = 1;
    }
    if (WipeCheck() == 0) {
        if (lbl_1_bss_8C4 == 1) {
            WipeColorSet(255, 255, 255);
            WipeCreate(1, 0, 60);
            lbl_1_bss_8C4 = 2;
            fn_1_A8F0();
            for (playerIndex = 0; playerIndex < 1; playerIndex++) {
            { Point3d scenePosition = {0.0f, 0.0f, 0.0f}; scenePositionPointer = &scenePosition; lbl_1_bss_41C[playerIndex].position = *scenePositionPointer; }
            Hu3DModelPosSetV(lbl_1_bss_41C[playerIndex].modelId[0],
                             &lbl_1_bss_41C[playerIndex].position);
            Hu3DModelPosSetV(lbl_1_bss_41C[playerIndex].modelId[1],
                             &lbl_1_bss_41C[playerIndex].position);
            { Point3d sceneRotation = {0.0f, 0.0f, 0.0f}; sceneRotationPointer = &sceneRotation; lbl_1_bss_41C[playerIndex].rotation = *sceneRotationPointer; }
            Hu3DModelRotSetV(lbl_1_bss_41C[playerIndex].modelId[0],
                             &lbl_1_bss_41C[playerIndex].rotation);
            Hu3DModelRotSetV(lbl_1_bss_41C[playerIndex].modelId[1],
                             &lbl_1_bss_41C[playerIndex].rotation);
            }
            Hu3DModelPosSet(lbl_1_bss_418, 0.0f, (5.0f), 0.0f);
            Hu3DModelRotSet(lbl_1_bss_418, 0.0f, 0.0f, 0.0f);
            Hu3DModelAttrReset(lbl_1_bss_858, 1);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                Hu3DModelShadowSet(lbl_1_bss_C8[playerIndex].modelId);
                Hu3DModelPosSet(lbl_1_bss_C8[playerIndex].modelId,
                                (-300.0f) + (200.0f) * playerIndex,
                                (232.0f) + lbl_1_bss_41C[0].position.y,
                                0.0f);
                Hu3DModelRotSet(lbl_1_bss_C8[playerIndex].modelId, 0.0f,
                                0.0f, 0.0f);
                CharMotionSet(lbl_1_bss_C8[playerIndex].characterNo,
                              lbl_1_bss_C8[playerIndex].motionIds[0]);
                Hu3DModelAttrReset(lbl_1_bss_C8[playerIndex].modelId, 1);
                Hu3DModelScaleSet(lbl_1_bss_C8[playerIndex].modelId, (1.0f),
                                  (1.0f), (1.0f));
            }
            { Point3d cameraCenter = {0.0f, 100.0f, -100.0f}; cameraCenterPointer = &cameraCenter; lbl_1_bss_8C8.center = *cameraCenterPointer; }
            { Point3d cameraRotation = {-30.0f, 0.0f, 0.0f}; cameraRotationPointer = &cameraRotation; lbl_1_bss_8C8.rot = *cameraRotationPointer; }
            lbl_1_bss_8C8.zoom = (2250.0f);
            omCameraViewSet(&lbl_1_bss_8C8);
            Hu3DGLightColorSet(lbl_1_bss_94C, 255, 255, 255, 255);
            if (lbl_1_bss_8E4 == 0) {
                Hu3DGLightColorSet(lbl_1_bss_920, 0, 0, 0, 255);
            } else {
                lbl_1_bss_914.y = (1000.0f);
                Hu3DGLightPosSetV(lbl_1_bss_920, &lbl_1_bss_914, &lbl_1_bss_908);
            }
            Hu3DAmbColorSet((1.0f), (1.0f), (1.0f));
            if (lbl_1_bss_8E4 == 0) {
                Hu3DFogClear();
            }
        } else if (lbl_1_bss_8C4 == 2) {
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                if (lbl_1_bss_C8[playerIndex].sequenceCount < 2) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[5],
                                       0.0f, (8.0f), 0);
                    coinBonus = 0;
                } else {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[6],
                                       0.0f, (8.0f), 0);
                    coinBonus = lbl_1_bss_C8[playerIndex].sequenceCount - 1;
                }
                if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                    GwPlayer[playerIndex].mgCoinBonus = coinBonus;
                }
            }
            result = 1;
            lbl_1_bss_8C8.zoom = (1500.0f);
            omCameraViewMove(&lbl_1_bss_8C8, 100, 0);
        }
    } else if (lbl_1_bss_8C4 == 1) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            MgPlayerAttrSet(lbl_1_bss_C8[playerIndex].player, 1);
        }
        fn_1_2E08();
        fn_1_4060();
        fn_1_2188();
        MgActorExec();
        fn_1_A428();
        if (Hu3DMotionEndCheck(lbl_1_bss_86E) == 0) {
            lbl_1_bss_8C0 += 1;
        }
    }
    if (lbl_1_bss_8C4 < 2) {
        return 0;
    }
    fn_1_640C();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelPosSet(lbl_1_bss_C8[playerIndex].modelId,
                        (-300.0f) + (200.0f) * (f32) playerIndex,
                        (232.0f) + lbl_1_bss_41C[0].position.y,
                        0.0f);
    }
    return result;
}

/* Called by the winner-phase callback to maintain the lineup and start the results jingle. */
void fn_1_13DC(void) {
    s32 playerIndex;

    fn_1_640C();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelPosSet(lbl_1_bss_C8[playerIndex].modelId,
                        (-300.0f) + (200.0f) * (f32) playerIndex,
                        (232.0f) + lbl_1_bss_41C[0].position.y, (0.0f));
    }
    if (lbl_1_bss_C == -1) {
        lbl_1_bss_C = HuAudJinglePlay(MSM_STREAM_MGMUS_17);
    }
}

/* Called by the following results sequence callback to keep the lineup in place. */
void fn_1_14C4(void) {
    s32 playerIndex;

    fn_1_640C();
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelPosSet(lbl_1_bss_C8[playerIndex].modelId,
                        (-300.0f) + (200.0f) * (f32) playerIndex,
                        (232.0f) + lbl_1_bss_41C[0].position.y, (0.0f));
    }
}

/* Called during scene setup to configure the selected day or night lighting and fog. */
void fn_1_1584(void) {

    Point3d *firstPositionPointer;
    Point3d *firstDirectionPointer;
    Point3d *secondPositionPointer;
    Point3d *secondDirectionPointer;

    lbl_1_bss_8E8.x = lbl_1_rodata_6C[lbl_1_bss_8E4].x;
    lbl_1_bss_8E8.y = lbl_1_rodata_6C[lbl_1_bss_8E4].y;
    lbl_1_bss_8E8.z = lbl_1_rodata_6C[lbl_1_bss_8E4].z;
    Hu3DAmbColorSet(lbl_1_rodata_6C[lbl_1_bss_8E4].x,
                    lbl_1_rodata_6C[lbl_1_bss_8E4].y,
                    lbl_1_rodata_6C[lbl_1_bss_8E4].z);
    Hu3DFogSet((500.0f), (30000.0f), 5, 0, 10);

    { Point3d firstPosition = {1000.0f, 1000.0f, 5000.0f}; firstPositionPointer = &firstPosition; lbl_1_bss_940 = *firstPositionPointer; }
    { Point3d firstDirection = {-1.0f, -1.0f, -1.0f}; firstDirectionPointer = &firstDirection; lbl_1_bss_934 = *firstDirectionPointer; }
    lbl_1_bss_930.r = lbl_1_rodata_60[lbl_1_bss_8E4][0];
    lbl_1_bss_930.g = lbl_1_rodata_60[lbl_1_bss_8E4][1];
    lbl_1_bss_930.b = lbl_1_rodata_60[lbl_1_bss_8E4][2];
    lbl_1_bss_930.a = 255;
    lbl_1_bss_94C = Hu3DGLightCreateV(&lbl_1_bss_940, &lbl_1_bss_934,
                                      &lbl_1_bss_930);
    lbl_1_bss_924[0] = lbl_1_bss_930.r;
    lbl_1_bss_924[1] = lbl_1_bss_930.g;
    lbl_1_bss_924[2] = lbl_1_bss_930.b;
    Hu3DGLightStaticSet(lbl_1_bss_94C, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_94C);

    { Point3d secondPosition = {0.0f, 700.0f, 0.0f}; secondPositionPointer = &secondPosition; lbl_1_bss_914 = *secondPositionPointer; }
    { Point3d secondDirection = {0.0f, -1.0f, 0.0f}; secondDirectionPointer = &secondDirection; lbl_1_bss_908 = *secondDirectionPointer; }
    lbl_1_bss_904.r = lbl_1_rodata_66[lbl_1_bss_8E4][0];
    lbl_1_bss_904.g = lbl_1_rodata_66[lbl_1_bss_8E4][1];
    lbl_1_bss_904.b = lbl_1_rodata_66[lbl_1_bss_8E4][2];
    lbl_1_bss_904.a = 255;
    lbl_1_bss_920 = Hu3DGLightCreateV(&lbl_1_bss_914, &lbl_1_bss_908,
                                      &lbl_1_bss_904);
    lbl_1_bss_8F8[0] = lbl_1_bss_904.r;
    lbl_1_bss_8F8[1] = lbl_1_bss_904.g;
    lbl_1_bss_8F8[2] = lbl_1_bss_904.b;
    Hu3DGLightStaticSet(lbl_1_bss_920, 1);
    lbl_1_bss_8F4 = (120.0f);
    Hu3DGLightSpotSet(lbl_1_bss_920, lbl_1_bss_8F4, GX_SP_COS);
}

/* Called after course frame 1370 to brighten primary and ambient light; dim the day spotlight or
 * raise it at night. */
void fn_1_1A70(void) {
    s32 fadeDivisor;

    fadeDivisor = 150;

    /* Integer division raises each primary RGB channel by one level per update, clamped at
     * white. */
    lbl_1_bss_924[0] += (255 - lbl_1_rodata_60[lbl_1_bss_8E4][0]) / fadeDivisor;
    if (lbl_1_bss_924[0] > 255) {
        lbl_1_bss_924[0] = 255;
    }
    lbl_1_bss_924[1] += (255 - lbl_1_rodata_60[lbl_1_bss_8E4][1]) / fadeDivisor;
    if (lbl_1_bss_924[1] > 255) {
        lbl_1_bss_924[1] = 255;
    }
    lbl_1_bss_924[2] += (255 - lbl_1_rodata_60[lbl_1_bss_8E4][2]) / fadeDivisor;
    if (lbl_1_bss_924[2] > 255) {
        lbl_1_bss_924[2] = 255;
    }
    lbl_1_bss_930.r = (u8) lbl_1_bss_924[0];
    lbl_1_bss_930.g = (u8) lbl_1_bss_924[1];
    lbl_1_bss_930.b = (u8) lbl_1_bss_924[2];
    Hu3DGLightColorSet(lbl_1_bss_94C, lbl_1_bss_930.r, lbl_1_bss_930.g,
                        lbl_1_bss_930.b, lbl_1_bss_930.a);

    if (lbl_1_bss_8E4 == 0) {
        lbl_1_bss_8F8[0] -= lbl_1_rodata_66[lbl_1_bss_8E4][0] / fadeDivisor;
        if (lbl_1_bss_8F8[0] < 0) {
            lbl_1_bss_8F8[0] = 0;
        }
        lbl_1_bss_8F8[1] -= lbl_1_rodata_66[lbl_1_bss_8E4][1] / fadeDivisor;
        if (lbl_1_bss_8F8[1] < 0) {
            lbl_1_bss_8F8[1] = 0;
        }
        lbl_1_bss_8F8[2] -= lbl_1_rodata_66[lbl_1_bss_8E4][2] / fadeDivisor;
        if (lbl_1_bss_8F8[2] < 0) {
            lbl_1_bss_8F8[2] = 0;
        }
        lbl_1_bss_904.r = (u8) lbl_1_bss_8F8[0];
        lbl_1_bss_904.g = (u8) lbl_1_bss_8F8[1];
        lbl_1_bss_904.b = (u8) lbl_1_bss_8F8[2];
        Hu3DGLightColorSet(lbl_1_bss_920, lbl_1_bss_904.r, lbl_1_bss_904.g,
                            lbl_1_bss_904.b, lbl_1_bss_904.a);
    } else {
        if (lbl_1_bss_914.y < (1000.0f)) {
            lbl_1_bss_914.y += (2.5f);
            Hu3DGLightPosSetV(lbl_1_bss_920, &lbl_1_bss_914, &lbl_1_bss_908);
        }
    }
    lbl_1_bss_8E8.x += ((1.0f) - (lbl_1_rodata_6C + lbl_1_bss_8E4)->x) / fadeDivisor;
    if (lbl_1_bss_8E8.x > (1.0f)) {
        lbl_1_bss_8E8.x = (1.0f);
    }
    lbl_1_bss_8E8.y += ((1.0f) - (lbl_1_rodata_6C + lbl_1_bss_8E4)->y) / fadeDivisor;
    if (lbl_1_bss_8E8.y > (1.0f)) {
        lbl_1_bss_8E8.y = (1.0f);
    }
    lbl_1_bss_8E8.z += ((1.0f) - (lbl_1_rodata_6C + lbl_1_bss_8E4)->z) / fadeDivisor;
    if (lbl_1_bss_8E8.z > (1.0f)) {
        lbl_1_bss_8E8.z = (1.0f);
    }
    Hu3DAmbColorSet(lbl_1_bss_8E8.x, lbl_1_bss_8E8.y, lbl_1_bss_8E8.z);
    if (lbl_1_bss_8E4 == 0) {
        Hu3DFogClear();
    }
}

/* Called during scene setup to set the initial camera view over the raft. */
void fn_1_20B0(void) {

    HuVecF *centerPointer;
    HuVecF *rotationPointer;

    { HuVecF center = {0.0f, 100.0f, 0.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; }

    { HuVecF rotation = {-40.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; }
    lbl_1_bss_8C8.zoom = (2155.0f);
    omCameraViewSet(&lbl_1_bss_8C8);
}

/* Called by sequence updates to move the camera at the raft scene's timed beats. */
void fn_1_2188(void)
{
    switch (lbl_1_bss_8C0) {
    case 148: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {0.0f, 550.0f, 0.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-15.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 2000.0f; omCameraViewMove(&lbl_1_bss_8C8, 200, 3); }
        return;
    }
    case 538: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {0.0f, 0.0f, 0.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-40.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 2500.0f; omCameraViewMove(&lbl_1_bss_8C8, 240, 3); }
        return;
    }
    case 950: {
        { HuVecF *centerPointer; { HuVecF center = {0.0f, 100.0f, 0.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } omCameraViewMove(&lbl_1_bss_8C8, 240, 3); }
        return;
    }
    case 1538: {
        { HuVecF *rotationPointer; { HuVecF rotation = {-52.0f, -70.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } omCameraViewMove(&lbl_1_bss_8C8, 400, 3); }
        return;
    }
    case 2338: {
        { HuVecF *rotationPointer; { HuVecF rotation = {-40.0f, 10.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } omCameraViewMove(&lbl_1_bss_8C8, 300, 3); }
        return;
    }
    case 2638: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {-496.5f, 300.0f, 112.5f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-45.5f, -2.700000047683716f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 2545.5f; omCameraViewMove(&lbl_1_bss_8C8, 200, 3); }
        return;
    }
    case 2938: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {0.0f, 300.0f, 500.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-30.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } omCameraViewMove(&lbl_1_bss_8C8, 200, 3); }
        return;
    }
    case 3100: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {0.0f, 500.0f, -200.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-30.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 2200.0f; omCameraViewMove(&lbl_1_bss_8C8, 300, 3); }
        return;
    }
    case 3638: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {635.0f, 200.0f, 17.5f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-414.5f, -20.5f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 2772.0f; omCameraViewMove(&lbl_1_bss_8C8, 400, 3); }
        return;
    }
    case 4638: {
        { HuVecF *centerPointer; HuVecF *rotationPointer; { HuVecF center = {0.0f, 100.0f, 0.0f}; centerPointer = &center; lbl_1_bss_8C8.center = *centerPointer; } { HuVecF rotation = {-40.0f, 0.0f, 0.0f}; rotationPointer = &rotation; lbl_1_bss_8C8.rot = *rotationPointer; } lbl_1_bss_8C8.zoom = 3000.0f; omCameraViewMove(&lbl_1_bss_8C8, 400, 3); }
        break;
    }
    }
}

/* Called during scene setup to create four player actors, character motions and falling-player
 * effects. */
void fn_1_2848(void) {
    MGACTOR_PARAM actorParam;
    Point3d opponentPosition;
    Point3d playerPosition;
    Point3d separation;
    s32 playerIndex;
    s32 motionIndex;

    actorParam.height = (150.0f);
    actorParam.radius = (40.0f);
    actorParam.param = 0;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = NULL;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        lbl_1_bss_C8[playerIndex].characterNo = GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_C8[playerIndex].padNo = GwPlayerConf[playerIndex].padNo;
        lbl_1_bss_18[playerIndex].difficulty = GwPlayerConf[playerIndex].comDif;
        lbl_1_bss_18[playerIndex].type = GwPlayerConf[playerIndex].type;
        lbl_1_bss_18[playerIndex].sessionState[0] = 0;
        lbl_1_bss_18[playerIndex].sessionState[1] = 0;
        lbl_1_bss_18[playerIndex].sessionState[2] = -1;
        lbl_1_bss_18[playerIndex].sessionState[3] = 0;
        lbl_1_bss_C8[playerIndex].player = MgPlayerCreate(
            (s16) playerIndex, &actorParam, 8, 1, 47, NULL);
        MgPlayerAttrSet(lbl_1_bss_C8[playerIndex].player, 1);
        Hu3DModelObjPosGet(lbl_1_bss_85C[playerIndex].modelId,
                           "fook00",
                           &lbl_1_bss_C8[playerIndex].player->actor->pos);
        lbl_1_bss_C8[playerIndex].player->actor->pos.y += (100.0f);
        lbl_1_bss_C8[playerIndex].player->actor->rotY = (0.0f);
        lbl_1_bss_C8[playerIndex].modelId =
            lbl_1_bss_C8[playerIndex].player->actor->mdlId;
        Hu3DModelCameraSet(lbl_1_bss_C8[playerIndex].modelId, 1);
        Hu3DModelShadowSet(lbl_1_bss_C8[playerIndex].modelId);
        Hu3DModelLayerSet(lbl_1_bss_C8[playerIndex].modelId, 1);
        CharEffectLayerSet(3);
        for (motionIndex = 0; motionIndex < 7; motionIndex++) {
            lbl_1_bss_C8[playerIndex].motionIds[motionIndex] =
                CharMotionCreate(lbl_1_bss_C8[playerIndex].characterNo,
                                 lbl_1_data_28[motionIndex]);
        }
        CharMotionDataClose(lbl_1_bss_C8[playerIndex].characterNo);
        lbl_1_bss_C8[playerIndex].currentMotion = 0;
        lbl_1_bss_C8[playerIndex].actionState = 0;
        lbl_1_bss_C8[playerIndex].sequenceCount = 0;
        lbl_1_bss_C8[playerIndex].stunSoundLatched = 0;
        lbl_1_bss_C8[playerIndex].fallEffectModelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 66), HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DModelCameraSet(lbl_1_bss_C8[playerIndex].fallEffectModelId, 1);
        Hu3DModelAttrSet(lbl_1_bss_C8[playerIndex].fallEffectModelId, 1);
        Hu3DModelScaleSet(lbl_1_bss_C8[playerIndex].fallEffectModelId,
                          (1.5f), (1.5f), (1.5f));
        Hu3DModelLayerSet(lbl_1_bss_C8[playerIndex].fallEffectModelId, 5);
        lbl_1_bss_C8[playerIndex].fallEffectMotionId = Hu3DJointMotion(
            lbl_1_bss_C8[playerIndex].fallEffectModelId,
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 67), HU_MEMNUM_OVL, HEAP_DVD));
        MgPlayerVibrateCreate(lbl_1_bss_C8[playerIndex].player);
    }

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerPosition = lbl_1_bss_C8[playerIndex].player->actor->pos;
        opponentPosition = lbl_1_bss_C8[3 - playerIndex].player->actor->pos;
        PSVECSubtract(&opponentPosition, &playerPosition, &separation);
        lbl_1_bss_C8[playerIndex].player->actor->rotY =
            (180.0) *
            (atan2((f64) separation.x, (f64) separation.z) /
             (3.141592653589793));
    }
    MgActorExec();
}

/* Called by sequence updates to process player falls, AI input and temporary hit reactions. */
void fn_1_2E08(void) {
    Point3d actorPosition;
    f32 movementLength;
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (lbl_1_bss_C8[playerIndex].stunSoundLatched == 0) {
            if (lbl_1_bss_C8[playerIndex].player->stunType == 1 ||
                lbl_1_bss_C8[playerIndex].player->stunType == 2) {
                lbl_1_bss_C8[playerIndex].stunSoundLatched = 1;
                fn_1_B910(2142, lbl_1_bss_C8[playerIndex].modelId, 1);
            }
        } else if (lbl_1_bss_C8[playerIndex].player->stunType == 6) {
            lbl_1_bss_C8[playerIndex].stunSoundLatched = 0;
        }

        if (lbl_1_bss_18[playerIndex].type != 0) {
            fn_1_AC58(playerIndex);
        }

        if (lbl_1_bss_C8[playerIndex].actionState >= 0 &&
            lbl_1_bss_C8[playerIndex].player->actor->pos.y < (0.0f)) {
            actorPosition = lbl_1_bss_C8[playerIndex].player->actor->pos;
            actorPosition.y = (0.0f);
            Hu3DModelPosSetV(lbl_1_bss_C8[playerIndex].fallEffectModelId, &actorPosition);
            Hu3DModelRotSet(lbl_1_bss_C8[playerIndex].fallEffectModelId, (0.0f),
                            lbl_1_bss_C8[playerIndex].player->actor->rotY, (0.0f));
            Hu3DModelAttrReset(lbl_1_bss_C8[playerIndex].fallEffectModelId, 1);
            Hu3DMotionSet(lbl_1_bss_C8[playerIndex].fallEffectModelId,
                          lbl_1_bss_C8[playerIndex].fallEffectMotionId);
            Hu3DModelShadowReset(lbl_1_bss_C8[playerIndex].modelId);
            omVibrate((s16) playerIndex, 20, 20, 0);
            if (MgSeqModeGet() == 5) {
                lbl_1_bss_C8[playerIndex].sequenceCount = lbl_1_bss_8BC;
                lbl_1_bss_8BC--;
            }
            lbl_1_bss_C8[playerIndex].actionState = -2;
            fn_1_B910(2140, lbl_1_bss_C8[playerIndex].modelId, 1);
        }

        fn_1_3868(playerIndex);
        if (lbl_1_bss_C8[playerIndex].actionState != 0) {
            MgPlayerAttrSet(lbl_1_bss_C8[playerIndex].player, 1);
            switch (lbl_1_bss_C8[playerIndex].actionState) {
            case -1:
                if (lbl_1_bss_C8[playerIndex].currentMotion == 0) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[3],
                                       (0.0f), (8.0f),
                                       HU3D_MOTATTR_LOOP);
                    lbl_1_bss_C8[playerIndex].currentMotion = 3;
                    lbl_1_bss_C8[playerIndex].movementTimer = 0;
                    lbl_1_bss_C8[playerIndex].movementOrigin =
                        lbl_1_bss_C8[playerIndex].player->actor->pos;
                    lbl_1_bss_C8[playerIndex].launchHeight =
                        lbl_1_bss_C8[playerIndex].player->actor->pos.y;
                    PSVECSubtract(&lbl_1_bss_C8[playerIndex].flightDestination,
                                  &lbl_1_bss_C8[playerIndex].movementOrigin,
                                  &lbl_1_bss_C8[playerIndex].movementDirection);
                    movementLength = PSVECMag(&lbl_1_bss_C8[playerIndex].movementDirection);
                    PSVECNormalize(&lbl_1_bss_C8[playerIndex].movementDirection,
                                   &lbl_1_bss_C8[playerIndex].movementDirection);
                    PSVECScale(&lbl_1_bss_C8[playerIndex].movementDirection,
                               &lbl_1_bss_C8[playerIndex].movementDirection,
                               movementLength / (120.0f));
                }
                if (lbl_1_bss_C8[playerIndex].currentMotion == 3) {
                    if (++lbl_1_bss_C8[playerIndex].movementTimer < 120) {
                        lbl_1_bss_C8[playerIndex].movementOrigin =
                            lbl_1_bss_C8[playerIndex].player->actor->pos;
                        /* Rebuild Y from the launch height and arc each frame, then add the fixed
                         * displacement; X/Z accumulate. */
                        lbl_1_bss_C8[playerIndex].movementOrigin.y =
                            lbl_1_bss_C8[playerIndex].launchHeight +
                            (1200.0) *
                                sin(((3.141592653589793) *
                                     (f64) ((1.5f) *
                                            (f32) lbl_1_bss_C8[playerIndex].movementTimer)) /
                                    (180.0));
                        PSVECAdd(&lbl_1_bss_C8[playerIndex].movementDirection,
                                 &lbl_1_bss_C8[playerIndex].movementOrigin,
                                 &lbl_1_bss_C8[playerIndex].movementOrigin);
                        lbl_1_bss_C8[playerIndex].player->actor->pos =
                            lbl_1_bss_C8[playerIndex].movementOrigin;
                        Hu3DModelPosSetV(lbl_1_bss_C8[playerIndex].modelId,
                                         &lbl_1_bss_C8[playerIndex].movementOrigin);
                    } else {
                        Hu3DModelAttrSet(lbl_1_bss_C8[playerIndex].modelId, 1);
                    }
                }
                break;
            case 1:
                if (lbl_1_bss_C8[playerIndex].currentMotion == 0) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[2],
                                       (0.0f), (8.0f),
                                       HU3D_MOTATTR_LOOP);
                    lbl_1_bss_C8[playerIndex].currentMotion = 2;
                }
                if (lbl_1_bss_C8[playerIndex].actionTimer >= 90) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[0],
                                       (0.0f), (8.0f),
                                       HU3D_MOTATTR_LOOP);
                    lbl_1_bss_C8[playerIndex].currentMotion = 0;
                    lbl_1_bss_C8[playerIndex].actionState = 0;
                    MgPlayerAttrReset(lbl_1_bss_C8[playerIndex].player, 1);
                    lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0] =
                        lbl_1_bss_C8[playerIndex].savedMotionId;
                }
                break;
            case 2:
                if (lbl_1_bss_C8[playerIndex].currentMotion == 0) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[1],
                                       (0.0f), (8.0f),
                                       HU3D_MOTATTR_LOOP);
                    lbl_1_bss_C8[playerIndex].currentMotion = 1;
                }
                if (lbl_1_bss_C8[playerIndex].actionTimer >= 90) {
                    CharMotionShiftSet(lbl_1_bss_C8[playerIndex].characterNo,
                                       lbl_1_bss_C8[playerIndex].motionIds[0],
                                       (0.0f), (8.0f),
                                       HU3D_MOTATTR_LOOP);
                    lbl_1_bss_C8[playerIndex].currentMotion = 0;
                    lbl_1_bss_C8[playerIndex].actionState = 0;
                    MgPlayerAttrReset(lbl_1_bss_C8[playerIndex].player, 1);
                    lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0] =
                        lbl_1_bss_C8[playerIndex].savedMotionId;
                }
                break;
            }
            lbl_1_bss_C8[playerIndex].actionTimer++;
        }
    }
}

/* Called for each player by fn_1_2E08 to detect collisions with the raft hazards. */
void fn_1_3868(s32 playerIndex) {
    s32 tableIndex;
    s32 modelIndex;
    Point3d playerPosition;
    Point3d effectPosition;
    Point3d difference;
    f32 randomOffset[2];
    f32 distance;

    playerPosition = lbl_1_bss_C8[playerIndex].player->actor->pos;
    playerPosition.y += (75.0f);
    if (lbl_1_bss_8C0 > 2500) {
        for (tableIndex = 0; tableIndex < 2; tableIndex++) {
            for (modelIndex = 0; modelIndex < 3; modelIndex++) {
                if (lbl_1_bss_248[tableIndex][modelIndex].state38 == 2) {
                    effectPosition = lbl_1_bss_248[tableIndex][modelIndex].position;
                    PSVECSubtract(&effectPosition, &playerPosition, &difference);
                    distance = PSVECMag(&difference);
                    if (distance < (88.0f) && lbl_1_bss_C8[playerIndex].actionState == 0) {
                        lbl_1_bss_C8[playerIndex].actionState = -1;
                        lbl_1_bss_C8[playerIndex].actionTimer = 0;
                        lbl_1_bss_248[tableIndex][modelIndex].state40 = 2;
                        randomOffset[0] = (u32) (frandmod(150) + 3000);
                        randomOffset[1] = (u32) frandmod(600);
                        if ((u32) frandmod(2) != 0U) {
                            randomOffset[1] *= (-1.0f);
                        }
                        lbl_1_bss_C8[playerIndex].flightDestination.y = (0.0f);
                        if (lbl_1_bss_464 == 0) {
                            lbl_1_bss_C8[playerIndex].flightDestination.x =
                                playerPosition.x + randomOffset[0];
                            lbl_1_bss_C8[playerIndex].flightDestination.z =
                                playerPosition.z + randomOffset[1];
                        } else if (lbl_1_bss_464 == 1) {
                            lbl_1_bss_C8[playerIndex].flightDestination.x =
                                playerPosition.x + randomOffset[1];
                            lbl_1_bss_C8[playerIndex].flightDestination.z =
                                playerPosition.z + randomOffset[0];
                        } else {
                            lbl_1_bss_C8[playerIndex].flightDestination.x =
                                playerPosition.x - randomOffset[0];
                            lbl_1_bss_C8[playerIndex].flightDestination.z =
                                playerPosition.z + randomOffset[1];
                        }
                        MgPlayerDespawn(lbl_1_bss_C8[playerIndex].player);
                        Hu3DModelAttrReset(lbl_1_bss_C8[playerIndex].modelId, 1);
                        omVibrate(playerIndex, 20, 20, 0);
                        if ((u32) MgSeqModeGet() == 5U) {
                            lbl_1_bss_C8[playerIndex].sequenceCount = lbl_1_bss_8BC;
                            lbl_1_bss_8BC -= 1;
                        }
                    }
                }
            }
        }
    } else if (lbl_1_bss_8C0 > 1700) {
        for (tableIndex = 0; tableIndex < 5; tableIndex++) {
            for (modelIndex = 0; modelIndex < 4; modelIndex++) {
                if (lbl_1_bss_6F4[tableIndex][modelIndex] == 0) {

                    Hu3DModelObjPosGet(lbl_1_bss_844[tableIndex],
                                      modelIndex == 0 ? "nuts_hook0"
                                      : modelIndex == 1 ? "nuts_hook1"
                                      : modelIndex == 2 ? "nuts_hook2"
                                                        : "nuts_hook3",
                                      &effectPosition);
                    effectPosition.y -= lbl_1_bss_6A4[tableIndex][modelIndex];
                    PSVECSubtract(&effectPosition, &playerPosition, &difference);
                    distance = PSVECMag(&difference);
                    if (distance < (84.0f)) {
                        if (lbl_1_bss_C8[playerIndex].actionState == 0) {
                            if (lbl_1_bss_C8[playerIndex].player->stunType == 0) {
                                lbl_1_bss_C8[playerIndex].actionState = 2;
                                lbl_1_bss_C8[playerIndex].actionTimer = 0;
                                lbl_1_bss_C8[playerIndex].savedMotionId =
                                    lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0];
                                lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0] =
                                    lbl_1_bss_C8[playerIndex].motionIds[1];
                            }
                            omVibrate(playerIndex, 20, 7, 3);
                        }
                        lbl_1_bss_6F4[tableIndex][modelIndex] = 1;
                    }
                }
            }
        }
    }
}

/* Called during scene setup to create the raft scene and initialize its collision map. */
void fn_1_3E74(void) {
    /* The collision list collects nine course and raft model IDs through this six-entry local
     * array. */
    s16 collisionMap[6];
    Point3d targetPosition;
    Point3d currentPosition;

    Point3d *targetCopyPointer;
    s32 mapCount;

    mapCount = 0;
    lbl_1_bss_86E = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 20), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_86E, 1);
    Hu3DModelPosSet(lbl_1_bss_86E, (0.0f),
                    (0.0f), (0.0f));
    lbl_1_bss_850[0] = lbl_1_bss_850[1] = (0.0f);
    Hu3DMotionSpeedSet(lbl_1_bss_86E, lbl_1_bss_850[0]);
    Hu3DMotionCalc(lbl_1_bss_86E);
    Hu3DModelScaleSet(lbl_1_bss_86E, (10.0f),
                      (10.0f), (10.0f));
    Hu3DModelObjPosGet(lbl_1_bss_86E, "fk_ikada", &currentPosition);
    { Point3d targetPositionCopy = {0.0f, 0.0f, 0.0f}; targetCopyPointer = &targetPositionCopy; targetPosition = *targetCopyPointer; }
    PSVECSubtract(&targetPosition, &currentPosition, &lbl_1_bss_40C);
    mapCount += fn_1_420C(&collisionMap[mapCount]);
    mapCount += fn_1_4848(&collisionMap[mapCount]);
    fn_1_4DD8();
    fn_1_5770();
    mapCount += fn_1_58E8(&collisionMap[mapCount]);
    fn_1_6704();
    fn_1_7E24();
    fn_1_8E44();
    MgActorColMapInit(collisionMap, (s16) mapCount, 8);
}

/* Runs during active play to update the raft motion and advance its scene animations. */
void fn_1_4060(void) {
    Point3d targetPosition;
    Point3d currentPosition;

    Point3d *targetCopyPointer;

    if (lbl_1_bss_8C0 == 0) {
        lbl_1_bss_850[0] = lbl_1_bss_850[1] = (0.02500000037252903f);
        Hu3DMotionSpeedSet(lbl_1_bss_86E, lbl_1_bss_850[0]);
    }
    Hu3DModelObjPosGet(lbl_1_bss_86E, "fk_ikada", &currentPosition);
    { Point3d targetPositionCopy = {0.0f, 0.0f, 0.0f}; targetCopyPointer = &targetPositionCopy; targetPosition = *targetCopyPointer; }
    PSVECSubtract(&targetPosition, &currentPosition, &lbl_1_bss_40C);
    if ((lbl_1_bss_8C0 >= 2138) && (lbl_1_bss_8C0 <= 2438)) {
        lbl_1_bss_850[1] = lbl_1_bss_850[1] - (0.0013333333190530539f) * lbl_1_bss_850[0];
        Hu3DMotionSpeedSet(lbl_1_bss_86E, lbl_1_bss_850[1]);
        if (lbl_1_bss_8C0 == 2438) {
            lbl_1_bss_850[0] = lbl_1_bss_850[1];
        }
    }
    fn_1_476C();
    fn_1_4CC8();
    fn_1_5108();
    fn_1_5860();
    fn_1_6D7C();
    fn_1_8244();
    fn_1_9704();
    fn_1_5D3C();
}

/* Called by fn_1_3E74 to create the day or night course models and collect their collision model
 * IDs. */
s32 fn_1_420C(s16 *createdModelIds) {
    s32 modelCount = 0;
    s32 modelIndex;
    f32 motionSpeed;
    s32 alternateModelIndex;
    s32 modelNumber;
    s32 layer;

    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        if (lbl_1_bss_8E4 == 0) {
            lbl_1_bss_8A0[modelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(modelIndex + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            if (modelIndex == 0) {
                alternateModelIndex = 0;
            } else {
                alternateModelIndex = modelIndex + 3;
            }
            lbl_1_bss_8A0[modelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(alternateModelIndex + DATA_m664,
                                     HU_MEMNUM_OVL, HEAP_MODEL));
        }
        Hu3DModelCameraSet(lbl_1_bss_8A0[modelIndex], 1);
        Hu3DModelPosSetV(lbl_1_bss_8A0[modelIndex], &lbl_1_bss_40C);
        Hu3DModelScaleSet(lbl_1_bss_8A0[modelIndex], (10.0f),
                          (10.0f), (10.0f));
        if (modelIndex == 0) {
            Hu3DModelLayerSet(lbl_1_bss_8A0[modelIndex], 4);
        }
        if (modelIndex > 0) {
            Hu3DModelAttrSet(lbl_1_bss_8A0[modelIndex], 1);
        }

        lbl_1_bss_894[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelIndex + DATA_m664 + 7, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_894[modelIndex], 1);
        Hu3DModelPosSetV(lbl_1_bss_894[modelIndex], &lbl_1_bss_40C);
        Hu3DModelScaleSet(lbl_1_bss_894[modelIndex], (10.0f),
                          (10.0f), (10.0f));
        if (modelIndex > 0) {
            Hu3DModelAttrSet(lbl_1_bss_894[modelIndex], 1);
        }
        *createdModelIds = lbl_1_bss_894[modelIndex];
        createdModelIds++;
        modelCount++;
    }

    if (lbl_1_bss_8E4 == 0) {
        modelNumber = 11;
    } else {
        modelNumber = 12;
    }
    lbl_1_bss_89E = Hu3DModelCreate(
        HuDataSelHeapReadNum(modelNumber + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_89E, 1);
    Hu3DModelPosSetV(lbl_1_bss_89E, &lbl_1_bss_40C);
    Hu3DModelScaleSet(lbl_1_bss_89E, (10.0f),
                      (10.0f), (10.0f));
    Hu3DModelAttrSet(lbl_1_bss_89E, 1);

    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        lbl_1_bss_870[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelIndex + DATA_m664 + 18, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_870[modelIndex], 1);
        Hu3DModelPosSetV(lbl_1_bss_870[modelIndex], &lbl_1_bss_40C);
        Hu3DModelAttrSet(lbl_1_bss_870[modelIndex], HU3D_MOTATTR_LOOP);
        if (modelIndex == 0) {
            motionSpeed = (0.20000000298023224f);
        } else {
            motionSpeed = (0.10000000149011612f);
        }
        Hu3DMotionSpeedSet(lbl_1_bss_870[modelIndex], motionSpeed);
        Hu3DMotionCalc(lbl_1_bss_870[modelIndex]);
        Hu3DModelScaleSet(lbl_1_bss_870[modelIndex], (10.0f),
                          (10.0f), (10.0f));
        if (modelIndex == 0) {
            layer = 3;
        } else {
            layer = 2;
        }
        Hu3DModelLayerSet(lbl_1_bss_870[modelIndex], layer);
    }

    lbl_1_bss_86C = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 21), HU_MEMNUM_OVL, HEAP_DVD));
    Hu3DModelCameraSet(lbl_1_bss_86C, 1);
    Hu3DModelPosSetV(lbl_1_bss_86C, &lbl_1_bss_40C);
    Hu3DModelScaleSet(lbl_1_bss_86C, (10.0f),
                      (10.0f), (10.0f));
    Hu3DModelAttrSet(lbl_1_bss_86C, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_86C, (0.33000001311302185f));
    Hu3DModelLayerSet(lbl_1_bss_86C, 4);
    return modelCount;
}

/* Called by fn_1_4060 to translate scene models by the negative fk_ikada position sampled from the
 * course motion model. */
void fn_1_476C(void) {
    s32 modelIndex;
    s32 unusedModelIndex;
    modelIndex = 0;
    while (modelIndex < 4)
    {
        Hu3DModelPosSetV(* (s16 *)((u8 *) & lbl_1_bss_8A0 + (modelIndex * 2)), & lbl_1_bss_40C);
        Hu3DModelPosSetV(* (s16 *)((u8 *) & lbl_1_bss_894 + (modelIndex * 2)), & lbl_1_bss_40C);
        modelIndex += 1;
    }
    Hu3DModelPosSetV(lbl_1_bss_89E, & lbl_1_bss_40C);
    modelIndex = 0;
    while (modelIndex < 2)
    {
        Hu3DModelPosSetV(* (s16 *)((u8 *) & lbl_1_bss_870 + (modelIndex * 2)), & lbl_1_bss_40C);
        modelIndex += 1;
    }
    Hu3DModelPosSetV(lbl_1_bss_86C, & lbl_1_bss_40C);
}

Point3d lbl_1_data_80 = { -6000.0f, 0.0f, 22397.119140625f };

/* Called by fn_1_3E74 to create the four player raft models and their hook objects. */
s32 fn_1_4848(s16 *objectModelOutput) {
    Point3d position;
    Point3d rotation;
    f32 rotationY;
    f32 selectedAngle;
    f32 baseAngle;
    s32 modelCount;
    s32 modelIndex;
    s32 modelNumber;

    modelCount = 0;
    for (modelIndex = 0; modelIndex < 4; modelIndex++) {
        lbl_1_bss_85C[modelIndex].modelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 22), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_85C[modelIndex].modelId, 1);

        position = lbl_1_bss_40C;
        PSVECAdd(&position, &lbl_1_data_80, &position);
        Hu3DModelPosSetV(lbl_1_bss_85C[modelIndex].modelId, &position);

        rotation.x = rotation.z = (0.0f);
        if (modelIndex == 0) {
            rotationY = (0.0f);
        } else {
            if (modelIndex == 1) {
                selectedAngle = (-90.0f);
            } else {
                if (modelIndex == 2) {
                    baseAngle = (90.0f);
                } else {
                    baseAngle = (180.0f);
                }
                selectedAngle = baseAngle;
            }
            rotationY = selectedAngle;
        }
        rotation.y = rotationY;
        Hu3DModelRotSetV(lbl_1_bss_85C[modelIndex].modelId, &rotation);
        Hu3DModelScaleSet(lbl_1_bss_85C[modelIndex].modelId, (10.0f),
                          (10.0f), (10.0f));
        Hu3DMotionSpeedSet(lbl_1_bss_85C[modelIndex].modelId, (0.0f));
        Hu3DMotionCalc(lbl_1_bss_85C[modelIndex].modelId);

        lbl_1_bss_85C[modelIndex].objectModelId = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 23), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelObjPosGet(lbl_1_bss_85C[modelIndex].modelId, "fook00", &position);
        Hu3DModelPosSetV(lbl_1_bss_85C[modelIndex].objectModelId, &position);
        *objectModelOutput = lbl_1_bss_85C[modelIndex].objectModelId;
        objectModelOutput++;
        modelCount++;
    }

    lbl_1_bss_85A =
        Hu3DModelCreate(HuDataSelHeapReadNum(DATANUM(DATA_m664, 24), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_85A, 1);
    position = lbl_1_bss_40C;
    PSVECAdd(&position, &lbl_1_data_80, &position);
    Hu3DModelPosSetV(lbl_1_bss_85A, &position);
    Hu3DModelScaleSet(lbl_1_bss_85A, (10.0f),
                      (10.0f), (10.0f));

    if (lbl_1_bss_8E4 == 0) {
        modelNumber = 25;
    } else {
        modelNumber = 26;
    }
    lbl_1_bss_858 = Hu3DModelCreate(HuDataSelHeapReadNum(modelNumber + DATA_m664,
                                                         HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_858, 1);
    Hu3DModelPosSet(lbl_1_bss_858, (0.0f), (0.0f),
                    (0.0f));
    Hu3DModelScaleSet(lbl_1_bss_858, (10.0f),
                      (10.0f), (10.0f));
    Hu3DModelAttrSet(lbl_1_bss_858, 1);
    Hu3DModelShadowMapSet(lbl_1_bss_858);
        Hu3DModelAttrSet(lbl_1_bss_858, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_858, (0.20000000298023224f));
    return modelCount;
}

/* Called by fn_1_4060 during scene updates to align the player raft models with the course. */
void fn_1_4CC8(void) {
    Point3d position;
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        position = lbl_1_bss_40C;
        PSVECAdd(&position, &lbl_1_data_80, &position);
        Hu3DModelPosSetV(lbl_1_bss_85C[playerIndex].modelId, &position);
        Hu3DModelObjPosGet(lbl_1_bss_85C[playerIndex].modelId, "fook00",
                           &position);
        Hu3DModelPosSetV(lbl_1_bss_85C[playerIndex].objectModelId, &position);
    }
    position = lbl_1_bss_40C;
    PSVECAdd(&position, &lbl_1_data_80, &position);
    Hu3DModelPosSetV(lbl_1_bss_85A, &position);
}

/* Called by fn_1_3E74 to create diver models at the named diver objects on the course. */
void fn_1_4DD8(void) {
    Mtx objectMatrix;
    Point3d position;
    Point3d rotation;
    s32 modelIndex;
    char *objectName14;
    char *objectName13;
    char *objectName12;
    char *objectName11;
    char *objectName10;
    char *objectName9;
    char *objectName8;
    char *objectName7;
    char *objectName6;
    char *objectName5;
    char *objectName4;
    char *objectName3;
    char *objectName2;

    for (modelIndex = 0; modelIndex < 16; modelIndex++) {
        if (modelIndex < 2) {
            lbl_1_bss_874[modelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(modelIndex + DATA_m664 + 13, HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSetV(lbl_1_bss_874[modelIndex], &lbl_1_bss_40C);
        } else {
            lbl_1_bss_874[modelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 15), HU_MEMNUM_OVL, HEAP_MODEL));
            if (modelIndex == 2) {
                objectName14 = "drfook00";
            } else if (modelIndex == 3) {
                objectName13 = "drfook01";
                goto objectName14From13;
            } else if (modelIndex == 4) {
                objectName12 = "drfook02";
                goto objectName13From12;
            } else if (modelIndex == 5) {
                objectName11 = "drfook03";
                goto objectName12From11;
            } else if (modelIndex == 6) {
                objectName10 = "drfook04";
                goto objectName11From10;
            } else if (modelIndex == 7) {
                objectName9 = "drfook05";
                goto objectName10From9;
            } else if (modelIndex == 8) {
                objectName8 = "drfook06";
                goto objectName9From8;
            } else if (modelIndex == 9) {
                objectName7 = "drfook07";
                goto objectName8From7;
            } else if (modelIndex == 10) {
                objectName6 = "drfook08";
                goto objectName7From6;
            } else if (modelIndex == 11) {
                objectName5 = "drfook09";
                goto objectName6From5;
            } else if (modelIndex == 12) {
                objectName4 = "drfook10";
                goto objectName5From4;
            } else if (modelIndex == 13) {
                objectName3 = "drfook11";
                goto objectName4From3;
            } else if (modelIndex == 14) {
                objectName2 = "drfook12";
                goto objectName3From2;
            } else {
                objectName2 = "drfook13";
                goto objectName3From2;
            }
            goto objectNameReady;
objectName3From2:
            objectName3 = objectName2;
objectName4From3:
            objectName4 = objectName3;
objectName5From4:
            objectName5 = objectName4;
objectName6From5:
            objectName6 = objectName5;
objectName7From6:
            objectName7 = objectName6;
objectName8From7:
            objectName8 = objectName7;
objectName9From8:
            objectName9 = objectName8;
objectName10From9:
            objectName10 = objectName9;
objectName11From10:
            objectName11 = objectName10;
objectName12From11:
            objectName12 = objectName11;
objectName13From12:
            objectName13 = objectName12;
objectName14From13:
            objectName14 = objectName13;
objectNameReady:
            Hu3DModelObjMtxGet(lbl_1_bss_874[1], objectName14, objectMatrix);
            Hu3DMtxTransGet(objectMatrix, &position);
            Hu3DMtxRotGet(objectMatrix, &rotation);
            rotation.y += (180.0f);
            Hu3DModelPosSetV(lbl_1_bss_874[modelIndex], &position);
            Hu3DModelRotSetV(lbl_1_bss_874[modelIndex], &rotation);
        }
        Hu3DModelCameraSet(lbl_1_bss_874[modelIndex], 1);
        Hu3DMotionSpeedSet(lbl_1_bss_874[modelIndex], (0.0f));
        Hu3DMotionCalc(lbl_1_bss_874[modelIndex]);
        Hu3DModelScaleSet(lbl_1_bss_874[modelIndex], (10.0f),
                          (10.0f), (10.0f));
        Hu3DModelLayerSet(lbl_1_bss_874[modelIndex], 4);
    }
}

/* Called by fn_1_4060 to position divers and trigger timed motions; randomized dive sounds use the
 * opposite diver of each animated pair. */
void fn_1_5108(void) {
    s32 index;
    Point3d objectPosition;

    for (index = 0; index < 16; index++) {
        if (index < 2) {
            Hu3DModelPosSetV(lbl_1_bss_874[index], &lbl_1_bss_40C);
        } else {
            Hu3DModelObjPosGet(lbl_1_bss_874[1],
                               index == 2  ? "drfook00"
                               : index == 3  ? "drfook01"
                               : index == 4  ? "drfook02"
                               : index == 5  ? "drfook03"
                               : index == 6  ? "drfook04"
                               : index == 7  ? "drfook05"
                               : index == 8  ? "drfook06"
                               : index == 9  ? "drfook07"
                               : index == 10 ? "drfook08"
                               : index == 11 ? "drfook09"
                               : index == 12 ? "drfook10"
                               : index == 13 ? "drfook11"
                               : index == 14 ? "drfook12"
                                             : "drfook13",
                               &objectPosition);
            Hu3DModelPosSetV(lbl_1_bss_874[index], &objectPosition);
        }
    }

    switch (lbl_1_bss_8C0) {
    case 288:
        Hu3DMotionSpeedSet(lbl_1_bss_874[0], (0.03999999910593033f));
        break;
    case 585:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[2] : lbl_1_bss_874[9], (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 2 : 9], 1);
        break;
    case 630:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[3] : lbl_1_bss_874[10],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 3 : 10], 1);
        break;
    case 670:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[4] : lbl_1_bss_874[11],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 4 : 11], 1);
        break;
    case 720:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[5] : lbl_1_bss_874[12],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 5 : 12], 1);
        break;
    case 765:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[6] : lbl_1_bss_874[13],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 6 : 13], 1);
        break;
    case 830:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[7] : lbl_1_bss_874[14],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 7 : 14], 1);
        break;
    case 890:
        index = frandmod(2);
        Hu3DMotionSpeedSet(index != 0 ? lbl_1_bss_874[8] : lbl_1_bss_874[15],
                           (0.6000000238418579f));
        fn_1_B910(2128, lbl_1_bss_874[index == 0 ? 8 : 15], 1);
        break;
    }

    if (lbl_1_bss_8C0 == 315) {
        HuAudFXPlay(MSM_SE_M664_SCENE_TRANSITION);
    }
}

/* Called by fn_1_3E74 to create the day or night model used in the course reveal. */
void fn_1_5770(void) {
    s32 modelAssetOffset;

    if ((s32) lbl_1_bss_8E4 == 0) {
        modelAssetOffset = 16;
    } else {
        modelAssetOffset = 17;
    }
    lbl_1_bss_89C = Hu3DModelCreate(
        HuDataSelHeapReadNum(modelAssetOffset + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_89C, 1U);
    Hu3DModelPosSetV(lbl_1_bss_89C, &lbl_1_bss_40C);
    Hu3DModelScaleSet(lbl_1_bss_89C, (10.0f), (10.0f), (10.0f));
    Hu3DMotionSpeedSet(lbl_1_bss_89C, (0.0f));
    Hu3DMotionCalc(lbl_1_bss_89C);
}

/* Called by fn_1_4060 during scene updates to animate the course model and
 * play its reveal cue. */
void fn_1_5860(void) {
    if (lbl_1_bss_8C0 > 1370) {
        fn_1_1A70();
    }
    Hu3DModelPosSetV(lbl_1_bss_89C, &lbl_1_bss_40C);
    switch (lbl_1_bss_8C0) {
    case 1368:
        Hu3DMotionSpeedSet(lbl_1_bss_89C, (1.0f));
        HuAudFXPlay(MSM_SE_M664_COURSE_REVEAL);
        break;
    }
}

/* Called by fn_1_3E74 to create the raft sway models and initialize their motion phases. */
s32 fn_1_58E8(s16 *createdModelIds) {

    Point3d *startPositionPointer;
    Point3d *startRotationPointer;
    s32 modelCount = 0;
    s32 modelIndex;
    s32 horizontalDirection;
    s32 verticalDirection;

    for (modelIndex = 0; modelIndex < 1; modelIndex++) {
        lbl_1_bss_41C[modelIndex].modelId[0] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 52), HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DModelCameraSet(lbl_1_bss_41C[modelIndex].modelId[0], 1);
        { Point3d startPosition = {0.0f, 0.0f, 0.0f}; startPositionPointer = &startPosition; lbl_1_bss_41C[modelIndex].position = *startPositionPointer; }
        Hu3DModelPosSetV(lbl_1_bss_41C[modelIndex].modelId[0],
                         &lbl_1_bss_41C[modelIndex].position);
        { Point3d startRotation = {0.0f, 0.0f, 0.0f}; startRotationPointer = &startRotation; lbl_1_bss_41C[modelIndex].rotation = *startRotationPointer; }
        Hu3DModelRotSetV(lbl_1_bss_41C[modelIndex].modelId[0],
                         &lbl_1_bss_41C[modelIndex].rotation);
    Hu3DModelShadowMapSet(lbl_1_bss_41C[modelIndex].modelId[0]);
        lbl_1_bss_41C[modelIndex].modelId[1] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 53), HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DModelCameraSet(lbl_1_bss_41C[modelIndex].modelId[1], 1);
        Hu3DModelPosSetV(lbl_1_bss_41C[modelIndex].modelId[1],
                         &lbl_1_bss_41C[modelIndex].position);
        Hu3DModelRotSetV(lbl_1_bss_41C[modelIndex].modelId[1],
                         &lbl_1_bss_41C[modelIndex].rotation);
        Hu3DModelAttrSet(lbl_1_bss_41C[modelIndex].modelId[1], 1);
        *createdModelIds = lbl_1_bss_41C[modelIndex].modelId[1];
        createdModelIds++;
        modelCount++;
        lbl_1_bss_41C[modelIndex].heightIncreasing = 0;
        lbl_1_bss_41C[modelIndex].heightPhase = 0.0f;
        lbl_1_bss_41C[modelIndex].yawIncreasing = 0;
        lbl_1_bss_41C[modelIndex].yawPhase = (90.0f);
    }
    lbl_1_bss_418 = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 54), HU_MEMNUM_OVL, HEAP_DVD));
    Hu3DModelCameraSet(lbl_1_bss_418, 1);
    Hu3DModelPosSet(lbl_1_bss_418, 0.0f, (5.0f), 0.0f);
    Hu3DModelRotSet(lbl_1_bss_418, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(lbl_1_bss_418, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_418, (0.30000001192092896f));
    Hu3DModelLayerSet(lbl_1_bss_418, 4);
    lbl_1_bss_404[0] = lbl_1_bss_404[1] = (0.0f);
    if (frandmod(2) != 0U) {
        horizontalDirection = 1;
    } else {
        horizontalDirection = -1;
    }
    lbl_1_bss_3FC[0] = horizontalDirection;
    if (frandmod(2) != 0U) {
        verticalDirection = 1;
    } else {
        verticalDirection = -1;
    }
    lbl_1_bss_3FC[1] = verticalDirection;
    lbl_1_bss_3F8 = 0;
    return modelCount;
}

/* Advances the raft sway and vertical bob once per active gameplay frame. */
void fn_1_5D3C(void) {
    if ((s32) lbl_1_bss_3F8 > 0) {
        lbl_1_bss_3F8 -= 1;
    }
    fn_1_5D90();
    fn_1_640C();
}

/* Called by fn_1_5D3C each gameplay frame to vary the raft's pitch and yaw. */
void fn_1_5D90(void)
{
    s32 modelIndex;
    f32 randomStep;
    f32 rotationA;
    f32 rotationB;
    f32 motionRate;
    f32 phaseRange;

    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        randomStep = (u32) (frandmod(5) + 1);
        randomStep /= (100.0f);
        if (lbl_1_bss_3FC[modelIndex] == 1) {
            lbl_1_bss_404[modelIndex] += randomStep;
            if (lbl_1_bss_404[modelIndex] > (1.0f)) {
                lbl_1_bss_404[modelIndex] = (1.0f);
                lbl_1_bss_3FC[modelIndex] = -1;
            }
        } else {
            lbl_1_bss_404[modelIndex] -= randomStep;
            if (lbl_1_bss_404[modelIndex] < (-1.0f)) {
                lbl_1_bss_404[modelIndex] = (-1.0f);
                lbl_1_bss_3FC[modelIndex] = 1;
            }
        }
        if (modelIndex == 0) {
            rotationA = lbl_1_bss_404[0];
        } else {
            rotationB = lbl_1_bss_404[1];
        }
    }

    for (modelIndex = 0; modelIndex < 1; modelIndex++) {
        if (lbl_1_bss_3F8 > 0) {
            lbl_1_bss_41C[modelIndex].targetRotation.x = (10.0f) * -rotationA;
            lbl_1_bss_41C[modelIndex].targetRotation.z = (10.0f) * -rotationB;
            lbl_1_bss_41C[modelIndex].rotation.x += (lbl_1_bss_41C[modelIndex].targetRotation.x -
                                                     lbl_1_bss_41C[modelIndex].rotation.x) /
                                                    (20.0f);
            lbl_1_bss_41C[modelIndex].rotation.z += (lbl_1_bss_41C[modelIndex].targetRotation.z -
                                                     lbl_1_bss_41C[modelIndex].rotation.z) /
                                                    (20.0f);
        } else {
            lbl_1_bss_41C[modelIndex].targetRotation.x = (5.0f) * -rotationA;
            lbl_1_bss_41C[modelIndex].targetRotation.z = (5.0f) * -rotationB;
            lbl_1_bss_41C[modelIndex].rotation.x += (lbl_1_bss_41C[modelIndex].targetRotation.x -
                                                     lbl_1_bss_41C[modelIndex].rotation.x) /
                                                    (40.0f);
            lbl_1_bss_41C[modelIndex].rotation.z += (lbl_1_bss_41C[modelIndex].targetRotation.z -
                                                     lbl_1_bss_41C[modelIndex].rotation.z) /
                                                    (40.0f);
        }

        motionRate = (120.0f);
        if (lbl_1_bss_41C[modelIndex].yawIncreasing != 0) {
            lbl_1_bss_41C[modelIndex].yawPhase += (180.0f) / motionRate;
            if (lbl_1_bss_41C[modelIndex].yawPhase >= (180.0f)) {
                lbl_1_bss_41C[modelIndex].yawPhase = (180.0f);
                lbl_1_bss_41C[modelIndex].yawIncreasing = 0;
            }
            phaseRange = (15.0f);
        } else {
            lbl_1_bss_41C[modelIndex].yawPhase -= (180.0f) / motionRate;
            if (lbl_1_bss_41C[modelIndex].yawPhase <= (-180.0f)) {
                lbl_1_bss_41C[modelIndex].yawPhase = (-180.0f);
                lbl_1_bss_41C[modelIndex].yawIncreasing = 1;
            }
            phaseRange = (-15.0f);
        }
        lbl_1_bss_41C[modelIndex].rotation.y +=
            (phaseRange / motionRate) *
            sin((lbl_1_bss_41C[modelIndex].yawPhase * (3.141592653589793)) / (180.0));
        Hu3DModelRotSetV(lbl_1_bss_41C[modelIndex].modelId[0], &lbl_1_bss_41C[modelIndex].rotation);
        Hu3DModelRotSetV(lbl_1_bss_41C[modelIndex].modelId[1], &lbl_1_bss_41C[modelIndex].rotation);
    }

    Hu3DModelRotSet(lbl_1_bss_418, (0.0f), lbl_1_bss_41C[0].rotation.y,
                    (0.0f));
}

/* Moves the raft models through their repeating vertical bob during gameplay and the results
 * lineup. */
void fn_1_640C(void) {
    f32 frameScale;
    s32 modelIndex;

    for (modelIndex = 0; modelIndex < 1; modelIndex++) {
        if (lbl_1_bss_3F8 > 0) {
            frameScale = (20.0f);
        } else {
            frameScale = (40.0f);
        }
        if (lbl_1_bss_41C[modelIndex].heightIncreasing != 0) {
            lbl_1_bss_41C[modelIndex].heightPhase += (90.0f) / frameScale;
            if (lbl_1_bss_41C[modelIndex].heightPhase >= (90.0f)) {
                lbl_1_bss_41C[modelIndex].heightPhase = (90.0f);
                lbl_1_bss_41C[modelIndex].heightIncreasing = 0;
            }
        } else {
            lbl_1_bss_41C[modelIndex].heightPhase -= (90.0f) / frameScale;
            if (lbl_1_bss_41C[modelIndex].heightPhase <= (-90.0f)) {
                lbl_1_bss_41C[modelIndex].heightPhase = (-90.0f);
                lbl_1_bss_41C[modelIndex].heightIncreasing = 1;
            }
        }
        if (lbl_1_bss_3F8 > 0) {
            lbl_1_bss_41C[modelIndex].position.y +=
                (0.800000011920929) *
                sin(((3.141592653589793) * lbl_1_bss_41C[modelIndex].heightPhase) /
                    (180.0));
        } else {
            lbl_1_bss_41C[modelIndex].position.y +=
                (0.4000000059604645) *
                sin(((3.141592653589793) * lbl_1_bss_41C[modelIndex].heightPhase) /
                    (180.0));
        }
        Hu3DModelPosSetV(lbl_1_bss_41C[modelIndex].modelId[0],
                         &lbl_1_bss_41C[modelIndex].position);
        Hu3DModelPosSetV(lbl_1_bss_41C[modelIndex].modelId[1],
                         &lbl_1_bss_41C[modelIndex].position);
    }
}

/* Called by fn_1_3E74 to create the timed course groups, attached parts, and effect models. */
void fn_1_6704(void)
{
    Mtx modelMatrix;
    HuVecF position;
    s32 modelIndex;
    s32 motionIndex;
    HU3D_MODELID modelId;
    s32 modelAssetNumber;
    s32 layerAssetNumber;
    s32 motionAssetNumber;
    char *objectName;
    s32 objectIndex;
    s32 positionIndex;

    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        if (lbl_1_bss_8E4 == 0) {
            modelAssetNumber = 27 + modelIndex;
        } else {
            modelAssetNumber = 30 + modelIndex;
        }
        lbl_1_bss_69E[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelAssetNumber + DATA_m664, HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DModelCameraSet(lbl_1_bss_69E[modelIndex], 1);
        Hu3DModelPosSetV(lbl_1_bss_69E[modelIndex], &lbl_1_bss_40C);
        Hu3DModelScaleSet(lbl_1_bss_69E[modelIndex], (10.0f),
                          (10.0f), (10.0f));
        Hu3DModelAttrSet(lbl_1_bss_69E[modelIndex], 1);
        if (modelIndex == 1) {
            Hu3DMotionSpeedSet(lbl_1_bss_69E[1], (0.0f));
            Hu3DMotionCalc(lbl_1_bss_69E[1]);
        }

        for (motionIndex = 0; motionIndex < 7; motionIndex++) {
            lbl_1_bss_674[modelIndex][motionIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 33) + modelIndex, HU_MEMNUM_OVL, HEAP_DVD));
            Hu3DModelCameraSet(lbl_1_bss_674[modelIndex][motionIndex], 1);
            Hu3DModelPosSetV(lbl_1_bss_674[modelIndex][motionIndex], &lbl_1_bss_40C);
            Hu3DMotionSpeedSet(lbl_1_bss_674[modelIndex][motionIndex], (0.0f));
            Hu3DMotionCalc(lbl_1_bss_674[modelIndex][motionIndex]);
            Hu3DModelScaleSet(lbl_1_bss_674[modelIndex][motionIndex], (10.0f),
                              (10.0f), (10.0f));
        }
    }

    for (modelIndex = 0; modelIndex < 7; modelIndex++) {
        if (lbl_1_bss_8E4 == 0) {
            layerAssetNumber = 36 + modelIndex;
        } else {
            layerAssetNumber = 44 + modelIndex;
        }
        lbl_1_bss_46C[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(layerAssetNumber + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_46C[modelIndex], 1);
        Hu3DModelObjMtxGet(lbl_1_bss_674[0][modelIndex], "sl0", modelMatrix);
        Hu3DMtxTransGet(modelMatrix, &lbl_1_bss_578[0][modelIndex]);
        Hu3DMtxRotGet(modelMatrix, &lbl_1_bss_47C[0][modelIndex]);
        Hu3DModelPosSetV(lbl_1_bss_46C[modelIndex], &lbl_1_bss_578[0][modelIndex]);
        Hu3DModelRotSetV(lbl_1_bss_46C[modelIndex], &lbl_1_bss_47C[0][modelIndex]);
        Hu3DModelAttrSet(lbl_1_bss_46C[modelIndex], HU3D_MOTATTR_LOOP);
        Hu3DModelScaleSet(lbl_1_bss_46C[modelIndex], (8.5f),
                          (8.5f), (8.5f));
        Hu3DModelAttrSet(lbl_1_bss_46C[modelIndex], 1);
        Hu3DModelLayerSet(lbl_1_bss_46C[modelIndex], 5);
        if (modelIndex >= 5) {
            if (lbl_1_bss_8E4 == 0) {
                motionAssetNumber = 43;
            } else {
                motionAssetNumber = 51;
            }
            lbl_1_bss_468[modelIndex - 5] = Hu3DJointMotion(
                lbl_1_bss_46C[modelIndex],
                HuDataSelHeapReadNum(motionAssetNumber + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSpeedSet(lbl_1_bss_46C[modelIndex], (0.0f));
        }
    }

    lbl_1_bss_464 = 0;
    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        lbl_1_bss_458[modelIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 56), HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DModelCameraSet(lbl_1_bss_458[modelIndex], 1);
        if (modelIndex == 0) {
            objectName = "he0";
        } else {
            objectName = "he1";
        }
        if (modelIndex == 0) {
            objectIndex = 0;
        } else {
            if (modelIndex == 1) {
                positionIndex = 5;
            } else {
                positionIndex = 6;
            }
            objectIndex = positionIndex;
        }
        Hu3DModelObjPosGet(lbl_1_bss_46C[objectIndex], objectName, &position);
        Hu3DModelPosSetV(lbl_1_bss_458[modelIndex], &position);
        Hu3DModelAttrSet(lbl_1_bss_458[modelIndex], 1);
        /* Each model overwrites the shared pair; later changes for models 1 and 2 reuse the final
         * pair loaded for model 2. */
        lbl_1_bss_454[0] = Hu3DJointMotion(
            lbl_1_bss_458[modelIndex],
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 57), HU_MEMNUM_OVL, HEAP_DVD));
        lbl_1_bss_454[1] = Hu3DJointMotion(
            lbl_1_bss_458[modelIndex],
            HuDataSelHeapReadNum(DATANUM(DATA_m664, 58), HU_MEMNUM_OVL, HEAP_DVD));
        Hu3DMotionSet(lbl_1_bss_458[modelIndex], lbl_1_bss_454[0]);
        Hu3DModelAttrSet(lbl_1_bss_458[modelIndex], HU3D_MOTATTR_LOOP);
    }
}

/* Called by fn_1_4060 to update timed course motions and falling props during scene playback. */
void fn_1_6D7C(void)
{
    Mtx matrix;
    Point3d target;
    Point3d position;
    Point3d delta;
    Point3d finalPosition;

    f32 angle;
    f32 motionSpeed;
    s32 item;
    s32 index;
    char *jointName;
    char *alternateJointName;
    Point3d *source;

    for (index = 0; index < 3; index++) {
        Hu3DModelPosSetV(lbl_1_bss_69E[index], &lbl_1_bss_40C);
        for (item = 0; item < 7; item++) {
            Hu3DModelPosSetV(lbl_1_bss_674[index][item], &lbl_1_bss_40C);
        }
    }
    if (lbl_1_bss_8C0 == 3587 || lbl_1_bss_8C0 == 3837) {
        lbl_1_bss_464++;
    }
    for (index = 0; index < 7; index++) {
        if (lbl_1_bss_464 == 0) {
            jointName = "sl0";
        } else {
            if (lbl_1_bss_464 == 1) {
                alternateJointName = "sl1";
            } else {
                alternateJointName = "sl2";
            }
            jointName = alternateJointName;
        }
        Hu3DModelObjMtxGet(lbl_1_bss_674[lbl_1_bss_464][index], jointName, matrix);
        Hu3DMtxTransGet(matrix, &((Point3d (*)[7])lbl_1_bss_578)[lbl_1_bss_464][index]);
        Hu3DMtxRotGet(matrix, &((Point3d (*)[7])lbl_1_bss_47C)[lbl_1_bss_464][index]);
        Hu3DModelPosSetV(lbl_1_bss_46C[index],
                         &((Point3d(*)[7]) lbl_1_bss_578)[lbl_1_bss_464][index]);
        Hu3DModelRotSetV(lbl_1_bss_46C[index],
                         &((Point3d(*)[7]) lbl_1_bss_47C)[lbl_1_bss_464][index]);
        /* During group 0's speed ramps, each of the seven parts changes the shared rate;
         * later parts can receive different speeds within the same update. */
        if (lbl_1_bss_464 == 0) {
            if (lbl_1_bss_8C0 >= 2958) {
                lbl_1_bss_460 = (0.0010000000474974513f) + lbl_1_bss_460;
                if (lbl_1_bss_460 > (0.20000000298023224f)) lbl_1_bss_460 = (0.20000000298023224f);
                Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][index], lbl_1_bss_460);
            } else if (lbl_1_bss_8C0 >= 2858) {
                lbl_1_bss_460 -= (0.0010000000474974513f);
                if (lbl_1_bss_460 < (0.11999999731779099f)) lbl_1_bss_460 = (0.11999999731779099f);
                Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][index], lbl_1_bss_460);
            } else {
                lbl_1_bss_460 = (0.20000000298023224f);
            }
        } else {
            if (lbl_1_bss_464 == 1) {
                motionSpeed = (0.5f);
            } else {
                motionSpeed = (0.11999999731779099f);
            }
            lbl_1_bss_460 = motionSpeed;
        }
    }
    if (lbl_1_bss_8C0 == 3338) Hu3DMotionSpeedSet(lbl_1_bss_69E[1], (1.0f));
    if (lbl_1_bss_8C0 == 3353) Hu3DMotionSpeedSet(lbl_1_bss_69E[1], (0.0f));
    if (lbl_1_bss_8C0 == 3618) Hu3DMotionSpeedSet(lbl_1_bss_69E[1], (1.0f));
    if (lbl_1_bss_8C0 == 3618) HuAudFXPlay(MSM_SE_M664_LATE_SCENE_CUE);
    if (lbl_1_bss_8C0 == 3633) HuAudFXPlay(MSM_SE_M664_LATE_SCENE_TRANSITION);
    switch (lbl_1_bss_8C0) {
    case 3838: case 3588: case 2638:
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][0], lbl_1_bss_460);
        fn_1_B910(2141, lbl_1_bss_46C[0], 1);
        break;
    case 3857: case 3595: case 2648:
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][1], lbl_1_bss_460);
        break;
    case 3602: case 3872: case 2657:
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][2], lbl_1_bss_460);
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][5], lbl_1_bss_460);
        break;
    case 3886: case 3609: case 2665:
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][3], lbl_1_bss_460);
        break;
    case 3902: case 3615: case 2674:
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][4], lbl_1_bss_460);
        Hu3DMotionSpeedSet(lbl_1_bss_674[lbl_1_bss_464][6], lbl_1_bss_460);
        break;
    }
    if (lbl_1_bss_8C0 >= 2638 && lbl_1_bss_8C0 < 4630 && lbl_1_bss_10.soundHandle[1] >= -1) {
        if (lbl_1_bss_8C0 == 2638)
            lbl_1_bss_10.soundHandle[1] = fn_1_B910(2134, lbl_1_bss_46C[0], 1);
        else lbl_1_bss_10.soundHandle[1] = fn_1_B910(-30634, lbl_1_bss_46C[0], 1);
    }
    if (lbl_1_bss_8C0 == 4630) {
        if (lbl_1_bss_10.soundHandle[1] > -1) {
            HuAudFXFadeOut(lbl_1_bss_10.soundHandle[1], 300);
            lbl_1_bss_10.soundHandle[1] = -1;
        }
        if (lbl_1_bss_10.soundHandle[0] != -1) {
            HuAudFXFadeOut(lbl_1_bss_10.soundHandle[0], 300);
            lbl_1_bss_10.soundHandle[0] = -1;
        }
    }
    for (index = 0; index < 2; index++) {
        Hu3DModelObjPosGet(lbl_1_bss_46C[index + 5], "bom", &position);
        { Point3d origin = {0.0f, 0.0f, 0.0f}; source = &origin; target = *source; }
        PSVECSubtract(&target, &position, &delta);
        /* the direction in degrees, wrapped into (-180, 180] */
        angle = (180.0) * (atan2(-delta.z, delta.x) / (3.141592653589793)) > (180.0)
                    ? (180.0) * (atan2(-delta.z, delta.x) / (3.141592653589793)) - (360.0)
                    : (180.0) * (atan2(-delta.z, delta.x) / (3.141592653589793)) <= (-180.0)
                          ? (360.0) + (180.0) * (atan2(-delta.z, delta.x) / (3.141592653589793))
                          : (180.0) * (atan2(-delta.z, delta.x) / (3.141592653589793));
        Hu3DModelRotSet(lbl_1_bss_46C[index + 5], (0.0f), angle, (0.0f));
    }
    switch (lbl_1_bss_8C0) {
    case M664_SCENE_FRAME_2828:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (2.0f)); break;
    case M664_SCENE_FRAME_2853:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (1.0f)); break;
    case M664_SCENE_FRAME_2978:
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[2], (2.0f)); break;
    case M664_SCENE_FRAME_3003:
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[2], (1.0f)); break;
    case M664_SCENE_FRAME_3648:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (2.0f)); break;
    case M664_SCENE_FRAME_3673:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (1.0f)); break;
    case M664_SCENE_FRAME_3653:
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[2], (2.0f)); break;
    case M664_SCENE_FRAME_3678:
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[2], (1.0f)); break;
    case M664_SCENE_FRAME_4378:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (2.0f)); break;
    case M664_SCENE_FRAME_4403:
        Hu3DMotionShiftSet(lbl_1_bss_458[1], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (1.0f)); break;
    case M664_SCENE_FRAME_4508:
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[1], (0.0f), (8.0f), 0);
        Hu3DMotionSpeedSet(lbl_1_bss_458[2], (2.0f)); break;
    case M664_SCENE_FRAME_4533:
        /* This event changes model 2's motion but resets model 1's playback speed. */
        Hu3DMotionShiftSet(lbl_1_bss_458[2], lbl_1_bss_454[0], (0.0f), (8.0f), HU3D_MOTATTR_LOOP);
        Hu3DMotionSpeedSet(lbl_1_bss_458[1], (1.0f)); break;
    }
    /* Reuse the last side's heading, turned by 90 degrees, for both side companions. */
    angle += (90.0f);
    for (index = 0; index < 3; index++) {
        Hu3DModelObjPosGet(lbl_1_bss_46C[index == 0 ? 0 : index == 1 ? 5 : 6],
                           index == 0 ? "he0" : "he1", &finalPosition);
        Hu3DModelPosSetV(lbl_1_bss_458[index], &finalPosition);
        if (index == 0)
            Hu3DModelRotSet(lbl_1_bss_458[index], (0.0f),
                            (90.0f) + ((Point3d(*)[7]) lbl_1_bss_47C)[lbl_1_bss_464][0].y, (0.0f));
        else Hu3DModelRotSet(lbl_1_bss_458[index], (0.0f), angle, (0.0f));
    }
}

/* Called by fn_1_3E74 to create course props and splash-effect models for both sides. */
void fn_1_7E24(void) {
    s32 sideIndex;
    s32 modelIndex;
    for (sideIndex = 0; sideIndex < 2; sideIndex++) {
        for (modelIndex = 0; modelIndex < 3; modelIndex++) {
            lbl_1_bss_248[sideIndex][modelIndex].modelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 55), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_248[sideIndex][modelIndex].modelId, 1);
            Hu3DModelShadowSet(lbl_1_bss_248[sideIndex][modelIndex].modelId);
            Hu3DModelPosSet(lbl_1_bss_248[sideIndex][modelIndex].modelId, (0.0f), (0.0f),
                            (0.0f));
            Hu3DModelScaleSet(lbl_1_bss_248[sideIndex][modelIndex].modelId, (8.0f), (8.0f),
                              (8.0f));
            Hu3DModelAttrSet(lbl_1_bss_248[sideIndex][modelIndex].modelId, 1);
            lbl_1_bss_248[sideIndex][modelIndex].state38 = 0;
            lbl_1_bss_248[sideIndex][modelIndex].state40 = 0;
            lbl_1_bss_248[sideIndex][modelIndex].destinationX = (u32) frandmod(400);
            if (frandmod(2) != 0U) {
                lbl_1_bss_248[sideIndex][modelIndex].destinationX *= (-1.0f);
            }
            lbl_1_bss_248[sideIndex][modelIndex].destinationY = (250.0f);
            lbl_1_bss_248[sideIndex][modelIndex].destinationZ = (u32) frandmod(400);
            if (frandmod(2) != 0U) {
                lbl_1_bss_248[sideIndex][modelIndex].destinationZ *= (-1.0f);
            }
            lbl_1_bss_248[sideIndex][modelIndex].effectModelId = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 68), HU_MEMNUM_OVL, HEAP_DVD));
            Hu3DModelCameraSet(lbl_1_bss_248[sideIndex][modelIndex].effectModelId, 1);
            Hu3DModelAttrSet(lbl_1_bss_248[sideIndex][modelIndex].effectModelId, 1);
            Hu3DModelScaleSet(lbl_1_bss_248[sideIndex][modelIndex].effectModelId, (3.5f),
                              (3.5f), (3.5f));
            Hu3DModelLayerSet(lbl_1_bss_248[sideIndex][modelIndex].effectModelId, 5);
            lbl_1_bss_248[sideIndex][modelIndex].effectMotionId = Hu3DJointMotion(
                lbl_1_bss_248[sideIndex][modelIndex].effectModelId,
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 69), HU_MEMNUM_OVL, HEAP_DVD));
        }
    }
}

/* Called by fn_1_4060 to animate falling course props, splashes, and player hit reactions. */
void fn_1_8244(void) {
    s32 effectSet;
    s32 side;
    s32 motionIndex;
    s32 playerIndex;
    s32 transitionFrame;
    f32 blend;
    Point3d effectPosition;

    effectSet = lbl_1_bss_464;
    side = -1;
    switch (lbl_1_bss_8C0) {
    case M664_SCENE_FRAME_2850:
        side = 0;
        break;
    case M664_SCENE_FRAME_3000:
        side = 1;
        break;
    case M664_SCENE_FRAME_3670:
        side = 0;
        break;
    case M664_SCENE_FRAME_3675:
        side = 1;
        break;
    case M664_SCENE_FRAME_4400:
        side = 0;
        break;
    case M664_SCENE_FRAME_4530:
        side = 1;
        break;
    }
    if (side != -1 && lbl_1_bss_248[side][effectSet].state38 == 0) {
        lbl_1_bss_248[side][effectSet].state38 = 1;
        lbl_1_bss_248[side][effectSet].motionFrame = 0;
        motionIndex = side == 0 ? 5 : 6;
        Hu3DMotionSet(lbl_1_bss_46C[motionIndex], lbl_1_bss_468[side]);
        Hu3DMotionSpeedSet(lbl_1_bss_46C[motionIndex], (1.0f));
    }

    for (side = 0; side < 2; side++) {
        motionIndex = side == 0 ? 5 : 6;
        if (lbl_1_bss_248[side][effectSet].state38 == 1) {
            transitionFrame = lbl_1_bss_464 == 1 ? 33 : 20;
            if (Hu3DMotionTimeGet(lbl_1_bss_46C[motionIndex]) < (f32) transitionFrame) {
                continue;
            }
            lbl_1_bss_248[side][effectSet].state38 = 2;
            Hu3DModelObjPosGet(lbl_1_bss_46C[motionIndex], "bom",
                               &lbl_1_bss_248[side][effectSet].startPosition);
        } else if (lbl_1_bss_248[side][effectSet].state38 == 2) {
            if (lbl_1_bss_248[side][effectSet].motionFrame == 0) {
                fn_1_B910(MSM_SE_M664_PROP_RELEASE, lbl_1_bss_46C[motionIndex], 1);
            }
            lbl_1_bss_248[side][effectSet].motionFrame++;
            blend = (f32) lbl_1_bss_248[side][effectSet].motionFrame / (120.0f);
            lbl_1_bss_248[side][effectSet].position.x =
                blend * lbl_1_bss_248[side][effectSet].destinationX +
                ((1.0f) - blend) * lbl_1_bss_248[side][effectSet].startPosition.x;
            lbl_1_bss_248[side][effectSet].position.y =
                (f32) ((f64) (blend * lbl_1_bss_248[side][effectSet].destinationY +
                              ((1.0f) - blend) * lbl_1_bss_248[side][effectSet].startPosition.y) +
                       (1200.0) * sin((3.141592653589793) * (f64) blend));
            lbl_1_bss_248[side][effectSet].position.z =
                blend * lbl_1_bss_248[side][effectSet].destinationZ +
                ((1.0f) - blend) * lbl_1_bss_248[side][effectSet].startPosition.z;
            Hu3DModelPosSetV(lbl_1_bss_248[side][effectSet].modelId,
                             &lbl_1_bss_248[side][effectSet].position);
            Hu3DModelAttrReset(lbl_1_bss_248[side][effectSet].modelId, 1);
            if (lbl_1_bss_248[side][effectSet].position.y <
                    lbl_1_bss_248[side][effectSet].destinationY - (10.0f) &&
                lbl_1_bss_248[side][effectSet].state40 == 0) {
                lbl_1_bss_248[side][effectSet].state40 =
                    lbl_1_bss_248[side][effectSet].destinationY > (0.0f) ? 1 : -1;
                if (lbl_1_bss_248[side][effectSet].state40 == 1) {
                    fn_1_B910(MSM_SE_M664_PROP_LANDING,
                              lbl_1_bss_248[side][effectSet].modelId, 1);
                } else {
                    fn_1_B910(MSM_SE_M664_PROP_MISS,
                              lbl_1_bss_248[side][effectSet].modelId, 1);
                }
            }
            if (lbl_1_bss_248[side][effectSet].state40 != 0) {
                effectPosition = lbl_1_bss_248[side][effectSet].position;
                Hu3DModelPosSetV(lbl_1_bss_248[side][effectSet].effectModelId, &effectPosition);
                Hu3DModelRotSet(lbl_1_bss_248[side][effectSet].effectModelId, (0.0f),
                                (f32) (u32) frandmod(360), (0.0f));
                Hu3DModelAttrReset(lbl_1_bss_248[side][effectSet].effectModelId, 1);
                Hu3DMotionSet(lbl_1_bss_248[side][effectSet].effectModelId,
                              lbl_1_bss_248[side][effectSet].effectMotionId);
                if (lbl_1_bss_248[side][effectSet].state40 == 1) {
                    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                        if ((lbl_1_bss_C8[playerIndex].player->actor->colGroundAttr & 0x407F) !=
                            0) {
                            if (lbl_1_bss_C8[playerIndex].actionState == 0) {
                                if (lbl_1_bss_C8[playerIndex].player->stunType == 0) {
                                    lbl_1_bss_C8[playerIndex].actionState = 1;
                                    lbl_1_bss_C8[playerIndex].actionTimer = 0;
                                    lbl_1_bss_C8[playerIndex].savedMotionId =
                                        lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0];
                                    lbl_1_bss_C8[playerIndex].player->omObj->mtnId[0] =
                                        lbl_1_bss_C8[playerIndex].motionIds[2];
                                }
                                omVibrate((s16) playerIndex, 20, 7, 3);
                            }
                            lbl_1_bss_3F8 = 180;
                        }
                    }
                    lbl_1_bss_248[side][effectSet].state38 = 3;
                } else if (lbl_1_bss_248[side][effectSet].state40 == -1) {
                    Hu3DModelAttrSet(lbl_1_bss_248[side][effectSet].modelId, 1);
                    Hu3DModelShadowReset(lbl_1_bss_248[side][effectSet].modelId);
                    lbl_1_bss_248[side][effectSet].state38 = 4;
                } else {
                    lbl_1_bss_248[side][effectSet].state38 = 3;
                    lbl_1_bss_248[side][effectSet].state40 = 1;
                }
            }
        } else if (lbl_1_bss_248[side][effectSet].state38 == 3) {
            if (lbl_1_bss_248[side][effectSet].state40 % 2 != 0) {
                Hu3DModelScaleSet(lbl_1_bss_248[side][effectSet].modelId, (10.0f), (10.0f),
                                  (10.0f));
            } else {
                Hu3DModelScaleSet(lbl_1_bss_248[side][effectSet].modelId, (8.0f), (8.0f), (8.0f));
            }
            if (++lbl_1_bss_248[side][effectSet].state40 == 5) {
                Hu3DModelAttrSet(lbl_1_bss_248[side][effectSet].modelId, 1);
                Hu3DModelShadowReset(lbl_1_bss_248[side][effectSet].modelId);
                lbl_1_bss_248[side][effectSet].state38 = 4;
            }
        }
        if ((79.0f) == Hu3DMotionTimeGet(lbl_1_bss_46C[motionIndex])) {
            Hu3DMotionSpeedSet(lbl_1_bss_46C[motionIndex], (0.0f));
        }
    }
}

/* Called by fn_1_3E74 to build the five hook assemblies, their falling nuts, and the animated hook
 * tip. */
void fn_1_8E44(void) {
    Mtx objectMatrix;
    Point3d objectPosition;
    Point3d objectRotation;
    s32 boardIndex;
    s32 objectIndex;
    s32 nameIndex;
    s32 modelNumber;
    s16 jointMotionId;

    lbl_1_bss_84E = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 62), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_84E, 1);
    Hu3DModelPosSetV(lbl_1_bss_84E, &lbl_1_bss_40C);
    Hu3DModelScaleSet(lbl_1_bss_84E, (10.0f),
                      (10.0f), (10.0f));

    for (boardIndex = 0; boardIndex < 5; boardIndex++) {
        if (lbl_1_bss_8E4 == 0) {
            modelNumber = 59;
        } else {
            modelNumber = 65;
        }
        lbl_1_bss_844[boardIndex] = Hu3DModelCreate(
            HuDataSelHeapReadNum(modelNumber + DATA_m664, HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelCameraSet(lbl_1_bss_844[boardIndex], 1);
        Hu3DModelObjMtxGet(lbl_1_bss_84E,
                           boardIndex == 0 ? "ut_hook0" :
                           boardIndex == 1 ? "ut_hook1" :
                           boardIndex == 2 ? "ut_hook2" :
                           boardIndex == 3 ? "ut_hook3" : "ut_hook4",
                           objectMatrix);
        Hu3DMtxTransGet(objectMatrix, &objectPosition);
        Hu3DMtxRotGet(objectMatrix, &objectRotation);
        Hu3DModelPosSetV(lbl_1_bss_844[boardIndex], &objectPosition);
        Hu3DModelRotSetV(lbl_1_bss_844[boardIndex], &objectRotation);
        Hu3DModelAttrSet(lbl_1_bss_844[boardIndex], HU3D_MOTATTR_LOOP);
        Hu3DMotionCalc(lbl_1_bss_844[boardIndex]);
        Hu3DMotionSpeedSet(lbl_1_bss_844[boardIndex], (0.0f));
        Hu3DModelScaleSet(lbl_1_bss_844[boardIndex], (10.0f),
                          (10.0f), (10.0f));
        Hu3DModelAttrSet(lbl_1_bss_844[boardIndex], 1);

        for (objectIndex = 0; objectIndex < 4; objectIndex++) {
            lbl_1_bss_81C[boardIndex][objectIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 60), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_81C[boardIndex][objectIndex], 1);
            Hu3DModelObjPosGet(lbl_1_bss_844[boardIndex],
                               objectIndex == 0 ? "nuts_hook0" :
                               objectIndex == 1 ? "nuts_hook1" :
                               objectIndex == 2 ? "nuts_hook2" : "nuts_hook3",
                               &objectPosition);
            Hu3DModelPosSetV(lbl_1_bss_81C[boardIndex][objectIndex],
                             &objectPosition);
            Hu3DModelScaleSet(lbl_1_bss_81C[boardIndex][objectIndex],
                              (2.0f), (2.0f),
                              (2.0f));
            Hu3DModelAttrSet(lbl_1_bss_81C[boardIndex][objectIndex], 1);
            Hu3DModelShadowSet(lbl_1_bss_81C[boardIndex][objectIndex]);
            lbl_1_bss_744[boardIndex][objectIndex] = frandmod(2);
            lbl_1_bss_744[boardIndex][objectIndex] *= frandmod(50);
            lbl_1_bss_6A4[boardIndex][objectIndex] = (0.0f);
            lbl_1_bss_6F4[boardIndex][objectIndex] = 0;
            lbl_1_bss_7CC[boardIndex][objectIndex][0] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 61), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_7CC[boardIndex][objectIndex][0], 1);
            Hu3DModelAttrSet(lbl_1_bss_7CC[boardIndex][objectIndex][0], 1);
            Hu3DMotionSpeedSet(lbl_1_bss_7CC[boardIndex][objectIndex][0],
                               (0.0f));
            Hu3DModelScaleSet(lbl_1_bss_7CC[boardIndex][objectIndex][0],
                              (2.0f), (2.0f),
                              (2.0f));
            Hu3DModelLayerSet(lbl_1_bss_7CC[boardIndex][objectIndex][0], 5);
            lbl_1_bss_7CC[boardIndex][objectIndex][1] = Hu3DModelCreate(
                HuDataSelHeapReadNum(DATANUM(DATA_m664, 66), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelCameraSet(lbl_1_bss_7CC[boardIndex][objectIndex][1], 1);
            Hu3DModelAttrSet(lbl_1_bss_7CC[boardIndex][objectIndex][1], 1);
            Hu3DMotionSpeedSet(lbl_1_bss_7CC[boardIndex][objectIndex][1],
                               (0.0f));
            Hu3DModelScaleSet(lbl_1_bss_7CC[boardIndex][objectIndex][1],
                              (2.0f), (2.0f),
                              (2.0f));
            Hu3DModelLayerSet(lbl_1_bss_7CC[boardIndex][objectIndex][1], 5);
        }

        nameIndex = frandmod(3);
        sprintf(lbl_1_bss_798[boardIndex], "%s",
                nameIndex == 0 ? "uk_hook" :
                nameIndex == 1 ? "uk_hook1" : "uk_hook2");
    }

    lbl_1_bss_7CA = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 63), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelCameraSet(lbl_1_bss_7CA, 1);
    Hu3DModelObjMtxGet(lbl_1_bss_844[0], lbl_1_bss_798[0], objectMatrix);
    Hu3DMtxTransGet(objectMatrix, &objectPosition);
    Hu3DMtxRotGet(objectMatrix, &objectRotation);
    Hu3DModelPosSetV(lbl_1_bss_7CA, &objectPosition);
    Hu3DModelRotSetV(lbl_1_bss_7CA, &objectRotation);
    Hu3DModelScaleSet(lbl_1_bss_7CA, (1.5f),
                      (1.5f), (1.5f));
    Hu3DModelAttrSet(lbl_1_bss_7CA, 1);
    jointMotionId = Hu3DJointMotion(
        lbl_1_bss_7CA,
        HuDataSelHeapReadNum(DATANUM(DATA_m664, 64), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSet(lbl_1_bss_7CA, jointMotionId);
    Hu3DModelAttrSet(lbl_1_bss_7CA, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(lbl_1_bss_7CA, (0.0f));
}

/* Called by fn_1_4060 each active frame to update hook and nut motion, collisions, and effects. */
void fn_1_9704(void) {
    Mtx objectMatrix;
    Point3d objectPosition;
    Point3d objectRotation;
    s32 pieceIndex;
    s32 boardIndex;
    s32 eventFrame;
    s32 boardChoice;

    if (lbl_1_bss_8C0 == 0) {
        for (boardIndex = 0; boardIndex < 5; boardIndex++) {
            Hu3DMotionSpeedSet(lbl_1_bss_844[boardIndex], (1.0f));
        }
        Hu3DMotionSpeedSet(lbl_1_bss_7CA, (1.0f));
        lbl_1_bss_794 = (0.0f);
    }

    for (pieceIndex = 0; pieceIndex < 4; pieceIndex++) {
        if (lbl_1_bss_8C0 >= 1650 && lbl_1_bss_744[0][pieceIndex] != 0) {
            eventFrame = lbl_1_bss_744[0][pieceIndex] + 1650;
            if (lbl_1_bss_8C0 >= eventFrame) {
                lbl_1_bss_6A4[0][pieceIndex] +=
                    (0.20000000298023224f) * (f32) (lbl_1_bss_8C0 - eventFrame);
            }
            if (lbl_1_bss_8C0 == eventFrame) {
                fn_1_B910(MSM_SE_M664_NUT_DROP, lbl_1_bss_81C[0][pieceIndex], 1);
            }
        }
        if (lbl_1_bss_8C0 >= 1780 && lbl_1_bss_744[1][pieceIndex] != 0) {
            eventFrame = lbl_1_bss_744[1][pieceIndex] + 1780;
            if (lbl_1_bss_8C0 >= eventFrame) {
                lbl_1_bss_6A4[1][pieceIndex] +=
                    (0.20000000298023224f) * (f32) (lbl_1_bss_8C0 - eventFrame);
            }
            if (lbl_1_bss_8C0 == eventFrame) {
                fn_1_B910(MSM_SE_M664_NUT_DROP, lbl_1_bss_81C[1][pieceIndex], 1);
            }
        }
        if (lbl_1_bss_8C0 >= 1860 && lbl_1_bss_744[2][pieceIndex] != 0) {
            eventFrame = lbl_1_bss_744[2][pieceIndex] + 1860;
            if (lbl_1_bss_8C0 >= eventFrame) {
                lbl_1_bss_6A4[2][pieceIndex] +=
                    (0.20000000298023224f) * (f32) (lbl_1_bss_8C0 - eventFrame);
            }
            if (lbl_1_bss_8C0 == eventFrame) {
                fn_1_B910(MSM_SE_M664_NUT_DROP, lbl_1_bss_81C[2][pieceIndex], 1);
            }
        }
        if (lbl_1_bss_8C0 >= 2020 && lbl_1_bss_744[3][pieceIndex] != 0) {
            eventFrame = lbl_1_bss_744[3][pieceIndex] + 2020;
            if (lbl_1_bss_8C0 >= eventFrame) {
                lbl_1_bss_6A4[3][pieceIndex] +=
                    (0.20000000298023224f) * (f32) (lbl_1_bss_8C0 - eventFrame);
            }
            if (lbl_1_bss_8C0 == eventFrame) {
                fn_1_B910(MSM_SE_M664_NUT_DROP, lbl_1_bss_81C[3][pieceIndex], 1);
            }
        }
        if (lbl_1_bss_8C0 >= 2115 && lbl_1_bss_744[4][pieceIndex] != 0) {
            eventFrame = lbl_1_bss_744[4][pieceIndex] + 2115;
            if (lbl_1_bss_8C0 >= eventFrame) {
                lbl_1_bss_6A4[4][pieceIndex] +=
                    (0.20000000298023224f) * (f32) (lbl_1_bss_8C0 - eventFrame);
            }
            if (lbl_1_bss_8C0 == eventFrame) {
                fn_1_B910(MSM_SE_M664_NUT_DROP, lbl_1_bss_81C[4][pieceIndex], 1);
            }
        }
    }

    Hu3DModelPosSetV(lbl_1_bss_84E, &lbl_1_bss_40C);
    for (boardIndex = 0; boardIndex < 5; boardIndex++) {
        Hu3DModelObjPosGet(lbl_1_bss_84E,
            boardIndex == 0 ? "ut_hook0" :
            boardIndex == 1 ? "ut_hook1" :
            boardIndex == 2 ? "ut_hook2" :
            boardIndex == 3 ? "ut_hook3" : "ut_hook4",
            &objectPosition);
        Hu3DModelPosSetV(lbl_1_bss_844[boardIndex], &objectPosition);

        for (pieceIndex = 0; pieceIndex < 4; pieceIndex++) {
            Hu3DModelObjPosGet(lbl_1_bss_844[boardIndex],
                pieceIndex == 0 ? "nuts_hook0" :
                pieceIndex == 1 ? "nuts_hook1" :
                pieceIndex == 2 ? "nuts_hook2" : "nuts_hook3",
                &objectPosition);
            objectPosition.y -= lbl_1_bss_6A4[boardIndex][pieceIndex];
            Hu3DModelPosSetV(lbl_1_bss_81C[boardIndex][pieceIndex], &objectPosition);

            if (lbl_1_bss_6F4[boardIndex][pieceIndex] == 0) {
                Point3d collisionStart;
                Point3d collisionEnd;
                /* The polygon query treats this single word's address as a full output record; only
                 * its success flag is used. */
                s32 hitPolygon;
                collisionStart = objectPosition;
                collisionEnd = objectPosition;
                collisionEnd.y -= (44.0f);
                if (collisionEnd.y > (50.0f) &&
                    MgActorColMapPolyGet(&collisionStart, &collisionEnd,
                                         -1U, (MGACTOR_COLMAP_POLY *)&hitPolygon) != 0) {
                    lbl_1_bss_6F4[boardIndex][pieceIndex] = 1;
                    fn_1_B910(MSM_SE_M664_NUT_IMPACT,
                              lbl_1_bss_81C[boardIndex][pieceIndex], 1);
                }
                if (objectPosition.y < (0.0f)) {
                    lbl_1_bss_6F4[boardIndex][pieceIndex] = -1;
                    fn_1_B910(MSM_SE_M664_PROP_MISS,
                              lbl_1_bss_81C[boardIndex][pieceIndex], 1);
                }
            }
            if (lbl_1_bss_6F4[boardIndex][pieceIndex] == 1) {
                Hu3DMotionSpeedSet(lbl_1_bss_7CC[boardIndex][pieceIndex][0], (1.0f));
                Hu3DModelAttrSet(lbl_1_bss_81C[boardIndex][pieceIndex], 1);
                Hu3DModelAttrReset(lbl_1_bss_7CC[boardIndex][pieceIndex][0], 1);
                Hu3DModelPosSetV(lbl_1_bss_7CC[boardIndex][pieceIndex][0], &objectPosition);
                lbl_1_bss_6F4[boardIndex][pieceIndex] = 2;
                Hu3DModelShadowReset(lbl_1_bss_81C[boardIndex][pieceIndex]);
            } else if (lbl_1_bss_6F4[boardIndex][pieceIndex] == 2 &&
                       Hu3DMotionEndCheck(lbl_1_bss_7CC[boardIndex][pieceIndex][0]) != 0) {
                Hu3DModelAttrSet(lbl_1_bss_7CC[boardIndex][pieceIndex][0], 1);
            }
            if (lbl_1_bss_6F4[boardIndex][pieceIndex] == -1) {
                Hu3DMotionSpeedSet(lbl_1_bss_7CC[boardIndex][pieceIndex][1], (1.0f));
                Hu3DModelAttrReset(lbl_1_bss_7CC[boardIndex][pieceIndex][1], 1);
                Hu3DModelPosSetV(lbl_1_bss_7CC[boardIndex][pieceIndex][1], &objectPosition);
                lbl_1_bss_6F4[boardIndex][pieceIndex] = -2;
                Hu3DModelShadowReset(lbl_1_bss_81C[boardIndex][pieceIndex]);
            } else if (lbl_1_bss_6F4[boardIndex][pieceIndex] == -2 &&
                       Hu3DMotionEndCheck(lbl_1_bss_7CC[boardIndex][pieceIndex][1]) != 0) {
                Hu3DModelAttrSet(lbl_1_bss_7CC[boardIndex][pieceIndex][1], 1);
                Hu3DModelAttrSet(lbl_1_bss_81C[boardIndex][pieceIndex], 1);
            }
        }
    }

    boardChoice = lbl_1_bss_8C0 < 1880 ? 0 :
                  lbl_1_bss_8C0 < 1980 ? 1 :
                  lbl_1_bss_8C0 < 2120 ? 2 :
                  lbl_1_bss_8C0 < 2210 ? 3 : 4;
    Hu3DModelObjMtxGet(lbl_1_bss_844[boardChoice], lbl_1_bss_798[boardChoice], objectMatrix);
    Hu3DMtxTransGet(objectMatrix, &objectPosition);
    eventFrame = (s32) Hu3DMotionTimeGet(lbl_1_bss_7CA);
    if (eventFrame >= 20 && eventFrame < 60) {
        lbl_1_bss_794 += (4.5f);
        objectPosition.y = (f32)((f64)objectPosition.y +
            (150.0) * sin(((3.141592653589793) * (f64)lbl_1_bss_794) / (180.0)));
    } else {
        lbl_1_bss_794 = (0.0f);
    }
    Hu3DMtxRotGet(objectMatrix, &objectRotation);
    Hu3DModelPosSetV(lbl_1_bss_7CA, &objectPosition);
    Hu3DModelRotSetV(lbl_1_bss_7CA, &objectRotation);
    if (lbl_1_bss_8C0 >= 1700 && lbl_1_bss_8C0 <= 2380 && eventFrame == 20) {
        fn_1_B910(MSM_SE_M664_HOOK_IMPACT, lbl_1_bss_7CA, 1);
    }
}

/* Called during active play, the Finish message and the results wipe to toggle models at scheduled
 * course frames. */
void fn_1_A428(void) {
    s32 playerIndex;
    s32 modelIndex;

    switch (lbl_1_bss_8C0) {
    case M664_SCENE_FRAME_250:
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            Hu3DModelAttrSet(lbl_1_bss_85C[playerIndex].modelId, 1);
            Hu3DModelAttrSet(lbl_1_bss_85C[playerIndex].objectModelId, 1);
        }
        Hu3DModelAttrSet(lbl_1_bss_85A, 1);
        break;
    case M664_SCENE_FRAME_1000:
        for (playerIndex = 0; playerIndex < 16; playerIndex++) {
            Hu3DModelAttrSet(lbl_1_bss_874[playerIndex], 1);
        }
        Hu3DModelAttrSet(lbl_1_bss_86C, 1);
        break;
    case M664_SCENE_FRAME_1350:
        Hu3DModelAttrReset(lbl_1_bss_8A0[1], 1);
        Hu3DModelAttrReset(lbl_1_bss_894[1], 1);
        break;
    case M664_SCENE_FRAME_1550:
        Hu3DModelAttrSet(lbl_1_bss_8A0[0], 1);
        Hu3DModelAttrSet(lbl_1_bss_894[0], 1);
        Hu3DModelAttrSet(lbl_1_bss_89C, 1);
        Hu3DModelAttrReset(lbl_1_bss_89E, 1);
        break;
    case M664_SCENE_FRAME_1700:
        Hu3DModelAttrReset(lbl_1_bss_7CA, 1);
        for (playerIndex = 0; playerIndex < 5; playerIndex++) {
            Hu3DModelAttrReset(lbl_1_bss_844[playerIndex], 1);
            for (modelIndex = 0; modelIndex < 4; modelIndex++) {
                Hu3DModelAttrReset(lbl_1_bss_81C[playerIndex][modelIndex], 1);
            }
        }
        break;
    case M664_SCENE_FRAME_2000:
        Hu3DModelAttrReset(lbl_1_bss_8A0[2], 1);
        Hu3DModelAttrReset(lbl_1_bss_894[2], 1);
        break;
    case M664_SCENE_FRAME_2500:
        Hu3DModelAttrSet(lbl_1_bss_89E, 1);
        for (playerIndex = 0; playerIndex < 5; playerIndex++) {
            Hu3DModelAttrSet(lbl_1_bss_844[playerIndex], 1);
            for (modelIndex = 0; modelIndex < 4; modelIndex++) {
                Hu3DModelAttrSet(lbl_1_bss_81C[playerIndex][modelIndex], 1);
                Hu3DModelAttrSet(lbl_1_bss_7CC[playerIndex][modelIndex][0], 1);
                Hu3DModelAttrSet(lbl_1_bss_7CC[playerIndex][modelIndex][1], 1);
            }
        }
        Hu3DModelAttrSet(lbl_1_bss_7CA, 1);
        Hu3DModelAttrReset(lbl_1_bss_69E[0], 1);
        for (playerIndex = 0; playerIndex < 7; playerIndex++) {
            Hu3DModelAttrReset(lbl_1_bss_46C[playerIndex], 1);
        }
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            Hu3DModelAttrReset(lbl_1_bss_458[playerIndex], 1);
        }
        break;
    case M664_SCENE_FRAME_3200:
        Hu3DModelAttrReset(lbl_1_bss_69E[1], 1);
        break;
    case M664_SCENE_FRAME_3300:
        Hu3DModelAttrReset(lbl_1_bss_8A0[3], 1);
        Hu3DModelAttrReset(lbl_1_bss_894[3], 1);
        break;
    case M664_SCENE_FRAME_3730:
        Hu3DModelAttrSet(lbl_1_bss_8A0[1], 1);
        Hu3DModelAttrSet(lbl_1_bss_894[1], 1);
        Hu3DModelAttrReset(lbl_1_bss_69E[2], 1);
        break;
    case M664_SCENE_FRAME_4000:
        Hu3DModelAttrSet(lbl_1_bss_8A0[2], 1);
        Hu3DModelAttrSet(lbl_1_bss_894[2], 1);
        Hu3DModelAttrSet(lbl_1_bss_69E[0], 1);
        break;
    case M664_SCENE_FRAME_3950:
        Hu3DModelAttrSet(lbl_1_bss_69E[1], 1);
        break;
    }
}

/* Called by fn_1_B74 after the results wipe completes; hides cutscene models before the results
 * lineup and camera are prepared. */
void fn_1_A8F0(void) {
    s32 playerIndex;
    s32 modelIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_C8[playerIndex].fallEffectModelId, 1);
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_8A0[playerIndex], 1);
        Hu3DModelAttrSet(lbl_1_bss_894[playerIndex], 1);
    }
    Hu3DModelAttrSet(lbl_1_bss_89E, 1);
    for (playerIndex = 0; playerIndex < 16; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_874[playerIndex], 1);
    }
    Hu3DModelAttrSet(lbl_1_bss_89C, 1);
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_870[playerIndex], 1);
    }
    Hu3DModelAttrSet(lbl_1_bss_86C, 1);
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_85C[playerIndex].modelId, 1);
        Hu3DModelAttrSet(lbl_1_bss_85C[playerIndex].objectModelId, 1);
    }
    Hu3DModelAttrSet(lbl_1_bss_85A, 1);
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_69E[playerIndex], 1);
    }
    for (playerIndex = 0; playerIndex < 7; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_46C[playerIndex], 1);
    }
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_458[playerIndex], 1);
    }
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        for (modelIndex = 0; modelIndex < 3; modelIndex++) {
            Hu3DModelAttrSet(lbl_1_bss_248[playerIndex][modelIndex].modelId, 1);
            Hu3DModelAttrSet(lbl_1_bss_248[playerIndex][modelIndex].effectModelId, 1);
        }
    }
    for (playerIndex = 0; playerIndex < 5; playerIndex++) {
        Hu3DModelAttrSet(lbl_1_bss_844[playerIndex], 1);
        for (modelIndex = 0; modelIndex < 4; modelIndex++) {
            Hu3DModelAttrSet(lbl_1_bss_81C[playerIndex][modelIndex], 1);
            Hu3DModelAttrSet(lbl_1_bss_7CC[playerIndex][modelIndex][0], 1);
            Hu3DModelAttrSet(lbl_1_bss_7CC[playerIndex][modelIndex][1], 1);
        }
    }
    Hu3DModelAttrSet(lbl_1_bss_7CA, 1);
}

/* Called by fn_1_2E08 for an AI-controlled player; selects an opponent, steers toward them, and
 * queues movement or an attack through that player's pad state. */
void fn_1_AC58(s32 playerIndex) {
    Point3d playerPosition;
    Point3d opponentPosition;
    Point3d direction;
    s32 setActionDelay;
    s32 skipPadUpdate;
    s32 opponentIndex;
    s32 randomValue;
    s32 actionStyle;
    f32 chaseAmount;
    f32 distance;

    if (lbl_1_bss_C8[playerIndex].sequenceCount != 0 ||
        lbl_1_bss_C8[playerIndex].actionState != 0) {
        MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, 0, 0, 0, 0);
        return;
    }
    fn_1_B8FC(&lbl_1_bss_18[playerIndex], 0, 0, 0, 0);
    playerPosition = lbl_1_bss_C8[playerIndex].player->actor->pos;
    playerPosition.y = (0.0f);
    setActionDelay = skipPadUpdate = 0;

    if ((lbl_1_bss_C8[playerIndex].player->actor->colGroundAttr & 0x407F) != 0) {
        if (lbl_1_bss_18[playerIndex].sessionState[1] != 0) {
            lbl_1_bss_18[playerIndex].sessionState[1]--;
            MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, 0, 0, 0, 0);
            return;
        }
        if (lbl_1_bss_C8[playerIndex].player->stunType != 0 &&
            lbl_1_bss_18[playerIndex].difficulty <= 1) {
            MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, 0, 0, 0, 0);
            return;
        }

        opponentPosition.x = opponentPosition.y = opponentPosition.z = (0.0f);
        PSVECSubtract(&opponentPosition, &playerPosition, &direction);
        distance = PSVECMag(&direction);
        if (lbl_1_bss_18[playerIndex].sessionState[3] != 0) {
            lbl_1_bss_18[playerIndex].sessionState[3] = 0;
        }

        if (lbl_1_bss_18[playerIndex].sessionState[0] == 0) {
            /* Prefer the first other player in a scripted hit reaction. If none qualifies,
             * retain any saved target and choose randomly only when the target is -1. */
            for (opponentIndex = 0; opponentIndex < 4; opponentIndex++) {
                if (opponentIndex == playerIndex ||
                    lbl_1_bss_C8[opponentIndex].sequenceCount != 0 ||
                    lbl_1_bss_C8[opponentIndex].actionState <= 0) {
                    continue;
                }
                lbl_1_bss_18[playerIndex].sessionState[2] = opponentIndex;
                break;
            }
            if (lbl_1_bss_18[playerIndex].sessionState[2] == -1) {
                for (opponentIndex = 0; opponentIndex < 4; opponentIndex++) {
                    if (opponentIndex == playerIndex ||
                        lbl_1_bss_C8[opponentIndex].sequenceCount != 0) {
                        continue;
                    }
                    if (frandmod(2) != 0U) {
                        lbl_1_bss_18[playerIndex].sessionState[2] = opponentIndex;
                        break;
                    } else if (lbl_1_bss_18[playerIndex].sessionState[2] <= -3) {
                        lbl_1_bss_18[playerIndex].sessionState[2] = opponentIndex;
                    } else if (opponentIndex == 3) {
                        opponentIndex = 0;
                        lbl_1_bss_18[playerIndex].sessionState[2]--;
                    }
                }
            }
        } else if (lbl_1_bss_18[playerIndex].sessionState[2] != -1) {
            opponentPosition =
                lbl_1_bss_C8[lbl_1_bss_18[playerIndex].sessionState[2]].player->actor->pos;
            opponentPosition.y = (0.0f);
            skipPadUpdate = 1;
            MgPlayerComStkOn(lbl_1_bss_C8[playerIndex].player);

            chaseAmount = lbl_1_bss_18[playerIndex].difficulty == 3 ? (10.0f) :
                          lbl_1_bss_18[playerIndex].difficulty == 2 ? (8.0f) :
                          lbl_1_bss_18[playerIndex].difficulty == 1 ? (6.0f) : (5.0f);
            if (MgPlayerVecChase(lbl_1_bss_C8[playerIndex].player, &opponentPosition, chaseAmount,
                                 (100.0f)) != 0) {
                MgPlayerComStkOff(lbl_1_bss_C8[playerIndex].player);
                skipPadUpdate = 0;
                if (lbl_1_bss_18[playerIndex].difficulty == 3) {
                    actionStyle = frandmod(2) != 0U ? 1 : 2;
                } else if (lbl_1_bss_18[playerIndex].difficulty == 2) {
                    actionStyle = frandmod(3) != 0U ? 2 : 1;
                } else if (lbl_1_bss_18[playerIndex].difficulty == 1) {
                    randomValue = frandmod(10);
                    actionStyle = randomValue > 4 ? 1 : (randomValue > 1 ? 2 : 0);
                } else {
                    randomValue = frandmod(10);
                    actionStyle = randomValue > 5 ? 1 : (randomValue > 1 ? 2 : 0);
                }
                opponentPosition =
                    lbl_1_bss_C8[lbl_1_bss_18[playerIndex].sessionState[2]].player->actor->pos;
                opponentPosition.y = (0.0f);
                PSVECSubtract(&opponentPosition, &playerPosition, &direction);
                if (actionStyle == 1) {
                    fn_1_B8FC(&lbl_1_bss_18[playerIndex], (s32) direction.x, (s32) -direction.z,
                              PAD_BUTTON_A, 0);
                    lbl_1_bss_18[playerIndex].sessionState[1] = 0;
                } else if (actionStyle == 2) {
                    fn_1_B8FC(&lbl_1_bss_18[playerIndex], (s32) direction.x, (s32) -direction.z,
                              PAD_BUTTON_B, 0);
                    setActionDelay = 1;
                } else {
                    lbl_1_bss_18[playerIndex].sessionState[2] = -1;
                    lbl_1_bss_18[playerIndex].sessionState[0] = 0;
                }
            }
        } else {
            lbl_1_bss_18[playerIndex].sessionState[0] = 0;
        }
    } else if (lbl_1_bss_18[playerIndex].sessionState[3] != 0) {
        opponentPosition.x = opponentPosition.y = opponentPosition.z = (0.0f);
        PSVECSubtract(&opponentPosition, &playerPosition, &direction);
        PSVECNormalize(&direction, &direction);
        PSVECScale(&direction, &direction, (50.0f));
        fn_1_B8FC(&lbl_1_bss_18[playerIndex], (s32) direction.x, (s32) -direction.z, 0,
                  PAD_BUTTON_A);
    } else if (lbl_1_bss_18[playerIndex].sessionState[2] != -1) {
        opponentPosition =
            lbl_1_bss_C8[lbl_1_bss_18[playerIndex].sessionState[2]].player->actor->pos;
        opponentPosition.y = (0.0f);
        PSVECSubtract(&opponentPosition, &playerPosition, &direction);
        PSVECNormalize(&direction, &direction);
        PSVECScale(&direction, &direction, (10.0f));
        if (++lbl_1_bss_18[playerIndex].sessionState[1] < 5) {
            fn_1_B8FC(&lbl_1_bss_18[playerIndex], (s32) direction.x, (s32) -direction.z, 0,
                      PAD_BUTTON_A);
        } else {
            if (lbl_1_bss_18[playerIndex].difficulty >= 2) {
                fn_1_B8FC(&lbl_1_bss_18[playerIndex], 0, 0, PAD_BUTTON_B, PAD_BUTTON_A);
            } else if (lbl_1_bss_18[playerIndex].difficulty == 1) {
                if (frandmod(3) == 0U) {
                    fn_1_B8FC(&lbl_1_bss_18[playerIndex], 0, 0, PAD_BUTTON_B, PAD_BUTTON_A);
                }
            } else if (frandmod(5) == 0U) {
                fn_1_B8FC(&lbl_1_bss_18[playerIndex], 0, 0, PAD_BUTTON_B, PAD_BUTTON_A);
            }
            setActionDelay = 1;
        }
    } else {
        lbl_1_bss_18[playerIndex].sessionState[0] = 0;
    }

    if (setActionDelay != 0) {
        lbl_1_bss_18[playerIndex].sessionState[1] = lbl_1_bss_18[playerIndex].difficulty == 3   ? 0
                                                    : lbl_1_bss_18[playerIndex].difficulty == 2 ? 60
                                                    : lbl_1_bss_18[playerIndex].difficulty == 1
                                                        ? 120
                                                        : 180;
    }
    /* Read opponent status before validating the index, including -1 when no opponent is
     * selected. */
    if (lbl_1_bss_C8[lbl_1_bss_18[playerIndex].sessionState[2]].sequenceCount != 0) {
        lbl_1_bss_18[playerIndex].sessionState[2] = -1;
        lbl_1_bss_18[playerIndex].sessionState[0] = 0;
    }
    if (lbl_1_bss_18[playerIndex].sessionState[2] > -1) {
        lbl_1_bss_18[playerIndex].sessionState[0]++;
        lbl_1_bss_18[playerIndex].sessionState[0] %= 300;
        if (lbl_1_bss_18[playerIndex].sessionState[0] == 0) {
            lbl_1_bss_18[playerIndex].sessionState[2] = -1;
        }
    }
    if (skipPadUpdate == 0) {
        MgPlayerPadSet(lbl_1_bss_C8[playerIndex].player, lbl_1_bss_18[playerIndex].sessionState[4],
                       lbl_1_bss_18[playerIndex].sessionState[5],
                       lbl_1_bss_18[playerIndex].sessionState[6],
                       lbl_1_bss_18[playerIndex].sessionState[7]);
    }
}

/* Called by fn_1_AC58 to store stick and button inputs for the next player pad update. */
void fn_1_B8FC(void *inputSettings, s32 stickX, s32 stickY, s32 buttonsDown, s32 buttonsHeld) {
    (*(s32 *)((s8 *)(inputSettings) + (M664_INPUT_STICK_X_OFFSET))) = stickX;
    (*(s32 *)((s8 *)(inputSettings) + (M664_INPUT_STICK_Y_OFFSET))) = stickY;
    (*(s32 *)((s8 *)(inputSettings) + (M664_INPUT_BUTTONS_DOWN_OFFSET))) = buttonsDown;
    (*(s32 *)((s8 *)(inputSettings) + (M664_INPUT_BUTTONS_HELD_OFFSET))) = buttonsHeld;
}

/* Plays or repans scene/player effects; offscreen exceptions clamp X and ignore Y.
 * Pan uses X for the nine named effects below and mirrored X for other IDs. */
s16 fn_1_B910(s16 soundEffect, s16 modelId, s16 cameraMask) {
    Point3d modelPosition;
    Point3d screenPosition;
    s16 soundHandle;
    f32 panAmount;
    f32 panScale;
    s32 pan;
    s32 panMinimum = M664_SOUND_PAN_MIN;
    s32 panMaximum = M664_SOUND_PAN_MAX;

    panScale = (576.0f) / (f32)(panMaximum - panMinimum);
    modelPosition = Hu3DData[modelId].pos;
    Hu3D3Dto2D(&modelPosition, cameraMask, &screenPosition);

    if (((soundEffect & 0x7FFF) == MSM_SE_M664_PROP_MISS) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_PLAYER_STUN) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_SCENE_EFFECT) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_SCENE_SOUND) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_HOOK_IMPACT) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_NUT_DROP)) {
        if (screenPosition.x < (0.0f)) {
            screenPosition.x = (0.0f);
        }
        if (screenPosition.x > (576.0)) {
            screenPosition.x = (576.0f);
        }
    } else {
        if ((screenPosition.x < (0.0f)) || (screenPosition.x > (576.0f))) {
            return -1;
        }
        if ((screenPosition.y < (0.0f)) || (screenPosition.y > (480.0f))) {
            return -1;
        }
    }

    if (((soundEffect & 0x7FFF) == MSM_SE_M664_PROP_MISS) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_PLAYER_STUN) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_SCENE_EFFECT) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_SCENE_SOUND) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_PROP_RELEASE) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_PROP_LANDING) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_HOOK_IMPACT) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_NUT_DROP) ||
        ((soundEffect & 0x7FFF) == MSM_SE_M664_NUT_IMPACT)) {
        panAmount = screenPosition.x;
    } else {
        panAmount = (576.0f) - screenPosition.x;
    }
    pan = panMinimum + (s32)(panAmount / panScale);

    if ((soundEffect & 0x8000) == 0) {
        soundHandle = HuAudFXPlay(soundEffect & 0x7FFF);
        if ((soundEffect & 0x7FFF) == MSM_SE_M664_SCENE_SOUND) {
            lbl_1_bss_8 = soundHandle;
        }
    } else {
        /* Any high-bit sound ID reuses the saved scene-sound handle, regardless of the requested
         * effect. */
        soundHandle = lbl_1_bss_8;
    }
    HuAudFXPanning(soundHandle, (s16)pan);
    return soundHandle;
}
