/* Boo'd Off the Stage module setup, player control, cameras and effects. */
#include <math.h>
#include "REL/m660dll/scene.h"
#include "REL/m660dll/camera_lighting.h"
#include "humath.h"
#include "game/hu3d.h"
#include "dolphin/mtx.h"
#include "game/printfunc.h"
#include "dolphin/types.h"
#include "game/hsfex.h"
#include "REL/m660dll/player_slots.h"
#include "game/main.h"
#include "game/mg/actman.h"
#include "dolphin/gx.h"
#include "game/object.h"
#include "game/audio.h"
#include "msm_se.h"
#include "game/charman.h"
#include "game/gamemes.h"
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
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
#include "REL/m660dll/particles.h"
#include "REL/particle_parameters.h"
#include "REL/m660dll/effects.h"
#include "dolphin/os.h"
#include "datanum/charmot.h"
#include <string.h>
#include "REL/m660dll.h"



void MgTimerDispOff(MGTIMER *timer);
void fn_1_1C1C(s16 cameraIndex);
void fn_1_1C50(void);
void fn_1_1E24(void);
void fn_1_2090(void);
void fn_1_21A0(void);
void fn_1_30A0(s16 controller, s16 minimum, s16 maximum);
void fn_1_3338(s16 controllerIndex, s16 subjectIndex);
s32 fn_1_3BBC(HU3D_MODELID modelId, HU3D_LIGHTID lightId);
s16 fn_1_57F8(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity,
              f32 initialScale, s16 state, const ParticleParameters *parameters);
void fn_1_59F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_5E70(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_64D0(void);
void fn_1_6614(void);
void fn_1_6774(s16 streamSelector);

extern s16 lbl_802C0278;
extern HuVecF lbl_80266A4C, lbl_80266A40;
void fn_1_176C(s16 cameraIndex);

typedef void (*VoidFunc)(void);
extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];
void ObjectSetup(void);

void fn_1_118(s16 mode, s16 frameNo);
void fn_1_26C(s16 mode, s16 frameNo);
void fn_1_298(s16 mode, s16 frameNo);
void fn_1_2C4(s16 mode, s16 frameNo);
void fn_1_2F0(s16 mode, s16 frameNo);
void fn_1_31C(s16 mode, s16 frameNo);
void fn_1_348(s16 mode, s16 frameNo);
void fn_1_374(s16 mode, s16 frameNo);
void fn_1_3A0(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_118, fn_1_26C, fn_1_298, fn_1_2C4, fn_1_2F0,
    fn_1_31C, fn_1_348, fn_1_374, fn_1_3A0
};

/* Runs module constructors and starts the minigame when the REL is loaded. */
int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    ObjectSetup();
    return 0;
}

/* Runs module destructors when the REL is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates the actor object and starts the minigame sequence during startup. */
void ObjectSetup(void) {
    lbl_1_bss_2678.objects = fn_80080418();
    fn_8003AB38(lbl_1_bss_2678.objects, 32730, 0U, 0U, -1, fn_8003D4FC);
    fn_80070788(&lbl_1_data_0);
    fn_80071CE8(2, 1000);
    fn_80071CE8(7, 1000);
}

/* Clears round state and initializes cameras, models and audio on sequence entry. */
void fn_1_118(s16 mode, s16 frameNo)
{
    s16 nightFlag;

    memset(&lbl_1_bss_2530, 0, sizeof(lbl_1_bss_2530));
    memset(&lbl_1_bss_24F4, 0, sizeof(lbl_1_bss_24F4));
    memset(&lbl_1_bss_24A4, 0, sizeof(lbl_1_bss_24A4));
    memset(&lbl_1_bss_2484, 0, sizeof(lbl_1_bss_2484));
    memset(lbl_1_bss_60C, 0, sizeof(lbl_1_bss_60C));
    memset(lbl_1_bss_544, 0, sizeof(lbl_1_bss_544));
    memset(lbl_1_bss_2DC, 0, sizeof(lbl_1_bss_2DC));
    memset(&lbl_1_bss_1F4, 0, sizeof(lbl_1_bss_1F4));
    memset(&lbl_1_bss_1E8, 0, sizeof(lbl_1_bss_1E8));
    memset(lbl_1_bss_80, 0, sizeof(lbl_1_bss_80));
    memset(&lbl_1_bss_38, 0, sizeof(lbl_1_bss_38));
    memset(lbl_1_bss_0, 0, sizeof(lbl_1_bss_0));
    nightFlag = lbl_802C0278;
    lbl_1_bss_1F4.unk70 = nightFlag;
    fn_1_176C(0);
    fn_1_2090();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_A3E4();
    fn_1_64D0();
    fn_80071A9C();
}

/* Advances the sequence when the opening scene completes. */
void fn_1_26C(s16 mode, s16 frameNo)
{
    if (fn_1_A9B0() != 0) fn_80071A9C();
}

/* Continues opening-scene updates during the next sequence phase. */
void fn_1_298(s16 mode, s16 frameNo)
{
    if (fn_1_A9F0() != 0) fn_80071A9C();
}

/* Runs a gameplay tick and advances the sequence when the round ends. */
void fn_1_2C4(s16 mode, s16 frameNo)
{
    if (fn_1_6F58() != 0) fn_80071A9C();
}

/* Fades the stream and updates the result scene; fn_1_D29C always returns zero. */
void fn_1_2F0(s16 mode, s16 frameNo)
{
    if (fn_1_D29C() != 0) fn_80071A9C();
}

/* Updates the result scene and advances when its controller signals completion. */
void fn_1_31C(s16 mode, s16 frameNo)
{
    if (fn_1_D2E8() != 0) fn_80071A9C();
}

/* Starts the result motions once, then updates the scene; fn_1_D328 always returns zero. */
void fn_1_348(s16 mode, s16 frameNo)
{
    if (fn_1_D328() != 0) fn_80071A9C();
}

/* Updates the result scene and the stage; fn_1_D57C always returns zero. */
void fn_1_374(s16 mode, s16 frameNo)
{
    if (fn_1_D57C() != 0) fn_80071A9C();
}

/* Stops all audio; fn_1_D5A4 always returns zero. */
void fn_1_3A0(s16 mode, s16 frameNo)
{
    if (fn_1_D5A4() != 0) fn_80071A9C();
}

/* An empty function. */
void fn_1_3CC(void)
{
}

/* Selects the first player in each team and initializes the two stage players. */
void fn_1_3D0(void)
{
    s16 playerLookup[2] = {-1, -1};
    Vec startingPositions[2] = {{-150.0f, 10.0f, 0.0f}, {150.0f, 10.0f, 0.0f}};
    Vec startingRotations[2] = {{0.0f, 180.0f, 0.0f}, {0.0f, 180.0f, 0.0f}};
    s16 player = 0;
    s16 group = 0;
    for (; player < 4; player++) {
        group = lbl_80265728[player].grpNo;

        if ((group == 0 || group == 1) &&
            playerLookup[group] == -1) {
            playerLookup[group] = player;
        }
    }

    for (player = 0; player < 2; player++) {
        lbl_1_bss_2DC[player].selectedPlayer =
            (s8)playerLookup[player];
        lbl_1_bss_2DC[player].playerIndex = (s8)player;
        lbl_1_bss_2DC[player].character =
            (s8)lbl_80265728[playerLookup[player]].charNo;
        lbl_1_bss_2DC[player].pad =
            (s8)lbl_80265728[playerLookup[player]].padNo;
        lbl_1_bss_2DC[player].unk18 =
            (s8)lbl_80265728[playerLookup[player]].grpNo;
        lbl_1_bss_2DC[player].type =
            (s8)lbl_80265728[playerLookup[player]].type;
        lbl_1_bss_2DC[player].difficulty =
            (s8)lbl_80265728[playerLookup[player]].comDif;
        lbl_1_bss_2DC[player].position =
            startingPositions[player];
        lbl_1_bss_2DC[player].rotation = startingRotations[player];
        lbl_1_bss_2DC[player].mode = -1;
    }
}

