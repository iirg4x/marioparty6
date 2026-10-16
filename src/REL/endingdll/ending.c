/* Runs the ending scene, including its characters, effects, lighting, and windows. */
#include "datadir_enum.h"
#include "messdir_enum.h"
#include "msm.h"
#include "msm_se.h"
#include "msm_stream.h"
#include "dolphin.h"
#include "game/armem.h"

#include "game/gamework.h"
#include "game/pad.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/process.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/wipe.h"

typedef void (*VoidFunc)(void);

typedef struct EndingWindowPlayers {
    s16 player[4];
} EndingWindowPlayers;

typedef struct EndingSpritePositions {
    Vec position[10];
} EndingSpritePositions;

typedef struct EndingModelObjectNames {
    char *name[10];
} EndingModelObjectNames;

typedef struct EndingAudioState {
    s32 channel[32];
} EndingAudioState;

typedef struct EndingLightVectors {
    HuVecF vector[2];
} EndingLightVectors;

typedef struct EndingParticleCounts {
    s16 count[5];
} EndingParticleCounts;

typedef struct EndingScenePositions {
    HuVecF position[3];
} EndingScenePositions;

typedef struct EndingMotionWork {
    s16 state; /* 0 is inactive; 1 is moving for the particle and entrance work. */
    float time; /* Callback time in frames; negative values can encode a delayed start or hidden
                 * effect state. */
    float duration; /* Total animation frames for the interpolation. */
    HuVecF startOrVelocity; /* Starting point for a path, or particle velocity per update. */
    HuVecF controlOrPosition; /* Path control point, hook position, or particle position. */
    HuVecF endOrEffectParams; /* Path endpoint, or particle scale in x and yaw step in y. */
    float start; /* Starting yaw in degrees or starting x position, depending on callback. */
    float end; /* Target yaw in degrees or target x position, depending on callback. */
    float finalMotionStarted; /* Zero until the character's follow-up motion starts. */
    float reserved; /* Retained storage with no reads or writes in this source file. */
} EndingMotionWork;

#define sind(x) sin((M_PI * (x)) / 180.0)
#define ENDING_ANIMATION_ALLOC_TAG 805306368
#define ENDING_FX_ID_1393 1393
#define ENDING_FX_ID_1394 1394
#define ENDING_FX_ID_1396 1396
#define ENDING_FX_ID_1397 1397
#define ENDING_FX_ID_1399 1399
#define ENDING_FX_ID_1400 1400

extern const VoidFunc _ctors[];
extern const VoidFunc _dtors[];

s32 lbl_1_bss_1DDC;
EndingAudioState lbl_1_bss_1D5C;
EndingMotionWork lbl_1_bss_1ADC[10];
EndingMotionWork lbl_1_bss_1A5C[2];
s16 lbl_1_bss_1A56[2];
s16 lbl_1_bss_1A52[2];
s16 lbl_1_bss_1A48[5];
ANIMDATA *lbl_1_bss_1A20[10];
s16 lbl_1_bss_1A1C[2];
s16 lbl_1_bss_1A08[10];
s16 lbl_1_bss_19F4[10];
EndingMotionWork lbl_1_bss_1834[7];
EndingMotionWork lbl_1_bss_34[96];
s16 lbl_1_bss_30;
s16 lbl_1_bss_2E;
s16 lbl_1_bss_2C;
float lbl_1_bss_28;
s16 lbl_1_bss_26;
s16 lbl_1_bss_24;
OMOBJ *lbl_1_bss_20;
OMOBJ *lbl_1_bss_1C;
OMOBJ *lbl_1_bss_18;
OMOBJ *lbl_1_bss_14;
OMOBJ *lbl_1_bss_10;
OMOBJ *lbl_1_bss_C;
OMOBJ *lbl_1_bss_8;
OMOBJ *lbl_1_bss_4;
OMOBJMAN *lbl_1_bss_0;
extern s16 lbl_1_bss_1E1C;
extern s16 lbl_1_bss_1E1E[2];
extern s16 lbl_1_bss_1E22[2];
extern s16 lbl_1_bss_1E26[2];
extern s16 lbl_1_bss_1E2A;
extern s16 lbl_1_bss_1E2C;
extern s16 lbl_1_bss_1DE0[6][5];
extern ANIMDATA *lbl_1_bss_1E30[3];

extern EndingLightVectors lbl_1_rodata_A0;
extern GXColor lbl_1_rodata_B8;

extern EndingParticleCounts lbl_1_rodata_390;

extern u32 lbl_1_data_128[3];

int rand8(void);
s32 frandmod(s32 modulus);
void fn_1_98(HUWINID window, u32 message, s16 character);
void fn_1_25C(OMOBJ *object);
void fn_1_1C0C(s16 index, s16 motion, float time);
void fn_1_10DEC(s16 index, HuVecF *pos, s16 mode);
void fn_1_DD14(void);
void fn_1_F1B8(s16 display, HuVecF *pos);
void fn_1_12838(void);
void fn_1_E1EC(s16 display, HuVecF *pos);
void fn_1_EAB8(s16 display, HuVecF *pos);
void fn_1_F068(s16 index, s16 display, HuVecF *pos);
void fn_1_F11C(s16 index, s16 count);
void fn_1_F23C(s16 count);
void fn_1_111B0(s16 display);
void fn_1_11208(s16 index, HuVecF *pos, s16 mode);
void fn_1_E0EC(s16 groupId, u32 attr);
void fn_1_EB54(s16 time);
void fn_1_1160(OMOBJ *object);
void fn_1_3B4C(OMOBJ *object);
void fn_1_3D5C(OMOBJ *object);
void fn_1_36E0(OMOBJ *object);
void fn_1_23D8(OMOBJ *object);
void fn_1_2AEC(OMOBJ *object);
void fn_1_E270(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_EBCC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_F2CC(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_FDD4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_10628(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_112E0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_11714(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx);
void fn_1_702C(void);
void fn_1_8F80(OMOBJ *object);
void fn_1_A964(void);
void fn_1_B568(void);
void fn_1_BE30(void);
void fn_1_C7DC(void);
void fn_1_E16C(HUSPR_GROUPID groupId, s32 attr);
void fn_1_D434(void);
void fn_1_7B4(void);
void fn_1_45C0(OMOBJ *object);
void fn_1_4BC8(OMOBJ *object);
void fn_1_11F34(void);
void fn_1_DF60(HuVecF *dest, HuVecF *target, float time);
float fn_1_DDF8(float start, float end, float time, float duration);

/* Plays ending sound effects while audio is enabled; scene callbacks use this wrapper. */
int fn_1_0(int seId)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlay(seId);
    }
    return -1;
}

/* Plays an ending sound at the requested volume and pan when audio is enabled; character setup uses
 * this wrapper. */
int fn_1_44(int seId, s16 volume, s16 pan)
{
    if (lbl_1_bss_26 == 0) {
        return HuAudFXPlayVolPan(seId, volume, pan);
    }
    return -1;
}

inline int fn_1_44(int seId, s16 volume, s16 pan);

u32 lbl_1_data_0 = -1;

/* Window callback: pans the new voice line and avoids replaying its sound; runs on message
 * changes. */
void fn_1_98(HUWINID winId, u32 mess, s16 index)
{
    s32 messNum[5] = {
        MESSNUM(MESS_ENDING, 1),
        MESSNUM(MESS_ENDING, 4),
        MESSNUM(MESS_ENDING, 5),
        MESSNUM(MESS_ENDING, 8),
        -1,
    };
    s32 fxNum[16] = {
        MSM_SE_GUIDE_25, MSM_SE_GUIDE_26, MSM_SE_GUIDE_27,
        MSM_SE_GUIDE_28, MSM_SE_GUIDE_29, MSM_SE_GUIDE_30,
        MSM_SE_GUIDE_31, -1,
        MSM_SE_GUIDE_17, MSM_SE_GUIDE_18, MSM_SE_GUIDE_19,
        MSM_SE_GUIDE_20, MSM_SE_GUIDE_21, MSM_SE_GUIDE_22,
        MSM_SE_GUIDE_23, -1,
    };
    s16 i;

    index--;
    OSReport("# ========== win callback :: %d\n", index);
    if (lbl_1_data_0 != mess) {
        lbl_1_data_0 = mess;
        for (i = 0;; i++) {
            if (messNum[i] == -1) {
                HuAudFXPlay(fxNum[index]);
                break;
            }
            if (mess == messNum[i]) {
                if (index >= 8) {
                    HuAudFXPlayPan(
                        fxNum[index], (MSM_PAN_CENTER + MSM_PAN_RIGHT) / 2);
                } else {
                    HuAudFXPlayPan(
                        fxNum[index], (MSM_PAN_LEFT + MSM_PAN_CENTER) / 2);
                }
                break;
            }
        }
    }
}

/* Character object callback: moves both item hooks around their models each update. */
void fn_1_25C(OMOBJ *object)
{
    HuVecF target;
    OMOBJ *objects[2];
    EndingMotionWork *firstWork;
    EndingMotionWork *secondWork;

    objects[1] = lbl_1_bss_C;
    objects[0] = lbl_1_bss_10;
    firstWork = &lbl_1_bss_1A5C[0];
    secondWork = &lbl_1_bss_1A5C[1];

    target.x = sin(((3.141592653589793) * object->work[0]) /
        (180.0)) * (180.0);
    target.y = (10.0f) + firstWork->startOrVelocity.y;
    if (target.y > (400.0f)) {
        target.y = (400.0f);
    }
    target.z = cos(((3.141592653589793) * object->work[0]) /
        (180.0)) * (180.0);
    fn_1_DF60(&firstWork->startOrVelocity, &target, (10.0f));

    target.x = sin(((3.141592653589793) * object->work[1]) /
        (180.0)) * (180.0);
    target.y = (10.0f) + firstWork->startOrVelocity.y;
    if (target.y > (400.0f)) {
        target.y = (400.0f);
    }
    target.z = cos(((3.141592653589793) * object->work[1]) /
        (180.0)) * (180.0);
    fn_1_DF60(&secondWork->startOrVelocity, &target, (10.0f));

    if ((object->work[0] += 2) > 360) {
        object->work[0] -= 360;
    }
    if ((object->work[1] += 2) > 360) {
        object->work[1] -= 360;
    }
    fn_1_10DEC(0, &firstWork->startOrVelocity, 1);
    fn_1_10DEC(1, &secondWork->startOrVelocity, 1);
}

/* Character object callback: tracks the item hooks, then starts their orbit after 61 updates. */
void fn_1_558(OMOBJ *object)
{
    OMOBJ *object0 = lbl_1_bss_C;
    OMOBJ *object1 = lbl_1_bss_10;
    EndingMotionWork *work0 = &lbl_1_bss_1A5C[0];
    EndingMotionWork *work1 = &lbl_1_bss_1A5C[1];

    Hu3DModelObjPosGet(object0->mdlId[0], "gN00m1-itemhook_R",
        &work0->startOrVelocity);
    Hu3DModelObjPosGet(object1->mdlId[0], "gN00m1-itemhook_R",
        &work1->startOrVelocity);
    fn_1_10DEC(0, &work0->startOrVelocity, 1);
    fn_1_10DEC(1, &work1->startOrVelocity, 1);
    if (++object->work[2] > 60) {
        fn_1_0(1407);
        fn_1_0(1406);
        object->work[2] = 0;
        object->objFunc = fn_1_25C;
    }
}

/* Starts the paired hook animation after the two character models have been positioned. */
void fn_1_664(void)
{
    OMOBJ *object = lbl_1_bss_10;
    OMOBJ *object0 = lbl_1_bss_C;
    OMOBJ *object1 = lbl_1_bss_10;
    EndingMotionWork *work0 = &lbl_1_bss_1A5C[0];
    EndingMotionWork *work1 = &lbl_1_bss_1A5C[1];

    Hu3DModelObjPosGet(object0->mdlId[0], "gN00m1-itemhook_R",
        &work0->startOrVelocity);
    Hu3DModelObjPosGet(object1->mdlId[0], "gN00m1-itemhook_R",
        &work1->startOrVelocity);
    object->work[0] = 0;
    object->work[1] = 180;
    object->work[2] = 0;
    object->objFunc = fn_1_558;
}

/* Stops updating the two item hooks when the character effect ends. */
void fn_1_71C(void)
{
    fn_1_10DEC(0, NULL, 2);
    fn_1_10DEC(1, NULL, 2);
    lbl_1_bss_10->objFunc = NULL;
}

/* Activates the ending scene camera when its scene setup requests it. */
void fn_1_76C(void)
{
    Hu3DCameraCreate(1);
}

/* Disables the ending scene camera during scene teardown. */
void fn_1_790(void)
{
    Hu3DCameraKill(1);
}

/* Creates the two fixed global lights used by the ending models. */
void fn_1_7B4(void)
{
    EndingLightVectors positions = {
        { { 0.0f, 1.0f, 1.0f }, { -1.0f, 1.0f, -1.0f } }
    };
    EndingLightVectors directions = {
        { { 0.0f, -1.0f, -1.0f }, { 1.0f, -1.0f, -1.0f } }
    };
    GXColor color = { 255, 255, 255, 255 };
    s16 light;

    for (light = 0; light < 2; light++) {
        lbl_1_bss_1A56[light] = Hu3DGLightCreateV(
            &positions.vector[light], &directions.vector[light], &color);
        Hu3DGLightInfinitytSet(lbl_1_bss_1A56[light]);
        Hu3DGLightStaticSet(lbl_1_bss_1A56[light], TRUE);
    }
}

/* Clears the scene's global lights when the ending lighting changes. */
void fn_1_904(void)
{
    Hu3DLightAllKill();
}

/* Adds a fixed local light to each character model for the paired scene. */
void fn_1_924(void)
{
    lbl_1_bss_1A52[0] = Hu3DLLightCreate(lbl_1_bss_C->mdlId[0],
        (-300.0f), (200.0f), (100.0f),
        0.0f, 0.0f, (-1.0f),
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightStaticSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0], 1);

    lbl_1_bss_1A52[1] = Hu3DLLightCreate(lbl_1_bss_10->mdlId[0],
        (300.0f), (200.0f), (100.0f),
        0.0f, 0.0f, (-1.0f),
        128, 128, 128);
    Hu3DLLightInfinitytSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    Hu3DLLightStaticSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1], 1);
}

