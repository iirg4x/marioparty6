/* Sets up and runs the Shoot Yer Mouth Off minigame. */
#include "dolphin/gx.h"

#include "dolphin/math.h"

#include "dolphin/types.h"

#include "REL/m666dll/state.h"

#include "game/main.h"

#include "game/gamework.h"

#include "game/audio.h"

#include "msm_se.h"

#include "REL/m666dll/player.h"

#include "game/hu3d.h"

#include "dolphin.h"

#include "game/mg/actman.h"

#include "game/charman.h"

#include "game/hsfex.h"

#include "game/flag.h"

#include "game/mg/seqman.h"

#include "game/object.h"

#include "game/gamemes.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/wipe.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "game/mic.h"

#include "REL/m666dll/recovered.h"

#include "REL/m666dll/tables.h"

#define M666_CLUSTER_MOTION_CLEAR_MASK HU3D_CLUSTER_ATTR_LOOP
#define M666_TRANSFORM_TRAILING_DATA_TWO (2 << 16)
#define M666_TRANSFORM_TRAILING_DATA_THREE (3 << 16)
#define M666_TRANSFORM_TRAILING_DATA_FIVE (5 << 16)
#define M666_TRANSFORM_TRAILING_DATA_SIX (6 << 16)
#define M666_TRANSFORM_TRAILING_DATA_SEVEN (7 << 16)
#define M666_FLOAT_30_BIT_PATTERN 0x41F00000
#define M666_FLOAT_28_BIT_PATTERN 0x41E00000

/* Sound effects named for the events that trigger them. */
#define MSM_SE_M666_TARGET_ASSIGNMENT 2173
#define MSM_SE_M666_BOMB_HOOK 2174
#define MSM_SE_M666_LANE_CENTER_CROSSING 2175
#define MSM_SE_M666_LANE_ACTIVATION 2176
#define MSM_SE_M666_MARKER_ROTATION 2177
#define MSM_SE_M666_EVENT_ANIMATION_FRAME 2178
#define MSM_SE_M666_OBSTACLE_RESET 2179
#define MSM_SE_M666_PLAYER_RELEASE 2180
#define MSM_SE_M666_EVENT_FALLBACK 2181

typedef struct {
    MGACTOR *actor;
    MGACTOR *secondaryActor;
    s16 modelId;
    s16 alternateModelId;
    union {
        struct {
            s16 startMotionId;
            s16 fallMotionId;
            s16 loopMotionId;
            s16 alternateMotionId;
        } obstacleMotions;
        s16 jointMotionIds[4];
    } motion;
    s16 secondaryModelId;
    s16 slotIndex;
    s16 state;
    s16 timer;
    s16 targetPlayerIndex;
    /* Written with the obstacle grid-cell index; game code never reads this field. */
    s16 unknown1E;
    f32 verticalStep;
    f32 actorRotation;
} M666MovingObstacle;

typedef struct {
    s16 modelId;
    s16 active;
    f32 scale;
    f32 alpha;
} M666SequenceParticle;

typedef struct {
    /* Game code never reads these bytes. */
    u8 unk00[4];
    s16 rootModelId;
    /* Game code never reads these bytes. */
    u8 unk06[2];
    union {
        u8 particleStorage[96];
        M666SequenceParticle particles[8];
    };
    s16 state;
    union {
        s16 status;
        s16 laneIndex;
    };
    union {
        u8 stateData[8];
        struct {
            s16 timer;
            f32 movement;
        };
    };
    union {
        f32 initialAlpha;
        f32 alpha;
    };
} M666SequenceItemView;

typedef M666SequenceItemView M666SequenceLane;

typedef struct {
    M666SequenceItemView items[3];
} M666SequenceGroupView;

s16 lbl_1_bss_EA0[9];
Vec lbl_1_bss_E94;
s32 lbl_1_bss_E90;
M666ActorWork lbl_1_bss_E3C[3];
s16 lbl_1_bss_E38;
f32 lbl_1_bss_E14[9];
M666InitialState lbl_1_bss_DF0;
M666MotionWork lbl_1_bss_D84[3];
M666MovingObstacle lbl_1_bss_604[6][8];
M666Bss580Entry lbl_1_bss_580[6];
M666SequenceGroupView lbl_1_bss_148[3];

void fn_1_17C8(s16 motionIndex, f32 blend, u32 flags);

void fn_1_7E9C(void);

s16 fn_1_1080(void);

void fn_1_143C(void);

void fn_1_1004(s16 motionIndex, f32 blend);

void fn_1_1680(void);

void fn_1_8164(void);

void fn_1_F8C(void);

void fn_1_80E0(void);

void fn_1_9D4(void);

void fn_1_15F0(void);

void fn_1_7DA8(void);

void fn_1_84C4(void);

void fn_1_B54(void);

void fn_1_4A40(void);

void fn_1_4F24(void);

void fn_1_81E4(void);

void fn_1_A1CC(void);

void fn_1_B55C(void);

void fn_1_D794(void);

/* Initializes the play scene after fn_1_D8 prepares the microphone interface. */
void fn_1_8BC(void) {
    s16 nightMode;

    memset(&lbl_1_bss_DF0, 0, 36);
    nightMode = GwMgNightF;
    lbl_1_bss_DF0.night = nightMode;
    fn_1_9D4();
    fn_1_B54();
    fn_1_4A40();
    fn_1_4F24();
    fn_1_81E4();
    fn_1_A1CC();
    fn_1_B55C();
    fn_1_D794();
}

/* Clears the actor and sound state after fn_1_244 closes the sequence listener. */
void fn_1_92C(void) {
    s32 soundIndex;

    fn_1_15F0();
    fn_1_7DA8();
    fn_1_84C4();
    soundIndex = 0;
    while (soundIndex < 9) {
        lbl_1_bss_DC[soundIndex].state = 0;
        if ((s32) lbl_1_bss_DC[soundIndex].soundHandle != -1) {
            HuAudFXStop(lbl_1_bss_DC[soundIndex].soundHandle);
            lbl_1_bss_DC[soundIndex].soundHandle = -1;
        }
        soundIndex += 1;
    }
}

extern HUPROCESS *lbl_1_bss_4;

typedef struct {
    Vec positions[6];
} M666ShadowPositionTable;

typedef struct {
    f32 x;
    f32 y;
} M666ShadowPlane;

typedef struct {
    union {
        Vec position;
        struct {
            f32 x;
            f32 y;
            f32 z;
        };
    };
    /* Game code never reads these bytes. */
    u8 unk0C[4];
    union {
        s16 direction;
        s16 unknown10;
    };
} M666DataPoint;

typedef struct {
    Point3d position;
} M666SequencePosition;

typedef struct {
    u32 fileId;
    u32 flags;
} M666JointMotionFile;

typedef struct {
    Point3d position;
    union {
        f32 rotation;
        f32 rotationY;
    };
    /* Game code never reads these final four bytes. */
    union {
        u32 trailingData;
        f32 unobserved10;
        u32 unknown10;
    };
} M666DataTransform;

typedef M666DataTransform M666ModelTransform;

typedef struct {
    u32 fileId;
    u32 flags;
} M666JointMotionResource;

extern M666SoundWork lbl_1_bss_DC[9];
extern M666Bss98Work lbl_1_bss_98;

Vec lbl_1_data_70 = {-2000.0f, 10000.0f, -400.0f};

Vec lbl_1_data_7C = {-0.4f, -0.8f, -0.8f};

GXColor lbl_1_data_88[2] = {
    {192, 192, 192, 192}, {144, 144, 144, 144}
};

u32 lbl_1_data_90[2] = {DATANUM(DATA_m666, 0), DATANUM(DATA_m666, 12)};

u32 lbl_1_data_98[2] = {DATANUM(DATA_m666, 1), DATANUM(DATA_m666, 13)};

u32 lbl_1_data_A0[2] = {DATANUM(DATA_m666, 2), DATANUM(DATA_m666, 14)};

u32 lbl_1_data_A8[9][2] = {
    {DATANUM(DATA_m666, 3), DATANUM(DATA_m666, 15)},
    {DATANUM(DATA_m666, 4), DATANUM(DATA_m666, 16)},
    {DATANUM(DATA_m666, 5), DATANUM(DATA_m666, 17)},
    {DATANUM(DATA_m666, 6), DATANUM(DATA_m666, 18)},
    {DATANUM(DATA_m666, 7), DATANUM(DATA_m666, 19)},
    {DATANUM(DATA_m666, 8), DATANUM(DATA_m666, 20)},
    {DATANUM(DATA_m666, 9), DATANUM(DATA_m666, 21)},
    {DATANUM(DATA_m666, 10), DATANUM(DATA_m666, 22)},
    {DATANUM(DATA_m666, 11), DATANUM(DATA_m666, 23)}
};

Vec lbl_1_data_F0[9] = {
    {-160.0f, 7.0f, -830.0f}, {140.0f, 7.0f, -830.0f},
    {440.0f, 7.0f, -830.0f}, {830.0f, 7.0f, 140.0f},
    {440.0f, 7.0f, 830.0f}, {140.0f, 7.0f, 830.0f},
    {-160.0f, 7.0f, 830.0f}, {-830.0f, 7.0f, 440.0f},
    {-830.0f, 7.0f, -160.0f}
};

Vec lbl_1_data_15C[3] = {
    {280.0f, 0.0f, 280.0f}, {-280.0f, 0.0f, 280.0f},
    {280.0f, 0.0f, -280.0f}
};
f32 lbl_1_data_180[6] = {-135.0f, 135.0f, -45.0f, 960.0f, 0.0f, 460.0f};
f32 lbl_1_data_198[4] = {30.0f, 35.0f, 40.0f, 50.0f};
Point3d lbl_1_data_1A8[3] = {
    {-600.0f, -100.0f, -600.0f}, {600.0f, -100.0f, -600.0f},
    {-600.0f, -100.0f, 600.0f}
};
f32 lbl_1_data_1CC[3] = {45.0f, -45.0f, 135.0f};
M666JointMotionFile lbl_1_data_1D8[7] = {
    {DATANUM(DATA_m666, 27), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 28), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 29), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 30), 0}, {DATANUM(DATA_m666, 31), 0},
    {DATANUM(DATA_m666, 32), 0}, {DATANUM(DATA_m666, 33), 0}
};
M666SequencePosition lbl_1_data_210[9] = {
    {{1500.0f, 200.0f, 0.0f}}, {{2080.0f, 200.0f, 0.0f}},
    {{2350.0f, 200.0f, 0.0f}}, {{-1500.0f, 200.0f, 300.0f}},
    {{-2080.0f, 200.0f, 300.0f}}, {{-2350.0f, 200.0f, 300.0f}},
    {{-1500.0f, 200.0f, -300.0f}}, {{-2080.0f, 200.0f, -300.0f}},
    {{-2350.0f, 200.0f, -300.0f}}
};
f32 lbl_1_data_27C[9] = {
    -90.0f, -90.0f, -90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f, 90.0f
};

M666DataTransform lbl_1_data_2A0[48] = {
    {{-300.0f, 60.0f, -1820.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -2040.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -2260.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -2480.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -2700.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -2920.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -3140.0f}, 0.0f, 65536},
    {{-300.0f, 60.0f, -3360.0f}, 0.0f, 65536},
    {{0.0f, 60.0f, -1820.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -2040.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -2260.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -2480.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -2700.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -2920.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -3140.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{0.0f, 60.0f, -3360.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_TWO},
    {{300.0f, 60.0f, -1820.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -2040.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -2260.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -2480.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -2700.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -2920.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -3140.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, -3360.0f}, 0.0f, M666_TRANSFORM_TRAILING_DATA_THREE},
    {{300.0f, 60.0f, 1820.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 2040.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 2260.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 2480.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 2700.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 2920.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 3140.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{300.0f, 60.0f, 3360.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_FIVE},
    {{0.0f, 60.0f, 1820.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 2040.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 2260.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 2480.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 2700.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 2920.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 3140.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{0.0f, 60.0f, 3360.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SIX},
    {{-300.0f, 60.0f, 1820.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 2040.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 2260.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 2480.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 2700.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 2920.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 3140.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN},
    {{-300.0f, 60.0f, 3360.0f}, 180.0f, M666_TRANSFORM_TRAILING_DATA_SEVEN}
};
M666JointMotionResource lbl_1_data_660[4] = {
    {DATANUM(DATA_m666, 35), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 36), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 37), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_m666, 38), HU3D_MOTATTR_LOOP}
};
M666MotionData lbl_1_data_680[6] = {
    {DATANUM(DATA_mariomot, 0), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_mariomot, 1), HU3D_MOTATTR_LOOP},
    {DATANUM(DATA_mariomot, 94), 0}, {DATANUM(DATA_mariomot, 64), 0},
    {DATANUM(DATA_mariomot, 6), 0}, {DATANUM(DATA_mariomot, 7), 0}
};
unsigned int lbl_1_data_6B0[16] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 3),
    DATANUM(DATA_mariomot, 4), DATANUM(DATA_mariomot, 6),
    DATANUM(DATA_mariomot, 40), DATANUM(DATA_mariomot, 8),
    DATANUM(DATA_mariomot, 10), DATANUM(DATA_mariomot, 9),
    DATANUM(DATA_mariomot, 24), DATANUM(DATA_mariomot, 23),
    DATANUM(DATA_mariomot, 17), DATANUM(DATA_mariomot, 18),
    DATANUM(DATA_mariomot, 34), 0
};
u32 lbl_1_data_6F0[14] = {
    DATANUM(DATA_m666, 46), DATANUM(DATA_m666, 47),
    DATANUM(DATA_m666, 48), DATANUM(DATA_m666, 49),
    DATANUM(DATA_m666, 50), DATANUM(DATA_m666, 51),
    DATANUM(DATA_m666, 52), DATANUM(DATA_m666, 53),
    DATANUM(DATA_m666, 54), DATANUM(DATA_m666, 55),
    DATANUM(DATA_m666, 56), DATANUM(DATA_m666, 57),
    DATANUM(DATA_m666, 58), DATANUM(DATA_m666, 59)
};

M666DataPoint lbl_1_data_728[9] = {
    {{-300.0f, -100.0f, -680.0f}, {67, 52, 0, 0}, 0},
    {{0.0f, -100.0f, -680.0f}, {67, 52, 0, 0}, 0},
    {{300.0f, -100.0f, -680.0f}, {67, 52, 0, 0}, 0},
    {{680.0f, -100.0f, 0.0f}, {66, 180, 0, 0}, 1},
    {{300.0f, -100.0f, 680.0f}, {0, 0, 0, 0}, 2},
    {{0.0f, -100.0f, 680.0f}, {0, 0, 0, 0}, 2},
    {{-300.0f, -100.0f, 680.0f}, {0, 0, 0, 0}, 2},
    {{-680.0f, -100.0f, 300.0f}, {194, 180, 0, 0}, 3},
    {{-680.0f, -100.0f, -300.0f}, {194, 180, 0, 0}, 3}
};

extern s32 HuMCProbe(s32 channel);

extern void omVibrate(s16 playerNo, s16 duration, s16 off, s16 on);

typedef struct {
    s16 playerNumber;
    s16 charNo;
    s8 padNo;
    u16 cameraBit;
    void *object;
    u8 actorWork[48];
    MGACTOR *rotationActor;
} M666RotationPlayer;

void fn_1_1198(s16 vibrationMode);

void fn_1_125C(s16 playerIndex);

typedef struct {
    u16 status;
    s16 nextObject;
    s16 priority;
    s16 previousObject;
    s16 nextObjectInGroup;
    s16 nextObjectNumber;
    s16 groupNumber;
    u16 memberNumber;
    u32 mode;
    void *callback;
    Vec translation;
    Vec rotation;
    Vec scale;
    u16 modelCount;
    s16 *modelIds;
    u16 motionCount;
    s16 *motionIds;
} M666MotionObjectView;

typedef struct {
    s16 playerNumber;
    s16 charNo;
    s8 padNo;
    u16 cameraBit;
    M666MotionObjectView *omObj;
} M666MotionPlayerView;

/* Sets the play camera when fn_1_8BC begins scene setup. */
void fn_1_9D4(void) {
    OMOBJ *cameraObject;

    Hu3DCameraCreate(1);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f, 640.0f, 480.0f, 0.0f, 1.0f);
    Hu3DCameraPerspectiveSet(1, 10.0f, 60.0f, 25000.0f, 1.2f);
    cameraObject = omAddObjEx(lbl_1_bss_4, 32730, 0U, 0U, -1, omOutView);
    Center.x = 350.0f;
    Center.y = 250.0f;
    Center.z = 350.0f;
    CRot.x = -36.0f;
    CRot.y = 45.0f;
    CRot.z = 0.0f;
    CZoom = 9100.0f;
}

