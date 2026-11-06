/* Word Herd player groups, herd movement, stage actors and microphone response handling. */
#include "game/charman.h"
#include "REL/m669DLL/vector.h"
#include "game/mic.h"
#include "datadir_enum.h"
#include "messdir_enum.h"

#include "include/dolphin/types.h"

#include "game/gamework.h"

#include "game/object.h"

#include "game/hu3d.h"

#include "game/mg/actman.h"

#include "game/audio.h"

#include "game/gamemes.h"

#include "game/mg/seqman.h"

#include "game/data.h"

#include "dolphin/gx.h"
#include "math.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "game/main.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "include/game/audio.h"
#include "include/game/data.h"
#include "include/game/memory.h"
#include "include/datadir_enum.h"
#include "include/game/frand.h"
#include "include/string.h"
#include <math.h>
#include "include/game/gamework.h"
#include "include/game/gamemes.h"
#include "dolphin.h"
#include "humath.h"
#include "game/window.h"
#include "game/process.h"
#include "game/flag.h"
#include "game/armem.h"
#include "game/mgdata.h"
#include "game/printfunc.h"
#include "game/saveload.h"
#include "game/esprite.h"
#include "game/thpmain.h"
#include "game/board/main.h"
#include "game/board/audio.h"
#include "game/board/masu.h"
#include "game/board/player.h"
#include <string.h>

/* All bits set in the recognition slot means that no word is pending. */
#define M669_NO_WORD ((u16)-1)

extern HUPROCESS *lbl_1_bss_0;

const HuVecF lbl_1_rodata_90[16] = {
    { -600.0f, 0.0f, -600.0f },
    { -200.0f, 0.0f, -600.0f },
    { 200.0f, 0.0f, -600.0f },
    { 600.0f, 0.0f, -600.0f },
    { -600.0f, 0.0f, -200.0f },
    { -200.0f, 0.0f, -200.0f },
    { 200.0f, 0.0f, -200.0f },
    { 600.0f, 0.0f, -200.0f },
    { -600.0f, 0.0f, 200.0f },
    { -200.0f, 0.0f, 200.0f },
    { 200.0f, 0.0f, 200.0f },
    { 600.0f, 0.0f, 200.0f },
    { -600.0f, 0.0f, 600.0f },
    { -200.0f, 0.0f, 600.0f },
    { 200.0f, 0.0f, 600.0f },
    { 600.0f, 0.0f, 600.0f }
};

const HuVecF lbl_1_rodata_150[36] = {
    { -375.0f, 0.0f, -600.0f },
    { -225.0f, 0.0f, -600.0f },
    { -75.0f, 0.0f, -600.0f },
    { 75.0f, 0.0f, -600.0f },
    { 225.0f, 0.0f, -600.0f },
    { 375.0f, 0.0f, -600.0f },
    { -375.0f, 0.0f, -450.0f },
    { -225.0f, 0.0f, -450.0f },
    { -75.0f, 0.0f, -450.0f },
    { 75.0f, 0.0f, -450.0f },
    { 225.0f, 0.0f, -450.0f },
    { 375.0f, 0.0f, -450.0f },
    { -375.0f, 0.0f, -300.0f },
    { -225.0f, 0.0f, -300.0f },
    { -75.0f, 0.0f, -300.0f },
    { 75.0f, 0.0f, -300.0f },
    { 225.0f, 0.0f, -300.0f },
    { 375.0f, 0.0f, -300.0f },
    { -375.0f, 0.0f, -150.0f },
    { -225.0f, 0.0f, -150.0f },
    { -75.0f, 0.0f, -150.0f },
    { 75.0f, 0.0f, -150.0f },
    { 225.0f, 0.0f, -150.0f },
    { 375.0f, 0.0f, -150.0f },
    { -375.0f, 0.0f, 0.0f },
    { -225.0f, 0.0f, 0.0f },
    { -75.0f, 0.0f, 0.0f },
    { 75.0f, 0.0f, 0.0f },
    { 225.0f, 0.0f, 0.0f },
    { 375.0f, 0.0f, 0.0f },
    { -375.0f, 0.0f, 150.0f },
    { -225.0f, 0.0f, 150.0f },
    { -75.0f, 0.0f, 150.0f },
    { 75.0f, 0.0f, 150.0f },
    { 225.0f, 0.0f, 150.0f },
    { 375.0f, 0.0f, 150.0f }
};

s32 lbl_1_data_58[4] = {
    DATANUM(0x800000, 8), DATANUM(0x800000, 9),
    DATANUM(0x800000, 10), DATANUM(0x800000, 11)
};

s32 lbl_1_data_68[6] = {
    DATANUM(0x800000, 1), DATANUM(0x800000, 2), DATANUM(0x800000, 3),
    DATANUM(0x800000, 4), DATANUM(0x800000, 5), DATANUM(0x800000, 6),
};

unsigned int lbl_1_data_80[19] = {
    DATANUM(0x930000, 0), DATANUM(0x930000, 1),
    DATANUM(0x930000, 2), DATANUM(0x930000, 3),
    DATANUM(0x8e0000, 136), DATANUM(0x930000, 5),
    DATANUM(0x8e0000, 89), DATANUM(0x8e0000, 129),
    DATANUM(0x8e0000, 129), DATANUM(0x8e0000, 176),
    DATANUM(0x930000, 15), DATANUM(0x930000, 16),
    DATANUM(0x8e0000, 30), DATANUM(0x8e0000, 144),
    DATANUM(0x930000, 38), DATANUM(0x930000, 40),
    DATANUM(0x930000, 21), DATANUM(0x930000, 23), 0,
};

HuVecF lbl_1_data_CC[3] = {
    {-630.0f, 135.0f, -910.0f},
    {820.0f, 380.0f, -435.0f},
    {-550.0f, 380.0f, 1110.0f},
};

HuVecF lbl_1_data_F0[3] = { { -550.0f, 0.0f, 0.0f },
                            { 516.83f, 0.0f, 188.11f },
                            { 0.0f, 0.0f, 550.0f } };

HuVecF lbl_1_data_114[3] = { { -1200.0f, 0.0f, 0.0f },
                             { 1127.63f, 0.0f, 410.42f },
                             { 0.0f, 0.0f, 1200.0f } };

HuVecF lbl_1_data_138 = { 0.0f, 709.6183f, 1777.934f };

HuVecF lbl_1_data_144 = { 0.0f, 324.954f, 58.183f };

HuVecF lbl_1_data_150 = { 0.0f, 1579.4247f, 2891.4707f };

HuVecF lbl_1_data_15C = { 0.0f, 264.77008f, 121.68128f };

s16 lbl_1_data_168 = -1;

char lbl_1_data_16A[20] = "/mic/ctx/m669_words";

u16 lbl_1_data_17E = M669_NO_WORD;

typedef struct M669PlayerWorkHeader {
    s16 playerIndex; /* Index in GwPlayerConf. */
    s16 character; /* Character selected for this player. */
    s16 groupIndex; /* Zero for the microphone player, one for the three other players. */
    s16 positionIndex; /* Slot among the three players in group one. */
    MGPLAYER *player; /* Character actor and motion state. */
    s16 state; /* Player update phase; initialized to -1. */
    s16 timer; /* Reset during player initialization. */
} M669PlayerWorkHeader;

typedef struct M669PlayerSetupWork {
    M669PlayerWorkHeader header;
    s16 motion;
} M669PlayerSetupWork;

typedef struct M669ImpactWork {
    M669PlayerWorkHeader header;
    s16 phase;
    s16 motion;
    HuVecF velocity;
    s32 hitCounted;
} M669ImpactWork;

typedef struct M669ActorWork {
    s16 actorIndex; /* Herd slot; slot zero owns the models shared by the other 35 actors. */
    u16 action;
    HU3D_MODELID model;
    HU3D_MOTIONID motionIds[4];
    HuVecF position;
    HuVecF velocity;
    HuVecF frameDisplacement;
    HuVecF frameVelocity;
    HuVecF dampedVelocity;
    HuVecF gravity;
    f32 collisionWeight;
    f32 angle; /* Heading in radians. */
    f32 motionTime;
    s16 motion;
    s16 unidentifiedMotionValue;
    s32 frameCount;
    s32 timer;
    f32 motionSpeed;
    s32 ready;
    HU3D_MODELID particleModel;
    HU3D_MODELID motionModel;
    HU3D_MOTIONID motionDataId;
    s32 motionComplete;
    HU3D_MODELID positionModel;
    s16 unidentifiedState;
} M669ActorWork;

typedef struct M669MotionWork {
    s16 state; /* Callback phase: 0, 1, and 2 select the next stage handler. */
    s16 model;
    s16 motionIds[6];
    HuVecF position;
    s16 activationRequested;
    s16 motion;
    s32 frameCount;
    HuVecF velocity;
    struct M669FollowingEffectWork *secondaryWork;
} M669MotionWork;

typedef struct M669FollowingEffectWork {
    M669MotionWork *owner;
    s16 model;
    s16 state;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF targetPosition;
    HuVecF velocity;
    HuVecF frameVelocity;
    HuVecF frameDisplacement;
    s32 frameCount;
    s32 hitPlayer;
    s32 enteredArena;
} M669FollowingEffectWork;

typedef struct M669HerdWork {
    s16 counts[16];
    s16 radialCounts[2]; /* Inside and outside a 400-unit radius. */
    s16 zSideCounts[2]; /* Negative and nonnegative Z. */
    s16 xSideCounts[2]; /* Negative and nonnegative X. */
    HuVecF position;
    u32 sequence;
    ANIMDATA *animation;
} M669HerdWork;

typedef struct M669PlayerAreaWork {
    s16 playerCounts[16];
    s16 nearestPlayerIndices[16];
    s16 hitCount;
} M669PlayerAreaWork;

M669HerdWork *lbl_1_bss_CC;
M669ActorWork *lbl_1_bss_3C[36];
M669PlayerAreaWork *lbl_1_bss_38;
M669ImpactWork *lbl_1_bss_2C[3];
M669PlayerSetupWork *lbl_1_bss_28;

M669MotionWork *lbl_1_bss_1C[3];

M669PlayerWorkHeader *lbl_1_bss_C[4];

s16 lbl_1_bss_A;

s16 lbl_1_bss_8;

M669HerdWork *fn_1_4F88(void) {
    return lbl_1_bss_CC;
}

M669ActorWork *fn_1_4F98(s32 index) { return lbl_1_bss_3C[index]; }

s16 *fn_1_4FB0(void) {
    return lbl_1_bss_38->playerCounts;
}

M669ImpactWork *fn_1_4FC0(s32 index) { return lbl_1_bss_2C[index]; }

M669PlayerSetupWork *fn_1_4FD8(void) {
    return lbl_1_bss_28;
}

M669MotionWork *fn_1_4FE8(s32 index) { return lbl_1_bss_1C[index]; }

/* Selects the one-versus-three groups during minigame setup, defaulting to player zero alone. */
void fn_1_5000(s16 *groupOutput)
{
    s16 groupCounts[4] = {0, 0, 0, 0};
    s32 index;

    for (index = 0; index < 4; index++) {
        groupCounts[lbl_80265728[index].grpNo]++;
    }
    if (groupCounts[0] != 1) {
        groupOutput[0] = 0;
        groupOutput[1] = 1;
        groupOutput[2] = 1;
        groupOutput[3] = 1;
    } else {
        for (index = 0; index < 4; index++) {
            groupOutput[index] = lbl_80265728[index].grpNo;
        }
    }
}

extern void *fn_1_1A8(s32 priority, u32 bytes, void (*update)(OMOBJ *));

extern int lbl_1_data_28[2][6];

extern void fn_1_248(OMOBJ *object, OMOBJ_FUNC callback);

extern void fn_1_5554(OMOBJ *object);

extern void fn_1_59F4(OMOBJ *object);

extern void fn_1_5C74(OMOBJ *object);

extern void fn_1_5FA8(OMOBJ *object);

extern void fn_1_6CC4(OMOBJ *object);

extern void fn_1_7308(OMOBJ *object);

extern void fn_1_10C24(OMOBJ *object);

extern void fn_1_EA4C(OMOBJ *object);

typedef struct M669StageAudioWork {
    s32 startSequenceState; /* Cleared when the held start sequence enters mode two. */
    s16 streamHandle;
} M669StageAudioWork;

typedef struct M669VecBlock {
    HuVecF vector;
} M669VecBlock;

/* Initializes the minigame camera, shadow and player callback work. */
void fn_1_50D8(OMOBJ *object)
{
    s16 playerGroups[4];
    M669StageAudioWork *managerWork;
    s32 playerIndex;
    s32 activeIndex;
    M669PlayerSetupWork *selectedPlayer;

    managerWork = object->data;
    managerWork->streamHandle = -1;
    lbl_1_bss_A = 0;
    fn_1_5000(playerGroups);

    Hu3DCameraCreate(1);
    Hu3DCameraPerspectiveSet(1, 30.0f, 20.0f,
        15000.0f, 1.2f);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        640.0f, 480.0f, 0.0f,
        1.0f);
    {
    M669VecBlock shadowPosition = {{-100.0f, 4500.0f, -1500.0f}};
    M669VecBlock shadowUpDirection = {{0.0f, 1.0f, 0.5f}};
    M669VecBlock shadowTargetPosition = {{0.0f, 0.0f, 0.0f}};
    Hu3DShadowCreate(30.0f, 10.0f, 10000.0f);
    fn_8002AAB4(64U, 0U, 0U);
    Hu3DShadowPosSet(&shadowPosition.vector, &shadowUpDirection.vector,
        &shadowTargetPosition.vector);
    Hu3DShadowTPLvlSet(0.5f);

    fn_1_1A8(10, 12, fn_1_59F4);
    fn_1_1A8(10, 52, fn_1_5C74);
    lbl_1_bss_CC = fn_1_1A8(10, 64, fn_1_5FA8);
    fn_1_1A8(10, 16, fn_1_10C24);
    lbl_1_bss_38 = fn_1_1A8(10, 66, fn_1_6CC4);
    activeIndex = 0;
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        if (playerGroups[playerIndex] == 0) {
            lbl_1_bss_28 = fn_1_1A8(10, 20, fn_1_EA4C);
            lbl_1_bss_28->header.playerIndex = (s16)playerIndex;
            lbl_1_bss_28->header.groupIndex = 0;
            lbl_1_bss_C[playerIndex] = &lbl_1_bss_28->header;
        } else {
            lbl_1_bss_2C[activeIndex] =
                fn_1_1A8(10, 36, fn_1_7308);
            lbl_1_bss_2C[activeIndex]->header.playerIndex = (s16)playerIndex;
            lbl_1_bss_2C[activeIndex]->header.positionIndex = (s16)activeIndex;
            lbl_1_bss_2C[activeIndex]->header.groupIndex = 1;
            lbl_1_bss_C[playerIndex] = &lbl_1_bss_2C[activeIndex]->header;
            activeIndex++;
        }
    }
    fn_8005DDEC(7);
    lbl_1_bss_8 = HuMCContextCreate(lbl_1_data_16A);
    selectedPlayer = lbl_1_bss_28;
    HuMCSelWinItemSet(MESSNUM(MESS_MIC_MIN_665, 17), 5,
                      lbl_80265728[selectedPlayer->header.playerIndex].padNo);
    fn_1_248(object, fn_1_5554);
    }
}

void fn_1_55B4(OMOBJ *obj);

/* Holds the start sequence in mode two and installs the herd-readiness callback. */
void fn_1_5554(OMOBJ *obj)
{
    M669StageAudioWork *work = obj->data;

    if (fn_80071A8C() == 2) {
        fn_80071B08();
        work->startSequenceState = 0;
        fn_1_248(obj, fn_1_55B4);
        return;
    }
}

void fn_1_561C(OMOBJ *object);

/* Advances the held start sequence once the herd controller reaches state three. */
void fn_1_55B4(OMOBJ *object)
{
    s32 managerData;
    M669HerdWork *herdController;
    managerData = (s32)object->data;
    herdController = lbl_1_bss_CC;
    if (herdController->sequence == 3) {
        fn_80071A9C();
        fn_1_248(object, fn_1_561C);
        return;
    }
}

void fn_1_56A4(OMOBJ *object);

/* Starts stage music when the message allows it, then advances when play mode begins. */
void fn_1_561C(OMOBJ *object)
{
    M669StageAudioWork *work = object->data;

    if (work->streamHandle == -1 &&
        (GameMesStatGet(fn_80071DF4()) & 0x10) != 0) {
        work->streamHandle = HuAudSStreamPlay(MSM_STREAM_M666);
    }
    if (fn_80071A8C() == 5) {
        fn_1_248(object, fn_1_56A4);
        return;
    }
}

void fn_1_5758(OMOBJ *object);

/* Waits during play for both herd status fields to clear, or proceeds when finish mode begins. */
void fn_1_56A4(OMOBJ *object)
{
    void *work = object->data;

    if (fn_80071A8C() == 5) {
        M669HerdWork *player = lbl_1_bss_CC;

        if (player->radialCounts[0] == 0) {
            M669HerdWork *modePlayer = lbl_1_bss_CC;

            if (modePlayer->radialCounts[1] == 0) {
                fn_80071A9C();
                fn_1_248(object, fn_1_5758);
                return;
            }
        }
    }
    if (fn_80071A8C() == 6) {
        fn_1_248(object, fn_1_5758);
        return;
    }
}

extern void fn_1_5958(OMOBJ *);

/* After herd movement settles, selects the winning group, awards coins and fades the stage
 * music. */
void fn_1_5758(OMOBJ *object)
{
    M669StageAudioWork *work = object->data;
    s32 winners[4] = {-1, -1, -1, -1};
    s32 index;
    s32 winnerCount;
    s32 activeEntries;
    M669ActorWork *entry;
    M669ActorWork *stateEntry;
    M669ActorWork *nextStateEntry;

    winnerCount = 0;
    activeEntries = 0;
    for (index = 0; index < 36; index++) {
        entry = lbl_1_bss_3C[index];

        if (entry->action == 1) {
            return;
        }
    }
    for (index = 0; index < 36; index++) {
        stateEntry = lbl_1_bss_3C[index];
        if (stateEntry->action != 2) {
            nextStateEntry = lbl_1_bss_3C[index];
            if (nextStateEntry->action != 3) {
                activeEntries += 1;
            }
        }
    }
    if (activeEntries == 0) {
        lbl_1_data_168 = 1;
    } else {
        lbl_1_data_168 = 0;
    }
    for (index = 0; index < 4; index++) {
        if (lbl_1_data_168 == lbl_1_bss_C[index]->groupIndex) {
            winners[winnerCount] = lbl_1_bss_C[index]->character;
            if (!fn_8006A124(65551)) {
                lbl_80265750[index].mgCoinBonus = 10;
            }
            winnerCount++;
        }
    }
    fn_80071C18(winners[0], winners[1], winners[2], winners[3]);
    if (work->streamHandle != -1) {
        HuAudSStreamFadeOut(work->streamHandle, 100);
    }
    fn_1_248(object, fn_1_5958);
}

void fn_1_59E0(OMOBJ *object);