/* Removes the local lights attached to the two character models. */
void fn_1_AC8(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
}

/* Sets the shadow projection used by the ending models during scene setup. */
void fn_1_B2C(void)
{
    HuVecF position = { 0.0f, 500.0f, 1700.0f };
    HuVecF up = { 0.0f, 1.0f, 0.0f };
    HuVecF target = { 0.0f, 0.0f, 0.0f };

    Hu3DShadowCreate((30.0f), (10.0f), (10000.0f));
    Hu3DShadowPosSet(&position, &up, &target);
}

void fn_1_B2C(void);

s16 lbl_1_data_38 = -1;

/* Closes the currently open ending message window when one is active. */
void fn_1_BE0(void)
{
    if (lbl_1_data_38 != -1) {
        HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
        lbl_1_data_38 = -1;
    }
}

void fn_1_F34(void);

/* Opens a selected ending window and starts its message when a scene requests dialogue. */
void fn_1_C44(s16 window, u32 message)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 2048);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
}

/* Shows an ending message and optionally waits before its caller continues. */
void fn_1_DAC(s16 window, u32 message, s16 delay)
{
    if (lbl_1_data_38 != -1 && lbl_1_data_38 != window) {
        fn_1_BE0();
    }
    if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
        HuWinExOpen(lbl_1_bss_1A48[window]);
        lbl_1_data_38 = window;
    }
    HuWinAttrSet(lbl_1_bss_1A48[window], 2048);
    HuWinMesSet(lbl_1_bss_1A48[window], message);
    if (lbl_1_data_0 != message) {
        lbl_1_data_0 = -1;
    }
    if (delay > 0) {
        HuPrcSleep(delay);
    }
}

/* Creates the four character dialogue windows and the small caption window at scene start. */
void fn_1_F34(void)
{
    EndingWindowPlayers players = { { 0, 4, 3, 5 } };
    s16 window;

    HuWinInit(1);
    for (window = 0; window < 4; window++) {
        lbl_1_bss_1A48[window] = HuWinExCreateFrame(
            (16.0f), (372.0f), 544, 68, -1,
            players.player[window]);
        HuWinDispOff(lbl_1_bss_1A48[window]);
        HuWinBGTPLvlSet(lbl_1_bss_1A48[window], (0.8999999761581421f));
        winData[lbl_1_bss_1A48[window]].padMask = 1;
    }
    for (window = 0; window < 4; window++) {
        HuWinCallbackSet(lbl_1_bss_1A48[window], fn_1_98);
    }
    lbl_1_bss_1A48[4] = HuWinExCreateFrame((16.0f),
        (337.0f), 544, 42, -1, 0);
    HuWinDispOff(lbl_1_bss_1A48[4]);
    HuWinBGTPLvlSet(lbl_1_bss_1A48[4], 0.0f);
}

/* Destroys the ending dialogue windows after the scene finishes. */
void fn_1_1104(void)
{
    s16 window;

    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
}

/* Sprite object callback: slides the selected character marker during its transition. */
void fn_1_1160(OMOBJ *object)
{
    switch (object->work[0]) {
        case 0:
            lbl_1_bss_28 += (0.10000000149011612f);
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                0.0f);
            break;
        case 1:
            lbl_1_bss_28 -= (0.10000000149011612f);
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], lbl_1_bss_28,
                0.0f);
            break;
        case 2:
            lbl_1_bss_28 -= (0.10000000149011612f);
            HuSprGrpPosSet(lbl_1_bss_1A1C[0], 0.0f,
                lbl_1_bss_28);
            break;
    }
}

/* Selects which character markers are visible and starts their slide callback. */
void fn_1_12A8(s16 state)
{
    if (state == 2) {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], 0.0f,
            0.0f);
    } else {
        HuSprGrpPosSet(lbl_1_bss_1A1C[0], 0.0f,
            0.0f);
    }
    fn_1_E0EC(lbl_1_bss_1A1C[0], HUSPR_ATTR_DISPOFF);
    lbl_1_bss_4->work[0] = state;
    lbl_1_bss_4->objFunc = fn_1_1160;

    switch (state) {
        case 0:
            lbl_1_bss_28 = 0.0f;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 4,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 6,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 3,
                HUSPR_ATTR_DISPOFF);
            break;
        case 1:
            lbl_1_bss_28 = 0.0f;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 1,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 5,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 8,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 9,
                HUSPR_ATTR_DISPOFF);
            break;
        case 2:
            lbl_1_bss_28 = 0.0f;
            HuSprAttrReset(lbl_1_bss_1A1C[0], 0,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 2,
                HUSPR_ATTR_DISPOFF);
            HuSprAttrReset(lbl_1_bss_1A1C[0], 7,
                HUSPR_ATTR_DISPOFF);
            break;
        case 3:
            lbl_1_bss_4->objFunc = NULL;
            break;
    }
}

/* Loads and lays out the ten character markers used by the ending scene. */
void fn_1_14F4(void)
{
    EndingSpritePositions positions = {
        {
            { 288, 263, 0 },
            { 225, 365, 1 },
            { 120, 303, 1 },
            { 424, 303, 0 },
            { 108, 359, 2 },
            { 91, 279, 0 },
            { 338, 251, 1 },
            { 476, 343, 1 },
            { 301, 371, 2 },
            { 471, 403, 3 },
        }
    };
    s16 index;

    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A20[index] = HuSprAnimRead(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 20) + index, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    lbl_1_bss_1A1C[0] = HuSprGrpCreate(10);
    for (index = 0; index < 10; index++) {
        lbl_1_bss_1A08[index] = HuSprCreate(lbl_1_bss_1A20[index],
            positions.position[index].z, 0);
        HuSprGrpMemberSet(lbl_1_bss_1A1C[0], index,
            lbl_1_bss_1A08[index]);
        HuSprPosSet(lbl_1_bss_1A1C[0], index,
            positions.position[index].x, positions.position[index].y);
    }
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], 0.0f,
        0.0f);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    HuSprExecLayerSet(64, 2);
    HuSprGrpDrawNoSet(lbl_1_bss_1A1C[0], 64);
}

inline void fn_1_14F4(void);

/* Creates the centered ending logo sprite used during the fade-in. */
void fn_1_16D4(void)
{
    s16 sprite;
    void *fileData;
    ANIMDATA *animation;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 19), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_1A1C[1] = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 0, 0);
    HuSprGrpMemberSet(lbl_1_bss_1A1C[1], 0, sprite);
    HuSprGrpPosSet(lbl_1_bss_1A1C[1], (288.0f),
        (240.0f));
    fn_1_E0EC(lbl_1_bss_1A1C[1], 4);
}

/* Fades in the ending logo over 60 scheduler passes when the credits sequence calls it. */
void fn_1_1798(void)
{
    s16 frame;
    float transparency;

    HuSprGrpTPLvlSet(lbl_1_bss_1A1C[1], 0.0f);
    fn_1_E16C(lbl_1_bss_1A1C[1], 4);
    for (frame = 0; frame < 60; frame++) {
        HuPrcVSleep();
        transparency = fn_1_DDF8(0.0f, 1.0f, frame, 60.0f);
        HuSprGrpTPLvlSet(lbl_1_bss_1A1C[1], transparency);
    }
    HuSprGrpTPLvlSet(lbl_1_bss_1A1C[1], 1.0f);
}

/* Object initializer: loads the opening scene model and starts its embedded motion. */
void fn_1_189C(OMOBJ *object)
{
    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 0), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[0] = Hu3DMotionIDGet(object->mdlId[0]);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    object->objFunc = NULL;
}

/* Character object callback: turns each character toward the scene and resumes its idle motion. */
void fn_1_193C(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;
    HuVecF rotation;

    for (model = 0; model < 10; model++, work++) {
        if (0.0f == work->time) {
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                lbl_1_bss_8->mtnId[3 + (model * 6)],
                0.0f, (10.0f), HU3D_MOTATTR_LOOP);
        }
        Hu3DModelRotGet(object->mdlId[model], &rotation);
        rotation.y = fn_1_DDF8(work->start, work->end, work->time,
            work->duration);
        Hu3DModelRotSetV(object->mdlId[model], &rotation);
        if ((work->time += (1.0f)) > work->duration) {
            work->time = (1.0f) + work->duration;
    if (0.0f == work->finalMotionStarted) {
        work->finalMotionStarted = (1.0f);
                Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[model],
                    lbl_1_bss_8->mtnId[model * 6], 0.0f,
                    (5.0f), HU3D_MOTATTR_LOOP);
            }
        }
    }
}

/* Copies each character's named attachment position from the main model to its display model. */
void fn_1_1B20(void)
{
    Mtx matrix;
    EndingModelObjectNames names = {
        {
            "test005-mario",
            "test005-luigi",
            "test005-peach",
            "test005-yoshi",
            "test005-wario",
            "test005-daisy",
            "test005-waluigi",
            "test005-kinopio",
            "test005-teresa",
            "test005-koopa",
        }
    };
    s16 model;

    for (model = 0; model < 10; model++) {
        Hu3DModelObjMtxGet(lbl_1_bss_4->mdlId[0], names.name[model], matrix);
        Hu3DModelPosSet(lbl_1_bss_8->mdlId[model], matrix[0][3],
            matrix[1][3], matrix[2][3]);
    }
}