/* Creates the lighting, shadows, models and initial positions for fn_1_8BC. */
void fn_1_B54(void) {
    M666ShadowPositionTable shadowPositions = { { { -300.0f, 60.0f, -820.0f },
                                                  { 0.0f, 60.0f, -820.0f },
                                                  { 300.0f, 60.0f, -820.0f },
                                                  { 300.0f, 60.0f, 820.0f },
                                                  { 0.0f, 60.0f, 820.0f },
                                                  { -300.0f, 60.0f, 820.0f } } };
    Vec shadowPosition = {0.0f, 10000.0f, -100.0f};
    Vec shadowUpVector = {0.0f, 1.0f, 0.0f};
    Vec shadowTargetPosition = {0.0f, 0.0f, -1000.0f};
    M666ShadowPlane shadowPlane = {0.8f, 0.3f};
    s16 lightId;
    s16 modelId;
    s16 collisionModelId;
    s32 playerIndex;
    s32 modelIndex;

    lightId = Hu3DGLightCreateV(&lbl_1_data_70, &lbl_1_data_7C,
                                &lbl_1_data_88[lbl_1_bss_DF0.night]);
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
    Hu3DShadowCreate(30.0f, 20.0f, 20000.0f);
    Hu3DShadowPosSet(&shadowPosition, &shadowUpVector, &shadowTargetPosition);

    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_90[lbl_1_bss_DF0.night], HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_DF0.shadowModelId = modelId;
    Hu3DModelShadowMapSet(modelId);
    modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        lbl_1_data_98[lbl_1_bss_DF0.night], HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_DF0.modelId = modelId;

    for (modelIndex = 0; modelIndex < 6; modelIndex++) {
        modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_A0[lbl_1_bss_DF0.night], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSetV(modelId, &shadowPositions.positions[modelIndex]);
        if (modelIndex >= 3) {
            Hu3DModelRotSet(modelId, 0.0f, 180.0f,
                            0.0f);
        }
        lbl_1_bss_580[modelIndex].modelId = modelId;
            lbl_1_bss_580[modelIndex].data.obstacleQueue.obstacleCount = 8;
        Hu3DMotionSpeedSet(modelId, 0.0f);
    }

    for (modelIndex = 0; modelIndex < 9; modelIndex++) {
        modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_A8[modelIndex][lbl_1_bss_DF0.night],
            HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSetV(modelId, &lbl_1_data_F0[modelIndex]);
        Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
        lbl_1_bss_DC[modelIndex].modelId = modelId;
        lbl_1_bss_DC[modelIndex].state = 0;
        lbl_1_bss_DC[modelIndex].perimeterIndex = -1;
        lbl_1_bss_DC[modelIndex].soundHandle = -1;
    }

    for (modelIndex = 0; modelIndex < 9; modelIndex++) {
        lbl_1_bss_E14[modelIndex] = 180.0 *
            (atan2((f64)lbl_1_data_728[modelIndex].z,
                   (f64)lbl_1_data_728[modelIndex].x) / 3.141592653589793);
    }

    modelId = collisionModelId = Hu3DModelCreate(HuDataSelHeapReadNum(
        DATANUM(DATA_m666, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelAttrSet(modelId, 1);
    MgActorColMapInit(&collisionModelId, 1, 40);
}

typedef void (*VoidFunc)(void);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

void fn_1_A0(void);

extern MGSEQ_PARAM lbl_1_data_0;

extern void HuMCThresholdSet(s32 threshold);

extern void fn_1_8BC(void);

extern char lbl_1_data_28[20];

extern s16 lbl_1_bss_0;

void fn_1_4AC(void);

void fn_1_3C9C(void);

s32 fn_1_D7DC(void);

typedef struct M666MicrophoneResponse M666MicrophoneResponse;

extern void fn_1_324(M666MicrophoneResponse *response);

extern void fn_1_92C(void);

typedef struct M666RecognizedWordIndex {
    s16 firstWordIndex;
} M666RecognizedWordIndex;

struct M666MicrophoneResponse {
    s16 status;
    u16 recognitionScore;
    s16 wordCount;
    M666RecognizedWordIndex *recognizedWords;
    s32 word;
};

extern const char lbl_1_data_3C[];

typedef struct {
    s16 effectIds[14];
} M666EffectTable;

void fn_1_9ED8(void);

void fn_1_B30C(void);

void fn_1_7CA0(void);

void fn_1_7D24(void);

typedef struct {
    s16 modelId;
    s16 active;
    f32 scale;
    f32 alpha;
} M666SequenceModelView;

/* Called by fn_1_78C for the microphone-player victory; sets its winner and awards 10 coins
 * outside practice mode. */
void fn_1_F8C(void) {
    s32 playerIndex;

    MgSeqWinnerSet(lbl_1_bss_98.charNo, -1, -1, -1);
    playerIndex = lbl_1_bss_98.playerIndex;
    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
        GwPlayer[playerIndex].mgCoinBonus = 10;
    }
}

/* Applies the opening-character motion selected by motionIndex with the caller's blend
 * and table flags. */
void fn_1_1004(s16 motionIndex, f32 blend) {
    CharMotionShiftSet(
        lbl_1_bss_98.charNo,
        lbl_1_bss_98.motionIds[motionIndex],
        0.0f, blend,
        lbl_1_data_680[motionIndex].flags);
}

/* Turns the opening character by 5 degrees per call, switching motions at -135 and 45 degrees;
 * returns 1 when the rotation reaches 45 degrees. */
s16 fn_1_1080(void) {
    Point3d modelRotation;

    Hu3DModelRotGet(lbl_1_bss_98.modelId, &modelRotation);
    if (-135.0f == modelRotation.y) {
        CharMotionShiftSet(lbl_1_bss_98.charNo, lbl_1_bss_98.motionIds[1], 0.0f, 8.0f,
                           lbl_1_data_680[1].flags);
    }
    modelRotation.y += 5.0f;
    Hu3DModelRotSetV(lbl_1_bss_98.modelId, &modelRotation);
    if (45.0f == modelRotation.y) {
        CharMotionShiftSet(lbl_1_bss_98.charNo, lbl_1_bss_98.motionIds[0], 0.0f, 8.0f,
                           lbl_1_data_680[0].flags);
        return 1;
    }
    return 0;
}

/* Rumbles the selected player when microphone probes on both channels return nonzero. */
void fn_1_1198(s16 vibrationMode) {
    if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
        switch (vibrationMode) {
        case 0:
            omVibrate(lbl_1_bss_98.playerIndex, 20, 4, 4);
            break;
        case 1:
            omVibrate(lbl_1_bss_98.playerIndex, 20, 7, 3);
            break;
        case 2:
            omVibrate(lbl_1_bss_98.playerIndex, 20, 20, 0);
            break;
        }
    }
}

/* Stores the selected player's setup and creates its opening model and six motions. */
void fn_1_125C(s16 playerIndex) {
    s16 charNo;
    s16 modelId;
    s16 motionId;
    s32 motionIndex;

    lbl_1_bss_98.playerIndex = playerIndex;
    lbl_1_bss_98.padNo = GwPlayerConf[playerIndex].padNo;
    charNo = GwPlayerConf[playerIndex].charNo;
    lbl_1_bss_98.charNo = charNo;
    if (GwPlayerConf[playerIndex].type == 0) {
        lbl_1_bss_98.difficulty = -1;
    } else {
        lbl_1_bss_98.difficulty = GwPlayerConf[playerIndex].comDif;
    }

    modelId = CharModelCreate(charNo, 4);
    Hu3DModelPosSet(modelId, 960.0f, 0.0f, 460.0f);
    Hu3DModelRotSet(modelId, 0.0f, -135.0f, 0.0f);
    Hu3DModelShadowSet(modelId);
    lbl_1_bss_98.modelId = modelId;
    for (motionIndex = 0; motionIndex < 6; motionIndex++) {
        motionId = CharMotionCreate(charNo, lbl_1_data_680[motionIndex].fileId);
        lbl_1_bss_98.motionIds[motionIndex] = motionId;
    }
    CharMotionShiftSet(lbl_1_bss_98.charNo, lbl_1_bss_98.motionIds[0], 0.0f,
                        0.0f, lbl_1_data_680[0].flags);
}

/* Adjusts active player yaw during fn_1_78C's pre-winner update after gameplay. */
void fn_1_143C(void) {
    s32 playerIndex;
    f32 rotation;
    MGPLAYER *player;
    MGACTOR *actor;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            player = lbl_1_bss_8[playerIndex].player;
            actor = player->actor;
            MgActorRotYGet(actor, &rotation);
            while (rotation > 180.0f) {
                rotation -= 360.0f;
            }
            while (rotation <= -180.0f) {
                rotation += 360.0f;
            }
            if (rotation < 45.0f && rotation > -135.0f) {
                rotation += 5.0f;
                if (rotation > 45.0f) {
                    rotation = 45.0f;
                }
            } else {
                /* Rotations at or below -135 jump directly to 45 after this decrement. */
                rotation -= 5.0f;
                if (rotation < 45.0f) {
                    rotation = 45.0f;
                }
            }
            MgActorRotYSet(actor, rotation);
        }
    }
}

/* Restarts the three player motions when fn_1_92C clears the scene state. */
void fn_1_15F0(void) {
    MGPLAYER *player;
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 3) {
    player = lbl_1_bss_8[playerIndex].player;
        MgPlayerAttrSet(player, 1U);
        CharMotionShiftSet(player->charNo, *player->omObj->mtnId, 0.0f, 8.0f, 0U);
        playerIndex += 1;
    }
}

/* fn_1_78C calls this at the result phase to set all three winners, select their motions,
 * and award 10 coins outside practice mode. */
void fn_1_1680(void) {
    s32 playerIndex;
    s32 playerNumber;
    MGPLAYER *player;

    MgSeqWinnerSet(lbl_1_bss_8[0].charNo, lbl_1_bss_8[1].charNo,
                   lbl_1_bss_8[2].charNo, -1);
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        player = lbl_1_bss_8[playerIndex].player;
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[5],
                               0.0f, 8.0f, 0);
        } else {
            CharMotionShiftSet(player->charNo, player->omObj->mtnId[0],
                               0.0f, 8.0f, 0);
        }
        playerNumber = lbl_1_bss_8[playerIndex].playerNumber;
        if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
            GwPlayer[playerNumber].mgCoinBonus = 10;
        }
    }
}

/* Applies one motion blend to all three players for sequence-phase callbacks. */
void fn_1_17C8(s16 motionIndex, f32 blend, u32 flags) {
    s32 playerIndex;
    MGPLAYER *player;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        player = lbl_1_bss_8[playerIndex].player;
        CharMotionShiftSet(player->charNo, player->omObj->mtnId[motionIndex],
                           0.0f, blend, flags);
    }
}

typedef COL_NARROW_PARAM M666CollisionEvent;

extern void fn_1_4378(M666CollisionEvent *playerParam, M666CollisionEvent *objectParam);

/* Applies a selected rumble pattern to active players when an event requests feedback. */
void fn_1_1858(s16 vibrationMode) {
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            switch (vibrationMode) {
            case 0:
                omVibrate(lbl_1_bss_8[playerIndex].playerNumber,
                          20, 4, 4);
                break;
            case 1:
                omVibrate(lbl_1_bss_8[playerIndex].playerNumber,
                          20, 7, 3);
                break;
            case 2:
                omVibrate(lbl_1_bss_8[playerIndex].playerNumber,
                          20, 20, 0);
                break;
            }
        }
    }
}

/* Creates the three minigame player actors and models after fn_1_4A40 selects the opening
 * player. */
void fn_1_194C(s16 excludedPlayer) {
    MGACTOR_PARAM actorParam;
    s32 playerIndex;
    s32 playerCount;
    s16 charNo;
    s16 modelId;

    actorParam.height = 150.0f;
    actorParam.radius = 40.0f;
    actorParam.type = 0;
    actorParam.attr = 0;
    actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_4378;
    actorParam.correctHookParam = 0;
    actorParam.correctHook = NULL;

    for (playerIndex = 0, playerCount = 0; playerIndex < 4; playerIndex++) {
        if (playerIndex == excludedPlayer) {
            continue;
        }

        lbl_1_bss_8[playerCount].unknownFlag10 = 0;
        lbl_1_bss_8[playerCount].unknownFlag08 = 0;
        lbl_1_bss_8[playerCount].playerNumber = playerIndex;
        actorParam.param = playerCount;
        charNo = GwPlayerConf[playerIndex].charNo;
        lbl_1_bss_8[playerCount].charNo = charNo;
        lbl_1_bss_8[playerCount].padNumber = GwPlayerConf[playerIndex].padNo;
        if (GwPlayerConf[playerIndex].type == 1) {
            lbl_1_bss_8[playerCount].comDifficulty = GwPlayerConf[playerIndex].comDif;
        } else {
            lbl_1_bss_8[playerCount].comDifficulty = -1;
        }

        lbl_1_bss_8[playerCount].player = MgPlayerCreate(
            playerIndex, &actorParam, 4, 1, 1, lbl_1_data_6B0);
        lbl_1_bss_8[playerCount].player->actor->pos.x = lbl_1_data_15C[playerCount].x;
        lbl_1_bss_8[playerCount].player->actor->pos.y = lbl_1_data_15C[playerCount].y;
        lbl_1_bss_8[playerCount].player->actor->pos.z = lbl_1_data_15C[playerCount].z;
        lbl_1_bss_8[playerCount].player->actor->rotY = lbl_1_data_180[playerCount];
        Hu3DModelShadowSet((s16)lbl_1_bss_8[playerCount].player->actor->mdlId);

        modelId = Hu3DModelCreate(HuDataSelHeapReadNum(
            lbl_1_data_6F0[charNo], HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(modelId, 1);
        Hu3DModelScaleSet(modelId, 1.0f, 0.0f,
                          1.0f);
        lbl_1_bss_8[playerCount].modelId = modelId;
        lbl_1_bss_8[playerCount].targetAngle = -500.0f;
        playerCount++;
    }
}

extern void fn_1_40F8(s16 playerIndex, s16 action);

/* Checks one active player's zone, moving lanes, and obstacles during fn_1_3C9C's gameplay
 * update. */
void fn_1_1CBC(s16 playerIndex) {
    Point3d playerPosition;
    Point3d objectPosition;
    Vec separation;
    Vec angleDirection;
    Vec secondDirection;
    M666ActorWork *movingPlatform;
    MGPLAYER *player;
    MGACTOR *playerActor;
    M666SequenceItemView *obstacleSequence;
    f32 heading;
    f32 xDistance;
    f32 zDistance;
    s32 xZone;
    s32 zZone;
    s32 groupIndex;
    s32 itemIndex;

    if (MgSeqModeGet() == 5) {
        player = lbl_1_bss_8[playerIndex].player;
        playerActor = player->actor;
        MgActorPosGet(playerActor, &playerPosition);
        if (playerPosition.x <= -150.0f) {
            xZone = 0;
        } else if (playerPosition.x >= 150.0f) {
            xZone = 2;
        } else {
            xZone = 1;
        }
        if (playerPosition.z <= -150.0f) {
            zZone = 0;
        } else if (playerPosition.z >= 150.0f) {
            zZone = 2;
        } else {
            zZone = 1;
        }
        lbl_1_bss_8[playerIndex].zoneIndex =
            xZone + (zZone * 3);

        for (groupIndex = 0; groupIndex < 3; groupIndex++) {
            for (itemIndex = 0; itemIndex < 3; itemIndex++) {
                if (lbl_1_bss_148[groupIndex].items[itemIndex].state == 4) {
                    obstacleSequence = &lbl_1_bss_148[groupIndex].items[itemIndex];
                    Hu3DModelPosGet(obstacleSequence->rootModelId, &objectPosition);
                    objectPosition.y = 0.0f;
                    if (groupIndex == 0) {
                        objectPosition.x += 100.0f;
                    } else {
                        objectPosition.x -= 100.0f;
                    }
                    if (sqrtf(PSVECSquareDistance(&objectPosition,
                                                  &playerPosition)) <
                        140.0f) {
                        fn_1_40F8(playerIndex, 4);
                        separation.x = playerPosition.x - objectPosition.x;
                        separation.y = 0.0f;
                        separation.z = playerPosition.z - objectPosition.z;
                        if (groupIndex == 0) {
                            heading = 180.0f;
                        } else {
                            heading = 0.0f;
                        }
                        MgActorRotYSet(playerActor, heading);
                    }
                }
            }
        }

        for (groupIndex = 0; groupIndex < 3; groupIndex++) {
            movingPlatform = &lbl_1_bss_E3C[groupIndex];
            Hu3DModelPosGet(movingPlatform->modelIds[0], &objectPosition);
            if (objectPosition.y >= 150.0f) {
                continue;
            }
            switch (movingPlatform->state) {
            case 1:
                if ((playerPosition.x + 40.0f > objectPosition.x - 100.0f) &&
                    (playerPosition.x - 40.0f < objectPosition.x + 100.0f) &&
                    (playerPosition.z + 40.0f > objectPosition.z - 100.0f) &&
                    (playerPosition.z - 40.0f < objectPosition.z + 100.0f)) {
                    fn_1_40F8(playerIndex, 1);
                    movingPlatform->motionState = 1;
                }
                break;
            }
        }

        for (groupIndex = 0; groupIndex < 6; groupIndex++) {
            for (itemIndex = 0; itemIndex < 8; itemIndex++) {
                if (lbl_1_bss_604[groupIndex][itemIndex].state == 6) {
                    Hu3DModelPosGet(lbl_1_bss_604[groupIndex][itemIndex].secondaryModelId,
                                    &objectPosition);
                    objectPosition.y = 0.0f;
                    if (sqrtf(PSVECSquareDistance(&objectPosition,
                                                  &playerPosition)) <
                        120.0f) {
                        fn_1_40F8(playerIndex, 3);
                        separation.x = playerPosition.x - objectPosition.x;
                        separation.y = 0.0f;
                        separation.z = playerPosition.z - objectPosition.z;
                        heading = (f32)(180.0 *
                                  (atan2((f64) separation.z,
                                         (f64) separation.x) /
                                   3.141592653589793));
                        MgActorRotYSet(
                            lbl_1_bss_8[playerIndex].player->actor,
                            360.0f - heading);
                        lbl_1_bss_8[playerIndex].pushAngle =
                            heading;
                    }
                }
            }
        }
    }
}

u16 lbl_1_data_7DC[11] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 100};

char lbl_1_data_7F2[13] = "itemhook_bom";
char lbl_1_data_7FF[17] = "near heiho:%d\n";

const M666FourChoiceTable lbl_1_rodata_148 = {{45, 315, 225, 135}};

/* Stores a randomized target angle and input timer when an avoidance direction is chosen. */
void fn_1_2388(s16 playerIndex, s16 choiceIndex) {
    M666FourChoiceTable avoidanceAngles;
    M666PlayerRecord *playerState;

    playerState = &lbl_1_bss_8[playerIndex];
    avoidanceAngles = lbl_1_rodata_148;
    playerState->targetAngle =
        (f32)(((s16 *)avoidanceAngles.values)[choiceIndex] + 10 - frandmod(20));
    playerState->nudgeTimer = frandmod(15) + 15;
    playerState->nudgeDirection = choiceIndex;
}

typedef struct {
    f32 values[4];
} M666AvoidanceRangeTable;

/* Sets the selected player's temporary avoidance angle and input timer. */
static inline void m666SetAvoidanceInput(s16 playerIndex, s16 direction) {
    M666FourChoiceTable avoidanceAngles;
    M666PlayerRecord *player;

    player = &lbl_1_bss_8[playerIndex];
    avoidanceAngles = lbl_1_rodata_148;
    player->targetAngle =
        (f32)(((s16 *)avoidanceAngles.values)[direction] + 10 - frandmod(20));
    player->nudgeTimer = frandmod(15) + 15;
    player->nudgeDirection = direction;
}

    /* Per-difficulty probabilities for enabling the additional CPU steering-angle offset. */
const M666FourChoiceTable lbl_1_rodata_158 = {{90, 50, 10, 0}};
const M666AvoidanceRangeTable lbl_1_rodata_160 = {{250.0f, 300.0f, 350.0f, 400.0f}};

/* Chooses CPU steering around nearby obstacles and lanes; fn_1_3C9C calls it for active players. */
void fn_1_2458(s16 playerIndex) {
    Point3d objectPosition;
    Point3d playerPosition;
    Point3d objectDelta;
    Point3d nearestObjectPosition;
    f32 closestDistance;
    f32 distance;
    f32 desiredAngle;
    f32 currentAngle;
    s32 inputX;
    s32 inputZ;
    s32 groupIndex;
    s32 obstacleIndex;
    s32 closestGroup;
    s32 closestObstacle;
    s16 nearbyObstacleFound;
    s16 fallingPointOccupied;
    s16 occupiedCell;
    s16 outsideSteeringRadius;
    M666PlayerRecord *player = &lbl_1_bss_8[playerIndex];
    M666AvoidanceRangeTable avoidanceRanges;
    s16 directions[4];
    M666FourChoiceTable directionThresholds;
    M666PlayerRecord *currentPlayer;
    MGPLAYER *gamePlayer;
    MGACTOR *playerActor;

    directionThresholds = lbl_1_rodata_158;
    avoidanceRanges = lbl_1_rodata_160;
    gamePlayer = lbl_1_bss_8[playerIndex].player;
    playerActor = gamePlayer->actor;
    if (player->comDifficulty == -1) {
        return;
    }

    desiredAngle = -500.0f;
    outsideSteeringRadius = 0;
    nearbyObstacleFound = occupiedCell = fallingPointOccupied = 0;
    if (player->zoneIndex - 3 < 0) {
        directions[0] = -1;
    } else if (lbl_1_bss_EA0[player->zoneIndex - 3] != 0) {
        directions[0] = 1;
    } else {
        directions[0] = 0;
    }
    if (player->zoneIndex % 3 < 1) {
        directions[1] = -1;
    } else if (lbl_1_bss_EA0[player->zoneIndex - 1] != 0) {
        directions[1] = 1;
    } else {
        directions[1] = 0;
    }
    if (player->zoneIndex + 3 >= 9) {
        directions[2] = -1;
    } else if (lbl_1_bss_EA0[player->zoneIndex + 3] != 0) {
        directions[2] = 1;
    } else {
        directions[2] = 0;
    }
    if (player->zoneIndex % 3 > 1) {
        directions[3] = -1;
    } else if (lbl_1_bss_EA0[player->zoneIndex + 1] != 0) {
        directions[3] = 1;
    } else {
        directions[3] = 0;
    }
    if (lbl_1_bss_EA0[player->zoneIndex] != 0) {
        occupiedCell = 1;
    }

    MgActorPosGet(player->player->actor, &playerPosition);
    if (sqrtf((playerPosition.x * playerPosition.x) +
              (playerPosition.z * playerPosition.z)) > 510.0f) {
        outsideSteeringRadius = 1;
        player->unknownFlag80 = 1;
    } else {
        player->unknownFlag80 = 0;
    }

    closestDistance = 1000.0f;
    closestGroup = closestObstacle = -1;
    for (groupIndex = 0; groupIndex < 6; groupIndex++) {
        for (obstacleIndex = 0; obstacleIndex < 8; obstacleIndex++) {
            if (lbl_1_bss_604[groupIndex][obstacleIndex].state == 6) {
                Hu3DModelPosGet(lbl_1_bss_604[groupIndex][obstacleIndex].secondaryModelId,
                                &objectPosition);
                goto check_object_distance;
            }
            if (lbl_1_bss_604[groupIndex][obstacleIndex].actor) {
                MgActorPosGet(lbl_1_bss_604[groupIndex][obstacleIndex].actor,
                              &objectPosition);
                goto check_object_distance;
            }
            if (lbl_1_bss_604[groupIndex][obstacleIndex].state == 3) {
                Hu3DModelPosGet(lbl_1_bss_604[groupIndex][obstacleIndex].modelId,
                                &objectPosition);
                goto check_object_distance;
            }
            if (lbl_1_bss_604[groupIndex][obstacleIndex].state == 2) {
                Hu3DModelObjPosGet(lbl_1_bss_580[groupIndex].modelId,
                                   lbl_1_data_7F2, &objectPosition);
check_object_distance:
                distance = sqrtf(PSVECSquareDistance(&objectPosition, &playerPosition));
            if (distance >= avoidanceRanges.values[player->comDifficulty]) {
                continue;
            }
            if (distance < closestDistance) {
                closestDistance = distance;
                nearestObjectPosition.x = objectPosition.x;
                nearestObjectPosition.y = objectPosition.y;
                nearestObjectPosition.z = objectPosition.z;
                closestGroup = groupIndex;
                closestObstacle = obstacleIndex;
                }
            nearbyObstacleFound = 1;
            }
        }
    }

    if (lbl_1_bss_E38 != 0) {
        for (groupIndex = 0; groupIndex < 3; groupIndex++) {
            if (lbl_1_bss_E3C[groupIndex].state != 0) {
                if (player->zoneIndex == lbl_1_bss_E3C[groupIndex].pointIndex) {
                    fallingPointOccupied = 1;
                }
                if (player->zoneIndex - 3 == lbl_1_bss_E3C[groupIndex].pointIndex) {
                    directions[0] = 1;
                }
                if (player->zoneIndex - 1 == lbl_1_bss_E3C[groupIndex].pointIndex) {
                    directions[1] = 1;
                }
                if (player->zoneIndex + 3 == lbl_1_bss_E3C[groupIndex].pointIndex) {
                    directions[2] = 1;
                }
                if (player->zoneIndex + 1 == lbl_1_bss_E3C[groupIndex].pointIndex) {
                    directions[3] = 1;
                }
            }
        }
    }

    if (closestGroup != -1 && closestObstacle != -1) {
        objectDelta.x = playerPosition.x - nearestObjectPosition.x;
        objectDelta.y = 0.0f;
        objectDelta.z = playerPosition.z - nearestObjectPosition.z;
            desiredAngle = (f32)(180.0 +
            ((180.0 * (atan2((f64)objectDelta.z, (f64)objectDelta.x) /
                       3.141592653589793)) - 45.0));
        while (desiredAngle < 0.0f) {
            desiredAngle += 360.0f;
        }
        while (desiredAngle >= 360.0f) {
            desiredAngle -= 360.0f;
        }
    }

    /* These tests retain the last scanned distance rather than closestDistance. The state lookup
     * below also uses obstacleIndex after the scan has advanced it to 8. */
    if (fallingPointOccupied != 0 || occupiedCell != 0) {
        if (closestGroup != -1 && closestObstacle != -1 &&
            distance < 140.0f &&
             (lbl_1_bss_604[closestGroup][closestObstacle].state == 6 ||
              (lbl_1_bss_604[closestGroup][closestObstacle].state >= 4 &&
               lbl_1_bss_604[closestGroup][closestObstacle].timer > 120))) {
            if (player->targetAngle == -500.0f ||
                (lbl_1_bss_604[closestGroup][obstacleIndex].state == 6 &&
                 distance < 190.0f)) {
                if (frandmod(100) < ((s16 *)directionThresholds.values)[player->comDifficulty]) {
                    player->addSteeringOffset = 1;
                }
                player->targetAngle = desiredAngle;
                player->nudgeTimer = 40;
            } else if (outsideSteeringRadius != 0) {
                player->targetAngle += 5.0f;
            }
        } else if (directions[0] == 0) {
            m666SetAvoidanceInput(playerIndex, 0);
        } else if (directions[1] == 0) {
            m666SetAvoidanceInput(playerIndex, 1);
        } else if (directions[2] == 0) {
            m666SetAvoidanceInput(playerIndex, 2);
        } else if (directions[3] == 0) {
            m666SetAvoidanceInput(playerIndex, 3);
        } else if (directions[0] != -1) {
            m666SetAvoidanceInput(playerIndex, 0);
        } else if (directions[1] != -1) {
            m666SetAvoidanceInput(playerIndex, 1);
        } else if (directions[2] != -1) {
            m666SetAvoidanceInput(playerIndex, 2);
        } else if (directions[3] != -1) {
            m666SetAvoidanceInput(playerIndex, 3);
        }
    } else if (nearbyObstacleFound != 0) {
        if (player->targetAngle == -500.0f) {
            if (frandmod(100) < ((s16 *)directionThresholds.values)[player->comDifficulty]) {
                player->addSteeringOffset = 1;
            }
            player->targetAngle = desiredAngle;
            player->nudgeTimer = 40;
        } else if (outsideSteeringRadius != 0) {
            while (player->targetAngle < 0.0f) {
                player->targetAngle += 360.0f;
            }
            while (player->targetAngle >= 360.0f) {
                player->targetAngle -= 360.0f;
            }
            player->targetAngle += 5.0f;
        }
    } else {
        player->suppressAngleSmoothing = 0;
        if (player->nudgeTimer-- <= 0) {
            player->nudgeTimer = 0;
            player->targetAngle = -500.0f;
            player->addSteeringOffset = 0;
        }
    }

    if (closestGroup != -1 && closestObstacle != -1 &&
        lbl_1_bss_604[closestGroup][closestObstacle].state >= 5 &&
        distance < 240.0f &&
        outsideSteeringRadius == 0) {
        player->targetAngle = desiredAngle;
    }

    /* With no scanned obstacle, distance and nearestObjectPosition have no assigned values;
     * the wall-avoidance branches below can still read them. */
    if ((playerActor->colGroundAttr & 0x80) != 0 &&
        playerActor->colNorm.y == 0.0f && player->targetAngle != -500.0f) {
        player->unknownFlag08 = 1;
        player->suppressAngleSmoothing = 1;
        if (outsideSteeringRadius == 0) {
            if (playerActor->colNorm.x > 0.0f) {
                 if (distance > 250.0f &&
                    player->targetAngle > 90.0f && player->targetAngle < 180.0f) {
                    if (playerPosition.z > 0.0f) {
                        player->targetAngle = 50.0f;
                    } else {
                        player->targetAngle = 220.0f;
                    }
                } else if (playerPosition.z <= nearestObjectPosition.z) {
                    player->targetAngle += 10.0f;
                } else {
                    player->targetAngle -= 10.0f;
                }
            } else if (playerActor->colNorm.x < 0.0f) {
                 if (distance > 250.0f && player->targetAngle > 270.0f) {
                    if (playerPosition.z > 0.0f) {
                        player->targetAngle = 40.0f;
                    } else {
                        player->targetAngle = 230.0f;
                    }
                } else if (playerPosition.z <= nearestObjectPosition.z) {
                    player->targetAngle -= 10.0f;
                } else {
                    player->targetAngle += 10.0f;
                }
            } else if (playerActor->colNorm.z > 0.0f) {
                 if (distance > 250.0f && player->targetAngle < 90.0f) {
                    if (playerPosition.x > 0.0f) {
                        player->targetAngle = 310.0f;
                    } else {
                        player->targetAngle = 140.0f;
                    }
                } else if (playerPosition.x <= nearestObjectPosition.x) {
                    player->targetAngle -= 10.0f;
                } else {
                    player->targetAngle += 10.0f;
                }
            } else if (playerActor->colNorm.z < 0.0f) {
                 if (distance > 250.0f &&
                    player->targetAngle > 180.0f && player->targetAngle < 270.0f) {
                    if (playerPosition.x > 0.0f) {
                        player->targetAngle = 320.0f;
                    } else {
                        player->targetAngle = 130.0f;
                    }
                } else if (playerPosition.x <= nearestObjectPosition.x) {
                    player->targetAngle += 10.0f;
                } else {
                    player->targetAngle -= 10.0f;
                }
            }
        }
    }

    if (player->targetAngle != -500.0f) {
        while (player->targetAngle >= 360.0f) {
            player->targetAngle -= 360.0f;
        }
        while (player->targetAngle < 0.0f) {
            player->targetAngle += 360.0f;
        }
        if (desiredAngle != -500.0f && player->suppressAngleSmoothing == 0) {
            currentAngle = desiredAngle - (player->targetAngle - 45.0f);
            while (currentAngle >= 180.0f) {
                currentAngle -= 360.0f;
            }
            while (currentAngle < -180.0f) {
                currentAngle += 360.0f;
            }
            player->targetAngle += currentAngle / 5.0f;
        }
        currentAngle = player->targetAngle;
        if (player->addSteeringOffset != 0) {
            currentAngle += 60.0f;
        }
        MgPlayerPadSet(player->player,
            (s32)((f64)lbl_1_data_198[player->comDifficulty] *
                  sin((3.141592653589793 * (f64)currentAngle) / 180.0)),
            (s32)((f64)lbl_1_data_198[player->comDifficulty] *
                  cos((3.141592653589793 * (f64)currentAngle) / 180.0)), 0, 0);
    } else if (player->wanderFrames != 0) {
        MgPlayerPadSet(player->player,
            (s32)((f64)lbl_1_data_198[player->comDifficulty] *
                  sin((3.141592653589793 * (f64)player->wanderAngle) / 180.0)),
            (s32)((f64)lbl_1_data_198[player->comDifficulty] *
                  cos((3.141592653589793 * (f64)player->wanderAngle) / 180.0)), 0, 0);
        player->wanderFrames -= 1;
    } else if (frandmod(1000) < 30) {
        player->wanderFrames = frandmod(15) + 10;
        player->wanderAngle = frandmod(360);
    }
    player->unknownFlag10 = 0;
}

void fn_1_1CBC(s16 playerIndex);

void fn_1_2458(s16 playerIndex);

/* Updates all three players' movement and collision state each frame from the sequence
 * callbacks. */
void fn_1_3C9C(void) {
    s32 playerIndex;
    MGPLAYER *actorHolder;
    MGACTOR *actor;
    Vec direction;
    Vec position;
    Vec scale;

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        actorHolder = lbl_1_bss_8[playerIndex].player;
        actor = actorHolder->actor;
        switch (lbl_1_bss_8[playerIndex].motionSelection) {
        case 0:
            fn_1_1CBC(playerIndex);
            fn_1_2458(playerIndex);
            break;
        case 1:
            position.x = position.z = 2000.0f;
            position.y = 0.0f;
            MgActorPosSetRaw(actor, &position);
            Hu3DModelPosSetV((s16)actor->mdlId, &position);
            break;
        case 2:
            Hu3DModelPosGet(lbl_1_bss_8[playerIndex].modelId, &position);
            direction.x = position.x;
            direction.y = 0.0f;
            direction.z = position.z;
            PSVECNormalize(&direction, &direction);
            direction.x *= 10.0f;
            direction.z *= 10.0f;
            direction.y = 5.0f;
            position.x += direction.x;
            position.y = direction.y +
                (8.0 *
                  sin(((f64)(lbl_1_bss_8[playerIndex].displacementPhase++ * 8) *
                      3.141592653589793) / 180.0)) + position.y;
            position.z += direction.z;
            Hu3DModelPosSetV(lbl_1_bss_8[playerIndex].modelId, &position);
            Hu3DModelScaleGet(lbl_1_bss_8[playerIndex].modelId, &scale);
            if (scale.y < 1.0f) {
                scale.y += 0.1f;
            }
            Hu3DModelScaleSetV(lbl_1_bss_8[playerIndex].modelId, &scale);
            break;
        case 3:
        case 4:
        {
            direction.x = 40.0f;
             direction.y = 60.0f - lbl_1_bss_8[playerIndex].verticalPushDecay;
            direction.z = 0.0f;
            MgActorPushSet(actor, &direction);
            Hu3DModelPosGet((s16)actor->mdlId, &position);
            direction.x = (f32)(40.0 *
                cos((lbl_1_bss_8[playerIndex].pushAngle * 3.141592653589793) /
                    180.0));
            direction.z = (f32)(40.0 *
                sin((lbl_1_bss_8[playerIndex].pushAngle * 3.141592653589793) /
                    180.0));
            position.x += direction.x;
            position.y += direction.y;
            position.z += direction.z;
            Hu3DModelPosSetV((s16)actor->mdlId, &position);
            Hu3DModelRotSet((s16)actor->mdlId, 0.0f,
                            lbl_1_bss_8[playerIndex].pushAngle,
                            0.0f);
             lbl_1_bss_8[playerIndex].verticalPushDecay += 2.0f;
            break;
        }
        }
    }
}

