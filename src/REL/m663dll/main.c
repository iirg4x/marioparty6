/* Module entry and sequence hooks, pad input, model transforms, camera, light, shadow,
 * projection, particle effects, timer helpers and sound control. */
#include "REL/m663dll/transition.h"
#include "dolphin/gx.h"
#include "REL/m663dll/models.h"
#include "REL/m663dll/tutorial.h"
#include "dolphin/types.h"
#include "math.h"
#include <string.h>
#include "dolphin/mtx.h"
#include "REL/m663dll/input.h"
#include "REL/m663dll/player.h"
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
#include "REL/m663dll/camera.h"
#include "REL/m663dll/lighting.h"
#include "REL/m663dll/projection.h"
#include "REL/m663dll/shadow.h"
#include "REL/m663dll/particles.h"
#include "REL/m663dll/score.h"
#include "REL/m663dll/actors.h"
#include "REL/m663dll/motion.h"
#include "include/game/mg/actman.h"
#include "include/game/audio.h"
#include "include/game/gamemes.h"
#include "game/hu3d.h"
#include "datanum/charmot.h"
#include "humath.h"
#include "game/printfunc.h"
#include "REL/m663dll/effects.h"
#include "dolphin/os.h"

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_A0(void);

int _prolog(void)
{
    const VoidFunc *ctor = _ctors;
    while (*ctor != 0) {
        (*ctor)();
        ctor++;
    }
    fn_1_A0();
    return 0;
}

void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

extern HUPROCESS *lbl_1_bss_15A0;

void fn_1_124(s16 mode, s16 frameNo);

void fn_1_2A4(s16 mode, s16 frameNo);

void fn_1_2D0(s16 mode, s16 frameNo);

void fn_1_2FC(s16 mode, s16 frameNo);

void fn_1_328(s16 mode, s16 frameNo);

void fn_1_354(s16 mode, s16 frameNo);

void fn_1_380(s16 mode, s16 frameNo);

void fn_1_3AC(s16 mode, s16 frameNo);

void fn_1_3D8(s16 mode, s16 frameNo);

MGSEQ_PARAM lbl_1_data_0 = {
    0, 0, fn_1_124, fn_1_2A4, fn_1_2D0, fn_1_2FC, fn_1_328,
    fn_1_354, fn_1_380, fn_1_3AC, fn_1_3D8
};

/* Module setup: creates the minigame object manager and camera object, registers the
 * sequence hooks, and sets the delay frames for sequence modes 2 and 7. */
void fn_1_A0(void) {
    lbl_1_bss_15A0 = fn_80080418();
    fn_8003AB38(lbl_1_bss_15A0, 0x7FDA, 0U, 0U, -1, fn_8003D4FC);
    fn_80070788(&lbl_1_data_0);
    fn_80071CE8(2, 0x3E8);
    fn_80071CE8(7, 0x3E8);
    fn_80071CE8(2, -0x15A0);
}

extern s16 lbl_802C0278;

extern void fn_1_17E0(s16 cameraIndex);

extern void fn_1_9EC8(void);

/* Init phase: clears all module work, stores the night flag, sets up the camera, light,
 * shadow and black background, creates the game objects and sound settings, then advances
 * the sequence. */
void fn_1_124(s16 mode, s16 frameNo) {
    s16 nightFlag;

    memset(&lbl_1_bss_1428, 0, 0x178);
    memset(&lbl_1_bss_13EC, 0, 0x3C);
    memset(&lbl_1_bss_139C, 0, 0x50);
    memset(&lbl_1_bss_1378, 0, 0x24);
    memset(&lbl_1_bss_1368, 0, 0x10);
    memset(lbl_1_bss_750, 0, 0xC18);
    memset(lbl_1_bss_688, 0, 0xC8);
    memset(lbl_1_bss_4F0, 0, 0x198);
    memset(&lbl_1_bss_424, 0, 0xCC);
    memset(&lbl_1_bss_418, 0, 0xC);
    memset(lbl_1_bss_378, 0, 0xA0);
    memset(lbl_1_bss_350, 0, 0x28);
    memset(lbl_1_bss_120, 0, 0x230);
    memset(lbl_1_bss_0, 0, 0x120);

    nightFlag = lbl_802C0278;
    lbl_1_bss_424.nightFlag = nightFlag;
    fn_1_17E0(0);
    fn_1_1CC4();
    fn_1_2104();
    Hu3DBGColorSet(0, 0, 0);
    fn_1_9EC8();
    fn_1_5D14();
    fn_80071A9C();
}

void fn_1_2A4(s16 mode, s16 frameNo)
{
    if (fn_1_A464() != 0) fn_80071A9C();
}

void fn_1_2D0(s16 mode, s16 frameNo)
{
    if (fn_1_A4A8() != 0) fn_80071A9C();
}

void fn_1_2FC(s16 mode, s16 frameNo)
{
    if (fn_1_6804() != 0) fn_80071A9C();
}

void fn_1_328(s16 mode, s16 frameNo)
{
    if (fn_1_BCC8() != 0) fn_80071A9C();
}

void fn_1_354(s16 mode, s16 frameNo)
{
    if (fn_1_BD14() != 0) fn_80071A9C();
}

void fn_1_380(s16 mode, s16 frameNo)
{
    if (fn_1_BD58() != 0) fn_80071A9C();
}

void fn_1_3AC(s16 mode, s16 frameNo)
{
    if (fn_1_BFC0() != 0) fn_80071A9C();
}

s32 fn_1_BFE8(void);

void fn_1_3D8(s16 mode, s16 frameNo)
{
    if (fn_1_BFE8() != 0) fn_80071A9C();
}

void fn_1_404(void)
{
}

void fn_1_408(void)
{
    s16 playerLookup[2] = {-1, -1};
    HuVecF positions[2] = {{-150.0f, 10.0f, -100.0f}, {150.0f, 10.0f, -100.0f}};
    HuVecF offsets[2] = {{0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f}};
    s16 player = 0;
    s16 group = 0;

    for (; player < 4; player++) {
        group = lbl_80265728[player].grpNo;

        if ((group == 0 || group == 1) && playerLookup[group] == -1) {
            playerLookup[group] = player;
        }
    }

    for (player = 0; player < 2; player++) {
        lbl_1_bss_4F0[player].playerNo =
            (s8)playerLookup[player];
        lbl_1_bss_4F0[player].playerIndex = (s8)player;
        lbl_1_bss_4F0[player].characterNo =
            (s8)lbl_80265728[playerLookup[player]].charNo;
        lbl_1_bss_4F0[player].padNo =
            (s8)lbl_80265728[playerLookup[player]].padNo;
        lbl_1_bss_4F0[player].groupNo =
            (s8)lbl_80265728[playerLookup[player]].grpNo;
        lbl_1_bss_4F0[player].playerType =
            (s8)lbl_80265728[playerLookup[player]].type;
        lbl_1_bss_4F0[player].difficulty =
            (s8)lbl_80265728[playerLookup[player]].comDif;
        lbl_1_bss_4F0[player].position = positions[player];
        lbl_1_bss_4F0[player].rotation = offsets[player];
        lbl_1_bss_4F0[player].mode = -1;
    }
}

extern u16 lbl_802C0010[];

extern u16 lbl_802C0008[];

extern u16 lbl_802C0000[];

extern s8 lbl_802BFFFC[];

extern u8 lbl_802BFFEC[];

extern u8 lbl_802BFFE8[];

extern u8 lbl_802BFFE4[];

extern u8 lbl_802BFFE0[];