/* Prepares a character's yaw interpolation; called before the group turn animation starts. */
/* Prepares one character's yaw turn before the group-turn sequence starts. */
void fn_1_1C0C(s16 characterIndex, s16 motionModel, float turnDuration)
{
    HuVecF motionPosition;
    HuVecF characterPosition;
    EndingMotionWork *turnWork = &lbl_1_bss_1ADC[characterIndex];

    Hu3DModelPosGet(motionModel, &motionPosition);
    Hu3DModelPosGet(lbl_1_bss_8->mdlId[characterIndex], &characterPosition);
    characterPosition.x -= motionPosition.x;
    characterPosition.z -= motionPosition.z;
    characterPosition.y = -(atan2(characterPosition.x, -characterPosition.z) /
        (3.141592653589793) * (180.0));
    Hu3DModelRotGet(lbl_1_bss_8->mdlId[characterIndex], &motionPosition);
    for (;;) {
        if (motionPosition.y < 0.0f) {
            motionPosition.y += (360.0f);
        } else {
            break;
        }
    }
    for (;;) {
        if (motionPosition.y >= (360.0f)) {
            motionPosition.y -= (360.0f);
        } else {
            break;
        }
    }
    for (;;) {
        if (characterPosition.y < 0.0f) {
            characterPosition.y += (360.0f);
        } else {
            break;
        }
    }
    for (;;) {
        if (characterPosition.y >= (360.0f)) {
            characterPosition.y -= (360.0f);
        } else {
            break;
        }
    }
    turnWork->time = -(float)frandmod(20);
    turnWork->duration = turnDuration;
    turnWork->start = motionPosition.y;
    turnWork->end = characterPosition.y;
    turnWork->finalMotionStarted = 0.0f;
}

/* Sets up the same yaw interpolation for all ten characters before a group turn. */
void fn_1_1E3C(s16 motionModel, float turnDuration)
{
    s16 characterIndex;

    for (characterIndex = 0; characterIndex < 10; characterIndex++) {
        fn_1_1C0C(characterIndex, motionModel, turnDuration);
    }
}

/* Starts one character's selected motion when its stagger delay expires. */
void fn_1_1E90(s16 characterIndex, s16 motionIndex, float blend, u32 attr)
{
    Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[characterIndex],
        lbl_1_bss_8->mtnId[motionIndex + (characterIndex * 6)], 0.0f,
        blend, attr);
}

/* Starts one motion on every character when a scene transition calls for a group change. */
void fn_1_1F20(s16 motionIndex, float blend, u32 attr)
{
    s16 characterIndex;

    for (characterIndex = 0; characterIndex < 10; characterIndex++) {
        Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[characterIndex],
            lbl_1_bss_8->mtnId[motionIndex + (characterIndex * 6)], 0.0f,
            blend, attr);
    }
}

/* Slows all ten character models after their entrance motion. */
void fn_1_1FC4(void)
{
    s16 characterIndex;

    HuPrcSleep(2);
    for (characterIndex = 0; characterIndex < 10; characterIndex++) {
        Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[characterIndex], (0.25f));
    }
}

/* Starts a group motion after independently staggering each character's delay. */
void fn_1_2034(s16 motionIndex, float blend, u32 attr, s16 delay)
{
    s16 frame;
    s16 characterIndex;

    for (characterIndex = 0; characterIndex < 10; characterIndex++) {
        lbl_1_bss_19F4[characterIndex] = (rand8() % delay) + 1;
    }
    if (motionIndex == 4) {
        fn_1_0(596);
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (characterIndex = 0; characterIndex < 10; characterIndex++) {
            if (lbl_1_bss_19F4[characterIndex] == 0) {
                fn_1_1E90(characterIndex, motionIndex, blend, attr);
            }
            lbl_1_bss_19F4[characterIndex]--;
            if (lbl_1_bss_19F4[characterIndex] <= -10) {
                lbl_1_bss_19F4[characterIndex] = -10;
            }
        }
    }
}

/* Staggers the staff dance motion, then slows each character as it starts. */
void fn_1_2208(s16 motionIndex, float blend, u32 attr,
    s16 unused, s16 delay)
{
    s16 frame;
    s16 characterIndex;

    for (characterIndex = 0; characterIndex < 10; characterIndex++) {
        lbl_1_bss_19F4[characterIndex] = (rand8() % delay) + 1;
    }
    for (frame = 0; frame < delay + 5; frame++) {
        HuPrcVSleep();
        for (characterIndex = 0; characterIndex < 10; characterIndex++) {
            if (lbl_1_bss_19F4[characterIndex] == 0) {
                fn_1_1E90(characterIndex, motionIndex, blend, attr);
                Hu3DMotionSpeedSet(lbl_1_bss_8->mdlId[characterIndex],
                    (0.25f));
            }
            lbl_1_bss_19F4[characterIndex]--;
            if (lbl_1_bss_19F4[characterIndex] <= -10) {
                lbl_1_bss_19F4[characterIndex] = -10;
            }
        }
    }
}

/* Object initializer: loads the ten character models and their six motions each for the group
 * dance. */
void fn_1_23D8(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1ADC;
    s16 model;

    omSetStatBit(object, 256);
    for (model = 0; model < 10; model++, work++) {
        object->mdlId[model] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 53) + model, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[model * 6] = Hu3DJointMotion(object->mdlId[model],
        HuDataSelHeapReadNum(DATANUM(DATA_ending, 74) + model, HU_MEMNUM_OVL,
                HEAP_MODEL));
        object->mtnId[(model * 6) + 1] = Hu3DJointMotion(
                object->mdlId[model], HuDataSelHeapReadNum(DATANUM(DATA_ending, 85) + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 2] = Hu3DJointMotion(
                object->mdlId[model], HuDataSelHeapReadNum(DATANUM(DATA_ending, 63) + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 3] = Hu3DJointMotion(
                object->mdlId[model], HuDataSelHeapReadNum(DATANUM(DATA_ending, 96) + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 4] = Hu3DJointMotion(
                object->mdlId[model], HuDataSelHeapReadNum(DATANUM(DATA_ending, 107) + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[(model * 6) + 5] = Hu3DJointMotion(
                object->mdlId[model], HuDataSelHeapReadNum(DATANUM(DATA_ending, 118) + model,
                HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelScaleSet(object->mdlId[model], (0.8999999761581421f),
            (0.8999999761581421f), (0.8999999761581421f));
        Hu3DMotionShiftSet(object->mdlId[model], object->mtnId[model * 6],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        work->time = (11.0f);
        work->duration = (10.0f);
                work->finalMotionStarted = (1.0f);
    }
    object->objFunc = fn_1_193C;
}

/* Releases the ten character models and motions when the group dance ends. */
void fn_1_26D4(OMOBJ *characterObject)
{
    s16 model;
    s16 motion;

    if (characterObject) {
        for (model = 0; model < 10; model++) {
            for (motion = 0; motion < 6; motion++) {
                Hu3DMotionKill(characterObject->mtnId[motion + (model * 6)]);
            }
            Hu3DModelKill(characterObject->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, characterObject);
    }
    characterObject = NULL;
}

/* Sets a vector to a supplied world-space position during entrance setup. */
void fn_1_2790(HuVecF *position, float x, float y, float z)
{
    position->x = x;
    position->y = y;
    position->z = z;
}

/* Evaluates one axis of the curved character entrance path at normalized progress. */
float fn_1_27A0(float start, float middle, float end, float time)
{
    float inverse = (1.0f) - time;

    return end * (time * time)
        + (start * (inverse * inverse)
        + (2.0f) * (middle * (inverse * time)));
}

/* Builds a point on the characters' quadratic entrance path for fn_1_2AEC. */
void fn_1_27FC(HuVecF *dest, HuVecF *start, HuVecF *middle,
    HuVecF *end, float time)
{
    dest->x = fn_1_27A0(start->x, middle->x, end->x, time);
    dest->y = fn_1_27A0(start->y, middle->y, end->y, time);
    dest->z = fn_1_27A0(start->z, middle->z, end->z, time);
}

/* Eases scene motion from its starting value to its endpoint over the given frames. */
float fn_1_2A04(float startValue, float endValue, float time, float duration)
{
    if (time <= 0.0f) {
        return startValue;
    }
    if (time >= duration) {
        return endValue;
    }
    return startValue + ((endValue - startValue) * sind((90.0f / duration) * time));
}

/* Character object callback, installed by fn_1_31FC: follows two curved entrance paths each
 * update. */
void fn_1_2AEC(OMOBJ *object)
{
    EndingMotionWork *firstWork = &lbl_1_bss_1A5C[0];
    EndingMotionWork *secondWork = &lbl_1_bss_1A5C[1];
    HuVecF position;

    if (firstWork->state == 1) {
        fn_1_27FC(&position, &firstWork->startOrVelocity, &firstWork->controlOrPosition,
                  &firstWork->endOrEffectParams,
                  fn_1_2A04(0.0f, 1.0f, firstWork->time, firstWork->duration));
        Hu3DModelPosSetV(lbl_1_bss_C->mdlId[0], &position);
        fn_1_12988(4, &position, 1, 5);
        if (++firstWork->time > firstWork->duration) {
            HuAudFXStop(lbl_1_bss_1D5C.channel[2]);
            fn_1_12988(4, &position, 0, 6);
            firstWork->state = 0;
            Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
                0.0f, (30.0f), HU3D_MOTATTR_LOOP);
        }
    }

    if (secondWork->state == 1) {
        fn_1_27FC(&position, &secondWork->startOrVelocity, &secondWork->controlOrPosition,
                  &secondWork->endOrEffectParams,
                  fn_1_2A04(0.0f, 1.0f, secondWork->time, secondWork->duration));
        Hu3DModelPosSetV(lbl_1_bss_10->mdlId[0], &position);
        fn_1_12988(5, &position, 1, 6);
        if (++secondWork->time > secondWork->duration) {
            HuAudFXStop(lbl_1_bss_1D5C.channel[3]);
            fn_1_12988(5, &position, 0, 6);
            secondWork->state = 0;
            Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[0],
                0.0f, (30.0f), HU3D_MOTATTR_LOOP);
        }
    }
}

/* Starts both characters on their entrance paths and installs the per-frame path callback. */
void fn_1_31FC(void)
{
    OMOBJ *object = lbl_1_bss_C;
    EndingMotionWork *first = &lbl_1_bss_1A5C[0];
    EndingMotionWork *second = &lbl_1_bss_1A5C[1];

    first->state = 1;
    first->time = 0.0f;
    first->duration = 120.0f;
    fn_1_2790(&first->startOrVelocity, -100.0f, 200.0f, -1000.0f);
    fn_1_2790(&first->controlOrPosition, -500.0f, 300.0f, -500.0f);
    fn_1_2790(&first->endOrEffectParams, -300.0f, 200.0f, 0.0f);

    second->state = 1;
    second->time = 0.0f;
    second->duration = 120.0f;
    fn_1_2790(&second->startOrVelocity, 100.0f, 200.0f, -1000.0f);
    fn_1_2790(&second->controlOrPosition, 500.0f, 300.0f, -500.0f);
    fn_1_2790(&second->endOrEffectParams, 300.0f, 200.0f, 0.0f);

    Hu3DModelPosSet(lbl_1_bss_C->mdlId[0], -100.0f,
        200.0f, -1000.0f);
    Hu3DModelPosSet(lbl_1_bss_10->mdlId[0], 100.0f,
        200.0f, -1000.0f);
    Hu3DModelRotSet(lbl_1_bss_C->mdlId[0], 0.0f,
        15.0f, 0.0f);
    Hu3DModelRotSet(lbl_1_bss_10->mdlId[0], 0.0f,
        -15.0f, 0.0f);
    Hu3DModelAttrReset(lbl_1_bss_C->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DModelAttrReset(lbl_1_bss_10->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[1],
        0.0f, 0.0f, 0);
    Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[1],
        0.0f, 0.0f, 0);
    lbl_1_bss_1D5C.channel[2] = fn_1_44(1150, 90, 64);
    lbl_1_bss_1D5C.channel[3] = fn_1_44(1148, 90, 64);
    object->objFunc = fn_1_2AEC;
}

/* Selects a pose for either featured character; fn_1_702C calls this between ending dialogue
 * beats. */
void fn_1_35F4(s16 index, s16 motion, float blend, u32 attr)
{
    if (index == 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0],
            lbl_1_bss_C->mtnId[motion], 0.0f, blend, attr);
    } else {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0],
            lbl_1_bss_10->mtnId[motion], 0.0f, blend, attr);
    }
}

/* Object callback installed by fn_1_39AC or fn_1_3A7C; interpolates a character turn or slide each
 * update. */
void fn_1_36E0(OMOBJ *object)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[object->work[0]];
    float value;
    float rotationValue;

    if (object->work[1] == 1) {
        float start;
        float time;
        float end;
        float duration;

        duration = work->duration;
        time = work->time;
        end = work->end;
        start = work->start;

        if (time <= 0.0f) {
            rotationValue = start;
        } else if (time >= duration) {
            rotationValue = end;
        } else {
            rotationValue = start + ((end - start) *
                sin((M_PI * (90.0f / duration * time)) / 180.0));
        }
        value = rotationValue;
        Hu3DModelRotSet(object->mdlId[0], 0.0f, value,
            0.0f);
        if ((work->time += (1.0f)) > work->duration) {
            object->objFunc = NULL;
        }
    } else {
        HuVecF position;
        float positionValue;
        float start;
        float time;
        float end;
        float duration;

        duration = work->duration;
        time = work->time;
        end = work->end;
        start = work->start;

        if (time <= 0.0f) {
            positionValue = start;
        } else if (time >= duration) {
            positionValue = end;
        } else {
            positionValue = start + ((end - start) *
                sin((M_PI * (90.0f / duration * time)) / 180.0));
        }
        value = positionValue;
        Hu3DModelPosGet(object->mdlId[0], &position);
        Hu3DModelPosSet(object->mdlId[0], value, position.y, position.z);
        if ((work->time += (1.0f)) > work->duration) {
            object->objFunc = NULL;
        }
    }
}

/* Starts a character yaw turn and assigns fn_1_36E0 to animate it on later updates. */
void fn_1_39AC(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF rotation;

    work->time = 0.0f;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelRotGet(object->mdlId[0], &rotation);
    work->start = rotation.y;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 1;
    object->objFunc = fn_1_36E0;
}

/* Starts a character horizontal slide and assigns fn_1_36E0 to animate it on later updates. */
void fn_1_3A7C(s16 index, float end, float duration)
{
    EndingMotionWork *work = &lbl_1_bss_1A5C[index];
    OMOBJ *object;
    HuVecF position;

    work->time = 0.0f;
    work->duration = duration;
    if (index == 0) {
        object = lbl_1_bss_C;
    } else {
        object = lbl_1_bss_10;
    }
    Hu3DModelPosGet(object->mdlId[0], &position);
    work->start = position.x;
    work->end = end;
    object->work[0] = index;
    object->work[1] = 0;
    object->objFunc = fn_1_36E0;
}

/* Object initializer: loads the first speaking character model, its motions, and its scene
 * transform. */
void fn_1_3B4C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 30), HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_ending, 31) + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(object->mdlId[0], (-200.0f),
        (200.0f), 0.0f);
    Hu3DModelRotSet(object->mdlId[0], 0.0f,
        (15.0f), 0.0f);
    Hu3DModelScaleSet(object->mdlId[0], (1.25f),
        (1.25f), (1.25f));
    object->objFunc = NULL;
}