/* Samples controller input for the two stage players and all four controller slots. */
void fn_1_71C(void)
{
    s32 index;
    s32 stickResult;
    s32 deadZoneValue;

    index = 0;
    for (; index < 2; index++) {
        if (lbl_1_bss_2DC[index].type != 0 ||
            lbl_1_bss_2DC[index].inputDisabled != 0) {
            memset(&lbl_1_bss_2DC[index].input, 0, sizeof(lbl_1_bss_2DC[index].input));
        } else {
            lbl_1_bss_2DC[index].input.unk00 =
                lbl_802C0010[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk04 =
                lbl_802C0008[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk08 =
                lbl_802C0000[lbl_1_bss_2DC[index].pad];
            /* Only the nonpositive X branch applies the dead zone; positive X and Y pass
             * through. */
            if (lbl_802BFFFC[lbl_1_bss_2DC[index].pad] > 0) {
                stickResult = lbl_802BFFFC[lbl_1_bss_2DC[index].pad];
            } else {
                if ((s8)lbl_802BFFFC[lbl_1_bss_2DC[index].pad] * -1 < 10) {
                    deadZoneValue = 0;
                } else {
                    deadZoneValue = lbl_802BFFFC[lbl_1_bss_2DC[index].pad];
                }
                stickResult = deadZoneValue;
            }
            lbl_1_bss_2DC[index].input.unk0A = stickResult;
            lbl_1_bss_2DC[index].input.unk0B =
                (s8)lbl_802BFFF8[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk0C =
                (s8)lbl_802BFFF4[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk0D =
                (s8)lbl_802BFFF0[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk0E = lbl_802BFFEC[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk0F = lbl_802BFFE8[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk10 =
                lbl_802BFFE4[lbl_1_bss_2DC[index].pad];
            lbl_1_bss_2DC[index].input.unk11 =
                lbl_802BFFE0[lbl_1_bss_2DC[index].pad];
        }
    }

    index = 0;
    for (; index < 4; index++) {
        lbl_1_bss_1F4.samples[index].unk00 = lbl_802C0010[index];
        lbl_1_bss_1F4.samples[index].unk04 = lbl_802C0008[index];
        lbl_1_bss_1F4.samples[index].unk08 = lbl_802C0000[index];
        lbl_1_bss_1F4.samples[index].unk0A = (s8)lbl_802BFFFC[index];
        lbl_1_bss_1F4.samples[index].unk0B = (s8)lbl_802BFFF8[index];
        lbl_1_bss_1F4.samples[index].unk0C = (s8)lbl_802BFFF4[index];
        lbl_1_bss_1F4.samples[index].unk0D = (s8)lbl_802BFFF0[index];
        lbl_1_bss_1F4.samples[index].unk0E = lbl_802BFFEC[index];
        lbl_1_bss_1F4.samples[index].unk0F = lbl_802BFFE8[index];
        lbl_1_bss_1F4.samples[index].unk10 = lbl_802BFFE4[index];
        lbl_1_bss_1F4.samples[index].unk11 = lbl_802BFFE0[index];
    }
}

/* Applies stored position and rotation state to stage models during object updates. */
void fn_1_CDC(void)
{
    s32 modelIndex = 2;
    while (modelIndex < 75) {
        fn_80026760(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].position.x,
            lbl_1_bss_60C[modelIndex].position.y,
            lbl_1_bss_60C[modelIndex].position.z);
        fn_80026808(lbl_1_bss_60C[modelIndex].modelId,
            lbl_1_bss_60C[modelIndex].rotation.x,
            lbl_1_bss_60C[modelIndex].rotation.y,
            lbl_1_bss_60C[modelIndex].rotation.z);
        modelIndex++;
    }
    modelIndex = 0;
    while (modelIndex <= 1) {
        if (lbl_1_bss_2DC[modelIndex].state == 0 ||
            lbl_1_bss_2DC[modelIndex].state == 2) {
            fn_80026760(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].position.x,
                lbl_1_bss_2DC[modelIndex].position.y,
                lbl_1_bss_2DC[modelIndex].position.z);
            fn_80026808(lbl_1_bss_60C[modelIndex].modelId,
                lbl_1_bss_2DC[modelIndex].rotation.x,
                lbl_1_bss_2DC[modelIndex].rotation.y,
                lbl_1_bss_2DC[modelIndex].rotation.z);
        }
        modelIndex++;
    }
}

/* Loads the seven character motions used by each stage player during setup. */
void fn_1_EB8(void)
{
    u32 motionData[7] = {
        CHARMOT_HSF_c000m1_300,
        CHARMOT_HSF_c000m1_302,
        CHARMOT_HSF_c000m1_306,
        CHARMOT_HSF_c000m1_307,
        CHARMOT_HSF_c000m1_323,
        CHARMOT_HSF_c000m1_324,
        CHARMOT_HSF_c000m1_344,
    };
    s16 playerIndex;
    s16 motionIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        motionIndex = 0;
        while (motionIndex < 7) {
            lbl_1_bss_544[playerIndex].motionIds[motionIndex] =
                fn_8005B0B8(lbl_1_bss_2DC[playerIndex].character,
                                 motionData[motionIndex]);
            motionIndex++;
        }
        playerIndex++;
    }
}

/* Applies motion settings and handles motion shifts for both player models each update. */
void fn_1_FA8(void)
{
    s16 playerIndex = 0;

    while (playerIndex < 2) {
        if (lbl_1_bss_544[playerIndex].mode != lbl_1_bss_544[playerIndex].previousMode) {
            fn_8002C95C(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].motionIds[lbl_1_bss_544[playerIndex].mode]);
            lbl_1_bss_544[playerIndex].previousMode = lbl_1_bss_544[playerIndex].mode;
            lbl_1_bss_544[playerIndex].unk24 = fn_8002D530(
                lbl_1_bss_60C[playerIndex].modelId);
        }
        if (lbl_1_bss_544[playerIndex].speed != lbl_1_bss_544[playerIndex].previousSpeed) {
            fn_8002D86C(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].speed);
            lbl_1_bss_544[playerIndex].previousSpeed = lbl_1_bss_544[playerIndex].speed;
        }
        if (lbl_1_bss_544[playerIndex].attributes !=
            lbl_1_bss_544[playerIndex].previousAttributes) {
            fn_800269E8(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].attributes);
            lbl_1_bss_544[playerIndex].previousAttributes =
                lbl_1_bss_544[playerIndex].attributes;
        }
        if (lbl_1_bss_544[playerIndex].attributesToReset != 0) {
            fn_80026A44(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].attributesToReset);
            lbl_1_bss_544[playerIndex].attributesToReset =
                ~lbl_1_bss_544[playerIndex].attributesToReset;
            lbl_1_bss_544[playerIndex].previousAttributes =
                lbl_1_bss_544[playerIndex].attributes =
                    lbl_1_bss_544[playerIndex].attributes &
                    lbl_1_bss_544[playerIndex].attributesToReset;
            lbl_1_bss_544[playerIndex].attributesToReset = 0;
        }
        if (lbl_1_bss_544[playerIndex].time != lbl_1_bss_544[playerIndex].previousTime) {
            fn_8002D414(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].time);
        }

        switch (lbl_1_bss_544[playerIndex].shiftState) {
        case 0:
            break;
        case 1:
            fn_8002CCC0(lbl_1_bss_60C[playerIndex].modelId,
                lbl_1_bss_544[playerIndex].motionIds[lbl_1_bss_544[playerIndex].shiftMotion],
                lbl_1_bss_544[playerIndex].shiftStart, lbl_1_bss_544[playerIndex].shiftEnd,
                lbl_1_bss_544[playerIndex].shiftFlags);
            lbl_1_bss_544[playerIndex].unk5C = fn_8002D5D4(
                lbl_1_bss_60C[playerIndex].modelId);
            lbl_1_bss_544[playerIndex].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_544[playerIndex].shiftTime = fn_8002D508(
                lbl_1_bss_60C[playerIndex].modelId);
            if ((lbl_1_bss_544[playerIndex].shiftFlags & ((1 << 30) | (1 << 0))) == 0 &&
                lbl_1_bss_544[playerIndex].shiftTime >=
                    lbl_1_bss_544[playerIndex].shiftEnd - 1.0f) {
                lbl_1_bss_544[playerIndex].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_544[playerIndex].shiftState = 0;
            break;
        }

        lbl_1_bss_544[playerIndex].time = fn_8002D4E0(
            lbl_1_bss_60C[playerIndex].modelId);
        lbl_1_bss_544[playerIndex].previousTime = lbl_1_bss_544[playerIndex].time;
        lbl_1_bss_544[playerIndex].motionEnded = fn_8002D75C(
            lbl_1_bss_60C[playerIndex].modelId);
        if (lbl_1_bss_544[playerIndex].motionEnded != 0 &&
            lbl_1_bss_544[playerIndex].resetRequested != 0) {
            lbl_1_bss_544[playerIndex].motionEnded = 0;
            lbl_1_bss_544[playerIndex].resetRequested = 0;
            lbl_1_bss_544[playerIndex].mode = lbl_1_bss_544[playerIndex].resetMode;
            lbl_1_bss_544[playerIndex].time = lbl_1_bss_544[playerIndex].transition;
            lbl_1_bss_544[playerIndex].attributes =
                lbl_1_bss_544[playerIndex].resetAttributes;
            lbl_1_bss_544[playerIndex].attributesToReset =
                lbl_1_bss_544[playerIndex].resetAttributesToClear;
        }
        playerIndex++;
    }
}