/* Removes an active player from play and starts the requested reaction when fn_1_4378 reports
 * contact. */
static inline void setPlayerMotionState(s16 playerIndex, s16 motionState) {
    Point3d position;
    MGACTOR *playerActor;
    MGPLAYER *activePlayer;

    activePlayer = lbl_1_bss_8[playerIndex].player;
    playerActor = activePlayer->actor;
    if ((lbl_1_bss_8[playerIndex].motionSelection == 0) && (MgSeqModeGet() == 5)) {
        omVibrate(lbl_1_bss_8[playerIndex].playerNumber, 20, 20, 0);
        MgPlayerAttrSet(activePlayer, 1U);
        lbl_1_bss_8[playerIndex].motionSelection = motionState;
        MgPlayerDespawn(activePlayer);
        MgActorColAttrSet(playerActor, 2U);
        MgActorGravitySet(playerActor, 0.0f);
        CharMotionShiftSet(activePlayer->charNo, activePlayer->omObj->mtnId[14],
                           0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        if (motionState == 1) {
            Hu3DModelAttrReset(lbl_1_bss_8[playerIndex].modelId, 1U);
            MgActorPosGet(playerActor, &position);
            Hu3DModelPosSetV(lbl_1_bss_8[playerIndex].modelId, &position);
            Hu3DMotionClusterSpeedSet(lbl_1_bss_8[playerIndex].modelId, 0, 2.0f);
            Hu3DModelClusterAttrSet(lbl_1_bss_8[playerIndex].modelId, 0,
                                    M666_CLUSTER_MOTION_CLEAR_MASK);
            Hu3DModelRotSet(lbl_1_bss_8[playerIndex].modelId, 0.0f,
                            (f32) (180.0 * (atan2((f64) position.x, (f64) position.z) /
                                          3.141592653589793)), 0.0f);
            position.x = 2000.0f;
            position.z = 2000.0f;
            MgActorPosSetRaw(playerActor, &position);
            Hu3DModelPosSetV((s16) playerActor->mdlId, &position);
        }
    }
}

/* Starts a player's collision reaction; fn_1_1CBC calls this for a lane, marker, or obstacle
 * hit. */
void fn_1_40F8(s16 playerIndex, s16 action) {
    Point3d actorPosition;
    MGACTOR *playerActor;
    MGPLAYER *player;

        player = lbl_1_bss_8[playerIndex].player;
    playerActor = player->actor;
    if ((lbl_1_bss_8[playerIndex].motionSelection == 0) && (MgSeqModeGet() == 5)) {
        omVibrate(lbl_1_bss_8[playerIndex].playerNumber, 20, 20, 0);
        MgPlayerAttrSet(player, 1U);
        lbl_1_bss_8[playerIndex].motionSelection = action;
        MgPlayerDespawn(player);
        MgActorColAttrSet(playerActor, 2U);
        MgActorGravitySet(playerActor, 0.0f);
        CharMotionShiftSet(player->charNo, player->omObj->mtnId[14], 0.0f, 0.0f, HU3D_MOTATTR_LOOP);
        if (action == 1) {
            Hu3DModelAttrReset(lbl_1_bss_8[playerIndex].modelId, 1U);
            MgActorPosGet(playerActor, &actorPosition);
            Hu3DModelPosSetV(lbl_1_bss_8[playerIndex].modelId, &actorPosition);
            Hu3DMotionClusterSpeedSet(lbl_1_bss_8[playerIndex].modelId, 0, 2.0f);
            Hu3DModelClusterAttrSet(lbl_1_bss_8[playerIndex].modelId, 0,
                                    M666_CLUSTER_MOTION_CLEAR_MASK);
            Hu3DModelRotSet(lbl_1_bss_8[playerIndex].modelId, 0.0f,
                            (f32) (180.0 * (atan2((f64) actorPosition.x, (f64) actorPosition.z) /
                                            3.141592653589793)),
                            0.0f);
            actorPosition.x = 2000.0f;
            actorPosition.z = 2000.0f;
            MgActorPosSetRaw(playerActor, &actorPosition);
            Hu3DModelPosSetV((s16) playerActor->mdlId, &actorPosition);
        }
    }
}

/* Handles player collision callbacks for secondary obstacles, other players, and obstacles,
 * updating reactions and target angles. */
void fn_1_4378(M666CollisionEvent *playerEvent,
               M666CollisionEvent *targetEvent) {
    Point3d playerPosition;
    Point3d targetPosition;
    Point3d separation;
    Point3d angleDirection;
    /* The second target-angle test reads this direction without initializing its components. */
    Point3d uninitializedDirection;
    MGPLAYER *player;
    f32 heading;
    s16 targetPlayerIndex;
    s32 playerIndex;
    s32 actorIndex;

    playerIndex = (s16) playerEvent->paramB;
    player = lbl_1_bss_8[playerIndex].player;
    if (playerEvent->paramB < 4) {
        if ((targetEvent->paramB >= 52) && (targetEvent->paramB < 100)) {
            if (lbl_1_bss_8[(s16) playerIndex].motionSelection == 0) {
                MgActorPosGet(lbl_1_bss_8[(s16) playerIndex].player->actor,
                              &playerPosition);
                actorIndex = targetEvent->paramB - 52;
                MgActorPosGet(
                    lbl_1_bss_604[actorIndex / 8][actorIndex % 8].secondaryActor,
                    &targetPosition);
                setPlayerMotionState((s16) playerIndex, 3);
                separation.x = playerPosition.x - targetPosition.x;
                separation.y = 0.0f;
                separation.z = playerPosition.z - targetPosition.z;
                heading = (f32) (180.0 *
                          (atan2((f64) separation.z, (f64) separation.x) /
                           3.141592653589793));
                MgActorRotYSet(lbl_1_bss_8[(s16) playerIndex].player->actor,
                               360.0f - heading);
                lbl_1_bss_8[(s16) playerIndex].pushAngle = heading;
                goto clearFlags;
            }
        } else {
            if (targetEvent->paramB < 4) {
                targetPlayerIndex = (s16) targetEvent->paramB;
                if (lbl_1_bss_8[(s16) playerIndex].targetAngle != -500.0f) {
                MgActorPosGet(lbl_1_bss_8[(s16) playerIndex].player->actor,
                              &playerPosition);
                MgActorPosGet(lbl_1_bss_8[targetPlayerIndex].player->actor,
                              &targetPosition);
                separation.x = targetPosition.x - playerPosition.x;
                separation.y = 0.0f;
                separation.z = targetPosition.z - playerPosition.z;
                angleDirection.x = (f32) sin(
                    (3.141592653589793 *
                     (f64) (lbl_1_bss_8[(s16) playerIndex].targetAngle -
                            45.0f)) / 180.0);
                angleDirection.y = 0.0f;
                angleDirection.z = (f32) -cos(
                    (3.141592653589793 *
                     (f64) (lbl_1_bss_8[(s16) playerIndex].targetAngle -
                            45.0f)) / 180.0);
                if (PSVECDotProduct(&separation, &angleDirection) > 0.0f) {
                    lbl_1_bss_8[(s16) playerIndex].targetAngle += 5.0f;
                }
                if (PSVECDotProduct(&separation, &uninitializedDirection) > 0.0f) {
                    lbl_1_bss_8[(s16) playerIndex].secondTargetAngle += 5.0f;
                }
                }
            } else if ((targetEvent->paramB >= 4) &&
                       (targetEvent->paramB < 52)) {
                MgActorPosGet(lbl_1_bss_8[(s16) playerIndex].player->actor,
                              &playerPosition);
                actorIndex = targetEvent->paramB - 4;
                MgActorPosGet(
                    lbl_1_bss_604[actorIndex / 8][actorIndex % 8].actor,
                    &targetPosition);
                separation.x = targetPosition.x - playerPosition.x;
                separation.y = 0.0f;
                separation.z = targetPosition.z - playerPosition.z;
                angleDirection.x = (f32) sin(
                    (3.141592653589793 *
                     (f64) (lbl_1_bss_8[(s16) playerIndex].targetAngle -
                            45.0f)) / 180.0);
                angleDirection.y = 0.0f;
                angleDirection.z = (f32) -cos(
                    (3.141592653589793 *
                     (f64) (lbl_1_bss_8[(s16) playerIndex].targetAngle -
                            45.0f)) / 180.0);
                lbl_1_bss_8[(s16) playerIndex].unknownFlag10 = 1;
                if (PSVECDotProduct(&separation, &angleDirection) > 0.0f) {
                    lbl_1_bss_8[(s16) playerIndex].targetAngle += 30.0f;
                }
            }
            goto clearFlags;
        }
    } else {
clearFlags:
        lbl_1_bss_8[(s16) playerIndex].unknownFlag08 = 0;
    }
}

void fn_1_194C(s16 playerIndex);

/* Selects the first group-0 player, falling back to player 0, and creates its opening model and
 * motions. */
void fn_1_4A40(void) {
    s16 playerIndex;
    s32 candidateIndex;
    s16 charNo;
    s16 modelId;
    s16 motionId;
    s32 motionIndex;

    playerIndex = 0;
    for (candidateIndex = 0; candidateIndex < 4; candidateIndex++) {
        if (GwPlayerConf[candidateIndex].grpNo == 0) {
            playerIndex = candidateIndex;
            break;
        }
    }

    lbl_1_bss_98.playerIndex = playerIndex;
    lbl_1_bss_98.padNo = GwPlayerConf[playerIndex].padNo;
    charNo = GwPlayerConf[playerIndex].charNo;
    lbl_1_bss_98.charNo = charNo;
    if (GwPlayerConf[playerIndex].type == 0) {
        lbl_1_bss_98.difficulty = -1;
    } else {
        lbl_1_bss_98.difficulty = GwPlayerConf[playerIndex].comDif;
    }

    modelId = CharModelCreate(charNo, 4);
    Hu3DModelPosSet(modelId, 960.0f, 0.0f, 460.0f);
    Hu3DModelRotSet(modelId, 0.0f, -135.0f, 0.0f);
    Hu3DModelShadowSet(modelId);
    lbl_1_bss_98.modelId = modelId;
    for (motionIndex = 0; motionIndex < 6; motionIndex++) {
        motionId = CharMotionCreate(charNo, lbl_1_data_680[motionIndex].fileId);
        lbl_1_bss_98.motionIds[motionIndex] = motionId;
    }
    CharMotionShiftSet(lbl_1_bss_98.charNo, lbl_1_bss_98.motionIds[0], 0.0f,
                       0.0f, lbl_1_data_680[0].flags);
    fn_1_194C(playerIndex);
}

void fn_1_5580(s16 eventCode);

void fn_1_C630(void);

void fn_1_5654(s16 eventStatus);

typedef struct {
    s16 values[4];
} DelayChoiceTable;

const DelayChoiceTable lbl_1_rodata_1CC = {{180, 120, 60, 30}};

/* Returns a random minigame event code after its difficulty-scaled delay expires. */
u16 fn_1_4C60(void) {
    DelayChoiceTable delayChoices;
    M666Bss98Work *state;

    state = &lbl_1_bss_98;
    delayChoices = lbl_1_rodata_1CC;
    if (state->delayCounter != 0) {
        state->delayCounter -= 1;
        return 0;
    }
    state->delayCounter = (s16)(delayChoices.values[state->difficulty] +
                                frandmod(delayChoices.values[state->difficulty]));
    return lbl_1_data_7DC[frandmod(11)];
}

/* Advances the opening character motion and triggers the selected minigame event. */
void fn_1_4D20(void) {
    DelayChoiceTable delayChoices;
    s16 eventSentinel;
    u16 eventCode;
    u16 selectedCode;
    f32 maximumMotionTime;
    M666Bss98Work *work;
    M666Bss98Work *delayWork;

    work = &lbl_1_bss_98;
    eventCode = 0;
    if (work->motionState == 0) {
        if (lbl_1_bss_E38 == 0) {
            if (work->difficulty == -1) {
                eventCode = lbl_1_bss_2;
            } else {
                delayWork = &lbl_1_bss_98;
                delayChoices = lbl_1_rodata_1CC;
                if (delayWork->delayCounter != 0) {
                    delayWork->delayCounter -= 1;
                    selectedCode = 0;
                } else {
                    delayWork->delayCounter =
                        (s16)(delayChoices.values[delayWork->difficulty] +
                              frandmod(delayChoices.values[delayWork->difficulty]));
                    selectedCode = lbl_1_data_7DC[frandmod(11)];
                }
                eventCode = selectedCode;
            }
        }
    } else {
        maximumMotionTime = CharMotionMaxTimeGet(work->charNo);
        if (CharMotionTimeGet(work->charNo) >= maximumMotionTime) {
            work->motionState = 0;
            CharMotionShiftSet(lbl_1_bss_98.charNo, lbl_1_bss_98.motionIds[0], 0.0f,
                               8.0f, lbl_1_data_680[0].flags);
            fn_1_5654(work->eventStatus);
        }
    }

    eventSentinel = -1;
    if (eventCode != 0) {
        switch (eventCode) {
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
        case 8:
        case 9:
            fn_1_5580((s16)eventCode);
            break;
        case 10:
            fn_1_C630();
            break;
        default:
            fn_1_5580(100);
            break;
        }
        lbl_1_bss_2 = 0;
    }}

typedef M666PerimeterWork M666SetupWorkView;

typedef struct {
    s16 values[3];
} M666WorkTypeTable;

const M666WorkTypeTable lbl_1_rodata_1D4 = {{0, 1, 3}};

typedef struct {
    s32 fileIds[10];
} M666SpriteFileTable;

const M666SpriteFileTable lbl_1_rodata_1DC = { { DATANUM(DATA_m666, 60), DATANUM(DATA_m666, 61),
                                                 DATANUM(DATA_m666, 62), DATANUM(DATA_m666, 63),
                                                 DATANUM(DATA_m666, 64), DATANUM(DATA_m666, 65),
                                                 DATANUM(DATA_m666, 66), DATANUM(DATA_m666, 67),
                                                 DATANUM(DATA_m666, 68), DATANUM(DATA_m666, 69) } };

/* Loads ten sprite animations and creates three perimeter models, each with seven joint motions
 * and a sprite group. */
void fn_1_4F24(void) {
    M666SpriteFileTable spriteFileIds;
    ANIMDATA *spriteAnimations[10];
    M666WorkTypeTable workTypes;
    s32 modelIndex;
    s32 loopIndex;
    s16 modelId;
    s16 spriteGroupId;
    s16 jointMotionId;

    workTypes = lbl_1_rodata_1D4;
    spriteFileIds = lbl_1_rodata_1DC;

    for (modelIndex = 0; modelIndex < 10; modelIndex++) {
        spriteAnimations[modelIndex] = HuSprAnimRead(
            HuDataSelHeapReadNum(spriteFileIds.fileIds[modelIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    }

    for (modelIndex = 0; modelIndex < 3; modelIndex++) {
        modelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 26), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelPosSetV(modelId, &lbl_1_data_1A8[modelIndex]);
        Hu3DModelShadowSet(modelId);
        Hu3DModelRotSet(modelId, 0.0f, lbl_1_data_1CC[modelIndex],
                        0.0f);

        for (loopIndex = 0; loopIndex < 7; loopIndex++) {
            jointMotionId = Hu3DJointMotion(modelId, HuDataSelHeapReadNum(
                lbl_1_data_1D8[loopIndex].fileId, HU_MEMNUM_OVL, HEAP_MODEL));
            lbl_1_bss_D84[modelIndex].jointMotionIds[loopIndex] = jointMotionId;
        }

        Hu3DMotionSet(modelId, lbl_1_bss_D84[modelIndex].jointMotionIds[0]);
        Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
        lbl_1_bss_D84[modelIndex].modelId = modelId;
        lbl_1_bss_D84[modelIndex].direction = workTypes.values[modelIndex];
        lbl_1_bss_D84[modelIndex].rotation = lbl_1_data_1CC[modelIndex];
        lbl_1_bss_D84[modelIndex].targetIndex = -1;
        spriteGroupId = HuSprGrpCreate(10);
        lbl_1_bss_D84[modelIndex].spriteGroupId = spriteGroupId;

        for (loopIndex = 0; loopIndex < 10; loopIndex++) {
            HuSprGrpMemberSet(spriteGroupId, (s16)loopIndex,
                              HuSprCreate(spriteAnimations[loopIndex], 10000, 0));
            HuSprAttrSet(spriteGroupId, (s16)loopIndex, 4);
        }
    }
}

/* Starts the selected perimeter model's base joint motion and clears its motion state. */
void fn_1_51F4(s16 perimeterIndex) {
    M666MotionWork *motionEntry;

    motionEntry = &lbl_1_bss_D84[perimeterIndex];
    Hu3DMotionShiftSet(motionEntry->modelId, motionEntry->jointMotionIds[0],
                       0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    motionEntry->motionState = 0;
}

/* Starts the selected perimeter model's joint motion 4 and marks its target event as active. */
void fn_1_5264(s16 perimeterIndex) {
    M666MotionWork *perimeterWork;

    perimeterWork = &lbl_1_bss_D84[perimeterIndex];
    Hu3DMotionShiftSet(perimeterWork->modelId,
                       perimeterWork->jointMotionIds[4], 0.0f,
                       8.0f, 0);
    perimeterWork->motionState = 3;
    lbl_1_bss_DC[perimeterWork->targetIndex].state = 2;
}

/* Starts the selected perimeter model's joint motion 1 and sets motion state 2. */
void fn_1_52EC(s16 perimeterIndex) {
    M666MotionWork *motionEntry;

    motionEntry = &lbl_1_bss_D84[perimeterIndex];
    Hu3DMotionShiftSet(motionEntry->modelId, motionEntry->jointMotionIds[1],
                       0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    motionEntry->motionState = 2;
}

typedef M666PerimeterWork M666PerimeterTurnWork;

/* Selects an idle perimeter model closest in angular position to the requested target point and
 * records its turn direction. */
s16 fn_1_535C(s16 eventCode) {
    Point3d modelPosition;
    f32 relativeAngle;
    f32 bestFacing;
    s16 angleDirection;
    M666PerimeterTurnWork *perimeterWork;
    s16 bestModelIndex;
    s32 candidateModelIndex;

    if (eventCode == 0) {
        return -1;
    }

    angleDirection = 0;
    bestFacing = -1.0f;
    bestModelIndex = -1;
    for (candidateModelIndex = 0; candidateModelIndex < 3; candidateModelIndex++) {
        perimeterWork = &((M666PerimeterTurnWork *)&lbl_1_bss_D84)[candidateModelIndex];
        if (perimeterWork->motionState != 0) {
            continue;
        }

        Hu3DModelPosGet(((M666PerimeterTurnWork *) &lbl_1_bss_D84)[candidateModelIndex].modelId,
                        &modelPosition);
        relativeAngle = 180.0 *
            (atan2((f64)modelPosition.z, (f64)modelPosition.x) / 3.141592653589793);
        perimeterWork->facingAngle = relativeAngle - 45.0f;
        relativeAngle -= lbl_1_bss_E14[eventCode - 1];
        if (cos((3.141592653589793 * relativeAngle) / 180.0) >= bestFacing) {
            bestFacing = cos((3.141592653589793 * relativeAngle) / 180.0);
            bestModelIndex = candidateModelIndex;
            while (relativeAngle >= 180.0f) {
                relativeAngle -= 360.0f;
            }
            while (relativeAngle < -180.0f) {
                relativeAngle += 360.0f;
            }
            if (relativeAngle >= 0.0f) {
                angleDirection = 1;
            } else {
                angleDirection = 0;
            }
        }
    }

    if (bestModelIndex != -1) {
        ((M666PerimeterTurnWork *)&lbl_1_bss_D84)[bestModelIndex].turnDirection = angleDirection;
    }
    return bestModelIndex;
}

typedef M666PerimeterWork M666PerimeterMotionWork;

s16 fn_1_535C(s16 eventCode);

s16 fn_1_CB8C(s16 slotIndex, s16 perimeterIndex);

/* Starts the opening character's event animation; fn_1_4D20 calls it for event codes 1-9 or
 * fallback. */
void fn_1_5580(s16 eventCode) {
    if (eventCode <= 9) {
        CharMotionShiftSet(lbl_1_bss_98.charNo,
                           lbl_1_bss_98.motionIds[2],
                           0.0f, 8.0f,
                           lbl_1_data_680[2].flags);
    } else {
        CharMotionShiftSet(lbl_1_bss_98.charNo,
                           lbl_1_bss_98.motionIds[3],
                           0.0f, 8.0f,
                           lbl_1_data_680[3].flags);
    }
    lbl_1_bss_98.motionState = 1;
    lbl_1_bss_98.eventStatus = eventCode;
}

/* Resolves the microphone player's completed event animation and assigns its target;
 * fn_1_4D20 calls this after the animation. */
void fn_1_5654(s16 eventCode) {
    s32 perimeterIndex;
    s16 selectedPerimeterIndex;

    if (eventCode <= 9) {
        selectedPerimeterIndex = fn_1_535C(eventCode);
        if (selectedPerimeterIndex != -1) {
            OSReport(lbl_1_data_7FF, selectedPerimeterIndex);
            if (fn_1_CB8C(eventCode - 1, selectedPerimeterIndex) == 0) {
                HuAudFXPlayPan(MSM_SE_M666_TARGET_ASSIGNMENT, (s16)(64.0 -
                    (32.0 * sin((3.141592653589793 *
                     (f64)lbl_1_bss_D84[selectedPerimeterIndex].facingAngle) /
                     180.0))));
                lbl_1_bss_D84[selectedPerimeterIndex].motionState = 1;
                /* Clear the previous target even when targetIndex is still -1. */
                lbl_1_bss_DC[lbl_1_bss_D84[selectedPerimeterIndex].targetIndex].state = 0;
                lbl_1_bss_DC[lbl_1_bss_D84[selectedPerimeterIndex].targetIndex].perimeterIndex = -1;
                lbl_1_bss_D84[selectedPerimeterIndex].targetIndex = eventCode - 1;
                Hu3DMotionShiftSet(lbl_1_bss_D84[selectedPerimeterIndex].modelId,
                                   lbl_1_bss_D84[selectedPerimeterIndex].jointMotionIds[2],
                                   0.0f, 8.0f,
                                   HU3D_MOTATTR_LOOP);
                HuSprAttrReset(lbl_1_bss_D84[selectedPerimeterIndex].spriteGroupId,
                               eventCode - 1, 4);
                lbl_1_bss_DC[eventCode - 1].state = 1;
                lbl_1_bss_DC[eventCode - 1].soundHandle = HuAudFXPlay(MSM_SE_M666_MARKER_ROTATION);
                lbl_1_bss_DC[eventCode - 1].perimeterIndex = selectedPerimeterIndex;
                return;
            }
            for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
                if (lbl_1_bss_D84[perimeterIndex].motionState == 0) {
                    lbl_1_bss_D84[perimeterIndex].motionState = 6;
            lbl_1_bss_D84[perimeterIndex].countdown = 60;
                    HuSprAttrReset(lbl_1_bss_D84[perimeterIndex].spriteGroupId, 9, 4);
                    Hu3DMotionShiftSet(lbl_1_bss_D84[perimeterIndex].modelId,
                                       lbl_1_bss_D84[perimeterIndex].jointMotionIds[6],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    HuAudFXPlay(MSM_SE_M666_EVENT_FALLBACK);
                }
            }
        }
    } else {
        for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
            if (lbl_1_bss_D84[perimeterIndex].motionState == 0) {
                lbl_1_bss_D84[perimeterIndex].motionState = 6;
                lbl_1_bss_D84[perimeterIndex].countdown = 60;
                HuSprAttrReset(lbl_1_bss_D84[perimeterIndex].spriteGroupId, 9, 4);
                Hu3DMotionShiftSet(lbl_1_bss_D84[perimeterIndex].modelId,
                                   lbl_1_bss_D84[perimeterIndex].jointMotionIds[6],
                                   0.0f, 8.0f,
                                   HU3D_MOTATTR_LOOP);
                HuAudFXPlay(MSM_SE_M666_EVENT_FALLBACK);
            }
        }
    }
}

/* Separates overlapping perimeter models; fn_1_77B0 runs this after their movement updates. */
void fn_1_5A78(void) {
    Vec perimeterPosition;
    Vec otherPerimeterPosition;
    s32 perimeterIndex;
    s32 otherPerimeterIndex;
    s16 workType;

    for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
        Hu3DModelPosGet(lbl_1_bss_D84[perimeterIndex].modelId, &perimeterPosition);
        for (otherPerimeterIndex = 0; otherPerimeterIndex < 3; otherPerimeterIndex++) {
            if (perimeterIndex != otherPerimeterIndex) {
                Hu3DModelPosGet(lbl_1_bss_D84[otherPerimeterIndex].modelId,
                                &otherPerimeterPosition);
                if (!(sqrtf(PSVECSquareDistance(&perimeterPosition,
                                               &otherPerimeterPosition)) > 100.0f) &&
                    lbl_1_bss_D84[otherPerimeterIndex].motionState != 3) {
                    switch (lbl_1_bss_D84[otherPerimeterIndex].direction) {
                    case 0:
                        if (otherPerimeterPosition.z > perimeterPosition.z) {
                            otherPerimeterPosition.z += 5.0f;
                        } else if ((otherPerimeterPosition.z == -680.0f) &&
                                   (perimeterPosition.z == -680.0f)) {
                            otherPerimeterPosition.z += 5.0f;
                        } else {
                            otherPerimeterPosition.z -= 5.0f;
                        }
                        if (otherPerimeterPosition.z < -680.0f) {
                            otherPerimeterPosition.z = -680.0f;
                        }
                        if (otherPerimeterPosition.z > -550.0f) {
                            otherPerimeterPosition.z = -550.0f;
                        }
                        break;
                    case 2:
                        if (otherPerimeterPosition.z < perimeterPosition.z) {
                            otherPerimeterPosition.z -= 5.0f;
                        } else if ((otherPerimeterPosition.z == 680.0f) &&
                                   (perimeterPosition.z == 680.0f)) {
                            otherPerimeterPosition.z -= 5.0f;
                        } else {
                            otherPerimeterPosition.z += 5.0f;
                        }
                        if (otherPerimeterPosition.z > 680.0f) {
                            otherPerimeterPosition.z = 680.0f;
                        }
                        if (otherPerimeterPosition.z < 550.0f) {
                            otherPerimeterPosition.z = 550.0f;
                        }
                        break;
                    case 1:
                        if (otherPerimeterPosition.x < perimeterPosition.x) {
                            otherPerimeterPosition.x -= 5.0f;
                        } else if ((otherPerimeterPosition.x == 680.0f) &&
                                   (perimeterPosition.x == 680.0f)) {
                            otherPerimeterPosition.x -= 5.0f;
                        } else {
                            otherPerimeterPosition.x += 5.0f;
                        }
                        if (otherPerimeterPosition.x > 680.0f) {
                            otherPerimeterPosition.x = 680.0f;
                        }
                        if (otherPerimeterPosition.x < 550.0f) {
                            otherPerimeterPosition.x = 550.0f;
                        }
                        break;
                    case 3:
                        if (otherPerimeterPosition.x > perimeterPosition.x) {
                            otherPerimeterPosition.x += 5.0f;
                        } else if ((otherPerimeterPosition.x == -680.0f) &&
                                   (perimeterPosition.x == -680.0f)) {
                            otherPerimeterPosition.x += 5.0f;
                        } else {
                            otherPerimeterPosition.x -= 5.0f;
                        }
                        if (otherPerimeterPosition.x < -680.0f) {
                            otherPerimeterPosition.x = -680.0f;
                        }
                        if (otherPerimeterPosition.x > -550.0f) {
                            otherPerimeterPosition.x = -550.0f;
                        }
                        break;
                    }
                    Hu3DModelPosSetV(lbl_1_bss_D84[otherPerimeterIndex].modelId,
                                     &otherPerimeterPosition);
                }
            }
        }
        Hu3DModelPosSetV(lbl_1_bss_D84[perimeterIndex].modelId, &perimeterPosition);
    }
}

typedef M666PerimeterWork M666PerimeterMotionState;

/* Moves a perimeter model toward the selected point while turning it along the route. */
void fn_1_6030(s32 perimeterIndex) {
    M666PerimeterMotionState *perimeter;
    Vec position;
    s16 targetDirection;
    M666PerimeterMotionState *positiveXArrival;
    M666PerimeterMotionState *positiveZArrival;
    M666PerimeterMotionState *negativeXArrival;
    M666PerimeterMotionState *negativeZArrival;
    M666PerimeterMotionState *reverseNegativeXArrival;
    M666PerimeterMotionState *reverseNegativeZArrival;
    M666PerimeterMotionState *reversePositiveXArrival;
    M666PerimeterMotionState *reversePositiveZArrival;

    perimeter = &((M666PerimeterMotionState *)lbl_1_bss_D84)[perimeterIndex];
    targetDirection = lbl_1_data_728[perimeter->targetIndex].direction;
    Hu3DModelPosGet(perimeter->modelId, &position);

    if (perimeter->turnDirection == 0) {
        switch (perimeter->direction) {
        case 0:
            if (90.0f == perimeter->rotation) {
                position.x += 5.0f;
            } else if (perimeter->rotation <= 90.0f &&
                       perimeter->rotation >= -90.0f) {
                perimeter->rotation += 5.0f;
            } else {
                perimeter->rotation -= 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.x >= lbl_1_data_728[perimeter->targetIndex].position.x) {
                    position.x = lbl_1_data_728[perimeter->targetIndex].position.x;
                    positiveXArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(positiveXArrival->modelId,
                                       positiveXArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    positiveXArrival->motionState = 2;
                }
            } else if (position.x >= 600.0f) {
                position.x = 600.0f;
                perimeter->direction = 1;
            }
            break;
        case 1:
            if (0.0f == perimeter->rotation) {
                position.z += 5.0f;
            } else if (perimeter->rotation >= 0.0f &&
                       perimeter->rotation <= 180.0f) {
                perimeter->rotation -= 5.0f;
            } else {
                perimeter->rotation += 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.z >= lbl_1_data_728[perimeter->targetIndex].position.z) {
                    position.z = lbl_1_data_728[perimeter->targetIndex].position.z;
                    positiveZArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(positiveZArrival->modelId,
                                       positiveZArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    positiveZArrival->motionState = 2;
                }
            } else if (position.z >= 600.0f) {
                position.z = 600.0f;
                perimeter->direction = 2;
            }
            break;
        case 2:
            if (-90.0f == perimeter->rotation) {
                position.x -= 5.0f;
            } else if (perimeter->rotation <= 90.0f &&
                       perimeter->rotation >= -90.0f) {
                perimeter->rotation -= 5.0f;
            } else {
                perimeter->rotation += 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.x <= lbl_1_data_728[perimeter->targetIndex].position.x) {
                    position.x = lbl_1_data_728[perimeter->targetIndex].position.x;
                    negativeXArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(negativeXArrival->modelId,
                                       negativeXArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    negativeXArrival->motionState = 2;
                }
            } else if (position.x <= -600.0f) {
                position.x = -600.0f;
                perimeter->direction = 3;
            }
            break;
        case 3:
            if (180.0f == perimeter->rotation) {
                position.z -= 5.0f;
            } else if (perimeter->rotation >= 0.0f &&
                       perimeter->rotation <= 180.0f) {
                perimeter->rotation += 5.0f;
            } else {
                perimeter->rotation -= 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.z <= lbl_1_data_728[perimeter->targetIndex].position.z) {
                    position.z = lbl_1_data_728[perimeter->targetIndex].position.z;
                    negativeZArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(negativeZArrival->modelId,
                                       negativeZArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    negativeZArrival->motionState = 2;
                }
            } else if (position.z <= -600.0f) {
                position.z = -600.0f;
                perimeter->direction = 0;
            }
            break;
        }
    } else {
        switch (perimeter->direction) {
        case 0:
            if (-90.0f == perimeter->rotation) {
                position.x -= 5.0f;
            } else if (perimeter->rotation <= 90.0f &&
                       perimeter->rotation >= -90.0f) {
                perimeter->rotation -= 5.0f;
            } else {
                perimeter->rotation += 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.x <= lbl_1_data_728[perimeter->targetIndex].position.x) {
                    position.x = lbl_1_data_728[perimeter->targetIndex].position.x;
                    reverseNegativeXArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(reverseNegativeXArrival->modelId,
                                       reverseNegativeXArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    reverseNegativeXArrival->motionState = 2;
                }
            } else if (position.x <= -600.0f) {
                position.x = -600.0f;
                perimeter->direction = 3;
            }
            break;
        case 1:
            if (180.0f == perimeter->rotation) {
                position.z -= 5.0f;
            } else if (perimeter->rotation >= 0.0f &&
                       perimeter->rotation <= 180.0f) {
                perimeter->rotation += 5.0f;
            } else {
                perimeter->rotation -= 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.z <= lbl_1_data_728[perimeter->targetIndex].position.z) {
                    position.z = lbl_1_data_728[perimeter->targetIndex].position.z;
                    reverseNegativeZArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(reverseNegativeZArrival->modelId,
                                       reverseNegativeZArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    reverseNegativeZArrival->motionState = 2;
                }
            } else if (position.z <= -600.0f) {
                position.z = -600.0f;
                perimeter->direction = 0;
            }
            break;
        case 2:
            if (90.0f == perimeter->rotation) {
                position.x += 5.0f;
            } else if (perimeter->rotation <= 90.0f &&
                       perimeter->rotation >= -90.0f) {
                perimeter->rotation += 5.0f;
            } else {
                perimeter->rotation -= 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.x >= lbl_1_data_728[perimeter->targetIndex].position.x) {
                    position.x = lbl_1_data_728[perimeter->targetIndex].position.x;
                    reversePositiveXArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(reversePositiveXArrival->modelId,
                                       reversePositiveXArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    reversePositiveXArrival->motionState = 2;
                }
            } else if (position.x >= 600.0f) {
                position.x = 600.0f;
                perimeter->direction = 1;
            }
            break;
        case 3:
            if (0.0f == perimeter->rotation) {
                position.z += 5.0f;
            } else if (perimeter->rotation >= 0.0f &&
                       perimeter->rotation <= 180.0f) {
                perimeter->rotation -= 5.0f;
            } else {
                perimeter->rotation += 5.0f;
            }
            if (perimeter->direction == targetDirection) {
                if (position.z >= lbl_1_data_728[perimeter->targetIndex].position.z) {
                    position.z = lbl_1_data_728[perimeter->targetIndex].position.z;
                    reversePositiveZArrival =
                        &((M666PerimeterMotionState *) lbl_1_bss_D84)[(s16) perimeterIndex];
                    Hu3DMotionShiftSet(reversePositiveZArrival->modelId,
                                       reversePositiveZArrival->jointMotionIds[1],
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    reversePositiveZArrival->motionState = 2;
                }
            } else if (position.z >= 600.0f) {
                position.z = 600.0f;
                perimeter->direction = 2;
            }
            break;
        }
    }

    while (perimeter->rotation > 180.0f) {
        perimeter->rotation -= 360.0f;
    }
    while (perimeter->rotation <= -180.0f) {
        perimeter->rotation += 360.0f;
    }

    Hu3DModelPosSetV(perimeter->modelId, &position);
    Hu3DModelRotSet(perimeter->modelId, 0.0f, perimeter->rotation,
                    0.0f);
}

/* Moves a perimeter model toward its target point; fn_1_77B0 calls this in motion state 2. */
void fn_1_6D70(s16 perimeterIndex) {
    Vec position;
    M666PerimeterMotionWork *perimeter;
    M666PerimeterMotionWork *northboundPerimeter;
    M666PerimeterMotionWork *eastboundPerimeter;
    M666PerimeterMotionWork *southboundPerimeter;
    M666PerimeterMotionWork *westboundPerimeter;

    perimeter = &lbl_1_bss_D84[perimeterIndex];
    Hu3DModelPosGet(perimeter->modelId, &position);
    switch (perimeter->targetIndex) {
    case 0:
    case 1:
    case 2:
        if (perimeter->rotation == 180.0f) {
            position.z -= 2.5f;
            if (position.z <= lbl_1_data_728[0].z) {
                position.z = lbl_1_data_728[0].z;
                northboundPerimeter = &lbl_1_bss_D84[perimeterIndex];
                Hu3DMotionShiftSet(northboundPerimeter->modelId,
                                   northboundPerimeter->jointMotionIds[4],
                                   0.0f, 8.0f, 0U);
                northboundPerimeter->motionState = 3;
                lbl_1_bss_DC[northboundPerimeter->targetIndex].state = 2;
            }
        } else if ((perimeter->rotation >= 0.0f) &&
                   (perimeter->rotation <= 180.0f)) {
            perimeter->rotation += 5.0f;
        } else {
            perimeter->rotation -= 5.0f;
        }
        break;
    case 3:
        if (perimeter->rotation == 90.0f) {
            position.x += 2.5f;
            if (position.x >= lbl_1_data_728[3].x) {
                position.x = lbl_1_data_728[3].x;
                eastboundPerimeter = &lbl_1_bss_D84[perimeterIndex];
                Hu3DMotionShiftSet(eastboundPerimeter->modelId,
                                   eastboundPerimeter->jointMotionIds[4],
                                   0.0f, 8.0f, 0U);
                eastboundPerimeter->motionState = 3;
                lbl_1_bss_DC[eastboundPerimeter->targetIndex].state = 2;
            }
        } else if ((perimeter->rotation <= 90.0f) &&
                   (perimeter->rotation >= -90.0f)) {
            perimeter->rotation += 5.0f;
        } else {
            perimeter->rotation -= 5.0f;
        }
        break;
    case 4:
    case 5:
    case 6:
        if (perimeter->rotation == 0.0f) {
            position.z += 2.5f;
            if (position.z >= lbl_1_data_728[4].z) {
                position.z = lbl_1_data_728[4].z;
                southboundPerimeter = &lbl_1_bss_D84[perimeterIndex];
                Hu3DMotionShiftSet(southboundPerimeter->modelId,
                                   southboundPerimeter->jointMotionIds[4],
                                   0.0f, 8.0f, 0U);
                southboundPerimeter->motionState = 3;
                lbl_1_bss_DC[southboundPerimeter->targetIndex].state = 2;
            }
        } else if ((perimeter->rotation >= 0.0f) &&
                   (perimeter->rotation <= 180.0f)) {
            perimeter->rotation -= 5.0f;
        } else {
            perimeter->rotation += 5.0f;
        }
        break;
    case 7:
    case 8:
        if (perimeter->rotation == -90.0f) {
            position.x -= 2.5f;
            if (position.x <= lbl_1_data_728[7].x) {
                position.x = lbl_1_data_728[7].x;
                westboundPerimeter = &lbl_1_bss_D84[perimeterIndex];
                Hu3DMotionShiftSet(westboundPerimeter->modelId,
                                   westboundPerimeter->jointMotionIds[4],
                                   0.0f, 8.0f, 0U);
                westboundPerimeter->motionState = 3;
                lbl_1_bss_DC[westboundPerimeter->targetIndex].state = 2;
            }
        } else if ((perimeter->rotation <= 90.0f) &&
                   (perimeter->rotation >= -90.0f)) {
            perimeter->rotation -= 5.0f;
        } else {
            perimeter->rotation += 5.0f;
        }
        break;
    }

    while (perimeter->rotation > 180.0f) {
        perimeter->rotation -= 360.0f;
    }
    while (perimeter->rotation <= -180.0f) {
        perimeter->rotation += 360.0f;
    }
    Hu3DModelPosSetV(perimeter->modelId, &position);
    Hu3DModelRotSet(perimeter->modelId, 0.0f, perimeter->rotation, 0.0f);
}

/* Turns a perimeter model away from its target and restores its base motion at the final angle;
 * fn_1_77B0 calls this in state 4. Its position is written back unchanged. */
void fn_1_7348(s16 perimeterIndex) {
    Vec position;
    M666PerimeterMotionWork *perimeter;
    M666PerimeterMotionWork *zeroRotationPerimeter;
    M666PerimeterMotionWork *negativeQuarterTurnPerimeter;
    M666PerimeterMotionWork *halfTurnPerimeter;
    M666PerimeterMotionWork *quarterTurnPerimeter;

    perimeter = &lbl_1_bss_D84[perimeterIndex];
    Hu3DModelPosGet(perimeter->modelId, &position);
    switch (perimeter->targetIndex) {
    case 0:
    case 1:
    case 2:
        if (perimeter->rotation == 0.0f) {
            zeroRotationPerimeter = &lbl_1_bss_D84[perimeterIndex];
            Hu3DMotionShiftSet(zeroRotationPerimeter->modelId,
                               zeroRotationPerimeter->jointMotionIds[0],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            zeroRotationPerimeter->motionState = 0;
        } else if ((perimeter->rotation >= 0.0f) &&
                   (perimeter->rotation <= 180.0f)) {
            perimeter->rotation -= 5.0f;
        } else {
            perimeter->rotation += 5.0f;
        }
        break;
    case 3:
        if (perimeter->rotation == -90.0f) {
            negativeQuarterTurnPerimeter = &lbl_1_bss_D84[perimeterIndex];
            Hu3DMotionShiftSet(negativeQuarterTurnPerimeter->modelId,
                               negativeQuarterTurnPerimeter->jointMotionIds[0],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            negativeQuarterTurnPerimeter->motionState = 0;
        } else if ((perimeter->rotation <= 90.0f) &&
                   (perimeter->rotation >= -90.0f)) {
            perimeter->rotation -= 5.0f;
        } else {
            perimeter->rotation += 5.0f;
        }
        break;
    case 4:
    case 5:
    case 6:
        if (perimeter->rotation == 180.0f) {
            halfTurnPerimeter = &lbl_1_bss_D84[perimeterIndex];
            Hu3DMotionShiftSet(halfTurnPerimeter->modelId,
                               halfTurnPerimeter->jointMotionIds[0],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            halfTurnPerimeter->motionState = 0;
        } else if ((perimeter->rotation >= 0.0f) &&
                   (perimeter->rotation <= 180.0f)) {
            perimeter->rotation += 5.0f;
        } else {
            perimeter->rotation -= 5.0f;
        }
        break;
    case 7:
    case 8:
        if (perimeter->rotation == 90.0f) {
            quarterTurnPerimeter = &lbl_1_bss_D84[perimeterIndex];
            Hu3DMotionShiftSet(quarterTurnPerimeter->modelId,
                               quarterTurnPerimeter->jointMotionIds[0],
                               0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            quarterTurnPerimeter->motionState = 0;
        } else if ((perimeter->rotation <= 90.0f) &&
                   (perimeter->rotation >= -90.0f)) {
            perimeter->rotation += 5.0f;
        } else {
            perimeter->rotation -= 5.0f;
        }
        break;
    }

    while (perimeter->rotation > 180.0f) {
        perimeter->rotation -= 360.0f;
    }
    while (perimeter->rotation <= -180.0f) {
        perimeter->rotation += 360.0f;
    }

    Hu3DModelPosSetV(perimeter->modelId, &position);
    Hu3DModelRotSet(perimeter->modelId, 0.0f,
                    perimeter->rotation, 0.0f);
}

typedef M666PerimeterWork M666PerimeterMotion;

void fn_1_6030(s32 perimeterIndex);

void fn_1_6D70(s16 perimeterIndex);

void fn_1_7348(s16 perimeterIndex);

void fn_1_CBE8(s16 eventParameter);

void fn_1_5A78(void);

/* Updates the three perimeter models, their motion states, and their screen-space sprites. */
void fn_1_77B0(void) {
    Point3d modelPosition;
    Point3d screenPosition;
    f32 maximumMotionTime;
    s32 perimeterIndex;
    s32 spriteIndex;
    M666PerimeterMotion *perimeterMotion;

    for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
        perimeterMotion = &lbl_1_bss_D84[perimeterIndex];
        switch (perimeterMotion->motionState) {
        case 1:
            fn_1_6030(perimeterIndex);
            break;
        case 2:
            fn_1_6D70((s16)perimeterIndex);
            break;
        case 3:
            maximumMotionTime = Hu3DMotionMaxTimeGet(perimeterMotion->modelId);
            if (Hu3DMotionTimeGet(perimeterMotion->modelId) >= maximumMotionTime) {
                fn_1_CBE8(perimeterMotion->targetIndex);
                perimeterMotion->motionState = 4;
                Hu3DMotionShiftSet(perimeterMotion->modelId, perimeterMotion->jointMotionIds[1],
                                    0.0f, 8.0f, HU3D_MOTATTR_LOOP);
                for (spriteIndex = 0; spriteIndex < 10; spriteIndex++) {
                    HuSprAttrSet(perimeterMotion->spriteGroupId, (s16)spriteIndex, 4);
                }
            }
            if (18.0f == Hu3DMotionTimeGet(perimeterMotion->modelId)) {
                HuAudFXPlay(MSM_SE_M666_EVENT_ANIMATION_FRAME);
            }
            break;
        case 4:
            fn_1_7348((s16)perimeterIndex);
            break;
        case 6:
            if (perimeterMotion->countdown-- == 0) {
                perimeterMotion->motionState = 0;
                HuSprAttrSet(perimeterMotion->spriteGroupId, 9, 4);
                Hu3DMotionShiftSet(perimeterMotion->modelId, perimeterMotion->jointMotionIds[0],
                                    0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            }
            break;
        }

        Hu3DModelPosGet(perimeterMotion->modelId, &modelPosition);
        Hu3D3Dto2D(&modelPosition, 1, &screenPosition);
        HuSprGrpPosSet(perimeterMotion->spriteGroupId, screenPosition.x,
                       screenPosition.y - 50.0f);
    }
    fn_1_5A78();
}

/* Initializes each opening model's motion state and sprite attributes. */
void fn_1_79B4(void) {
    s32 perimeterIndex;
    s32 spriteIndex;
    M666PerimeterMotion *perimeterMotion;

    for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
        perimeterMotion = &lbl_1_bss_D84[perimeterIndex];
        perimeterMotion->motionState = 5;
        Hu3DMotionShiftSet(perimeterMotion->modelId,
                           perimeterMotion->jointMotionIds[5],
                           0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (spriteIndex = 0; spriteIndex < 10; spriteIndex++) {
            HuSprAttrSet(perimeterMotion->spriteGroupId, (s16)spriteIndex, 4);
        }
    }
}

/* Resets state-5 perimeter motions and stops sounds for their distant target points. */
void fn_1_7A64(void) {
    Point3d modelPosition;
    s32 perimeterIndex;
    s32 pointIndex;
    M666PerimeterMotion *perimeterMotion;

    for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
        perimeterMotion = &lbl_1_bss_D84[perimeterIndex];
        if (perimeterMotion->motionState == 5) {
            Hu3DMotionShiftSet(perimeterMotion->modelId, perimeterMotion->jointMotionIds[0],
                                0.0f, 8.0f, HU3D_MOTATTR_LOOP);
            perimeterMotion->motionState = 0;
            Hu3DModelPosGet(perimeterMotion->modelId, &modelPosition);

            for (pointIndex = 0; pointIndex < 9; pointIndex++) {
                if (perimeterMotion->targetIndex == pointIndex) {
                    if (sqrtf(PSVECSquareDistance(
                            &modelPosition, &lbl_1_data_728[pointIndex].position)) >
                        100.0f) {
                        lbl_1_bss_DC[pointIndex].state = 0;
                        if (lbl_1_bss_DC[pointIndex].soundHandle != -1) {
                            HuAudFXStop(lbl_1_bss_DC[pointIndex].soundHandle);
                            lbl_1_bss_DC[pointIndex].soundHandle = -1;
                        }
                    }
                }
            }
        }
    }
}

/* Starts the perimeter models' joint motion 3 when fn_1_4AC reaches timer 40 in state 3. */
void fn_1_7CA0(void) {
    s32 perimeterIndex;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        Hu3DMotionShiftSet(lbl_1_bss_D84[perimeterIndex].modelId,
                           lbl_1_bss_D84[perimeterIndex].jointMotionIds[3], 0.0f,
                           8.0f, HU3D_MOTATTR_LOOP);
        perimeterIndex += 1;
    }
}

/* Restores the perimeter models' base motions from fn_1_4AC state 5 before the sequence
 * advances. */
void fn_1_7D24(void) {
    s32 perimeterIndex;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        Hu3DMotionShiftSet(lbl_1_bss_D84[perimeterIndex].modelId,
                           lbl_1_bss_D84[perimeterIndex].jointMotionIds[0], 0.0f,
                           8.0f, HU3D_MOTATTR_LOOP);
        perimeterIndex += 1;
    }
}

/* Restores state-1 perimeter motions and hides their sprites during fn_1_92C's cleanup. */
void fn_1_7DA8(void) {
    s32 spriteIndex;
    s32 perimeterIndex;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        if (lbl_1_bss_D84[perimeterIndex].motionState == 1) {
            lbl_1_bss_D84[perimeterIndex].motionState = 0;
            Hu3DMotionShiftSet(lbl_1_bss_D84[perimeterIndex].modelId,
                               lbl_1_bss_D84[perimeterIndex].jointMotionIds[0], 0.0f,
                               8.0f, HU3D_MOTATTR_LOOP);
            spriteIndex = 0;
            while (spriteIndex < 10) {
                HuSprAttrSet(lbl_1_bss_D84[perimeterIndex].spriteGroupId, (s16)spriteIndex, 4);
                spriteIndex += 1;
            }
        }
        perimeterIndex += 1;
    }
}

/* Updates the opening models' rotation angle. */
void fn_1_7E9C(void) {
    s32 perimeterIndex;
    M666MotionWork *perimeterWork;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        perimeterWork = &lbl_1_bss_D84[perimeterIndex];
        if (perimeterWork->motionState != 7) {
            perimeterWork->motionState = 7;
            Hu3DMotionShiftSet(perimeterWork->modelId,
                               perimeterWork->jointMotionIds[1], 0.0f, 8.0f,
                               HU3D_MOTATTR_LOOP);
        }
        if ((perimeterWork->rotation < 45.0f) &&
            (perimeterWork->rotation > -135.0f)) {
            perimeterWork->rotation += 5.0f;
            if (perimeterWork->rotation > 45.0f) {
                perimeterWork->rotation = (f32)45.0f;
                Hu3DMotionShiftSet(perimeterWork->modelId,
                                   perimeterWork->jointMotionIds[0], 0.0f, 8.0f,
                                   HU3D_MOTATTR_LOOP);
            }
        } else if (45.0f != perimeterWork->rotation) {
            /* Rotations at or below -135 jump directly to 45 after this decrement. */
            perimeterWork->rotation -= 5.0f;
            if (perimeterWork->rotation < 45.0f) {
                perimeterWork->rotation = (f32)45.0f;
                Hu3DMotionShiftSet(perimeterWork->modelId,
                                   perimeterWork->jointMotionIds[0], 0.0f, 8.0f,
                                   HU3D_MOTATTR_LOOP);
            }
        }
        while (perimeterWork->rotation > 180.0f) {
            perimeterWork->rotation -= 360.0f;
        }
        while (perimeterWork->rotation <= -180.0f) {
            perimeterWork->rotation += 360.0f;
        }
        Hu3DModelRotSet(perimeterWork->modelId, 0.0f,
                        perimeterWork->rotation, 0.0f);
        perimeterIndex += 1;
    }
}

/* Starts perimeter joint motion 3 for the microphone-player result in fn_1_78C state 2. */
void fn_1_80E0(void) {
    s32 perimeterIndex;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        Hu3DMotionShiftSet(
            lbl_1_bss_D84[perimeterIndex].modelId,
            lbl_1_bss_D84[perimeterIndex].jointMotionIds[3],
            0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        perimeterIndex += 1;
    }
}

/* Starts perimeter joint motion 6 for the three-player result in fn_1_78C state 2. */
void fn_1_8164(void) {
    s32 perimeterIndex;

    perimeterIndex = 0;
    while (perimeterIndex < 3) {
        Hu3DMotionShiftSet(
            lbl_1_bss_D84[perimeterIndex].modelId,
            lbl_1_bss_D84[perimeterIndex].jointMotionIds[6],
            0.0f, 8.0f, 0U);
        perimeterIndex += 1;
    }
}

/* Creates the obstacle model set during minigame setup before obstacle updates begin. */
void fn_1_81E4(void) {
    s32 groupIndex;
    s32 entryIndex;
    s32 motionIndex;
    s16 baseModelId;
    s16 modelId;
    s16 jointMotionId;

    for (groupIndex = 0; groupIndex < 6; groupIndex++) {
        for (entryIndex = 0; entryIndex < 8; entryIndex++) {
            if (groupIndex == 0 && entryIndex == 0) {
                baseModelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 34), HU_MEMNUM_OVL, HEAP_MODEL));
                modelId = baseModelId;
            } else {
                modelId = Hu3DModelLink(baseModelId);
            }

            Hu3DModelPosSetV(
                modelId, &lbl_1_data_2A0[entryIndex + groupIndex * 8].position);
            Hu3DModelRotSet(modelId, 0.0f,
                            lbl_1_data_2A0[entryIndex + groupIndex * 8].rotation,
                            0.0f);
            lbl_1_bss_604[groupIndex][entryIndex].modelId = modelId;
            Hu3DModelShadowSet(modelId);

            for (motionIndex = 0; motionIndex < 4; motionIndex++) {
                jointMotionId = Hu3DJointMotion(
                    modelId, HuDataSelHeapReadNum(
                                 lbl_1_data_660[motionIndex].fileId,
                                  HU_MEMNUM_OVL, HEAP_MODEL));
                lbl_1_bss_604[groupIndex][entryIndex]
                    .motion.jointMotionIds[motionIndex] = jointMotionId;
            }

            Hu3DMotionSet(
                modelId, lbl_1_bss_604[groupIndex][entryIndex].motion.jointMotionIds[1]);
            Hu3DModelAttrSet(modelId, HU3D_MOTATTR_LOOP);
            lbl_1_bss_604[groupIndex][entryIndex].slotIndex = entryIndex;
            lbl_1_bss_604[groupIndex][entryIndex].state = 0;
            lbl_1_bss_580[groupIndex].data.entryRow.entryValues[entryIndex] = entryIndex;

            modelId = Hu3DModelCreate(
         HuDataSelHeapReadNum(DATANUM(DATA_m666, 45), HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DMotionSpeedSet(modelId, 0.0f);
            Hu3DModelAttrSet(modelId, 1);
            lbl_1_bss_604[groupIndex][entryIndex].secondaryModelId = modelId;
            lbl_1_bss_604[groupIndex][entryIndex].actor =
                (lbl_1_bss_604[groupIndex][entryIndex].secondaryActor = NULL);

            modelId = Hu3DModelLink(baseModelId);
            Hu3DModelShadowSet(modelId);
            Hu3DModelAttrSet(modelId, 1);
            lbl_1_bss_604[groupIndex][entryIndex].alternateModelId = modelId;
        }
    }
}

void fn_1_854C(s16 groupIndex, s16 entryIndex);

void fn_1_8720(s16 groupIndex, s16 obstacleIndex);

void fn_1_88B4(s16 groupIndex, s16 entryIndex);

void fn_1_8964(s16 groupIndex, s16 entryIndex);

typedef M666DataTransform M666StartPosition;

void fn_1_8A14(s16 groupIndex, s16 obstacleIndex);

void fn_1_901C(s32 groupIndex, s32 entryIndex);

/* Sets every state-4 obstacle's timer to 180, triggering its next timed transition. */
void fn_1_84C4(void) {
    s32 groupIndex;
    s32 entryIndex;

    for (groupIndex = 0; groupIndex < 6; groupIndex++) {
        for (entryIndex = 0; entryIndex < 8; entryIndex++) {
            if (lbl_1_bss_604[groupIndex][entryIndex].state == 4) {
                lbl_1_bss_604[groupIndex][entryIndex].timer = 180;
            }
        }
    }
}

/* Creates the primary collision actor for an obstacle and positions it at the model;
 * rear groups face 180 degrees. */
void fn_1_854C(s16 groupIndex, s16 entryIndex) {
    MGACTOR_PARAM actorParam;
    Point3d modelPosition;

    if ((u32) lbl_1_bss_604[groupIndex][entryIndex].actor == 0U) {
        Hu3DModelPosGet(lbl_1_bss_604[groupIndex][entryIndex].modelId, &modelPosition);
        actorParam.height = 100.0f;
        actorParam.radius = 50.0f;
        actorParam.param = entryIndex + ((groupIndex << 3) + 4);
        actorParam.type = 1;
        actorParam.attr = 0;
        actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_4378;
        actorParam.correctHook = NULL;
        lbl_1_bss_604[groupIndex][entryIndex].actor =
            MgActorCreate(&actorParam, lbl_1_bss_604[groupIndex][entryIndex].modelId);
        MgActorPosSet(lbl_1_bss_604[groupIndex][entryIndex].actor, &modelPosition);
        if (groupIndex >= 3) {
            MgActorRotYSet(lbl_1_bss_604[groupIndex][entryIndex].actor, 180.0f);
            lbl_1_bss_604[groupIndex][entryIndex].actorRotation = 180.0f;
        }
    }
}

/* Creates an obstacle's secondary collision actor, positions it at the model,
 * and disables gravity. */
void fn_1_8720(s16 groupIndex, s16 obstacleIndex) {
    MGACTOR_PARAM actorParam;
    Point3d modelPosition;

    if ((u32)lbl_1_bss_604[groupIndex][obstacleIndex].secondaryActor == 0U) {
        Hu3DModelPosGet(lbl_1_bss_604[groupIndex][obstacleIndex].secondaryModelId,
                        &modelPosition);
        actorParam.height = 200.0f;
        actorParam.radius = 100.0f;
        actorParam.param = obstacleIndex + ((groupIndex << 3) + 52);
        actorParam.type = 2;
        actorParam.attr = 2;
        actorParam.narrowHook = NULL;
        actorParam.correctHook = NULL;

        lbl_1_bss_604[groupIndex][obstacleIndex].secondaryActor =
            MgActorCreate(&actorParam,
                          lbl_1_bss_604[groupIndex][obstacleIndex].secondaryModelId);
        MgActorPosSet(lbl_1_bss_604[groupIndex][obstacleIndex].secondaryActor,
                      &modelPosition);
        MgActorGravitySet(lbl_1_bss_604[groupIndex][obstacleIndex].secondaryActor,
                          0.0f);
    }
}

/* Kills and clears an obstacle's primary collision actor when it exists. */
void fn_1_88B4(s16 groupIndex, s16 entryIndex) {
    if (lbl_1_bss_604[groupIndex][entryIndex].actor) {
        MgActorKill(lbl_1_bss_604[groupIndex][entryIndex].actor);
        lbl_1_bss_604[groupIndex][entryIndex].actor = NULL;
    }
}

/* Kills and clears an obstacle's secondary collision actor when it exists. */
void fn_1_8964(s16 groupIndex, s16 entryIndex) {
    if (lbl_1_bss_604[groupIndex][entryIndex].secondaryActor) {
        MgActorKill(lbl_1_bss_604[groupIndex][entryIndex].secondaryActor);
        lbl_1_bss_604[groupIndex][entryIndex].secondaryActor = NULL;
    }
}

/* Appends an obstacle index to its group queue, places it at the final start transform, shows it,
 * and starts its loop motion. */
void fn_1_8A14(s16 groupIndex, s16 obstacleIndex) {
    M666Bss580Entry *group;
    M666MovingObstacle *obstacle;

    group = &lbl_1_bss_580[groupIndex];
    obstacle = &lbl_1_bss_604[groupIndex][obstacleIndex];
    group->data.obstacleQueue.obstacleIndices[group->data.obstacleQueue.obstacleCount] =
        obstacleIndex;
    lbl_1_bss_604[groupIndex][obstacleIndex].slotIndex =
        group->data.obstacleQueue.obstacleCount;
    group->data.obstacleQueue.obstacleCount++;
    Hu3DModelPosSetV(obstacle->modelId,
                     &lbl_1_data_2A0[groupIndex * 8 + 7].position);
    Hu3DModelRotSet(obstacle->modelId, 0.0f,
                    lbl_1_data_2A0[groupIndex * 8 + 7].rotationY,
                    0.0f);
    Hu3DModelAttrReset(obstacle->modelId, 1);
    obstacle->state = 1;
    Hu3DMotionShiftSet(obstacle->modelId,
                       obstacle->motion.obstacleMotions.loopMotionId,
                        0.0f, 8.0f, HU3D_MOTATTR_LOOP);
    Hu3DMotionSpeedSet(obstacle->modelId, 1.0f);
}

void fn_1_8B88(s32 groupIndex, s32 entryIndex);

typedef M666DataTransform M666ObstacleSpawnPoint;

f32 lbl_1_data_810[8] = {
    820.0f, 1040.0f, 1260.0f, 1480.0f,
    1700.0f, 1700.0f, 1700.0f, 1700.0f
};

/* Adds collision to a landed obstacle when fn_1_9260 advances it into the resting state. */
static inline void m666CreateObstacleActor(s16 groupIndex, s16 obstacleIndex) {
    MGACTOR_PARAM actorParam;
    Point3d modelPosition;

    if ((u32)lbl_1_bss_604[groupIndex][obstacleIndex].actor == 0U) {
        Hu3DModelPosGet(lbl_1_bss_604[groupIndex][obstacleIndex].modelId,
                        &modelPosition);
        actorParam.height = 100.0f;
        actorParam.radius = 50.0f;
        actorParam.param = obstacleIndex + ((groupIndex << 3) + 4);
        actorParam.type = 1;
        actorParam.attr = 0;
        actorParam.narrowHook = (COL_NARROW_HOOK)fn_1_4378;
        actorParam.correctHook = NULL;
        lbl_1_bss_604[groupIndex][obstacleIndex].actor =
            MgActorCreate(&actorParam,
                          lbl_1_bss_604[groupIndex][obstacleIndex].modelId);
        MgActorPosSet(lbl_1_bss_604[groupIndex][obstacleIndex].actor,
                      &modelPosition);
        if (groupIndex >= 3) {
            MgActorRotYSet(lbl_1_bss_604[groupIndex][obstacleIndex].actor,
                           180.0f);
            lbl_1_bss_604[groupIndex][obstacleIndex].actorRotation =
                180.0f;
        }
    }
}

/* Starts the secondary model animation and removes the primary collision actor after
 * fn_1_9260's delay, with rumble and sound. */
static inline void m666ResetObstacle(s32 groupIndex, s32 obstacleIndex) {
    Point3d modelPosition;
    M666MovingObstacle *entry;
    s32 playerIndex;

    entry = &lbl_1_bss_604[groupIndex][obstacleIndex];
    Hu3DModelAttrSet(entry->alternateModelId, 1);
    Hu3DModelAttrReset(entry->modelId, 1);
    Hu3DModelPosGet(entry->modelId, &modelPosition);
    entry->state = 6;
    entry->timer = 0;
    Hu3DModelPosSetV(entry->secondaryModelId, &modelPosition);
    Hu3DModelAttrReset(entry->secondaryModelId, 1);
    Hu3DModelAttrSet(entry->modelId, 1);
    Hu3DMotionSpeedSet(entry->secondaryModelId, 1.0f);
    Hu3DMotionTimeSet(entry->secondaryModelId, 0.0f);
    if (lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].actor) {
        MgActorKill(lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].actor);
        lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].actor = NULL;
    }

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            omVibrate(lbl_1_bss_8[playerIndex].playerNumber,
                      20, 4, 4);
        }
    }

    if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
        omVibrate(lbl_1_bss_98.playerIndex, 20, 4, 4);
    }
    HuAudFXPlayPan(MSM_SE_M666_OBSTACLE_RESET, 64.0 -
        32.0 * sin((3.141592653589793 * (f64)modelPosition.x) /
                               180.0));
}