/* Releases the first speaking character's motions, model, and object slot during scene cleanup. */
void fn_1_3CD4(OMOBJ *object)
{
    s16 motion;

    if (object) {
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

/* Object initializer: loads the second speaking character and hooks its held item to the model. */
void fn_1_3D5C(OMOBJ *object)
{
    s16 motion;

    omSetStatBit(object, 256);
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 37), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 38), HU_MEMNUM_OVL, HEAP_MODEL));
    for (motion = 0; motion < 6; motion++) {
        object->mtnId[motion] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_ending, 39) + motion, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelHookSet(object->mdlId[0], "gN01m1-itemhook_R",
        object->mdlId[1]);
    Hu3DModelAttrSet(object->mdlId[0], HU3D_ATTR_DISPOFF);
    Hu3DMotionShiftSet(object->mdlId[0], object->mtnId[0],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DModelPosSet(object->mdlId[0], (200.0f),
        (200.0f), 0.0f);
    Hu3DModelRotSet(object->mdlId[0], 0.0f,
        (-15.0f), 0.0f);
    Hu3DModelScaleSet(object->mdlId[0], (1.25f),
        (1.25f), (1.25f));
    object->objFunc = NULL;
}

/* Detaches and releases the second character's models and motions during scene cleanup. */
void fn_1_3F20(OMOBJ *object)
{
    s16 motion;

    if (object) {
        Hu3DModelHookReset(object->mdlId[0]);
        for (motion = 0; motion < 6; motion++) {
            Hu3DMotionKill(object->mtnId[motion]);
        }
        Hu3DModelKill(object->mdlId[0]);
        Hu3DModelKill(object->mdlId[1]);
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

/* Starts the effect model's shape motion in the forward direction when the scene enables it. */
void fn_1_3FC0(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], 0.0f);
    model->motShapeWork.speed = (2.0f);
    object->work[3] = 1;
}

/* Rewinds the effect model's shape motion when the scene hides that effect. */
void fn_1_4058(void)
{
    OMOBJ *object = lbl_1_bss_18;
    HU3D_MODEL *model = &Hu3DData[object->mdlId[0]];

    Hu3DMotionShapeSet(object->mdlId[0], object->mtnId[0]);
    Hu3DMotionShapeTimeSet(object->mdlId[0], 0.0f);
    model->motShapeWork.speed = (-2.0f);
    object->work[3] = 0;
}

/* Effect object callback, installed by fn_1_45C0: cycles translucent models during the scene. */
void fn_1_40F0(OMOBJ *object)
{
    EndingMotionWork *work = lbl_1_bss_1834;
    s16 model;
    float alpha;

    for (model = 0; model < 7; model++, work++) {
        if (work->time < 0.0f) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        } else if (work->time >= 0.0f) {
            if (0.0f == work->time) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    0.0f);
                Hu3DMotionSet(object->mdlId[model + 2],
                    object->mtnId[model + 2]);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    0.0f);
                Hu3DMotionSet(object->mdlId[model + 9],
                    object->mtnId[model + 2]);
            } else if (work->time < (50.0f)) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8(0.0f, (0.800000011920929f),
                    work->time, (50.0f));
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            } else if (work->time < (90.0f)) {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelTPLvlSet(object->mdlId[model + 2],
                    (0.800000011920929f));
                Hu3DModelTPLvlSet(object->mdlId[model + 9],
                    (0.800000011920929f));
            } else {
                Hu3DModelAttrReset(object->mdlId[model + 2],
                    HU3D_ATTR_DISPOFF);
                Hu3DModelAttrReset(object->mdlId[model + 9],
                    HU3D_ATTR_DISPOFF);
                alpha = fn_1_DDF8((0.800000011920929f), 0.0f,
                    work->time - (90.0f), (30.0f));
                Hu3DModelTPLvlSet(object->mdlId[model + 2], alpha);
                Hu3DModelTPLvlSet(object->mdlId[model + 9], alpha);
            }
        }
        if ((work->time += (1.0f)) > (120.0f)) {
            work->time = (-90.0f);
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    }
    if (object->work[3] == 0) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 9], HU3D_ATTR_DISPOFF);
        }
    } else if (object->work[3] == 1) {
        for (model = 0; model < 7; model++) {
            Hu3DModelAttrSet(object->mdlId[model + 2], HU3D_ATTR_DISPOFF);
        }
    }
}

/* Object initializer: creates the repeated display models and configures the preloaded particle
 * effects for the ending scene. */
void fn_1_45C0(OMOBJ *object)
{
    Vec position = { 0.0f, 280.0f, 150.0f };
    s16 model;
    OMOBJ *effectObject;
    HU3D_MODEL *modelData;

    omSetStatBit(object, OM_STAT_MODELPAUSE);
    for (model = 0; model < 9; model++) {
        object->mdlId[model] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 9) + model, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[model] = Hu3DMotionIDGet(object->mdlId[model]);
        Hu3DModelLayerSet(object->mdlId[model], 1);
        Hu3DMotionShiftSet(object->mdlId[model], object->mtnId[model],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelPosSet(object->mdlId[model], position.x, position.y,
            position.z);
        Hu3DModelRotSet(object->mdlId[model], (25.0f),
            0.0f, 0.0f);
        Hu3DModelScaleSet(object->mdlId[model], (0.800000011920929f),
            (0.800000011920929f), (0.800000011920929f));
    }
    Hu3DModelLayerSet(object->mdlId[0], 0);
    Hu3DModelAttrSet(object->mdlId[0], HU3D_MOTATTR_PAUSE);
    Hu3DModelPosSet(object->mdlId[1], position.x, position.y,
        position.z - (100.0f));

    for (model = 9; model < 16; model++) {
        object->mdlId[model] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 18), HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[model] = Hu3DMotionIDGet(object->mdlId[model]);
        Hu3DModelLayerSet(object->mdlId[model], 1);
        Hu3DMotionShiftSet(object->mdlId[model], object->mtnId[model],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        Hu3DModelPosSet(object->mdlId[model], position.x, position.y,
            position.z);
        Hu3DModelRotSet(object->mdlId[model], (25.0f),
            0.0f, 0.0f);
        Hu3DModelScaleSet(object->mdlId[model], (0.800000011920929f),
            (0.800000011920929f), (0.800000011920929f));
    }

    for (model = 0; model < 7; model++) {
        lbl_1_bss_1834[model].time = 120 - (model * 30);
    }
    effectObject = lbl_1_bss_18;
    modelData = &Hu3DData[effectObject->mdlId[0]];
    Hu3DMotionShapeSet(effectObject->mdlId[0], effectObject->mtnId[0]);
    Hu3DMotionShapeTimeSet(effectObject->mdlId[0], 0.0f);
    modelData->motShapeWork.speed = (-2.0f);
    effectObject->work[3] = 0;
    fn_1_E1EC(1, &position);
    fn_1_EAB8(1, &position);
    object->objFunc = fn_1_40F0;
}