/* Applies a camera preset and creates its viewport and perspective during setup. */
void fn_1_176C(s16 cameraIndex)
{
    f32 presets[4][13] = { { 0.0f, 101.0f, 61.0f, -78.0f, 0.0f, 0.0f, 1039.1f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 50.21f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f },
                                        { 0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f,
                                          0.0f, 0.0f, 640.0f, 480.0f, 45.0f } };
    s16 cameraIds[4] = {1, 2, 4, 8};

    lbl_1_bss_2530.records[cameraIndex].unk00 = cameraIndex;
    /* Copy the preset position and rotation into the selected record. */
    lbl_1_bss_2530.records[cameraIndex].unk1C[0] = presets[cameraIndex][0];
    lbl_1_bss_2530.records[cameraIndex].unk1C[1] = presets[cameraIndex][1];
    lbl_1_bss_2530.records[cameraIndex].unk1C[2] = presets[cameraIndex][2];
    lbl_1_bss_2530.records[cameraIndex].unk10[0] = presets[cameraIndex][3];
    lbl_1_bss_2530.records[cameraIndex].unk10[1] = presets[cameraIndex][4];
    lbl_1_bss_2530.records[cameraIndex].unk10[2] = presets[cameraIndex][5];
    lbl_1_bss_2530.records[cameraIndex].unk34 = presets[cameraIndex][6];
    lbl_1_bss_2530.records[cameraIndex].aspectRatio = presets[cameraIndex][7];
    lbl_1_bss_2530.records[cameraIndex].viewport[0] = presets[cameraIndex][8];
    lbl_1_bss_2530.records[cameraIndex].viewport[1] = presets[cameraIndex][9];
    lbl_1_bss_2530.records[cameraIndex].viewport[2] = presets[cameraIndex][10];
    lbl_1_bss_2530.records[cameraIndex].viewport[3] = presets[cameraIndex][11];
    lbl_1_bss_2530.records[cameraIndex].fieldOfView = presets[cameraIndex][12];

    Hu3DCameraCreate(cameraIds[cameraIndex]);
    Hu3DCameraViewportSet(cameraIds[cameraIndex],
                          lbl_1_bss_2530.records[cameraIndex].viewport[0],
                          lbl_1_bss_2530.records[cameraIndex].viewport[1],
                          lbl_1_bss_2530.records[cameraIndex].viewport[2],
                          lbl_1_bss_2530.records[cameraIndex].viewport[3],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds[cameraIndex],
                             lbl_1_bss_2530.records[cameraIndex].fieldOfView,
                             20.0f, 40000.0f,
                             lbl_1_bss_2530.records[cameraIndex].aspectRatio);
    /* Apply the same preset's position, rotation and zoom to the live camera. */
    lbl_80266A4C.x = presets[cameraIndex][0];
    lbl_80266A4C.y = presets[cameraIndex][1];
    lbl_80266A4C.z = presets[cameraIndex][2];
    lbl_80266A40.x = presets[cameraIndex][3];
    lbl_80266A40.y = presets[cameraIndex][4];
    lbl_80266A40.z = presets[cameraIndex][5];
    lbl_802C028C = presets[cameraIndex][6];
    fn_1_1C1C(cameraIndex);
}

s16 lbl_1_bss_2680[4];
M660ManagerState lbl_1_bss_2678;
M660CameraState lbl_1_bss_2530;
LightInitState lbl_1_bss_24F4;
MinigameShadowCamera lbl_1_bss_24A4;
M660ParticleModuleData lbl_1_bss_2484;
M660Model lbl_1_bss_60C[75];
MinigameParticipantMotionState lbl_1_bss_544[2];
M660Player lbl_1_bss_2DC[2];
/* Shared stage sequence, controller samples and audio state. */
M660StageState lbl_1_bss_1F4;
M660PlayerSlots lbl_1_bss_1E8;
M660Slot lbl_1_bss_80[9];
M660SceneLight lbl_1_bss_38;
/* Edge directions indexed by stage model and edge. */
s8 lbl_1_bss_0[14][4];

/* Copies the four camera masks locally. */
void fn_1_1C1C(s16 cameraIndex)
{
    s16 cameraMasks[4] = {1, 2, 4, 8};
}

/* Creates the directional stage light during scene initialization. */
void fn_1_1C50(void) {
    fn_800285BC();

    lbl_1_bss_24F4.position.x = 0.0f;
    lbl_1_bss_24F4.position.y = 0.0f;
    lbl_1_bss_24F4.position.z = 0.0f;
    lbl_1_bss_24F4.aim.x = 0.0f;
    lbl_1_bss_24F4.aim.y = 0.0f;
    lbl_1_bss_24F4.aim.z = 0.0f;
    lbl_1_bss_24F4.orientation.x = -70.0f;
    lbl_1_bss_24F4.orientation.y = 0.0f;
    lbl_1_bss_24F4.orientation.z = 0.0f;
    lbl_1_bss_24F4.range = 1000.0f;

    lbl_1_bss_24F4.color.r = 255;
    lbl_1_bss_24F4.color.g = 255;
    lbl_1_bss_24F4.color.b = 255;
    lbl_1_bss_24F4.color.a = 255;

    HuSubVecF(&lbl_1_bss_24F4.direction, &lbl_1_bss_24F4.aim, &lbl_1_bss_24F4.position);
    lbl_1_bss_24F4.id = fn_800286D0(
        &lbl_1_bss_24F4.position, &lbl_1_bss_24F4.direction,
        &lbl_1_bss_24F4.color);
    fn_1_1E24();
    Hu3DGLightStaticSet(lbl_1_bss_24F4.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_24F4.id);
}

/* Repositions the stage light from its target, range and orientation settings. */
void fn_1_1E24(void) {
    lbl_1_bss_24F4.position.x =
        lbl_1_bss_24F4.aim.x +
        lbl_1_bss_24F4.range *
            (cos(M_PI * lbl_1_bss_24F4.orientation.x /
                 180.0) *
             sin(M_PI * lbl_1_bss_24F4.orientation.y /
                 180.0));
    lbl_1_bss_24F4.position.y =
        lbl_1_bss_24F4.aim.y +
        lbl_1_bss_24F4.range *
            -sin(M_PI * lbl_1_bss_24F4.orientation.x /
                 180.0);
    lbl_1_bss_24F4.position.z =
        lbl_1_bss_24F4.aim.z +
        lbl_1_bss_24F4.range *
            (cos(M_PI * lbl_1_bss_24F4.orientation.x /
                 180.0) *
             cos(M_PI * lbl_1_bss_24F4.orientation.y /
                 180.0));
    HuSubVecF(&lbl_1_bss_24F4.direction, &lbl_1_bss_24F4.aim, &lbl_1_bss_24F4.position);
    VECNormalize(&lbl_1_bss_24F4.direction, &lbl_1_bss_24F4.direction);
    Hu3DGLightPosSetV(lbl_1_bss_24F4.id, &lbl_1_bss_24F4.position,
                      &lbl_1_bss_24F4.direction);
    Hu3DGLightColorSet(lbl_1_bss_24F4.id, lbl_1_bss_24F4.color.r,
                       lbl_1_bss_24F4.color.g, lbl_1_bss_24F4.color.b,
                       lbl_1_bss_24F4.color.a);
}

/* Initializes the scene shadow camera and its starting orbit during setup. */
void fn_1_2090(void)
{
    Hu3DShadowCreate(5.0f, 16000.0f, 21000.0f);
    Hu3DShadowSizeSet(480);
    lbl_1_bss_24A4.elevation = -90.0f;
    lbl_1_bss_24A4.azimuth = 0.0f;
    lbl_1_bss_24A4.unk14 = 0.0f;
    lbl_1_bss_24A4.target.x = 0.0f;
    lbl_1_bss_24A4.target.y = -100.0f;
    lbl_1_bss_24A4.target.z = 0.0f;
    lbl_1_bss_24A4.shadowOpacity = 0.2f;
    lbl_1_bss_24A4.orbitDistance = 21000.0f;
    fn_1_21A0();
}