void fn_1_9260(void);

f32 lbl_1_data_830[3] = {720.0f, 1100.0f, 1380.0f};

/* Spawns one lane-effect particle for a lane while fn_1_A798 advances its crossing state. */
static inline void m666ActivateLaneModel(s16 groupIndex, s16 itemIndex) {
    Point3d position;
    s32 modelIndex;

    for (modelIndex = 0; modelIndex < 8; modelIndex++) {
        if (lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex].active != 0) {
            continue;
        }

        Hu3DModelPosGet(lbl_1_bss_148[groupIndex].items[itemIndex].rootModelId,
                        &position);
        position.x += groupIndex == 0 ? 200.0f : -200.0f;
        position.x += -30.0f + (f32)frandmod(60);
        position.y += -30.0f + (f32)frandmod(60);
        position.z += -30.0f + (f32)frandmod(60);
        Hu3DModelPosSetV(
            lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex]
                .modelId,
            &position);
        Hu3DModelAttrReset(
            lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex]
                .modelId,
            1);
        Hu3DModelTPLvlSet(
            lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex]
                .modelId,
            1.0f);
        Hu3DModelScaleSet(
            lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex]
                .modelId,
            1.0f, 1.0f, 1.0f);
        lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex].active = 1;
        lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex].alpha = 1.0f;
        lbl_1_bss_148[groupIndex].items[itemIndex].particles[modelIndex].scale = 1.0f;
        return;
    }
}