/* Releases all repeated effect models and their object during ending scene cleanup. */
void fn_1_4A0C(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_E1EC(0, NULL);
        fn_1_EAB8(0, NULL);
        for (model = 0; model < 16; model++) {
            Hu3DMotionKill(object->mtnId[model]);
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

/* Destroys ending particle effects and dialogue windows, then clears lights and the camera. */
void fn_1_4AB4(void)
{
    s16 window;

    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
}

/* Object callback installed after the player confirms; fades audio and returns from the ending
 * overlay. */
void fn_1_4B20(OMOBJ *object)
{
    s16 window;

    if (WipeCheck() == 0) {
        HuAudFadeOut(1000);
        fn_1_12838();
        for (window = 0; window < 4; window++) {
            HuWinExKill(lbl_1_bss_1A48[window]);
        }
        HuWinAllKill();
        Hu3DLightAllKill();
        Hu3DCameraKill(1);
        HuAudAllStop();
        omOvlReturnEx(1, 1);
        object->objFunc = NULL;
    }
}

/* Confirmation object callback: shows the prompt after wipes and waits for Start before beginning
 * the return wipe. */
void fn_1_4BC8(OMOBJ *object)
{
    HuWinDispOff(lbl_1_bss_1A48[4]);
    if (WipeCheck() != 0) {
        return;
    }
    if (lbl_1_bss_24 == 1) {
        object->work[0]++;
        if (object->work[0] < 5) {
            return;
        }
    } else {
        object->work[0] = 0;
        return;
    }
    if (lbl_1_bss_2C == 0) {
        HuWinPosSet(lbl_1_bss_1A48[4], (16.0f),
            (337.0f));
    } else {
        HuWinPosSet(lbl_1_bss_1A48[4], (16.0f),
            (400.0f));
    }
    HuWinDispOn(lbl_1_bss_1A48[4]);
    HuWinAttrSet(lbl_1_bss_1A48[4], HUWIN_ATTR_ALIGN_CENTER);
    HuWinMesSet(lbl_1_bss_1A48[4], MESSNUM(MESS_SYS_GUIDE, 5));
    HuWinMesSpeedSet(lbl_1_bss_1A48[4], 0);
    HuWinPriSet(lbl_1_bss_1A48[4], 0);
    switch (lbl_1_bss_2E) {
    case 0:
        if (lbl_1_bss_24 == 1 && (HuPadBtnDown[0] & PAD_BUTTON_START)) {
            lbl_1_bss_2E = 1;
            lbl_1_bss_26 = 1;
            HuAudSStreamFadeOut(lbl_1_bss_1DDC, 1000);
            WipeCreate(2, 0, 60);
            object->objFunc = fn_1_4B20;
        }
        break;
    }
}

/* Child process: runs the ending sequence, fades out, cleans up the scene, then returns from the
 * overlay. */
void fn_1_4DAC(void)
{
    s16 window;

    fn_1_DD14();
    HuAudSStreamFadeOut(lbl_1_bss_1DDC, 1000);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_12838();
    for (window = 0; window < 4; window++) {
        HuWinExKill(lbl_1_bss_1A48[window]);
    }
    HuWinAllKill();
    Hu3DLightAllKill();
    Hu3DCameraKill(1);
    omOvlReturnEx(1, 1);
    HuPrcEnd();
    while (TRUE) {
        HuPrcVSleep();
    }
}

/* Sets up the ending object manager and launches its scene process when this overlay starts. */
void fn_1_4E5C(void)
{
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    Hu3DCameraCreate(1);
    fn_1_B2C();
    fn_1_F34();
    fn_1_11F34();

    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_189C);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 4096, 16, 96, -1,
        fn_1_23D8);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3B4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3D5C);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 4096, 32, 32, -1,
        fn_1_45C0);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 4096, 96, 16, -1, NULL);
    if (GwCommon.unkFlag4 != 0) {
        lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 4096, 0, 0, -1,
            fn_1_4BC8);
    }
    GwCommon.unkFlag4 = 1;
    HuPrcChildCreate(fn_1_4DAC, 12288, 12288, 0, lbl_1_bss_0);
}

inline void fn_1_4E5C(void);

/* Alternate ending setup entry point that creates the same scene objects as fn_1_4E5C. */
void fn_1_52D8(void)
{
    OSReport("\n-----===== MARIO PARTY 6 :: ENDING =====-----\n\n");
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    Hu3DCameraCreate(1);
    fn_1_B2C();
    fn_1_F34();
    fn_1_11F34();

    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_189C);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 4096, 16, 96, -1,
        fn_1_23D8);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3B4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3D5C);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 4096, 32, 32, -1,
        fn_1_45C0);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 4096, 96, 16, -1, NULL);
    if (GwCommon.unkFlag4 != 0) {
        lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 4096, 0, 0, -1,
            fn_1_4BC8);
    }
    GwCommon.unkFlag4 = 1;
    HuPrcChildCreate(fn_1_4DAC, 12288, 12288, 0, lbl_1_bss_0);
}

inline void fn_1_52D8(void);

/* Runs module constructors and creates the ending object manager and scene processes when the
 * overlay loads. */
int _prolog(void)
{
    const VoidFunc *ctor;

    for (ctor = _ctors; *ctor != 0; ctor++) {
        (*ctor)();
    }
    OSReport("\n-----===== MARIO PARTY 6 :: ENDING =====-----\n\n");
    lbl_1_bss_0 = omInitObjMan(11, 8192);
    Hu3DCameraCreate(1);
    fn_1_B2C();
    fn_1_F34();
    fn_1_11F34();

    lbl_1_bss_4 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_189C);
    lbl_1_bss_8 = omAddObjEx(lbl_1_bss_0, 4096, 16, 96, -1,
        fn_1_23D8);
    lbl_1_bss_14 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1, NULL);
    lbl_1_bss_C = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3B4C);
    lbl_1_bss_10 = omAddObjEx(lbl_1_bss_0, 4096, 16, 16, -1,
        fn_1_3D5C);
    lbl_1_bss_18 = omAddObjEx(lbl_1_bss_0, 4096, 32, 32, -1,
        fn_1_45C0);
    lbl_1_bss_1C = omAddObjEx(lbl_1_bss_0, 4096, 96, 16, -1, NULL);
    if (GwCommon.unkFlag4 != 0) {
        lbl_1_bss_20 = omAddObjEx(lbl_1_bss_0, 4096, 0, 0, -1,
            fn_1_4BC8);
    }
    GwCommon.unkFlag4 = 1;
    HuPrcChildCreate(fn_1_4DAC, 12288, 12288, 0, lbl_1_bss_0);
    return 0;
}

/* Runs the module destructors when the ending overlay unloads. */
void _epilog(void)
{
    const VoidFunc *dtor;

    for (dtor = _dtors; *dtor != 0; dtor++) {
        (*dtor)();
    }
}

void fn_1_6AB4(s16 motion0, s16 motion1, s16 window, s32 message,
    s16 count, s16 value, s16 motion, s16 delay);

/* Positions the opening display models and initializes each character's yaw target relative to the
 * scene model. */
static inline void endingOpeningModelsInit(void)
{
    fn_1_1B20();
    fn_1_1E3C(lbl_1_bss_18->mdlId[0], 1.0f);
}

/* Moves the ending camera into its close framing, then raises its target during the scene
 * transition. */
void fn_1_5C70(void)
{
    s16 frame;
    s16 duration;
    Vec position;
    Vec up;
    Vec target;
    Vec nextPosition;
    Vec nextTarget;

    Hu3DCameraPosGet(1, &position, &up, &target);
    duration = 60;
    for (frame = 0; frame < duration; frame++) {
        HuPrcVSleep();
        nextPosition.x = fn_1_2A04(position.x, 0.0f, frame, duration);
        nextPosition.y = fn_1_2A04(position.y, 280.0f, frame, duration);
        nextPosition.z = fn_1_2A04(position.z, 750.0f, frame, duration);
        nextTarget.x = fn_1_2A04(target.x, 0.0f, frame, duration);
        nextTarget.y = fn_1_2A04(target.y, 280.0f, frame, duration);
        nextTarget.z = fn_1_2A04(target.z, 0.0f, frame, duration);
        Hu3DCameraPosSetV(1, &nextPosition, &up, &nextTarget);
    }

    HuPrcSleep(60);
    Hu3DCameraPosGet(1, &position, &up, &target);
    duration = 600;
    for (frame = 0; frame < duration; frame++) {
        HuPrcVSleep();
        nextTarget.x = fn_1_2A04(target.x, 0.0f, frame, duration);
        nextTarget.y = fn_1_2A04(target.y, 840.0f, frame, duration);
        nextTarget.z = fn_1_2A04(target.z, 0.0f, frame, duration);
        Hu3DCameraPosSetV(1, &position, &up, &nextTarget);
    }
}

/* Applies speaker motions, window dialogue, and character reactions for each beat in fn_1_702C. */
void fn_1_6AB4(s16 motion0, s16 motion1, s16 window, s32 message,
    s16 count, s16 value, s16 motion, s16 delay)
{
    s16 index;
    s16 model;
    s16 currentModel;

    if (motion0 != 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[2],
            0.0f, (30.0f), HU3D_MOTATTR_LOOP);
    }
    if (motion1 != 0) {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[2],
            0.0f, (30.0f), HU3D_MOTATTR_LOOP);
    }
    if (window != -1 && message != -1) {
        if (lbl_1_data_38 != -1 && lbl_1_data_38 != window &&
            lbl_1_data_38 != -1) {
            HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
            lbl_1_data_38 = -1;
        }
        if (lbl_1_data_38 == -1 || lbl_1_data_38 != window) {
            HuWinExOpen(lbl_1_bss_1A48[window]);
            lbl_1_data_38 = window;
        }
        HuWinAttrSet(lbl_1_bss_1A48[window], HUWIN_ATTR_ALIGN_CENTER);
        HuWinMesSet(lbl_1_bss_1A48[window], message);
        if (lbl_1_data_0 != (u32)message) {
            lbl_1_data_0 = -1;
        }
    }
    if (count != -1) {
        for (index = 0; index < 10; index++) {
            fn_1_1C0C(index, count, (float)value);
        }
    } else if (motion != -1) {
        for (model = 0; model < 10; model++) {
            currentModel = model;
            Hu3DMotionShiftSet(lbl_1_bss_8->mdlId[currentModel],
                lbl_1_bss_8->mtnId[(motion + (currentModel * 6))], 0.0f,
                (float)value, HU3D_MOTATTR_LOOP);
        }
    }
    if (delay > 0) {
        HuPrcSleep(delay);
    }
    if (motion0 != 0) {
        Hu3DMotionShiftSet(lbl_1_bss_C->mdlId[0], lbl_1_bss_C->mtnId[0],
            0.0f, (30.0f), HU3D_MOTATTR_LOOP);
    }
    if (motion1 != 0) {
        Hu3DMotionShiftSet(lbl_1_bss_10->mdlId[0], lbl_1_bss_10->mtnId[0],
            0.0f, (30.0f), HU3D_MOTATTR_LOOP);
    }
}

/* Repositions the opening cast and slows the main scene model as the intro sequence advances. */
void fn_1_6EC4(void)
{
    s16 index;
    s16 motion;

    fn_1_1B20();
    motion = lbl_1_bss_18->mdlId[0];
    for (index = 0; index < 10; index++) {
        fn_1_1C0C(index, motion, (1.0f));
    }
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[0], (0.25f));
}

/* Runs the opening ending dialogue and character motions; the overlay entrypoint fn_1_DD14 calls
 * it. */