/* Recalculates the shadow camera vectors and passes them to Hu3DShadowPosSet. */
void fn_1_21A0(void)
{
    lbl_1_bss_24A4.cameraPosition.x = (f32)((f64)lbl_1_bss_24A4.target.x +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0) *
          sin((M_PI * (f64)lbl_1_bss_24A4.azimuth) / 180.0))));
    lbl_1_bss_24A4.cameraPosition.y = (f32)((f64)lbl_1_bss_24A4.target.y +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         -sin((M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0)));
    lbl_1_bss_24A4.cameraPosition.z = (f32)((f64)lbl_1_bss_24A4.target.z +
        ((f64)lbl_1_bss_24A4.orbitDistance *
         (cos((M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0) *
          cos((M_PI * (f64)lbl_1_bss_24A4.azimuth) / 180.0))));

    lbl_1_bss_24A4.cameraUp.x = (f32)(
        sin((M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0) *
        sin((M_PI * (f64)lbl_1_bss_24A4.azimuth) / 180.0));
    lbl_1_bss_24A4.cameraUp.y = (f32)cos(
        (M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0);
    lbl_1_bss_24A4.cameraUp.z = (f32)(
        sin((M_PI * (f64)lbl_1_bss_24A4.elevation) / 180.0) *
        cos((M_PI * (f64)lbl_1_bss_24A4.azimuth) / 180.0));

    Hu3DShadowTPLvlSet(lbl_1_bss_24A4.shadowOpacity);
    Hu3DShadowPosSet(&lbl_1_bss_24A4.cameraPosition, &lbl_1_bss_24A4.cameraUp,
                     &lbl_1_bss_24A4.target);
}

/* Synchronizes player and prop transforms and motions, then refreshes the shadow camera each
 * frame. */
void fn_1_24C4(void)
{
    Vec playerPosition;
    s16 playerIndex;

    fontcolor = 15;
    playerIndex = 0;
    while (playerIndex < 2) {
        playerPosition.x = lbl_1_bss_2DC[playerIndex].position.x;
        playerPosition.y = lbl_1_bss_2DC[playerIndex].position.y;
        playerPosition.z = lbl_1_bss_2DC[playerIndex].position.z;
        fn_80086B04(lbl_1_bss_1E8.players[playerIndex]->actor,
                         &playerPosition);
        lbl_1_bss_1E8.players[playerIndex]->actor->rotY =
            lbl_1_bss_2DC[playerIndex].rotation.y;
        lbl_1_bss_60C[playerIndex].position =
            lbl_1_bss_2DC[playerIndex].position;
        lbl_1_bss_60C[playerIndex].rotation =
            lbl_1_bss_2DC[playerIndex].rotation;
        playerIndex++;
    }

    fn_1_CDC();

    fn_1_FA8();
    fn_1_1C1C(0);
    fn_1_21A0();
}

/* Runs actor movement, retaining player positions and collision partners per frame. */
void fn_1_2B34(void) {
    Point3d position;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_2DC[playerIndex].state != 0) {
            if ((s8) lbl_1_bss_2DC[playerIndex].inputDisabled != 0) {
                fn_80080520(lbl_1_bss_1E8.players[playerIndex], 0, 0, 0, 0);
            } else if ((s8) lbl_1_bss_2DC[playerIndex].type != 0) {
                fn_80080520(lbl_1_bss_1E8.players[playerIndex],
                               lbl_1_bss_2DC[playerIndex].stickXWork,
                               lbl_1_bss_2DC[playerIndex].stickYWork,
                               lbl_1_bss_2DC[playerIndex].pressedButtonsWork,
                               lbl_1_bss_2DC[playerIndex].buttonsWork);
            }
            fn_80086BC8(lbl_1_bss_1E8.players[playerIndex]->actor, &position);
            lbl_1_bss_2DC[playerIndex].previousPosition.x = position.x;
            lbl_1_bss_2DC[playerIndex].previousPosition.y = position.y;
            lbl_1_bss_2DC[playerIndex].previousPosition.z = position.z;
            lbl_1_bss_2DC[playerIndex].unk5C = lbl_1_bss_2DC[playerIndex].storedMesh;
            lbl_1_bss_2DC[playerIndex].storedMesh = -1;
            if ((lbl_1_bss_2DC[playerIndex].previousPartner !=
                 lbl_1_bss_2DC[playerIndex].partnerIndex) ||
                (lbl_1_bss_2DC[playerIndex].partnerIndex == -1)) {
                lbl_1_bss_2DC[playerIndex].collisionCount = 0;
            }
            lbl_1_bss_2DC[playerIndex].previousPartner = lbl_1_bss_2DC[playerIndex].partnerIndex;
            lbl_1_bss_2DC[playerIndex].partnerIndex = -1;
        }
        playerIndex += 1;
    }
    fn_800856A8();
    playerIndex = 0;
    while (playerIndex < 2) {
        if ((s8) lbl_1_bss_2DC[playerIndex].state != 0) {
            fn_80086BC8(lbl_1_bss_1E8.players[playerIndex]->actor, &position);
            lbl_1_bss_2DC[playerIndex].position.x = position.x;
            lbl_1_bss_2DC[playerIndex].position.y = position.y;
            lbl_1_bss_2DC[playerIndex].position.z = position.z;
            lbl_1_bss_2DC[playerIndex].rotation.y =
                (f32) lbl_1_bss_1E8.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

/* Advances by one step of the requested count. Returns the truncated remaining-step
 * estimate plus one, or zero when the endpoint test snaps the value to the end. */
s16 fn_1_2E80(f32 *value, f32 stepCount, f32 start, f32 end)
{
    f32 startDistance;
    f32 currentDistance;
    s16 estimate = 0;
    f32 increment = (end - start) / stepCount;

    *value += increment;
    if (increment) {
        estimate = (s16)((end - *value) / increment);
    }
    estimate++;
    if (start - end > 0.0f) {
        startDistance = start - end;
    } else {
        startDistance = start - -1.0f * end;
    }
    increment = startDistance;
    if (start - *value > 0.0f) {
        currentDistance = start - *value;
    } else {
        currentDistance = start - -1.0f * *value;
    }
    start = currentDistance;
    if (increment <= start) {
        *value = end;
        estimate = 0;
    }
    return estimate;
}

/* Creates and positions a timer, converting its duration and end time from seconds to frames. */
MGTIMER *fn_1_2FA0(int type,int duration,int end,float x,float y) {
    MGTIMER *timer=MgTimerCreate(type);
    MgTimerParamSet(timer,duration*60,end*60,0);
    MgTimerPosSet(timer,x,y);
    return timer;
}

/* Starts the timer. */
void fn_1_3014(MGTIMER *timer) {
    MgTimerModeOnSet(timer, 1);
}

/* Returns the timer completion check used by the gameplay sequence. */
s32 fn_1_3040(MGTIMER *timer) {
    return MgTimerDoneCheck(timer);
}

/* Hides the timer display, then kills the timer and releases its resources. */
void fn_1_3068(MGTIMER *timer) {
    MgTimerDispOff(timer);
    MgTimerKill(timer);
}

/* Changes the selected object with controller input and prints selected transforms. */
void fn_1_30A0(s16 controller, s16 minimum, s16 maximum)
{
    s16 player;

    if (lbl_802BFFEC[controller] > 50) {
        if (lbl_802C0008[controller] & PAD_BUTTON_X) {
            lbl_1_bss_1F4.selectedObject++;
        }
        if (lbl_802C0008[controller] & PAD_BUTTON_Y) {
            lbl_1_bss_1F4.selectedObject--;
        }
    }

    if (lbl_1_bss_1F4.selectedObject < minimum) {
        lbl_1_bss_1F4.selectedObject = maximum;
    }
    if (lbl_1_bss_1F4.selectedObject > maximum) {
        lbl_1_bss_1F4.selectedObject = minimum;
    }
    fn_1_3338(controller, lbl_1_bss_1F4.selectedObject);

    if ((lbl_802C0008[controller] & PAD_BUTTON_X) &&
        (lbl_802C0008[controller] & PAD_BUTTON_Y)) {
        for (player = minimum; player <= maximum; player++) {
            if (player >= 0 && player <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n", player,
                       lbl_1_bss_2DC[player].position.x,
                       lbl_1_bss_2DC[player].position.y,
                       lbl_1_bss_2DC[player].position.z,
                       lbl_1_bss_2DC[player].rotation.y);
            } else {
                /* The original model dump passes an index absent from its format. */
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n", player,
                       lbl_1_bss_60C[player].position.x,
                       lbl_1_bss_60C[player].position.y,
                       lbl_1_bss_60C[player].position.z,
                       lbl_1_bss_60C[player].rotation.y);
            }
        }
    }
}

/* Moves or rotates the selected player or prop from controller input. */
void fn_1_3338(s16 controllerIndex, s16 subjectIndex)
{
    f32 movementStep = 5.0f;
    s16 textX = 25;
    s16 textY = 430;

    if (subjectIndex >= 0 && subjectIndex <= 1) {
        if ((lbl_802C0000[controllerIndex] & PAD_BUTTON_X) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.z += movementStep;
            }
        } else if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_X) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 10) {
                lbl_1_bss_2DC[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -10) {
                lbl_1_bss_2DC[subjectIndex].position.z += movementStep;
            }
        }
        if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_A) != 0) {
            lbl_1_bss_2DC[subjectIndex].rotation.y += 90.0f;
        }
        if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_B) != 0) {
            lbl_1_bss_2DC[subjectIndex].rotation.y -= 90.0f;
        }
        print8(textX, textY, 1.5f,
               "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               subjectIndex,
               lbl_1_bss_2DC[subjectIndex].position.x,
               lbl_1_bss_2DC[subjectIndex].position.y,
               lbl_1_bss_2DC[subjectIndex].position.z,
               lbl_1_bss_2DC[subjectIndex].rotation.y);
        return;
    }

    if ((lbl_802C0000[controllerIndex] & PAD_BUTTON_X) != 0) {
        if (lbl_802BFFFC[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.x += movementStep;
        }
        if (lbl_802BFFFC[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.x -= movementStep;
        }
        if (lbl_802BFFF8[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.z -= movementStep;
        }
        if (lbl_802BFFF8[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.z += movementStep;
        }
    } else if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_X) != 0) {
        if (lbl_802BFFFC[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.x += movementStep;
        }
        if (lbl_802BFFFC[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.x -= movementStep;
        }
        if (lbl_802BFFF8[controllerIndex] > 10) {
            lbl_1_bss_60C[subjectIndex].position.z -= movementStep;
        }
        if (lbl_802BFFF8[controllerIndex] < -10) {
            lbl_1_bss_60C[subjectIndex].position.z += movementStep;
        }
    }
    if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_A) != 0) {
        lbl_1_bss_60C[subjectIndex].rotation.y += 90.0f;
    }
    if ((lbl_802C0008[controllerIndex] & PAD_BUTTON_B) != 0) {
        lbl_1_bss_60C[subjectIndex].rotation.y -= 90.0f;
    }
    print8(textX, textY, 1.5f,
           "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
           subjectIndex,
           lbl_1_bss_60C[subjectIndex].position.x,
           lbl_1_bss_60C[subjectIndex].position.y,
           lbl_1_bss_60C[subjectIndex].position.z,
           lbl_1_bss_60C[subjectIndex].rotation.y);
}

/* Adds a local light to a model unless it is already attached or the slots are full. */
s16 fn_1_3AA4(s16 modelIndex, s16 lightId)
{
    HU3D_MODEL *model;
    s16 attachResult;
    s16 lightSlot;

    attachResult = 0;
    model = &lbl_802C01C8[modelIndex];
    lightSlot = 0;
    while (lightSlot < 8) {
        if (model->LLightId[lightSlot] == lightId) {
            attachResult = -1;
            break;
        }
        lightSlot++;
    }

    if (attachResult != -1) {
        lightSlot = 0;
        while (lightSlot < 8) {
            if (model->LLightId[lightSlot] == -1) {
                model->LLightId[lightSlot] = lightId;
                model->attr |= (1 << 12);
                attachResult = 1;
                break;
            }
            lightSlot++;
        }
    }

    if (attachResult == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", modelIndex, lightId);
        attachResult = -1;
    }

    return attachResult;
}

/* Removes a light ID from the model's local-light list, returning 1 when it is present. */
s32 fn_1_3BBC(HU3D_MODELID modelId, HU3D_LIGHTID lightId) {
    HU3D_MODEL *model;
    s32 removeResult;
    s16 lightSlot;

    removeResult = -1;
    model = &lbl_802C01C8[modelId];
    lightSlot = 0;
    while (lightSlot < 8) {
        if (model->LLightId[lightSlot] == lightId) {
            model->LLightId[lightSlot] = -1;
            removeResult = 1;
            break;
        }
        lightSlot++;
    }
    return removeResult;
}

/* Returns a random integer below the positive range as a float for effect choices. */
f32 fn_1_3C54(f32 range)
{
    s32 randomValue;

    randomValue = (s32)range;
    randomValue = frandmod(randomValue);
    range = (f32)randomValue;
    return range;
}

/* Loads particle resources and initializes the four particle effect models at setup. */
void fn_1_3CCC(void)
{
    u32 dataNumbers[4] = { DATANUM(0x770000, 36), DATANUM(0x770000, 37),
                           DATANUM(0x770000, 37), DATANUM(0x770000, 37) };
    s16 maximumParticles[4] = {100, 500, 100, 1};
    s16 layers[4] = {0, 6, 1, 5};
    s16 cameras[4] = {1, 1, 1, 1};
    s16 blendModes[4] = {1, 0, 0, 0};
    HU3D_PARTICLE_HOOK hooks[4] = {fn_1_59F8, fn_1_59F8, fn_1_59F8, fn_1_5E70};
    s16 effectIndex = 0;

    while (effectIndex < 4) {
        ANIMDATA *animation;
        HU3D_PARTICLE *context;
        HU3D_PARTICLE_DATA *entry;
        s16 particleIndex;

        lbl_1_bss_2484.unk08[effectIndex] =
            maximumParticles[effectIndex];
        animation = HuSprAnimDataRead(dataNumbers[effectIndex]);
        lbl_1_bss_2484.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, maximumParticles[effectIndex] * sizeof(ParticleParameters),
            0x10000000);
        lbl_1_bss_2484.modelIds[effectIndex] = fn_8003216C(
            animation, maximumParticles[effectIndex]);
        fn_80026B8C(lbl_1_bss_2484.modelIds[effectIndex],
                          layers[effectIndex]);
        fn_800326D0(lbl_1_bss_2484.modelIds[effectIndex],
                            hooks[effectIndex]);
        fn_80026B64(lbl_1_bss_2484.modelIds[effectIndex],
                           cameras[effectIndex]);
        fn_8003269C(lbl_1_bss_2484.modelIds[effectIndex],
                                  (u8)blendModes[effectIndex]);

        context = lbl_802C01C8[lbl_1_bss_2484.modelIds[effectIndex]].hookData;
        context->emitCnt = 0;
        context->work = lbl_1_bss_2484.allocation[effectIndex];
        context->count = 0;
        particleIndex = 0;
        entry = context->data;
        while (particleIndex < context->maxCnt) {
            entry->scale = 0.0f;
            entry->time = 0;
            particleIndex++;
            entry++;
        }
        effectIndex++;
    }
}