void fn_1_9D3C(s16 groupIndex);

void fn_1_9F3C(void);

void fn_1_A798(void);

/* At timer 5, selects the nearest active player; later calls retain that target, turn toward it,
 * push forward, and update the grid cell. */
void fn_1_8B88(s32 groupIndex, s32 entryIndex) {
    M666MovingObstacle *entry;
    MGACTOR *actor;
    MGPLAYER *playerHolder;
    MGACTOR *playerActor;
    Vec direction;
    Vec actorPosition;
    Vec targetPosition;
    Vec currentPosition;
    f32 nearestDistance;
    f32 distanceSquared;
    f32 candidateDistance;
    f32 desiredRotation;
    f32 rotationDifference;
    f32 absoluteDifference;
    s16 nearestPlayer;
    s32 playerIndex;
    s32 gridX;
    s32 gridZ;

    entry = &lbl_1_bss_604[groupIndex][entryIndex];
    actor = entry->actor;
    if (entry->timer == 5) {
        nearestDistance = 10000.0f;
        nearestPlayer = -1;
        MgActorPosGet(entry->actor, &actorPosition);
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
                playerHolder = lbl_1_bss_8[playerIndex].player;
                playerActor = playerHolder->actor;
                MgActorPosGet(playerActor, &targetPosition);
                distanceSquared = PSVECSquareDistance(&actorPosition,
                                                       &targetPosition);
                candidateDistance = sqrtf(distanceSquared);
                if (nearestDistance >= candidateDistance) {
                    nearestDistance = candidateDistance;
                    nearestPlayer = playerIndex;
                }
            }
        }
        entry->targetPlayerIndex = nearestPlayer;
    }

    if (entry->targetPlayerIndex != -1) {
        playerHolder = lbl_1_bss_8[entry->targetPlayerIndex].player;
        playerActor = playerHolder->actor;
        MgActorPosGet(playerActor, &targetPosition);
        MgActorPosGet(entry->actor, &actorPosition);
        direction.x = targetPosition.x - actorPosition.x;
        direction.y = 0.0f;
        direction.z = targetPosition.z - actorPosition.z;
        desiredRotation = (f32)(180.0 *
            (atan2((f64)direction.x, (f64)direction.z) / 3.141592653589793));
        rotationDifference = desiredRotation - entry->actorRotation;
        while (rotationDifference > 180.0f) {
            rotationDifference -= 360.0f;
        }
        while (rotationDifference <= -180.0f) {
            rotationDifference += 360.0f;
        }

        absoluteDifference = __fabsf(rotationDifference);
        if (absoluteDifference < 5.0) {
            entry->actorRotation = desiredRotation;
        } else if (rotationDifference > 0.0f) {
            entry->actorRotation += 5.0f;
        } else {
            entry->actorRotation -= 5.0f;
        }
    }

    MgActorRotYSet(entry->actor, entry->actorRotation);
    direction.x = 0.0f;
    direction.y = 0.0f;
    direction.z = 5.0f;
    MgActorPushSet(actor, &direction);
    MgActorPosGet(entry->actor, &currentPosition);
    if (currentPosition.x <= -150.0f) {
        gridX = 0;
    } else if (currentPosition.x >= 150.0f) {
        gridX = 2;
    } else {
        gridX = 1;
    }
    if (currentPosition.z <= -150.0f) {
        gridZ = 0;
    } else if (currentPosition.z >= 150.0f) {
        gridZ = 2;
    } else {
        gridZ = 1;
    }
    entry->unknown1E = gridX + (gridZ * 3);
}