void fn_1_702C(void)
{
    endingOpeningModelsInit();
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[0], 0.25f);
    Hu3DModelCameraInfoSet(lbl_1_bss_4->mdlId[0], TRUE);
    Hu3DLightAllKill();
    Hu3DModelLightInfoSet(lbl_1_bss_4->mdlId[0], 1);
    fn_1_924();
    HuPrcSleep(30);
    lbl_1_bss_1DDC = HuAudSStreamPlay(MSM_STREAM_W04);
    lbl_1_bss_1D5C.channel[0] = fn_1_0(1413);
    lbl_1_bss_1D5C.channel[1] = fn_1_0(1423);
    WipeCreate(1, 0, 60);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    lbl_1_bss_2C = 0;
    lbl_1_bss_24 = 1;
    fn_1_6AB4(0, 0, 0, MESSNUM(MESS_ENDING, 0), -1, 30, -1, 180);
    fn_1_2034(1, 5.0f, 0U, 120);

    HuPrcSleep(90);

    fn_1_31FC();
    fn_1_6AB4(0, 0, 3, MESSNUM(MESS_ENDING, 1), -1, 30, -1, 0);
    HuPrcSleep(50);
    fn_1_0(594);
    fn_1_2034(2, 5.0f, 0U, 20);

    HuPrcSleep(110);

    fn_1_39AC(0, 60.0f, 60.0f);
    fn_1_6AB4(1, 0, 1, MESSNUM(MESS_ENDING, 2), lbl_1_bss_C->mdlId[0], 30, -1, 180);

    fn_1_39AC(1, -60.0f, 60.0f);
    fn_1_6AB4(0, 1, 2, MESSNUM(MESS_ENDING, 3), lbl_1_bss_10->mdlId[0], 30, -1, 180);

    fn_1_39AC(0, 15.0f, 60.0f);
    fn_1_39AC(1, -15.0f, 60.0f);
    fn_1_6AB4(0, 0, 3, MESSNUM(MESS_ENDING, 4), lbl_1_bss_18->mdlId[0], 30, -1, 180);
    fn_1_35F4(0, 5, 30.0f, 0U);
    fn_1_35F4(1, 5, 30.0f, 0U);
    fn_1_6AB4(0, 0, 3, MESSNUM(MESS_ENDING, 5), -1, 30, -1, 0);
    HuPrcSleep(50);
    fn_1_0(595);
    fn_1_2034(2, 5.0f, 0U, 20);

    HuPrcSleep(110);
    fn_1_35F4(1, 0, 30.0f, HU3D_MOTATTR_LOOP);
    fn_1_6AB4(1, 0, 1, MESSNUM(MESS_ENDING, 6), lbl_1_bss_C->mdlId[0], 30, -1, 180);
    fn_1_6AB4(0, 1, 2, MESSNUM(MESS_ENDING, 7), lbl_1_bss_10->mdlId[0], 30, -1, 180);
    if (lbl_1_data_38 != -1) {
        HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
        lbl_1_data_38 = -1;
    }
    lbl_1_bss_2C = 1;
    fn_1_6AB4(0, 0, -1, -1, lbl_1_bss_18->mdlId[0], 30, -1, 0);
    fn_1_3A7C(0, -200.0f, 30.0f);
    fn_1_3A7C(1, 200.0f, 30.0f);
    HuPrcSleep(1);
    fn_1_35F4(0, 3, 10.0f, HU3D_MOTATTR_LOOP);
    fn_1_35F4(1, 3, 10.0f, HU3D_MOTATTR_LOOP);
    Hu3DMotionShiftStartEndSet(lbl_1_bss_C->mdlId[0], 15.0f, 45.0f);
    Hu3DMotionShiftStartEndSet(lbl_1_bss_10->mdlId[0], 15.0f, 45.0f);
    HuPrcSleep(60);
    fn_1_2034(4, 5.0f, HU3D_MOTATTR_LOOP, 10);
    fn_1_0(1405);
    fn_1_EB54(1);
    HuPrcSleep(30);
    fn_1_EB54(0);
    HuPrcSleep(30);
    fn_1_0(1405);
    fn_1_EB54(1);
    HuPrcSleep(30);
    fn_1_EB54(0);
    HuPrcSleep(30);
    fn_1_0(1405);
    fn_1_EB54(1);
    HuPrcSleep(30);
    fn_1_EB54(0);
    HuPrcSleep(60);
    fn_1_35F4(1, 0, 30.0f, HU3D_MOTATTR_LOOP);
    lbl_1_bss_2C = 0;
    fn_1_6AB4(1, 0, 1, MESSNUM(MESS_ENDING, 8), -1, 30, 0, 180);
    if (lbl_1_data_38 != -1) {
        HuWinExClose(lbl_1_bss_1A48[lbl_1_data_38]);
        lbl_1_data_38 = -1;
    }
    lbl_1_bss_2C = 1;
    HuPrcSleep(30);
    fn_1_664();
    fn_1_35F4(0, 4, 5.0f, 0U);
    fn_1_35F4(1, 4, 5.0f, 0U);
    HuPrcSleep(6);
    Hu3DMotionSpeedSet(lbl_1_bss_C->mdlId[0], 0.5f);
    Hu3DMotionSpeedSet(lbl_1_bss_10->mdlId[0], 0.5f);
    HuPrcSleep(60);
    fn_1_35F4(0, 5, 30.0f, 0U);
    fn_1_35F4(1, 5, 30.0f, 0U);
    lbl_1_bss_1D5C.channel[4] = fn_1_0(1392);
    fn_1_EB54(1);
    HuPrcSleep(30);
    fn_1_3FC0();
    fn_1_2208(5, 10.0f, 0U, 0, 60);
    HuPrcSleep(180);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

/* Releases the opening cast's lights and object callbacks before the first staff-roll section. */
void fn_1_8CFC(void)
{
    Hu3DLLightKill(lbl_1_bss_C->mdlId[0], lbl_1_bss_1A52[0]);
    Hu3DLLightKill(lbl_1_bss_10->mdlId[0], lbl_1_bss_1A52[1]);
    fn_1_10DEC(0, NULL, 2);
    fn_1_10DEC(1, NULL, 2);
    lbl_1_bss_10->objFunc = NULL;

    fn_1_26D4(lbl_1_bss_8);
    lbl_1_bss_8->objFunc = NULL;
    lbl_1_bss_8 = NULL;

    fn_1_3CD4(lbl_1_bss_C);
    lbl_1_bss_C->objFunc = NULL;
    lbl_1_bss_C = NULL;

    fn_1_3F20(lbl_1_bss_10);
    lbl_1_bss_10->objFunc = NULL;
    lbl_1_bss_10 = NULL;
}

/* Advances the 96 drifting sparkles and starts one when a slot is inactive and the shared countdown
 * reaches zero; assigned by fn_1_93A0. */
void fn_1_8F80(OMOBJ *object)
{
    EndingMotionWork *work;
    s16 model;
    Vec rotation;

    lbl_1_bss_30--;
    for (model = 0, work = lbl_1_bss_34; model < 96; model++, work++) {
        if (work->state == 0 && lbl_1_bss_30 <= 0) {
            lbl_1_bss_30 = 5;
            work->state = 1;
            work->startOrVelocity.x = frandmod(100) - 50;
            work->startOrVelocity.y = frandmod(50) + 50;
            work->startOrVelocity.z = frandmod(100) - 50;
            PSVECNormalize(&work->startOrVelocity, &work->startOrVelocity);
            work->startOrVelocity.x *= 4.0f;
            work->startOrVelocity.y *= 7.0f;
            work->startOrVelocity.z *= 4.0f;
            work->controlOrPosition.x = frandmod(60) - 30;
            work->controlOrPosition.y = frandmod(100) + 230;
            work->controlOrPosition.z = 130.0f;
            work->endOrEffectParams.y = frandmod(20) - (1.0f);
            work->endOrEffectParams.x = frandmod(10) * 0.01f + 0.2f;
            Hu3DModelPosSetV(object->mdlId[model], &work->controlOrPosition);
            Hu3DModelRotSet(object->mdlId[model], 0.0f,
                0.0f, 0.0f);
            Hu3DModelScaleSet(object->mdlId[model], work->endOrEffectParams.x,
                work->endOrEffectParams.x, work->endOrEffectParams.x);
            Hu3DModelAttrReset(object->mdlId[model], 1);
        } else if (work->state == 1) {
            work->controlOrPosition.x += work->startOrVelocity.x;
            work->controlOrPosition.y += work->startOrVelocity.y;
            work->controlOrPosition.z += work->startOrVelocity.z;
            Hu3DModelPosSetV(object->mdlId[model], &work->controlOrPosition);
            fn_1_11208(model, &work->controlOrPosition, 1);
            Hu3DModelRotGet(object->mdlId[model], &rotation);
            rotation.y += work->endOrEffectParams.y;
            Hu3DModelRotSetV(object->mdlId[model], &rotation);
            if (work->controlOrPosition.y > 2000.0f) {
                fn_1_11208(model, NULL, 0);
                work->state = 0;
                Hu3DModelAttrSet(object->mdlId[model], 1);
            }
        }
    }
}

/* Creates the shared sparkle model slots and installs their per-frame callback for fn_1_A6D8. */
void fn_1_93A0(OMOBJ *object)
{
    EndingMotionWork *work;
    s16 model;

    omSetStatBit(object, 256);
    for (model = 0, work = lbl_1_bss_34; model < 96; model++, work++) {
        work->state = 0;
    }
    for (model = 0; model < 96; model++) {
        if (model == 0) {
            object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
                DATANUM(DATA_ending, 52), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            object->mdlId[model] = Hu3DModelLink(object->mdlId[0]);
        }
        Hu3DModelAttrSet(object->mdlId[model], HU3D_ATTR_DISPOFF);
    }
    object->objFunc = fn_1_8F80;
}

inline void fn_1_93A0(OMOBJ *object);