/* Emits a small randomized particle effect at the supplied position. */
void fn_1_4004(Vec *origin)
{
    Vec velocity;
    Vec position;
    ParticleParameters parameters;
    f32 initialScale;

    parameters.unk00 = 0;
    parameters.startColor.r = 255;
    parameters.startColor.g = 255;
    parameters.startColor.b = 255;
    parameters.startColor.a = 255;
    parameters.targetColor.r = 0;
    parameters.targetColor.g = 0;
    parameters.targetColor.b = 0;
    parameters.targetColor.a = 0;
    parameters.rotationIncrement = 0.0f;
    parameters.alphaIncrement = 0.01f;
    parameters.colorBlendStep = 0.01f;
    parameters.verticalAcceleration = 0.0f;

    position.x = origin->x;
    position.y = origin->y;
    position.z = origin->z;
    initialScale = 30.0f;
    parameters.scaleIncrement = -0.1f;
    parameters.scaleIncrementMultiplier = 1.2f;
    parameters.velocityScale[0] = 1.05f;
    parameters.velocityScale[1] = 0.95f;
    parameters.velocityScale[2] = 1.0f;
    velocity.x = 0.4f * (fn_1_3C54(3.0f) - 1.0f);
    velocity.y = 3.0f + fn_1_3C54(4.0f);
    velocity.z = 0.0f;

    fn_1_57F8(0, -1, position, velocity, initialScale, 20, &parameters);
}