/* Resets an obstacle's models and actor, enters state 6, and signals players
 * with rumble and sound. */
void fn_1_901C(s32 groupIndex, s32 entryIndex) {
    Point3d modelPosition;
    M666MovingObstacle *entry;
    s32 playerIndex;

    entry = &lbl_1_bss_604[groupIndex][entryIndex];
    Hu3DModelAttrSet(entry->alternateModelId, 1);
    Hu3DModelAttrReset(entry->modelId, 1);
    Hu3DModelPosGet(entry->modelId, &modelPosition);
    entry->state = 6;
    entry->timer = 0;
    Hu3DModelPosSetV(entry->secondaryModelId, &modelPosition);
    Hu3DModelAttrReset(entry->secondaryModelId, 1);
    Hu3DModelAttrSet(entry->modelId, 1);
    Hu3DMotionSpeedSet(entry->secondaryModelId, 1.0f);
    Hu3DMotionTimeSet(entry->secondaryModelId, 0.0f);
    if (lbl_1_bss_604[(s16)groupIndex][(s16)entryIndex].actor) {
        MgActorKill(lbl_1_bss_604[(s16)groupIndex][(s16)entryIndex].actor);
        lbl_1_bss_604[(s16)groupIndex][(s16)entryIndex].actor = NULL;
    }

    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            omVibrate(lbl_1_bss_8[playerIndex].playerNumber, 20, 4, 4);
        }
    }

    if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
        omVibrate(lbl_1_bss_98.playerIndex, 20, 4, 4);
    }
    HuAudFXPlayPan(MSM_SE_M666_OBSTACLE_RESET, (s16)(64.0 -
        32.0 * sin((3.141592653589793 * (f64)modelPosition.x) / 180.0)));
}

/* Advances all moving obstacles through their states, updating models, actors, effects,
 * and the active-position average. */