/* Deletes all sparkle model links and their object; fn_1_B0A4 calls this during scene cleanup. */
void fn_1_9498(OMOBJ *object)
{
    s16 model;

    if (object) {
        fn_1_111B0(0);
        for (model = 95; model >= 0; model--) {
            Hu3DModelKill(object->mdlId[model]);
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
    object = NULL;
}

/* Places the active camera at the staff-roll starting view when the scene setup requests it. */
void fn_1_951C(void)
{
    Vec pos;
    Vec up;
    Vec target;

    Hu3DCameraPosGet(1, &pos, &up, &target);
    pos.x = 0.0f;
    pos.y = (250.0f);
    pos.z = (400.0f);
    target.x = 0.0f;
    target.y = (280.0f);
    target.z = 0.0f;
    Hu3DCameraPosSetV(1, &pos, &up, &target);
}

/* Transitions the staff-roll camera through its two framing moves; fn_1_A964 starts this
 * sequence. */
void fn_1_95C0(void)
{
    s16 frame;
    s16 duration;
    Vec position;
    Vec up;
    Vec target;
    Vec nextPosition;
    Vec nextTarget;

    Hu3DCameraPosGet(1, &position, &up, &target);
    duration = 240;
    for (frame = 0; frame < duration; frame++) {
        HuPrcVSleep();
        nextPosition.x = position.x;
        nextPosition.y = fn_1_2A04(position.y, 250.0f, frame, duration);
        nextPosition.z = fn_1_2A04(position.z, 600.0f, frame, duration);
        nextTarget.x = fn_1_2A04(target.x, 0.0f, frame, duration);
        nextTarget.y = fn_1_2A04(target.y, 280.0f, frame, duration);
        nextTarget.z = fn_1_2A04(target.z, 0.0f, frame, duration);
        Hu3DCameraPosSetV(1, &nextPosition, &up, &nextTarget);
    }

    Hu3DCameraPosGet(1, &position, &up, &target);
    duration = 360;
    for (frame = 0; frame < duration; frame++) {
        HuPrcVSleep();
        nextPosition.x = position.x;
        nextPosition.y = fn_1_2A04(position.y, 280.0f, frame, duration);
        nextPosition.z = fn_1_2A04(position.z, 600.0f, frame, duration);
        nextTarget.x = fn_1_2A04(target.x, 0.0f, frame, duration);
        nextTarget.y = fn_1_2A04(target.y, 700.0f, frame, duration);
        nextTarget.z = fn_1_2A04(target.z, 0.0f, frame, duration);
        Hu3DCameraPosSetV(1, &nextPosition, &up, &nextTarget);
    }
}

s16 lbl_1_data_10E = -1;

/* Creates the small center-screen ending emblem sprite for the end-scene presentation. */
void fn_1_A58C(void)
{
    ANIMDATA *animation;
    void *fileData;
    s16 sprite;

    fileData = HuAR_ARAMtoMRAMFileRead(
        DATANUM(DATA_win, 51), ENDING_ANIMATION_ALLOC_TAG, HEAP_MODEL);
    animation = HuSprAnimRead(fileData);
    lbl_1_data_10E = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 1, 0);
    HuSprGrpMemberSet(lbl_1_data_10E, 0, sprite);
    HuSprTPLvlSet(lbl_1_data_10E, 0, (0.30000001192092896f));
    HuSprGrpPosSet(lbl_1_data_10E, (288.0f),
        (240.0f));
    HuSprScaleSet(lbl_1_data_10E, 0, (20.0f),
        (20.0f));
}

/* Removes the center-screen ending emblem when the overlay is dismissed. */
void fn_1_A698(void)
{
    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
}

const HuVecF lbl_1_rodata_288 = { 0.0f, 280.0f, 150.0f };

/* Sets up the staff-roll lights and sparkle object when the credits scene begins. */
void fn_1_A6D8(void)
{
    HuVecF position = lbl_1_rodata_288;

    fn_1_93A0(lbl_1_bss_1C);
    fn_1_F1B8(1, &position);
    HuPrcVSleep();
    fn_1_F23C(1);
    fn_1_F068(1, 1, &position);
    fn_1_F11C(1, 0);
    Hu3DLightAllKill();
    fn_1_7B4();
}

/* Prepares the staff-roll lights and sparkle object for fn_1_A964's camera transition. */
static inline void ending_inline_A6D8(HuVecF *position)
{
    s16 light;

    *position = lbl_1_rodata_288;
    fn_1_93A0(lbl_1_bss_1C);
    fn_1_F1B8(1, position);
    HuPrcVSleep();
    fn_1_F23C(1);
    fn_1_F068(1, 1, position);
    fn_1_F11C(1, 0);
    Hu3DLightAllKill();
    fn_1_7B4();
}

/* Presents the first staff-roll section and starts its camera move; called by the ending
 * entrypoint. */
void fn_1_A964(void)
{
    OMOBJ *groupObject;
    OMOBJ *boardObject;
    OMOBJ *characterObject;
    OMOBJ *endingObject;
    EndingMotionWork *motionWork;
    s16 row;
    s16 column;
    s16 firstMotion;
    s16 secondMotion;
    s16 stateIndex;
    s16 modelIndex;
    s16 lightIndex;
    s32 firstSound;
    s32 secondSound;
    HuVecF position;
    HuVecF cameraTarget;
    HuVecF cameraUp;
    HuVecF cameraPosition;
    EndingLightVectors lightPosition;
    EndingLightVectors lightDirection;
    GXColor lightColor;
    s16 sprite;
    void *fileData;
    ANIMDATA *animation;

    HuAudFXStop(lbl_1_bss_1D5C.channel[4]);
    WipeCreate(2, 0, 10);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }

    fn_1_8CFC();

    ending_inline_A6D8(&position);

    Hu3DCameraPosGet(1, &cameraPosition, &cameraUp, &cameraTarget);
    cameraPosition.x = 0.0f;
    cameraPosition.y = 250.0f;
    cameraPosition.z = 400.0f;
    cameraTarget.x = 0.0f;
    cameraTarget.y = 280.0f;
    cameraTarget.z = 0.0f;
    Hu3DCameraPosSetV(1, &cameraPosition, &cameraUp, &cameraTarget);

    if (lbl_1_bss_26 == 0) {
        firstSound = HuAudFXPlay(ENDING_FX_ID_1393);
    } else {
        firstSound = -1;
    }
    lbl_1_bss_1D5C.channel[8] = firstSound;
    if (lbl_1_bss_26 == 0) {
        secondSound = HuAudFXPlay(ENDING_FX_ID_1394);
    } else {
        secondSound = -1;
    }
    lbl_1_bss_1D5C.channel[9] = secondSound;

    fileData = HuAR_ARAMtoMRAMFileRead(
        DATANUM(DATA_win, 51), ENDING_ANIMATION_ALLOC_TAG, HEAP_MODEL);
    animation = HuSprAnimRead(fileData);
    lbl_1_data_10E = HuSprGrpCreate(1);
    sprite = HuSprCreate(animation, 1, 0);
    HuSprGrpMemberSet(lbl_1_data_10E, 0, sprite);
    HuSprTPLvlSet(lbl_1_data_10E, 0, 0.3f);
    HuSprGrpPosSet(lbl_1_data_10E, 288.0f, 240.0f);
    HuSprScaleSet(lbl_1_data_10E, 0, 20.0f, 20.0f);
    WipeCreate(1, 0, 10);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    fn_1_95C0();
    HuPrcSleep(120);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

/* Frees the first staff-roll props and sparkle object after that section ends. */
void fn_1_B0A4(void)
{
    OMOBJ *object;

    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }
    object = lbl_1_bss_4;
    Hu3DMotionKill(object->mtnId[0]);
    Hu3DModelKill(object->mdlId[0]);

    fn_1_9498(lbl_1_bss_1C);
    lbl_1_bss_1C->objFunc = NULL;
    lbl_1_bss_1C = NULL;

    fn_1_4A0C(lbl_1_bss_18);
    lbl_1_bss_18->objFunc = NULL;
    lbl_1_bss_18 = NULL;
}

const HuVecF lbl_1_rodata_294 = { -1710.0f, 285.0f, 185.0f };

const HuVecF lbl_1_rodata_2A0 = { -300.0f, 350.0f, 0.0f };

/* Loads the staff models and configures their particle effects before the next credits section. */
void fn_1_B244(void)
{
    OMOBJ *object;
    s16 index;
    HuVecF position0;
    HuVecF position1;

    omSetStatBit(lbl_1_bss_14, 256);
    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 1] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 1) + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 1] = Hu3DMotionIDGet(
            object->mdlId[index + 1]);
        Hu3DModelAttrSet(object->mdlId[index + 1], HU3D_ATTR_DISPOFF);
        Hu3DMotionShiftSet(object->mdlId[index + 1],
            object->mtnId[index + 1], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
    }

    position0 = lbl_1_rodata_294;
    position1 = lbl_1_rodata_2A0;
    fn_1_F1B8(1, &position0);
    fn_1_F23C(0);
    fn_1_F068(1, 1, NULL);
    fn_1_F11C(1, 2);

    object = lbl_1_bss_14;
    object->mdlId[0] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 48), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[1] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 51), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index] = Hu3DJointMotion(object->mdlId[0],
            HuDataSelHeapReadNum(DATANUM(DATA_ending, 49) + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelScaleSet(object->mdlId[1], (0.800000011920929f),
        (0.800000011920929f), (0.800000011920929f));
    Hu3DModelHookSet(object->mdlId[0], "c005m0-itemhook-r",
        object->mdlId[1]);
    Hu3DMotionSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[0]);
    Hu3DMotionSet(lbl_1_bss_4->mdlId[0], lbl_1_bss_4->mtnId[0]);
    HuPrcVSleep();
    Hu3DMotionTimeSet(lbl_1_bss_14->mdlId[0], (60.0f));
    Hu3DMotionTimeSet(lbl_1_bss_4->mdlId[1], (60.0f));
}

/* Runs the staff-roll model, lighting, and audio sequence; the ending entrypoint calls this
 * stage. */
void fn_1_B568(void)
{
    s16 frame;
    OMOBJ *mainObject;
    OMOBJ *staffObject;
    OMOBJ *effectObject;
    s16 staffModel;
    s16 effectModel;
    HU3D_MODEL *modelData;
    HU3D_LIGHTID lightId;
    float color;
    s32 sound;

    HuAudFXStop(lbl_1_bss_1D5C.channel[0]);
    HuAudFXStop(lbl_1_bss_1D5C.channel[1]);
    HuAudFXStop(lbl_1_bss_1D5C.channel[8]);
    HuAudFXStop(lbl_1_bss_1D5C.channel[9]);
    WipeCreate(2, 0, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    if (lbl_1_data_10E != -1) {
        HuSprGrpKill(lbl_1_data_10E);
    }

    mainObject = lbl_1_bss_4;
    Hu3DMotionKill(mainObject->mtnId[0]);
    Hu3DModelKill(mainObject->mdlId[0]);

    staffObject = lbl_1_bss_1C;
    if (staffObject) {
        fn_1_111B0(0);
        for (staffModel = 95; staffModel >= 0; staffModel--) {
            Hu3DModelKill(staffObject->mdlId[staffModel]);
        }
        omDelObjEx(lbl_1_bss_0, staffObject);
    }
    staffObject = NULL;
    lbl_1_bss_1C->objFunc = NULL;
    lbl_1_bss_1C = NULL;

    effectObject = lbl_1_bss_18;
    if (effectObject) {
        fn_1_E1EC(0, NULL);
        fn_1_EAB8(0, NULL);
        for (effectModel = 0; effectModel < 16; effectModel++) {
            Hu3DMotionKill(effectObject->mtnId[effectModel]);
            Hu3DModelKill(effectObject->mdlId[effectModel]);
        }
        omDelObjEx(lbl_1_bss_0, effectObject);
    }
    effectObject = NULL;
    lbl_1_bss_18->objFunc = NULL;
    lbl_1_bss_18 = NULL;

    fn_1_B244();
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[1], 1);
    Hu3DModelAttrSet(lbl_1_bss_4->mdlId[2], 1);
    Hu3DModelCameraInfoSet(lbl_1_bss_4->mdlId[1], 1);
    Hu3DLightAllKill();
    Hu3DModelLightInfoSet(lbl_1_bss_4->mdlId[1], 1);
    modelData = &Hu3DData[lbl_1_bss_4->mdlId[1]];
    lightId = modelData->lightId[0];
    Hu3DGLightColorSet(lightId, 64, 64, 64, 255);
    if (lbl_1_bss_26 == 0) {
        HuAudFXPlay(ENDING_FX_ID_1396);
    }

    WipeCreate(1, 0, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    HuPrcSleep(35);
    for (frame = 0; frame < 65; frame++) {
        HuPrcVSleep();
        color = fn_1_DDF8((64.0f), (255.0f),
            frame, (65.0f));
        Hu3DGLightColorSet(lightId, color, color, color, 255);
    }
    Hu3DGLightColorSet(lightId, 255, 255, 255, 255);
    HuPrcSleep(130);

    Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[0], lbl_1_bss_14->mtnId[1],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[2], lbl_1_bss_4->mtnId[2],
        0.0f, 0.0f, 0);
    Hu3DMotionShiftSet(lbl_1_bss_4->mdlId[1], lbl_1_bss_4->mtnId[1],
        0.0f, 0.0f, 0);
    Hu3DModelAttrSet(lbl_1_bss_4->mdlId[1], 1);
    Hu3DModelAttrReset(lbl_1_bss_4->mdlId[2], 1);
    Hu3DModelCameraInfoSet(lbl_1_bss_4->mdlId[2], 1);
    Hu3DLightAllKill();
    Hu3DModelLightInfoSet(lbl_1_bss_4->mdlId[2], 1);
    if (lbl_1_bss_26 == 0) {
        sound = HuAudFXPlay(ENDING_FX_ID_1397);
    } else {
        sound = -1;
    }
    lbl_1_bss_1D5C.channel[5] = sound;
    HuPrcSleep(120);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

/* Releases the staff models and their hooked attachments after the model sequence. */
void fn_1_BB10(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 1]);
        Hu3DModelKill(object->mdlId[model + 1]);
    }
    object = lbl_1_bss_14;
    Hu3DModelHookReset(object->mdlId[0]);
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model]);
    }
    Hu3DModelKill(object->mdlId[0]);
    Hu3DModelKill(object->mdlId[1]);
}

const HuVecF lbl_1_rodata_2B8 = { 270.0f, 270.0f, 2123.0f };

/* Builds the next staff-roll model set and creates its prop before the following scene
 * animation. */