/* Emits randomized particles for the selected effect around the model position. The origin draw
* is overwritten with zero, so origin 0 is always used while the draw still advances the RNG. */
void fn_1_42E0(M660Model *origin, s16 effectIndex)
{
    M660ParticleEffectConfig configs[3] = {
    {3, 160, -8.0f, 0.0f, 80.0f, 450.0f, 190.0f, 128, 0, 30.0f, 140.0f, 20.0f, 100.0f},
    {3, 160, -5.0f, 20.0f, 120.0f, 450.0f, 400.0f, 128, 0, 70.0f, 280.0f, 30.0f, 200.0f},
    {5, 160, -4.0f, 40.0f, 220.0f, 600.0f, 600.0f, 128, 0, 70.0f, 500.0f, 40.0f, 200.0f}
    };
    ParticleParameters parameters;
    Vec spawnOrigins[3];
    Vec velocity;
    Vec position;
    f32 angle;
    f32 radius;
    f32 particleScale;
    f32 alpha;
    s16 particleIndex;
    s16 selectedOrigin;
    s16 lifetime;

    spawnOrigins[0].x = (f32)((f64)origin->position.x -
         ((f64)configs[effectIndex].horizontalRadius *
         sin((M_PI * (origin->rotation.y - 90.0f)) / 180.0)));
    spawnOrigins[0].z = (f32)((f64)origin->position.z -
         ((f64)configs[effectIndex].horizontalRadius *
         cos((M_PI * (origin->rotation.y - 90.0f)) / 180.0)));
    position.x = (f32)((f64)origin->position.x -
         ((f64)configs[effectIndex].forwardDistance *
         sin((M_PI * (origin->rotation.y - 90.0f)) / 180.0)));
    position.z = (f32)((f64)origin->position.z -
         ((f64)configs[effectIndex].forwardDistance *
         cos((M_PI * (origin->rotation.y - 90.0f)) / 180.0)));
    spawnOrigins[1].x = (f32)((f64)position.x +
         ((f64)configs[effectIndex].sideDistance *
         sin((M_PI * origin->rotation.y) / 180.0)));
    spawnOrigins[1].z = (f32)((f64)position.z +
         ((f64)configs[effectIndex].sideDistance *
         cos((M_PI * origin->rotation.y) / 180.0)));
    spawnOrigins[2].x = (f32)((f64)position.x -
         ((f64)configs[effectIndex].sideDistance *
         sin((M_PI * origin->rotation.y) / 180.0)));
    spawnOrigins[2].z = (f32)((f64)position.z -
         ((f64)configs[effectIndex].sideDistance *
         cos((M_PI * origin->rotation.y) / 180.0)));
    particleIndex = 0;

     while (particleIndex < configs[effectIndex].particleCount) {
        selectedOrigin = frandmod(100);
        selectedOrigin = 0;
        if (selectedOrigin == 0) {
             alpha = configs[effectIndex].alpha;
             radius = configs[effectIndex].spawnRadiusLimit;
             particleScale = configs[effectIndex].baseParticleScale;
        } else {
             alpha = configs[effectIndex].alternateAlpha;
             radius = configs[effectIndex].alternateRadiusLimit;
             particleScale = configs[effectIndex].alternateParticleScale;
        }

        lifetime = 10000;
        parameters.unk00 = 0;
        parameters.startColor.r = 255;
        parameters.startColor.g = 159;
        parameters.startColor.b = 159;
        parameters.startColor.a = (u8)alpha;
        parameters.targetColor.r = 255;
        parameters.targetColor.g = 159;
        parameters.targetColor.b = 159;
        parameters.targetColor.a = 0;
        parameters.rotationIncrement = 0.0f;
         parameters.alphaIncrement = configs[effectIndex].opacityChange;
        parameters.colorBlendStep = 0.01f;
        parameters.verticalAcceleration = 0.0f;

        angle = fn_1_3C54(360.0f);
        radius = fn_1_3C54(radius);
        position.x = (f32)((f64)spawnOrigins[selectedOrigin].x +
            ((f64)radius * sin((M_PI * angle) / 180.0)));
         position.y = configs[effectIndex].spawnHeight;
        position.z = (f32)((f64)spawnOrigins[selectedOrigin].z +
            ((f64)radius * cos((M_PI * angle) / 180.0)));
        particleScale += fn_1_3C54(50.0f);
        if (effectIndex == 0) {
            parameters.scaleIncrement = -8.0f;
        } else if (effectIndex == 1) {
            parameters.scaleIncrement = -14.0f;
        } else if (effectIndex == 2) {
            parameters.scaleIncrement = -14.0f;
        }
        parameters.scaleIncrementMultiplier = 1.0f;
        parameters.velocityScale[0] = 1.0f;
        parameters.velocityScale[1] = 1.0f;
        parameters.velocityScale[2] = 1.0f;
        velocity.x = 0.0f;
        velocity.y = 1.0f;
        velocity.z = 0.0f;
        fn_1_57F8(1, -1, position, velocity, particleScale, lifetime, &parameters);
        particleIndex++;
    }
}

/* Emits three randomized particles around a model attachment point. */
void fn_1_4B38(M660Model *origin)
{
    Vec velocity;
    Vec position;
    Vec sourcePosition;
    ParticleParameters parameters;
    s16 effectIndex;

    sourcePosition = origin->attachment;
    sourcePosition.x += 10.0f;
    effectIndex = 0;

    while (effectIndex < 3) {
        s16 lifetime = 10000;
        f32 angle;
        f32 radius;
        f32 initialScale;

        parameters.unk00 = 0;
        parameters.startColor.r = 255;
        parameters.startColor.g = 159;
        parameters.startColor.b = 159;
        parameters.startColor.a = 128;
        parameters.targetColor.r = 255;
        parameters.targetColor.g = 159;
        parameters.targetColor.b = 159;
        parameters.targetColor.a = 0;
        parameters.rotationIncrement = 0.0f;
        parameters.alphaIncrement = -3.0f;
        parameters.colorBlendStep = 0.01f;
        parameters.verticalAcceleration = 0.0f;

        angle = fn_1_3C54(360.0f);
        radius = fn_1_3C54(80.0f);
        position.x = (f32)((f64)sourcePosition.x +
                            ((f64)radius *
                            sin((M_PI * (f64)angle) /
                                180.0)));
        position.y = (f32)((f64)sourcePosition.y +
                            ((f64)radius *
                            cos((M_PI * (f64)angle) /
                                180.0)));
        position.z = 40.0f + sourcePosition.z;
        initialScale = 150.0f + fn_1_3C54(50.0f);
        parameters.scaleIncrement = -2.0f;
        parameters.scaleIncrementMultiplier = 1.0f;
        parameters.velocityScale[0] = 1.0f;
        parameters.velocityScale[1] = 1.0f;
        parameters.velocityScale[2] = 1.0f;
        velocity.x = 3.0f;
        velocity.y = 0.0f;
        velocity.z = 0.0f;

        fn_1_57F8(1, -1, position, velocity, initialScale, lifetime,
                  &parameters);
        effectIndex++;
    }
}

/* Sets origin Y to 200 and model 2 to layer 1, then attempts 30 particle emissions.
 * Randomizes velocity, alpha, and fade rate. The first angle and radius draws are unused. */
void fn_1_4F64(Vec *origin, s16 effectIndex)
{
    ParticleParameters parameters;
    Vec velocity;
    Vec position;
    f32 verticalPositions[3] = {200.0f, 200.0f, 200.0f};
    s32 effectLayers[3] = {1, 1, 1};
    s16 particleIndex;

    origin->y = verticalPositions[effectIndex];
    fn_80026B8C(lbl_1_bss_2484.modelIds[2],
                      (s16)effectLayers[effectIndex]);

    particleIndex = 0;
    while (particleIndex < 30) {
        s16 lifetime = 10000;
        f32 angle;
        f32 radius;
        f32 initialScale;

        parameters.unk00 = 0;
        parameters.startColor.r = 255;
        parameters.startColor.g = 159;
        parameters.startColor.b = 159;
        parameters.startColor.a = 128;
        parameters.targetColor.r = 255;
        parameters.targetColor.g = 159;
        parameters.targetColor.b = 159;
        parameters.targetColor.a = 0;
        parameters.rotationIncrement = 0.0f;
        parameters.alphaIncrement = -6.0f;
        parameters.colorBlendStep = 0.01f;
        parameters.verticalAcceleration = 0.0f;
        parameters.startColor.a = (u8)(frandmod(48) + 96);

        parameters.alphaIncrement = -(4.0f + fn_1_3C54(3.0f));
        angle = fn_1_3C54(360.0f);
        radius = fn_1_3C54(50.0f);
        position = *origin;
        initialScale = 10.0f + fn_1_3C54(10.0f);
        parameters.scaleIncrement = 4.0f;
        parameters.scaleIncrementMultiplier = 1.0f;
        parameters.velocityScale[0] = 0.94f;
        parameters.velocityScale[1] = 0.94f;
        parameters.velocityScale[2] = 0.94f;
        angle = fn_1_3C54(360.0f);
        velocity.x = (f32)(sin((M_PI * (f64)angle) /
                                180.0) *
                           (f64)(fn_1_3C54(20.0f) - 10.0f));
        velocity.y = fn_1_3C54(20.0f) -
                     10.0f;
        velocity.z = (f32)(cos((M_PI * (f64)angle) /
                                180.0) *
                           (f64)(fn_1_3C54(20.0f) -
                                 10.0f));

        fn_1_57F8(2, -1, position, velocity, initialScale, lifetime, &parameters);
        particleIndex++;
    }
}