/* Advances the result sequence when all three player callbacks report completion. */
void fn_1_5958(OMOBJ *object)
{
    void *work = object->data;

    if (fn_80071A8C() == 7) {
        if (lbl_1_bss_A == 3) {
            fn_80071A9C();
            fn_1_248(object, fn_1_59E0);
            return;
        }
    } else if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_59E0);
        return;
    }
}

/* Leaves the stage manager idle after the result sequence has advanced. */
void fn_1_59E0(OMOBJ *object)
{
    void *work;

    work = object->data;
}

typedef struct M669SceneSetupWork {
    OMOBJ *scene;
    s32 soundHandle;
    s32 frameCount;
} M669SceneSetupWork;

typedef struct M669CameraWork {
    s16 state;
    HuVecF positionStep;
    HuVecF targetStep;
    HuVecF position;
    HuVecF targetPosition;
} M669CameraWork;

void fn_1_5BAC(OMOBJ *);

/* Creates the day or night scenery during setup, then waits to start its ambient sound. */
void fn_1_59F4(OMOBJ *object)
{
    M669SceneSetupWork *work = object->data;
    s16 night;
    s32 nightIndex;
    s32 modelIndex;
    s16 shadowNight;

    work->scene = fn_8003AB38(lbl_1_bss_0, 32730, 6, 0, -1, 0);
    for (modelIndex = 0; modelIndex < 6; modelIndex++) {
        night = lbl_802C0278;
        if (night == 0) {
            nightIndex = 0;
        } else {
            nightIndex = 1;
        }
        work->scene->mdlId[modelIndex] = fn_80025A04(
            fn_80008DD0(lbl_1_data_28[nightIndex][modelIndex],
                (s32)0x10000000, 2));
    }
    fn_80029850(work->scene->mdlId[0], 1);
    fn_80026B8C(work->scene->mdlId[0], 3);
    fn_80026B8C(work->scene->mdlId[2], 5);
    fn_80026B8C(work->scene->mdlId[3], 3);
    fn_80026B8C(work->scene->mdlId[4], 3);
    fn_80026FFC(work->scene->mdlId[0]);
    fn_80026FFC(work->scene->mdlId[3]);
    shadowNight = lbl_802C0278;
    if (shadowNight == 0) {
        fn_80026E44(work->scene->mdlId[2]);
    }
    fn_800269E8(work->scene->mdlId[5], (1 << 0));
    fn_800804E8(&work->scene->mdlId[5], 1, 128);
    fn_1_248(object, fn_1_5BAC);
}

void fn_1_12C58(OMOBJ *obj);
typedef struct M669Bounds {
    HuVecF min;
    HuVecF max;
} M669Bounds;

typedef struct M669Sphere {
    HuVecF center;
    f32 radius;
} M669Sphere;

extern f32 fn_1_2C24(M669Sphere *first, M669Sphere *second);
extern void fn_1_2504(M669Bounds *bounds, M669Sphere *sphere);
extern s32 fn_1_27FC(M669Bounds *first, M669Bounds *second);

void fn_1_5C14(OMOBJ *object);

/* Starts the scenery sound when the start sequence reaches mode two. */
void fn_1_5BAC(OMOBJ *object) {
    M669SceneSetupWork *data = object->data;

    if (fn_80071A8C() == 2) {
        data->soundHandle = HuAudFXPlay(MSM_SE_MG669_SCENERY_AMBIENCE);
        data->frameCount = 0;
        fn_1_248(object, fn_1_5C14);
        return;
    }
}

/* Stops the scenery sound after the post-incremented counter reaches 180, then disables itself. */
void fn_1_5C14(OMOBJ *object) {
    M669SceneSetupWork *work = object->data;

    if (work->frameCount++ >= 0xB4) {
        HuAudFXStop(work->soundHandle);
        fn_1_248(object, 0);
        return;
    }
}

extern void fn_1_5D34(OMOBJ *);

/* Sets the opening camera during setup and installs the herd-readiness wait. */
void fn_1_5C74(OMOBJ *object)
{
    M669CameraWork *work = object->data;
    HuVecF upDirection = { 0.0f, 1.0f, 0.0f };

    work->position = lbl_1_data_138;
    work->targetPosition = lbl_1_data_144;
    Hu3DCameraPosSetV(1, &work->position, &upDirection, &work->targetPosition);
    work->state = 0;
    fn_1_248(object, fn_1_5D34);
}

extern void fn_1_5E58(OMOBJ *);

static inline M669ActorWork *M669ActorWorkGet(s32 index)
{
    return lbl_1_bss_3C[index];
}

/* Starts the camera move when every herd actor is ready or the readiness counter exceeds 600. */
void fn_1_5D34(OMOBJ *object)
{
    M669CameraWork *work = object->data;
    s32 ready = 1;
    s32 index;
    for (index = 0; index < 36; index++) {
        if (M669ActorWorkGet(index)->ready == 0) {
            ready = 0;
            break;
        }
    }
    if (ready != 0 || work->state++ > 600) {
        fn_1_4F88()->sequence = 1;
        M669VecSubtract(&work->positionStep, &lbl_1_data_150, &lbl_1_data_138);
        M669VecScale(&work->positionStep, &work->positionStep, 0.025f);
        M669VecSubtract(&work->targetStep, &lbl_1_data_15C, &lbl_1_data_144);
        M669VecScale(&work->targetStep, &work->targetStep, 0.025f);
        work->state = 0;
        fn_1_248(object, fn_1_5E58);
        return;
    }
}

extern void fn_1_5F6C(OMOBJ *);

/* Moves the opening camera each update and snaps to its final pose at counter 40 or play mode. */
void fn_1_5E58(OMOBJ *object)
{
    M669CameraWork *work = object->data;
    HuVecF upDirection = { 0.0f, 1.0f, 0.0f };
    M669VecAdd(&work->position, &work->position, &work->positionStep);
    M669VecAdd(&work->targetPosition, &work->targetPosition, &work->targetStep);
    Hu3DCameraPosSetV(1, &work->position, &upDirection, &work->targetPosition);
    if (work->state++ >= 40 || fn_80071A8C() == 5) {
        work->position = lbl_1_data_150;
        work->targetPosition = lbl_1_data_15C;
        Hu3DCameraPosSetV(1, &work->position, &upDirection, &work->targetPosition);
        fn_1_248(object, fn_1_5F6C);
        return;
    }
}

/* Disables the camera callback after the opening move finishes. */
void fn_1_5F6C(OMOBJ *object)
{
    M669CameraWork *work = object->data;

    fn_1_248(object, NULL);
}

extern void fn_1_9144(OMOBJ *);

void fn_1_60A0(OMOBJ *object);

/* Loads the shared particle animation and creates all 36 herd actors during setup. */
void fn_1_5FA8(OMOBJ *object)
{
    M669HerdWork *work = object->data;
    s32 index;
    void *animationData;

    work->sequence = 0;
    for (index = 0; index < 16; index++) {
        work->counts[index] = 0;
    }
    animationData = fn_80008DD0(DATANUM(0x210000, 7), 0x10000000, 2);
    work->animation = HuSprAnimRead(animationData);
    for (index = 0; index < 36; index++) {
        lbl_1_bss_3C[index] = fn_1_1A8(20, 136, fn_1_9144);
        lbl_1_bss_3C[index]->actorIndex = index;
    }
    fn_1_248(object, fn_1_60A0);
}

static inline M669ActorWork *M669ParticleGet(s32 index)
{
    return lbl_1_bss_3C[index];
}

/* Computes the clamped sphere-distance term used by the herd collision helpers. */
static inline f32 fn_1_2CC8(const M669Sphere *first, const M669Sphere *second)
{
    HuVecF offset;
    f32 distanceSquared;

    M669VecSubtract(&offset, &first->center, &second->center);
    distanceSquared = M669VecLengthSquared(&offset);
    distanceSquared -= first->radius * first->radius +
        second->radius * second->radius;
    if (distanceSquared < 0.0f) distanceSquared = 0.0f;
    return sqrtf(distanceSquared);
}

/* Each herd-controller update accumulates movement, resolves overlaps and constrains the herd
 * area. */
void fn_1_60A0(OMOBJ *object)
{
    M669HerdWork * work = object->data;
    HuVecF response[36];
    HuVecF zero = { 0.0f, 0.0f, 0.0f };
    s32 i;
    s32 j;
    s32 k;
    s32 l;
    memset(response, 0, sizeof (response));
    for (i = 0; i < 36; i++)
    {
        M669ActorWork * particle = M669ParticleGet(i);

        M669VecAdd(& particle->velocity, & particle->velocity, & particle->frameVelocity);
        M669VecAdd(& particle->velocity, & particle->velocity, & particle->dampedVelocity);
        M669VecAdd(& particle->velocity, & particle->velocity, & particle->gravity);
        particle->frameDisplacement = particle->velocity;
        particle->velocity.x = particle->velocity.z = 0.0f;
        M669VecScale(& particle->dampedVelocity, & particle->dampedVelocity, 0.95f);
    }
    for (i = 0; i < 16; i++)
    {
        work->counts[i] = 0;
    }
    for (i = 0; i < 2; i++)
    {
        work->radialCounts[i] = 0;
        work->zSideCounts[i] = 0;
        work->xSideCounts[i] = 0;
    }
    fn_1_31C(& work->position, 0.0f, 0.0f, 0.0f);
    for (i = 0; i < 36; i++)
    {
        M669ActorWork * particle = M669ParticleGet(i);
        HuVecF direction;
        if (particle->action != 2 && particle->action != 3)
        {
            M669VecAdd(& work->position, & work->position, & particle->position);
            M669VecSubtract(& direction, & zero, & particle->position);
            if (fn_1_32C(& direction) < 400.0f)
            {
                work->radialCounts[0]++;
            }
            else
            {
                work->radialCounts[1]++;
            }
            if (particle->position.z < 0.0f)
            {
                work->zSideCounts[0]++;
            }
            else
            {
                work->zSideCounts[1]++;
            }
            if (particle->position.x < 0.0f)
            {
                work->xSideCounts[0]++;
            }
            else
            {
                work->xSideCounts[1]++;
            }
        }
    }
    if (work->radialCounts[0] + work->radialCounts[1] > 1)
    {
        M669VecScale(&work->position, &work->position,
                 1.0f / (f32) (work->radialCounts[0] + work->radialCounts[1]));
    }
    for (k = 0; k < 36; k++)
    {
        M669ActorWork * particle = M669ParticleGet(k);
        M669Sphere sphere;
        M669Bounds particleBounds;
        if (particle->action == 3)
        {
            continue;
        }
        sphere.center = particle->position;
        sphere.center.y += 50.0f;
        sphere.radius = 50.0f;
        fn_1_2504(& particleBounds, & sphere);
        if (particle->action != 2)
        {
            for (l = 0; l < 16; l++)
            {
                M669Bounds areaBounds;
                areaBounds.max = lbl_1_rodata_90[l];
                areaBounds.min = areaBounds.max;
                areaBounds.min.x -= 200.0f;
                areaBounds.min.z -= 200.0f;
                areaBounds.max.x += 200.0f;
                areaBounds.max.y += 200.0f;
                areaBounds.max.z += 200.0f;
                if (fn_1_27FC(& particleBounds, & areaBounds))
                {
                    work->counts[l]++;
                }
            }
        }
        for (j = 0; j < 36; j++)
        {
            M669ActorWork * b = M669ParticleGet(j);
            M669Sphere sphereB;
            M669Bounds boundsB;
            HuVecF away;
            f32 radiusSquared;
            f32 separation;
            f32 push;
            if (b->action == 3 || k == j)
            {
                continue;
            }
            sphereB.center = b->position;
            sphereB.center.y += 50.0f;
            sphereB.radius = 50.0f;
            fn_1_2504(& boundsB, & sphereB);
            if (! fn_1_27FC(& particleBounds, & boundsB))
            {
                continue;
            }
            radiusSquared = sphere.radius * sphere.radius + sphereB.radius * sphereB.radius;
            if (fn_1_2C24(& sphere, & sphereB) <= radiusSquared)
            {
                push =
                    0.9f * (b->collisionWeight / (b->collisionWeight + particle->collisionWeight));
                M669VecSubtract(& away, & sphere.center, & sphereB.center);
                separation = fn_1_554(& away, & away);
                M669VecScale(& away, & away,
                             push * ((sphere.radius + sphereB.radius) - separation));
                M669VecAdd(& response[k], & response[k], & away);
            }
        }
    }
    for (k = 0; k < 36; k++)
    {
        M669ActorWork * a = M669ParticleGet(k);
        M669Sphere sphereA;
        M669Bounds boundsA;
        sphereA.center = a->position;
        sphereA.center.y += 50.0f;
        sphereA.radius = 50.0f;
        fn_1_2504(& boundsA, & sphereA);
        for (j = 0; j < 3; j++)
        {
            M669ImpactWork * target = fn_1_4FC0(j);
            M669Sphere targetSphere;
            M669Bounds targetBounds;
            HuVecF away;
            fn_80086BC8(fn_1_4FC0(j)->header.player->actor, & targetSphere.center);
            targetSphere.center.y += 40.0f;
            targetSphere.radius = 40.0f;
            fn_1_2504(& targetBounds, & targetSphere);
            if (fn_1_27FC(& boundsA, & targetBounds) != 0)
            {
                f32 radiusSquared =
                    sphereA.radius * sphereA.radius + targetSphere.radius * targetSphere.radius;
                if (fn_1_2C24(& sphereA, & targetSphere) <= radiusSquared)
                {
                    f32 separation;
                    M669VecSubtract(& away, & sphereA.center, & targetSphere.center);
                    separation = fn_1_554(& away, & away);
                    M669VecScale(&away, &away,
                             0.5f * (sphereA.radius + targetSphere.radius - separation));
                    M669VecAdd(& response[k], & response[k], & away);
                    M669VecScale(& away, & away, -1.2f);
                    M669VecAdd(& target->velocity, & target->velocity, & away);
                }
            }
        }
    }
    for (i = 0; i < 36; i++)
    {
        M669ActorWork * particle = M669ParticleGet(i);
        M669VecAdd(& particle->frameDisplacement, & particle->frameDisplacement, & response[i]);

    }
    for (i = 0; i < 36; i++)
    {
        M669ActorWork * particle = M669ParticleGet(i);

        M669VecAdd(& particle->position, & particle->position, & particle->frameDisplacement);
        if (particle->position.y <= 0.0f)
        {
            particle->position.y = 0.0f;
            particle->velocity.y = 0.0f;
            particle->frameDisplacement.y = 0.0f;
        }
    }
    for (i = 0; i < 36; i++)
    {
        M669ActorWork * particle = M669ParticleGet(i);
        HuVecF to;
        M669VecSubtract(& to, & zero, & particle->position);
        if (particle->action != 2 && particle->position.y > 120.0f &&
            M669VecLengthSquared(& to) >= 562500.0f)
        {
            particle->action = 2;
        }
    }
    {
        M669Sphere area;
        fn_1_31C(& area.center, 0.0f, 50.0f, 0.0f);
        area.radius = 800.0f;
        for (i = 0; i < 36; i++)
        {
            M669ActorWork * particle = M669ParticleGet(i);
            HuVecF direction;
            M669Sphere sphere;
            f32 distance;
            if (particle->action == 2 || particle->action == 3)
            {
                continue;
            }
            sphere.center = particle->position;
            sphere.center.y += 50.0f;
            sphere.radius = 50.0f;
            M669VecSubtract(& direction, & area.center, & sphere.center);
            distance = fn_1_554(& direction, & direction);
            if (distance > area.radius - sphere.radius)
            {
                M669VecScale(& direction, & direction, distance - (area.radius - sphere.radius));
                M669VecAdd(& particle->position, & particle->position, & direction);
            }
        }
        for (i = 0; i < 36; i++)
        {
            M669ActorWork * particle = M669ParticleGet(i);
            HuVecF direction;
            M669Sphere sphere;
            f32 distance;
            if (particle->action != 2 || particle->position.y > 120.0f)
            {
                continue;
            }
            sphere.center = particle->position;
            sphere.center.y += 50.0f;
            sphere.radius = 50.0f;
            M669VecSubtract(& direction, & sphere.center, & area.center);
            distance = fn_1_554(& direction, & direction);
            if (distance <= area.radius + sphere.radius)
            {
                M669VecScale(& direction, & direction, (area.radius + sphere.radius) - distance);
                M669VecAdd(& particle->position, & particle->position, & direction);
            }
        }
        for (i = 0; i < 36; i++)
        {
            M669ActorWork * particle = M669ParticleGet(i);
            particle->frameVelocity.x = particle->frameVelocity.y = particle->frameVelocity.z =
                0.0f;
            fn_80026790(particle->model, & particle->position);
        }
    }
}

void fn_1_6D38(OMOBJ *object);

/* Clears the actor motion queue during setup, then installs its per-frame update. */
void fn_1_6CC4(OMOBJ *object)
{
    M669PlayerAreaWork *work = object->data;
    s32 index;

    work->hitCount = 0;
    for (index = 0; index < 16; index++) {
        work->playerCounts[index] = 0;
    }
    fn_1_248(object, fn_1_6D38);
}

/* Counts player overlaps for each board area and records its nearest player. */
void fn_1_6D38(OMOBJ *object)
{
    M669PlayerAreaWork *work = object->data;
    f32 nearestDistance = 3.4028235e38f;
    M669Sphere playerSphere;
    M669Bounds playerBounds;
    M669Bounds areaBounds;
    HuVecF areaDirection;
    HuVecF playerPosition;
    s32 playerIndex;
    s32 areaIndex;
    s32 nearestPlayerIndex;

    for (playerIndex = 0; playerIndex < 16; playerIndex++) {
        work->playerCounts[playerIndex] = 0;
    }
    for (playerIndex = 0; playerIndex < 3; playerIndex++) {
        M669ImpactWork *target = lbl_1_bss_2C[playerIndex];

        fn_80086BC8(target->header.player->actor, &playerSphere.center);
        playerSphere.radius = 100.0f;
        fn_1_2504(&playerBounds, &playerSphere);
        for (areaIndex = 0; areaIndex < 16; areaIndex++) {
            areaBounds.max = lbl_1_rodata_90[areaIndex];
            areaBounds.min = areaBounds.max;
            areaBounds.min.x -= 200.0f;
            areaBounds.min.z -= 200.0f;
            areaBounds.max.x += 200.0f;
            areaBounds.max.y += 200.0f;
            areaBounds.max.z += 200.0f;
            if (fn_1_27FC(&playerBounds, &areaBounds)) {
                work->playerCounts[areaIndex]++;
            }
        }
    }
    for (areaIndex = 0; areaIndex < 16; areaIndex++) {
        for (playerIndex = 0; playerIndex < 3; playerIndex++) {
            M669ImpactWork *target = lbl_1_bss_2C[playerIndex];
            f32 distance;

            fn_80086BC8(target->header.player->actor, &playerPosition);
            M669VecSubtract(&areaDirection, &playerPosition,
                &lbl_1_rodata_90[areaIndex]);
            distance = M669VecLengthSquared(&areaDirection);

            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestPlayerIndex = playerIndex;
            }
        }
        work->nearestPlayerIndices[areaIndex] = nearestPlayerIndex;
    }
}