void fn_1_BBEC(void)
{
    OMOBJ *object;
    s16 index;

    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 3) + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 3] = Hu3DMotionIDGet(
            object->mdlId[index + 3]);
        Hu3DMotionShiftSet(object->mdlId[index + 3],
            object->mtnId[index + 3], 0.0f,
            0.0f, HU3D_MOTATTR_LOOP);
    }
    Hu3DModelShadowMapSet(object->mdlId[3]);
    {
        HuVecF position = lbl_1_rodata_2B8;

        fn_1_F1B8(1, &position);
    }

    object = lbl_1_bss_14;
    object->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        object->mtnId[index + 2] = Hu3DJointMotion(object->mdlId[2],
            HuDataSelHeapReadNum(DATANUM(DATA_ending, 46) + index, HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    Hu3DModelPosSet(object->mdlId[2], 0.0f,
        0.0f, 0.0f);
    Hu3DMotionShiftSet(object->mdlId[2], object->mtnId[3],
        0.0f, 0.0f, 0);
    Hu3DModelShadowSet(object->mdlId[2]);
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[4], (0.25f));
}

/* Runs the final staff-roll camera and prop sequence; called by the ending entrypoint. */
void fn_1_BE30(void)
{
    OMOBJ *mainObject;
    OMOBJ *object;
    s16 index;
    s16 cleanupIndex;
    s16 frame;
    s32 sound;
    HuVecF position;
    float height;

    HuAudFXStop(lbl_1_bss_1D5C.channel[5]);
    WipeCreate(2, 0, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    object = lbl_1_bss_4;
    for (cleanupIndex = 0; cleanupIndex < 2; cleanupIndex++) {
        Hu3DMotionKill(object->mtnId[cleanupIndex + 1]);
        Hu3DModelKill(object->mdlId[cleanupIndex + 1]);
    }

    object = lbl_1_bss_14;
    Hu3DModelHookReset(object->mdlId[0]);
    for (cleanupIndex = 0; cleanupIndex < 2; cleanupIndex++) {
        Hu3DMotionKill(object->mtnId[cleanupIndex]);
    }
    Hu3DModelKill(object->mdlId[0]);
    Hu3DModelKill(object->mdlId[1]);

    mainObject = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        mainObject->mdlId[index + 3] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 3) + index, HU_MEMNUM_OVL, HEAP_MODEL));
        mainObject->mtnId[index + 3] = Hu3DMotionIDGet(mainObject->mdlId[index + 3]);
        Hu3DMotionShiftSet(mainObject->mdlId[index + 3], mainObject->mtnId[index + 3],
            0.0f, 0.0f, HU3D_MOTATTR_LOOP);
    }
    Hu3DModelShadowMapSet(mainObject->mdlId[3]);

    position = lbl_1_rodata_2B8;
    fn_1_F1B8(1, &position);

    mainObject = lbl_1_bss_14;
    mainObject->mdlId[2] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 45), HU_MEMNUM_OVL, HEAP_MODEL));
    for (index = 0; index < 2; index++) {
        mainObject->mtnId[index + 2] = Hu3DJointMotion(
            mainObject->mdlId[2], HuDataSelHeapReadNum(
                DATANUM(DATA_ending, 46) + index, HU_MEMNUM_OVL, HEAP_MODEL));
    }
    Hu3DModelPosSet(mainObject->mdlId[2], 0.0f,
        0.0f, 0.0f);
    Hu3DMotionShiftSet(mainObject->mdlId[2], mainObject->mtnId[3],
        0.0f, 0.0f, 0);
    Hu3DModelShadowSet(mainObject->mdlId[2]);
    HuPrcVSleep();

    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[4], (0.25f));
    Hu3DModelCameraInfoSet(lbl_1_bss_4->mdlId[3], 1);
    Hu3DLightAllKill();
    Hu3DModelLightInfoSet(lbl_1_bss_4->mdlId[3], 1);
    if (lbl_1_bss_26 == 0) {
        HuAudFXPlay(ENDING_FX_ID_1399);
    }
    WipeCreate(1, 0, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }

    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    for (frame = 0; frame < 160; frame++) {
        HuPrcVSleep();
        height = fn_1_DDF8(0.0f, (200.0f),
            frame, (200.0f));
        Hu3DModelPosSet(lbl_1_bss_14->mdlId[2], 0.0f,
            0.0f, height);
    }

    HuPrcSleep(60);
    Hu3DModelPosSet(lbl_1_bss_14->mdlId[2], 0.0f,
        0.0f, 0.0f);
    Hu3DModelRotSet(lbl_1_bss_14->mdlId[2], 0.0f,
        (-25.0f), 0.0f);
    Hu3DMotionShiftSet(lbl_1_bss_14->mdlId[2], lbl_1_bss_14->mtnId[2],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);

    Hu3DModelCameraInfoSet(lbl_1_bss_4->mdlId[4], 1);
    Hu3DLightAllKill();
    Hu3DModelLightInfoSet(lbl_1_bss_4->mdlId[4], 1);
    if (lbl_1_bss_26 == 0) {
        sound = HuAudFXPlay(ENDING_FX_ID_1400);
    } else {
        sound = -1;
    }
    lbl_1_bss_1D5C.channel[6] = sound;
    HuPrcSleep(120);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

/* Releases the two final staff props and hides the ending burst models before fn_1_C7DC
 * continues. */
void fn_1_C40C(void)
{
    OMOBJ *object;
    s16 model;

    object = lbl_1_bss_4;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 3]);
        Hu3DModelKill(object->mdlId[model + 3]);
    }
    fn_1_F1B8(0, 0);
    fn_1_F068(1, 0, NULL);
    object = lbl_1_bss_14;
    for (model = 0; model < 2; model++) {
        Hu3DMotionKill(object->mtnId[model + 2]);
    }
    Hu3DModelKill(object->mdlId[2]);
}

/* Loads the ending character markers and resets the camera for the closing scene. */
void fn_1_C4F0(void)
{
    OMOBJ *object;
    Vec pos;
    Vec up;
    Vec target;

    fn_1_14F4();
    object = lbl_1_bss_4;
    object->mdlId[7] = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 7), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mtnId[7] = Hu3DMotionIDGet(object->mdlId[7]);
    Hu3DMotionShiftSet(object->mdlId[7], object->mtnId[7],
        0.0f, 0.0f, HU3D_MOTATTR_LOOP);

    Hu3DCameraPosGet(1, &pos, &up, &target);
    pos.x = 0.0f;
    pos.y = (500.0f);
    pos.z = (1500.0f);
    target.x = 0.0f;
    target.y = (500.0f);
    target.z = 0.0f;
    Hu3DCameraPosSetV(1, &pos, &up, &target);
    fn_1_EB54(1);
}

inline void fn_1_12A8(s16 state);
inline void fn_1_C40C(void);
inline void fn_1_C4F0(void);

const EndingScenePositions lbl_1_rodata_2CC = {
    { { -150.0f, 700.0f, 1200.0f }, { 150.0f, 700.0f, 1200.0f }, { 0.0f, 700.0f, 1200.0f } }
};

/* Runs the three-character closing tableau and its staggered particle and sound cues. */
void fn_1_C7DC(void)
{
    float particleY = (650.0f);
    float phaseDuration;
    EndingScenePositions positions = lbl_1_rodata_2CC;

    HuAudFXStop(lbl_1_bss_1D5C.channel[6]);
    WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 10);
    while (WipeCheck()) {
        HuPrcVSleep();
    }
    fn_1_C40C();
    fn_1_C4F0();

    fn_1_12A8(0);
    fn_1_EAB8(1, &positions.position[0]);
    positions.position[0].y = particleY;
    fn_1_F1B8(1, &positions.position[0]);
    fn_1_F23C(1);
    HuPrcSleep(60);
    lbl_1_bss_1D5C.channel[10] = fn_1_0(1402);
    WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 10);
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    phaseDuration = (60.0f);
    HuPrcSleep(60);

    lbl_1_bss_1D5C.channel[11] = fn_1_0(1403);
    fn_1_12A8(1);
    fn_1_EAB8(1, &positions.position[1]);
    positions.position[1].y = particleY;
    fn_1_F1B8(1, &positions.position[1]);
    phaseDuration = (60.0f);
    HuPrcSleep(48);

    lbl_1_bss_1D5C.channel[7] = fn_1_0(1404);
    fn_1_12A8(2);
    fn_1_EAB8(1, &positions.position[2]);
    positions.position[2].y = particleY;
    fn_1_F1B8(1, &positions.position[2]);
    phaseDuration = (120.0f);
    HuPrcSleep(132);
    lbl_1_bss_24 = 0;
    HuPrcSleep(5);
}

/* Removes the last camera model and returns the scene sprite group to its centered layout. */
void fn_1_D028(void)
{
    OMOBJ *object = lbl_1_bss_4;

    Hu3DMotionKill(object->mtnId[7]);
    Hu3DModelKill(object->mdlId[7]);
    fn_1_EAB8(0, NULL);
    fn_1_F1B8(0, NULL);
    HuSprGrpPosSet(lbl_1_bss_1A1C[0], 0.0f,
        0.0f);
    fn_1_E0EC(lbl_1_bss_1A1C[0], 4);
    lbl_1_bss_4->work[0] = 3;
    lbl_1_bss_4->objFunc = fn_1_1160;
    lbl_1_bss_4->objFunc = NULL;
}

/* Loads the final camera and background models used by the closing part of the ending. */
void fn_1_D104(void)
{
    OMOBJ *object;
    s16 index;

    object = lbl_1_bss_4;
    for (index = 0; index < 2; index++) {
        object->mdlId[index + 5] = Hu3DModelCreate(HuDataSelHeapReadNum(
            DATANUM(DATA_ending, 5) + index, HU_MEMNUM_OVL, HEAP_MODEL));
        object->mtnId[index + 5] = Hu3DMotionIDGet(
            object->mdlId[index + 5]);
        Hu3DMotionShiftSet(object->mdlId[index + 5],
            object->mtnId[index + 5], 0.0f, 0.0f,
            HU3D_MOTATTR_LOOP);
    }
    fn_1_16D4();
    object->mtnId[7] = Hu3DMotionCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_ending, 8), HU_MEMNUM_OVL, HEAP_MODEL));
    object->mdlId[7] = Hu3DModelCameraCreate(object->mtnId[7], TRUE);
    Hu3DCameraMotionStart(object->mdlId[7], TRUE);
    Hu3DLightAllKill();
    fn_1_7B4();
    HuPrcVSleep();
    Hu3DMotionSpeedSet(lbl_1_bss_4->mdlId[6], 0.04f);
}

/* Runs the last dialogue and exit wipe; the ending entrypoint calls this final scene stage. */
void fn_1_D434(void)
{
    HuAudFXStop(lbl_1_bss_1D5C.channel[7]);
    WipeCreate(2, 6, 120);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    fn_1_D028();

    fn_1_D104();
    WipeCreate(1, 6, 120);
    while (WipeCheck() != 0) {
        HuPrcVSleep();
    }
    lbl_1_bss_2C = 1;
    lbl_1_bss_24 = 1;
    HuPrcSleep(120);
    lbl_1_bss_2C = 0;

    fn_1_DAC(0, MESSNUM(MESS_ENDING, 9), 300);

    fn_1_DAC(0, MESSNUM(MESS_ENDING, 10), 300);

    fn_1_DAC(0, MESSNUM(MESS_ENDING, 11), 300);

    fn_1_1798();
    HuPrcSleep(200);
    lbl_1_bss_24 = 0;
    HuPrcSleep(100);
}

/* Waits ten scheduler ticks as a short ending-sequence delay callback. */
void fn_1_DCF0(void)
{
    HuPrcSleep(10);
}

/* Dispatches the ending scene stages while audio remains enabled; the overlay calls this
 * entrypoint. */
void fn_1_DD14(void)
{
    if (lbl_1_bss_26 == 0) {
        HuPrcSleep(10);
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_702C();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_A964();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_B568();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_BE30();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_C7DC();
    }
    if (lbl_1_bss_26 == 0) {
        fn_1_D434();
    }
    if (lbl_1_bss_26 == 1) {
        while (TRUE) {
            HuPrcVSleep();
        }
    }
}