/* Creates a particle that follows a stage object at one of three fixed heights. */
void fn_1_55DC(s16 objectIndex, s16 variant)
{
    ParticleParameters parameters;
    Vec velocity;
    Vec position;
    f32 positionValues[3] = {350.0f, 550.0f, 750.0f};
    f32 scaleValues[3] = {300.0f, 400.0f, 500.0f};
    s16 particleState = 1;

    parameters.unk00 = 0;
    parameters.startColor.r = 255;
    parameters.startColor.g = 159;
    parameters.startColor.b = 159;
    parameters.startColor.a = 208;
    parameters.targetColor.r = 255;
    parameters.targetColor.g = 159;
    parameters.targetColor.b = 159;
    parameters.targetColor.a = 48;
    parameters.rotationIncrement = 0.0f;
    parameters.alphaIncrement = 0.0f;
    parameters.colorBlendStep = 0.0f;
    parameters.verticalAcceleration = 0.0f;

    position = lbl_1_bss_60C[objectIndex].position;
    position.y = positionValues[variant];

    parameters.scaleIncrement = 0.0f;
    parameters.scaleIncrementMultiplier = 0.0f;
    parameters.velocityScale[0] = 1.0f;
    parameters.velocityScale[1] = 1.0f;
    parameters.velocityScale[2] = 1.0f;
    velocity.x = velocity.z = 0.0f;
    velocity.y = positionValues[variant];
    fn_1_57F8(3, objectIndex, position, velocity, scaleValues[variant],
              particleState, &parameters);
}

/* Stores a particle in the first inactive entry and returns its index, or -1 if full. */
s16 fn_1_57F8(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity,
              f32 initialScale, s16 state, const ParticleParameters *parameters) {
    HU3D_MODEL *model;
    s16 result;
    HU3D_PARTICLE *context;
    ParticleParameters *currentParameters;
    s16 entryIndex;
    HU3D_PARTICLE_DATA *entry;

    result = -1;
    model = &lbl_802C01C8[lbl_1_bss_2484.modelIds[modelNumber]];
    context = model->hookData;
    currentParameters = context->work;
    entryIndex = 0;
    entry = context->data;

    while (entryIndex < context->maxCnt) {
        if (entry->time <= 0) {
            currentParameters[entryIndex] = *parameters;
            currentParameters[entryIndex].playerMovementOffset.z =
                currentParameters[entryIndex].playerMovementOffset.y =
                    currentParameters[entryIndex].playerMovementOffset.x = 0.0f;

            entry->pos = position;
            entry->vel = velocity;
            entry->accel = position;
            entry->scaleBase = entry->colorIdx = entry->speedDecay = 0.0f;
            entry->color = currentParameters[entryIndex].startColor;
            entry->scale = initialScale;
            entry->time = state;
            entry->parManId = ownerIndex;
            context->emitCnt = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

/* Advances particle lifetime, position, velocity, rotation, RGBA and scale. */
void fn_1_59F8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    ParticleParameters *parameters = particle->work;
    HU3D_PARTICLE_DATA *entry = particle->data;
    s16 entryNo = 0;

    while (entryNo < particle->maxCnt) {
        if (entry->time-- > 0) {
            s16 channelValue;

            entry->vel.x *= parameters[entryNo].velocityScale[0];
            entry->vel.y *= parameters[entryNo].velocityScale[1];
            entry->vel.z *= parameters[entryNo].velocityScale[2];
            VECAdd(&entry->vel, &entry->pos, &entry->pos);
            entry->vel.y += parameters[entryNo].verticalAcceleration;
            entry->zRot += parameters[entryNo].rotationIncrement;

            channelValue = (s16)((f32)entry->color.r +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].targetColor.r -
                                        parameters[entryNo].startColor.r)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.r = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.g +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].targetColor.g -
                                        parameters[entryNo].startColor.g)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.g = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.b +
                                 (parameters[entryNo].colorBlendStep *
                                  (f32)(parameters[entryNo].targetColor.b -
                                        parameters[entryNo].startColor.b)));
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.b = (u8)channelValue;

            channelValue = (s16)((f32)entry->color.a +
                                 parameters[entryNo].alphaIncrement);
            if (channelValue < 0) {
                channelValue = 0;
            } else if (channelValue > 255) {
                channelValue = 255;
            }
            entry->color.a = (u8)channelValue;
            if (channelValue <= 0) {
                entry->time = 0;
                entry->scale = 0.0f;
            }

            parameters[entryNo].scaleIncrement *= parameters[entryNo].scaleIncrementMultiplier;
            entry->scale += parameters[entryNo].scaleIncrement;
            if (entry->scale <= 0.0f) {
                entry->scale = 0.0f;
                entry->time = 0;
            }
            DCStoreRangeNoSync(particle->data,
                               particle->maxCnt * sizeof(*particle->data));
        } else {
            entry->scale = 0.0f;
            entry->time = -1;
        }
        entryNo++;
        entry++;
    }
}

/* Follows each particle's stage object until the result phase, then follows the losing player or
 * hides the particle on a tie. */
void fn_1_5E70(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    ParticleParameters *parameters = particle->work;
    HU3D_PARTICLE_DATA *entry = particle->data;
    s16 index = 0;

    while (index < particle->maxCnt) {
        switch (entry->time) {
        case 0:
            break;
        case 1:
        {
            entry->pos = lbl_1_bss_60C[entry->parManId].position;
            entry->pos.y = entry->vel.y;
            if (lbl_1_bss_1F4.phase >= 2) {
                entry->parManId = 72;
                entry->scale = 250.0f;
                entry->vel.z = 80.0f;
                entry->time++;
            }
            break;
        }
        case 2:
            if (lbl_1_bss_1F4.tie == 0) {
                entry->pos = lbl_1_bss_2DC[lbl_1_bss_1F4.losingPlayerIndex].position;
                entry->pos.y += 30.0f;
                entry->pos.z += 100.0f;
            } else {
                entry->scale = 0.0f;
                entry->time = 0;
            }
            break;
        }
        DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
        index++;
        entry++;
    }
}

/* Sets voice pan and volume, then plays and clears a pending character hit voice.
 * Both callers use mode 6, which pans by world X and uses maximum volume. */
void fn_1_601C(M660Player *player, s16 spatialMode)
{
    s16 pan;
    s16 volume;
    f32 panScale;
    f32 volumeScale;
    f32 positionDelta;

    if (spatialMode == 0 || spatialMode == 1 || spatialMode == 6) {
        if (lbl_1_bss_1F4.xMaximum >= player->position.x &&
            lbl_1_bss_1F4.xMinimum <= player->position.x) {
            panScale =
                (f32)(lbl_1_bss_1F4.panMaximum - lbl_1_bss_1F4.panMinimum) /
                (lbl_1_bss_1F4.xMaximum - lbl_1_bss_1F4.xMinimum);
            positionDelta = player->position.x - lbl_1_bss_1F4.xMinimum;
            positionDelta *= panScale;
            pan = (s16)positionDelta;
            pan += lbl_1_bss_1F4.panMinimum;
        } else if (lbl_1_bss_1F4.xMaximum < player->position.x) {
            pan = lbl_1_bss_1F4.panMaximum;
        } else if (lbl_1_bss_1F4.xMinimum > player->position.x) {
            pan = lbl_1_bss_1F4.panMinimum;
        }
    }

    if (spatialMode == 0 || spatialMode == 2) {
        if (lbl_1_bss_1F4.zMaximum >= player->position.z &&
            lbl_1_bss_1F4.zMinimum <= player->position.z) {
            volumeScale =
                (f32)(lbl_1_bss_1F4.volumeMaximum - lbl_1_bss_1F4.volumeMinimum) /
                (lbl_1_bss_1F4.zMaximum - lbl_1_bss_1F4.zMinimum);
            positionDelta = player->position.z - lbl_1_bss_1F4.zMinimum;
            positionDelta *= volumeScale;
            volume = (s16)positionDelta;
            volume += lbl_1_bss_1F4.volumeMinimum;
        } else if (lbl_1_bss_1F4.zMaximum < player->position.z) {
            volume = lbl_1_bss_1F4.volumeMaximum;
        } else if (lbl_1_bss_1F4.zMinimum > player->position.z) {
            volume = lbl_1_bss_1F4.volumeMinimum;
        }
    }

    if (spatialMode == 2 || spatialMode == 3 || spatialMode == 4 || spatialMode == 5) {
        pan = lbl_1_bss_1F4.panMinimum +
              (lbl_1_bss_1F4.panMaximum - lbl_1_bss_1F4.panMinimum) / 2;
    }
    if (spatialMode == 1 || spatialMode == 3) {
        volume = lbl_1_bss_1F4.volumeMinimum +
                 (lbl_1_bss_1F4.volumeMaximum - lbl_1_bss_1F4.volumeMinimum) / 2;
    }
    if (spatialMode == 4) {
        volume = lbl_1_bss_1F4.volumeMinimum;
    }
    if (spatialMode == 5 || spatialMode == 6) {
        volume = lbl_1_bss_1F4.volumeMaximum;
    }

    fn_80060F14(player->character, pan);
    fn_80060F40(player->character, volume);
    if (player->hitVoiceRequest == 1) {
        s32 soundIds[14] = {MSM_SE_CHARVOICE_HIT_YELP,
            MSM_SE_CHARVOICE_LUIGI_HIT_YELP, MSM_SE_CHARVOICE_PEACH_HIT_YELP,
            MSM_SE_CHARVOICE_YOSHI_HIT_YELP, MSM_SE_CHARVOICE_WARIO_HIT_YELP,
            MSM_SE_CHARVOICE_DAISY_HIT_YELP, MSM_SE_CHARVOICE_WALUIGI_HIT_YELP,
            MSM_SE_CHARVOICE_KINOPIO_HIT_YELP, MSM_SE_CHARVOICE_TERESA_HIT_YELP,
            MSM_SE_CHARVOICE_MINIKOOPA_HIT_YELP, MSM_SE_CHARVOICE_KINOPIKO_HIT_YELP,
            MSM_SE_CHARVOICE_MINIKOOPAR_HIT_YELP, MSM_SE_CHARVOICE_MINIKOOPAG_HIT_YELP,
            MSM_SE_CHARVOICE_MINIKOOPAB_HIT_YELP};

        HuAudFXPlayVolPan(soundIds[player->character], volume, pan);
        player->hitVoiceRequest = -1;
    }
}