void fn_1_9260(void) {
    M666MovingObstacle *obstacle;
    Point3d position;
    s32 groupIndex;
    s32 obstacleIndex;
    s32 playerIndex;
    f32 angle;

    lbl_1_bss_E90 = 0;
    lbl_1_bss_E94.x = lbl_1_bss_E94.y = lbl_1_bss_E94.z = 0.0f;
    for (groupIndex = 0; groupIndex < 6; groupIndex++) {
        for (obstacleIndex = 0; obstacleIndex < 8; obstacleIndex++) {
            obstacle = (M666MovingObstacle *)&lbl_1_bss_604[groupIndex][obstacleIndex];
            Hu3DModelPosGet(obstacle->modelId, &position);
            switch (obstacle->state) {
            case 2:
                break;
            case 1:
                if (fabs(position.z) > lbl_1_data_810[obstacle->slotIndex]) {
                    position.z += groupIndex < 3 ?
                                  5.0f : -5.0f;
                } else {
                    Hu3DMotionShiftSet(obstacle->modelId,
                                        obstacle->motion.obstacleMotions.startMotionId,
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    obstacle->state = 0;
                }
                break;
            case 3:
                Hu3DModelPosGet(obstacle->modelId, &position);
                position.y += obstacle->verticalStep;
                position.z += groupIndex < 3 ?
                              5.0f : -5.0f;
                obstacle->verticalStep -= 1.0f;
                if (position.y <= 0.0f) {
                    Hu3DMotionShiftSet(obstacle->modelId,
                                        obstacle->motion.obstacleMotions.fallMotionId,
                                       0.0f, 8.0f,
                                       HU3D_MOTATTR_LOOP);
                    m666CreateObstacleActor(groupIndex, obstacleIndex);
                    position.y = 0.0f;
                    obstacle->state = 4;
                    if (MgSeqModeGet() == 5) {
                        obstacle->timer = 0;
                    } else {
                        obstacle->timer = 180;
                    }
                    obstacle->targetPlayerIndex = -1;
                }
                Hu3DModelPosSetV(obstacle->modelId, &position);
                break;
            case 4:
                fn_1_8B88(groupIndex, obstacleIndex);
                if (obstacle->timer++ >= 180) {
                    obstacle->state = 5;
                    MgActorPosGet(obstacle->actor, &position);
                    Hu3DModelPosSetV(obstacle->alternateModelId, &position);
                    Hu3DModelAttrSet(obstacle->modelId, 1U);
                    Hu3DModelAttrReset(obstacle->alternateModelId, 1U);
                    Hu3DMotionSet(obstacle->alternateModelId,
                                   obstacle->motion.obstacleMotions.alternateMotionId);
                    MgActorRotYGet(obstacle->actor, &angle);
                    Hu3DModelRotSet(obstacle->alternateModelId,
                                    0.0f, angle, 0.0f);
                    HuAudFXPlayPan(MSM_SE_GUIDE_05,
                                   64.0 - 32.0 *
                                   sin((3.141592653589793 * (f64)position.x) /
                                       180.0));
                    obstacle->timer = 0;
                }
                break;
            case 5:
                if (obstacle->timer++ >= 60) {
                    m666ResetObstacle(groupIndex, obstacleIndex);
                }
                break;
            case 6:

                if (Hu3DMotionTimeGet(obstacle->secondaryModelId) >=
                    Hu3DMotionMaxTimeGet(obstacle->secondaryModelId)) {

                    M666Bss580Entry *group =
                        &lbl_1_bss_580[(s16)groupIndex];
                    M666MovingObstacle *slotObstacle = (M666MovingObstacle *)
                        &lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex];

                    group->data.obstacleGroup.slotIndices[
                        group->data.obstacleGroup.activeCount] = (s16)obstacleIndex;
                    ((M666MovingObstacle *)&lbl_1_bss_604[(s16)groupIndex]
                        [(s16)obstacleIndex])->slotIndex =
                        group->data.obstacleGroup.activeCount;
                    group->data.obstacleGroup.activeCount += 1;
                    Hu3DModelPosSetV(slotObstacle->modelId,
                                     &lbl_1_data_2A0[(s16)groupIndex * 8 + 7].position);
                    Hu3DModelRotSet(slotObstacle->modelId, 0.0f,
                                    lbl_1_data_2A0[(s16)groupIndex * 8 + 7].rotation,
                                    0.0f);
                    Hu3DModelAttrReset(slotObstacle->modelId, 1U);
                    slotObstacle->state = 1;
                    Hu3DMotionShiftSet(slotObstacle->modelId,
                                        slotObstacle->motion.obstacleMotions.loopMotionId, 0.0f,
                                       8.0f,
                                       HU3D_MOTATTR_LOOP);
                    Hu3DMotionSpeedSet(slotObstacle->modelId, 1.0f);
                    /* Stop this entire update after recycling one obstacle; later entries and the
                     * final average are skipped this frame. */
                    return;
                }
                break;
            }
            Hu3DModelPosSetV(obstacle->modelId, &position);

            if (Hu3DMotionTimeGet(obstacle->secondaryModelId) >=
                Hu3DMotionMaxTimeGet(obstacle->secondaryModelId)) {

                Hu3DModelAttrSet(obstacle->secondaryModelId, 1U);
                if (lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].secondaryActor) {
                    MgActorKill(lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].secondaryActor);
                    lbl_1_bss_604[(s16)groupIndex][(s16)obstacleIndex].secondaryActor = NULL;
                }
            }
            if (obstacle->state >= 3) {
                if (obstacle->state <= 5) {
                    Hu3DModelPosGet(obstacle->modelId, &position);
                } else {
                    Hu3DModelPosGet(obstacle->secondaryModelId, &position);
                }
                lbl_1_bss_E94.x += position.x;
                lbl_1_bss_E94.z += position.z;
                lbl_1_bss_E90++;
            }
        }
    }
    if (lbl_1_bss_E90 != 0) {
        lbl_1_bss_E94.x /= (f32)lbl_1_bss_E90;
        lbl_1_bss_E94.z /= (f32)lbl_1_bss_E90;
    }
}

/* Attaches the queued obstacle to the group model's bomb hook, resets its transform, restarts the
 * group motion, and plays the hook sound. */
void fn_1_9D3C(s16 groupIndex) {
    M666MovingObstacle *obstacleEntry;

    if (lbl_1_bss_580[groupIndex].data.motionEvent.state == 0) {
        lbl_1_bss_580[groupIndex].data.motionEvent.state = 1;
        obstacleEntry = &lbl_1_bss_604[groupIndex]
            [lbl_1_bss_580[groupIndex].data.motionEvent.obstacleIndices[0]];
        Hu3DModelHookSet(lbl_1_bss_580[groupIndex].modelId, lbl_1_data_7F2,
                         obstacleEntry->modelId);
        Hu3DModelPosSet(obstacleEntry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(obstacleEntry->modelId, 0.0f, 0.0f, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_580[groupIndex].modelId, 1.0f);
        Hu3DMotionTimeSet(lbl_1_bss_580[groupIndex].modelId, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_580[groupIndex].modelId, HU3D_MOTATTR_REV);
        HuAudFXPlay(MSM_SE_M666_BOMB_HOOK);
    }
}

/* Puts every obstacle entry into its initial active state. */
void fn_1_9ED8(void) {
    s32 groupIndex;
    s32 entryIndex;
    groupIndex = 0;
    while (groupIndex < 6)
    {
        entryIndex = 0;
        while (entryIndex < 8)
        {
            lbl_1_bss_604[groupIndex][entryIndex].state = 1;
            entryIndex += 1;
        }
        groupIndex += 1;
    }
}

/* Advances the six obstacle-group motions, detaches launched models, and compacts queued
 * obstacle indices as motions finish. */
void fn_1_9F3C(void) {
    Point3d modelPosition;
    f32 maximumMotionTime;
    s32 eventIndex;
    s32 queueIndex;
    M666Bss580Entry *motionEvent;
    M666MovingObstacle *obstacle;

    for (eventIndex = 0; eventIndex < 6; eventIndex++) {
        motionEvent = &lbl_1_bss_580[eventIndex];
        switch (motionEvent->data.motionEvent.state) {
        case 1:
            maximumMotionTime = Hu3DMotionMaxTimeGet(motionEvent->modelId);
            if (Hu3DMotionTimeGet(motionEvent->modelId) >= maximumMotionTime) {
                obstacle =
                    &lbl_1_bss_604[eventIndex][motionEvent->data.motionEvent.obstacleIndices[0]];
                Hu3DModelObjPosGet(motionEvent->modelId, lbl_1_data_7F2, &modelPosition);
                Hu3DModelHookReset(motionEvent->modelId);
                Hu3DModelPosSetV(obstacle->modelId, &modelPosition);
                if (eventIndex >= 3) {
                    Hu3DModelRotSet(obstacle->modelId, 0.0f,
                                    180.0f, 0.0f);
                }
                 Hu3DModelAttrSet(motionEvent->modelId, HU3D_MOTATTR_REV);
                obstacle->state = 3;
                obstacle->verticalStep = 8.0f;
                motionEvent->data.motionEvent.state = 2;
            }
            break;
        case 2:
            if (Hu3DMotionTimeGet(motionEvent->modelId) < 10.0f) {
                motionEvent->data.motionEvent.state = 3;
                if (motionEvent->data.motionEvent.obstacleCount > 1) {
                    for (queueIndex = 0;
                         queueIndex < motionEvent->data.motionEvent.obstacleCount - 1;
                         queueIndex++) {
                        motionEvent->data.motionEvent.obstacleIndices[queueIndex] =
                            motionEvent->data.motionEvent.obstacleIndices[queueIndex + 1];
                        obstacle = &lbl_1_bss_604[eventIndex]
                            [motionEvent->data.motionEvent.obstacleIndices[queueIndex]];
                        obstacle->state = 1;
                        obstacle->slotIndex -= 1;
                        Hu3DMotionShiftSet(obstacle->modelId,
                                           obstacle->motion.obstacleMotions.loopMotionId, 0.0f,
                                           8.0f, HU3D_MOTATTR_LOOP);
                    }
                    motionEvent->data.motionEvent.obstacleCount -= 1;
                } else {
                    motionEvent->data.motionEvent.obstacleIndices[0] = -1;
                    motionEvent->data.motionEvent.obstacleCount = 0;
                }
            }
            break;
        case 3:
            if (motionEvent->data.motionEvent.obstacleCount != 0) {
                if (lbl_1_bss_604[eventIndex][motionEvent->data.motionEvent.obstacleIndices[0]]
                        .state == 0) {
                    motionEvent->data.motionEvent.state = 0;
                }
            }
            break;
        }
    }
}

/* Creates the nine lane models and eight particle models for each lane. */
void fn_1_A1CC(void) {
    s32 groupIndex;
    s32 itemIndex;
    s32 modelIndex;
    s16 baseModelId;
    s16 modelId;

    for (groupIndex = 0; groupIndex < 3; groupIndex++) {
        for (itemIndex = 0; itemIndex < 3; itemIndex++) {
            if (groupIndex == 0 && itemIndex == 0) {
                baseModelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 43), HU_MEMNUM_OVL, HEAP_MODEL));
                modelId = baseModelId;
            } else {
                modelId = Hu3DModelLink(baseModelId);
            }

            Hu3DModelPosSetV(modelId, &lbl_1_data_210[itemIndex + groupIndex * 3].position);
            Hu3DModelRotSet(modelId, 0.0f,
                            lbl_1_data_27C[itemIndex + groupIndex * 3], 0.0f);
            Hu3DModelScaleSet(modelId, 0.5f,
                              0.5f, 0.5f);
            Hu3DModelShadowSet(modelId);
            lbl_1_bss_148[groupIndex].items[itemIndex].rootModelId = modelId;
            lbl_1_bss_148[groupIndex].items[itemIndex].status = itemIndex;
            lbl_1_bss_148[groupIndex].items[itemIndex].initialAlpha = 1.0f;

            for (modelIndex = 0; modelIndex < 8; modelIndex++) {
                modelId = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 44), HU_MEMNUM_OVL, HEAP_MODEL));
                Hu3DModelAttrSet(modelId, 1);
                ((M666SequenceModelView *)
                    (lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage +
                     modelIndex * 12))->modelId = modelId;
                ((M666SequenceModelView *)
                    (lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage +
                     modelIndex * 12))->active = 0;
                ((M666SequenceModelView *)
                    (lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage +
                     modelIndex * 12))->scale = ((M666SequenceModelView *)
                    (lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage +
                     modelIndex * 12))->alpha = 0.0f;
            }
        }
    }
}

/* Activates the first unused lane particle at a randomized side offset with scale and opacity set
 * to one. */