void fn_1_754(void)
{
    s32 index;
    s32 stickResult;
    s32 deadZoneValue;

    index = 0;
    for (; index < 2; index++) {
        if (lbl_1_bss_4F0[index].playerType != 0 ||
            lbl_1_bss_4F0[index].inputMode != 0) {
            memset(&lbl_1_bss_4F0[index], 0, 0x14);
        } else {
            lbl_1_bss_4F0[index].input.buttons = lbl_802C0010[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.buttonsDown = lbl_802C0008[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.buttonsRepeat = lbl_802C0000[lbl_1_bss_4F0[index].padNo];
            if (lbl_802BFFFC[lbl_1_bss_4F0[index].padNo] > 0) {
                stickResult = lbl_802BFFFC[lbl_1_bss_4F0[index].padNo];
            } else {
                if (lbl_802BFFFC[lbl_1_bss_4F0[index].padNo] * -1 < 0xA) {
                    deadZoneValue = 0;
                } else {
                    deadZoneValue = lbl_802BFFFC[lbl_1_bss_4F0[index].padNo];
                }
                stickResult = deadZoneValue;
            }
            lbl_1_bss_4F0[index].input.stickX = stickResult;
            lbl_1_bss_4F0[index].input.stickY = (s8)lbl_802BFFF8[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.substickX = (s8)lbl_802BFFF4[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.substickY = (s8)lbl_802BFFF0[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.triggerL = lbl_802BFFEC[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.triggerR = lbl_802BFFE8[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.dpad = lbl_802BFFE4[lbl_1_bss_4F0[index].padNo];
            lbl_1_bss_4F0[index].input.dpadRepeat = lbl_802BFFE0[lbl_1_bss_4F0[index].padNo];
        }
    }

    index = 0;
    for (; index < 4; index++) {
        lbl_1_bss_424.padInputs[index].buttons = lbl_802C0010[index];
        lbl_1_bss_424.padInputs[index].buttonsDown = lbl_802C0008[index];
        lbl_1_bss_424.padInputs[index].buttonsRepeat = lbl_802C0000[index];
        lbl_1_bss_424.padInputs[index].stickX = (s8)lbl_802BFFFC[index];
        lbl_1_bss_424.padInputs[index].stickY = (s8)lbl_802BFFF8[index];
        lbl_1_bss_424.padInputs[index].substickX = (s8)lbl_802BFFF4[index];
        lbl_1_bss_424.padInputs[index].substickY = (s8)lbl_802BFFF0[index];
        lbl_1_bss_424.padInputs[index].triggerL = lbl_802BFFEC[index];
        lbl_1_bss_424.padInputs[index].triggerR = lbl_802BFFE8[index];
        lbl_1_bss_424.padInputs[index].dpad = lbl_802BFFE4[index];
        lbl_1_bss_424.padInputs[index].dpadRepeat = lbl_802BFFE0[index];
    }
}

/* Applies stored positions and rotations to models 2-85, and to the two player models when
 * their transform mode is 0 or 2. */
void fn_1_D14(void) {
    s32 index;
    index = 2;
    while (index < 0x56)
    {
        fn_80026760(lbl_1_bss_750[index].modelId, (lbl_1_bss_750[index].position.x), (lbl_1_bss_750[index].position.y), (lbl_1_bss_750[index].position.z));
        fn_80026808(lbl_1_bss_750[index].modelId, (lbl_1_bss_750[index].rotation.x), (lbl_1_bss_750[index].rotation.y), (lbl_1_bss_750[index].rotation.z));
        index += 1;
    }
    index = 0;
    while (index <= 1)
    {
        if (((s8) lbl_1_bss_4F0[index].transformMode == 0) || ((s8) lbl_1_bss_4F0[index].transformMode == 2))
        {
            fn_80026760(lbl_1_bss_750[index].modelId, lbl_1_bss_4F0[index].position.x, lbl_1_bss_4F0[index].position.y, lbl_1_bss_4F0[index].position.z);
            fn_80026808(lbl_1_bss_750[index].modelId, lbl_1_bss_4F0[index].rotation.x, lbl_1_bss_4F0[index].rotation.y, lbl_1_bss_4F0[index].rotation.z);
        }
        index += 1;
    }
}

void fn_1_EF0(void)
{
    u32 motionData[8] = {
        CHARMOT_HSF_c000m1_300,
        CHARMOT_HSF_c000m1_302,
        CHARMOT_HSF_c000m1_306,
        CHARMOT_HSF_c000m1_307,
        CHARMOT_HSF_c000m1_346,
        CHARMOT_HSF_c000m1_348,
        CHARMOT_HSF_c000m1_338,
        CHARMOT_HSF_c000m1_365,
    };
    s16 playerIndex;
    s16 motionIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        motionIndex = 0;
        while (motionIndex < 8) {
            lbl_1_bss_688[playerIndex].motionIds[motionIndex] =
                fn_8005B0B8(lbl_1_bss_4F0[playerIndex].characterNo,
                                 motionData[motionIndex]);
            motionIndex++;
        }
        playerIndex++;
    }
}

void fn_1_FE8(void)
{
    s16 playerIndex;

    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        if (lbl_1_bss_688[playerIndex].motionIndex != lbl_1_bss_688[playerIndex].previousMotionIndex) {
            fn_8002C95C(lbl_1_bss_750[playerIndex].modelId,
                          lbl_1_bss_688[playerIndex].motionIds[lbl_1_bss_688[playerIndex].motionIndex]);
            lbl_1_bss_688[playerIndex].previousMotionIndex = lbl_1_bss_688[playerIndex].motionIndex;
            lbl_1_bss_688[playerIndex].motionLength =
                fn_8002D530(lbl_1_bss_750[playerIndex].modelId);
        }
        if (lbl_1_bss_688[playerIndex].motionSpeed != lbl_1_bss_688[playerIndex].previousMotionSpeed) {
            fn_8002D86C(lbl_1_bss_750[playerIndex].modelId,
                               lbl_1_bss_688[playerIndex].motionSpeed);
            lbl_1_bss_688[playerIndex].previousMotionSpeed = lbl_1_bss_688[playerIndex].motionSpeed;
        }
        if (lbl_1_bss_688[playerIndex].modelAttributes != lbl_1_bss_688[playerIndex].previousModelAttributes) {
            fn_800269E8(lbl_1_bss_750[playerIndex].modelId,
                             lbl_1_bss_688[playerIndex].modelAttributes);
            lbl_1_bss_688[playerIndex].previousModelAttributes =
                lbl_1_bss_688[playerIndex].modelAttributes;
        }
        if (lbl_1_bss_688[playerIndex].attributesToClear != 0) {
            fn_80026A44(lbl_1_bss_750[playerIndex].modelId,
                               lbl_1_bss_688[playerIndex].attributesToClear);
            lbl_1_bss_688[playerIndex].attributesToClear =
                ~lbl_1_bss_688[playerIndex].attributesToClear;
            lbl_1_bss_688[playerIndex].previousModelAttributes =
                lbl_1_bss_688[playerIndex].modelAttributes =
                    lbl_1_bss_688[playerIndex].modelAttributes &
                    lbl_1_bss_688[playerIndex].attributesToClear;
            lbl_1_bss_688[playerIndex].attributesToClear = 0;
        }
        if (lbl_1_bss_688[playerIndex].motionTime != lbl_1_bss_688[playerIndex].previousMotionTime) {
            fn_8002D414(lbl_1_bss_750[playerIndex].modelId,
                              lbl_1_bss_688[playerIndex].motionTime);
        }

        switch (lbl_1_bss_688[playerIndex].shiftState) {
        case 0:
            break;
        case 1:
            fn_8002CCC0(lbl_1_bss_750[playerIndex].modelId,
                               lbl_1_bss_688[playerIndex].motionIds[lbl_1_bss_688[playerIndex].shiftMotionIndex],
                               lbl_1_bss_688[playerIndex].shiftStartTime,
                               lbl_1_bss_688[playerIndex].shiftDuration,
                               lbl_1_bss_688[playerIndex].shiftAttributes);
            lbl_1_bss_688[playerIndex].shiftLength =
                fn_8002D5D4(lbl_1_bss_750[playerIndex].modelId);
            lbl_1_bss_688[playerIndex].shiftState = 2;
            break;
        case 2:
            lbl_1_bss_688[playerIndex].currentShiftTime =
                fn_8002D508(lbl_1_bss_750[playerIndex].modelId);
            if (lbl_1_bss_688[playerIndex].currentShiftTime >=
                lbl_1_bss_688[playerIndex].shiftDuration - 1.0f) {
                lbl_1_bss_688[playerIndex].shiftState = 3;
            }
            break;
        case 3:
            lbl_1_bss_688[playerIndex].shiftState = 0;
            break;
        }

        if (lbl_1_bss_688[playerIndex].shiftState == 0) {
            lbl_1_bss_688[playerIndex].motionTime = fn_8002D4E0(lbl_1_bss_750[playerIndex].modelId);
            lbl_1_bss_688[playerIndex].previousMotionTime = lbl_1_bss_688[playerIndex].motionTime;
        } else {
            lbl_1_bss_688[playerIndex].previousMotionTime =
                lbl_1_bss_688[playerIndex].motionTime = -1.0f;
        }

        lbl_1_bss_688[playerIndex].motionEnded = fn_8002D75C(lbl_1_bss_750[playerIndex].modelId);
        if (lbl_1_bss_688[playerIndex].motionEnded != 0 &&
            lbl_1_bss_688[playerIndex].holdMotion != 0) {
            lbl_1_bss_688[playerIndex].motionEnded = 0;
            lbl_1_bss_688[playerIndex].holdMotion = 0;
            lbl_1_bss_688[playerIndex].motionIndex = lbl_1_bss_688[playerIndex].nextMotionIndex;
            lbl_1_bss_688[playerIndex].motionTime = lbl_1_bss_688[playerIndex].nextMotionTime;
            lbl_1_bss_688[playerIndex].modelAttributes = lbl_1_bss_688[playerIndex].nextModelAttributes;
            lbl_1_bss_688[playerIndex].attributesToClear =
                lbl_1_bss_688[playerIndex].nextAttributesToClear;
        }
    }
}

extern void fn_1_1C90(s16 cameraIndex);

void fn_1_17E0(s16 cameraIndex)
{
    f32 presets[4][13] = {
        {0.0f, 2.6e+02f, 25.0f, -37.7f, 0.0f, 0.0f, 3026.3f, 1.33f, 0.0f, 0.0f, 6.4e+02f, 4.8e+02f, 25.5f},
        {0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 6.4e+02f, 4.8e+02f, 45.0f},
        {0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 6.4e+02f, 4.8e+02f, 45.0f},
        {0.0f, 208.0f, 75.0f, -21.0f, 0.0f, 0.0f, 1415.0f, 1.33f, 0.0f, 0.0f, 6.4e+02f, 4.8e+02f, 45.0f}
    };
    s16 cameraIds[4] = {1, 2, 4, 8};

    lbl_1_bss_1428.cameras[cameraIndex].cameraIndex = cameraIndex;
    lbl_1_bss_1428.cameras[cameraIndex].position[0] = presets[cameraIndex][0];
    lbl_1_bss_1428.cameras[cameraIndex].position[1] = presets[cameraIndex][1];
    lbl_1_bss_1428.cameras[cameraIndex].position[2] = presets[cameraIndex][2];
    lbl_1_bss_1428.cameras[cameraIndex].rotation[0] = presets[cameraIndex][3];
    lbl_1_bss_1428.cameras[cameraIndex].rotation[1] = presets[cameraIndex][4];
    lbl_1_bss_1428.cameras[cameraIndex].rotation[2] = presets[cameraIndex][5];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[0] = presets[cameraIndex][6];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[1] = presets[cameraIndex][7];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[2] = presets[cameraIndex][8];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[3] = presets[cameraIndex][9];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[4] = presets[cameraIndex][10];
    lbl_1_bss_1428.cameras[cameraIndex].viewport[5] = presets[cameraIndex][11];
    lbl_1_bss_1428.cameras[cameraIndex].fieldOfView = presets[cameraIndex][12];

    Hu3DCameraCreate(cameraIds[cameraIndex]);
    Hu3DCameraViewportSet(cameraIds[cameraIndex],
                          lbl_1_bss_1428.cameras[cameraIndex].viewport[2],
                          lbl_1_bss_1428.cameras[cameraIndex].viewport[3],
                          lbl_1_bss_1428.cameras[cameraIndex].viewport[4],
                          lbl_1_bss_1428.cameras[cameraIndex].viewport[5],
                          0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(cameraIds[cameraIndex],
                             lbl_1_bss_1428.cameras[cameraIndex].fieldOfView,
                             20.0f, 30000.0f,
                             lbl_1_bss_1428.cameras[cameraIndex].viewport[1]);
    lbl_80266A4C.x = presets[cameraIndex][0];
    lbl_80266A4C.y = presets[cameraIndex][1];
    lbl_80266A4C.z = presets[cameraIndex][2];
    lbl_80266A40.x = presets[cameraIndex][3];
    lbl_80266A40.y = presets[cameraIndex][4];
    lbl_80266A40.z = presets[cameraIndex][5];
    lbl_802C028C = presets[cameraIndex][6];
    fn_1_1C90(cameraIndex);
}

inline void fn_1_1C90(s16 cameraIndex) {
    unsigned short values[4] = {1, 2, 4, 8};
}

/* Resets the light slots, creates a white global light aimed at the origin from 1000 units
 * at a -70 degree angle, places it, and makes it static and infinite. */
void fn_1_1CC4(void) {
    fn_800285BC();

    lbl_1_bss_13EC.position.x = 0.0f;
    lbl_1_bss_13EC.position.y = 0.0f;
    lbl_1_bss_13EC.position.z = 0.0f;
    lbl_1_bss_13EC.aim.x = 0.0f;
    lbl_1_bss_13EC.aim.y = 0.0f;
    lbl_1_bss_13EC.aim.z = 0.0f;
    lbl_1_bss_13EC.orientation.x = -70.0f;
    lbl_1_bss_13EC.orientation.y = 0.0f;
    lbl_1_bss_13EC.orientation.z = 0.0f;
    lbl_1_bss_13EC.range = 1000.0f;

    lbl_1_bss_13EC.color.r = 0xff;
    lbl_1_bss_13EC.color.g = 0xff;
    lbl_1_bss_13EC.color.b = 0xff;
    lbl_1_bss_13EC.color.a = 0xff;

    PSVECSubtract(&lbl_1_bss_13EC.aim, &lbl_1_bss_13EC.position,
                  &lbl_1_bss_13EC.direction);
    fn_1_1E98(lbl_1_bss_13EC.id = fn_800286D0(
        &lbl_1_bss_13EC.position, &lbl_1_bss_13EC.direction,
        &lbl_1_bss_13EC.color));
    Hu3DGLightStaticSet(lbl_1_bss_13EC.id, 1);
    Hu3DGLightInfinitytSet(lbl_1_bss_13EC.id);
}

extern double sin(double);
extern double cos(double);

/* Places the global light at its range from the aim point along its orientation angles,
 * aims it at that point and applies its color. */
void fn_1_1E98(HU3D_LIGHTID lightId) {
    lbl_1_bss_13EC.position.x =
        lbl_1_bss_13EC.aim.x +
        lbl_1_bss_13EC.range *
            (cos(M_PI * lbl_1_bss_13EC.orientation.x /
                 180.0) *
             sin(M_PI * lbl_1_bss_13EC.orientation.y /
                 180.0));
    lbl_1_bss_13EC.position.y =
        lbl_1_bss_13EC.aim.y +
        lbl_1_bss_13EC.range *
            -sin(M_PI * lbl_1_bss_13EC.orientation.x /
                 180.0);
    lbl_1_bss_13EC.position.z =
        lbl_1_bss_13EC.aim.z +
        lbl_1_bss_13EC.range *
            (cos(M_PI * lbl_1_bss_13EC.orientation.x /
                 180.0) *
             cos(M_PI * lbl_1_bss_13EC.orientation.y /
                 180.0));
    PSVECSubtract(&lbl_1_bss_13EC.aim, &lbl_1_bss_13EC.position,
                  &lbl_1_bss_13EC.direction);
    PSVECNormalize(&lbl_1_bss_13EC.direction, &lbl_1_bss_13EC.direction);
    Hu3DGLightPosSetV(lbl_1_bss_13EC.id, &lbl_1_bss_13EC.position,
                      &lbl_1_bss_13EC.direction);
    Hu3DGLightColorSet(lbl_1_bss_13EC.id, lbl_1_bss_13EC.color.r,
                       lbl_1_bss_13EC.color.g, lbl_1_bss_13EC.color.b,
                       lbl_1_bss_13EC.color.a);
}

/* Creates the shadow map, sets its size, view angles, target, distance and transparency,
 * then applies them. */
void fn_1_2104(void) {
    Hu3DShadowCreate(5.0f, 1.6e+04f, 2.1e+04f);
    Hu3DShadowSizeSet(0x1E0);
    lbl_1_bss_139C.azimuth = -9e+01f;
    lbl_1_bss_139C.elevation = 0.0f;
    lbl_1_bss_139C.unobservedValue = 0.0f;
    lbl_1_bss_139C.target.x = 0.0f;
    lbl_1_bss_139C.target.y = -1e+02f;
    lbl_1_bss_139C.target.z = 0.0f;
    lbl_1_bss_139C.transparency = 0.4f;
    lbl_1_bss_139C.distance = 2.1e+04f;
    fn_1_2214();
}

void fn_1_2214(void)
{
    lbl_1_bss_139C.cameraPosition.x = (f32)((f64)lbl_1_bss_139C.target.x +
        ((f64)lbl_1_bss_139C.distance *
         (cos((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
          sin((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0))));
    lbl_1_bss_139C.cameraPosition.y = (f32)((f64)lbl_1_bss_139C.target.y +
        ((f64)lbl_1_bss_139C.distance *
         -sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0)));
    lbl_1_bss_139C.cameraPosition.z = (f32)((f64)lbl_1_bss_139C.target.z +
        ((f64)lbl_1_bss_139C.distance *
         (cos((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
          cos((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0))));

    lbl_1_bss_139C.cameraUp.x = (f32)(
        sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
        sin((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0));
    lbl_1_bss_139C.cameraUp.y = (f32)cos(
        (M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0);
    lbl_1_bss_139C.cameraUp.z = (f32)(
        sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
        cos((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0));

    Hu3DShadowTPLvlSet(lbl_1_bss_139C.transparency);
    Hu3DShadowPosSet(&lbl_1_bss_139C.cameraPosition, &lbl_1_bss_139C.cameraUp,
                     &lbl_1_bss_139C.target);
}

void fn_1_2700(void);                                /* static */

/* Creates the projection from module data, sets its view parameters, and assigns it to
 * every model except model 2, models 26-78 and models 80-83, then applies its view. */
void fn_1_2538(void) {
    s16 modelIndex;

    lbl_1_bss_1378.projectionId = Hu3DProjectionCreate(HuSprAnimRead(fn_80008DD0(DATANUM(0x7a0000, 35), 0x10000000, 2)), 1.0f, 2e+01f, 3e+04f);
    lbl_1_bss_1378.target.x = 0.0f;
    lbl_1_bss_1378.target.y = 5e+01f;
    lbl_1_bss_1378.target.z = 0.0f;
    lbl_1_bss_1378.horizontalAngle = 0.0f;
    lbl_1_bss_1378.verticalAngle = 0.0f;
    lbl_1_bss_1378.unobservedValue = 0.0f;
    lbl_1_bss_1378.distance = 5e+03f;
    lbl_1_bss_1378.transparency = 1.0f;
    modelIndex = 0;
    while (modelIndex < 0x56) {
        if ((modelIndex != 2) && ((modelIndex < 0x1A) || (modelIndex > 0x4E)) && (modelIndex != 0x50) && (modelIndex != 0x51) && (modelIndex != 0x52) && (modelIndex != 0x53)) {
            fn_8002769C(lbl_1_bss_750[modelIndex].modelId, lbl_1_bss_1378.projectionId);
        }
        modelIndex += 1;
    }
    fn_1_2700();
}

typedef struct ProjectionVectors {
    Vec cameraUp;
    Vec cameraPosition;
} ProjectionVectors;

typedef struct M663ProjectionWorkVectors {
    Vec cameraPosition;
    Vec cameraUp;
} M663ProjectionWorkVectors;

/* Places the projection camera at its distance from the target along its angles, aims it
 * at the target and applies its transparency. */
void fn_1_2700(void) {
    ProjectionVectors vectors;

    vectors.cameraPosition.x = lbl_1_bss_1378.target.x + lbl_1_bss_1378.distance *
        (cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) * sin(M_PI * lbl_1_bss_1378.verticalAngle / 180.0));
    vectors.cameraPosition.y = lbl_1_bss_1378.target.y + lbl_1_bss_1378.distance *
        -sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0);
    vectors.cameraPosition.z = lbl_1_bss_1378.target.z + lbl_1_bss_1378.distance *
        (cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) * cos(M_PI * lbl_1_bss_1378.verticalAngle / 180.0));
    vectors.cameraUp.x = sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) * sin(M_PI * lbl_1_bss_1378.verticalAngle / 180.0);
    vectors.cameraUp.y = cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0);
    vectors.cameraUp.z = sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) * cos(M_PI * lbl_1_bss_1378.verticalAngle / 180.0);

    Hu3DProjectionPosSet(lbl_1_bss_1378.projectionId, &vectors.cameraPosition,
                         &vectors.cameraUp, &lbl_1_bss_1378.target);
    Hu3DProjectionTPLvlSet(lbl_1_bss_1378.projectionId,
                           lbl_1_bss_1378.transparency);
}

typedef struct WordTable {
    unsigned short words[4];
} WordTable;

extern void fn_1_FE8(void);

static inline void fn_1_1E98_for_2A00(void)
{
    lbl_1_bss_13EC.position.x =
        lbl_1_bss_13EC.aim.x +
        lbl_1_bss_13EC.range *
            (cos(M_PI * lbl_1_bss_13EC.orientation.x / 180.0) *
             sin(M_PI * lbl_1_bss_13EC.orientation.y / 180.0));
    lbl_1_bss_13EC.position.y =
        lbl_1_bss_13EC.aim.y +
        lbl_1_bss_13EC.range *
            -sin(M_PI * lbl_1_bss_13EC.orientation.x / 180.0);
    lbl_1_bss_13EC.position.z =
        lbl_1_bss_13EC.aim.z +
        lbl_1_bss_13EC.range *
            (cos(M_PI * lbl_1_bss_13EC.orientation.x / 180.0) *
             cos(M_PI * lbl_1_bss_13EC.orientation.y / 180.0));
    PSVECSubtract(&lbl_1_bss_13EC.aim, &lbl_1_bss_13EC.position,
                  &lbl_1_bss_13EC.direction);
    PSVECNormalize(&lbl_1_bss_13EC.direction, &lbl_1_bss_13EC.direction);
    Hu3DGLightPosSetV(lbl_1_bss_13EC.id, &lbl_1_bss_13EC.position,
                      &lbl_1_bss_13EC.direction);
    Hu3DGLightColorSet(lbl_1_bss_13EC.id, lbl_1_bss_13EC.color.r,
                       lbl_1_bss_13EC.color.g, lbl_1_bss_13EC.color.b,
                       lbl_1_bss_13EC.color.a);
}

static inline void fn_1_2214_for_2A00(void)
{
    lbl_1_bss_139C.cameraPosition.x = (f32)((f64)lbl_1_bss_139C.target.x +
        ((f64)lbl_1_bss_139C.distance *
         (cos((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
          sin((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0))));
    lbl_1_bss_139C.cameraPosition.y = (f32)((f64)lbl_1_bss_139C.target.y +
        ((f64)lbl_1_bss_139C.distance *
         -sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0)));
    lbl_1_bss_139C.cameraPosition.z = (f32)((f64)lbl_1_bss_139C.target.z +
        ((f64)lbl_1_bss_139C.distance *
         (cos((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
          cos((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0))));
    lbl_1_bss_139C.cameraUp.x = (f32)(
        sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
        sin((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0));
    lbl_1_bss_139C.cameraUp.y = (f32)cos(
        (M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0);
    lbl_1_bss_139C.cameraUp.z = (f32)(
        sin((M_PI * (f64)lbl_1_bss_139C.azimuth) / 180.0) *
        cos((M_PI * (f64)lbl_1_bss_139C.elevation) / 180.0));
    Hu3DShadowTPLvlSet(lbl_1_bss_139C.transparency);
    Hu3DShadowPosSet(&lbl_1_bss_139C.cameraPosition,
                     &lbl_1_bss_139C.cameraUp, &lbl_1_bss_139C.target);
}

static inline void fn_1_2700_for_2A00(M663ProjectionWorkVectors *vectors)
{
    vectors->cameraPosition.x = lbl_1_bss_1378.target.x + lbl_1_bss_1378.distance *
        (cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) *
         sin(M_PI * lbl_1_bss_1378.verticalAngle / 180.0));
    vectors->cameraPosition.y = lbl_1_bss_1378.target.y + lbl_1_bss_1378.distance *
        -sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0);
    vectors->cameraPosition.z = lbl_1_bss_1378.target.z + lbl_1_bss_1378.distance *
        (cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) *
         cos(M_PI * lbl_1_bss_1378.verticalAngle / 180.0));
    vectors->cameraUp.x = sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) *
        sin(M_PI * lbl_1_bss_1378.verticalAngle / 180.0);
    vectors->cameraUp.y = cos(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0);
    vectors->cameraUp.z = sin(M_PI * lbl_1_bss_1378.horizontalAngle / 180.0) *
        cos(M_PI * lbl_1_bss_1378.verticalAngle / 180.0);
    Hu3DProjectionPosSet(lbl_1_bss_1378.projectionId, &vectors->cameraPosition,
                         &vectors->cameraUp, &lbl_1_bss_1378.target);
    Hu3DProjectionTPLvlSet(lbl_1_bss_1378.projectionId,
                           lbl_1_bss_1378.transparency);
}

void fn_1_2A00(void)
{
    struct {
        M663ProjectionWorkVectors projection;
        Vec playerPosition;
    } work;
    s16 playerIndex;
    s32 modelIndex;

    fontcolor = 0xF;
    for (playerIndex = 0; playerIndex < 2; playerIndex++) {
        work.playerPosition.x = lbl_1_bss_4F0[playerIndex].position.x;
        work.playerPosition.y = lbl_1_bss_4F0[playerIndex].position.y;
        work.playerPosition.z = lbl_1_bss_4F0[playerIndex].position.z;

        fn_80086B04(
            lbl_1_bss_418.players[playerIndex]->actor,
            &work.playerPosition);
        lbl_1_bss_418.players[playerIndex]->actor->rotY =
            lbl_1_bss_4F0[playerIndex].rotation.y;
        lbl_1_bss_750[playerIndex].position = lbl_1_bss_4F0[playerIndex].position;
        lbl_1_bss_750[playerIndex].rotation = lbl_1_bss_4F0[playerIndex].rotation;
    }

    for (modelIndex = 2; modelIndex < 0x56; modelIndex++) {
        fn_80026760(lbl_1_bss_750[modelIndex].modelId,
                        lbl_1_bss_750[modelIndex].position.x,
                        lbl_1_bss_750[modelIndex].position.y,
                        lbl_1_bss_750[modelIndex].position.z);
        fn_80026808(lbl_1_bss_750[modelIndex].modelId,
                        lbl_1_bss_750[modelIndex].rotation.x,
                        lbl_1_bss_750[modelIndex].rotation.y,
                        lbl_1_bss_750[modelIndex].rotation.z);
    }

    for (modelIndex = 0; modelIndex <= 1; modelIndex++) {
        if (lbl_1_bss_4F0[modelIndex].transformMode == 0 ||
            lbl_1_bss_4F0[modelIndex].transformMode == 2) {
            fn_80026760(lbl_1_bss_750[modelIndex].modelId,
                            lbl_1_bss_4F0[modelIndex].position.x,
                            lbl_1_bss_4F0[modelIndex].position.y,
                            lbl_1_bss_4F0[modelIndex].position.z);
            fn_80026808(lbl_1_bss_750[modelIndex].modelId,
                            lbl_1_bss_4F0[modelIndex].rotation.x,
                            lbl_1_bss_4F0[modelIndex].rotation.y,
                            lbl_1_bss_4F0[modelIndex].rotation.z);
        }
    }

    fn_1_FE8();
    fn_1_1C90(0);

    fn_1_1E98_for_2A00();

    fn_1_2214_for_2A00();

    fn_1_2700_for_2A00(&work.projection);
}

/* For each player under actor control, sets COM or zeroed input, saves its previous position
 * and resets its per-frame collision fields, then runs the minigame actor update and copies
 * back each player's position and Y rotation. */
void fn_1_3588(void) {
    Point3d position;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 2) {
        if (lbl_1_bss_4F0[playerIndex].transformMode != 0) {
            if (lbl_1_bss_4F0[playerIndex].inputMode != 0) {
                fn_80080520(lbl_1_bss_418.players[playerIndex], 0, 0, 0, 0);
            } else if (lbl_1_bss_4F0[playerIndex].playerType != 0) {
                fn_80080520(lbl_1_bss_418.players[playerIndex], lbl_1_bss_4F0[playerIndex].resetValues[2], lbl_1_bss_4F0[playerIndex].resetValues[3], lbl_1_bss_4F0[playerIndex].resetValues[1], lbl_1_bss_4F0[playerIndex].resetValues[0]);
            }
            fn_80086BC8(lbl_1_bss_418.players[playerIndex]->actor, &position);
            lbl_1_bss_4F0[playerIndex].previousPosition.x = position.x;
            lbl_1_bss_4F0[playerIndex].previousPosition.y = position.y;
            lbl_1_bss_4F0[playerIndex].previousPosition.z = position.z;
            lbl_1_bss_4F0[playerIndex].previousMotionIndex = lbl_1_bss_4F0[playerIndex].motionIndex;
            lbl_1_bss_4F0[playerIndex].motionIndex = -1;
            if ((lbl_1_bss_4F0[playerIndex].previousPairedIndex != lbl_1_bss_4F0[playerIndex].pairedIndex) || (lbl_1_bss_4F0[playerIndex].pairedIndex == -1)) {
                lbl_1_bss_4F0[playerIndex].chaseTimer = 0;
            }
            lbl_1_bss_4F0[playerIndex].previousPairedIndex = lbl_1_bss_4F0[playerIndex].pairedIndex;
            lbl_1_bss_4F0[playerIndex].pairedIndex = -1;
        }
        playerIndex += 1;
    }
    fn_800856A8();
    playerIndex = 0;
    while (playerIndex < 2) {
        if (lbl_1_bss_4F0[playerIndex].transformMode != 0) {
            fn_80086BC8(lbl_1_bss_418.players[playerIndex]->actor, &position);
            lbl_1_bss_4F0[playerIndex].position.x = position.x;
            lbl_1_bss_4F0[playerIndex].position.y = position.y;
            lbl_1_bss_4F0[playerIndex].position.z = position.z;
            lbl_1_bss_4F0[playerIndex].rotation.y = (f32) lbl_1_bss_418.players[playerIndex]->actor->rotY;
        }
        playerIndex += 1;
    }
}

/* fn_1_38D4: mechanical copy of fn_1_2EA4 in m661dll (100% alike), names and constants adapted by copier.py; to be placed in its file. */

s32 fn_1_38D4(f32 *value, f32 step, f32 start, f32 end)
{
    f32 startDistance;
    f32 currentDistance;
    s32 frames = 0;
    f32 increment = (end - start) / step;

    *value += increment;
    if (increment) {
        frames = (s32)((end - *value) / increment);
    }
    frames++;
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
        frames = 0;
    }
    return frames;
}

/* Creates a minigame timer of the given type with start and end values in seconds, places
 * it and returns it. */
MGTIMER *fn_1_39F4(int type, int duration, int end, float x, float y) {
    MGTIMER *timer = MgTimerCreate(type);
    MgTimerParamSet(timer, duration * 60, end * 60, 0);
    MgTimerPosSet(timer, x, y);
    return timer;
}

void fn_1_3A68(MGTIMER *timer) {
    MgTimerModeOnSet(timer, 1);
}

void fn_1_3A94(MGTIMER *timer) {
    MgTimerDoneCheck(timer);
}

void fn_1_3ABC(MGTIMER *timerWork) {
    MgTimerDispOff(timerWork);
    MgTimerKill(timerWork);
}

void fn_1_3D8C(s16 player, s16 selection);
extern s32 printf();

/* Debug editor: holding L, X and Y step the selected index within lower..upper (wrapping),
 * the selection is edited, and pressing X and Y together prints the positions in range. */
void fn_1_3AF4(s16 player, s16 lower, s16 upper) {
    s16 index;

    if (lbl_802BFFEC[player] > 0x32) {
        if (lbl_802C0008[player] & 0x0400) {
            lbl_1_bss_424.effectTimer++;
        }
        if (lbl_802C0008[player] & 0x0800) {
            lbl_1_bss_424.effectTimer--;
        }
    }

    if (lbl_1_bss_424.effectTimer < lower) {
        lbl_1_bss_424.effectTimer = upper;
    }
    if (lbl_1_bss_424.effectTimer > upper) {
        lbl_1_bss_424.effectTimer = lower;
    }

    fn_1_3D8C(player, lbl_1_bss_424.effectTimer);

    if ((lbl_802C0008[player] & 0x0400) && (lbl_802C0008[player] & 0x0800)) {
        for (index = lower; index <= upper; index++) {
            if (index >= 0 && index <= 1) {
                printf("OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f\n",
                       index, lbl_1_bss_4F0[index].position.x,
                       lbl_1_bss_4F0[index].position.y,
                       lbl_1_bss_4F0[index].position.z,
                       lbl_1_bss_4F0[index].rotation.y);
            } else {
                printf("{{%.1ff,%.1ff,%.1ff},{0.0f,%.1ff,0.0f}},\n",
                       index, lbl_1_bss_750[index].position.x,
                       lbl_1_bss_750[index].position.y,
                       lbl_1_bss_750[index].position.z,
                       lbl_1_bss_750[index].rotation.y);
            }
        }
    }
}

void fn_1_3D8C(s16 controllerIndex, s16 subjectIndex)
{
    f32 movementStep = 5.0f;
    s16 textX = 0x19;
    s16 textY = 0x1AE;

    if (subjectIndex >= 0 && subjectIndex <= 1) {
        if ((lbl_802C0000[controllerIndex] & 0x400) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 0xA) {
                lbl_1_bss_4F0[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -0xA) {
                lbl_1_bss_4F0[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 0xA) {
                lbl_1_bss_4F0[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -0xA) {
                lbl_1_bss_4F0[subjectIndex].position.z += movementStep;
            }
        } else if ((lbl_802C0008[controllerIndex] & 0x400) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 0xA) {
                lbl_1_bss_4F0[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -0xA) {
                lbl_1_bss_4F0[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 0xA) {
                lbl_1_bss_4F0[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -0xA) {
                lbl_1_bss_4F0[subjectIndex].position.z += movementStep;
            }
        }
        if ((lbl_802C0008[controllerIndex] & 0x100) != 0) {
            lbl_1_bss_4F0[subjectIndex].rotation.y += 9e+01f;
        }
        if ((lbl_802C0008[controllerIndex] & 0x200) != 0) {
            lbl_1_bss_4F0[subjectIndex].rotation.y -= 9e+01f;
        }
        print8(textX, textY, 1.5f,
               "OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               subjectIndex, lbl_1_bss_4F0[subjectIndex].position.x,
               lbl_1_bss_4F0[subjectIndex].position.y,
               lbl_1_bss_4F0[subjectIndex].position.z,
               lbl_1_bss_4F0[subjectIndex].rotation.y);
        return;
    }

    {
        if ((lbl_802C0000[controllerIndex] & 0x400) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 0xA) {
                lbl_1_bss_750[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -0xA) {
                lbl_1_bss_750[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 0xA) {
                lbl_1_bss_750[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -0xA) {
                lbl_1_bss_750[subjectIndex].position.z += movementStep;
            }
        } else if ((lbl_802C0008[controllerIndex] & 0x400) != 0) {
            if (lbl_802BFFFC[controllerIndex] > 0xA) {
                lbl_1_bss_750[subjectIndex].position.x += movementStep;
            }
            if (lbl_802BFFFC[controllerIndex] < -0xA) {
                lbl_1_bss_750[subjectIndex].position.x -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] > 0xA) {
                lbl_1_bss_750[subjectIndex].position.z -= movementStep;
            }
            if (lbl_802BFFF8[controllerIndex] < -0xA) {
                lbl_1_bss_750[subjectIndex].position.z += movementStep;
            }
        }
        if ((lbl_802C0008[controllerIndex] & 0x100) != 0) {
            lbl_1_bss_750[subjectIndex].rotation.y += 9e+01f;
        }
        if ((lbl_802C0008[controllerIndex] & 0x200) != 0) {
            lbl_1_bss_750[subjectIndex].rotation.y -= 9e+01f;
        }
        print8(textX, textY, 1.5f,
               "    OBJ W%d X:%.1f Y:%.1f Z:%.1f ROTY:%.1f",
               subjectIndex, lbl_1_bss_750[subjectIndex].position.x,
               lbl_1_bss_750[subjectIndex].position.y,
               lbl_1_bss_750[subjectIndex].position.z,
               lbl_1_bss_750[subjectIndex].rotation.y);
    }
}

/* Adds a local light to a model's first free light slot and sets model attribute 0x1000;
 * returns 1, or -1 (printing a message when no slot is free) if it cannot be added. */
s16 fn_1_44F8(s16 modelIndex, s16 lightId) {
    s16 result = 0;
    HU3D_MODEL *model = &lbl_802C01C8[modelIndex];
    s16 slot = 0;

    while (slot < 8) {
        if (model->LLightId[slot] == lightId) {
            result = -1;
            break;
        }
        slot++;
    }

    if (result != -1) {
        slot = 0;
        while (slot < 8) {
            if (model->LLightId[slot] == -1) {
                model->LLightId[slot] = lightId;
                model->attr |= 0x1000;
                result = 1;
                break;
            }
            slot++;
        }
    }

    if (result == 0) {
        printf("ModelIntoLLight False model:%d light:%d\n", modelIndex, lightId);
        result = -1;
    }
    return result;
}

/* Removes a local light from a model's light slots; returns 1 if found, otherwise -1. */
s32 fn_1_4610(HU3D_MODELID modelId, HU3D_LIGHTID lightId) {
    HU3D_MODEL *model;
    s32 result;
    s16 index;

    result = -1;
    model = &lbl_802C01C8[modelId];
    index = 0;
    while (index < 8) {
        if (model->LLightId[index] == lightId) {
            model->LLightId[index] = -1;
            result = 1;
            break;
        }
        index++;
    }
    return result;
}

/* Returns a random whole number from 0 to below the given value, as a float. */
f32 fn_1_46A8(f32 value) {
    s32 result = value;
    result = frandmod(result);
    value = result;
    return value;
}

/* Creates the two particle effect models with their animations, pools, layers, hooks,
 * cameras and blend modes, and clears every particle. */
void fn_1_4720(void) {
    u32 dataNumbers[2] = {0x007A0021, 0x007A0022};
    s16 maximumParticles[2] = {500, 2};
    s16 layers[2] = {6, 7};
    s16 cameras[2] = {1, 1};
    s16 blendModes[2] = {0, 0};
    HU3D_PARTICLE_HOOK hooks[2] = {
        (HU3D_PARTICLE_HOOK)fn_1_5294,
        (HU3D_PARTICLE_HOOK)fn_1_54C8
    };
    s16 effectIndex;

    for (effectIndex = 0; effectIndex < 2; effectIndex++) {
        ANIMDATA *animation;
        HU3D_PARTICLE *context;
        HU3D_PARTICLE_DATA *entry;
        s16 particleIndex;

        lbl_1_bss_1368.maximumCounts[effectIndex] = maximumParticles[effectIndex];
        animation = HuSprAnimRead(fn_80008DD0(
            dataNumbers[effectIndex], 0x10000000, 2));
        lbl_1_bss_1368.allocation[effectIndex] = HuMemDirectMallocNum(
            HEAP_HEAP, maximumParticles[effectIndex] * 0x3C, 0x10000000);
        lbl_1_bss_1368.modelIds[effectIndex] = fn_8003216C(
            animation, maximumParticles[effectIndex]);
        fn_80026B8C(lbl_1_bss_1368.modelIds[effectIndex], layers[effectIndex]);
        fn_800326D0(lbl_1_bss_1368.modelIds[effectIndex], hooks[effectIndex]);
        fn_80026B64(lbl_1_bss_1368.modelIds[effectIndex], cameras[effectIndex]);
        fn_8003269C(lbl_1_bss_1368.modelIds[effectIndex],
                                 (u8)blendModes[effectIndex]);

        context = lbl_802C01C8[lbl_1_bss_1368.modelIds[effectIndex]].hookData;
        context->emitCnt = 0;
        context->work = lbl_1_bss_1368.allocation[effectIndex];
        context->count = 0;
        particleIndex = 0;
        entry = context->data;
        for (; particleIndex < context->maxCnt; particleIndex++, entry++) {
            entry->scale = 0.0f;
            entry->time = 0;
        }
    }
}

static inline f32 fn_1_46A8_for_49F8(f32 value)
{
    s32 result = value;

    result = frandmod(result);
    value = result;
    return value;
}

void fn_1_49F8(Vec *origin, f32 baseAngle)
{
    M663EffectParameters parameters;
    Vec velocity;
    Vec position;
    f32 alpha;
    f32 angle[4];
    f32 angleDeviation;
    s16 effectState = 0x3E8;
    s16 effectIndex;
    f32 speed;

    parameters.unobservedValue = 0;
    parameters.color.r = 0xFF;
    parameters.color.g = 0xFF;
    parameters.color.b = 0xFF;
    parameters.color.a = 0xC;
    parameters.fields14[0] = -1;
    parameters.fields14[1] = -1;
    parameters.fields14[2] = -1;
    parameters.fields14[3] = 0;
    parameters.fields24[1] = 0.0f;
    parameters.opacityChange = -1.0f;
    parameters.fields38 = 0.0f;
    parameters.fields24[0] = 0.0f;

    alpha = 3e+01f +
        fn_1_46A8_for_49F8(1e+01f);
    parameters.verticalPosition = 1.0f +
        (fn_1_46A8_for_49F8(2e+01f) / 1e+01f);
    parameters.verticalScale = 1.0f;

    angleDeviation = fn_1_46A8_for_49F8(45.0f);
    angle[0] = baseAngle + angleDeviation;
    angle[1] = baseAngle - angleDeviation;
    angleDeviation = 6e+01f + fn_1_46A8_for_49F8(6e+01f);
    angle[2] = baseAngle + angleDeviation;
    angle[3] = baseAngle - angleDeviation;

    for (effectIndex = 0; effectIndex < 2; effectIndex++) {
        position.x = origin->x + 60.0 *
            sin((M_PI * angle[effectIndex]) / 180.0);
        position.y = origin->y;
        position.z = origin->z + 60.0 *
            cos((M_PI * angle[effectIndex]) / 180.0);
        speed = fn_1_46A8_for_49F8(3e+01f) / 1e+01f;
        velocity.x = (f64)speed * sin((M_PI * angle[effectIndex + 2]) /
                                      180.0);
        velocity.y = 0.0f;
        velocity.z = (f64)speed * cos((M_PI * angle[effectIndex + 2]) /
                                      180.0);
        parameters.velocityScale[0] = 0.98f;
        parameters.velocityScale[1] = 1.0f;
        parameters.velocityScale[2] = 0.98f;
        fn_1_5094(0, -1, position, velocity, alpha, effectState, &parameters);
    }
}

/* Starts one particle of the second effect for each player; its hook makes it follow that
 * player. */
void fn_1_4FA8(void) {
    Vec from = {0.0f, 0.0f, 0.0f};
    Vec to = {0.0f, 0.0f, 0.0f};
    M663EffectParameters camera;
    s16 player;

    for (player = 0; player < 2; player++) {
        fn_1_5094(1, player, to, from, 0.0f, 1, &camera);
    }
}

s16 fn_1_5094(s16 modelNumber, s16 ownerIndex, Vec position, Vec velocity,
              f32 alpha, s16 state, M663EffectParameters *parameters) {
    HU3D_MODEL *model;
    s16 result;
    M663EffectContext *context;
    M663EffectParameters *currentParameters;
    s16 entryIndex;
    M663EffectEntry *entry;

    result = -1;
    model = &lbl_802C01C8[lbl_1_bss_1368.modelIds[modelNumber]];
    context = model->hookData;
    currentParameters = context->parameters;
    entryIndex = 0;
    entry = context->entries;

    while (entryIndex < context->entryCount) {
        if (entry->state <= 0) {
            currentParameters[entryIndex] = *parameters;
            currentParameters[entryIndex].fields04[2] =
                currentParameters[entryIndex].fields04[1] =
                    currentParameters[entryIndex].fields04[0] = 0.0f;

            entry->position = position;
            entry->velocity = velocity;
            entry->origin = position;
            entry->fields20[2] = entry->fields20[1] =
                entry->fields20[0] = 0.0f;
            entry->color = currentParameters[entryIndex].color;
            entry->height = alpha;
            entry->state = state;
            entry->ownerIndex = ownerIndex;
            context->lastEntry = entryIndex;
            result = entryIndex;
            break;
        }
        entryIndex++;
        entry++;
    }
    return result;
}

/* Particle hook for the first effect: moves live particles with damped velocity, fades them
 * every third frame, updates their height, and ends those that fade out or reach zero. */
void fn_1_5294(s32, M663EffectContext *context) {
    M663EffectParameters *parameters = context->parameters;
    M663EffectEntry *entry = context->entries;
    s16 index;

    for (index = 0; index < context->entryCount; index++, entry++) {
        if (entry->state-- > 0) {
            entry->velocity.x *= parameters[index].velocityScale[0];
            entry->velocity.y *= parameters[index].velocityScale[1];
            entry->velocity.z *= parameters[index].velocityScale[2];
            PSVECAdd(&entry->velocity, &entry->position, &entry->position);

            if (entry->state % 3 == 0) {
                s16 opacity = (s16)((f32)entry->color.a + parameters[index].opacityChange);
                if (opacity < 0) {
                    opacity = 0;
                } else if (opacity > 0xFF) {
                    opacity = 0xFF;
                }
                entry->color.a = (u8)opacity;
                if (opacity <= 0) {
                    entry->state = 0;
                    entry->height = 0.0f;
                }
            }

            parameters[index].verticalPosition *= parameters[index].verticalScale;
            entry->height += parameters[index].verticalPosition;
            if (entry->height <= 0.0f) {
                entry->height = 0.0f;
                entry->state = 0;
            }

            DCStoreRangeNoSync(context->entries, context->entryCount * 0x48);
        } else {
            entry->height = 0.0f;
            entry->state = -1;
        }
    }
}

/* Particle hook for the second effect: when rumble is off, fades each player's marker in
 * while that player is idle near the goal and no grab is pending, otherwise fades it out;
 * the marker follows the player and bobs its height between 70 and 100. */
void fn_1_54C8(s32, M663EffectContext *context) {
    struct M663EffectParameters *positionOffsets = context->parameters;
    M663PlayerWork *player;
    M663EffectEntry *entry = context->entries;
    s16 entryIndex;

    if (lbl_1_bss_424.rumbleEnabled == 0) {
      for (entryIndex = 0; entryIndex < context->entryCount; entryIndex++, entry++) {
        player = &lbl_1_bss_4F0[entry->ownerIndex];

        if (player->skipApproach != 0 && player->score == 0 && lbl_1_bss_424.winnerReady == 0 && lbl_1_bss_4F0[entryIndex ^ 1].scoreStatus == 0) {
                if ((s32)entry->color.a + 0x10 >= 0xFF) {
                    entry->color.a = 0xFF;
                } else {
                    entry->color.a += 0x10;
                }
                if (entry->state == 1) {
                    entry->state = 2;
                }
        } else {
            entry->state = 1;
            if ((s32)entry->color.a - 0x30 <= 0) {
                entry->color.a = 0;
            } else {
                entry->color.a -= 0x30;
            }
        }

            entry->position = player->position;
            entry->position.y = 2.5e+02f;
            entry->height += positionOffsets[entryIndex].verticalPosition;
            if (entry->height <= 7e+01f) {
                entry->height = 7e+01f;
                positionOffsets[entryIndex].verticalPosition = 3.0f;
            }
            if (entry->height >= 1e+02f) {
                entry->height = 1e+02f;
                positionOffsets[entryIndex].verticalPosition = -3.0f;
            }
        entry->color.r = entry->color.g = entry->color.b = 0xFF;
      }
    }
}

/* Draw hook: draws the six "kiri" objects of models 80-85 from each model's position,
 * rotation and scale, skipping any whose control has reached mode 2. */
void fn_1_56E8(s32, Mtx *viewMatrix) {
    char objectNames[6][0x20] = {"kiri1", "kiri2", "kiri3", "kiri4", "kiri5", "kiri6"};
    f32 transform[3][4];
    f32 rotation[3][4];
    s16 objectIndex;
    s16 playerIndex;

    fn_8002472C();

    for (objectIndex = 0x50, playerIndex = 0; objectIndex <= 0x55; objectIndex++, playerIndex++) {
        M663ModelWork *object = &lbl_1_bss_750[objectIndex];
        HU3D_MODEL *model = &lbl_802C01C8[object->modelId];

        if (lbl_1_bss_0[playerIndex].mode < 2) {
            fn_80022E38(rotation, model->rot.x, model->rot.y, model->rot.z);
            PSMTXTrans(transform, model->pos.x, model->pos.y, model->pos.z);
            PSMTXConcat(transform, rotation, transform);
            mtxScaleCat(transform, object->scale, object->scale, object->scale);
            PSMTXConcat(transform, rotation, transform);
            PSMTXConcat(*viewMatrix, transform, transform);
            fn_800247C4(object->modelId, objectNames[playerIndex], transform);
        }
    }
}

void fn_1_584C(M663PlayerWork *character, s16 mode)
{
    s16 pan;
    s16 volume;
    f32 panScale;
    f32 volumeScale;
    f32 positionOffset;

    if (mode == 0 || mode == 1 || mode == 6) {
        if (lbl_1_bss_424.maxX >= character->position.x &&
            lbl_1_bss_424.minX <= character->position.x) {
            panScale =
                (f32)(lbl_1_bss_424.maxPanX - lbl_1_bss_424.minPanX) /
                (lbl_1_bss_424.maxX - lbl_1_bss_424.minX);
            positionOffset = character->position.x - lbl_1_bss_424.minX;
            positionOffset *= panScale;
            pan = (s16)positionOffset;
            pan += lbl_1_bss_424.minPanX;
        } else if (lbl_1_bss_424.maxX < character->position.x) {
            pan = lbl_1_bss_424.maxPanX;
        } else if (lbl_1_bss_424.minX > character->position.x) {
            pan = lbl_1_bss_424.minPanX;
        }
    }

    if (mode == 0 || mode == 2) {
        if (lbl_1_bss_424.maxY >= character->position.z &&
            lbl_1_bss_424.minY <= character->position.z) {
            volumeScale =
                (f32)(lbl_1_bss_424.maxPanY - lbl_1_bss_424.minPanY) /
                (lbl_1_bss_424.maxY - lbl_1_bss_424.minY);
            positionOffset = character->position.z - lbl_1_bss_424.minY;
            positionOffset *= volumeScale;
            volume = (s16)positionOffset;
            volume += lbl_1_bss_424.minPanY;
        } else if (lbl_1_bss_424.maxY < character->position.z) {
            volume = lbl_1_bss_424.maxPanY;
        } else if (lbl_1_bss_424.minY > character->position.z) {
            volume = lbl_1_bss_424.minPanY;
        }
    }

    if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
        pan = lbl_1_bss_424.minPanX +
              (lbl_1_bss_424.maxPanX - lbl_1_bss_424.minPanX) / 2;
    }
    if (mode == 1 || mode == 3) {
        volume = lbl_1_bss_424.minPanY +
                 (lbl_1_bss_424.maxPanY - lbl_1_bss_424.minPanY) / 2;
    }
    if (mode == 4) {
        volume = lbl_1_bss_424.minPanY;
    }
    if (mode == 5 || mode == 6) {
        volume = lbl_1_bss_424.maxPanY;
    }

    fn_80060F14(character->characterNo, pan);
    if (mode != 5 && mode != 6) {
        fn_80060F40(character->characterNo, volume);
    }
    if (character->requestPending == 1) {
        u32 soundIds[14] = {
            MSM_SE_CHARVOICE_HAPPY_CHEER, MSM_SE_CHARVOICE_LUIGI_HAPPY_CHEER,
            MSM_SE_CHARVOICE_PEACH_HAPPY_CHEER, MSM_SE_CHARVOICE_YOSHI_HAPPY_CHEER,
            MSM_SE_CHARVOICE_WARIO_HAPPY_CHEER, MSM_SE_CHARVOICE_DAISY_HAPPY_CHEER,
            MSM_SE_CHARVOICE_WALUIGI_HAPPY_CHEER, MSM_SE_CHARVOICE_KINOPIO_HAPPY_CHEER,
            MSM_SE_CHARVOICE_TERESA_HAPPY_CHEER, MSM_SE_CHARVOICE_MINIKOOPA_HAPPY_CHEER,
            MSM_SE_CHARVOICE_KINOPIKO_HAPPY_CHEER, MSM_SE_CHARVOICE_MINIKOOPAR_HAPPY_CHEER,
            MSM_SE_CHARVOICE_MINIKOOPAG_HAPPY_CHEER, MSM_SE_CHARVOICE_MINIKOOPAB_HAPPY_CHEER
        };

        HuAudFXPlayVolPan(soundIds[(s8)character->characterNo], volume, pan);
        character->requestPending = -1;
    }
}

/* Sets the default BGM and jingle, the pan and volume ranges over the -550..550 play area,
 * and clears the sound effect slots. */
void fn_1_5D14(void) {
    s16 soundIndex;

    lbl_1_bss_424.bgmHandle = -1;
    lbl_1_bss_424.jingleHandle = -1;
    lbl_1_bss_424.bgmId = MSM_STREAM_NIGHT_DUEL;
    lbl_1_bss_424.jingleId = -1;
    lbl_1_bss_424.minPanX = 0x30;
    lbl_1_bss_424.maxPanX = 0x50;
    lbl_1_bss_424.minX = (f32) -5.5e+02f;
    lbl_1_bss_424.maxX = 5.5e+02f;
    lbl_1_bss_424.minPanY = 0x50;
    lbl_1_bss_424.maxPanY = 0x6E;
    lbl_1_bss_424.minY = (f32) -5.5e+02f;
    lbl_1_bss_424.maxY = 5.5e+02f;
    soundIndex = 0;
    while (soundIndex < 4) {
        lbl_1_bss_424.soundActive[soundIndex] = -1;
        lbl_1_bss_424.soundHandles[soundIndex] = -1;
        soundIndex += 1;
    }
}

void fn_1_5E58(void) {
    if (((s32) lbl_1_bss_424.jingleId != -1) && ((s32) lbl_1_bss_424.jingleHandle == -1)) {
        lbl_1_bss_424.jingleHandle = HuAudJinglePlay((s16) lbl_1_bss_424.jingleId);
    }
}

void fn_1_5EBC(void) {
    if ((s32) lbl_1_bss_424.bgmHandle == -1) {
        lbl_1_bss_424.bgmHandle = HuAudBGMPlay((s16) lbl_1_bss_424.bgmId);
    }
}

s16 lbl_1_data_F6[4] = { MSM_SE_MG663_PLAY_START_AMBIENCE, MSM_SE_MG663_BACKGROUND_ACTOR_MOTION,
                         MSM_SE_MG663_GOAL_GRAB, MSM_SE_MG663_GOAL_CAPTURE };

/* Fades out the BGM (nonzero slot) or jingle stream over the given frames and clears its
 * handle. */
void fn_1_5F0C(s16 streamSlot, s16 fadeFrames) {
    s16 streamId = streamSlot != 0
                       ? lbl_1_bss_424.bgmHandle
                       : lbl_1_bss_424.jingleHandle;
    if (streamId != -1) {
        HuAudSStreamFadeOut(streamId, fadeFrames);
        if (streamSlot != 0) {
            lbl_1_bss_424.bgmHandle = -1;
            return;
        }
        lbl_1_bss_424.jingleHandle = -1;
    }
}

/* Stops the BGM stream when mode is nonzero, otherwise the jingle stream. */
void fn_1_5FB8(s16 mode) {
    s16 streamId;

    if (mode != 0) {
        streamId = (s16)lbl_1_bss_424.bgmHandle;
    } else {
        streamId = (s16)lbl_1_bss_424.jingleHandle;
    }
    HuAudSStreamStop(streamId);
}

void fn_1_6018(s16 playerIndex, s16 mode, Vec *position)
{
    float xPanScale;
    float zPanScale;
    float positionOffset;
    s16 panY;
    s16 panX;

    if ((playerIndex != 0 || lbl_1_bss_424.soundActive[playerIndex] == -1) && mode <= 7) {
        if (mode == 0 || mode == 1 || mode == 6) {
            if (lbl_1_bss_424.maxX >= position->x && lbl_1_bss_424.minX <= position->x) {
                xPanScale = (float)(lbl_1_bss_424.maxPanX - lbl_1_bss_424.minPanX) /
                    (lbl_1_bss_424.maxX - lbl_1_bss_424.minX);
                positionOffset = position->x - lbl_1_bss_424.minX;
                positionOffset *= xPanScale;
                panX = (s16)positionOffset;
                panX += lbl_1_bss_424.minPanX;
            } else if (lbl_1_bss_424.maxX < position->x) {
                panX = lbl_1_bss_424.maxPanX;
            } else if (lbl_1_bss_424.minX > position->x) {
                panX = lbl_1_bss_424.minPanX;
            }
        }
        if (mode == 0 || mode == 2) {
            if (lbl_1_bss_424.maxY >= position->z && lbl_1_bss_424.minY <= position->z) {
                zPanScale = (float)(lbl_1_bss_424.maxPanY - lbl_1_bss_424.minPanY) /
                    (lbl_1_bss_424.maxY - lbl_1_bss_424.minY);
                positionOffset = position->z - lbl_1_bss_424.minY;
                positionOffset *= zPanScale;
                panY = (s16)positionOffset;
                panY += lbl_1_bss_424.minPanY;
            } else if (lbl_1_bss_424.maxY < position->z) {
                panY = lbl_1_bss_424.maxPanY;
            } else if (lbl_1_bss_424.minY > position->z) {
                panY = lbl_1_bss_424.minPanY;
            }
        }
        if (mode == 2 || mode == 3 || mode == 4 || mode == 5) {
            panX = (s16)(lbl_1_bss_424.minPanX +
                (lbl_1_bss_424.maxPanX - lbl_1_bss_424.minPanX) / 2);
        }
        if (mode == 1 || mode == 3) {
            panY = (s16)(lbl_1_bss_424.minPanY +
                (lbl_1_bss_424.maxPanY - lbl_1_bss_424.minPanY) / 2);
        }
        if (mode == 4) {
            panY = lbl_1_bss_424.minPanY;
        }
        if (mode == 5 || mode == 6 || mode == 7) {
            panY = lbl_1_bss_424.maxPanY;
        }
        if (mode == 7) {
            panX = lbl_1_bss_424.centerPanX;
        }
        lbl_1_bss_424.soundActive[playerIndex] = 1;
        lbl_1_bss_424.soundHandles[playerIndex] = HuAudFXPlayVolPan(lbl_1_data_F6[playerIndex], panY, panX);
    }
}