/* Blends to a different character motion when requested by the player update callbacks. */
void fn_1_6F90(M669ImpactWork *work, s16 motion, float blend, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        fn_8005C2C8(work->header.character,
            work->header.player->omObj->mtnId[work->motion],
            0.0f, blend, attr);
    }
}

typedef struct M669Triangle { HuVecF start, edge1, edge2; } M669Triangle;

extern f32 fn_1_2E54(Point3d *, M669Triangle *, f32 *, f32 *);

/* Detects players within the actor's impact sweep and marks each contacted target. */
void fn_1_7018(M669ImpactWork *object)
{
    M669Triangle triangle;
    M669Sphere probe;
    f32 angle;
    M669ActorWork *targetEntry;
    M669ActorWork *target;
    f32 collisionDistance;
    s32 index;
    M669PlayerAreaWork *hitCount;

    if (object->header.player->mode == 3) {
        fn_80086BC8(object->header.player->actor, &triangle.start);
        fn_80086CB0(object->header.player->actor, &angle);
        angle = (f32)(M_PI * (angle / 180.0f));
        fn_1_31C(&triangle.edge1,
            (f32)(200.0 * sin((M_PI * 40.0 / 180.0) + angle)),
            0.0f,
            (f32)(200.0 * cos((M_PI * 40.0 / 180.0) + angle)));
        fn_1_31C(&triangle.edge2,
            (f32)(200.0 * sin(angle - (M_PI * 40.0 / 180.0))),
            0.0f,
            (f32)(200.0 * cos(angle - (M_PI * 40.0 / 180.0))));

        for (index = 0; index < 36; index++) {
            targetEntry = lbl_1_bss_3C[index];
            target = targetEntry;
            if (target->action != 2 && target->action != 3) {
                probe.center = target->position;
                probe.center.y += 40.0f;
                probe.radius = 40.0f;
                if (fn_1_2E54(&probe.center, &triangle, 0, 0) <=
                    probe.radius * probe.radius) {
                    target->action = 1;
                    target->frameVelocity.y = 30.0f;
                    target->dampedVelocity.x = (f32)(0.5 *
                        (15.0 * sin(angle)));
                    target->dampedVelocity.z = (f32)(0.5 *
                        (15.0 * cos(angle)));
                    fn_8006A030(object->header.playerIndex, 12, 7, 3);
                }
            }
        }
        if (object->hitCounted == 0) {
            object->hitCounted = 1;
            hitCount = lbl_1_bss_38;
            hitCount->hitCount++;
        }
    } else {
        object->hitCounted = 0;
    }
}

void fn_1_7308(OMOBJ *);

void fn_1_7498(OMOBJ *);

void fn_1_7508(OMOBJ *);

void fn_1_77AC(OMOBJ *object);

/* Creates the stage actor and starts its sequence callback. */
void fn_1_7308(OMOBJ *object)
{
    M669ImpactWork *work = object->data;

    work->header.character = lbl_80265728[work->header.playerIndex].charNo;
    work->header.state = 0;
    work->motion = -1;
    work->hitCounted = 0;
    {
        MGACTOR_PARAM actorParam = { 0.0f, 0.0f, 0, 0, NULL, 0, 0, NULL };
        actorParam.height = 150.0f;
        actorParam.radius = 40.0f;
        actorParam.param = work->header.playerIndex;
        actorParam.type = 0;
        actorParam.attr = 0;
        actorParam.correctHookParam = 0;
        actorParam.narrowHook = NULL;
        actorParam.correctHook = NULL;
        work->header.player =
            fn_80080728(work->header.playerIndex, &actorParam, 2, 1,
                           ((1 << 0) | (1 << 2) | (1 << 5)),
                           lbl_1_data_80);
        fn_8007E818(work->header.player);
        fn_8007ED60(work->header.player);
        fn_80026B8C(work->header.player->actor->mdlId, 6);
        fn_80086AA8(work->header.player->actor, &lbl_1_data_114[work->header.positionIndex]);
        fn_80026E44(work->header.player->actor->mdlId);
        fn_1_248(object, fn_1_7498);
    }
}

/* Advances the stage sequence when its phase flag reaches 1. */
void fn_1_7498(OMOBJ *object)
{
    M669ImpactWork *work = object->data;
    M669HerdWork *sequence = lbl_1_bss_CC;

    if (sequence->sequence == 1) {
        work->phase = 0;
        fn_1_248(object, fn_1_7508);
        return;
    }
}

/* Applies the landing velocity and releases the player's movement lock at the stage endpoint. */
void fn_1_7508(OMOBJ *object)
{
    M669ImpactWork *work = object->data;
    f32 velocity;

    work->phase++;
    if (work->phase == 40) {
        if (work->header.player->actor->gravity) {
            velocity = work->header.player->actor->gravity +
                work->header.player->actor->gravity *
                    ((-1.0f + sqrtf(1.0f +
                        8.0f * (20000.0f /
                            work->header.player->actor->gravity))) / 2.0f);
        } else {
            velocity = 0.0f;
        }
        fn_80086CC4(work->header.player->actor, velocity);
        fn_800805CC(work->header.player->actor, 2);
    }
    if (work->phase == 73) {
        fn_8008062C(work->header.player->actor, 2);
    }
    if (fn_8007E8B4(work->header.player, &lbl_1_data_F0[work->header.positionIndex],
            7.0f, 10.0f) != 0) {
        M669HerdWork *sequence;

        fn_8007E888(work->header.player);
        fn_8007E818(work->header.player);
        sequence = lbl_1_bss_CC;
        sequence->sequence = 3;
        fn_1_248(object, fn_1_77AC);
        return;
    }
}

void fn_1_78D8(OMOBJ *object);

/* Plays the waiting motion before play, then clears player velocity and selects the input
 * callback. */
void fn_1_77AC(OMOBJ *object) {
    M669ImpactWork *playerWork;
    playerWork = object->data;
    if (playerWork->motion != 0x11) {
        playerWork->motion = 0x11;
        fn_8005C2C8(playerWork->header.character,
                           playerWork->header.player
                               ->omObj->mtnId[playerWork->motion],
                           0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    if (fn_80071A8C() == 5) {
        if (playerWork->motion != 0) {
            playerWork->motion = 0;
            fn_8005C2C8(playerWork->header.character,
                               playerWork->header.player
                                   ->omObj->mtnId[playerWork->motion],
                               0.0f, 4.0f, ((1 << 30) | (1 << 0)));
        }
        playerWork->velocity.x = 0.0f;
        playerWork->velocity.y = 0.0f;
        playerWork->velocity.z = 0.0f;
        fn_1_248(object, fn_1_78D8);
        return;
    }
}

void fn_1_7980(OMOBJ *object);

void fn_1_7D0C(OMOBJ *object);

void fn_1_8354(OMOBJ *object);

/* Selects human or computer control during play and switches to the finish callback in mode six. */
void fn_1_78D8(OMOBJ *object) {
    M669ImpactWork *playerWork;

    playerWork = object->data;
    if (fn_80071A8C() == 6) {
        fn_8007E818(playerWork->header.player);
        fn_1_248(object, fn_1_8354);
        return;
    }
    if (lbl_80265728[playerWork->header.playerIndex].type != 0) {
        fn_1_248(object, fn_1_7D0C);
        return;
    }
    fn_8007E888(playerWork->header.player);
    fn_1_248(object, fn_1_7980);
}

void fn_1_80A0(OMOBJ *object);

static inline M669ActorWork *M669ImpactTargetGet(s32 index)
{
    return lbl_1_bss_3C[index];
}

/* During player updates, applies impact velocity to nearby herd actors and counts each attack
 * once. */
static inline void M669ImpactCheck(M669ImpactWork *work, OMOBJ *object)
{
    M669Triangle triangle;
    M669Sphere probe;
    f32 angle;
    M669ActorWork *targetEntry;
    M669ActorWork *target;
    s32 index;
    M669PlayerAreaWork *hitCount;

    if (work->header.player->mode == 3) {
        fn_80086BC8(work->header.player->actor, &triangle.start);
        fn_80086CB0(work->header.player->actor, &angle);
        angle = (f32)(M_PI * (angle / 180.0f));
        fn_1_31C(&triangle.edge1,
            (f32)(200.0 * sin((M_PI * 40.0 / 180.0) + angle)),
            0.0f,
            (f32)(200.0 * cos((M_PI * 40.0 / 180.0) + angle)));
        fn_1_31C(&triangle.edge2,
            (f32)(200.0 * sin(angle - (M_PI * 40.0 / 180.0))),
            0.0f,
            (f32)(200.0 * cos(angle - (M_PI * 40.0 / 180.0))));

        for (index = 0; index < 36; index++) {
            target = M669ImpactTargetGet(index);
            if (target->action != 2 && target->action != 3) {
                probe.center = target->position;
                probe.center.y += 40.0f;
                probe.radius = 40.0f;
                if (fn_1_2E54(&probe.center, &triangle, 0, 0) <=
                    probe.radius * probe.radius) {
                    target->action = 1;
                    target->frameVelocity.y = 30.0f;
                    target->dampedVelocity.x = (f32)(0.5 *
                        (15.0 * sin(angle)));
                    target->dampedVelocity.z = (f32)(0.5 *
                        (15.0 * cos(angle)));
                    fn_8006A030(work->header.playerIndex, 12, 7, 3);
                }
            }
        }
        if (work->hitCounted != 0) {
            return;
        }
        work->hitCounted = 1;
        hitCount = lbl_1_bss_38;
        hitCount->hitCount++;
    } else {
        work->hitCounted = 0;
    }
}

/* Checks nearby actors against the player's impact range and counts the hit. */
void fn_1_7980(OMOBJ *object)
{
    M669ImpactWork * work = object->data;

    if (fn_80071A8C() == 6)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_8354);
        return;
    }
    if (work->header.state == 1)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_80A0);
        return;
    }
    fn_80086CCC(work->header.player->actor, & work->velocity);
    work->velocity.x = 0.0f;
    work->velocity.y = 0.0f;
    work->velocity.z = 0.0f;
    M669ImpactCheck(work, object);
}

extern s32 fn_1_13BA0(M669ImpactWork *);

/* Checks nearby actors against the player's impact range and counts the hit. */
void fn_1_7D0C(OMOBJ *object)
{
    M669Triangle triangle;
    M669ImpactWork * work = object->data;
    M669Sphere probe;
    s32 index2;
    M669ActorWork * target;
    M669ActorWork * targetEntry;
    M669ActorWork * targetPointer;
    M669PlayerAreaWork * hitCount;
    f32 angle;
    s32 index;
    if (fn_80071A8C() == 6)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_8354);
        return;
    }
    if (work->header.state == 1)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_80A0);
        return;
    }
    fn_80086CCC(work->header.player->actor, & work->velocity);
    work->velocity.x = 0.0f;
    work->velocity.y = 0.0f;
    work->velocity.z = 0.0f;
    fn_1_13BA0(work);
    if (work->header.player->mode == 3)
    {
        fn_80086BC8(work->header.player->actor, &triangle.start);
        fn_80086CB0(work->header.player->actor, & angle);
        angle = (f32)(M_PI * (angle / 180.0f));
        fn_1_31C(&triangle.edge1, (f32) (200.0 * sin((M_PI * 40.0 / 180.0) + angle)), 0.0f,
                 (f32) (200.0 * cos((M_PI * 40.0 / 180.0) + angle)));
        fn_1_31C(&triangle.edge2, (f32) (200.0 * sin(angle - (M_PI * 40.0 / 180.0))), 0.0f,
                 (f32) (200.0 * cos(angle - (M_PI * 40.0 / 180.0))));
        for (index = 0; index < 36; index++)
        {
            index2 = index;
            targetEntry = lbl_1_bss_3C[index2];
            targetPointer = targetEntry;
            target = targetPointer;
            if (target->action != 2 && target->action != 3)
            {
                probe.center = target->position;
                probe.center.y += 40.0f;
                probe.radius = 40.0f;
                if (fn_1_2E54(& probe.center, &triangle, 0, 0) <= probe.radius * probe.radius)
                {
                    target->action = 1;
                    target->frameVelocity.y = 30.0f;
                    target->dampedVelocity.x = (f32)(0.5 * (15.0 * sin(angle)));
                    target->dampedVelocity.z = (f32)(0.5 * (15.0 * cos(angle)));
                    fn_8006A030(work->header.playerIndex, 12, 7, 3);
                }
            }
        }
        if (work->hitCounted != 0)
        {
            return;
        }
        work->hitCounted = 1;
        hitCount = lbl_1_bss_38;
        hitCount->hitCount++;
    }
    else
    {
        work->hitCounted = 0;
    }
}

void fn_1_8168(OMOBJ *object);

/* Starts the 300-frame player pause, unless finish mode has already selected the result
 * callback. */