/* Initializes streamed audio handles, spatial sound bounds and four effect-sound slots. */
void fn_1_64D0(void) {
    s16 soundSlot;

    lbl_1_bss_1F4.backgroundStreamHandle = -1;
    lbl_1_bss_1F4.jingleStreamHandle = -1;
    lbl_1_bss_1F4.backgroundTrackId = MSM_STREAM_NIGHT_DUEL;
    lbl_1_bss_1F4.jingleId = -1;
    lbl_1_bss_1F4.panMinimum = 32;
    lbl_1_bss_1F4.panMaximum = 96;
    lbl_1_bss_1F4.xMinimum = (f32) -800.0f;
    lbl_1_bss_1F4.xMaximum = (f32) 800.0f;
    lbl_1_bss_1F4.volumeMinimum = 32;
    lbl_1_bss_1F4.volumeMaximum = 110;
    lbl_1_bss_1F4.zMinimum = (f32) -800.0f;
    lbl_1_bss_1F4.zMaximum = (f32) 800.0f;
    soundSlot = 0;
    while (soundSlot < 4) {
        lbl_1_bss_1F4.soundStatus[soundSlot] = -1;
        lbl_1_bss_1F4.soundEffectHandle[soundSlot] = -1;
        soundSlot += 1;
    }
}

/* Starts the queued jingle once the second stream channel becomes available. */
void fn_1_6614(void) {
    if (((s32) lbl_1_bss_1F4.jingleId != -1) &&
        ((s32) lbl_1_bss_1F4.jingleStreamHandle == -1)) {
        lbl_1_bss_1F4.jingleStreamHandle = HuAudJinglePlay((s16) lbl_1_bss_1F4.jingleId);
    }
}

/* Starts the selected background track when no first-channel stream is active. */
void fn_1_6678(void) {
    if ((s32) lbl_1_bss_1F4.backgroundStreamHandle == -1) {
        lbl_1_bss_1F4.backgroundStreamHandle =
            HuAudBGMPlay((s16) lbl_1_bss_1F4.backgroundTrackId);
    }
}

/* Fades the selected stream and clears its handle after requesting the fade. */
void fn_1_66C8(s16 streamSlot, s16 fadeSpeed) {
    s16 streamHandle = streamSlot != 0
                       ? lbl_1_bss_1F4.backgroundStreamHandle
                       : lbl_1_bss_1F4.jingleStreamHandle;
    if (streamHandle != -1) {
        HuAudSStreamFadeOut(streamHandle, fadeSpeed);
        if (streamSlot != 0) {
            lbl_1_bss_1F4.backgroundStreamHandle = -1;
            return;
        }
        lbl_1_bss_1F4.jingleStreamHandle = -1;
    }
}

/* Stops the selected jingle or background stream immediately. */
void fn_1_6774(s16 streamSelector)
{
    s16 streamHandle;

    if (streamSelector != 0) {
        streamHandle = (s16)lbl_1_bss_1F4.backgroundStreamHandle;
    } else {
        streamHandle = (s16)lbl_1_bss_1F4.jingleStreamHandle;
    }

    HuAudSStreamStop(streamHandle);
}

/* Starts a stage sound with the selected pan and volume mode; every caller uses mode 5 and a NULL
* position, centering pan and forcing maximum volume without reading the position. */
void fn_1_67D4(s16 slot, s16 mode, Vec *position)
{
    s16 pan;
    s16 volume;
    f32 positionDelta;
    f32 panScale;
    f32 volumeScale;

    if (mode <= 5) {
    if (mode == 0 || mode == 1) {
        if (lbl_1_bss_1F4.xMaximum >= position->x &&
            lbl_1_bss_1F4.xMinimum <= position->x) {
            panScale = (f32)(lbl_1_bss_1F4.panMaximum -
                             lbl_1_bss_1F4.panMinimum) /
                       (lbl_1_bss_1F4.xMaximum -
                        lbl_1_bss_1F4.xMinimum);
            positionDelta = position->x -
                            lbl_1_bss_1F4.xMinimum;
            positionDelta *= panScale;
            pan = (s16)positionDelta;
            pan += lbl_1_bss_1F4.panMinimum;
        } else if (lbl_1_bss_1F4.xMaximum < position->x) {
            pan = lbl_1_bss_1F4.panMaximum;
        } else if (lbl_1_bss_1F4.xMinimum > position->x) {
            pan = lbl_1_bss_1F4.panMinimum;
        }
    }

    if (mode == 0 || mode == 2) {
        if (lbl_1_bss_1F4.zMaximum >= position->z &&
            lbl_1_bss_1F4.zMinimum <= position->z) {
            volumeScale = (f32)(lbl_1_bss_1F4.volumeMaximum -
                                lbl_1_bss_1F4.volumeMinimum) /
                          (lbl_1_bss_1F4.zMaximum -
                           lbl_1_bss_1F4.zMinimum);
            positionDelta = position->z -
                            lbl_1_bss_1F4.zMinimum;
            positionDelta *= volumeScale;
            volume = (s16)positionDelta;
            volume += lbl_1_bss_1F4.volumeMinimum;
        } else if (lbl_1_bss_1F4.zMaximum < position->z) {
            volume = lbl_1_bss_1F4.volumeMaximum;
        } else if (lbl_1_bss_1F4.zMinimum > position->z) {
            volume = lbl_1_bss_1F4.volumeMinimum;
        }
    }

    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
        pan = (s16)(lbl_1_bss_1F4.panMinimum +
                    (lbl_1_bss_1F4.panMaximum -
                     lbl_1_bss_1F4.panMinimum) / 2);
    }

    if (mode == 1 || mode == 3) {
        volume = (s16)(lbl_1_bss_1F4.volumeMinimum +
                       (lbl_1_bss_1F4.volumeMaximum -
                        lbl_1_bss_1F4.volumeMinimum) / 2);
    }

    if (mode == 4) {
        volume = lbl_1_bss_1F4.volumeMinimum;
    }

    if (mode == 5) {
        volume = lbl_1_bss_1F4.volumeMaximum;
    }
    lbl_1_bss_1F4.soundStatus[slot] = 1;
    lbl_1_bss_1F4.soundEffectHandle[slot] =
        HuAudFXPlayVolPan(lbl_1_data_F6[slot], volume, pan);
    }
}

s16 lbl_1_data_F6[5] = { MSM_SE_MG660_STAGE_OPENING, MSM_SE_MG660_WAVE_APPEAR_FIRST,
                         MSM_SE_MG660_WAVE_APPEAR_SECOND, MSM_SE_MG660_PLAYER_ACTION, 0 };