void fn_1_A430(s16 groupIndex, s16 itemIndex) {
    Point3d position;
    f32 groupOffset;
    s32 modelIndex;

    for (modelIndex = 0; modelIndex < 8; modelIndex++) {
        if (((M666SequenceModelView *)
                  lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
                .active != 0) {
            continue;
        }

        Hu3DModelPosGet(lbl_1_bss_148[groupIndex].items[itemIndex].rootModelId,
                        &position);
        if (groupIndex == 0) {
            groupOffset = 200.0f;
        } else {
            groupOffset = -200.0f;
        }
        position.x += groupOffset;
        position.x += -30.0f + (f32)frandmod(60);
        position.y += -30.0f + (f32)frandmod(60);
        position.z += -30.0f + (f32)frandmod(60);
        Hu3DModelPosSetV(
            ((M666SequenceModelView *)
                  lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
                .modelId,
            &position);
        Hu3DModelAttrReset(
            ((M666SequenceModelView *)
                  lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
                .modelId,
            1);
        Hu3DModelTPLvlSet(
            ((M666SequenceModelView *)
                  lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
                .modelId,
            1.0f);
        Hu3DModelScaleSet(
            ((M666SequenceModelView *)
                  lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
                .modelId,
            1.0f, 1.0f, 1.0f);
        ((M666SequenceModelView *)
              lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
            .active = 1;
        ((M666SequenceModelView *)
              lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
            .alpha = 1.0f;
        ((M666SequenceModelView *)
              lbl_1_bss_148[groupIndex].items[itemIndex].particleStorage)[modelIndex]
            .scale = 1.0f;
        return;
    }
}

/* Advances the 3-by-3 lane models through their movement states and updates fading particles. */
void fn_1_A798(void) {
    M666SequenceLane *lane;
    s32 groupIndex;
    M666SequenceParticle *particle;
    Point3d position;
    s32 laneIndex;
    s32 loopIndex;

    for (groupIndex = 0; groupIndex < 9; groupIndex++) {
        lbl_1_bss_EA0[groupIndex] = 0;
    }
    for (groupIndex = 0; groupIndex < 3; groupIndex++) {
        for (laneIndex = 0; laneIndex < 3; laneIndex++) {
            lane = &lbl_1_bss_148[groupIndex].items[laneIndex];
            Hu3DModelPosGet(lane->rootModelId, &position);
            switch (lane->state) {
            case 1:
                if (fabs(position.x) != lbl_1_data_830[lane->laneIndex]) {
                    lane->state = 2;
                }
                break;
            case 2:
                Hu3DModelTPLvlSet(lane->rootModelId, 1.0f);
                if (fabs(position.x) > lbl_1_data_830[lane->laneIndex]) {
                    position.x += groupIndex == 0 ? -5.0f : 5.0f;
                } else {
                    position.x = groupIndex == 0 ?
                                 lbl_1_data_830[lane->laneIndex] :
                                 -lbl_1_data_830[lane->laneIndex];
                    lane->state = 1;
                }
                break;
            case 3:
                if (lane->movement < 13.333333f) {
                    lane->movement += 0.6666667f;
                }
                if (groupIndex == 0) {
                    position.x -= lane->movement;
                } else {
                    position.x += lane->movement;
                }
                 if (fabs(position.x) < 300.0) {
                    for (loopIndex = 0; loopIndex < 3; loopIndex++) {
                        if (loopIndex != laneIndex) {
                            lbl_1_bss_148[groupIndex].items[loopIndex].laneIndex--;
                        }
                    }
                    HuAudFXPlay(MSM_SE_M666_LANE_CENTER_CROSSING);
                    lane->state = 4;
                }
                break;
            case 4:
                if (lane->movement < 13.333333f) {
                    lane->movement += 0.6666667f;
                }
                if (groupIndex == 0) {
                    position.x -= lane->movement;
                } else {
                    position.x += lane->movement;
                }
                if (fabs(position.x) > 2000.0) {
                    lane->movement = 0.0f;
                    lane->state = 2;
                    position.x = groupIndex == 0 ?
                                 lbl_1_data_830[2] : -lbl_1_data_830[2];
                }
                break;
            }

            Hu3DModelPosSetV(
                lbl_1_bss_148[groupIndex].items[laneIndex].rootModelId,
                &position);
            if (lane->state == 3 || lane->state == 4) {
                if (MgSeqModeGet() >= 6U) {
                    lane->alpha -= 0.02f;
                    if (lane->alpha < 0.0f) {
                        lane->alpha = 0.0f;
                    }
                    Hu3DModelTPLvlSet(lane->rootModelId, lane->alpha);
                } else {
                    if (lane->timer++ > 12) {
                        lane->timer = 0;
                        m666ActivateLaneModel(groupIndex, laneIndex);
                    }
                }
                switch (groupIndex) {
                    case 0:
                        if (!(position.x <= -500.0f)) {
                            if (position.x <= -300.0f) {
                                lbl_1_bss_EA0[3] = 1;
                            } else if (position.x <= -150.0f) {
                                lbl_1_bss_EA0[3] = lbl_1_bss_EA0[4] = 1;
                            } else {
                                lbl_1_bss_EA0[3] = lbl_1_bss_EA0[4] =
                                    lbl_1_bss_EA0[5] = 1;
                            }
                        }
                        break;
                    case 1:
                        if (!(position.x >= 500.0f)) {
                            if (position.x >= 300.0f) {
                                lbl_1_bss_EA0[8] = 1;
                            } else if (position.x >= 150.0f) {
                                lbl_1_bss_EA0[8] = lbl_1_bss_EA0[7] = 1;
                            } else {
                                lbl_1_bss_EA0[8] = lbl_1_bss_EA0[7] =
                                    lbl_1_bss_EA0[6] = 1;
                            }
                        }
                        break;
                    case 2:
                        if (!(position.x >= 500.0f)) {
                            if (position.x >= 300.0f) {
                                lbl_1_bss_EA0[2] = 1;
                            } else if (position.x >= 150.0f) {
                                lbl_1_bss_EA0[2] = lbl_1_bss_EA0[1] = 1;
                            } else {
                                lbl_1_bss_EA0[2] = lbl_1_bss_EA0[1] =
                                    lbl_1_bss_EA0[0] = 1;
                            }
                        }
                        break;
                }
            }
            for (loopIndex = 0; loopIndex < 8; loopIndex++) {
                particle = &lbl_1_bss_148[groupIndex].items[laneIndex].particles[loopIndex];
                if (particle->active != 0) {
                    particle->scale = (f32)((f64)particle->scale +
                                     (0.01 + (f64)((f32)frandmod(100) /
                                                   10000.0f)));
                    particle->alpha -= 0.01;
                    if (particle->alpha <= 0.0f) {
                        particle->alpha = 0.0f;
                        particle->active = 0;
                        Hu3DModelAttrSet(particle->modelId, 1U);
                    }
                    Hu3DModelTPLvlSet(particle->modelId, particle->alpha);
                    Hu3DModelScaleSet(particle->modelId, particle->scale,
                                      particle->scale, particle->scale);
                }
            }
        }
    }
}

/* Sets every lane item in the 3-by-3 grid to state 2. */
void fn_1_B30C(void) {
    s32 groupIndex;
    s32 itemIndex;
    groupIndex = 0;
    while (groupIndex < 3)
    {
        itemIndex = 0;
        while (itemIndex < 3)
        {
            lbl_1_bss_148[groupIndex].items[itemIndex].state = 2;
            itemIndex += 1;
        }
        groupIndex += 1;
    }
}

/* Starts the first lane with queue position zero when it is ready, with rumble and sound. */
void fn_1_B370(s16 groupIndex) {
    s32 markerIndex;
    s32 playerIndex;

    for (markerIndex = 0; markerIndex < 3; markerIndex++) {
        if (lbl_1_bss_148[groupIndex].items[markerIndex].status == 0) {
            break;
        }
    }
    if (markerIndex != 3 &&
        lbl_1_bss_148[groupIndex].items[markerIndex].state == 1) {
        lbl_1_bss_148[groupIndex].items[markerIndex].status = 2;
        lbl_1_bss_148[groupIndex].items[markerIndex].state = 3;
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
                omVibrate(lbl_1_bss_8[playerIndex].playerNumber, 20, 7, 3);
            }
        }
        if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
            omVibrate(lbl_1_bss_98.playerIndex, 20, 7, 3);
        }
        HuAudFXPlay(MSM_SE_M666_LANE_ACTIVATION);
    }
}

/* Unused entry point: initializes only local values and leaves game state unchanged. */
void fn_1_B4F4(s16 firstIndex, s16 secondIndex) {
    s32 localValue[2];
    s32 zeroValueB;
    s32 zeroValueA;
    s32 count;
    s32 combinedIndex;
    f32 initialFloatA;
    f32 initialFloatB;

    initialFloatB = 100.0f;
    initialFloatA = 50.0f;
    combinedIndex = secondIndex + ((firstIndex * 3) + 100);
    count = 3;
    zeroValueB = 0;
    zeroValueA = 0;
    localValue[1] = 0;
}

void fn_1_B558(void) {
}

/* Creates three hidden falling models and their hidden impact-animation models. */
void fn_1_B55C(void) {
    s32 fallingModelIndex;
    s16 baseModel;
    s16 model;

    for (fallingModelIndex = 0; fallingModelIndex < 3; fallingModelIndex++) {
        if (fallingModelIndex == 0) {
            baseModel = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 41), HU_MEMNUM_OVL, HEAP_MODEL));
            model = baseModel;
        } else {
            model = Hu3DModelLink(baseModel);
        }
        Hu3DModelShadowSet(model);
        Hu3DModelAttrSet(model, 1);
        lbl_1_bss_E3C[fallingModelIndex].modelIds[0] = model;

        model = Hu3DModelCreate(
        HuDataSelHeapReadNum(DATANUM(DATA_m666, 42), HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelAttrSet(model, 1);
        Hu3DMotionSpeedSet(model, 0.0f);
        lbl_1_bss_E3C[fallingModelIndex].modelIds[1] = model;
    }
}

/* Creates a collision actor for a falling model only during gameplay, once its bounce settles. */
void fn_1_B654(s16 fallingModelIndex) {
    MGACTOR_PARAM actorParam;
    Point3d modelPosition;

    if (lbl_1_bss_E3C[fallingModelIndex].actor) {
        return;
    }
    if (MgSeqModeGet() != 5) {
        return;
    }

    Hu3DModelPosGet(lbl_1_bss_E3C[fallingModelIndex].modelIds[0], &modelPosition);
    actorParam.height = 100.0f;
    actorParam.radius = 120.0f;
    actorParam.param = fallingModelIndex + 109;
    actorParam.type = 4;
    actorParam.attr = 0;
    actorParam.narrowHook = NULL;
    actorParam.correctHook = NULL;
    lbl_1_bss_E3C[fallingModelIndex].actor =
        MgActorCreate(&actorParam, lbl_1_bss_E3C[fallingModelIndex].modelIds[0]);
    MgActorPosSet(lbl_1_bss_E3C[fallingModelIndex].actor, &modelPosition);
    MgActorGravitySet(lbl_1_bss_E3C[fallingModelIndex].actor, 0.0f);
}

/* Saves the collision actor's position, kills and clears the actor, then copies the saved position
 * to its model. */
void fn_1_B794(s16 fallingModelIndex) {
    Point3d actorPosition;

    if (lbl_1_bss_E3C[fallingModelIndex].actor != 0) {
        MgActorPosGet(lbl_1_bss_E3C[fallingModelIndex].actor, &actorPosition);
        MgActorKill(lbl_1_bss_E3C[fallingModelIndex].actor);
        lbl_1_bss_E3C[fallingModelIndex].actor = 0;
        Hu3DModelPosSetV(lbl_1_bss_E3C[fallingModelIndex].modelIds[0], &actorPosition);
    }
}

/* Updates the falling and rising models, their collision actors and impact effects, and resets
 * obstacles they overlap. */
void fn_1_B854(void) {
    Point3d modelPosition;
    Point3d otherPosition;
    Point3d resetPosition;
    Point3d soundPosition;
    Point3d pointPosition;
    s16 mode;
    s32 fallingModelIndex;
    s32 otherGroup;
    s32 entryIndex;
    s32 vibrationIndex;
    s32 openingIndex;
    s32 pointIndex;
    f32 distanceSquared;
    f32 impactVerticalStep;
    f32 magnitude;
    f32 absoluteVerticalStep;
    M666ActorWork *actorWork;
    M666PerimeterMotion *perimeterMotion;

    mode = MgSeqModeGet();
    for (fallingModelIndex = 0; fallingModelIndex < 3; fallingModelIndex++) {
        actorWork = &lbl_1_bss_E3C[fallingModelIndex];
        if (actorWork->state == 0) {
            continue;
        }
        Hu3DModelPosGet(actorWork->modelIds[0], &modelPosition);
        switch (actorWork->state) {
        case 1:
            if (actorWork->countdown > 0) {
                if (mode != 5) {
                    actorWork->state = 0;
                }
                actorWork->countdown -= 1;
                continue;
            }
            if (mode != 5 && actorWork->motionState == 0) {
                actorWork->opacity = (f32)((f64)actorWork->opacity - 0.1);
                if (actorWork->opacity < 0.0f) {
                    actorWork->opacity = 0.0f;
                }
                Hu3DModelTPLvlSet(actorWork->modelIds[1], actorWork->opacity);
            }
            Hu3DModelAttrReset(actorWork->modelIds[0], 1);
            Hu3DModelTPLvlSet(actorWork->modelIds[0], actorWork->opacity);
                actorWork->verticalStep += 2.5f;
                modelPosition.y -= actorWork->verticalStep;
            if (modelPosition.y <= 0.0f) {
                if (actorWork->impactState == 0) {
                    HuAudFXPlayPan(MSM_SE_GUIDE_15, (s16)(64.0 - 32.0 *
                        sin(3.141592653589793 * (f64)modelPosition.x / 180.0)));
                    HuAudFXPlayPan(MSM_SE_GUIDE_14, (s16)(64.0 - 32.0 *
                        sin(3.141592653589793 * (f64)modelPosition.x / 180.0)));
                    Hu3DModelAttrReset(actorWork->modelIds[1], 1);
                    Hu3DMotionTimeSet(actorWork->modelIds[1], 0.0f);
                    Hu3DMotionSpeedSet(actorWork->modelIds[1], 1.0f);
                    for (vibrationIndex = 0; vibrationIndex < 3; vibrationIndex++) {
                        if (lbl_1_bss_8[vibrationIndex].motionSelection == 0) {
                            omVibrate(lbl_1_bss_8[vibrationIndex].playerNumber,
                                      20, 7, 3);
                        }
                    }
                    if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
                        omVibrate(lbl_1_bss_98.playerIndex, 20, 7, 3);
                    }
                    actorWork->impactState = 1;
                }
                modelPosition.y = 0.0f;
                actorWork->verticalStep *= -0.2;
                impactVerticalStep = actorWork->verticalStep;
                magnitude = __fabsf(impactVerticalStep);
                absoluteVerticalStep = magnitude;
                if (absoluteVerticalStep < 1.0) {
                    fn_1_B654((s16)fallingModelIndex);
                    actorWork->state = 2;
                    if (fallingModelIndex < 2) {
                        lbl_1_bss_E3C[fallingModelIndex + 1].state = 1;
                        lbl_1_bss_E3C[fallingModelIndex + 1].countdown =
                            frandmod(15) + 5;
                    } else {
                        for (otherGroup = 0; otherGroup < 3; otherGroup++) {
                            lbl_1_bss_E3C[otherGroup].state = 3;
                            if (MgSeqModeGet() == 5) {
                                lbl_1_bss_E3C[otherGroup].countdown = 60;
                            }
                        }
                    }
                }
            }
            if (modelPosition.y <= 100.0f) {
                for (otherGroup = 0; otherGroup < 6; otherGroup++) {
                    for (entryIndex = 0; entryIndex < 8; entryIndex++) {
                if ((lbl_1_bss_604[otherGroup][entryIndex].state == 4 ||
                     lbl_1_bss_604[otherGroup][entryIndex].state == 5) &&
                            lbl_1_bss_604[otherGroup][entryIndex].actor) {
                            MgActorPosGet(
                                lbl_1_bss_604[otherGroup][entryIndex].actor,
                                &otherPosition);
                            if ((50.0f + otherPosition.x > modelPosition.x - 100.0f) &&
                                (otherPosition.x - 50.0f < modelPosition.x + 100.0f) &&
                                (50.0f + otherPosition.z > modelPosition.z - 100.0f) &&
                                (otherPosition.z - 50.0f < modelPosition.z + 100.0f)) {
                                fn_1_901C(otherGroup, entryIndex);
                            }
                        }
                    }
                }
            }
            break;
        case 2:
            if (mode != 5) {
                actorWork->state = 3;
            }
            break;
        case 3:
            if (actorWork->countdown > 0) {
                actorWork->countdown -= 1;
                continue;
            }
            if (lbl_1_bss_E3C[fallingModelIndex].actor) {
                MgActorPosGet(lbl_1_bss_E3C[fallingModelIndex].actor, &modelPosition);
            }
            modelPosition.y += 33.333332f;
            if (actorWork->opacity >= 1.0f - modelPosition.y / 1000.0f) {
                actorWork->opacity = 1.0f - modelPosition.y / 1000.0f;
            } else {
                actorWork->opacity = (f32)((f64)actorWork->opacity - 0.1);
            }
            if (actorWork->opacity < 0.0f) {
                actorWork->opacity = 0.0f;
            }
            Hu3DModelTPLvlSet(actorWork->modelIds[0], actorWork->opacity);
            if (modelPosition.y > 1000.0f) {
                fn_1_B794((s16)fallingModelIndex);
                Hu3DModelAttrSet(actorWork->modelIds[1], 1);
                Hu3DModelAttrSet(actorWork->modelIds[0], 1);
                actorWork->state = 0;
                    fn_1_7A64();
                lbl_1_bss_E38 = 0;
                actorWork->impactState = 0;
            }
            if (modelPosition.y > 100.0f) {
                for (otherGroup = 0; otherGroup < 3; otherGroup++) {
                    if (lbl_1_bss_8[otherGroup].motionSelection == 1) {
                        lbl_1_bss_8[otherGroup].motionSelection = 2;
                        /* Restarts the sine-displacement phase used by fn_1_3C9C in motion state
                         * 2. */
                        lbl_1_bss_8[otherGroup].displacementPhase = 0;
                        HuAudFXPlay(MSM_SE_M666_PLAYER_RELEASE);
                    }
                }
            }
            if (lbl_1_bss_E3C[fallingModelIndex].actor) {
                MgActorPosSetRaw(lbl_1_bss_E3C[fallingModelIndex].actor,
                                 &modelPosition);
            }
            break;
        default:
            break;
        }
        Hu3DModelPosSetV(actorWork->modelIds[0], &modelPosition);
    }
}

typedef struct {
    Vec points[9];
} M666SpawnPointTable;

const M666SpawnPointTable lbl_1_rodata_2AC = { { { -300.0f, 1000.0f, -300.0f },
                                                 { 0.0f, 1000.0f, -300.0f },
                                                 { 300.0f, 1000.0f, -300.0f },
                                                 { -300.0f, 1000.0f, 0.0f },
                                                 { 0.0f, 1000.0f, 0.0f },
                                                 { 300.0f, 1000.0f, 0.0f },
                                                 { -300.0f, 1000.0f, 300.0f },
                                                 { 0.0f, 1000.0f, 300.0f },
                                                 { 300.0f, 1000.0f, 300.0f } } };

/* Starts three staggered falling-model arrivals at distinct grid points when fn_1_4D20 selects
 * event code 10. */
void fn_1_C630(void) {
    M666SpawnPointTable spawnPoints;
    Vec modelPosition;
    M666PerimeterMotionWork *perimeterMotion;
    s32 spawnIndex;
    s32 candidateIndex;
    s16 chosenPoint;
    s32 spriteIndex;
    s32 perimeterIndex;

    spawnPoints = lbl_1_rodata_2AC;
    if (lbl_1_bss_E3C[0].state != 0) {
        return;
    }

    for (perimeterIndex = 0; perimeterIndex < 3; perimeterIndex++) {
        perimeterMotion = &lbl_1_bss_D84[perimeterIndex];
        perimeterMotion->motionState = 5;
        Hu3DMotionShiftSet(perimeterMotion->modelId,
                           perimeterMotion->jointMotionIds[5],
                           0.0f, 8.0f, HU3D_MOTATTR_LOOP);
        for (spriteIndex = 0; spriteIndex < 10; spriteIndex++) {
            HuSprAttrSet(perimeterMotion->spriteGroupId, (s16)spriteIndex, 4);
        }
    }

    lbl_1_bss_E3C[0].state = 1;
    lbl_1_bss_E3C[0].countdown = 60;
    for (spawnIndex = 0; spawnIndex < 3; spawnIndex++) {
        do {
            chosenPoint = frandmod(9);
            if (spawnIndex != 0) {
                for (candidateIndex = 0; candidateIndex < spawnIndex;
                     candidateIndex++) {
                    if (chosenPoint == lbl_1_bss_E3C[candidateIndex].pointIndex) {
                        break;
                    }
                }
                if (candidateIndex != spawnIndex) {
                    continue;
                }
            }
            break;
        } while (1);

        lbl_1_bss_E3C[spawnIndex].pointIndex = chosenPoint;
        modelPosition.x = spawnPoints.points[chosenPoint].x;
        modelPosition.y = 1000.0f;
        modelPosition.z = spawnPoints.points[chosenPoint].z;

        modelPosition.x += -50.0f + (f32)frandmod(100);
        modelPosition.z += -50.0f + (f32)frandmod(100);

        if (modelPosition.x < -300.0f) {
            modelPosition.x = -300.0f;
        }
        if (modelPosition.x > 300.0f) {
            modelPosition.x = 300.0f;
        }
        if (modelPosition.z < -300.0f) {
            modelPosition.z = -300.0f;
        }
        if (modelPosition.z > 300.0f) {
            modelPosition.z = 300.0f;
        }

        Hu3DModelPosSetV(lbl_1_bss_E3C[spawnIndex].modelIds[0], &modelPosition);
        Hu3DModelPosSet(lbl_1_bss_E3C[spawnIndex].modelIds[1], modelPosition.x,
                        0.0f, modelPosition.z);
        lbl_1_bss_E3C[spawnIndex].opacity = 1.0f;
        lbl_1_bss_E3C[spawnIndex].motionState = 0;
    }

    lbl_1_bss_E38 = 1;
    for (spawnIndex = 0; spawnIndex < 9; spawnIndex++) {
        lbl_1_bss_DC[spawnIndex].state = 0;
        if (lbl_1_bss_DC[spawnIndex].soundHandle != -1) {
            HuAudFXStop(lbl_1_bss_DC[spawnIndex].soundHandle);
            lbl_1_bss_DC[spawnIndex].soundHandle = -1;
        }
    }
}

/* Activates the next marker and rumbles active players when fn_1_CBE8 reaches a marker slot. */
static inline void activateSequenceMarker(s16 groupIndex) {
    s32 markerIndex;
    s32 playerIndex;

    for (markerIndex = 0; markerIndex < 3; markerIndex++) {
        if (lbl_1_bss_148[groupIndex].items[markerIndex].status == 0) {
            break;
        }
    }
    if (markerIndex != 3 &&
        lbl_1_bss_148[groupIndex].items[markerIndex].state == 1) {
        lbl_1_bss_148[groupIndex].items[markerIndex].status = 2;
        lbl_1_bss_148[groupIndex].items[markerIndex].state = 3;
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
                omVibrate(lbl_1_bss_8[playerIndex].playerNumber, 20, 7, 3);
            }
        }
        if (HuMCProbe(0) != 0 && HuMCProbe(1) != 0) {
            omVibrate(lbl_1_bss_98.playerIndex, 20, 7, 3);
        }
        HuAudFXPlay(MSM_SE_M666_LANE_ACTIVATION);
    }
}

/* Attaches the queued obstacle model to its group's bomb hook when fn_1_CBE8 reaches a bomb
 * target point. */
static inline void hookBombModel(s16 groupIndex) {
    void *model;

    if (lbl_1_bss_580[groupIndex].data.motionEvent.state == 0) {
        lbl_1_bss_580[groupIndex].data.motionEvent.state = 1;
        model = &lbl_1_bss_604[groupIndex]
                              [lbl_1_bss_580[groupIndex].data.motionEvent.obstacleIndices[0]];
        Hu3DModelHookSet(lbl_1_bss_580[groupIndex].modelId, lbl_1_data_7F2,
                        ((M666MovingObstacle *)model)->modelId);
        Hu3DModelPosSet(((M666MovingObstacle *)model)->modelId,
                        0.0f, 0.0f, 0.0f);
        Hu3DModelRotSet(((M666MovingObstacle *)model)->modelId,
                        0.0f, 0.0f, 0.0f);
        Hu3DMotionSpeedSet(lbl_1_bss_580[groupIndex].modelId, 1.0f);
        Hu3DMotionTimeSet(lbl_1_bss_580[groupIndex].modelId, 0.0f);
        Hu3DModelAttrReset(lbl_1_bss_580[groupIndex].modelId, HU3D_MOTATTR_REV);
        HuAudFXPlay(MSM_SE_M666_BOMB_HOOK);
    }
}

/* Advances the nine event-marker rotations and stops each sound after its marker settles. */
void fn_1_CA2C(void) {
    Point3d modelRotation;
    s32 eventIndex;

    eventIndex = 0;
    while (eventIndex < 9) {
        Hu3DModelRotGet(lbl_1_bss_DC[eventIndex].modelId, &modelRotation);
        if (lbl_1_bss_DC[eventIndex].state == 1) {
            modelRotation.y += 5.0f;
        } else if (0.0f != modelRotation.y) {
            modelRotation.y += 5.0f;
        } else if ((s32)lbl_1_bss_DC[eventIndex].soundHandle != -1) {
            HuAudFXStop(lbl_1_bss_DC[eventIndex].soundHandle);
            lbl_1_bss_DC[eventIndex].soundHandle = -1;
        }
        while (modelRotation.y >= 360.0f) {
            modelRotation.y -= 360.0f;
        }
        Hu3DModelRotSetV(lbl_1_bss_DC[eventIndex].modelId, &modelRotation);
        eventIndex += 1;
    }
}

/* Returns zero for an inactive marker or one assigned to this perimeter model; returns one
 * when a different perimeter model owns the active marker. */
s16 fn_1_CB8C(s16 slotIndex, s16 perimeterIndex) {
    if (lbl_1_bss_DC[slotIndex].state == 0) {
        return 0;
    }
    if (perimeterIndex == lbl_1_bss_DC[slotIndex].perimeterIndex) {
        return 0;
    }
    return 1;
}

/* Dispatches a perimeter model's completed target event to an obstacle hook or lane crossing
 * when fn_1_77B0 finishes its target animation. */
void fn_1_CBE8(s16 targetPointIndex) {
    switch (targetPointIndex) {
    case 0:
        hookBombModel(0);
        break;
    case 1:
        hookBombModel(1);
        break;
    case 2:
        hookBombModel(2);
        break;
    case 3:
        activateSequenceMarker(0);
        break;
    case 4:
        hookBombModel(3);
        break;
    case 5:
        hookBombModel(4);
        break;
    case 6:
        hookBombModel(5);
        break;
    case 7:
        activateSequenceMarker(1);
        break;
    case 8:
        activateSequenceMarker(2);
        break;
    }
}

s16 print8(s16 x, s16 y, f32 scale, char *format, ...);

void fn_1_D620(OMOBJ *obj);

/* Runs each frame after fn_1_D794 registers it, advancing the minigame scene and target-marker
 * rotations. */
void fn_1_D620(OMOBJ *obj) {
    Point3d modelRotation;
    s32 eventIndex;

    fn_1_A798();
    fn_1_9260();
    fn_1_9F3C();
    fn_1_77B0();
    fn_1_B854();
    eventIndex = 0;
    while (eventIndex < 9) {
        Hu3DModelRotGet(lbl_1_bss_DC[eventIndex].modelId, &modelRotation);
        if (lbl_1_bss_DC[eventIndex].state == 1) {
            modelRotation.y += 5.0f;
        } else if (0.0f != modelRotation.y) {
            modelRotation.y += 5.0f;
        } else if ((s32) lbl_1_bss_DC[eventIndex].soundHandle != -1) {
            HuAudFXStop(lbl_1_bss_DC[eventIndex].soundHandle);
            lbl_1_bss_DC[eventIndex].soundHandle = -1;
        }
        while (modelRotation.y >= 360.0f) {
            modelRotation.y -= 360.0f;
        }
        Hu3DModelRotSetV(lbl_1_bss_DC[eventIndex].modelId, &modelRotation);
        eventIndex += 1;
    }
}

/* Registers the per-frame scene update callback with the minigame object manager. */
void fn_1_D794(void) {
    OMOBJ *openingUpdateObj;

    openingUpdateObj = omAddObjEx(lbl_1_bss_4, 80, 0U, 0U, -1, fn_1_D620);
}

/* Returns whether all three players have left active play; on success selects the microphone-
 * player result before fn_1_1D0 advances the sequence. */
s32 fn_1_D7DC(void) {
    s32 playerIndex;

    playerIndex = 0;
    while (playerIndex < 3) {
        if (lbl_1_bss_8[playerIndex].motionSelection == 0) {
            return 0;
        }
        playerIndex += 1;
    }
    lbl_1_bss_DF0.microphonePlayerWon = 1;
    return 1;
}

/* Applies controller 0's debug-camera inputs while the camera-control chord is held. */
void fn_1_D83C(void) {
    if ((HuPadBtn[0] & 0x4000) != 0) {
        if ((HuPadBtn[0] & 0x400) != 0) {
            CZoom += 5.0f;
        }
        if ((HuPadBtn[0] & 0x800) != 0) {
            CZoom -= 5.0f;
        }
        if (HuPadSubStkY[0] > 32) {
            CRot.x += 1.0f;
        }
        if (HuPadSubStkY[0] < -32) {
            CRot.x -= 1.0f;
        }
        if (HuPadSubStkX[0] > 32) {
            CRot.y += 1.0f;
        }
        if (HuPadSubStkX[0] < -32) {
            CRot.y -= 1.0f;
        }
        if ((HuPadStkX[0] > 5) || (HuPadStkX[0] < -5)) {
            Center.x += HuPadStkX[0];
        }
        if ((HuPadStkY[0] > 5) || (HuPadStkY[0] < -5)) {
            Center.z -= HuPadStkY[0];
        }
        if ((HuPadBtn[0] & 8) != 0) {
            Center.y += 5.0f;
        }
        if ((HuPadBtn[0] & 4) != 0) {
            Center.y -= 5.0f;
        }
        print8(100, 300, 1.8f, "CnrX:%f", Center.x);
        print8(100, 316, 1.8f, "CnrY:%f", Center.y);
        print8(100, 332, 1.8f, "CnrZ:%f", Center.z);
        /* The first RotZ label displays the X rotation. */
        print8(100, 348, 1.8f, "RotZ:%f", CRot.x);
        print8(100, 364, 1.8f, "RotY:%f", CRot.y);
        print8(100, 380, 1.8f, "RotZ:%f", CRot.z);
        print8(100, 396, 1.8f, "Zoom:%f", CZoom);
    }
}