void fn_1_80A0(OMOBJ *object) {
    M669ImpactWork *playerWork;

    playerWork = object->data;
    if (fn_80071A8C() == 6) {
        fn_8007E818(playerWork->header.player);
        fn_1_248(object, fn_1_8354);
        return;
    }
    playerWork->header.timer = 0x12C;
    if (playerWork->motion != 0x10) {
        playerWork->motion = 0x10;
        fn_8005C2C8(playerWork->header.character,
                           playerWork->header.player
                               ->omObj->mtnId[playerWork->motion],
                           0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    fn_1_248(object, fn_1_8168);
}

/* Slows the player actor, then returns to its idle motion when the timer ends. */
void fn_1_8168(OMOBJ *object)
{
    M669ImpactWork *work = object->data;

    if (fn_80071A8C() == 6) {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_8354);
        return;
    }
    work->velocity.x += 0.2f * -work->velocity.x;
    work->velocity.y += 0.2f * -work->velocity.y;
    work->velocity.z += 0.2f * -work->velocity.z;
    fn_80086CCC(work->header.player->actor, &work->velocity);
    if (work->header.player->omObj->mtnId[16] !=
        fn_8002D3C4(work->header.player->actor->mdlId)) {
        work->motion = -1;
    }
    if (work->motion != 16) {
        work->motion = 16;
        fn_8005C2C8(work->header.character, work->header.player->omObj->mtnId[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    work->header.timer--;
    if (work->header.timer <= 0) {
        work->header.state = 0;
        if (work->motion != 0) {
            work->motion = 0;
            fn_8005C2C8(work->header.character,
                               work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f,
                               ((1 << 30) | (1 << 0)));
        }
        fn_1_248(object, fn_1_78D8);
        return;
    }
}

void fn_1_843C(OMOBJ *);

/* Zeroes the actor velocity and advances its motion in pre-winner mode. */
void fn_1_8354(OMOBJ *object)
{
    M669ImpactWork *work = object->data;

    work->velocity.x = 0.0f;
    work->velocity.y = 0.0f;
    work->velocity.z = 0.0f;
    fn_80086CCC(work->header.player->actor, &work->velocity);
    if (work->motion != 0) {
        work->motion = 0;
        fn_8005C2C8(work->header.character,
            work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    if (fn_80071A8C() == 7) {
        fn_1_248(object, fn_1_843C);
        return;
    }
}

void fn_1_8790(OMOBJ *object);

/* Turns a character toward the front, then hands control back to the player callback. */
void fn_1_843C(OMOBJ *object)
{
    M669ImpactWork *work = object->data;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 currentAngle;
    f32 angleDifference;
    f32 cross;
    f32 adjustedAngle;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8005C2C8(work->header.character,
            work->header.player->omObj->mtnId[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    currentAngle = (f32)(M_PI *
        (work->header.player->actor->rotY / 180.0f));
    fn_1_31C(&targetDirection, (f32)sin(0.0),
        0.0f, (f32)cos(0.0));
    fn_1_31C(&currentDirection, (f32)sin(currentAngle),
        0.0f, (f32)cos(currentAngle));
    angleDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = currentAngle;
    if (cross >= 0.0f) {
        adjustedAngle += angleDifference / 20.0f;
    } else if (cross < 0.0f) {
        adjustedAngle -= angleDifference / 20.0f;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->header.player->actor->rotY = (f32)((f64)(180.0f * adjustedAngle) /
        M_PI);
    if (fabsf(adjustedAngle - currentAngle) <= 0.01f) {
        if (work->motion != 0) {
            work->motion = 0;
            fn_8005C2C8(work->header.character,
                work->header.player->omObj->mtnId[work->motion],
                0.0f, 4.0f, ((1 << 30) | (1 << 0)));
        }
        lbl_1_bss_A++;
        fn_1_248(object, fn_1_8790);
        return;
    }
}

/* Chooses the player victory or defeat motion in result mode, then disables the callback. */
void fn_1_8790(OMOBJ *object) {
    M669ImpactWork *playerWork;
    playerWork = object->data;
    if (fn_80071A8C() == 8) {
        if (lbl_1_data_168 == playerWork->header.groupIndex) {
            if (playerWork->motion != 0xE) {
                playerWork->motion = 0xE;
                fn_8005C2C8(playerWork->header.character,
                                   playerWork->header.player
                                       ->omObj->mtnId[playerWork->motion],
                                   0.0f, 4.0f, 0);
            }
        } else {
            if (playerWork->motion != 0xF) {
                playerWork->motion = 0xF;
                fn_8005C2C8(playerWork->header.character,
                                   playerWork->header.player
                                       ->omObj->mtnId[playerWork->motion],
                                   0.0f, 4.0f, 0);
            }
        }
        fn_1_248(object, 0);
        return;
    }
}

/* Pans a position based effect and chooses its sound from the accumulated events. */
void fn_1_88A0(void)
{
    M669HerdWork *countWorkA;
    M669HerdWork *countWorkB;
    M669HerdWork *positionWork;
    HuVecF screenPosition;
    HuVecF position;
    f32 rawPan;
    f32 clampedPan;
    f32 limitedPan;
    s32 eventCount;

    countWorkA = lbl_1_bss_CC;
    countWorkB = lbl_1_bss_CC;
    eventCount = countWorkB->radialCounts[0] + countWorkA->radialCounts[1];
    if (eventCount == 0) {
        return;
    }
    positionWork = lbl_1_bss_CC;
    position = positionWork->position;
    Hu3D3Dto2D(&position, 1, &screenPosition);
    rawPan = 64.0f * (screenPosition.x / 576.0f);
    if (rawPan < 0.0f) {
        clampedPan = 0.0f;
    } else {
        if (rawPan > 64.0f) {
            limitedPan = 64.0f;
        } else {
            limitedPan = rawPan;
        }
        clampedPan = limitedPan;
    }
    rawPan = clampedPan;
    rawPan += 32.0f;
    if (eventCount <= 5) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_SMALL, (s16)rawPan);
        return;
    }
    if (eventCount <= 10) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_MEDIUM, (s16)rawPan);
        return;
    }
    if (eventCount <= 25) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_LARGE, (s16)rawPan);
        return;
    }
    if (eventCount <= 50) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_HUGE, (s16)rawPan);
    }
}

/* Pins the model position and updates it when its tracked motion reaches the end. */
void fn_1_8A70(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* Places the actor's model above its stored position and starts its default motion. */
void fn_1_8B78(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y += 50.0f;
    fn_80026790(work->motionModel, &position);
    fn_8002C95C(work->motionModel, work->motionDataId);
    fn_80026A44(work->motionModel, (1 << 0));
    fn_8002D414(work->motionModel, 0.0f);
    fn_8002D86C(work->motionModel, 1.0f);
    work->motionComplete = 1;
}

/* Advances the actor's sine-driven vertical transition and fade. */
void fn_1_8C24(HU3D_MODEL *, HU3D_PARTICLE *particleP, Mtx)
{
    HU3D_PARTICLE_DATA *work = particleP->data;
    f32 verticalWave;

    if (work->time < 0) {
        return;
    }
    if (work->time < 8) {
        verticalWave = (f32)(((f64)0.3f) + sin((M_PI * (f64)(
                40.0f + 10.0f * (f32)(work->time + 1))) /
                180.0));
        work->scale = 100.0f * verticalWave;
        work->color.a = 255;
        verticalWave = (f32)sin((M_PI * (f64)(
            0.3f + 15.0f * (f32)(work->time + 1))) /
            180.0);
    } else {
        verticalWave = (f32)sin((M_PI * (f64)135.3f / 180.0));
    }
    work->pos.x = work->accel.x;
    work->pos.y = work->accel.y + 150.0f * verticalWave;
    work->pos.z = work->accel.z;
    work->time++;
    if (work->time > 20) {
        work->color.a -= 32;
        work->scale -= 8.0f;
        if (work->scale < 0.0f) {
            work->scale = 0.0f;
            work->time = -1;
        }
    }
}

/* Resets the model effect hook and places it fifty units above the source position. */
void fn_1_8E44(M669ActorWork *source)
{
    f32 verticalOffset = 50.0f;
    HU3D_PARTICLE *hookData = lbl_802C01C8[source->particleModel].hookData;
    HU3D_PARTICLE_DATA *effect = hookData->data;

    effect->time = 0;
    effect->scale = 1.0f;
    effect->accel.x = source->position.x;
    effect->accel.y = source->position.y + verticalOffset;
    effect->accel.z = source->position.z;
    effect->time = 0;
}

/* Changes a herd actor motion only when the requested motion index differs. */
void fn_1_8ED8(M669ActorWork *work, s16 motion) {
    if (work->motion != motion) {
        work->motion = motion;
        fn_8002C95C(work->model,
            work->motionIds[work->motion]);
    }
}

/* Turns the actor toward the requested heading at the supplied angular rate. */
f32 fn_1_8F3C(M669ActorWork *work, f64 heading, f32 rate)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 angularDifference;
    f32 cross;
    f32 adjustedAngle;
    f32 angleDelta;
    fn_1_31C(&targetDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_31C(&currentDirection, (f32)sin(work->angle), 0.0f,
        (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = work->angle;
    if (cross >= 0.0f) {
        angleDelta = angularDifference / rate;
    } else {
        angleDelta = -(angularDifference / rate);
    }
    adjustedAngle += angleDelta;
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->angle = adjustedAngle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

void fn_1_8C24(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

void fn_1_9614(OMOBJ *object);

/* Initializes one herd actor; the first loads models and motions that the remaining actors
 * share. */
void fn_1_9144(OMOBJ *object) {
    f32 radius;
    f64 angle;
    M669ActorWork * sourceModel;
    M669ActorWork * sourceMotions;
    M669ActorWork * sourceMotionModel;
    M669ActorWork * sourceMotionData;
    M669ActorWork * sourcePositionModel;
    M669HerdWork * board;
    s32 particleIndex;
    s32 motionIndex;
    HU3D_PARTICLE_DATA * particleData;
    HU3D_PARTICLE * particle;
    M669ActorWork * work;
    work = object->data;
    work->motionComplete = 0;
    work->unidentifiedState = 0;
    if (work->actorIndex == 0)
    {
        work->model =
            fn_80025A04(fn_80008DD0(DATANUM(0x800000, 7), 0x10000000, 2));
        motionIndex = 0;
        while (motionIndex < 4)
        {
            work->motionIds[motionIndex] =
                fn_80030458(work->model, fn_80008DD0(lbl_1_data_58[motionIndex],
                                                                  0x10000000, 2));
            motionIndex += 1;
        }
        work->motionModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x800000, 14), 0x10000000, 2));
        work->motionDataId =
            fn_80030458(work->motionModel, fn_80008DD0(DATANUM(0x800000, 15),
                                                                    0x10000000, 2));
    }
    else
    {
        sourceModel = * lbl_1_bss_3C;
        work->model = fn_80025D4C(sourceModel->model);
        motionIndex = 0;
        while (motionIndex < 4)
        {
            sourceMotions = * lbl_1_bss_3C;
            work->motionIds[motionIndex] = sourceMotions->motionIds[motionIndex];
            motionIndex += 1;
        }
        sourceMotionModel = * lbl_1_bss_3C;
        work->motionModel = fn_80025D4C(sourceMotionModel->motionModel);
        sourceMotionData = * lbl_1_bss_3C;
        work->motionDataId = sourceMotionData->motionDataId;
    }
    fn_800269E8(work->motionModel, (1 << 0));
    fn_80026B8C(work->motionModel, 7);
    fn_8002D86C(work->motionModel, 0.0f);
    work->motion = 2;
    fn_8002C95C(work->model, work->motionIds[work->motion]);
    fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
    fn_80026B8C(work->model, 5);
    angle = 2.0 * (M_PI * frandf());
    angle = (f64)(f32) angle;
    radius = 700.0f * frandf();
    fn_1_31C(&work->position, (f32) ((f64) radius * sin(angle)), 0.0f,
             (f32) ((f64) radius * cos(angle)));
    fn_80026790(work->model, &work->position);
    work->angle = (f32)(2.0 * (M_PI * frandf()));
    fn_80026808(work->model, 0.0f, (f32) ((f64) (180.0f * work->angle) / M_PI),
                    0.0f);
    fn_1_31C(&work->gravity, 0.0f, -2.5f, 0.0f);
    if (work->actorIndex == 0)
    {
        work->positionModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x800000, 12), 0x10000000, 2));
    }
    else
    {
        sourcePositionModel = * lbl_1_bss_3C;
        work->positionModel = fn_80025D4C(sourcePositionModel->positionModel);
    }
    fn_80026790(work->positionModel, &work->position);
    fn_80026B8C(work->positionModel, 4);
    board = lbl_1_bss_CC;
    work->particleModel = fn_8003216C(board->animation, 1);
    fn_800326D0(work->particleModel, fn_1_8C24);
    fn_80026B8C(work->particleModel, 7);
    fn_800269E8(work->particleModel, (1 << 22));
    fn_8003269C(work->particleModel, 0U);
    particle = lbl_802C01C8[work->particleModel].hookData;
    particleData = particle->data;
    particleIndex = 0;
    while (particleIndex < particle->maxCnt)
    {
        particleData->scale = 0.0f;
        particleData->time = - 1;
        particleIndex += 1;
        particleData++;
    }
    work->ready = 0;
    work->action = 0;
    work->collisionWeight = 1.0f;
    work->unidentifiedMotionValue = 0;
    work->timer = 0;
    work->motionSpeed = (f32)(1.0f + frandf());
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    if (work->motion != 2)
    {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    fn_1_248(object, fn_1_9614);
}

extern void fn_1_9864(OMOBJ *);

extern void fn_1_9D24(OMOBJ *);

/* During the herd motion sequence, positions the ground model and hides a finished effect
 * motion. */
static inline void M669MotionSequenceStep(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* During the herd motion sequence, turns toward a radian heading by a fraction of the angular
 * gap. */
static inline f32 M669MotionSequenceTurn(M669ActorWork *work, f64 heading, f32 rate)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 angularDifference;
    f32 cross;
    f32 adjustedAngle;

    fn_1_31C(&targetDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_31C(&currentDirection, (f32)sin(work->angle), 0.0f,
        (f32)cos(work->angle));

    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = work->angle;
    adjustedAngle += cross >= 0.0f ?
        angularDifference / rate : -(angularDifference / rate);
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->angle = adjustedAngle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

/* Advances the actor motion through its lift and settling phases. */
void fn_1_9614(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(
            fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    work->timer++;
    if (work->timer == 30) {
        fn_8002D86C(work->model, 1.5f);
        if (work->motion != 3) {
            work->motion = 3;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        fn_80026A44(work->model, ((1 << 30) | (1 << 0)));
        return;
    }
    if (work->timer >= 45) {
        if (work->motion != 2) {
            work->motion = 2;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        work->motionSpeed = 8.0f;
        fn_8002D86C(work->model,
            work->motionSpeed / 3.0f);
        fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
        work->timer = 0;
        fn_1_248(object, fn_1_9864);
        return;
    }
    work->frameVelocity.x += work->motionSpeed * sin(work->angle);
    work->frameVelocity.z += work->motionSpeed * cos(work->angle);
}

/* Turns the moving actor toward its travel direction, then advances it. */
void fn_1_9864(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position;
    HuVecF facingDirection;
    HuVecF travelDirection;
    f32 travelDistance;
    f32 turnAmount;
    f32 turnDirection;
    f32 nextAngle;

    M669MotionSequenceStep(work);
    M669VecSubtract(&position, &lbl_1_rodata_150[work->actorIndex % 36],
        &work->position);
    travelDistance = fn_1_554(&position, &position);
    M669MotionSequenceTurn(work, atan2f(position.x, position.z), 5.0f);
    if (travelDistance < work->motionSpeed) {
        work->motionSpeed = 0.5f * travelDistance;
        fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    }
    work->frameVelocity.x += work->motionSpeed * sin(work->angle);
    work->frameVelocity.z += work->motionSpeed * cos(work->angle);
    work->timer++;
    if (travelDistance <= 21.0f || work->timer > 300) {
        if (work->motion != 1) {
            work->motion = 1;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        fn_8002D86C(work->model, 1.0f);
        work->collisionWeight = 0.1f;
        work->motionSpeed = 1.0f;
        fn_1_248(object, fn_1_9D24);
        return;
    }
}

extern void fn_1_E0C0(OMOBJ *);

extern void fn_1_A3D8(OMOBJ *);

extern void fn_1_A7E4(OMOBJ *);

/* Positions the stage actor and clears its completed motion state. */
static inline void M669StageRotationStep(M669ActorWork *work)
{
    HuVecF position = work->position;
    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* Unused sequence helper that positions the ground model and hides a finished effect motion. */
static inline void M669SequencePositionUpdate(M669ActorWork *work)
{
    HuVecF position = work->position;
    f64 absoluteTimeDifference;
    f32 motionTimeDifference;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        absoluteTimeDifference = fabs(
            fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel));
        motionTimeDifference = (f32)absoluteTimeDifference;
        if (motionTimeDifference <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* Unused helper that writes the vector from the second point to the first and returns its squared
 * length. */
static inline f32 M669DirectionDifferenceSquared(HuVecF *direction, const HuVecF *from,
    const HuVecF *to)
{
    M669VecSubtract(direction, from, to);
    return M669VecLengthSquared(direction);
}

/* During the actor rotation callback, turns toward the requested radian heading at the supplied
 * rate. */
static inline f32 M669StageRotationTurnByRate(M669ActorWork *work, f64 targetAngle,
    f32 rate)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    Mtx motionRotation;
    f32 angularDifference;
    f32 cross;
    f32 angle;

    fn_1_31C(&targetDirection, (f32)sin(targetAngle), 0.0f,
        (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(work->angle), 0.0f,
        (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    angle = work->angle;
    angle += cross >= 0.0f ? angularDifference / rate : -(angularDifference / rate);
    while (angle > (2.0 * M_PI)) {
        angle = (f32)((f64)angle - (2.0 * M_PI));
    }
    while (angle < (-2.0 * M_PI)) {
        angle += (2.0 * M_PI);
    }
    work->angle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

/* During the actor return callback, turns through one tenth of the remaining angular gap. */
static inline f32 M669StageRotationTurnFixedRate(M669ActorWork *work,
    f64 targetAngle)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    Mtx motionRotation;
    f32 angularDifference;
    f32 cross;
    f32 angle;

    fn_1_31C(&targetDirection, (f32)sin(targetAngle), 0.0f,
        (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(work->angle), 0.0f,
        (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    angle = work->angle;
    angle += cross >= 0.0f ?
        angularDifference / 10.0f :
        -(angularDifference / 10.0f);
    while (angle > (2.0 * M_PI)) {
        angle = (f32)((f64)angle - (2.0 * M_PI));
    }
    while (angle < (-2.0 * M_PI)) {
        angle += (2.0 * M_PI);
    }
    work->angle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

/* Starts the actor sequence effect 50 units above the actor when its sequence requests it. */
static inline void M669SequenceActorEffectInit(M669ActorWork *source)
{
    f32 verticalOffset = 50.0f;
    HU3D_PARTICLE *hookData = lbl_802C01C8[source->particleModel].hookData;
    HU3D_PARTICLE_DATA *effect = hookData->data;

    effect->time = 0;
    effect->scale = 1.0f;
    effect->accel.x = source->position.x;
    effect->accel.y = source->position.y + verticalOffset;
    effect->accel.z = source->position.z;
    effect->time = 0;
}

static inline M669HerdWork *M669SequenceStateGet(void)
{
    return lbl_1_bss_CC;
}

/* Turns the moving actor toward the selected motion point and advances it. */
void fn_1_9D24(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF direction;
    f32 distance;
    f32 targetAngle;

    M669StageRotationStep(work);
    M669VecSubtract(&direction, &lbl_1_rodata_150[work->actorIndex % 36],
        &work->position);
    distance = fn_1_554(&direction, &direction);
    if (distance < 1.0f) {
        work->ready = 1;
        M669StageRotationTurnByRate(work,
            3.1415927f, 20.0f);
    } else {
        work->ready = 0;
        M669StageRotationTurnByRate(work,
                                    atan2f(direction.x, direction.z), 2.0f);
        work->frameVelocity.x = (f32)((f64)work->frameVelocity.x +
            (0.5 * ((f64)distance * sin((f64)work->angle))));
        work->frameVelocity.z = (f32)((f64)work->frameVelocity.z +
            (0.5 * ((f64)distance * cos((f64)work->angle))));
    }
    if (M669SequenceStateGet()->sequence == 3) {
        work->timer = 0;
        fn_8002D86C(work->model, 1.0f);
        if (work->motion != 3) {
            work->motion = 3;
            fn_8002C95C(work->model,
                work->motionIds[work->motion]);
        }
        fn_80026A44(work->model, ((1 << 30) | (1 << 0)));
        M669SequenceActorEffectInit(work);
        fn_1_248(object, fn_1_A3D8);
        return;
    }
}

/* Turns the stage actor toward the nearest player before switching to its next motion. */
void fn_1_A3D8(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF nearestDirection;
    HuVecF playerPosition;
    HuVecF candidateDirection;
    f32 minimumDistance;
    f32 distance;
    s32 index;

    minimumDistance = 3.4028235e38f;
    M669StageRotationStep(work);
    for (index = 0; index < 3; index++) {
        fn_80086BC8(fn_1_4FC0(index)->header.player->actor, &playerPosition);
        M669VecSubtract(&candidateDirection, &playerPosition, &work->position);
        distance = M669VecLengthSquared(&candidateDirection);
        if (distance < minimumDistance) {
            minimumDistance = distance;
            nearestDirection = candidateDirection;
        }
    }
    M669StageRotationTurnFixedRate(work, atan2f(nearestDirection.x,
        nearestDirection.z));
    work->timer++;
    if (work->timer >= 20) {
        fn_1_248(object, fn_1_A7E4);
        return;
    }
}

/* Starts a random herd movement burst and hands the actor to its movement callback. */
void fn_1_A7E4(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF direction;
    f32 angle;
    f32 moveRadius;

    M669StageRotationStep(work);
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
    work->collisionWeight = 1.0f;
    work->frameCount = 90;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (1.0f + 3.0f * frandf());
    angle = (f32)(2.0 * (M_PI * frandf()));
    moveRadius = 800.0f * frandf();
    fn_1_31C(&direction, (f32)(moveRadius * sin(angle)),
        0.0f, (f32)(moveRadius * cos(angle)));
    work->motionTime = atan2f(direction.x, direction.z);
    fn_8002D86C(work->model,
        work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_E0C0);
}

extern void fn_1_AAAC(OMOBJ *);

extern void fn_1_AD6C(OMOBJ *);

extern void fn_1_B1C0(OMOBJ *);

extern void fn_1_BD8C(OMOBJ *);

extern void fn_1_C2A8(OMOBJ *);

extern void fn_1_C844(OMOBJ *);

extern void fn_1_C040(OMOBJ *);

extern void fn_1_D010(OMOBJ *);

extern void fn_1_D830(OMOBJ *);

extern void fn_1_DA54(OMOBJ *);

extern void fn_1_DC78(OMOBJ *);

extern void fn_1_DE9C(OMOBJ *);

extern void fn_1_E474(OMOBJ *);

extern void fn_1_E648(OMOBJ *);

/* Advances the stage sequence, choosing a transition branch after its stage callback. */
void fn_1_AAAC(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;
    M669PlayerSetupWork *stageWork;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
            fn_1_248(object, fn_1_D010);
            return;
    default:
        work->unidentifiedMotionValue = (s16)(rand8() % 3);
        if (work->unidentifiedMotionValue == 0) {
            fn_1_248(object, fn_1_BD8C);
            return;
        }
        if (work->unidentifiedMotionValue == 1) {
            fn_1_248(object, fn_1_C2A8);
            return;
        }
        if (work->unidentifiedMotionValue == 2) {
            fn_1_248(object, fn_1_C844);
            return;
        }
    }
}

extern void fn_1_AF60(OMOBJ *);

/* Advances the stage actor's motion and selects its next transition callback. */
void fn_1_AD6C(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF setupPosition = work->position;
    HuVecF motionPosition;

    setupPosition.y = 0.01f;
    fn_80026790(work->positionModel, &setupPosition);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    motionPosition = work->position;
    motionPosition.y += 50.0f;
    fn_80026790(work->motionModel, &motionPosition);
    fn_8002C95C(work->motionModel, work->motionDataId);
    fn_80026A44(work->motionModel, (1 << 0));
    fn_8002D414(work->motionModel, 0.0f);
    fn_8002D86C(work->motionModel, 1.0f);
    work->motionComplete = 1;
    fn_1_248(object, fn_1_AF60);
}

/* Turns the stage model as its motion reaches the two handoff points. */
void fn_1_AF60(OMOBJ *object)
{
    M669ActorWork *work = object->data;

    fn_1_8A70(work);
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->angle += (M_PI / 6.0);
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (fabsf(work->frameDisplacement.y) <= 1.0f &&
        0.0f == work->position.y) {
        work->action = 0;
        fn_1_248(object, fn_1_AAAC);
        return;
    }
}

extern void fn_1_B3D0(OMOBJ *);

/* Updates the stage actor's projected sound pan and advances its callback. */
void fn_1_B1C0(OMOBJ *object)
{
    HuVecF screenPosition;
    HuVecF position;
    f32 screenPan;
    f32 clampedPan;
    f32 upperPan;
    M669ActorWork *work;

    work = object->data;
    position = work->position;
    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {

        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    Hu3D3Dto2D(&work->position, 1, &screenPosition);
    screenPan = 64.0f * (screenPosition.x / 576.0f);
    if (screenPan < 0.0f) {
        clampedPan = 0.0f;
    } else {
        if (screenPan > 64.0f) {
            upperPan = 64.0f;
        } else {
            upperPan = screenPan;
        }
        clampedPan = upperPan;
    }
    screenPan = clampedPan;
    screenPan += 32.0f;
    HuAudFXPlayPan(MSM_SE_MG669_STAGE_ACTOR_CUE, screenPan);
    work->motionSpeed = 15.0f;
    fn_1_248(object, fn_1_B3D0);
}

extern void fn_1_B960(OMOBJ *);

/* During herd movement, positions the ground model and hides the effect model when its motion
 * ends. */
static inline void fn_1_8A70_for_B3D0(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* During herd movement, adjusts the model heading toward the requested radian angle. */
static inline f32 M669MovingStageTurn(M669ActorWork *work,
    f64 heading, f32 rate)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 angularDifference;
    f32 cross;
    f32 adjustedAngle;

    fn_1_31C(&targetDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_31C(&currentDirection, (f32)sin(work->angle),
        0.0f, (f32)cos(work->angle));

    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = work->angle;
    adjustedAngle += cross >= 0.0f ?
        angularDifference / rate : -(angularDifference / rate);
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->angle = adjustedAngle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) /
            M_PI),
        0.0f);
    return angularDifference;
}

/* Turns the stage actor back toward its route after it moves outside it. */
void fn_1_B3D0(OMOBJ *object)
{
    M669ActorWork * work = object->data;
    HuVecF targetDirection = { 0.0f, 0.0f, -1.0f };
    HuVecF routePosition = { 0.0f, 0.0f, 0.0f };
    HuVecF leftDirection = { 0.0f, 1.0f, 0.0f };
    HuVecF rightDirection = { 0.0f, -1.0f, 0.0f };
    HuVecF direction;
    HuVecF collisionDirection;
    f32 distance;
    f32 cross;
    fn_1_8A70_for_B3D0(work);
    if (work->position.y > 0.0f)
    {
        return;
    }
    if (work->motion != 2)
    {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    M669VecSubtract(& direction, & routePosition, & work->position);
    distance = fn_1_554(& direction, & direction);
    cross = targetDirection.x * direction.z - targetDirection.z * direction.x;
    if (M669VecDot(& direction, & targetDirection) < 0.99f)
    {
        if (cross > 0.0f)
        {
            fn_1_4AC(& collisionDirection, & direction, & leftDirection);
        }
        else
        {
            fn_1_4AC(& collisionDirection, & direction, & rightDirection);
        }
        M669VecScale(& direction, & direction, - (900.0f - distance) / 15.0f);
        M669VecAdd(& collisionDirection, & collisionDirection, & direction);
        fn_1_554(& collisionDirection, & collisionDirection);
    }
    else
    {
        fn_1_248(object, fn_1_B960);
        return;
    }
    M669MovingStageTurn(work, atan2f(collisionDirection.x, collisionDirection.z), 10.0f);
    work->frameVelocity.x += (f64) work->motionSpeed * sin(work->angle);
    work->frameVelocity.z += (f64) work->motionSpeed * cos(work->angle);
    if (distance > 1200.0f)
    {
        fn_1_248(object, fn_1_B960);
        return;
    }
}

/* Turns and advances the stage actor toward the reference position. */
void fn_1_B960(OMOBJ *object)
{
    M669ActorWork * work = object->data;
    HuVecF referencePosition = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;
    HuVecF projectedDirection;
    f32 targetDistance;
    fn_1_8A70_for_B3D0(work);
    M669VecSubtract(& direction, & referencePosition, & work->position);
    targetDistance = fn_1_554(& direction, & direction);
    M669VecScale(& projectedDirection, & direction, -1.0f);
    M669MovingStageTurn(work, atan2f(projectedDirection.x, projectedDirection.z), 10.0f);
    work->frameVelocity.x += (f64) work->motionSpeed * sin(work->angle);
    work->frameVelocity.z += (f64) work->motionSpeed * cos(work->angle);
    if (targetDistance > 1200.0f)
    {
        fn_800269E8(work->model, (1 << 0));
        fn_800269E8(work->positionModel, (1 << 0));
        work->action = 3;
        fn_1_248(object, NULL);
        return;
    }
}

/* Advances the stage transition and dispatches the next stage callback. */
void fn_1_BD8C(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;
    M669PlayerSetupWork *stageWork;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
            fn_1_248(object, fn_1_D010);
            return;
    default:
        if (work->motion != 1) {
            work->motion = 1;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        work->frameCount = (s32)(60.0f * frandf());
        work->timer = 0;
        fn_8002D86C(work->model, 1.0f);
        fn_1_248(object, fn_1_C040);
        return;
    }
}

/* After updating its effect model, dispatches the herd actor from its action and microphone-player
 * state. */
void fn_1_C040(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;
    M669PlayerSetupWork *stageWork;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
            fn_1_248(object, fn_1_D010);
            return;
    default:
        work->timer++;
        if (work->timer >= work->frameCount) {
            fn_1_248(object, fn_1_AAAC);
            return;
        }
    }
}

extern void fn_1_C59C(OMOBJ *);

/* Advances the alternate stage transition and waits before choosing the next stage callback. */
void fn_1_C2A8(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;
    M669PlayerSetupWork *stageWork;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
            fn_1_248(object, fn_1_D010);
            return;
    default:
        if (work->motion != 2) {
            work->motion = 2;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        work->frameCount = (s32)(60.0f *
            (2.0f * frandf()));
        work->timer = 0;
        work->motionSpeed = 2.0f +
            3.0f * frandf();
        fn_8002D86C(work->model,
            work->motionSpeed / 3.0f);
        fn_1_248(object, fn_1_C59C);
        return;
    }
}

extern void fn_1_CBB4(OMOBJ *);

/* During random herd movement, updates the ground model and hides a completed effect motion. */
static inline void M669RandomStageStep(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* During the random turn callback, turns through one twentieth of the gap to the stored heading. */
static inline f32 M669TurningStageAngle(M669ActorWork *work)
{
    f32 heading = work->motionTime;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 angularDifference;
    f32 cross;
    f32 adjustedAngle;

    fn_1_31C(&targetDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_31C(&currentDirection, (f32)sin(work->angle), 0.0f,
        (f32)cos(work->angle));

    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = work->angle;
    adjustedAngle += cross >= 0.0f ?
        angularDifference / 20.0f :
        -(angularDifference / 20.0f);
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    work->angle = adjustedAngle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

/* Updates the stage path and selects the next callback for the active stage. */
void fn_1_C59C(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;
    M669PlayerSetupWork *stageWork;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
        fn_1_248(object, fn_1_D010);
        return;
    default:
        work->frameVelocity.x += work->motionSpeed * sin(work->angle);
        work->frameVelocity.z += work->motionSpeed * cos(work->angle);
        work->timer++;
        if (work->timer >= work->frameCount) {
            fn_1_248(object, fn_1_AAAC);
            return;
        }
    }
}

/* Chooses a random turn direction after a path stage and waits for its callback. */
void fn_1_C844(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF origin = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;
    HuVecF position;
    f32 motionEnd;
    M669PlayerSetupWork *stageWork;

    M669RandomStageStep(work);
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
        fn_1_248(object, fn_1_D010);
        return;
    default:
        if (work->motion != 0) {
            work->motion = 0;
            fn_8002C95C(work->model, work->motionIds[work->motion]);
        }
        if ((u32)fn_80071B58() > 1320U) {
            M669VecSubtract(&direction, &origin, &work->position);
        } else {
            M669VecSubtract(&direction, &work->position, &origin);
        }
        fn_1_554(&direction, &direction);
        work->motionTime = (f32)((f64)atan2f(direction.x, direction.z) +
            (M_PI * ((f64)(frandf() - 0.5f))));
        fn_8002D86C(work->model, 1.0f);
        fn_1_248(object, fn_1_CBB4);
        return;
    }
}

/* Turns the stage actor toward its selected direction before starting the next stage. */
void fn_1_CBB4(OMOBJ *object)
{
    M669ActorWork *work;
    f32 angleDifference;
    M669PlayerSetupWork *stageWork;
    work = object->data;
    M669RandomStageStep(work);
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }

    stageWork = lbl_1_bss_28;
    switch (stageWork->header.state) {
    case 0:
        fn_1_248(object, fn_1_D830);
        return;
    case 1:
        fn_1_248(object, fn_1_DA54);
        return;
    case 2:
        fn_1_248(object, fn_1_DC78);
        return;
    case 3:
        fn_1_248(object, fn_1_DE9C);
        return;
    case 4:
        fn_1_248(object, fn_1_E474);
        return;
    case 5:
        fn_1_248(object, fn_1_D010);
        return;
    default:
        angleDifference = M669TurningStageAngle(work);
        if (angleDifference <= 0.1f) {
            fn_1_248(object, fn_1_AAAC);
            return;
        }
        return;
    }
}

extern void fn_1_D2EC(OMOBJ *);

/* Places the position model at ground height and clears a finished motion. */
static inline void fn_1_D010UpdateModel(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* Aims the stage model at the seeded position and starts its randomized motion. */
void fn_1_D010(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF targetPosition = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;

    fn_1_D010UpdateModel(work);
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    else if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    else if (fn_80071A8C() == 8U) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    M669VecSubtract(&direction, &targetPosition, &work->position);
    fn_1_554(&direction, &direction);
    work->motionTime = (f32)(atan2f(direction.x, direction.z) +
        (frandf() - 0.5f) * M_PI);
    work->frameCount = 60;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (2.0f + 4.0f * frandf());
    fn_8002D86C(work->model,
        work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_D2EC);
}

/* During targeted herd movement, updates the ground model and hides a completed effect motion. */
static inline void M669MovingStageStep(M669ActorWork *work)
{
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
}

/* During targeted herd movement, turns toward the requested radian heading at the supplied rate. */
static inline f32 M669MovingStageSteer(M669ActorWork *work, f64 heading, f32 rate)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    Mtx motionRotation;
    f32 angularDifference;
    f32 cross;
    f32 angle;

    fn_1_31C(&targetDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_31C(&currentDirection, (f32)sin(work->angle),
        0.0f, (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    cross = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    angle = work->angle;
    angle += (cross >= 0.0f) ? angularDifference / rate : -(angularDifference / rate);
    while (angle > (2.0 * M_PI)) {
        angle = (f32)((f64)angle - (2.0 * M_PI));
    }
    while (angle < (-2.0 * M_PI)) {
        angle += (2.0 * M_PI);
    }
    work->angle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    return angularDifference;
}

/* Unused helper that reports whether the actor action is one or two. */
static inline s32 M669StageTargetActionReady(OMOBJ *object, M669ActorWork *work)
{
    if (work->action == 2) {

        return 1;
    }
    if (work->action == 1) {

        return 1;
    }
    return 0;
}

/* Turns the stage model toward the nearest player's position relative to the target. */
void fn_1_D2EC(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF direction = { 0.0f, 0.0f, 0.0f };

    M669MovingStageStep(work);
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8U) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    {
        HuVecF fixedTarget = { 0.0f, 0.0f, 0.0f };
        HuVecF targetDirection;
        HuVecF nearestDirection;
        HuVecF nearestPlayerPosition;
        HuVecF candidateDirection;
        HuVecF playerPosition;
        M669ImpactWork *playerWork;
        f32 minimumDistance;
        f32 distance;
        s32 index;
        minimumDistance = 3.4028235e38f;
        for (index = 0; index < 3; index++) {
            playerWork = fn_1_4FC0(index);
            fn_80086BC8(playerWork->header.player->actor, &playerPosition);
            M669VecSubtract(&candidateDirection, &work->position, &playerPosition);
            distance = M669VecLengthSquared(&candidateDirection);
            if (distance < minimumDistance) {
                minimumDistance = distance;
                nearestDirection = candidateDirection;
                nearestPlayerPosition = playerPosition;
            }
        }
        fn_1_554(&nearestDirection, &nearestDirection);
        M669VecSubtract(&targetDirection, &fixedTarget, &nearestPlayerPosition);
        fn_1_554(&targetDirection, &targetDirection);
        M669VecAdd(&direction, &targetDirection, &nearestDirection);
        fn_1_554(&direction, &direction);
        M669MovingStageSteer(work, atan2f(direction.x, direction.z), 20.0f);
        work->frameVelocity.x =
            work->frameVelocity.x + ((f64) work->motionSpeed * sin(work->angle));
        work->frameVelocity.z += (f64)work->motionSpeed * cos(work->angle);
        work->timer++;
        if (work->timer >= work->frameCount) {
            fn_1_248(object, fn_1_AAAC);
            return;
        }
    }
}

/* Positions the model, then starts the next motion stage after its wait ends. */
void fn_1_D830(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->frameCount = 60;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (2.0f + 4.0f * frandf());
    work->motionTime = 3.1415927f;
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_E0C0);
}

/* Positions the model, then starts the next motion stage after its wait ends. */
void fn_1_DA54(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->frameCount = 60;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (2.0f + 4.0f * frandf());
    work->motionTime = 0.0f;
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_E0C0);
}

/* Positions the model, then starts the next motion stage after its wait ends. */
void fn_1_DC78(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->frameCount = 60;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (2.0f + 4.0f * frandf());
    work->motionTime = -1.5707964f;
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_E0C0);
}

/* Positions the model, then starts the next motion stage after its wait ends. */
void fn_1_DE9C(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 2) {
        work->motion = 2;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->frameCount = 60;
    work->timer = 0;
    work->motionSpeed = 2.0f *
        (2.0f + 4.0f * frandf());
    work->motionTime = 1.5707964f;
    fn_8002D86C(work->model, work->motionSpeed / 3.0f);
    fn_1_248(object, fn_1_E0C0);
}

void fn_1_E968(u16 *response);

void fn_1_E9C4(M669PlayerSetupWork *work, s16 motion, float blend, u32 attr);

/* Moves the stage actor along its heading and turns its model toward travel. */
void fn_1_E0C0(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position;
    HuVecF currentDirection;
    HuVecF initialDirection;
    f32 angularDifference;
    f32 facingAngle;
    f32 cross;
    f32 angleDelta;
    f32 angle;

    position = work->position;
    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    facingAngle = work->motionTime;
    fn_1_31C(&initialDirection, (f32)sin(facingAngle),
        0.0f, (f32)cos(facingAngle));
    fn_1_31C(&currentDirection, (f32)sin(work->angle),
        0.0f, (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&initialDirection, &currentDirection));
    cross = initialDirection.x * currentDirection.z -
        initialDirection.z * currentDirection.x;
    angle = work->angle;
    if (cross >= 0.0f) {
        angleDelta = angularDifference / 10.0f;
    } else {
        angleDelta = -(angularDifference / 10.0f);
    }
    angle += angleDelta;
    while (angle > (2.0 * M_PI)) {
        angle = (f32)((f64)angle - (2.0 * M_PI));
    }
    while (angle < (-2.0 * M_PI)) {
        angle += (2.0 * M_PI);
    }
    work->angle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
    work->frameVelocity.x += (f64)work->motionSpeed * sin(work->angle);
    work->frameVelocity.z += (f64)work->motionSpeed * cos(work->angle);
    work->timer++;
    if (work->timer >= work->frameCount) {
        fn_1_248(object, fn_1_AAAC);
        return;
    }
}

/* Restores the stage position, then advances the active motion callback. */
void fn_1_E474(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position = work->position;

    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
                  fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->action == 2) {
        fn_1_248(object, fn_1_B1C0);
        return;
    }
    if (work->action == 1) {
        fn_1_248(object, fn_1_AD6C);
        return;
    }
    if (fn_80071A8C() == 8) {
        fn_1_248(object, fn_1_E648);
        return;
    }
    if (work->motion != 1) {
        work->motion = 1;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    work->frameCount = 60;
    work->timer = 0;
    fn_8002D86C(work->model, 1.0f);
    fn_1_248(object, fn_1_C040);
}

/* Updates the moving actor's position and turns it toward the forward heading. */
void fn_1_E648(OMOBJ *object)
{
    M669ActorWork *work = object->data;
    HuVecF position;
    HuVecF currentDirection;
    HuVecF initialDirection;
    f32 angularDifference;
    f32 cross;
    f32 angleDelta;
    f32 angle;

    position = work->position;
    position.y = 0.01f;
    fn_80026790(work->positionModel, &position);
    if (work->motionComplete != 0) {
        if (fabsf(fn_8002D4E0(work->motionModel) -
            fn_8002D530(work->motionModel)) <= 0.01f) {
            work->motionComplete = 0;
            fn_800269E8(work->motionModel, (1 << 0));
        }
    }
    if (work->motion != 0) {
        work->motion = 0;
        fn_8002C95C(work->model, work->motionIds[work->motion]);
    }
    fn_8002D86C(work->model, 1.0f);
    fn_1_31C(&initialDirection, (f32)sin(0.0),
        0.0f, (f32)cos(0.0));
    fn_1_31C(&currentDirection, (f32)sin(work->angle),
        0.0f, (f32)cos(work->angle));
    angularDifference = fn_1_298(M669VecDot(&initialDirection, &currentDirection));
    cross = initialDirection.x * currentDirection.z -
        initialDirection.z * currentDirection.x;
    angle = work->angle;
    if (cross >= 0.0f) {
        angleDelta = angularDifference / 30.0f;
    } else {
        angleDelta = -(angularDifference / 30.0f);
    }
    angle += angleDelta;
    while (angle > (2.0 * M_PI)) {
        angle = (f32)((f64)angle - (2.0 * M_PI));
    }
    while (angle < (-2.0 * M_PI)) {
        angle += (2.0 * M_PI);
    }
    work->angle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)((f64)(180.0f * work->angle) / M_PI),
        0.0f);
}

typedef struct M669MicrophoneCandidate {
    u16 score;
    s16 count;
    s16 *result;
} M669MicrophoneCandidate;

typedef struct M669MicrophoneResponse {
    s16 status;
    u16 score;
    s16 count;
    s16 *result;
    s32 word;
    M669MicrophoneCandidate candidates[10];
} M669MicrophoneResponse;

/* Microphone listener: rejects failed or empty responses and saves the word only above the score
 * threshold. */
void fn_1_E968(u16 *response) {
    if ((((M669MicrophoneResponse *)response)->status != 0) ||
        (((M669MicrophoneResponse *)response)->count == 0)) {
        return;
    }
    if (((M669MicrophoneResponse *)response)->score < 0xBB8U) {
        lbl_1_data_17E = M669_NO_WORD;
        return;
    }
    lbl_1_data_17E = (u16)*((M669MicrophoneResponse *)response)->result;
}

/* Changes the microphone player motion only when a different motion index is requested. */
void fn_1_E9C4(M669PlayerSetupWork *work, s16 motion, float blend, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        fn_8005C2C8(work->header.character, work->header.player->omObj->mtnId[work->motion],
            0.0f, blend, attr);
    }
}

extern void fn_1_EC3C(OMOBJ *);

/* Creates the player actor and schedules its setup callback. */
void fn_1_EA4C(OMOBJ *object)
{
    M669PlayerSetupWork *work = object->data;

    work->header.character = lbl_80265728[work->header.playerIndex].charNo;
    {
        MGACTOR_PARAM params = { 0.0f, 0.0f, 0, 0, NULL, 0, 0, NULL };
        params.height = 150.0f;
        params.radius = 40.0f;
        params.param = work->header.playerIndex;
        params.type = 0;
        params.attr = 0;
        params.correctHookParam = 0;
        params.narrowHook = NULL;
        params.correctHook = NULL;
        work->header.player =
            fn_80080728(work->header.playerIndex, &params, 2, 1,
                           ((1 << 0) | (1 << 2) | (1 << 5)),
                           &lbl_1_data_80[0]);
        fn_80086CBC(work->header.player->actor, 0.0f);
        fn_8007E818(work->header.player);
        fn_8007ED60(work->header.player);
        fn_800805CC(work->header.player->actor, 6);
        {
            HuVecF position = { 0.0f, 300.0f, -680.0f };
            fn_80086B04(work->header.player->actor, &position);
            work->header.state = -1;
            work->motion = -1;
            if (work->motion != 0) {
                work->motion = 0;
                fn_8005C2C8(work->header.character,
                                   work->header.player->omObj->mtnId[work->motion], 0.0f, 0.0f,
                                   ((1 << 30) | (1 << 0)));
            }
            work->header.timer = 0;
            fn_1_248(object, fn_1_EC3C);
        }
    }
}

void fn_1_EE80(OMOBJ *self);

/* Advances the actor's setup and plays the stage event cue at its current position. */
void fn_1_EC3C(OMOBJ *object)
{
    HuVecF position;
    HuVecF screenPosition;
    f32 screenPan;
    f32 clampedPan;
    f32 upperPan;
    M669HerdWork *countWorkA;
    M669HerdWork *countWorkB;
    M669HerdWork *positionWork;
    M669PlayerSetupWork *work = object->data;

    work->header.timer++;
    if (work->header.timer == 20) {
        s32 eventCount;

        if (work->motion != 12) {
            work->motion = 12;
            fn_8005C2C8(work->header.character,
                               work->header.player->omObj->mtnId[work->motion],
                               0.0f, 4.0f, 0);
        }
        countWorkA = lbl_1_bss_CC;
        countWorkB = lbl_1_bss_CC;
        eventCount = countWorkB->radialCounts[0] + countWorkA->radialCounts[1];
        if (eventCount != 0) {
            positionWork = lbl_1_bss_CC;
            position = positionWork->position;
            Hu3D3Dto2D(&position, 1, &screenPosition);
            screenPan = 64.0f * (screenPosition.x / 576.0f);
            if (screenPan < 0.0f) {
                clampedPan = 0.0f;
            } else {
                if (screenPan > 64.0f) {
                    upperPan = 64.0f;
                } else {
                    upperPan = screenPan;
                }
                clampedPan = upperPan;
            }
            screenPan = clampedPan;
            screenPan += 32.0f;
            if (eventCount <= 5) {
                HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_SMALL, screenPan);
            } else if (eventCount <= 10) {
                HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_MEDIUM, screenPan);
            } else if (eventCount <= 25) {
                HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_LARGE, screenPan);
            } else if (eventCount <= 50) {
                HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_HUGE, screenPan);
            }
        }
        fn_1_248(object, fn_1_EE80);
        return;
    }
}

void fn_1_EF10(OMOBJ *object);

/* Starts the microphone listener when play mode begins, then installs the input-selection
 * callback. */
void fn_1_EE80(OMOBJ *self) {
    M669PlayerSetupWork *sequence = self->data;

    if (fn_80071A8C() == 5) {
        HuMCListenerCreate(lbl_1_bss_8, fn_1_E968,
                           (u8)lbl_80265728[sequence->header.playerIndex].padNo);
        sequence->motion = -1;
        fn_1_248(self, fn_1_EF10);
        return;
    }
}

void fn_1_10A74(OMOBJ *object);

void fn_1_FAFC(OMOBJ *object);

void fn_1_EFB0(OMOBJ *object);

/* Dispatches microphone-player input to human or computer control, or to listener shutdown at
 * finish. */
void fn_1_EF10(OMOBJ *object) {
    M669PlayerSetupWork *work = object->data;
    if (fn_80071A8C() == 6) {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_10A74);
        return;
    }
    if (lbl_80265728[work->header.playerIndex].type != 0) {
        fn_1_248(object, fn_1_FAFC);
        return;
    }
    fn_1_248(object, fn_1_EFB0);
}

void fn_1_1097C(OMOBJ *object);

static inline M669HerdWork *M669StatusGet(void)
{
    return lbl_1_bss_CC;
}

/* For a human microphone command, chooses a herd-count sound and pans it from the herd center. */
static inline void fn_1_88A0_for_EFB0(void)
{
    f32 limitedPan;
    f32 clampedPan;
    f32 rawPan;
    M669HerdWork *positionWork;
    M669HerdWork *countWorkB;
    M669HerdWork *countWorkA;
    HuVecF screenPosition;
    HuVecF position;
    s32 eventCount;

    countWorkA = lbl_1_bss_CC;
    countWorkB = lbl_1_bss_CC;
    eventCount = countWorkB->radialCounts[0] + countWorkA->radialCounts[1];
    if (eventCount == 0) {
        return;
    }
    positionWork = lbl_1_bss_CC;
    position = positionWork->position;
    Hu3D3Dto2D(&position, 1, &screenPosition);
    rawPan = 64.0f * (screenPosition.x / 576.0f);
    if (rawPan < 0.0f) {
        clampedPan = 0.0f;
    } else {
        if (rawPan > 64.0f) {
            limitedPan = 64.0f;
        } else {
            limitedPan = rawPan;
        }
        clampedPan = limitedPan;
    }
    rawPan = clampedPan;
    rawPan += 32.0f;
    if (eventCount <= 5) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_SMALL, (s16)rawPan);
        return;
    }
    if (eventCount <= 10) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_MEDIUM, (s16)rawPan);
        return;
    }
    if (eventCount <= 25) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_LARGE, (s16)rawPan);
        return;
    }
    if (eventCount <= 50) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_HUGE, (s16)rawPan);
    }
}

/* Updates a player's stage sequence and plays its status sound. */
void fn_1_EFB0(OMOBJ *object)
{
    M669PlayerSetupWork * work = object->data;
    if ((u32) fn_80071A8C() == 6)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, (OMOBJ_FUNC) fn_1_10A74);
        return;
    }
    if (lbl_1_data_17E == 0)
    {
        work->header.state = 0;
        if (work->motion != 13)
        {
            work->motion = 13;
            fn_8005C2C8(work->header.character,
                               work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, 0U);
        }
        fn_1_88A0_for_EFB0();
    }
    else
    {
        if (lbl_1_data_17E == 1)
        {
            work->header.state = 1;
            if (work->motion != 13)
            {
                work->motion = 13;
                fn_8005C2C8(work->header.character,
                                   work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, 0U);
            }
            fn_1_88A0_for_EFB0();
        }
        else
        {
            if (lbl_1_data_17E == 2)
            {
                work->header.state = 3;
                if (work->motion != 13)
                {
                    work->motion = 13;
                    fn_8005C2C8(work->header.character,
                                       work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f,
                                       0U);
                }
                fn_1_88A0_for_EFB0();
            }
            else
            {
                if (lbl_1_data_17E == 3)
                {
                    work->header.state = 2;
                    if (work->motion != 13)
                    {
                        work->motion = 13;
                        fn_8005C2C8(work->header.character,
                                           work->header.player->omObj->mtnId[work->motion], 0.0f,
                                           4.0f, 0U);
                    }
                    fn_1_88A0_for_EFB0();
                }
                else
                {
                    if (lbl_1_data_17E == 4)
                    {
                        work->header.state = 5;
                        if (work->motion != 13)
                        {
                            work->motion = 13;
                            fn_8005C2C8(work->header.character,
                                               work->header.player->omObj->mtnId[work->motion],
                                               0.0f, 4.0f, 0U);
                        }
                        fn_1_88A0_for_EFB0();
                    }
                    else
                    {
                        if (lbl_1_data_17E == 5)
                        {
                            work->header.state = 6;
                            if (work->motion != 13)
                            {
                                work->motion = 13;
                                fn_8005C2C8(work->header.character,
                                                   work->header.player->omObj->mtnId[work->motion],
                                                   0.0f, 4.0f, 0U);
                            }
                        }
                    }
                }
            }
        }
    }
    if (work->header.state != - 1)
    {
        lbl_1_data_17E = M669_NO_WORD;
        work->header.timer = 60;
        fn_1_248(object, fn_1_1097C);
        return;
    }
}

/* For a computer-selected command, chooses a herd-count sound and pans it from the herd center. */
static inline void M669SequenceSound(void)
{
    f32 rawPan;
    HuVecF screenPosition;
    HuVecF position;
    s32 eventCount;

    eventCount = M669StatusGet()->radialCounts[0] + M669StatusGet()->radialCounts[1];
    if (eventCount == 0) {
        return;
    }
    position = M669StatusGet()->position;
    Hu3D3Dto2D(&position, 1, &screenPosition);
    rawPan = 64.0f * (screenPosition.x / 576.0f);
    rawPan = rawPan < 0.0f ? 0.0f :
        rawPan > 64.0f ? 64.0f : rawPan;
    rawPan += 32.0f;
    if (eventCount <= 5) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_SMALL, (s16)rawPan);
        return;
    }
    if (eventCount <= 10) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_MEDIUM, (s16)rawPan);
        return;
    }
    if (eventCount <= 25) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_LARGE, (s16)rawPan);
        return;
    }
    if (eventCount <= 50) {
        HuAudFXPlayPan(MSM_SE_MG669_HERD_CROWD_HUGE, (s16)rawPan);
    }
}

/* Stores a computer-selected command, starts its character motion and plays the herd-count
 * sound. */
static inline void M669SequenceStart(M669PlayerSetupWork *work, s16 sequence)
{
    work->header.state = sequence;
    if (work->motion != 13) {
        work->motion = 13;
        fn_8005C2C8(work->header.character, work->header.player->omObj->mtnId[work->motion],
                           0.0f, 4.0f, 0U);
    }
    M669SequenceSound();
}

static inline M669HerdWork *M669SteeringWorkGet(void)
{
    return lbl_1_bss_CC;
}

static inline M669PlayerAreaWork *M669PlayerListGet(void)
{
    return lbl_1_bss_38;
}

/* During computer microphone control, chooses a command from difficulty, herd distribution and
 * player proximity. */
void fn_1_FAFC(OMOBJ *object)
{
    M669PlayerSetupWork * work = object->data;
    f32 sequenceChance[4] = { 0.125f, 0.32f, 0.7f, 2.8f };
    M669Sphere eventCenter;
    M669Sphere playerPoints[3];
    M669HerdWork *centerWork;
    M669ImpactWork *playerWork;
    s16 difficulty;
    s16 firstDirectionMax;
    s16 secondDirectionMax;
    s32 index;
    difficulty = (s16) lbl_80265728[work->header.playerIndex].comDif;
    if ((u32) fn_80071A8C() == 6)
    {
        fn_8007E818(work->header.player);
        fn_1_248(object, (OMOBJ_FUNC) fn_1_10A74);
        return;
    }
    if (60.0f * frandf() <= sequenceChance[difficulty])
    {
        centerWork = lbl_1_bss_CC;
        eventCenter.center = centerWork->position;
        eventCenter.radius = 300.0f;
        for (index = 0; index < 3; ++index)
        {
            playerWork = lbl_1_bss_2C[index];
            fn_80086BC8(playerWork->header.player->actor, & playerPoints[index].center);
            playerPoints[index].radius = 100.0f;
        }
        if (M669PlayerListGet()->hitCount > 0 && M669PlayerListGet()->hitCount % 10 == 0) {
            work->header.state = 6;
            if (work->motion != 13)
            {
                work->motion = 13;
                fn_8005C2C8(work->header.character,
                                   work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, 0U);
            }
        } else {
            if (M669SteeringWorkGet()->radialCounts[0] +
                    M669SteeringWorkGet()->radialCounts[1] <= 10 &&
                (fn_1_2C24(&eventCenter, &playerPoints[0]) <= 0.0f ||
                 fn_1_2C24(&eventCenter, &playerPoints[1]) <= 0.0f ||
                 fn_1_2C24(&eventCenter, &playerPoints[2]) <= 0.0f)) {
                M669SequenceStart(work, 5);
            } else {
                firstDirectionMax =
                    M669StatusGet()->zSideCounts[0] > M669StatusGet()->zSideCounts[1]
                        ? M669StatusGet()->zSideCounts[0]
                        : M669StatusGet()->zSideCounts[1];
                secondDirectionMax =
                    M669StatusGet()->xSideCounts[0] > M669StatusGet()->xSideCounts[1]
                        ? M669StatusGet()->xSideCounts[0]
                        : M669StatusGet()->xSideCounts[1];
                if (firstDirectionMax > secondDirectionMax)
                {
                    if (M669StatusGet()->zSideCounts[0] >
                        M669StatusGet()->zSideCounts[1]) {
                        M669SequenceStart(work, 1);
                    } else {
                        M669SequenceStart(work, 0);
                    }
                }
                else
                {
                    if (M669StatusGet()->xSideCounts[0] >
                        M669StatusGet()->xSideCounts[1]) {
                        M669SequenceStart(work, 3);
                    } else {
                        M669SequenceStart(work, 2);
                    }
                }
            }
        }
    }
    if (work->header.state != - 1)
    {
        work->header.timer = 60;
        fn_1_248(object, fn_1_1097C);
        return;
    }
}

void fn_1_10AB8(OMOBJ *object);

/* Holds a microphone command until its timer expires, or hands off to listener shutdown at
 * finish. */
void fn_1_1097C(OMOBJ *object)
{
    M669PlayerSetupWork *work = object->data;

    if (fn_80071A8C() == 6) {
        fn_8007E818(work->header.player);
        fn_1_248(object, fn_1_10A74);
        return;
    }
    work->header.timer--;
    if (work->header.timer <= 0) {
        if (work->motion != 0) {
            work->motion = 0;
            fn_8005C2C8(work->header.character,
                               work->header.player->omObj->mtnId[work->motion], 0.0f, 10.0f,
                               ((1 << 30) | (1 << 0)));
        }
        work->header.state = -1;
        lbl_1_data_17E = M669_NO_WORD;
        fn_1_248(object, fn_1_EF10);
        return;
    }
}

/* Stops microphone listening at finish and installs the character result callback. */
void fn_1_10A74(OMOBJ *object) {
    M669PlayerSetupWork *objectData;

    objectData = object->data;
    HuMCListenerKill();
    fn_1_248(object, fn_1_10AB8);
}

/* In result mode, selects motion by comparing the winning-group value with the player index. */
void fn_1_10AB8(OMOBJ *object)
{
    M669PlayerSetupWork *work = object->data;

    work->header.state = -1;
    if (work->motion != 0) {
        work->motion = 0;
        fn_8005C2C8(work->header.character, work->header.player->omObj->mtnId[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    if (fn_80071A8C() == 8) {
        if (lbl_1_data_168 == work->header.groupIndex) {
            if (work->motion != 14) {
                work->motion = 14;
                fn_8005C2C8(work->header.character,
                                   work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, 0);
            }
        } else if (work->motion != 15) {
            work->motion = 15;
            fn_8005C2C8(work->header.character,
                               work->header.player->omObj->mtnId[work->motion], 0.0f, 4.0f, 0);
        }
        fn_1_248(object, 0);
        return;
    }
}

void fn_1_10D48(OMOBJ *);

void fn_1_10DE8(OMOBJ *object);

void fn_1_11134(OMOBJ *object);

typedef struct M669SequenceWaitWork {
    s16 actorIndices[3];
    s16 actorIndex;
    s32 framesRemaining;
} M669SequenceWaitWork;


/* Creates three auxiliary actors during setup and shuffles the order in which they will start. */
void fn_1_10C24(OMOBJ *object)
{
    M669SequenceWaitWork *work = object->data;
    s32 index;
    s8 randomIndex;
    s16 actorIndex;

    for (index = 0; index < 3; index++) {
        lbl_1_bss_1C[index] = fn_1_1A8(10, 52, fn_1_11134);
        lbl_1_bss_1C[index]->state = index;
    }
    for (index = 0; index < 3; index++) {
        work->actorIndices[index] = index;
    }
    for (index = 0; index < 3; index++) {
        randomIndex = rand8() % 3;
        actorIndex = work->actorIndices[index];
        work->actorIndices[index] = work->actorIndices[randomIndex];
        work->actorIndices[randomIndex] = actorIndex;
    }
    work->actorIndex = 0;
    fn_1_248(object, fn_1_10D48);
}

/* Activates the selected motion state and starts its sixty-frame wait. */
void fn_1_10D48(OMOBJ *object)
{
    M669SequenceWaitWork *work = object->data;
    M669PlayerSetupWork *motionState = lbl_1_bss_28;
    s32 actorIndex;
    M669MotionWork *motion;
    M669MotionWork *motionCopy;
    if (motionState->header.state == 6) {
        actorIndex = work->actorIndices[work->actorIndex];
        motion = lbl_1_bss_1C[actorIndex];
        motionCopy = motion;
        motionCopy->activationRequested = 1;
        work->actorIndex++;
        work->framesRemaining = 60;
        fn_1_248(object, fn_1_10DE8);
        return;
    }
}

/* After a scheduled delay, starts the next auxiliary actor or disables the completed scheduler. */
void fn_1_10DE8(OMOBJ *object) {
    M669SequenceWaitWork *work = object->data;

    work->framesRemaining--;
    if (work->framesRemaining <= 0) {
        if (work->actorIndex == 3) {
            fn_1_248(object, NULL);
            return;
        }
        fn_1_248(object, fn_1_10D48);
        return;
    }
}

/* Blends to a requested auxiliary-actor motion only when its motion index changes. */
void fn_1_10E64(M669MotionWork *work, s16 motion, float blend, u32 attr)
{
    if (work->motion != motion) {
        work->motion = motion;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, blend, attr);
    }
}

/* Turns a model toward a requested radian heading and returns the angular step taken this
 * update. */
f32 fn_1_10EE4(M669MotionWork *work, f32 targetAngle, f32 frames)
{
    HuVecF rotation;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 initialAngle;
    f32 adjustedAngle;
    f32 angleDifference;
    f32 crossY;

    fn_80026874(work->model, &rotation);
    initialAngle = (f32)(M_PI *
        (rotation.y / 180.0f));
    fn_1_31C(&targetDirection, (f32)sin(targetAngle),
        0.0f, (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(initialAngle),
        0.0f, (f32)cos(initialAngle));
    angleDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    crossY = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = initialAngle;
    if (crossY >= 0.0f) {
        adjustedAngle += angleDifference / frames;
    } else if (crossY < 0.0f) {
        adjustedAngle -= angleDifference / frames;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle -= (2.0 * M_PI);
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    rotation.y = (f32)(180.0f * adjustedAngle / M_PI);
    fn_80026838(work->model, &rotation);
    return (f32)fabs(adjustedAngle - initialAngle);
}

void fn_1_113EC(OMOBJ *);

void fn_1_11C64(OMOBJ *);

void fn_1_12198(OMOBJ *);

void fn_1_126EC(OMOBJ *);

void fn_1_130C8(OMOBJ *);

void fn_1_11368(OMOBJ *object);

void fn_1_11C04(OMOBJ *object);

void fn_1_12138(OMOBJ *object);

void fn_1_1268C(OMOBJ *object);

void fn_1_12B8C(OMOBJ *object);

void fn_1_12FD0(OMOBJ *object);

void fn_1_1173C(OMOBJ *object);

void fn_1_12238(OMOBJ *object);

void fn_1_117D0(OMOBJ *object);

void fn_1_11D04(OMOBJ *object);

void fn_1_116C0(OMOBJ *object);

/* During auxiliary-actor turns, adjusts the model toward a radian heading and returns its angular
 * step. */
static inline f32 M669MotionAngleAdjust(M669MotionWork *work, f32 targetAngle,
    f32 frames)
{
    HuVecF rotation;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 initialAngle;
    f32 adjustedAngle;
    f32 angleDifference;
    f32 crossY;

    fn_80026874(work->model, &rotation);
    initialAngle = (f32)(M_PI *
        (rotation.y / 180.0f));
    fn_1_31C(&targetDirection, (f32)sin(targetAngle),
        0.0f, (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(initialAngle),
        0.0f, (f32)cos(initialAngle));
    angleDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    crossY = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = initialAngle;
    if (crossY >= 0.0f) {
        adjustedAngle += angleDifference / frames;
    } else if (crossY < 0.0f) {
        adjustedAngle -= angleDifference / frames;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle -= (2.0 * M_PI);
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    rotation.y = (f32)(180.0f * adjustedAngle / M_PI);
    fn_80026838(work->model, &rotation);
    return fabsf(adjustedAngle - initialAngle);
}

/* Creates an auxiliary actor and its following effect during setup, facing the actor toward the
 * origin. */
void fn_1_11134(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    s32 motionIndex;

    work->activationRequested = 0;
    work->model =
        fn_80025A04(fn_80008DD0(DATANUM(0x800000, 0), 0x10000000, 2));
    for (motionIndex = 0; motionIndex < 6; motionIndex++) {
        work->motionIds[motionIndex] = fn_80030458(work->model,
            fn_80008DD0(lbl_1_data_68[motionIndex], 0x10000000, 2));
    }
    work->motion = 0;
    fn_8002C95C(work->model, work->motionIds[work->motion]);
    fn_800269E8(work->model, ((1 << 30) | (1 << 0)));
    work->position = lbl_1_data_CC[work->state];
    fn_80026790(work->model, &work->position);
    {
        HuVecF origin = { 0.0f, 0.0f, 0.0f };
        HuVecF direction;
        f32 directionX;
        f32 directionZ;
        f32 targetAngle;
        f32 rotationAngle;
        M669VecSubtract(&direction, &origin, &lbl_1_data_CC[work->state]);
        directionZ = direction.z;
        directionX = direction.x;
        targetAngle = (f32)atan2(directionX, directionZ);
        rotationAngle = targetAngle;
        fn_80026808(work->model, 0.0f,
            (f32)(180.0f * rotationAngle / M_PI),
            0.0f);
        if (work->state == 0) {
            fn_80026E44(work->model);
        }
        work->secondaryWork = fn_1_1A8(10, 92, fn_1_130C8);
        work->secondaryWork->owner = work;
        fn_1_248(object, fn_1_11368);
    }
}

/* Waits for an auxiliary actor request and an idle effect, or selects the finish sequence in mode
 * six. */
void fn_1_11368(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    if (fn_80071A8C() == 6) {
        fn_1_248(object, fn_1_12C58);
        return;
    }
    if (work->activationRequested == 1 && work->secondaryWork->state == 0) {
        fn_1_248(object, fn_1_113EC);
        return;
    }
}

/* Aims the requested auxiliary action at the nearest eligible player, then starts its timed
 * motion. */
void fn_1_113EC(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    HuVecF nearestPosition;
    HuVecF actorPosition;
    HuVecF candidateDirection;
    HuVecF nearestDirection;
    HuVecF screenPosition;
    f32 nearestDistance = 3.4028235e38f;
    f32 distance;
    f32 directionX;
    f32 directionZ;
    f32 angle;
    f32 rotationAngle;
    f32 screenPan;
    f32 clampedPan;
    f32 upperPan;
    s32 index;

    for (index = 0; index < 3; index++) {
        M669ImpactWork *targetSlot = lbl_1_bss_2C[index];
        M669ImpactWork *target = targetSlot;

        if (target->header.state != 1) {
            fn_80086BC8(target->header.player->actor, &actorPosition);
            M669VecSubtract(&candidateDirection, &actorPosition, &work->position);
            distance = M669VecLengthSquared(&candidateDirection);
            if (distance < nearestDistance) {
                nearestDistance = distance;
                nearestPosition = actorPosition;
            }
        }
    }
    M669VecSubtract(&nearestDirection, &nearestPosition, &work->position);
    directionZ = nearestDirection.z;
    directionX = nearestDirection.x;
    angle = (f32)atan2(directionX, directionZ);
    rotationAngle = angle;
    fn_80026808(work->model, 0.0f,
        (f32)(180.0f * rotationAngle / M_PI),
        0.0f);
    if (work->motion != 5) {
        work->motion = 5;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, 0);
    }
    work->frameCount = 0;
    Hu3D3Dto2D(&work->position, 1, &screenPosition);
    screenPan = 64.0f * (screenPosition.x / 576.0f);
    if (screenPan < 0.0f) {
        clampedPan = 0.0f;
    } else {
        if (screenPan > 64.0f) {
            upperPan = 64.0f;
        } else {
            upperPan = screenPan;
        }
        clampedPan = upperPan;
    }
    screenPan = clampedPan;
    screenPan += 32.0f;
    HuAudFXPlayPan(MSM_SE_MG669_AUXILIARY_ACTION_START, screenPan);
    fn_1_248(object, fn_1_116C0);
}

/* Activates the auxiliary effect on frame ten and selects the next movement after 120 frames. */
void fn_1_116C0(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    work->frameCount++;
    if (work->frameCount == 10) {
        work->secondaryWork->state = 1;
        return;
    }
    if (work->frameCount >= 120) {
        fn_1_248(object, fn_1_1173C);
        return;
    }
}

/* After the timed action, selects an exit movement according to the auxiliary actor slot. */
void fn_1_1173C(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    if (work->state == 0) {
        fn_1_248(object, fn_1_12238);
        return;
    }
    if (work->state == 1) {
        fn_1_248(object, fn_1_117D0);
        return;
    }
    if (work->state == 2) {
        fn_1_248(object, fn_1_11D04);
        return;
    }
}

/* Turns the auxiliary actor away from the origin, then prepares its upward exit movement. */
void fn_1_117D0(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    HuVecF targetPosition = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;
    f32 targetAngle;
    f32 angleDifference;
    f32 directionX;
    f32 directionZ;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    M669VecSubtract(&direction, &work->position, &targetPosition);
    direction.y = 0.0f;
    fn_1_554(&direction, &direction);
    directionZ = direction.z;
    directionX = direction.x;
    targetAngle = (f32)atan2(directionX, directionZ);
    angleDifference = M669MotionAngleAdjust(work, targetAngle, 20.0f);
    if (angleDifference < 0.01f) {
        M669VecScale(&direction, &direction, 5.0f);
        fn_1_31C(&work->velocity, direction.x, 20.0f, direction.z);
        work->frameCount = 0;
        if (work->motion != 4) {
            work->motion = 4;
            fn_8002CCC0(work->model, work->motionIds[work->motion],
                0.0f, 4.0f, 0);
        }
        fn_1_248(object, fn_1_11C04);
        return;
    }
}

/* Waits 16 updates after the auxiliary actor jump motion starts, then begins moving it. */
void fn_1_11C04(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    work->frameCount++;
    if (work->frameCount > 15) {
        fn_1_248(object, fn_1_11C64);
        return;
    }
}

/* Moves the auxiliary actor upward, reduces vertical velocity and hides it as soon as ascent
 * ends. */
void fn_1_11C64(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    M669VecAdd(&work->position, &work->position, &work->velocity);
    work->velocity.y -= 0.3f;
    fn_80026790(work->model, &work->position);
    if (work->velocity.y <= 0.0f) {
        fn_800269E8(work->model, (1 << 0));
        fn_1_248(object, 0);
        return;
    }
}

/* During the second auxiliary exit, turns the model through one twentieth of the remaining angular
 * gap. */
static inline f32 fn_1_10EE4_for_11D04(M669MotionWork *work,
    f32 targetAngle, f32 frames)
{
    HuVecF rotation;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 initialAngle;
    f32 adjustedAngle;
    f32 angleDifference;
    f32 crossY;

    fn_80026874(work->model, &rotation);
    initialAngle = (f32)(M_PI *
        (rotation.y / 180.0f));
    fn_1_31C(&targetDirection, (f32)sin(targetAngle),
        0.0f, (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(initialAngle),
        0.0f, (f32)cos(initialAngle));

    angleDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    crossY = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = initialAngle;
    if (crossY >= 0.0f) {
        adjustedAngle += angleDifference / 20.0f;
    } else if (crossY < 0.0f) {
        adjustedAngle -= angleDifference / 20.0f;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle -= (2.0 * M_PI);
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    rotation.y = (f32)(180.0f * adjustedAngle / M_PI);
    fn_80026838(work->model, &rotation);
    return fabsf(adjustedAngle - initialAngle);
}

/* Turns the auxiliary actor outward, then prepares its faster horizontal and upward exit. */
void fn_1_11D04(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    HuVecF targetPosition = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;
    f32 targetAngle;
    f32 angleDifference;
    f32 directionX;
    f32 directionZ;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    M669VecSubtract(&direction, &work->position, &targetPosition);
    direction.y = 0.0f;
    fn_1_554(&direction, &direction);
    directionZ = direction.z;
    directionX = direction.x;
    targetAngle = (f32)atan2(directionX, directionZ);
    angleDifference = fn_1_10EE4_for_11D04(work, targetAngle,
        20.0f);
    if (angleDifference < 0.01f) {
        M669VecScale(&direction, &direction, 10.0f);
        fn_1_31C(&work->velocity, direction.x, 10.0f,
            direction.z);
        work->frameCount = 0;
        if (work->motion != 4) {
            work->motion = 4;
            fn_8002CCC0(work->model, work->motionIds[work->motion],
                0.0f, 4.0f, 0);
        }
        fn_1_248(object, fn_1_12138);
        return;
    }
}

/* Waits 16 updates after the second exit motion starts, then begins moving the actor. */
void fn_1_12138(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    work->frameCount++;
    if (work->frameCount > 15) {
        fn_1_248(object, fn_1_12198);
        return;
    }
}

/* Moves the second auxiliary exit and hides the actor as soon as its ascent ends. */
void fn_1_12198(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    M669VecAdd(&work->position, &work->position, &work->velocity);
    work->velocity.y -= 0.3f;
    fn_80026790(work->model, &work->position);
    if (work->velocity.y <= 0.0f) {
        fn_800269E8(work->model, (1 << 0));
        fn_1_248(object, 0);
        return;
    }
}

/* Turns the auxiliary actor perpendicular to the inward direction, then prepares its curved
 * exit. */
void fn_1_12238(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    HuVecF startPosition = { 0.0f, 0.0f, 0.0f };
    HuVecF offset = { 0.0f, 1.0f, 0.0f };
    HuVecF direction;
    HuVecF targetDirection;
    f32 targetAngle;
    f32 angleDifference;
    f32 directionX;
    f32 directionZ;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    M669VecSubtract(&direction, &startPosition, &work->position);
    fn_1_4AC(&targetDirection, &direction, &offset);
    directionZ = targetDirection.z;
    directionX = targetDirection.x;
    targetAngle = (f32)atan2(directionX, directionZ);
    angleDifference = M669MotionAngleAdjust(work, targetAngle, 20.0f);
    if (angleDifference < 0.01f) {
        fn_1_554(&targetDirection, &targetDirection);
        M669VecScale(&targetDirection, &targetDirection, 4.0f);
        fn_1_31C(&work->velocity, targetDirection.x,
            3.0f, targetDirection.z);
        work->frameCount = 0;
        if (work->motion != 4) {
            work->motion = 4;
            fn_8002CCC0(work->model, work->motionIds[work->motion],
                0.0f, 4.0f, 0);
        }
        fn_1_248(object, fn_1_1268C);
        return;
    }
}

/* Waits 16 updates after the curved exit motion starts, then installs the movement callback. */
void fn_1_1268C(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    work->frameCount++;
    if (work->frameCount > 15) {
        fn_1_248(object, fn_1_126EC);
        return;
    }
}

void fn_1_127A0(OMOBJ *object);

/* Updates the falling stage object until it reaches the floor. */
void fn_1_126EC(OMOBJ *obj)
{
    M669MotionWork *work = obj->data;

    M669VecAdd(&work->position, &work->position, &work->velocity);
    work->velocity.y -= 0.3f;
    fn_80026790(work->model, &work->position);
    if (work->position.y <= 0.0f) {
        work->position.y = 0.0f;
        fn_80026790(work->model, &work->position);
        fn_1_248(obj, fn_1_127A0);
        return;
    }
}

static f32 M669MotionAngleAdjust_inlined(M669MotionWork *work, f32 targetAngle, f32 frames);

/* During the final auxiliary exit, adjusts the model toward a radian heading and returns its
 * angular step. */
static inline f32 M669MotionAngleAdjust_inlined(M669MotionWork *work, f32 targetAngle,
    f32 frames)
{
    HuVecF rotation;
    HuVecF targetDirection;
    HuVecF currentDirection;
    f32 initialAngle;
    f32 adjustedAngle;
    f32 angleDifference;
    f32 crossY;

    fn_80026874(work->model, &rotation);
    initialAngle = (f32)(M_PI *
        (rotation.y / 180.0f));
    fn_1_31C(&targetDirection, (f32)sin(targetAngle),
        0.0f, (f32)cos(targetAngle));
    fn_1_31C(&currentDirection, (f32)sin(initialAngle),
        0.0f, (f32)cos(initialAngle));

    angleDifference = fn_1_298(M669VecDot(&targetDirection, &currentDirection));
    crossY = targetDirection.x * currentDirection.z -
        targetDirection.z * currentDirection.x;
    adjustedAngle = initialAngle;
    if (crossY >= 0.0f) {
        adjustedAngle += angleDifference / frames;
    } else if (crossY < 0.0f) {
        adjustedAngle -= angleDifference / frames;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle -= (2.0 * M_PI);
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    rotation.y = (f32)(180.0f * adjustedAngle / M_PI);
    fn_80026838(work->model, &rotation);
    return fabsf(adjustedAngle - initialAngle);
}

/* Turns the auxiliary actor outward and prepares a horizontal exit at five units per update. */
void fn_1_127A0(OMOBJ *object)
{
    M669MotionWork *work = object->data;
    HuVecF targetPosition = { 0.0f, 0.0f, 0.0f };
    HuVecF direction;
    f32 targetAngle;
    f32 angleDifference;
    f32 directionX;
    f32 directionZ;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    M669VecSubtract(&direction, &work->position, &targetPosition);
    direction.y = 0.0f;
    fn_1_554(&direction, &direction);
    directionZ = direction.z;
    directionX = direction.x;
    targetAngle = (f32)atan2(directionX, directionZ);
    angleDifference = M669MotionAngleAdjust_inlined(work, targetAngle, 20.0f);
    if (angleDifference < 0.01f) {
        M669VecScale(&direction, &direction, 5.0f);
        fn_1_31C(&work->velocity, direction.x, 0.0f, direction.z);
        work->frameCount = 0;
        fn_1_248(object, fn_1_12B8C);
        return;
    }
}

/* Moves the auxiliary actor outward and hides it after the post-incremented counter exceeds 120. */
void fn_1_12B8C(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, ((1 << 30) | (1 << 0)));
    }
    M669VecAdd(&work->position, &work->position, &work->velocity);
    fn_80026790(work->model, &work->position);
    if (work->frameCount++ > 120) {
        fn_800269E8(work->model, (1 << 0));
        fn_1_248(object, 0);
        return;
    }
}

void fn_1_12CA8(OMOBJ *obj);

/* Advances the object when the sequence enters mode seven. */
void fn_1_12C58(OMOBJ *obj)
{
    void *objectData;

    objectData = obj->data;
    if (fn_80071A8C() == 7) {
        fn_1_248(obj, fn_1_12CA8);
        return;
    }
}

/* During the finish sequence, turns the auxiliary actor toward heading zero and returns it to
 * idle. */
void fn_1_12CA8(OMOBJ *obj)
{
    M669MotionWork *work = obj->data;
    HuVecF rotation;
    HuVecF fixedDirection;
    HuVecF currentDirection;
    f32 initialAngle;
    f32 adjustedAngle;
    f32 angleDifference;
    f32 crossY;

    if (work->motion != 1) {
        work->motion = 1;
        fn_8002CCC0(work->model, work->motionIds[work->motion],
            0.0f, 4.0f, 0);
    }
    fn_80026874(work->model, &rotation);
    initialAngle = (f32)(M_PI * (rotation.y / 180.0f));
    fn_1_31C(&fixedDirection, (f32)sin(0.0),
        0.0f, (f32)cos(0.0));
    fn_1_31C(&currentDirection, (f32)sin(initialAngle),
        0.0f, (f32)cos(initialAngle));
    angleDifference = fn_1_298(M669VecDot(&fixedDirection, &currentDirection));
    crossY = fixedDirection.x * currentDirection.z -
        fixedDirection.z * currentDirection.x;
    adjustedAngle = initialAngle;
    if (crossY >= 0.0f) {
        adjustedAngle += angleDifference / 20.0f;
    } else if (crossY < 0.0f) {
        adjustedAngle -= angleDifference / 20.0f;
    }
    while (adjustedAngle > (2.0 * M_PI)) {
        adjustedAngle -= (2.0 * M_PI);
    }
    while (adjustedAngle < (-2.0 * M_PI)) {
        adjustedAngle += (2.0 * M_PI);
    }
    rotation.y = (f32)(180.0f * adjustedAngle / M_PI);
    fn_80026838(work->model, &rotation);
    if (fabsf(adjustedAngle - initialAngle) <= 0.01f) {
        if (work->motion != 0) {
            work->motion = 0;
            fn_8002CCC0(work->model, work->motionIds[work->motion],
                0.0f, 4.0f, 0);
        }
        fn_1_248(obj, fn_1_12FD0);
        return;
    }
}

/* Selects the auxiliary actor result motion in mode eight and disables its callback. */
void fn_1_12FD0(OMOBJ *object)
{
    M669MotionWork *work = object->data;

    if (fn_80071A8C() == 8) {
        if (lbl_1_data_168 == 0) {
            if (work->motion != 2) {
                work->motion = 2;
                fn_8002CCC0(work->model, work->motionIds[work->motion],
                    0.0f, 4.0f, 0);
            }
        } else if (work->motion != 3) {
            work->motion = 3;
            fn_8002CCC0(work->model, work->motionIds[work->motion],
                0.0f, 4.0f, 0);
        }
        fn_1_248(object, 0);
        return;
    }
}

void fn_1_1316C(OMOBJ *);

/* Creates the auxiliary actor following effect hidden at its owner position, then waits for
 * activation. */
void fn_1_130C8(OMOBJ *object)
{
    M669FollowingEffectWork *work = object->data;

    work->state = 0;
    work->position = work->owner->position;
    work->model =
        fn_80025A04(fn_80008DD0(DATANUM(0x800000, 13), 0x10000000, 2));
    fn_80026790(work->model, &work->position);
    fn_800269E8(work->model, (1 << 0));
    fn_80026E44(work->model);
    fn_1_248(object, fn_1_1316C);
}

extern void fn_1_133B0(OMOBJ *);

/* On effect activation, aims from its owner toward the nearest eligible player and starts
 * movement. */
void fn_1_1316C(OMOBJ *object)
{
    M669FollowingEffectWork *work = object->data;
    f32 nearestDistance;
    f32 velocityMagnitude;
    HuVecF nearestPosition;
    HuVecF velocity;
    HuVecF actorPosition;
    HuVecF distanceDirection;
    s32 index;

    if (work->state == 1) {
        nearestDistance = 3.4028235e38f;

        for (index = 0; index < 3; index++) {
            M669ImpactWork *targetSlot = lbl_1_bss_2C[index];
            M669ImpactWork *target = targetSlot;

            if (target->header.state != 1) {
                f32 distance;

                fn_80086BC8(target->header.player->actor, &actorPosition);
                M669VecSubtract(&distanceDirection, &actorPosition, &work->owner->position);
                distance = M669VecLengthSquared(&distanceDirection);
                if (distance < nearestDistance) {
                    nearestDistance = distance;
                    nearestPosition = actorPosition;
                }
            }
        }
        work->previousPosition = work->owner->position;
        work->position = work->previousPosition;
        work->targetPosition = nearestPosition;
        work->targetPosition.y += 40.0f;

        M669VecSubtract(&velocity, &work->targetPosition, &work->owner->position);
        velocityMagnitude = fn_1_554(&velocity, &velocity);
        work->frameVelocity = velocity;
        M669VecScale(&work->frameVelocity, &work->frameVelocity, 40.0f);

        fn_1_31C(&work->velocity, 0.0f,
            0.0f, 0.0f);
        fn_1_31C(&work->frameDisplacement, 0.0f,
            0.0f, 0.0f);
        fn_80026A44(work->model, (1 << 0));
        fn_80026790(work->model, &work->position);
        work->hitPlayer = 0;
        work->enteredArena = 0;
        fn_1_248(object, fn_1_133B0);

        return;
    }
}

extern s32 fn_1_286C(M669Sphere *, HuVecF *, M669Sphere *, HuVecF *, f32, f32 *,
                     HuVecF *);

extern void fn_1_137A0(OMOBJ *);

/* Advances the moving effect toward a player and resolves its contact response. */
void fn_1_133B0(OMOBJ *object)
{
    M669FollowingEffectWork *work = object->data;
    HuVecF movementOffset = { 0.0f, 0.0f, 0.0f };
    HuVecF floorPosition = { 0.0f, 0.0f, 0.0f };
    M669ImpactWork *playerWork;
    M669ImpactWork *nearestPlayer;
    f32 bestHitTime;
    f32 distance;
    s32 index;
    M669VecAdd(&work->velocity, &work->velocity, &work->frameVelocity);
    M669VecAdd(&work->velocity, &work->velocity, &movementOffset);
    work->frameDisplacement = work->velocity;
    work->frameVelocity.x = work->frameVelocity.y = work->frameVelocity.z = 0.0f;
    if (work->hitPlayer == 0) {
        HuVecF collisionOffset = { 0.0f, 0.0f, 0.0f };
        M669Sphere movingSphere;
        M669Sphere playerSphere;
        HuVecF hitDirection;
        HuVecF playerPosition;
        f32 hitTime;

        bestHitTime = 1.0f;
        nearestPlayer = 0;
        movingSphere.center = work->position;
        movingSphere.radius = 30.0f;
        for (index = 0; index < 3; ++index) {
            playerWork = fn_1_4FC0(index);
            if (playerWork->header.state == 0) {
                fn_80086BC8(playerWork->header.player->actor, &playerPosition);
                playerSphere.center = playerPosition;
                playerSphere.radius = 40.0f;
                if (fn_1_286C(&movingSphere, &work->frameDisplacement, &playerSphere,
                              &collisionOffset, 1.0f, &hitTime, &hitDirection) != 0 &&
                    hitTime <= bestHitTime) {
                    bestHitTime = hitTime;
                    nearestPlayer = playerWork;
                }
            }
        }
        if (nearestPlayer != 0) {
            nearestPlayer->header.state = 1;
            nearestPlayer->velocity = work->frameDisplacement;
            fn_1_31C(&work->velocity, 0.0f, work->velocity.y, 0.0f);
            fn_1_554(&work->frameDisplacement, &work->frameDisplacement);
            M669VecScale(&work->frameDisplacement, &work->frameDisplacement, bestHitTime);
            work->hitPlayer = 1;
        }
    }
    work->previousPosition = work->position;
    M669VecAdd(&work->position, &work->position, &work->frameDisplacement);
    if (work->enteredArena == 0) {
        HuVecF floorDirection;
        M669VecSubtract(&floorDirection, &work->position, &floorPosition);
        if (fn_1_32C(&floorDirection) < 770.0f) {
            work->enteredArena = 1;
        }
    }
    if (work->enteredArena != 0) {
        HuVecF pushDirection;
        M669Sphere effectSphere;
        M669Sphere floorSphere;

        floorSphere.center = floorPosition;
        floorSphere.radius = 800.0f;
        effectSphere.center = work->position;
        effectSphere.radius = 30.0f;
        M669VecSubtract(&pushDirection, &floorSphere.center, &effectSphere.center);
        distance = fn_1_554(&pushDirection, &pushDirection);
        if (distance > floorSphere.radius - effectSphere.radius) {
            M669VecScale(&pushDirection, &pushDirection,
                     distance - (floorSphere.radius - effectSphere.radius));
            M669VecAdd(&work->position, &work->position, &pushDirection);
        }
    }
    fn_80026790(work->model, &work->position);
    if (work->position.y < 0.0f) {
        work->position.y = 0.0f;
        fn_80026790(work->model, &work->position);
        work->frameCount = 0;
        fn_1_248(object, fn_1_137A0);
        return;
    }
}

/* Blinks the effect after 30 updates and returns it to its hidden activation wait at frame 90. */
void fn_1_137A0(OMOBJ *object)
{
    M669FollowingEffectWork *work = object->data;

    work->frameCount++;
    if (work->frameCount > 30 && (work->frameCount & 1) != 0) {
        fn_800269E8(work->model, (1 << 0));
    } else {
        fn_80026A44(work->model, (1 << 0));
    }
    if (work->frameCount >= 90) {
        fn_800269E8(work->model, (1 << 0));
        work->state = 0;
        fn_1_248(object, fn_1_1316C);
        return;
    }
}

/* Turns the player away from nearby rivals when they overlap its movement space. */
void fn_1_1384C(M669ImpactWork *object, f32 targetAngle, f32 turnRate,
    f32 pushMagnitude)
{
    HuVecF targetDirection;
    HuVecF currentDirection;
    M669Sphere self;
    M669Sphere other;
    HuVecF awayDirection;
    f32 currentAngle;
    f32 angleDifference;
    f32 cross;
    f32 adjustedAngle;
    s32 index;
    M669ImpactWork *target;
    M669ImpactWork *targetCopy;

    if (object->header.player->mode != 3) {
        currentAngle = (f32)(M_PI *
            (object->header.player->actor->rotY / 180.0f));
        fn_1_31C(&targetDirection, (f32)sin(targetAngle),
            0.0f, (f32)cos(targetAngle));
        fn_80086BC8(object->header.player->actor, &self.center);
        self.radius = 80.0f;
        for (index = 0; index < 3; index++) {
            target = lbl_1_bss_2C[index];
            targetCopy = target;
            if (object != targetCopy) {
                fn_80086BC8(targetCopy->header.player->actor, &other.center);
                other.radius = 80.0f;
                if (fn_1_2C24(&self, &other) <= 0.0f) {
                    M669VecSubtract(&awayDirection, &self.center, &other.center);
                    fn_1_554(&awayDirection, &awayDirection);
                    M669VecAdd(&targetDirection, &targetDirection,
                        &awayDirection);
                    fn_1_554(&targetDirection, &targetDirection);
                }
            }
        }
        fn_1_31C(&currentDirection, (f32)sin(currentAngle),
            0.0f, (f32)cos(currentAngle));
        angleDifference = fn_1_298(
            M669VecDot(&targetDirection, &currentDirection));
        cross = targetDirection.x * currentDirection.z -
            targetDirection.z * currentDirection.x;
        adjustedAngle = currentAngle;
        if (cross >= 0.0f) {
            adjustedAngle += angleDifference * turnRate;
        } else if (cross < 0.0f) {
            adjustedAngle -= angleDifference * turnRate;
        }
        while (adjustedAngle > (2.0 * M_PI)) {
            adjustedAngle = (f32)((f64)adjustedAngle - (2.0 * M_PI));
        }
        while (adjustedAngle < (-2.0 * M_PI)) {
            adjustedAngle += (2.0 * M_PI);
        }
        object->header.player->actor->rotY = (f32)((f64)(180.0f * adjustedAngle) /
            M_PI);
        fn_1_31C(&object->header.player->actor->push, 0.0f,
            0.0f, pushMagnitude);
    }
}

/* Selects a steering response from nearby particles or active area counts. */
s32 fn_1_13BA0(M669ImpactWork *object)
{
    M669Bounds playerBounds;
    M669Bounds particleBounds;
    f32 difficultyTurn[4] = { 50.0f, 40.0f, 20.0f, 5.0f };
    f32 difficultyChance[4] = { 6.25f, 25.0f, 56.25f, 100.0f };
    HuVecF center = { 0.0f, 0.0f, 0.0f };
    s16 difficulty = (s16)lbl_80265728[object->header.playerIndex].comDif;
    M669HerdWork *work = M669SteeringWorkGet();
    M669PlayerAreaWork *players = M669PlayerListGet();
    HuVecF playerPosition;
    s32 i;

    fn_80086BC8(object->header.player->actor, &playerPosition);
    playerBounds.min = playerBounds.max = playerPosition;
    playerBounds.min.x -= 300.0f;
    playerBounds.min.y -= 300.0f;
    playerBounds.min.z -= 300.0f;
    playerBounds.max.x += 300.0f;
    playerBounds.max.y += 300.0f;
    playerBounds.max.z += 300.0f;
    if (work->radialCounts[0] < work->radialCounts[1]) {
        HuVecF facing;
        f32 bestDot = -1.0f;
        M669ActorWork *bestParticle = NULL;

        M669VecSubtract(&facing, &playerPosition, &center);
        fn_1_554(&facing, &facing);
        for (i = 0; i < 36; i++) {
            M669ActorWork *particle = M669ParticleGet(i);
            HuVecF direction;
            f32 distance;
            f32 dot;

            if (particle->action == 2 || particle->action == 3) {
                continue;
            }
            M669VecSubtract(&direction, &particle->position, &playerPosition);
            distance = fn_1_554(&direction, &direction);
            dot = M669VecDot(&facing, &direction);

            if (bestDot < dot && dot >= 0.75f && distance < 200.0f) {
                bestDot = dot;
                bestParticle = particle;
            }
        }
        if (bestParticle != 0) {
            HuVecF obstacleDirection;
            HuVecF forward;
            HuVecF towardCenter;
            f32 push = 7.0f;
            f32 distance;
            f32 facingAngle;
            f32 angle;

            M669VecSubtract(&obstacleDirection, &bestParticle->position, &playerPosition);
            distance = fn_1_554(&obstacleDirection, &obstacleDirection);
            if ((distance - 40.0f) - 50.0f < push) {
                push = (distance - 40.0f) - 50.0f;
            }
            if (push < 2.0f) {
                push = 2.0f;
            }
            facingAngle = (f32)(M_PI * (object->header.player->actor->rotY / 180.0f));
            fn_1_31C(&forward, (f32)sin(facingAngle), 0.0f, (f32)cos(facingAngle));
            fn_1_554(&forward, &forward);
            M669VecSubtract(&towardCenter, &playerPosition, &center);
            fn_1_554(&towardCenter, &towardCenter);
            if (M669VecDot(&forward, &towardCenter) >= 0.0f) {
                if (distance < 110.0f && M669VecDot(&forward, &obstacleDirection) >= 0.9f) {
                    if (100.0f * frandf() < difficultyChance[difficulty]) {
                        fn_8007EEBC(object->header.player, 3);
                    }
                    push = 1.0f;
                }
                fn_1_1384C(object, atan2f(obstacleDirection.x, obstacleDirection.z),
                           1.0f / difficultyTurn[difficulty], push);
            } else {
                push = 7.0f;
            }
            fn_1_1384C(object, atan2f(obstacleDirection.x, obstacleDirection.z),
                       1.0f / difficultyTurn[difficulty], push);
            return 1;
        }
    }
    {
        M669ActorWork *nearest = NULL;
        f32 nearestDistance = 3.4028235e38f;

        for (i = 0; i < 36; i++) {
            M669ActorWork *particle = M669ParticleGet(i);
            HuVecF direction;
            M669Sphere sphere;
            f32 squaredDistance;

            if (particle->action == 2 || particle->action == 3) {
                continue;
            }
            sphere.center = particle->position;
            sphere.radius = 50.0f;
            fn_1_2504(&particleBounds, &sphere);
            if (fn_1_27FC(&playerBounds, &particleBounds)) {
                M669VecSubtract(&direction, &particle->position, &playerPosition);
                squaredDistance = M669VecLengthSquared(&direction);
                if (squaredDistance < nearestDistance) {
                    nearestDistance = squaredDistance;
                    nearest = particle;
                }
            }
        }
        if (nearest != 0) {
            f32 sidePush[4] = { 2.0f, 3.0f, 4.0f, 5.0f };
            HuVecF obstacleDirection;
            HuVecF forward;
            HuVecF towardCenter;
            f32 push = 7.0f;
            f32 centerDistance;
            f32 distance;
            f32 facingAngle;

            M669VecSubtract(&towardCenter, &playerPosition, &center);
            centerDistance = fn_1_554(&towardCenter, &towardCenter);
            M669VecSubtract(&obstacleDirection, &nearest->position, &playerPosition);
            distance = fn_1_554(&obstacleDirection, &obstacleDirection);
            if ((distance - 40.0f) - 50.0f < push) {
                push = (distance - 40.0f) - 50.0f;
            }
            if (push < sidePush[difficulty]) {
                push = sidePush[difficulty];
            }
            facingAngle = (f32)(M_PI * (object->header.player->actor->rotY / 180.0f));
            fn_1_31C(&forward, (f32)sin(facingAngle), 0.0f, (f32)cos(facingAngle));
            if (M669VecDot(&forward, &towardCenter) >= 0.0f) {
                if (distance < 110.0f && M669VecDot(&forward, &obstacleDirection) >= 0.9f) {
                    if (100.0f * frandf() < difficultyChance[difficulty]) {
                        fn_8007EEBC(object->header.player, 3);
                    }
                    push = 1.0f;
                }
                fn_1_1384C(object, atan2f(obstacleDirection.x, obstacleDirection.z),
                           1.0f / difficultyTurn[difficulty], push);
            } else if ((difficulty == 3 || difficulty == 2) && distance <= 120.0f) {
                HuVecF left = { 0.0f, 1.0f, 0.0f };
                HuVecF right = { 0.0f, -1.0f, 0.0f };
                HuVecF side;

                if (towardCenter.x * obstacleDirection.z - towardCenter.z * obstacleDirection.x >
                    0.0f) {
                    fn_1_4AC(&side, &obstacleDirection, &left);
                } else {
                    fn_1_4AC(&side, &obstacleDirection, &right);
                }
                fn_1_1384C(object, atan2f(side.x, side.z), 0.2f, 6.0f);
            } else {
                if (distance < 110.0f && M669VecDot(&forward, &obstacleDirection) >= 0.9f) {
                    fn_8007EEBC(object->header.player, 3);
                }
                fn_1_1384C(object, atan2f(obstacleDirection.x, obstacleDirection.z),
                           1.0f / difficultyTurn[difficulty], 7.0f);
            }
            return 1;
        }
    }
    if (work->radialCounts[0] < work->radialCounts[1]) {
        s16 greatest = 0;
        s16 selected = -1;

        for (i = 0; i < 16; i++) {
            s32 count = work->counts[i];

            if ((players->nearestPlayerIndices[i] == object->header.positionIndex || count > 3) &&
                (count > greatest || (count == greatest && (rand8() & 1)))) {
                greatest = count;
                selected = i;
            }
        }
        if (selected != -1) {
            HuVecF direction;
            M669VecSubtract(&direction, &lbl_1_rodata_90[selected], &playerPosition);
            fn_1_1384C(object, atan2f(direction.x, direction.z),
                       1.0f / (2.0f * difficultyTurn[difficulty]), 7.0f);
            return 1;
        }
    }
    {
        f32 nearestDistance = 3.4028235e38f;
        M669ActorWork *nearest = NULL;

        for (i = 0; i < 36; i++) {
            M669ActorWork *particle = M669ParticleGet(i);
            HuVecF direction;
            f32 squaredDistance;

            if (particle->action == 2 || particle->action == 3) {
                continue;
            }
            M669VecSubtract(&direction, &particle->position, &playerPosition);
            squaredDistance = M669VecLengthSquared(&direction);
            if (squaredDistance < nearestDistance ||
                (squaredDistance == nearestDistance && (rand8() & 1))) {
                nearestDistance = squaredDistance;
                nearest = particle;
            }
        }
        if (nearest != 0) {
            HuVecF direction;
            M669VecSubtract(&direction, &nearest->position, &playerPosition);
            fn_1_1384C(object, atan2f(direction.x, direction.z),
                       1.0f / (2.0f * difficultyTurn[difficulty]), 7.0f);
            return 1;
        }
    }
    {
        HuVecF direction;
        M669VecSubtract(&direction, &center, &playerPosition);
        fn_1_1384C(object, atan2f(direction.x, direction.z),
                   1.0f / (2.0f * difficultyTurn[difficulty]), 7.0f);
    }
    return 1;
}
