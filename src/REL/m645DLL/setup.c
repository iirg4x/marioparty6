/* Scene setup, player models, collision response and route control for Clean Team. */
#include "math.h"

#include "string.h"

#include "dolphin/gx.h"

#include "REL/m645DLL/character.h"

#include "REL/m645DLL/route.h"

#include "include/humath.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/main.h"

#include "game/gamework.h"

#include "game/mg/seqman.h"

#include "game/audio.h"

#include "game/flag.h"

#include "game/gamemes.h"

#include "REL/m645DLL/shared_types.h"

#include "game/hu3d.h"

#include "game/wipe.h"

#include "game/hsfex.h"

#include "game/data.h"

#include "game/memory.h"

#include "game/mg/timer.h"

#include "game/mg/score.h"

#include "game/pad.h"

#include "game/frand.h"

#include "game/sprite.h"

#include "game/mg/actman.h"

#include "game/board/object.h"

#include "game/board/tutorial.h"

#include "datadir_enum.h"

#include "humath.h"

#include "game/printfunc.h"

extern s16 lbl_802C0278;

typedef void (*M645Callback)(OMOBJ *);

extern void *fn_1_164(s32 priority, u32 bytes, M645Callback callback);

extern void fn_1_2B8(HuVecF *out, f32 x, f32 y, f32 z);

extern f32 fn_1_4F0(HuVecF *dst, const HuVecF *src);

extern void fn_1_32E8(OMOBJ *object);

extern void fn_1_3F38(OMOBJ *object);

extern void fn_1_8738(OMOBJ *object);

extern void fn_1_557C(OMOBJ *object);

extern void fn_1_405C(OMOBJ *object);

extern void fn_1_204(OMOBJ *object, M645Callback callback);

extern void fn_1_2F3C(OMOBJ *object);

M645CharacterSetupWork *lbl_1_bss_160[4];
M645SceneWork **lbl_1_bss_15C;
s32 lbl_1_bss_158;
s32 lbl_1_bss_154;
u32 lbl_1_bss_150;
s32 lbl_1_bss_14C;
s32 lbl_1_bss_138[5];
M645RouteEntry lbl_1_bss_C[25];
M645RouteEntry *lbl_1_bss_8;

s32 fn_1_FC04(M645CharacterSetupWork *);
s32 fn_1_F1BC(M645CharacterSetupWork *);
s32 (*const lbl_1_rodata_60[4])(M645CharacterSetupWork *) = {fn_1_FC04, fn_1_F1BC, fn_1_FA1C, fn_1_FAF8};

const M645Segment lbl_1_rodata_70[2] = { { { -427.5f, 0.0f, 0.0f }, { 0.0f, 1400.0f, 0.0f } }, { { 427.5f, 0.0f, 0.0f }, { 0.0f, 1400.0f, 0.0f } } };

const f32 lbl_1_rodata_A0[5] = {1200.0f, 900.0f, 600.0f, 300.0f, 0.0f};

const s32 lbl_1_rodata_B4[2] = {1, 2};

void fn_1_29CC(s32 *playerOrder)
{
    s32 playerPermutation[4] = { -1, -1, -1, -1 };
    s32 playerIndex;

    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        playerPermutation[lbl_80265728[playerIndex].grpNo] = playerIndex;
    }
    if (playerPermutation[0] == -1 || playerPermutation[1] == -1 ||
        playerPermutation[2] == -1 || playerPermutation[3] == -1) {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerOrder[playerIndex] = playerIndex;
        }
    } else {
        for (playerIndex = 0; playerIndex < 4; playerIndex++) {
            playerOrder[playerIndex] = lbl_80265728[playerIndex].grpNo;
        }
    }
}

/* Sets up the board's lights, randomized player order, and player processes. */
void fn_1_2ABC(OMOBJ *object)
{
    s16 * work = object->data;
    s32 playerOrder[4];
    HuVecF direction;
    struct M645PlayerPermutation { s32 values[4]; };
    struct M645InitialWords { u32 first; u32 second; } initialWords = { 0, 2 };
    s32 playerIndex;
    s16 nightMode;
    s16 light;
    s16 playerNo;
    work[2] = - 1;
    fn_800285BC();
    nightMode = lbl_802C0278;
    if (nightMode == 1)
    {
        fn_1_2B8(& direction, 0.0f, -1000.0f, -3000.0f);
        fn_1_4F0(& direction, & direction);
        light = Hu3DGLightCreate(0.0f, 1000.0f, 3000.0f, direction.x, direction.y, direction.z, 0x40, 0x40, 0xFF);
        Hu3DGLightStaticSet(light, 1);
        Hu3DGLightInfinitytSet(light);
        fn_1_2B8(& direction, 0.0f, -3000.0f, 1000.0f);
        fn_1_4F0(& direction, & direction);
        light = Hu3DGLightCreate(0.0f, 3000.0f, -1000.0f, direction.x, direction.y, direction.z, 0xFF, 0xFF, 0xFF);
        Hu3DGLightStaticSet(light, 1);
        Hu3DGLightInfinitytSet(light);
    }
    else
    {
        playerNo = Hu3DGLightCreate(0.0f, 5000.0f, 1000.0f, 0.0f, 0.0f, -1.0f, 0xFF, 0xFF, 0xFF);
        Hu3DGLightStaticSet(playerNo, 1);
        Hu3DGLightInfinitytSet(playerNo);
    }
    lbl_1_bss_158 = - 1;
    lbl_1_bss_154 = 0;
    lbl_1_bss_14C = rand8() % 4;
    lbl_1_bss_150 = rand8() % 4;
    fn_1_29CC(playerOrder);
    fn_1_164(0xA, 0x58, fn_1_32E8);
    fn_1_164(0xA, 4, fn_1_3F38);
    for (playerIndex = 0; playerIndex < 4; playerIndex++)
    {
        lbl_1_bss_160[playerIndex] = fn_1_164(0x14, 0x68, fn_1_8738);
        lbl_1_bss_160[playerIndex]->playerNo = playerIndex;
        lbl_1_bss_160[playerIndex]->variation = (s16) playerOrder[playerIndex];
        lbl_1_bss_160[playerIndex]->routePlayerNo = playerOrder[playerIndex] < 2 ? 0 : 1;
    }
    fn_1_164(0x15, 8, fn_1_557C);
    lbl_1_bss_15C = fn_1_164(0x28, 8, fn_1_405C);
    fn_8005DDEC(5);
    fn_1_204(object, fn_1_2F3C);
}

extern void fn_1_204(OMOBJ *obj, OMOBJ_FUNC callback);

extern void fn_1_30D4(OMOBJ *obj);

typedef struct M645SequenceWork { s32 frame; } M645SequenceWork;

typedef struct M645StageAudioWork { s32 stageState; s16 streamHandle; } M645StageAudioWork;

void fn_1_2F9C(OMOBJ *obj);

void fn_1_2FFC(OMOBJ *obj);

void fn_1_3084(OMOBJ *obj);

void fn_1_2F3C(OMOBJ *obj)
{
    s32 *timer = obj->data;

    if (fn_80071A8C() == 2) {
        fn_80071B08();
        *timer = 0;
        fn_1_204(obj, fn_1_2F9C);
        return;
    }
}

void fn_1_2F9C(OMOBJ *obj)
{
    M645SequenceWork *work = obj->data;

    if (work->frame++ >= 620) {
        fn_80071A9C();
        fn_1_204(obj, fn_1_2FFC);
        return;
    }
}

void fn_1_2FFC(OMOBJ *object)
{
    M645StageAudioWork *work = object->data;

    if (work->streamHandle == -1 &&
        (GameMesStatGet(fn_80071DF4()) & 0x10) != 0) {
        work->streamHandle = HuAudSStreamPlay(MSM_STREAM_BLISSFUL);
    }
    if (fn_80071A8C() == 5) {
        fn_1_204(object, fn_1_3084);
        return;
    }
}

void fn_1_3084(OMOBJ *obj)
{
    s16 *work = obj->data;
    if (fn_80071A8C() == 6) {
        fn_1_204(obj, fn_1_30D4);
        return;
    }
}

extern void fn_1_3224(OMOBJ *obj);

extern void fn_1_3278(OMOBJ *obj);

void fn_1_30D4(OMOBJ *object)
{
    s16 *work = object->data;
    s32 winner[4] = { -1, -1, -1, -1 };
    s32 winnerCount;
    s32 i;
    winnerCount = 0;
    for (i = 0; i < 4; i++) {
        if (lbl_1_bss_158 == lbl_1_bss_160[i]->routePlayerNo) {
            winner[winnerCount] = lbl_1_bss_160[i]->characterNo;
            if (fn_8006A124(65551U) == 0) {
                lbl_80265750[i].mgCoinBonus = 10;
            }
            winnerCount++;
        }
    }
    fn_80071C18(winner[0], winner[1],
                   winner[2], winner[3]);
    if (work[2] != -1) {
        HuAudSStreamFadeOut(work[2], 100);
    }
    fn_1_204(object, fn_1_3224);
}

void fn_1_3224(OMOBJ *object)
{
    void *work = object->data;

    if (fn_80071A8C() == 7) {
        fn_80071B08();
        fn_1_204(object, fn_1_3278);
        return;
    }
}

void fn_1_3278(OMOBJ *object)
{
    void *work = object->data;

    if (lbl_1_bss_154 == 1 || lbl_1_bss_158 == -1) {
        fn_80071A9C();
        fn_80071B30();
        fn_1_204(object, NULL);
        return;
    }
}

extern HuVecF lbl_1_data_4C8[2];

extern HuVecF lbl_1_data_4E0[2];

extern void fn_1_480(HuVecF *out, HuVecF *a, HuVecF *b);

extern void fn_1_35D8(OMOBJ *object);

void fn_1_3634(OMOBJ *object);

void fn_1_3698(OMOBJ *object);

typedef struct CameraWork {
    s32 frame;
    f32 width;
    f32 height;
    HuVecF position[2];
    HuVecF target[2];
    HuVecF up[2];
} CameraWork;

/* Builds the split-screen cameras and the two positions used by the board view. */

const HuVecF lbl_1_rodata_100 = { 0.0f, 1.0f, 0.0f };

const HuVecF lbl_1_rodata_10C = { 0.0f, -1.0f, 0.0f };

void fn_1_32E8(OMOBJ *object)
{
    CameraWork *work = object->data;
    HuVecF cameraPosition;
    HuVecF cameraTarget;
    f32 aspectRatio;
    s32 viewIndex;

    work->width = 320.0f;
    work->height = 480.0f;
    aspectRatio = work->width / work->height;
    Hu3DCameraCreate(3);
    Hu3DCameraPerspectiveSet(3, 35.0f, 20.0f,
        15000.0f, aspectRatio);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        318.0f, work->height, 0.0f,
        1.0f);
    Hu3DCameraViewportSet(2, 322.0f, 0.0f,
        work->width, work->height, 0.0f,
        1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x13E,
        (u32)work->height);
    Hu3DCameraScissorSet(2, 0x142, 0, 0x13E,
        (u32)work->height);

    cameraPosition = lbl_1_rodata_100;
    cameraTarget = lbl_1_rodata_10C;
    for (viewIndex = 0; viewIndex < 2; viewIndex++) {
        work->position[viewIndex] = lbl_1_data_4C8[viewIndex];
        fn_1_2B8(&work->target[viewIndex],
            0.0f,
            (f32)sin(lbl_1_data_4E0[viewIndex].x),
            (f32)-cos(lbl_1_data_4E0[viewIndex].z));
        fn_1_480(&work->target[viewIndex],
            &work->target[viewIndex], &work->position[viewIndex]);
        work->up[viewIndex] = cameraPosition;
    }
    Hu3DCameraPosSetV(1, &work->position[0], &work->up[0], &work->target[0]);
    Hu3DCameraPosSetV(2, &work->position[0], &work->up[0], &work->target[0]);
    fn_1_204(object, fn_1_35D8);
}

extern void fn_1_3B28(OMOBJ *object);

/* Moves both board views through their camera transition and finishes at frame 60. */
void fn_1_35D8(OMOBJ *obj)
{
    CameraWork *work = obj->data;

    if (fn_80071A8C() == 2) {
        work->frame = 0;
        fn_1_204(obj, fn_1_3634);
        return;
    }
}

void fn_1_3634(OMOBJ *obj)
{
    CameraWork *work = obj->data;

    if (work->frame++ >= 521) {
        work->frame = 0;
        fn_1_204(obj, fn_1_3698);
        return;
    }
}

void fn_1_3698(OMOBJ *object)
{
    CameraWork *work = object->data;
    HuVecF cameraEye;
    HuVecF cameraTarget;
    HuVecF cameraUp;
    HuVecF startPosition;
    HuVecF startTarget;
    f32 transition;
    f32 perspective;
    f32 secondaryPerspective;
    s32 cameraIndex;
    transition = (f32)work->frame / 60.0f;
    work->width = 640.0f - 320.0f * transition;
    perspective = 320.0f / work->height;
    secondaryPerspective = perspective;
    Hu3DCameraPerspectiveSet(1, 35.0f, 20.0f,
        15000.0f, perspective);
    Hu3DCameraViewportSet(1, 0.0f, 0.0f,
        320.0f, work->height, 0.0f,
        1.0f);
    Hu3DCameraScissorSet(1, 0, 0, 0x13E, (u32)work->height);
    if (secondaryPerspective > 0.0f) {
        Hu3DCameraPerspectiveSet(2, 35.0f, 20.0f,
            15000.0f, secondaryPerspective);
    }
    Hu3DCameraViewportSet(2, 320.0f, 0.0f,
        320.0f, work->height, 0.0f,
        1.0f);
    Hu3DCameraScissorSet(2, 0x142, 0, 0x140, (u32)work->height);

    for (cameraIndex = 0; cameraIndex < 2; cameraIndex++) {
        startPosition = work->position[1];
        startTarget = work->target[1];
        if (cameraIndex == 0) {
            startPosition.x -= 32.0f;
            startTarget.x -= 32.0f;
        } else {
            startPosition.x += 32.0f;
            startTarget.x += 32.0f;
        }

        cameraEye.x = work->position[0].x * (1.0f - transition) + startPosition.x * transition;
        cameraEye.y = work->position[0].y * (1.0f - transition) + startPosition.y * transition;
        cameraEye.z = work->position[0].z * (1.0f - transition) + startPosition.z * transition;
        cameraTarget.x = work->target[0].x * (1.0f - transition) + startTarget.x * transition;
        cameraTarget.y = work->target[0].y * (1.0f - transition) + startTarget.y * transition;
        cameraTarget.z = work->target[0].z * (1.0f - transition) + startTarget.z * transition;
        cameraUp.x = work->up[0].x * (1.0f - transition) + work->up[1].x * transition;
        cameraUp.y = work->up[0].y * (1.0f - transition) + work->up[1].y * transition;
        cameraUp.z = work->up[0].z * (1.0f - transition) + work->up[1].z * transition;
        Hu3DCameraPosSetV(lbl_1_rodata_B4[cameraIndex], &cameraEye,
            &cameraUp, &cameraTarget);
    }

    if (work->frame++ >= 0x3C) {
        work->frame = 0;
        fn_1_204(object, fn_1_3B28);
        return;
    }
}

extern void fn_1_3BA8(OMOBJ *obj);

extern void fn_1_3EC4(OMOBJ *obj);

extern void fn_1_7694(M645CharacterSetupWork *work);

void fn_1_3B28(OMOBJ *obj)
{
    s32 *work = obj->data;
    if (lbl_1_bss_158 != -1) {
        if (fn_80071A8C() == 7) {
            WipeCreate(2, 0, 60);
            *work = 0;
            fn_1_204(obj, fn_1_3BA8);
            return;
        }
    }
}

void fn_1_3BA8(OMOBJ *obj)
{
    s32 *timer = obj->data;
    f32 distance, angle, zoom, tangent;
    s32 groupIndex;

    if ((*timer)++ >= 60) {
        Hu3DCameraPerspectiveSet(lbl_1_rodata_B4[lbl_1_bss_158], 35.0f,
            20.0f, 15000.0f, 1.2f);
        Hu3DCameraViewportSet(lbl_1_rodata_B4[lbl_1_bss_158], 32.0f,
            0.0f, 576.0f, 480.0f,
            0.0f, 1.0f);
        Hu3DCameraScissorSet(lbl_1_rodata_B4[lbl_1_bss_158], 0, 0, 640, 480);
        Hu3DCameraViewportSet(lbl_1_rodata_B4[(lbl_1_bss_158 + 1) % 2],
            640.0f, 480.0f, 576.0f,
            480.0f, 0.0f, 1.0f);
        distance = 500.0f;
        angle = 0.61086524f;
        tangent = (f32)tan((f64)(angle / 2.0f));
        zoom = distance / tangent;
        Hu3DCameraPosSet(lbl_1_rodata_B4[lbl_1_bss_158], 0.0f, 300.0f, zoom,
            0.0f, 1.0f, 0.0f,
            0.0f, 300.0f, 0.0f);
        WipeCreate(1, 5, 60);
        *timer = 0;
        for (groupIndex = 0; groupIndex < 4; groupIndex++) {
            fn_1_7694(lbl_1_bss_160[groupIndex]);
        }
        fn_1_204(obj, fn_1_3EC4);
        return;
    }
}

typedef struct M645SequenceTimerWork {
    s32 timer;
} M645SequenceTimerWork;

extern s32 lbl_1_data_B0[4];

void fn_1_3EC4(OMOBJ *obj)
{
    M645SequenceTimerWork *work = obj->data;

    if (work->timer++ >= 60) {
        lbl_1_bss_154++;
        fn_1_204(obj, NULL);
        return;
    }
}

void fn_1_3F38(OMOBJ *obj)
{
    s16 **modelIds = obj->data;
    s32 modelIndex;
    s16 nightFlag;
    s32 modelSet;

    *modelIds = HuMemDirectMallocNum(HEAP_HEAP, 4, 0x10000000);
    for (modelIndex = 0; modelIndex < 2; modelIndex++) {
        nightFlag = lbl_802C0278;
        if (nightFlag == 0) {
            modelSet = 0;
        } else {
            modelSet = 1;
        }
        (*modelIds)[modelIndex] = fn_80025A04(fn_80008DD0(
            ((s32 (*)[2])lbl_1_data_B0)[modelSet][modelIndex],
            0x10000000, 2));
    }
    fn_80026B8C((*modelIds)[0], 4);
    fn_80026B8C((*modelIds)[1], 4);
    fn_1_204(obj, NULL);
}

extern void *fn_1_164(s32 priority, u32 bytes, OMOBJ_FUNC update);

extern void fn_1_43C0(OMOBJ *object);

extern void fn_1_40F4(OMOBJ *object);

extern void fn_1_41F8(OMOBJ *object);

M645RouteObject *fn_1_401C(s16 groupIndex, s16 itemIndex)
{
    M645RouteObject *value;

    value = lbl_1_bss_15C[groupIndex]->routeObjects[itemIndex];
    return value;
}

/* Create both stage objects and connect their shared scene-work table. */
void fn_1_405C(OMOBJ *object) {
    M645SceneWork **work;
    s32 stageIndex;

    work = object->data;
    stageIndex = 0;
    while (stageIndex < 2) {
        work[stageIndex] = fn_1_164(40, 220, fn_1_43C0);
        work[stageIndex]->siblings = work;
        work[stageIndex]->stageIndex = (s16)stageIndex;
        stageIndex += 1;
    }
    fn_1_204(object, fn_1_40F4);
}

void fn_1_40F4(OMOBJ *object)
{
    M645SceneWork **children = object->data;
    s32 selectedIndex;

    if (fn_80071A8C() == 6) {
        fn_1_204(object, fn_1_41F8);
        return;
    }
    if (children[0]->selectionFlag != 0 || children[1]->selectionFlag != 0) {
        if (children[0]->selectionFlag != 0 && children[1]->selectionFlag != 0) {
            lbl_1_bss_158 = rand8() % 2;
        } else {
            if (children[0]->selectionFlag != 0) {
                selectedIndex = 0;
            } else {
                selectedIndex = 1;
            }
            lbl_1_bss_158 = selectedIndex;
        }
        fn_80071AC4(6);
        fn_1_204(object, fn_1_41F8);
        return;
    }
}

extern const s8 *lbl_1_data_518[4][5];

extern s32 lbl_1_data_480[2];

extern s32 lbl_1_data_488[2][2];

extern s32 lbl_1_data_498[12];

extern s32 lbl_1_data_C0[2];

extern HuVecF lbl_1_data_C8[2];

extern HuVecF lbl_1_data_24C[25];

extern HuVecF lbl_1_data_3F0[12];

extern void fn_1_49CC(OMOBJ *object);

extern void fn_1_4A64(OMOBJ *object);

void fn_1_41F8(OMOBJ *object)
{
    void *work;
    /* The target reads its work pointer even though this callback is idle. */
    work = object->data;
}

/* Position the two stage models, reset their motions and play the stage sound. */
void fn_1_420C(M645SceneWork *object, Point3d *position) {
    fn_80026790(object->stageModels[0], position);
    fn_80026A44(object->stageModels[0], 0x40000003U);
    fn_8002D414(object->stageModels[0], 0.0f);
    fn_80026790(object->stageModels[1], position);
    fn_80026A44(object->stageModels[1], 0x40000003U);
    fn_8002D414(object->stageModels[1], 0.0f);
    HuAudFXPlay(object->stageIndex == 0 ? 0x7BB : 0x7BC);
}

void fn_1_42C8(M645SceneWork *object, s32 position)
{
    s32 remainder;
    s32 quotient;
    s32 modelIndex;
    s32 soundId;
    s16 nightMode;

    remainder = (position % 5) - 1;
    quotient = position / 5;
    modelIndex = remainder + (quotient * 3);
    nightMode = lbl_802C0278;
    if (nightMode == 1) {
        fn_80026A44(
            object->routeMarkers[modelIndex],
            0x40000002U);
        if (object->stageIndex == 0) {
            soundId = 0x7BD;
        } else {
            soundId = 0x7BE;
        }
        HuAudFXPlay(soundId);
    }
    if (object->routeModels[modelIndex] != 0) {
        fn_8002C95C(object->routeModels[modelIndex],
            object->routeMotions[modelIndex][0]);
    }
}

void fn_1_43C0(OMOBJ *object)
{
    M645SceneWork *scene = object->data;
    s32 activeRoutes[25];
    s32 routeCell;
    s32 column;
    s32 activeRouteCount;
    s32 row;
    s32 route;
    const s8 **routeFlags;
    s32 routeClass;
    s32 motionIndex;
    HuVecF position;

    activeRouteCount = 0;
    routeCell = 0;
    scene->activeRouteCount = 0;
    scene->selectionFlag = 0;
    routeFlags = &lbl_1_data_518[lbl_1_bss_14C][0];
    for (row = 0; row < 4; row++) {
        for (column = 0; column < 5; column++) {
            if (routeFlags[row][column] == 0x40) {
                activeRoutes[routeCell] = 1;
                activeRouteCount++;
            } else {
                activeRoutes[routeCell] = 0;
            }
            routeCell++;
        }
    }

    for (route = 0; route < 12; route++) {
        column = (route % 3) + 1;
        row = route / 3;
        if (activeRoutes[row * 5 + column] != 0) {
            s16 modelNightMode = lbl_802C0278;

            scene->routeModels[route] = fn_80025A04(fn_80008DD0(
                lbl_1_data_480[modelNightMode], 0x10000000, 2));

            fn_80026B8C(scene->routeModels[route], 2);
            fn_80026790(scene->routeModels[route],
                &lbl_1_data_3F0[route]);
            fn_80026B64(scene->routeModels[route],
                (u16)lbl_1_rodata_B4[scene->stageIndex]);
            for (motionIndex = 0; motionIndex < 2; motionIndex++) {
                s16 motionNightMode = lbl_802C0278;

                scene->routeMotions[route][motionIndex] = fn_80030458(
                    scene->routeModels[route], fn_80008DD0(
                        lbl_1_data_488[motionNightMode][motionIndex],
                        0x10000000, 2));
            }
            fn_8002C95C(scene->routeModels[route],
                scene->routeMotions[route][1]);
            fn_800269E8(scene->routeModels[route], 0x40000001U);
        }
    }

    scene->stageModels[0] = fn_80025A04(
        fn_80008DD0(DATANUM(0x680000, 21), 0x10000000, 2));
    fn_80026B8C(scene->stageModels[0], 5);
    fn_80026B64(scene->stageModels[0],
        (u16)lbl_1_rodata_B4[scene->stageIndex]);
    fn_800269E8(scene->stageModels[0], 0x40000003U);

    scene->stageModels[1] = fn_80025A04(
        fn_80008DD0(DATANUM(0x680000, 22), 0x10000000, 2));
    fn_80026B8C(scene->stageModels[1], 5);
    fn_80026B64(scene->stageModels[1],
        (u16)lbl_1_rodata_B4[scene->stageIndex]);
    scene->portraitMotion = fn_80030458(scene->stageModels[1],
        fn_80008DD0(DATANUM(0x680000, 23), 0x10000000, 2));
    fn_8002C95C(scene->stageModels[1], scene->portraitMotion);
    fn_800269E8(scene->stageModels[1], 0x40000003U);

    for (route = 0; route < 25; route++) {
        scene->routeObjects[route] = fn_1_164(0x28, 0x1C, fn_1_4A64);
        scene->routeObjects[route]->scene = scene;
        scene->routeObjects[route]->routeIndex = (s16)route;
        scene->routeObjects[route]->stageIndex = scene->stageIndex;
        if (route >= 0x14) {
            routeClass = 7;
        } else {
            routeClass = activeRoutes[route] != 0 ? 0 : 6;
        }
        scene->routeObjects[route]->phase = routeClass;
    }

    for (route = 0; route < 2; route++) {
        scene->characterModels[route] = fn_80025A04(
            fn_80008DD0(
                lbl_1_data_C0[scene->stageIndex], 0x10000000, 2));
        fn_80026B8C(scene->characterModels[route], 4);
        fn_80026B64(scene->characterModels[route],
            (u16)lbl_1_rodata_B4[scene->stageIndex]);
        fn_80026790(scene->characterModels[route],
            &lbl_1_data_C8[route]);
    }

    {
        s16 markerNightMode = lbl_802C0278;

        if (markerNightMode == 1) {
        for (route = 0; route < 12; route++) {
            column = (route % 3) + 1;
            row = route / 3;
            position = lbl_1_data_24C[row * 5 + column];

            position.z = -100.0f;
            scene->routeMarkers[route] = fn_80025A04(fn_80008DD0(
                lbl_1_data_498[route], 0x10000000, 2));
            fn_80026B8C(scene->routeMarkers[route], 4);
            fn_80026B64(scene->routeMarkers[route],
                (u16)lbl_1_rodata_B4[scene->stageIndex]);
            fn_80026790(scene->routeMarkers[route], &position);
            fn_800269E8(scene->routeMarkers[route], 0x40000002U);
        }
        }
    }

    fn_1_204(object, fn_1_49CC);
}

extern void fn_1_4A28(OMOBJ *obj);

void fn_1_49CC(OMOBJ *obj)
{
    M645SceneWork *work = obj->data;
    if (work->activeRouteCount == 0) {
        work->selectionFlag = 1;
        fn_1_204(obj, fn_1_4A28);
        return;
    }
}

extern s32 lbl_1_data_E0[4];

extern void fn_1_4C80(OMOBJ *obj);

extern void fn_1_5074(OMOBJ *obj);

void fn_1_4A28(OMOBJ *object)
{
    void *work;

    work = object->data;
}

void fn_1_4A3C(M645RouteObject *work) {
    work->timer = 0;
    work->phase = 2;
}

void fn_1_4A50(M645RouteObject *work) {
    work->timer = 0;
    work->phase = 5;
}

void fn_1_4A64(OMOBJ *obj)
{
    HuVecF position;
    s32 motionIndex;
    M645RouteObject *work = obj->data;

    if (work->phase != 7) {
        work->displayModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x680000, 4), 0x10000000, 2));
        fn_80026B8C(work->displayModel, 5);
        fn_80026B64(work->displayModel,
            (u16)lbl_1_rodata_B4[work->stageIndex]);
        position = lbl_1_data_24C[work->routeIndex];
        fn_80026790(work->displayModel, &position);
        fn_800269E8(work->displayModel, ((1 << 30) | (1 << 1)));

        work->motionModel = fn_80025A04(
            fn_80008DD0(DATANUM(0x680000, 12), 0x10000000, 2));
        fn_80026B8C(work->motionModel, 5);
        fn_80026B64(work->motionModel,
            (u16)lbl_1_rodata_B4[work->stageIndex]);
        position.z += 10.0f;
        fn_80026790(work->motionModel, &position);
        fn_800269E8(work->motionModel, (1 << 0));
        for (motionIndex = 0; motionIndex < 4; motionIndex++) {
            work->motions[motionIndex] = fn_80030458(
                work->motionModel,
                fn_80008DD0(lbl_1_data_E0[motionIndex],
                    0x10000000, 2));
        }
        fn_8002C95C(work->motionModel, work->motions[0]);
    }
    if (work->phase == 7) {
        fn_1_204(obj, fn_1_5074);
        return;
    }
    if (work->phase == 0) {
        work->scene->activeRouteCount++;
        fn_1_204(obj, fn_1_4C80);
        return;
    }
    fn_800269E8(work->displayModel, (1 << 0));
    fn_1_204(obj, fn_1_5074);
}

extern void fn_1_4D90(OMOBJ *obj);

extern void fn_1_4E48(OMOBJ *obj);

void fn_1_4D10(OMOBJ *obj);

void fn_1_4C80(OMOBJ *obj)
{
    M645RouteObject *work = obj->data;

    if (fn_80071A8C() == 6) {
        fn_1_204(obj, 0);
        return;
    }
    if (work->phase == 2) {
        fn_80026A44(work->motionModel, (1 << 0));
        fn_8002C95C(work->motionModel, work->motions[1]);
        work->timer = 0;
        fn_1_204(obj, fn_1_4D10);
        return;
    }
}

void fn_1_4D10(OMOBJ *obj)
{
    M645RouteObject *work;

    work = obj->data;
    if (fn_80071A8C() == 6) {
        fn_1_204(obj, 0);
        return;
    }
    if (work->timer++ >= 0x50) {
        work->phase = 3;
        fn_1_204(obj, fn_1_4D90);
        return;
    }
}

void fn_1_4D90(OMOBJ *obj)
{
    M645RouteObject *work = obj->data;

    if (fn_80071A8C() == 6) {
        fn_1_204(obj, NULL);
        return;
    }
    if (fn_8002D3C4(work->motionModel) == work->motions[1] &&
        fn_8002D75C(work->motionModel)) {
        fn_8002C95C(work->motionModel, work->motions[2]);
    }
    if (work->phase == 5) {
        fn_8002C95C(work->motionModel, work->motions[3]);
        work->timer = 0;
        fn_1_204(obj, fn_1_4E48);
        return;
    }
}

void fn_1_4E48(OMOBJ *obj)
{
    M645RouteObject *work = obj->data;
    M645SceneWork *parent;
    M645SceneWork *motionParent;
    const HuVecF *position;
    s32 columnIndex;
    s32 quotient;
    s32 remainder;
    s32 row;
    s32 firstSound;
    s32 nightSound;
    s16 nightFlag;

    if (fn_80071A8C() == 6) {
        fn_1_204(obj, NULL);
    } else {
        work->timer += 1;
        if (work->timer == 0x0A) {
            fn_80026A44(work->displayModel, ((1 << 30) | (1 << 1)));
        }
        if (work->timer < 0x50) {
            return;
        } else {
            work->phase = 6;
            fn_800269E8(work->displayModel, (1 << 0));
            work->scene->activeRouteCount--;
            fn_800269E8(work->motionModel, (1 << 0));
            position = &lbl_1_data_24C[work->routeIndex];
            parent = work->scene;
            fn_80026790(parent->stageModels[0], (Point3d *)position);
            fn_80026A44(parent->stageModels[0], 0x40000003U);
            fn_8002D414(parent->stageModels[0], 0.0f);
            fn_80026790(parent->stageModels[1], (Point3d *)position);
            fn_80026A44(parent->stageModels[1], 0x40000003U);
            fn_8002D414(parent->stageModels[1], 0.0f);
            if (parent->stageIndex == 0) { firstSound = 0x7BB; }
            else { firstSound = 0x7BC; }
            HuAudFXPlay(firstSound);

            columnIndex = work->routeIndex;
            motionParent = work->scene;
            remainder = (columnIndex % 5) - 1;
            quotient = columnIndex / 5;
            row = remainder + quotient * 3;
            nightFlag = lbl_802C0278;
            if (nightFlag == 1) {
                fn_80026A44(motionParent->routeMarkers[row], 0x40000002U);
                if (motionParent->stageIndex == 0) { nightSound = 0x7BD; }
                else { nightSound = 0x7BE; }
                HuAudFXPlay(nightSound);
            }
            if (motionParent->routeModels[row] != 0) {
                fn_8002C95C(motionParent->routeModels[row], motionParent->routeMotions[row][0]);
            }
            fn_1_204(obj, fn_1_5074);
            return;
        }
    }
}

extern f32 fn_1_20C(f32 cosine);

typedef struct M645RouteGeometry {
    M645Triangle triangle;
    M645Bounds bounds;
} M645RouteGeometry;

typedef struct M645CollisionList {
    s16 partCount;
    u8 beforeParts[2];
    M645RouteGeometry *parts;
} M645CollisionList;

extern void fn_1_5088(f32 scalar);

extern void fn_1_53B4(M645CollisionList *work, f32 scalar);

extern void fn_1_708(M645Bounds *bounds, M645Capsule *capsule);

extern s32 fn_1_BC8(M645Bounds *boundsA, M645Bounds *boundsB);

extern f32 fn_1_1D70(M645Segment *segmentA, M645Segment *segmentB, f32 *parameterAOut, f32 *parameterBOut);

extern void fn_1_4B8(HuVecF *dst, const HuVecF *a, const HuVecF *b);

void fn_1_5074(OMOBJ *object)
{
    void *work;

    work = object->data;
}

static inline M645CharacterSetupWork *fn_1_29B4(s32 playerIndex)
{
    return lbl_1_bss_160[playerIndex];
}

void fn_1_5088(f32 radius)
{
    HuVecF movement[4];
    M645Capsule playerCapsule;
    M645Bounds playerBounds;
    M645Capsule otherCapsule;
    M645Bounds otherBounds;
    HuVecF playerPosition, separation, direction, otherPosition;
    s32 playerIndex;
    s32 otherIndex;
    M645CharacterSetupWork *player, *other;
    f32 distance, overlap;
    char *hookName;

    memset(movement, 0, sizeof(movement));
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        player = fn_1_29B4(playerIndex);

        playerPosition = player->position;
        playerPosition.y += radius;
        player->collisionFlag = 0;
        playerCapsule.segment.start = playerPosition;
        playerCapsule.segment.delta.x = 0.0f;
        playerCapsule.segment.delta.y = player->modelHeight -
            (2.0f * radius);
        playerCapsule.segment.delta.z = 0.0f;
        playerCapsule.radius = radius;
        fn_1_708(&playerBounds, &playerCapsule);

        for (otherIndex = 0; otherIndex < 4; otherIndex++) {
            other = fn_1_29B4(otherIndex);

            if (playerIndex != otherIndex && player->routePlayerNo == other->routePlayerNo) {
            otherPosition = other->position;
            otherPosition.y += radius;
            fn_1_4B8(&separation, &playerPosition, &otherPosition);
            fn_1_4F0(&direction, &separation);
            otherCapsule.segment.start = otherPosition;
            otherCapsule.segment.delta.x = 0.0f;
            otherCapsule.segment.delta.y = other->modelHeight -
                (2.0f * radius);
            otherCapsule.segment.delta.z = 0.0f;
            otherCapsule.radius = radius;
            fn_1_708(&otherBounds, &otherCapsule);
            if (fn_1_BC8(&playerBounds, &otherBounds) != 0) {
                distance = fn_1_1D70(&playerCapsule.segment,
                    &otherCapsule.segment, NULL, NULL);
                if (distance <= (radius + radius)) {
                    if (player->setupMode != other->setupMode) {
                        overlap = (radius + radius) - distance;
                    } else {
                        overlap = 0.5f * ((radius + radius) - distance);
                    }
                    if (player->setupMode == 0) {
                        movement[playerIndex].x += direction.x * overlap;
                    } else if (player->setupMode == 1) {
                        movement[playerIndex].y += direction.y * overlap;
                    }
                    player->collisionFlag = 1;
                }
            }
            }
        }
    }
    for (playerIndex = 0; playerIndex < 4; playerIndex++) {
        M645CharacterSetupWork *positionPlayer =
            fn_1_29B4(playerIndex);

        fn_1_480(&positionPlayer->position, &positionPlayer->position,
            &movement[playerIndex]);
    }
}

extern void fn_1_B34(M645Bounds *bounds, M645Sphere *sphere);

extern f32 fn_1_F6C(M645Sphere *sphere, M645Triangle *triangle, s32 barycentricS, s32 barycentricT);

extern f32 fn_1_E40(M645Sphere *sphere, M645Triangle *triangle, s32 barycentricS, s32 barycentricT);

void fn_1_53B4(M645CollisionList *collisionData, f32 radius)
{
    HuVecF totalMovement[4];
    s32 groupIndex;
    M645RouteGeometry *part;
    s32 partIndex;
    M645Sphere sphere;
    M645Bounds bounds;
    f32 contact;

    memset(totalMovement, 0, sizeof(totalMovement));
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        M645CharacterSetupWork *collisionGroup =
            lbl_1_bss_160[groupIndex];
        M645CharacterSetupWork *player = collisionGroup;
        sphere.center = player->position;
        sphere.center.y += radius;
        sphere.radius = radius;
        fn_1_B34(&bounds, &sphere);
        for (partIndex = 0; partIndex < collisionData->partCount; partIndex++) {
            part = &collisionData->parts[partIndex];
            if (fn_1_BC8(&part->bounds, &bounds) != 0) {
                contact = fn_1_F6C(&sphere, &part->triangle, 0, 0);
                if (0.0f == contact) {
                    contact = fn_1_E40(&sphere, &part->triangle, 0, 0);
                    if (sphere.center.x > part->triangle.start.x) {
                        totalMovement[groupIndex].x += radius - contact;
                    } else {
                        totalMovement[groupIndex].x -= radius - contact;
                    }
                }
            }
        }
    }
    for (groupIndex = 0; groupIndex < 4; groupIndex++) {
        M645CharacterSetupWork *positionGroup =
            lbl_1_bss_160[groupIndex];
        M645CharacterSetupWork *movedPlayer = positionGroup;
        fn_1_480(&movedPlayer->position, &movedPlayer->position,
            &totalMovement[groupIndex]);
    }
}

typedef struct M645StageCharacterData {
    const s8 *characters[5];
} M645StageCharacterData;

extern HuVecF lbl_1_data_378[5][2];

extern M645StageCharacterData lbl_1_data_574[4];

extern void fn_1_8C0(M645Bounds *bounds, M645Triangle *triangle);

extern void fn_1_6284(OMOBJ *obj);

static inline s32 fn_1_2984_for_557C(s32 index)
{
    return lbl_1_bss_138[index];
}

static inline void fn_1_299C_for_557C(s32 index, s32 value)
{
    lbl_1_bss_138[index] = value;
}

static inline M645RouteEntry *fn_1_2970(s32 index)
{
    return &lbl_1_bss_C[index];
}

const HuVecF lbl_1_rodata_174 = { 0.0f, 200.0f, 0.0f };

const HuVecF lbl_1_rodata_180 = { 0.0f, 0.0f, 200.0f };

void fn_1_557C(OMOBJ *object)
{
    M645CollisionList *work = object->data;
    M645RouteGeometry *geometry;
    s32 entryIndex;
    const s8 **characters;
    s16 secondModel = -1;
    s16 firstModel = -1;
    s32 characterIndex;
    s32 slotIndex;
    s32 routeIndex;
    s32 neighborCount;
    s32 neighborIndex;
    s32 groupIndex;
    s32 playerIndex;
    f32 yaw;
    s32 modelIndex;
    s16 models[2];

    characters = lbl_1_data_574[lbl_1_bss_150].characters;
    work->partCount = 0;
    for (characterIndex = 0; characterIndex < 4; characterIndex++) {
        for (slotIndex = 0; slotIndex < 2; slotIndex++) {
            if (characters[characterIndex][slotIndex] == 0x21) {
                work->partCount++;
            }
        }
    }
    work->parts = HuMemDirectMallocNum(HEAP_HEAP,
        work->partCount * 0x3C, 0x10000000);
    memset(work->parts, 0, work->partCount * 0x3C);
    entryIndex = 0;

    for (characterIndex = 0; characterIndex < 4; characterIndex++) {
        for (slotIndex = 0; slotIndex < 2; slotIndex++) {
            if (characters[characterIndex][slotIndex] == 0x21) {
                HuVecF edgeA = lbl_1_rodata_174;
                HuVecF edgeB = lbl_1_rodata_180;

                HuVecF position = lbl_1_data_378[characterIndex][slotIndex];
                geometry = &work->parts[entryIndex];
                yaw = 360.0f * frandf();
                geometry->triangle.start.x = position.x;
                geometry->triangle.start.y = position.y;
                geometry->triangle.start.z = position.z - 100.0f;
                geometry->triangle.edge1 = edgeA;
                geometry->triangle.edge2 = edgeB;
                fn_1_8C0(&work->parts[entryIndex].bounds, &geometry->triangle);

                if (firstModel == -1) {
                    firstModel = fn_80025A04(fn_80008DD0(
                        0x680005, 0x10000000, 2));
                    models[0] = firstModel;
                } else {
                    models[0] = fn_80025D4C(firstModel);
                }
                if (secondModel == -1) {
                    secondModel = fn_80025A04(fn_80008DD0(
                        0x680006, 0x10000000, 2));
                    models[1] = secondModel;
                } else {
                    models[1] = fn_80025D4C(secondModel);
                }
                for (modelIndex = 0; modelIndex < 2; modelIndex++) {
                    fn_80026B8C(models[modelIndex], 5);
                    fn_80026B64(models[modelIndex],
                        (u16)lbl_1_rodata_B4[modelIndex]);
                    fn_80026790(models[modelIndex], &position);
                    fn_80026808(models[modelIndex], 0.0f, yaw, 0.0f);
                }
                entryIndex++;
            }
        }
    }

    memset(lbl_1_bss_138, 0, 0x14);
    for (characterIndex = 0; characterIndex < 5; characterIndex++) {
        for (slotIndex = 0; slotIndex < 2; slotIndex++) {
            if (lbl_1_data_574[lbl_1_bss_150].characters[characterIndex][slotIndex] == 0x21) {
                fn_1_299C_for_557C(characterIndex, slotIndex + 1);
                break;
            }
        }
    }

    for (routeIndex = 0; routeIndex < 25; routeIndex++) {
        neighborCount = 0;
        if (routeIndex % 5 == 0) {
            neighborCount++;
            if (routeIndex >= 5) neighborCount++;
            if (routeIndex <= 15) neighborCount++;
        } else if (routeIndex % 5 == 4) {
            neighborCount++;
            if (routeIndex >= 9) neighborCount++;
            if (routeIndex <= 19) neighborCount++;
        } else if (routeIndex % 5 == 1) {
            neighborCount++;
            if (fn_1_2984_for_557C(routeIndex / 5) != 1) neighborCount++;
        } else if (routeIndex % 5 == 3) {
            neighborCount++;
            if (fn_1_2984_for_557C(routeIndex / 5) != 2) neighborCount++;
        } else if (routeIndex % 5 == 2) {
            if (fn_1_2984_for_557C(routeIndex / 5) == 0) neighborCount += 2;
            else if (fn_1_2984_for_557C(routeIndex / 5) == 1) neighborCount++;
            else if (fn_1_2984_for_557C(routeIndex / 5) == 2) neighborCount++;
        }
        fn_1_2970(routeIndex)->childCount = neighborCount;
        fn_1_2970(routeIndex)->children = HuMemDirectMallocNum(
            HEAP_HEAP, neighborCount * sizeof(M645RouteEntry *),
            0x10000000);
    }

    for (routeIndex = 0; routeIndex < 25; routeIndex++) {

        neighborIndex = 0;
        if (routeIndex % 5 == 0) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 1);
            if (routeIndex >= 5)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 5);
            if (routeIndex <= 15)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 5);
        } else if (routeIndex % 5 == 4) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 1);
            if (routeIndex >= 9)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 5);
            if (routeIndex <= 19)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 5);
        } else if (routeIndex % 5 == 1) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 1);
            if (fn_1_2984_for_557C(routeIndex / 5) != 1)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 1);
        } else if (routeIndex % 5 == 3) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 1);
            if (fn_1_2984_for_557C(routeIndex / 5) != 2)
                fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 1);
        } else if (routeIndex % 5 == 2) {
          if (fn_1_2984_for_557C(routeIndex / 5) == 0) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 1);
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 1);
        } else if (fn_1_2984_for_557C(routeIndex / 5) == 1) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex + 1);
        } else if (fn_1_2984_for_557C(routeIndex / 5) == 2) {
            fn_1_2970(routeIndex)->children[neighborIndex++] = fn_1_2970(routeIndex - 1);
        }
        }
        fn_1_2970(routeIndex)->pathIndex = routeIndex;
    }
    fn_1_204(object, fn_1_6284);
}

void fn_1_6284(OMOBJ *obj)
{
    M645CollisionList *work = obj->data;

    fn_1_5088(55.0f);
    fn_1_53B4(work, 100.0f);
    if (fn_80071A8C() == 6) {
        fn_1_204(obj, NULL);
        return;
    }
}

void fn_1_62F8(OMOBJ *object)
{
    void *work;

    work = object->data;
}

void fn_1_630C(M645LinkedModelWork *work, s16 motion, f32 transition, s32 attributes)
{
    if (work->motionNo != motion) {
        work->motionNo = motion;
        if (motion == 6 || motion == 7 || motion == 4 || motion == 5) {
            attributes = 0;
        }
        if (work->overlayActive != 0) {
            fn_8002CAF0(work->modelId);
            work->overlayActive = 0;
        }
        fn_8002CCC0(work->modelId,
            work->character->mainObject->mtnId[work->motionNo],
            0.0f, transition, attributes);
        if (motion == 0 || motion == 1 || motion == 2 || motion == 9) {
            fn_8002CAA8(work->modelId,
                work->character->mainObject->mtnId[
                    work->character->assetSelection == 0 ? 0x0A : 0x0B]);
            fn_800269E8(work->modelId, ((1 << 30) | (1 << 9)));
            work->overlayActive = 1;
        }
    }
    if (work->motionNo == 3) {
        fn_8002D414(work->modelId,
            fn_8002D4E0(work->character->mainObject->mdlId[0]));
    }
}

extern void fn_1_65E8(OMOBJ *obj);

void fn_1_649C(OMOBJ *object)
{
    s16 reserved;
    HSF_OBJECT *model;
    HSF_DATA *modelData;
    s16 cameraIndex;
    M645LinkedModelWork *work;

    work = object->data;
    reserved = work->character->characterNo;
    cameraIndex = work->character->routePlayerNo;
    work->modelId = fn_80025D4C(work->character->mainObject->mdlId[0]);
    modelData = lbl_802C01C8[work->modelId].hsf;
    model = modelData->object;
    model->flags &= ~2;
    fn_80026B64(work->modelId,
        (u16)lbl_1_rodata_B4[cameraIndex]);
    fn_800269E8(work->modelId, 0x800000U);
    fn_800273EC(work->modelId, 0.2f,
        0.2f, 0.2f);
    fn_80026B8C(work->modelId, 3);
    fn_800268B0(work->modelId, -1.0f,
        1.0f, 1.0f);
    fn_800269E8(work->modelId, 1U);
    fn_80026790(work->modelId, &work->character->position);
    fn_1_204(object, fn_1_65E8);
}

extern void fn_1_6884(OMOBJ *object);

/* Synchronizes the linked model with its character's current motion. */
void fn_1_65E8(OMOBJ *object)
{
    M645LinkedModelWork *work;
    HuVecF rotation, scale, position;
    s16 characterModel;

    work = object->data;
    characterModel = work->character->mainObject->mdlId[0];
    position = work->character->position;
    position.z = -300.0f;
    fn_80026790(work->modelId, &position);

    fn_1_630C(work, work->character->motionNo, 5.0f, ((1 << 30) | (1 << 0)));

    fn_1_2B8(&rotation, 0.0f,
        (180.0f * work->character->facing) /
            M_PI,
        0.0f);
    fn_80026808(work->modelId, rotation.x,
        180.0f + (f32)-rotation.y, rotation.z);

    fn_8002691C(characterModel, &scale);
    fn_800268B0(work->modelId, -scale.x, scale.y, scale.z);

    if (fn_80071A8C() == 5) {
        fn_80026A44(work->modelId, 1);
        fn_1_204(object, fn_1_6884);
        return;
    }
}

/* Updates the linked model's motion, overlay, rotation, and mirrored scale. */
void fn_1_6884(OMOBJ *object)
{
    M645LinkedModelWork *work;
    HuVecF rotation, scale, position;
    s16 characterModel;

    work = object->data;
    characterModel = work->character->mainObject->mdlId[0];
    position = work->character->position;
    position.z = -300.0f;
    fn_80026790(work->modelId, &position);

    fn_1_630C(work, work->character->motionNo, 5.0f, ((1 << 30) | (1 << 0)));

    fn_1_2B8(&rotation, 0.0f,
        (180.0f * work->character->facing) /
            M_PI,
        0.0f);
    fn_80026808(work->modelId, rotation.x,
        180.0f + (f32)-rotation.y, rotation.z);

    fn_8002691C(characterModel, &scale);
    fn_800268B0(work->modelId, -scale.x, scale.y, scale.z);

    if (fn_80071A8C() == 6) {
        fn_1_204(object, fn_1_6B14);
        return;
    }
}

extern char *lbl_1_data_58[];

void fn_1_6B14(OMOBJ *obj)
{
    M645LinkedModelWork *work;

    HuVecF position;

    work = obj->data;

    position = work->character->position;
    position.z = -300.0f;
    fn_80026790(work->modelId, &position);
    if (fn_80071A8C() == 8) {
        fn_1_204(obj, fn_1_65E8);
        return;
    }
}

void fn_1_6BA0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *work = particle->data;
    f32 verticalWave;

    if (work->time < 0) {
        return;
    }
    if (work->time < 8) {
        verticalWave = (f32)(0.3f + sin((M_PI * (f64)(
                40.0f + 10.0f * (f32)(work->time + 1))) /
                180.0));
        work->scale = 50.0f * verticalWave;
        work->color.a = 0xFF;
        verticalWave = (f32)sin((M_PI * (f64)(
            0.3f + 15.0f * (f32)(work->time + 1))) /
            180.0);
    } else {
        verticalWave = (f32)sin(((M_PI * 135.3f) / 180.0));
    }
    work->pos.x = work->accel.x;
    work->pos.y = work->accel.y + 150.0f * verticalWave;
    work->pos.z = work->accel.z;
    work->time++;
    if (work->time > 20) {
        work->color.a -= 0x20;
        work->scale -= 8.0f;
        if (work->scale < 0.0f) {
            work->scale = 0.0f;
            work->time = -1;
        }
    }
}

void fn_1_6DC0(M645CharacterSetupWork *object)
{
    HU3D_PARTICLE * particle;
    s32 useLargeOffset = 1;
    f32 offset;
    HuVecF position;
    f32 stateOffset;
    f32 verticalOffset;
    HU3D_PARTICLE_DATA * particleData;
    if (object->characterNo != 2 && object->characterNo != 5)
    {
        useLargeOffset = 0;
    }
    if (useLargeOffset != 0)
    {
        offset = 120.0f;
    }
    else
    {
        if (object->characterNo == 6)
        {
            stateOffset = 150.0f;
        }
        else
        {
            stateOffset = 100.0f;
        }
        offset = stateOffset;
    }
    verticalOffset = offset;
    Hu3DModelObjPosGet(object->routeObject->mdlId[0], lbl_1_data_58[object->assetSelection], & position);
    particle = lbl_802C01C8[object->particleModelId].hookData;
    particleData = particle->data;
    particleData->time = 0;
    particleData->scale = 1.0f;
    particleData->accel.x = position.x;
    particleData->accel.y = position.y + verticalOffset;
    particleData->accel.z = position.z;
    particleData->time = 0;
}

void fn_1_6F20(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    HU3D_PARTICLE_DATA *data = particle->data;
    s32 i;
    s32 alpha;

    for (i = 0; i < particle->maxCnt; i++, data++) {
        if (data->time == 0) {
            data->time++;
            data->vel.x = 2.0f * frandf() - 1.0f;
            data->vel.y = 2.0f * frandf() - 1.0f;
            data->vel.z = 0.0f;
            data->color.r = data->color.g = data->color.b = 224;
            data->color.a = 128;
            data->scale = 56.0f;
        }
        if (data->time == 1) {
            fn_1_480(&data->pos, &data->pos, &data->vel);
            data->scale *= 1.075f;
            alpha = data->color.a - 8;
            if (alpha < 0) {
                alpha = 0;
                data->scale = 0.0f;
                data->time = -1;
            }
            data->color.a = alpha;
        }
    }
}

s32 fn_1_7088(s32 groupIndex)
{
    void *groupData = lbl_1_bss_160[groupIndex];
    M645CharacterSetupWork *state = groupData;
    s32 tableIndex = state->pathState;
    s32 result;
    s32 direction;
    s32 phase;
    s32 outcome;

    if (state->setupMode == 1) {
        if (state->sideIndex == 0) {
            direction = 0;
        } else {
            direction = 4;
        }
        result = direction;
    } else {
        if (lbl_1_data_378[tableIndex][0].x > state->position.x) {
            phase = 1;
        } else {
            if (lbl_1_data_378[tableIndex][1].x < state->position.x) {
                outcome = 3;
            } else {
                outcome = 2;
            }
            phase = outcome;
        }
        result = phase;
    }
    return result + tableIndex * 5;
}

static inline void M645MotionChange(M645CharacterSetupWork *work, s16 motion, f32 transition, s32 attributes,
    s32 playSound)
{
    if (work->motionNo != motion) {
        work->motionNo = motion;
        if (work->overlayActive != 0) {
            fn_8002CAF0(work->mainObject->mdlId[0]);
            work->overlayActive = 0;
        }
        fn_8005C2C8(work->characterNo,
            work->mainObject->mtnId[work->motionNo],
            0.0f, transition, attributes);
        if (motion == 0 || motion == 1 || motion == 2 || motion == 9) {
            fn_8002CAA8(work->mainObject->mdlId[0],
                work->mainObject->mtnId[work->assetSelection == 0 ? 0x0A : 0x0B]);
            fn_800269E8(work->mainObject->mdlId[0], ((1 << 30) | (1 << 9)));
            work->overlayActive = 1;
        }
        if (playSound != 0) {
            if (motion == 4) {
                HuAudFXPlay(work->routePlayerNo == 0 ? 0x7B7 : 0x7B8);
            } else if (motion == 5) {
                HuAudFXPlay(work->routePlayerNo == 0 ? 0x7B9 : 0x7BA);
            }
        }
    }
}

void fn_1_7160(M645CharacterSetupWork *work, s16 motion, f32 transition, s32 attributes, s32 playSound)
{
    M645MotionChange(work, motion, transition, attributes, playSound);
}

extern f32 fn_1_418(HuVecF *a, HuVecF *b);

f32 fn_1_72FC(f32 angle, f32 heading, f32 rate)
{
    HuVecF firstDirection;
    HuVecF secondDirection;
    f32 angularDifference;
    f32 cross;
    f32 adjustedAngle;

    fn_1_2B8(&firstDirection, (f32)sin(heading), 0.0f,
        (f32)cos(heading));
    fn_1_2B8(&secondDirection, (f32)sin(angle), 0.0f,
        (f32)cos(angle));
    angularDifference = fn_1_20C(
        fn_1_418(&firstDirection, &secondDirection));
    cross = firstDirection.x * secondDirection.z -
        firstDirection.z * secondDirection.x;
    adjustedAngle = angle;
    if (cross >= 0.0f) {
        adjustedAngle += angularDifference / rate;
    } else if (cross < 0.0f) {
        adjustedAngle -= angularDifference / rate;
    }
    while (adjustedAngle > ((M_PI * 360.0) / 180.0)) {
        adjustedAngle -= ((M_PI * 360.0) / 180.0);
    }
    while (adjustedAngle < -((M_PI * 360.0) / 180.0)) {
        adjustedAngle += ((M_PI * 360.0) / 180.0);
    }
    return adjustedAngle;
}

extern HuVecF lbl_1_data_F0[];

void fn_1_74C4(M645CharacterSetupWork *work)
{
    work->position = lbl_1_data_F0[work->variation];
    fn_80026790(
        work->mainObject->mdlId[0], &work->position);
    work->facing = 0.0f;
    fn_80026808(work->mainObject->mdlId[0],
        0.0f,
        (180.0f * work->facing) /
            M_PI,
        0.0f);
    work->motionNo = -1;
    fn_1_7160(work, 0, 0.0f, ((1 << 30) | (1 << 0)), 0);
    fn_8002D86C(work->mainObject->mdlId[0], 1.0f);
}

void fn_1_7694(M645CharacterSetupWork *work)
{
    work->position = lbl_1_data_F0[work->variation];
    fn_80026790(
        work->mainObject->mdlId[0], &work->position);
    work->facing = 0.0f;
    fn_80026808(work->mainObject->mdlId[0],
        0.0f,
        (180.0f * work->facing) /
            M_PI,
        0.0f);
    work->motionNo = -1;
    fn_1_7160(work, 0x0C, 0.0f, ((1 << 30) | (1 << 0)), 0);
    fn_8002D86C(work->mainObject->mdlId[0], 1.0f);
}

void fn_1_780C(void *object)
{
    HuVecF position;
    f32 scale;

    position = ((M645CharacterSetupWork *)object)->position;
    if (((M645CharacterSetupWork *)object)->setupMode == 1) {
        scale = 1.0f -
            (((M645CharacterSetupWork *)object)->position.y / 1500.0f);
        position.y = 0.0f;
        fn_800268B0(((M645CharacterSetupWork *)object)->shadowModelId, scale,
            1.0f, scale);
    } else {
        fn_800268B0(((M645CharacterSetupWork *)object)->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(((M645CharacterSetupWork *)object)->shadowModelId, &position);
}

static inline void fn_1_780CInline(M645CharacterSetupWork *object)
{
    HuVecF position;
    f32 scale;

    position = object->position;
    if (object->setupMode == 1) {
        scale = 1.0f -
            (object->position.y / 1500.0f);
        position.y = 0.0f;
        fn_800268B0(object->shadowModelId, scale,
            1.0f, scale);
    } else {
        fn_800268B0(object->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(object->shadowModelId, &position);
}

extern f32 fn_1_2C8(const HuVecF *movement);

extern f32 fn_1_2518(M645Sphere *routeSphere, M645Sphere *playerSphere);

static void fn_1_780CInline(M645CharacterSetupWork *object);

static inline M645RouteObject *fn_1_401C_for_78F4(s16 groupIndex, s16 itemIndex)
{
    M645RouteObject *value;

    value = lbl_1_bss_15C[groupIndex]->routeObjects[itemIndex];
    return value;
}

/* Updates movement, facing, and route selection for a board character. */
s32 fn_1_78F4(M645CharacterSetupWork *character, HuVecF *movement, s32 routeCheck)
{
    f32 movementLength;
    f32 horizontalStep;
    f32 targetAngle;
    f32 currentAngle;
    f32 movementX;
    f32 movementZ;
    M645Sphere playerSphere;
    M645Sphere routeSphere;
    s16 desiredMotion;
    M645RouteObject *routeNode;
    s32 index;
    movementLength = fn_1_2C8(movement);
    horizontalStep = 8.0f * movement->x;
    desiredMotion = movementLength >= 0.8f ? 2 : movementLength > 0.0f ? 1 : 0;
    fn_1_7160(character, desiredMotion, 8.0f, ((1 << 30) | (1 << 0)), 1);
    if (movementLength > 0.0f)
    {
        movementZ = movement->z;
        movementX = movement->x;
        targetAngle = atan2(movementX, movementZ);
        currentAngle = character->facing;
        character->facing = fn_1_72FC(currentAngle, targetAngle, 10.0f);
    }
    character->position.x += horizontalStep;
    fn_80026790(character->mainObject->mdlId[0], &character->position);
    fn_80026808(character->mainObject->mdlId[0], 0.0f,
        (180.0f * character->facing) / M_PI, 0.0f);
    fn_1_780C(character);
    if (routeCheck != 0)
    {
        for (index = 0; index < 25; index++)
        {
            playerSphere.center = character->position;
            playerSphere.center.y += 40.0f;
            playerSphere.radius = 40.0f;
            routeSphere.center = lbl_1_data_24C[index];
            routeSphere.center.z = 0.0f;
            routeSphere.radius = 70.0f;
            if (fn_1_2518(&routeSphere, &playerSphere) <= 0.0f)
            {
                routeNode = fn_1_401C_for_78F4(character->routePlayerNo, index);
                if (character->assetSelection == 0)
                {
                    if (fn_1_401C_for_78F4(
                            character->routePlayerNo, index)->phase == 0)
                    {
                        fn_1_401C_for_78F4(
                            character->routePlayerNo, index)->phase = 1;
                        character->routeIndex = index;
                        character->setupMode = 2;
                        return 1;
                    }
                }
                if (character->assetSelection == 1)
                {
                    if (fn_1_401C_for_78F4(
                            character->routePlayerNo, index)->phase == 3)
                    {
                        fn_1_401C_for_78F4(
                            character->routePlayerNo, index)->phase = 4;
                        character->routeIndex = index;
                        character->setupMode = 3;
                        return 1;
                    }
                }
            }
        }
    }
    return 0;
}

/* Applies vertical route movement and keeps the player's position in bounds. */
const HuVecF lbl_1_rodata_214 = { 0.0f, 0.0f, 0.0f };

void fn_1_80CC(M645CharacterSetupWork *work, HuVecF *inputMovement)
{
    s16 model;
    HuVecF movement;
    f32 verticalDelta;
    f32 laneDifference;
    f32 directionX;
    s32 clamped;
    s32 lane;

    movement = lbl_1_rodata_214;
    model = work->mainObject->mdlId[0];
    clamped = 0;
    verticalDelta = 8.0f * inputMovement->y;
    directionX = inputMovement->x;
    work->facing = 3.1415927f;
    movement.y += verticalDelta;
    movement.x += (lbl_1_rodata_70[work->sideIndex].start.x -
        work->position.x) /
        5.0f;

    if (work->position.y < 0.0f) {
        work->position.y = 0.0f;
        clamped = 1;
    }
    if (work->position.y > 1250.0f) {
        work->position.y = 1250.0f;
        clamped = 1;
    }
    for (lane = 0; lane < 5; lane++) {
        if (work->position.y >= lbl_1_rodata_A0[lane] - 100.0f &&
            work->position.y < 100.0f + lbl_1_rodata_A0[lane] &&
            ((work->sideIndex == 0 && directionX >= 0.5f) ||
             (work->sideIndex == 1 && directionX <= -0.5f))) {
            laneDifference = lbl_1_rodata_A0[lane] - work->position.y;
            laneDifference = laneDifference > 8.0f ? 8.0f
                : laneDifference < -8.0f ? -8.0f : laneDifference;
            movement.y = laneDifference;
            if (fabsf(lbl_1_rodata_A0[lane] - work->position.y) <
                0.02f) {
                movement.x = 8.0f * directionX;
                movement.y = 0.0f;
            }
        }
    }
    for (lane = 0; lane < 5; lane++) {
        if (work->position.y >= lbl_1_rodata_A0[lane]) {
            work->pathState = (s16)lane;
            break;
        }
    }
    if (movement.y > 8.0f) {
        movement.y = 8.0f;
    }
    if (movement.y < -8.0f) {
        movement.y = -8.0f;
    }
    fn_1_480(&work->position, &work->position, &movement);
    if (work->position.y < 0.0f) {
        work->position.y = 0.0f;
        clamped = 1;
    }
    if (work->position.y > 1250.0f) {
        work->position.y = 1250.0f;
        clamped = 1;
    }
    fn_1_7160(work, 3, 8.0f, ((1 << 30) | (1 << 0)), 0);
    if (clamped != 0) {
        fn_8002D86C(model, 0.0f);
    } else {
        fn_8002D86C(model, 2.0f * (fabsf(movement.y) / 8.0f));
    }
    fn_80026790(model, &work->position);
    fn_80026808(model, 0.0f,
        (f32)((f64)(180.0f * work->facing) /
            M_PI), 0.0f);
    fn_1_780CInline(work);
}

extern HUPROCESS *lbl_1_bss_0;

extern s32 lbl_1_data_60[2];

extern s32 lbl_1_data_68[2][2];

extern u32 lbl_1_data_78[14];

typedef void *(*M645SetupAllocator)(s32 priority, u32 bytes, void (*update)(OMOBJ *));

extern void fn_1_6BA0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern void fn_1_6F20(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern void fn_1_8F98(OMOBJ *object);

extern void fn_1_AD20(OMOBJ *object);

extern void fn_1_A858(OMOBJ *object);

/* Create the character model, effects and linked board display models. */
void fn_1_8738(OMOBJ *object)
{
    M645CharacterSetupWork * work = object->data;
    s32 routePlayer;
    s16 characterModel;
    s32 particleIndex;
    s32 selectionIndex;
    s32 jointIndex;
    s32 tableIndex;
    s32 particleModel;
    s32 camera;
    s32 motion;
    s16 playerNo;
    s16 assetMode;
    HU3D_PARTICLE *particleData;
    HU3D_PARTICLE_DATA *particleEntry;
    HU3D_PARTICLE *childParticleData;
    HU3D_PARTICLE_DATA *childParticleEntry;
    void * animationData;
    void *animation;
    s32 childIndex;
    s16 flag;
    M645LinkedModelWork *linked;
    s32 assetValues[4] = { 0, 1, 0, 1 };
    OMOBJ * mainObject;
    OMOBJ * displayObject;
    work->characterNo = lbl_80265728[work->playerNo].charNo;
    work->motionNo = - 1;
    work->setupMode = 0;
    work->assetSelection = assetValues[work->variation];
    work->pathState = 4;
    for (routePlayer = 0; routePlayer < 4; routePlayer++)
    {
        if (routePlayer != work->playerNo) {
            M645CharacterSetupWork *peer = lbl_1_bss_160[routePlayer];
            if (work->routePlayerNo == peer->routePlayerNo) {
                work->pairedPlayerNo = routePlayer;
            }
        }
    }
    work->right.setup.resetWord = 0;
    work->modelHeight = fn_8005C4C8(work->characterNo);
    work->segmentNo = 0;
    work->setupState = (s16)(rand8() % 180);
    work->randomAngle = (s16)(rand8() % 2);
    work->mainObject = fn_8003AB38(lbl_1_bss_0, 0x65, 3, 0x0E, 0, NULL);
    omSetStatBit(work->mainObject, 0x100);
    characterModel = work->mainObject->mdlId[0] = fn_80057068(work->characterNo, 4);
    fn_80026B64(characterModel, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(characterModel, 7);
    work->displayModelId = fn_80025A04(fn_80008DD0(lbl_1_data_68[work->routePlayerNo][work->assetSelection], 0x10000000, 2));
    fn_80026B64(work->displayModelId, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(work->displayModelId, 7);
    work->mainObject->mdlId[2] = fn_8003216C(HuSprAnimRead(fn_80008634(0x210002, 0x10000000)), 0x3C);
    fn_80026B64(work->mainObject->mdlId[2], (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(work->mainObject->mdlId[2], 6);
    fn_8002741C(characterModel, fn_8005C0F0(work->characterNo, 4, 0), work->displayModelId);
    for (routePlayer = 0; routePlayer < 14; routePlayer++)
    {
        work->mainObject->mtnId[routePlayer] = fn_8005B0B8(work->characterNo, lbl_1_data_78[routePlayer]);
    }
    fn_8005B4BC(work->characterNo);
    fn_1_7160(work, 0, 0.0f, ((1 << 30) | (1 << 0)), 0);
    work->routeObject = fn_8003AB38(lbl_1_bss_0, 0x28, 1, 1, - 1, NULL);
    work->routeObject->mdlId[0] = fn_80025A04(fn_80008DD0(lbl_1_data_60[work->assetSelection], 0x10000000, 2));
    work->routeObject->mtnId[0] = fn_80030458(work->routeObject->mdlId[0], fn_80008DD0(lbl_1_data_60[work->assetSelection], 0x10000000, 2));
    fn_80026B64(work->routeObject->mdlId[0], (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(work->routeObject->mdlId[0], 6);
    work->shadowModelId = fn_80025A04(fn_80008DD0(0x68001C, 0x10000000, 2));
    fn_80026B64(work->shadowModelId, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(work->shadowModelId, 7);
    animationData = fn_80008DD0(0x210007, 0x10000000, 2);
    animation = HuSprAnimRead(animationData);
    work->particleModelId = fn_8003216C(animation, 1);
    fn_800326D0(work->particleModelId, fn_1_6BA0);
    fn_80026B64(work->particleModelId, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
    fn_80026B8C(work->particleModelId, 5);
    fn_800269E8(work->particleModelId, 0x400000);
    fn_8003269C(work->particleModelId, 0);
    particleData = lbl_802C01C8[work->particleModelId].hookData;
    particleEntry = particleData->data;
    for (particleIndex = 0; particleIndex < particleData->maxCnt; particleIndex++, particleEntry++)
    {
        particleEntry->scale = 0.0f;
        particleEntry->time = - 1;
    }
    childParticleData = lbl_802C01C8[work->mainObject->mdlId[2]].hookData;
    fn_8003269C(work->mainObject->mdlId[2], 1);
    fn_800326D0(work->mainObject->mdlId[2], fn_1_6F20);
    fn_80026760(work->mainObject->mdlId[2], 0.0f, 0.0f, 0.0f);
    childParticleEntry = childParticleData->data;
    for (childIndex = 0; childIndex < childParticleData->maxCnt; childIndex++, childParticleEntry++)
    {
        childParticleEntry->time = - 1;
        childParticleEntry->scale = 0.0f;
    }
    if (work->assetSelection == 0)
    {
        work->leftModelId = fn_80025A04(fn_80008DD0(0x680004, 0x10000000, 2));
        fn_80026B8C(work->leftModelId, 5);
        fn_80026B64(work->leftModelId, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
        fn_80026760(work->leftModelId, 265.0f, 152.0f, - 75.0f);
        fn_800269E8(work->leftModelId, 0x40000002);
        work->right.model.modelId = fn_80025A04(fn_80008DD0(0x68000C, 0x10000000, 2));
        fn_80026B8C(work->right.model.modelId, 5);
        fn_80026B64(work->right.model.modelId, (u16) lbl_1_rodata_B4[work->routePlayerNo]);
        fn_80026760(work->right.model.modelId, 265.0f, 152.0f, - 65.0f);
        fn_800269E8(work->right.model.modelId, 1);
        for (routePlayer = 0; routePlayer < 4; routePlayer++)
        {
            work->right.model.motionIds[routePlayer] = fn_80030458(work->right.model.modelId, fn_80008DD0(lbl_1_data_E0[routePlayer], 0x10000000, 2));
        }
        fn_8002C95C(work->right.model.modelId, work->right.model.motionIds[0]);
    }
    flag = lbl_802C0278;
    if (flag == 0)
    {
        linked = fn_1_164(0x1E, 0x10, fn_1_649C);
        linked->character = work;
    }
    fn_1_204(object, fn_1_8F98);
}

extern void fn_1_90B4(OMOBJ *object);

void fn_1_8F98(OMOBJ *object)
{
    M645CharacterSetupWork *child = object->data;
    HuVecF position;
    f32 scale;

    position = child->position;
    if (child->setupMode == 1) {
        scale = 1.0f -
            (child->position.y / 1500.0f);
        position.y = 0.0f;
        fn_800268B0(child->shadowModelId, scale,
            1.0f, scale);
    } else {
        fn_800268B0(child->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(child->shadowModelId, &position);
    if (fn_80071A8C() == 2) {
        child->motionState = 0;
        fn_1_204(object, fn_1_90B4);
        return;
    }
}

const HuVecF lbl_1_rodata_258 = { 265.0f, 152.0f, -75.0f };

void fn_1_90B4(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    Mtx matrix;
    Mtx hookMatrix;
    HuVecF position;
    HuVecF rotation;
    HuVecF hiddenPosition;
    HuVecF hookPosition;
    HuVecF shadowPosition;
    s16 characterModel;
    s32 overlayMotion;
    f32 frame;
    HU3D_PARTICLE *particleData;
    s32 itemIndex;
    HU3D_PARTICLE_DATA *particleEntry;
    f32 bob;

    fn_80023398(work->routeObject->mdlId[0],
        lbl_1_data_58[work->assetSelection], matrix);
    Hu3DMtxTransGet(matrix, &position);
    Hu3DMtxRotGet(matrix, &rotation);
    fn_80026790(work->mainObject->mdlId[0], &position);
    fn_80026838(work->mainObject->mdlId[0], &rotation);
    work->position = position;
    work->facing = (f32)(M_PI +
        M_PI * (rotation.y / -180.0f));

    fn_1_780C(work);
    work->motionState++;

    if (work->assetSelection == 0) {
        if (work->motionState == 1) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x5B) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x65) {
            fn_1_7160(work, 8, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x92) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0xBF) {
            fn_1_7160(work, 4, 5.0f, 0, 0);
            fn_8006A030(work->playerNo, 0xC, 7, 3);
            HuAudFXPlayPan(0x7BF, 0x40);
        } else if (work->motionState == 0x10F) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x14B) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x1B9) {
            fn_1_7160(work, 2, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x209) {
            fn_1_7160(work, 9, 5.0f, ((1 << 30) | (1 << 0)), 0);
            fn_8002D414(work->mainObject->mdlId[0], 61.0f);
        } else if (work->motionState == 0x245) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        }
        if (work->motionState == 0xBF) {
            fn_8002C95C(work->right.model.modelId, work->right.model.motionIds[1]);
            fn_80026A44(work->right.model.modelId, 1);
        } else if (work->motionState == 0x12D) {
            fn_8002C95C(work->right.model.modelId, work->right.model.motionIds[2]);
        } else if (work->motionState == 0x141) {
            fn_8002C95C(work->right.model.modelId, work->right.model.motionIds[3]);
        } else if (work->motionState == 0x191) {
            hiddenPosition = lbl_1_rodata_258;
            fn_800269E8(work->leftModelId, 1);
            fn_800269E8(work->right.model.modelId, 1);
            fn_1_420C(lbl_1_bss_15C[work->routePlayerNo], &hiddenPosition);
        }
        if (work->motionState == 0x14B) {
            fn_80026A44(work->leftModelId, 0x40000002);
        }
        if (work->motionState >= 0xBF && work->motionState <= 0x10E) {
            characterModel = work->mainObject->mdlId[2];
            particleData = lbl_802C01C8[characterModel].hookData;
            particleEntry = particleData->data;
            fn_80023398(work->mainObject->mdlId[0],
                fn_8005C0F0(work->characterNo, 4, 0), hookMatrix);
            Hu3DMtxTransGet(hookMatrix, &hookPosition);
            for (itemIndex = 0; itemIndex < particleData->maxCnt; itemIndex++, particleEntry++) {
                if (particleEntry->time < 0) {
                    particleEntry->time = 0;
                    particleEntry->pos = hookPosition;
                    break;
                }
            }
        }
    } else if (work->assetSelection == 1) {
        if (work->motionState == 1) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x65) {
            fn_1_7160(work, 8, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x92) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0xFB) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x141) {
            fn_1_7160(work, 5, 5.0f, 0, 0);
            fn_8006A030(work->playerNo, 0xC, 7, 3);
            HuAudFXPlayPan(0x7C0, 0x40);
        } else if (work->motionState == 0x191) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x195) {
            fn_1_7160(work, 9, 5.0f, ((1 << 30) | (1 << 0)), 0);
            fn_8002D414(work->mainObject->mdlId[0], 61.0f);
        } else if (work->motionState == 0x1CD) {
            fn_1_7160(work, 1, 5.0f, ((1 << 30) | (1 << 0)), 0);
        } else if (work->motionState == 0x231) {
            fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        }

    }

    if (work->motionState == 0x6A) {
        fn_80027508(work->mainObject->mdlId[0]);
    }
    if (work->motionState == 0x87) {
        fn_80026760(work->displayModelId, 0.0f,
            0.0f, 0.0f);
        fn_80026808(work->displayModelId, 0.0f,
            0.0f, 0.0f);
        fn_8002741C(work->mainObject->mdlId[0],
            fn_8005C0F0(work->characterNo, 4, 0),
            work->displayModelId);
    } else if (work->motionState >= 0x6A && work->motionState <= 0x87) {
        frame = work->motionState - 0x6A;
        bob = (f32)(100.0 *
            sin(((M_PI * 6.0) / 180.0) * (f64)frame));
        Hu3DModelObjPosGet(work->mainObject->mdlId[0],
            fn_8005C0F0(work->characterNo, 4, 0), &shadowPosition);
        shadowPosition.y += bob;
        fn_80026790(work->displayModelId, &shadowPosition);
        fn_80026808(work->displayModelId, 20.0f * frame,
            0.0f, 0.0f);
    }

    if (fn_80071A8C() == 5) {
        work->motionState = 0;
        fn_1_74C4(work);
        if (lbl_80265728[work->playerNo].type != 0) {
            fn_1_204(object, fn_1_AD20);
            return;
        }
        fn_1_204(object, fn_1_A858);
        return;
    }
}

extern void fn_1_BFCC(OMOBJ *object);

extern void fn_1_B090(OMOBJ *object);

extern void fn_1_AB2C(OMOBJ *object);

extern f32 fn_1_2178(M645Sphere *sphere, const M645Segment *segment);

extern void fn_1_AF3C(OMOBJ *object);

/* Applies stick movement, changes motion overlays, and checks nearby path segments. */
void fn_1_A858(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    HuVecF movement;
    M645Sphere sphere;
    s16 padNo = lbl_80265728[work->playerNo].padNo;
    s32 segmentIndex;

    if (fn_80071A8C() == 6) {
        fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        fn_1_204(object, fn_1_BFCC);
        return;
    }

    fn_1_2B8(&movement,
        (f32)lbl_802BFFFC[padNo] / 56.0f,
        0.0f,
        (f32)-lbl_802BFFF8[padNo] / 56.0f);
    if (fn_1_78F4(work, &movement, lbl_802C0008[padNo] & 0x0100) != 0) {
        fn_1_204(object, fn_1_B090);
        return;
    }
    for (segmentIndex = 0; segmentIndex < 2; segmentIndex++) {
        sphere.center = work->position;
        sphere.center.y += 40.0f;
        sphere.radius = 40.0f;
        if (fn_1_2178(&sphere,
                &lbl_1_rodata_70[segmentIndex]) <=
            0.0f) {
            work->sideIndex = (s16)segmentIndex;
            work->setupMode = 1;
            fn_1_204(object, fn_1_AB2C);
            return;
        }
    }
}

void fn_1_AB2C(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    HuVecF movement;
    M645Sphere sphere;
    s16 modelId = work->mainObject->mdlId[0];
    s16 padNo = lbl_80265728[work->playerNo].padNo;

    if (fn_80071A8C() == 6) {
        fn_8002D86C(modelId, 0.0f);
        fn_1_204(object, fn_1_BFCC);
        return;
    }
    fn_1_2B8(&movement,
        (f32)lbl_802BFFFC[padNo] / 56.0f,
        (f32)lbl_802BFFF8[padNo] / 56.0f, 0.0f);
    fn_1_80CC(work, &movement);
    sphere.center = work->position;
    sphere.center.y += 40.0f;
    sphere.radius = 40.0f;
    if (fn_1_2178(&sphere,
        &lbl_1_rodata_70[work->sideIndex]) > 0.0f) {
        fn_8002D86C(modelId, 1.0f);
        work->setupMode = 0;
        fn_1_204(object, fn_1_A858);
        return;
    }
}

void fn_1_AD20(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    M645Sphere sphere;
    s32 segmentIndex;

    if (fn_80071A8C() == 6) {
        fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        fn_1_204(object, fn_1_BFCC);
        return;
    }

    if (lbl_1_rodata_60[
            lbl_80265728[work->playerNo].comDif](work) != 0) {
        fn_1_204(object, fn_1_B090);
        return;
    }
    for (segmentIndex = 0; segmentIndex < 2; segmentIndex++) {
        sphere.center = work->position;
        sphere.center.y += 40.0f;
        sphere.radius = 40.0f;
        if (fn_1_2178(&sphere,
                &lbl_1_rodata_70[segmentIndex]) <=
            0.0f) {
            work->sideIndex = (s16)segmentIndex;
            work->setupMode = 1;
            fn_1_204(object, fn_1_AF3C);
            return;
        }
    }
}

void fn_1_AF3C(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    s16 modelId = work->mainObject->mdlId[0];
    M645Sphere sphere;

    if (fn_80071A8C() == 6) {
        fn_8002D86C(modelId, 0.0f);
        fn_1_204(object, fn_1_BFCC);
        return;
    }
    lbl_1_rodata_60[lbl_80265728[work->playerNo].comDif](work);
    sphere.center = work->position;
    sphere.center.y += 40.0f;
    sphere.radius = 40.0f;
    if (fn_1_2178(&sphere,
        &lbl_1_rodata_70[work->sideIndex]) >
        0.0f) {
        fn_8002D86C(modelId, 1.0f);
        work->setupMode = 0;
        fn_1_204(object, fn_1_AD20);
        return;
    }
}

extern void fn_1_BAB0(OMOBJ *object);

extern void fn_1_BD94(OMOBJ *object);

static inline M645RouteObject *fn_1_401C_for_B090(s16 groupIndex, s16 itemIndex)
{
    M645RouteObject *value;

    value = lbl_1_bss_15C[groupIndex]->routeObjects[itemIndex];
    return value;
}

/* Moves the character model toward its route point and changes its motion state. */
void fn_1_B090(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    HuVecF targetPosition;
    HuVecF firstDirection;
    HuVecF secondDirection;
    HuVecF alternateDirection;
    HuVecF currentDirection;
    Point3d modelPosition;
    f32 distance;
    f32 speed;
    if (fn_80071A8C() == 6)
    {
        fn_1_7160(work, 0, 5.0f, ((1 << 30) | (1 << 0)), 0);
        fn_1_204(object, fn_1_BFCC);
        return;
    }
    targetPosition = lbl_1_data_24C[work->routeIndex];
    targetPosition.y = work->position.y;
    targetPosition.z = work->position.z;
    fn_1_4B8(& firstDirection, & targetPosition, & work->position);
    distance = fn_1_4F0(& firstDirection, & firstDirection);
    speed = distance > 8.0f ? 8.0f : distance;
    if (distance <= 0.01f)
    {
        fn_1_7160(work, 1, 8.0f, ((1 << 30) | (1 << 0)), 0);
        work->facing = fn_1_72FC(work->facing, 3.1415927f, 10.0f);
    }
    else
    {
        fn_1_7160(work, 2, 8.0f, ((1 << 30) | (1 << 0)), 0);
        if (speed > 0.0f)
        {
            work->facing = fn_1_72FC(work->facing, atan2f(firstDirection.x, firstDirection.z), 10.0f);
        }
    }
    work->position.x += speed * firstDirection.x;
    fn_80026790(work->mainObject->mdlId[0], & work->position);
    fn_80026808(work->mainObject->mdlId[0], 0.0f, (f32)((f64)(180.0f * work->facing) / M_PI), 0.0f);
    fn_1_780C(work);
    if (distance <= 0.01f && fabsf((f32)((f64) fabsf(work->facing) - M_PI)) <= 0.1f)
    {
        work->facing = 3.1415927f;
        if (work->setupMode == 2)
        {
            M645RouteObject *pathSlot;
            pathSlot = fn_1_401C_for_B090(work->routePlayerNo, work->routeIndex);
            pathSlot->timer = 0;
            pathSlot->phase = 2;
            fn_8006A030(work->playerNo, 0xC, 7, 3);
            fn_1_204(object, fn_1_BAB0);
            return;
        }
        if (work->setupMode == 3)
        {
            M645RouteObject *pathSlot;
            pathSlot = fn_1_401C_for_B090(work->routePlayerNo, work->routeIndex);
            pathSlot->timer = 0;
            pathSlot->phase = 5;
            fn_8006A030(work->playerNo, 0xC, 7, 3);
            fn_1_204(object, fn_1_BD94);
            return;
        }
    }
}

/* Applies the result pose and attaches it to the first available model item. */
static inline M645RouteObject *fn_1_401C_for_BAB0(s16 groupIndex, s16 itemIndex)
{
    M645RouteObject *value;

    value = lbl_1_bss_15C[groupIndex]->routeObjects[itemIndex];
    return value;
}

void fn_1_BAB0(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    s16 characterModel;
    HU3D_PARTICLE *modelData;
    HU3D_PARTICLE_DATA *item;
    Mtx matrix;
    HuVecF position;
    s32 itemIndex;
    f32 scale;

    fn_1_7160(work, 4, 8.0f, 0, 1);
    if (fn_80071A8C() == 6) {
        fn_1_204(object, fn_1_BFCC);
        return;
    }

    characterModel = work->mainObject->mdlId[2];
    modelData = lbl_802C01C8[characterModel].hookData;
    item = modelData->data;
    fn_80023398(
        work->mainObject->mdlId[0],
        fn_8005C0F0(work->characterNo, 4, 0), matrix);
    Hu3DMtxTransGet(matrix, &position);
    for (itemIndex = 0; itemIndex < modelData->maxCnt;) {
        if (item->time < 0) {
            item->time = 0;
            item->pos = position;
            break;
        }
        itemIndex++;
        item++;
    }

    fn_1_780C(work);

    if (fn_1_401C_for_BAB0(work->routePlayerNo, work->routeIndex)->phase == 3) {
        work->setupMode = 0;
        if (lbl_80265728[work->playerNo].type != 0) {
            fn_1_204(object, fn_1_AD20);
            return;
        }
        fn_1_204(object, fn_1_A858);
        return;
    }
}

extern void fn_1_C140(OMOBJ *object);

void fn_1_BD94(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;

    fn_1_7160(work, 5, 8.0f, 0, 1);
    if (fn_80071A8C() == 6) {
        fn_1_204(object, fn_1_BFCC);
        return;
    }

    fn_1_780C(work);

    if (fn_1_401C(work->routePlayerNo, work->routeIndex)->phase == 6) {
        work->setupMode = 0;
        if (lbl_80265728[work->playerNo].type != 0) {
            fn_1_204(object, fn_1_AD20);
            return;
        }
        fn_1_204(object, fn_1_A858);
        return;
    }
}

extern void fn_1_C37C(OMOBJ *object);

void fn_1_BFCC(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    HuVecF position;
    f32 scale;

    if (work->setupMode == 1) {
        fn_8002D86C(
            work->mainObject->mdlId[0], 0.0f);
    }
    position = work->position;
    if (work->setupMode == 1) {
        scale = 1.0f -
            (work->position.y / 1500.0f);
        position.y = 0.0f;
        fn_800268B0(work->shadowModelId, scale,
            1.0f, scale);
    } else {
        fn_800268B0(work->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(work->shadowModelId, &position);
    if (fn_80071A8C() == 7) {
        if (work->motionState++ > 0x3C || lbl_1_bss_158 == -1) {
            fn_800269E8(work->displayModelId, 1);
            fn_1_204(object, fn_1_C140);
            return;
        }
    }
}

void fn_1_C140(OMOBJ *object)
{
    M645CharacterSetupWork *work = object->data;
    HuVecF modelPosition;
    HuVecF workPosition;
    HuVecF restoredPosition;
    f32 workScale;
    f32 restoredScale;

    fn_800267CC(work->mainObject->mdlId[0], &modelPosition);
    modelPosition.z = 100.0f;
    fn_80026790(work->mainObject->mdlId[0], &modelPosition);
    work->position = modelPosition;

    workPosition = work->position;
    if (work->setupMode == 1) {
        workScale = 1.0f -
            (work->position.y / 1500.0f);
        workPosition.y = 0.0f;
        fn_800268B0(work->shadowModelId, workScale,
            1.0f, workScale);
    } else {
        fn_800268B0(work->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(work->shadowModelId, &workPosition);

    if (fn_80071A8C() == 8) {
        fn_80026A44(work->mainObject->mdlId[0], 0x40000002);
        restoredPosition = work->position;
        if (work->setupMode == 1) {
            restoredScale = 1.0f -
                (work->position.y / 1500.0f);
            restoredPosition.y = 0.0f;
            fn_800268B0(work->shadowModelId, restoredScale,
                1.0f, restoredScale);
        } else {
            fn_800268B0(work->shadowModelId, 1.0f,
                1.0f, 1.0f);
        }
        fn_80026790(work->shadowModelId, &restoredPosition);
        fn_1_204(object, fn_1_C37C);
        return;
    }
}

void fn_1_C37C(OMOBJ *object)
{
    HuVecF position;
    f32 scale;
    M645CharacterSetupWork *work = object->data;

    if (lbl_1_bss_158 == work->routePlayerNo) {
        fn_1_7160(work, 6, 8.0f, 0, 0);
    } else if (work->setupMode != 1) {
        fn_1_7160(work, 7, 8.0f, 0, 0);
        fn_8002D86C(work->mainObject->mdlId[0],
            1.0f);
    }
    position = work->position;
    if (work->setupMode == 1) {
        scale = 1.0f - (work->position.y / 1500.0f);
        position.y = 0.0f;
        fn_800268B0(work->shadowModelId, scale,
            1.0f, scale);
    } else {
        fn_800268B0(work->shadowModelId, 1.0f,
            1.0f, 1.0f);
    }
    fn_80026790(work->shadowModelId, &position);
}

char *lbl_1_data_58[2] = { "no646madofuki-PC01splay", "no646madofuki-PC02mop" };

s32 lbl_1_data_60[2] = { 0x680008, 0x680009 };

s32 lbl_1_data_68[2][2] = { { 0x680013, 0x680011 }, { 0x680014, 0x680012 } };

u32 lbl_1_data_78[14] = { 0x930000, 0x930001, 0x930002, 0x8E0068, 0x8E00AC, 0x8E00AA, 0x930006, 0x930007, 0x930018, 0x8E0011, 0x8E00AB, 0x8E00A9, 0x930000, 0 };

s32 lbl_1_data_B0[4] = { 0x680000, 0x680001, 0x680002, 0x680003 };

s32 lbl_1_data_C0[2] = { 0x68000A, 0x68000B };

HuVecF lbl_1_data_C8[2] = { { -4.275f, 0.0f, 0.0f }, { 4.275f, 0.0f, 0.0f } };

s32 lbl_1_data_E0[4] = { 0x68000D, 0x68000E, 0x68000F, 0x680010 };

HuVecF lbl_1_data_F0[29] = { { -150.0f, 0.0f, 0.0f }, { 150.0f, 0.0f, 0.0f }, { -150.0f, 0.0f, 0.0f }, { 150.0f, 0.0f, 0.0f }, { -427.5f, 1400.0f, 80.0f }, { -345.0f, 1400.0f, 80.0f }, { -80.0f, 1400.0f, 80.0f }, { 185.0f, 1400.0f, 80.0f }, { 427.5f, 1400.0f, 80.0f }, { -427.5f, 1100.0f, 80.0f }, { -345.0f, 1100.0f, 80.0f }, { -80.0f, 1100.0f, 80.0f }, { 185.0f, 1100.0f, 80.0f }, { 427.5f, 1100.0f, 80.0f }, { -427.5f, 800.0f, 80.0f }, { -345.0f, 800.0f, 80.0f }, { -80.0f, 800.0f, 80.0f }, { 185.0f, 800.0f, 80.0f }, { 427.5f, 800.0f, 80.0f }, { -427.5f, 500.0f, 80.0f }, { -345.0f, 500.0f, 80.0f }, { -80.0f, 500.0f, 80.0f }, { 185.0f, 500.0f, 80.0f }, { 427.5f, 500.0f, 80.0f }, { -427.5f, 200.0f, 80.0f }, { -345.0f, 200.0f, 80.0f }, { -80.0f, 200.0f, 80.0f }, { 185.0f, 200.0f, 80.0f }, { 427.5f, 200.0f, 80.0f } };

HuVecF lbl_1_data_24C[25] = { { -427.5f, 1305.0f, -80.0f }, { -265.0f, 1305.0f, -80.0f }, { 0.0f, 1305.0f, -80.0f }, { 265.0f, 1305.0f, -80.0f }, { 427.5f, 1305.0f, -80.0f }, { -427.5f, 1005.0f, -80.0f }, { -265.0f, 1005.0f, -80.0f }, { 0.0f, 1005.0f, -80.0f }, { 265.0f, 1005.0f, -80.0f }, { 427.5f, 1005.0f, -80.0f }, { -427.5f, 705.0f, -80.0f }, { -265.0f, 705.0f, -80.0f }, { 0.0f, 705.0f, -80.0f }, { 265.0f, 705.0f, -80.0f }, { 427.5f, 705.0f, -80.0f }, { -427.5f, 405.0f, -80.0f }, { -265.0f, 405.0f, -80.0f }, { 0.0f, 405.0f, -80.0f }, { 265.0f, 405.0f, -80.0f }, { 427.5f, 405.0f, -80.0f }, { -427.5f, 105.0f, -80.0f }, { -265.0f, 105.0f, -80.0f }, { 0.0f, 105.0f, -80.0f }, { 265.0f, 105.0f, -80.0f }, { 427.5f, 105.0f, -80.0f } };

HuVecF lbl_1_data_378[5][2] = { { { -132.0f, 1200.0f, 0.0f }, { 132.0f, 1200.0f, 0.0f } }, { { -132.0f, 900.0f, 0.0f }, { 132.0f, 900.0f, 0.0f } }, { { -132.0f, 600.0f, 0.0f }, { 132.0f, 600.0f, 0.0f } }, { { -132.0f, 300.0f, 0.0f }, { 132.0f, 300.0f, 0.0f } }, { { -132.0f, 0.0f, 0.0f }, { 132.0f, 0.0f, 0.0f } } };

HuVecF lbl_1_data_3F0[12] = { { -265.0f, 1180.0f, -150.0f }, { 0.0f, 1180.0f, -150.0f }, { 265.0f, 1180.0f, -150.0f }, { -265.0f, 880.0f, -150.0f }, { 0.0f, 880.0f, -150.0f }, { 265.0f, 880.0f, -150.0f }, { -265.0f, 580.0f, -150.0f }, { 0.0f, 580.0f, -150.0f }, { 265.0f, 580.0f, -150.0f }, { -265.0f, 280.0f, -150.0f }, { 0.0f, 280.0f, -150.0f }, { 265.0f, 280.0f, -150.0f } };

s32 lbl_1_data_480[2] = { 0x68001D, 0x680020 };

s32 lbl_1_data_488[2][2] = { { 0x68001E, 0x68001F }, { 0x680021, 0x680022 } };

s32 lbl_1_data_498[12] = { 0x68001B, 0x680019, 0x680018, 0x680019, 0x68001B, 0x68001A, 0x68001A, 0x680018, 0x68001B, 0x680018, 0x680019, 0x68001A };

HuVecF lbl_1_data_4C8[2] = { { 265.0f, 280.0f, 1926.0f }, { 0.0f, 674.0f, 2880.0f } };

HuVecF lbl_1_data_4E0[2] = { { 0.0f, 0.0f, 0.0f }, { 0.01606052f, 0.0f, 0.0f } };

s8 lbl_1_data_4F8[0x6] = "| @@|";

s8 lbl_1_data_4FE[0x6] = "|@  |";

s8 lbl_1_data_504[0x6] = "|@ @|";

s8 lbl_1_data_50A[0x6] = "| @ |";

s8 lbl_1_data_510[0x8] = "-----";

const s8 *lbl_1_data_518[4][5] = { { lbl_1_data_4F8, lbl_1_data_4FE, lbl_1_data_504, lbl_1_data_50A, lbl_1_data_510 }, { lbl_1_data_4FE, lbl_1_data_4F8, lbl_1_data_50A, lbl_1_data_504, lbl_1_data_510 }, { lbl_1_data_50A, lbl_1_data_504, lbl_1_data_4F8, lbl_1_data_4FE, lbl_1_data_510 }, { lbl_1_data_504, lbl_1_data_50A, lbl_1_data_4FE, lbl_1_data_4F8, lbl_1_data_510 } };

s8 lbl_1_data_568[0x3] = { 0x20, 0x20, 0x00 };

s8 lbl_1_data_56B[0x3] = { 0x21, 0x20, 0x00 };

s8 lbl_1_data_56E[0x3] = { 0x20, 0x21, 0x00 };

s8 lbl_1_data_571[0x3] = { 0x2D, 0x2D, 0x00 };

M645StageCharacterData lbl_1_data_574[4] = { { { lbl_1_data_568, lbl_1_data_56B, lbl_1_data_56E, lbl_1_data_56E, lbl_1_data_571 } }, { { lbl_1_data_56E, lbl_1_data_568, lbl_1_data_56B, lbl_1_data_56E, lbl_1_data_571 } }, { { lbl_1_data_56B, lbl_1_data_56E, lbl_1_data_568, lbl_1_data_56B, lbl_1_data_571 } }, { { lbl_1_data_56E, lbl_1_data_56B, lbl_1_data_56E, lbl_1_data_568, lbl_1_data_571 } } };

typedef M645RouteEntry M645PathNode;

typedef struct M645PathSearchContext M645PathSearchContext;

extern s32 fn_1_C594(M645PathSearchContext *context, void *target, M645PathNode *current, s32 depth, s32 *limit, M645PathNode **result, M645PathNode **visitedIds);

struct M645PathSearchContext {
    u8 beforePlayerNo[0x0E];
    s16 playerNo;
    u8 beforeScoringPlayerNo[0x24];
    s16 scoringPlayerNo;
};

typedef M645CharacterSetupWork M645RouteSetupState;

typedef M645CharacterSetupWork M645PathController;

extern void fn_1_DE48(void *context);

extern s32 fn_1_C8D8(M645CharacterSetupWork *context, M645PathNode *target, M645PathNode *current, s32 depth, s32 *limit, M645PathNode **result, M645PathNode **visited);

extern void fn_1_D8B0(HuVecF *movement, M645RouteEntry *from, M645RouteEntry *to, M645CharacterSetupWork *playerWork);

extern void fn_1_80CC(M645CharacterSetupWork *work, HuVecF *movement);

static inline s32 M645RouteIndexGet(s32 groupIndex)
{
    M645CharacterSetupWork *state = fn_1_29B4(groupIndex);
    s32 tableIndex = state->pathState;
    s32 result;

    if (state->setupMode == 1) {
        result = state->sideIndex == 0 ? 0 : 4;
    } else {
        result = (lbl_1_data_378[tableIndex][0].x >
            state->position.x) ? 1 :
            ((lbl_1_data_378[tableIndex][1].x <
                state->position.x) ? 3 : 2);
    }
    return result + tableIndex * 5;
}

static inline M645RouteEntry *M645RouteGet(s32 index)
{
    return &lbl_1_bss_C[index];
}

static inline M645PathNode *M645RouteSearch(M645CharacterSetupWork *controller,
    M645PathNode *target, M645PathNode *current, s32 *statusResult)
{
    M645PathNode *visited[16];
    s32 startIndex;
    s32 limit;
    M645PathNode *searchResult;
    M645PathNode *result;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    searchStatus = fn_1_C8D8(controller, target, current, startIndex,
        &limit, &searchResult, visited);
    if (searchStatus < 999) {
        result = searchResult;
        *statusResult = searchStatus;
    }
    return result;
}

s32 fn_1_EB84(M645CharacterSetupWork *controller);

M645PathNode *fn_1_C854(M645PathSearchContext *context, void *target, M645PathNode *current);

M645PathNode *fn_1_CB18(M645CharacterSetupWork *context, M645PathNode *target, M645PathNode *current, s32 *statusResult);

typedef M645RouteObject M645PathStateSlot;

/* Checks whether a route step reaches a currently open neighboring tile. */
static inline s32 fn_1_2984(s32 index) { return lbl_1_bss_138[index]; }

s32 fn_1_CBAC(M645CharacterSetupWork *context, M645RouteEntry *route);

s32 fn_1_D5C4(M645CharacterSetupWork *context, M645PathNode *current, s32 depth, s32 *limit, M645PathNode **result, M645PathNode **visited);

M645RouteEntry *fn_1_D820(M645CharacterSetupWork *controller, M645RouteEntry *route);

extern s32 fn_1_E320(M645PathController *controller);

extern s32 fn_1_F1BC(M645RouteSetupState *state);

s32 fn_1_FC04(M645CharacterSetupWork *object);

/* Selects the shared movement range when two players occupy the same route slot. */
static inline M645CharacterSetupWork *fn_1_29B4_for_DE48(s32 index)
{
    return lbl_1_bss_160[index];
}

/* Advances route selection and clears stale routes for the active players. */
static inline M645RouteEntry *fn_1_D820_for_E320(void *controller, M645RouteEntry *route)
{
    M645PathNode *work[16];
    M645RouteEntry *result;
    s32 startIndex;
    s32 limit;
    M645PathNode *searchResult;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    if (fn_1_CBAC(controller, route) != 0) {
        return route;
    }
    searchStatus = fn_1_D5C4(controller, route, startIndex,
        &limit, &searchResult, work);
    if (searchStatus < 999) {
        result = searchResult;
    }
    return result;
}

static inline M645PathNode *M645PathSearchForE320(void *first,
    M645PathNode *second, M645PathNode *third)
{
    M645PathNode *visitedIds[16];
    M645PathNode *result;
    s32 startIndex;
    s32 limit;
    M645PathNode *searchResult;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    searchStatus = fn_1_C594(first, second, third, startIndex,
        &limit, &searchResult, visitedIds);
    if (searchStatus < 999) {
        result = searchResult;
    }
    return result;
}

static inline s32 M645RouteKeyForE320(s32 groupIndex)
{
    M645CharacterSetupWork *state = fn_1_29B4(groupIndex);
    s32 tableIndex = state->pathState;
    s32 result;

    if (state->setupMode == 1) {
        result = state->sideIndex == 0 ? 0 : 4;
    } else {
        result = (lbl_1_data_378[tableIndex][0].x >
            state->position.x) ? 1 :
            ((lbl_1_data_378[tableIndex][1].x <
                state->position.x) ? 3 : 2);
    }
    return result + tableIndex * 5;
}

/* Chooses the next route for the active player and updates movement direction. */
static inline s32 fn_1_7088_for_F1BC(s32 groupIndex)
{
    M645CharacterSetupWork *state = fn_1_29B4(groupIndex);
    s32 tableIndex = state->pathState;
    s32 result;
    s32 direction;
    s32 phase;
    s32 outcome;

    if (state->setupMode == 1) {
        if (state->sideIndex == 0) {
            direction = 0;
        } else {
            direction = 4;
        }
        result = direction;
    } else {
        if (lbl_1_data_378[tableIndex][0].x >
            state->position.x) {
            phase = 1;
        } else {
            if (lbl_1_data_378[tableIndex][1].x <
                state->position.x) {
                outcome = 3;
            } else {
                outcome = 2;
            }
            phase = outcome;
        }
        result = phase;
    }
    return result + tableIndex * 5;
}

static inline M645PathNode *M645RouteSearchForF1BC(M645CharacterSetupWork *controller,
    M645PathNode *target, M645PathNode *current, s32 *statusResult)
{
    M645PathNode *visited[16];
    M645PathNode *result;
    s32 startIndex;
    s32 limit;
    M645PathNode *searchResult;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    searchStatus = fn_1_C8D8(controller, target, current, startIndex,
        &limit, &searchResult, visited);
    if (searchStatus < 999) {
        result = searchResult;
        *statusResult = searchStatus;
    }
    return result;
}

s32 fn_1_C594(M645PathSearchContext *context, void *target, M645PathNode *current, s32 depth,
    s32 *limit, M645PathNode **result, M645PathNode **visitedIds)
{
    s32 bestScore = 0x3E7;
    M645PathNode *bestNode = current;
    M645PathNode *childResult;
    s32 visitedIndex;
    M645PathNode *child;
    s32 childScore;
    s32 character;
    s32 scoreAdjustment;
    s32 sideAdjustment;
    s16 playerNo;
    s16 nodeIndex;

    *result = NULL;
    for (visitedIndex = 0; visitedIndex < depth; visitedIndex++) {
        if (visitedIds[visitedIndex] == current) {
            return 0x3E7;
        }
    }
    visitedIds[depth++] = current;
    if (depth >= 16) {
        return 0x3E7;
    }
    if (*limit < depth) {
        return 0x3E7;
    }
    if (depth > 1) {
        s32 expectedScore;
        s32 playerNo;
        M645CharacterSetupWork *player;

        playerNo = context->scoringPlayerNo;
        player = fn_1_29B4(playerNo);
        character = player->pathState;
        if (player->setupMode == 1) {
            if (player->sideIndex == 0) {
                sideAdjustment = 0;
            } else {
                sideAdjustment = 4;
            }
            scoreAdjustment = sideAdjustment;
        } else {
            scoreAdjustment =
                (lbl_1_data_378[character][0].x >
                    player->position.x) ? 1 :
                ((lbl_1_data_378[character][1].x <
                    player->position.x) ? 3 : 2);
        }
        expectedScore = scoreAdjustment + character * 5;
        if (current->pathIndex == expectedScore) {
            return 0x3E7;
        }
    }
    if (current->pathIndex <= 0x13) {
        nodeIndex = current->pathIndex;
        playerNo = context->playerNo;
        if (target == (void *)(fn_1_401C(
                playerNo, nodeIndex))->phase &&
            depth < bestScore) {
            bestScore = depth;
            bestNode = current;
            lbl_1_bss_8 = bestNode;
        }
    }
    for (visitedIndex = 0; visitedIndex < current->childCount; visitedIndex++) {
        child = current->children[visitedIndex];
        childScore = fn_1_C594(context, target, child,
            depth, limit, &childResult, visitedIds);

        if (childScore < bestScore) {
            bestScore = childScore;
            bestNode = child;
            lbl_1_bss_8 = bestNode;
        }
    }
    *result = bestNode;
    if (bestScore == 0x3E7) {
        return 0x3E7;
    }
    if (*limit > bestScore) {
        *limit = bestScore;
    }
    return bestScore;
}

M645PathNode *fn_1_C854(M645PathSearchContext *context, void *target, M645PathNode *current)
{
    M645PathNode *visitedIds[16];
    s32 limit;
    M645PathNode *searchResult;
    s32 startIndex;
    s32 searchStatus;
    M645PathNode *result;

    result = 0;
    startIndex = 0;
    limit = 100;
    searchStatus = fn_1_C594(context, target, current, startIndex,
            &limit, &searchResult, visitedIds);
    if (searchStatus < 999) {
        result = searchResult;
    }
    return result;
}

s32 fn_1_C8D8(M645CharacterSetupWork *context, M645PathNode *target, M645PathNode *current, s32 depth,
    s32 *limit, M645PathNode **result, M645PathNode **visited)
{
    s32 bestScore = 0x3E7;
    M645PathNode *bestNode = current;
    M645PathNode *childResult;
    s32 visitedIndex;
    s32 character;
    M645PathNode *child;
    s32 childScore;
    s32 scoreAdjustment;
    s32 sideAdjustment;
    s32 positionAdjustment;

    *result = NULL;
    for (visitedIndex = 0; visitedIndex < depth; visitedIndex++) {
        if (visited[visitedIndex] == current) {
            return 0x3E7;
        }
    }
    visited[depth++] = current;
    if (depth >= 16) {
        return 0x3E7;
    }
    if (*limit < depth) {
        return 0x3E7;
    }
    if (depth > 1) {
        s32 expectedScore;
        s32 playerNo;
        M645CharacterSetupWork *player;

        playerNo = context->pairedPlayerNo;
        player = fn_1_29B4(playerNo);

        character = player->pathState;

        if (player->setupMode == 1) {
            if (player->sideIndex == 0) {
                sideAdjustment = 0;
            } else {
                sideAdjustment = 4;
            }
            scoreAdjustment = sideAdjustment;
        } else {
            if (lbl_1_data_378[character][0].x >
                player->position.x) {
                positionAdjustment = 1;
            } else {
                positionAdjustment =
                    (lbl_1_data_378[character][1].x <
                    player->position.x) ? 3 : 2;
            }
            scoreAdjustment = positionAdjustment;
        }
        expectedScore = scoreAdjustment + character * 5;
        if (current->pathIndex == expectedScore) {
            return 0x3E7;
        }
    }
    if (current == target && depth < bestScore) {
        bestScore = depth;
        bestNode = current;
    }
    for (visitedIndex = 0; visitedIndex < current->childCount; visitedIndex++) {
        child = current->children[visitedIndex];
        childScore = fn_1_C8D8(context, target, child, depth, limit,
            &childResult, visited);

        if (childScore < bestScore) {
            bestScore = childScore;
            bestNode = child;
        }
    }
    *result = bestNode;
    if (bestScore == 0x3E7) {
        return 0x3E7;
    }
    if (*limit > bestScore) {
        *limit = bestScore;
    }
    return bestScore;
}

M645PathNode *fn_1_CB18(M645CharacterSetupWork *context, M645PathNode *target,
    M645PathNode *current, s32 *statusResult)
{
    M645PathNode *visitedIds[16];
    s32 limit;
    M645PathNode *searchResult;
    s32 startIndex;
    M645PathNode *result;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    searchStatus = fn_1_C8D8(context, target, current, startIndex,
            &limit, &searchResult, visitedIds);
    if (searchStatus < 999) {
        result = searchResult;
        *statusResult = searchStatus;
    }
    return result;
}

s32 fn_1_CBAC(M645CharacterSetupWork *context, M645RouteEntry *route)
{
    if (fn_1_2984(route->pathIndex / 5) == 1 && route->pathIndex % 5 == 3) {
        if (fn_1_401C(context->routePlayerNo, route->pathIndex - 1)->phase == 6 &&
            fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6) return 1;
    }
    if (fn_1_2984(route->pathIndex / 5) == 2 && route->pathIndex % 5 == 1) {
        if (fn_1_401C(context->routePlayerNo, route->pathIndex + 1)->phase == 6 &&
            fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6) return 1;
    }
    if ((fn_1_2984(route->pathIndex / 5) == 1 || fn_1_2984(route->pathIndex / 5) == 2) && route->pathIndex % 5 == 2) {
        if (fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6) return 1;
    }
    if ((route->pathIndex >= 0x15 && route->pathIndex <= 0x17) || (route->pathIndex % 5 >= 1 && route->pathIndex % 5 <= 3)) {
        if (route->pathIndex / 5 != M645RouteIndexGet(context->pairedPlayerNo) / 5) {
            if (fn_1_2984(route->pathIndex / 5) == 0 && (route->pathIndex >= 0x14 || fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6)) return 1;
        }
    }
    if (route->pathIndex / 5 != M645RouteIndexGet(context->pairedPlayerNo) / 5) {
        if (fn_1_2984(route->pathIndex / 5) == 1 && route->pathIndex % 5 == 1 && fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6) return 1;
    }
    if (route->pathIndex / 5 != M645RouteIndexGet(context->pairedPlayerNo) / 5) {
        if (fn_1_2984(route->pathIndex / 5) == 2 && route->pathIndex % 5 == 3 && fn_1_401C(context->routePlayerNo, route->pathIndex)->phase == 6) return 1;
    }
    return 0;
}

s32 fn_1_D5C4(M645CharacterSetupWork *context, M645PathNode *current, s32 depth,
    s32 *limit, M645PathNode **result, M645PathNode **visited)
{
    s32 bestScore = 0x3E7;
    M645PathNode *bestNode = current;
    M645PathNode *childResult;
    s32 visitedIndex;
    s32 character;
    M645PathNode *child;
    s32 childScore;
    s32 scoreAdjustment;
    s32 sideAdjustment;

    *result = 0;
    for (visitedIndex = 0; visitedIndex < depth; visitedIndex++) {
        if (visited[visitedIndex] == current) {
            return 0x3E7;
        }
    }
    visited[depth++] = current;
    if (depth >= 16) {
        return 0x3E7;
    }
    if (*limit < depth) {
        return 0x3E7;
    }
    if (depth > 1) {
        s32 expectedScore;
        s32 playerNo = context->pairedPlayerNo;
        M645CharacterSetupWork *player = fn_1_29B4(playerNo);
        character = player->pathState;

        if (player->setupMode == 1) {
            if (player->sideIndex == 0) {
                sideAdjustment = 0;
            } else {
                sideAdjustment = 4;
            }
            scoreAdjustment = sideAdjustment;
        } else {
            scoreAdjustment =
                (lbl_1_data_378[character][0].x >
                    player->position.x) ? 1 :
                ((lbl_1_data_378[character][1].x <
                    player->position.x) ? 3 : 2);
        }
        expectedScore = scoreAdjustment + character * 5;
        if (current->pathIndex == expectedScore) {
            return 0x3E7;
        }
    }
    if (fn_1_CBAC(context, current) != 0 && depth < bestScore) {
        bestScore = depth;
        bestNode = current;
        lbl_1_bss_8 = bestNode;
    }
    for (visitedIndex = 0; visitedIndex < current->childCount; visitedIndex++) {
        child = current->children[visitedIndex];
        childScore = fn_1_D5C4(context, child, depth, limit,
            &childResult, visited);

        if (childScore < bestScore) {
            bestScore = childScore;
            bestNode = child;
            lbl_1_bss_8 = bestNode;
        }
    }
    *result = bestNode;
    if (bestScore == 0x3E7) {
        return 0x3E7;
    }
    if (bestScore < *limit) {
        *limit = bestScore;
    }
    return bestScore;
}

M645RouteEntry *fn_1_D820(M645CharacterSetupWork *controller, M645RouteEntry *route)
{
    M645PathNode *work[16];
    s32 limit;
    M645PathNode *searchResult;
    s32 startIndex;
    M645RouteEntry *result;
    s32 searchStatus;

    result = 0;
    startIndex = 0;
    limit = 100;
    if (fn_1_CBAC(controller, route) != 0) {
        return route;
    }
    searchStatus = fn_1_D5C4(controller, route, startIndex,
        &limit, &searchResult, work);
    if (searchStatus < 999) {
        result = searchResult;
    }
    return result;
}

void fn_1_D8B0(HuVecF *movement, M645RouteEntry *from, M645RouteEntry *to,
    M645CharacterSetupWork *playerWork)
{
    if (to != 0) {
        if (playerWork->setupMode == 0) {
            if (from == to) {
                f32 horizontalDistance = fabsf(lbl_1_data_24C[from->pathIndex].x -
                    playerWork->position.x);
                f32 strength;

                strength = (8.0f < horizontalDistance) ? 1.0f
                    : horizontalDistance / 8.0f;
                if (horizontalDistance < 10.0f) {
                    movement->x = 0.0f;
                } else {
                    movement->x = (lbl_1_data_24C[from->pathIndex].x >
                        playerWork->position.x) ? strength : -strength;
                }
            } else if (from->pathIndex / 5 == to->pathIndex / 5) {
                if (from->pathIndex > to->pathIndex) {
                    movement->x = -1.0f;
                } else {
                    movement->x = 1.0f;
                }
            } else if (from->pathIndex % 5 == 1) {
                movement->x = -1.0f;
            } else if (from->pathIndex % 5 == 3) {
                movement->x = 1.0f;
            }
        } else if (playerWork->setupMode == 1) {
            if (from->pathIndex / 5 < to->pathIndex / 5) {
                movement->y = -1.0f;
            } else if (from->pathIndex / 5 > to->pathIndex / 5) {
                movement->y = 1.0f;
            } else {
                f32 strength = fabsf(playerWork->position.y -
                    lbl_1_rodata_A0[playerWork->pathState]) / 8.0f;

                if (strength > 1.0f) {
                    strength = 1.0f;
                }
                if (playerWork->position.y >
                    lbl_1_rodata_A0[playerWork->pathState]) {
                    movement->y = -strength;
                } else if (playerWork->position.y <
                    lbl_1_rodata_A0[playerWork->pathState]) {
                    movement->y = strength;
                }
            }

            if (fabsf(playerWork->position.y -
                    lbl_1_rodata_A0[playerWork->pathState]) <=
                5.0f) {
                if (from->pathIndex % 5 < to->pathIndex % 5) {
                    movement->x = 1.0f;
                } else if (from->pathIndex % 5 > to->pathIndex % 5) {
                    movement->x = -1.0f;
                }
            }
        }
        fn_1_4F0(movement, movement);
    }
}

void fn_1_DE48(void *object)
{
    M645CharacterSetupWork *candidate;
    if (M645RouteIndexGet(((M645RouteSetupState *)object)->playerNo) != M645RouteIndexGet(((M645RouteSetupState *)object)->pairedPlayerNo))
        return;
    candidate = fn_1_29B4(((M645RouteSetupState *)object)->pairedPlayerNo);
    if (((M645RouteSetupState *)object)->setupMode == 1) {
        if (((M645RouteSetupState *)object)->sideIndex == 0) {
            if (((M645RouteSetupState *)object)->position.y > candidate->position.y) {
                ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(0);
            } else {
                ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(20);
            }
        } else if (((M645RouteSetupState *)object)->sideIndex == 1) {
            if (((M645RouteSetupState *)object)->position.y > candidate->position.y) {
                ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(4);
            } else {
                ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(24);
            }
        }
    } else {
        if (((M645RouteSetupState *)object)->position.x > candidate->position.x) {
            ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(M645RouteIndexGet(((M645RouteSetupState *)object)->playerNo) + 1);
        } else {
            ((M645RouteSetupState *)object)->currentRoute = ((M645RouteSetupState *)object)->nextRoute = M645RouteGet(M645RouteIndexGet(((M645RouteSetupState *)object)->playerNo) - 1);
        }
    }
}

const HuVecF lbl_1_rodata_294 = { 0.0f, 0.0f, 0.0f };

s32 fn_1_E320(M645PathController *controller)
{
    s32 index;
    HuVecF movement = lbl_1_rodata_294;
    M645RouteEntry *route;
    M645RouteEntry *selectedRoute;
    M645PathNode *visitedFirst[16];
    M645PathNode *visitedSecond[16];
    M645PathNode *visitedThird[16];
    M645PathNode *visitedFourth[16];
    M645PathNode *visitedFifth[16];
    M645CharacterSetupWork *playerWork;
    s32 emptySlots;

    s32 character;
    s32 lane;
    s32 candidateLane;
    s32 searchLimit;
    s32 searchDepth;
    s32 searchScore;
    s32 returned;
    s32 isAtRoute;
    s16 timer;
    s16 stageIndex;
    s16 candidateCharacter;
    s32 scanIndex;

    isAtRoute = 0;
    emptySlots = 0;
    if (--controller->segmentNo < 0) {
        controller->segmentNo = 0;
    }
    for (scanIndex = 0; scanIndex < 25; scanIndex++) {
        if (fn_1_401C(controller->routePlayerNo, scanIndex)->phase == 0) {
            emptySlots++;
        }
    }

    index = M645RouteKeyForE320(controller->playerNo);
    route = M645RouteGet(index);
    fn_1_DE48(controller);

    if (controller->assetSelection == 0) {
        if (emptySlots == 0) {
            if (controller->currentRoute == 0) {
                controller->currentRoute = fn_1_D820_for_E320(controller, route);
            }
            if (controller->currentRoute != 0) {
                fn_1_D8B0(&movement, route, controller->currentRoute,
                    (M645CharacterSetupWork *)controller);
            }
        } else {
            if (controller->segmentNo == 0) {
                controller->currentRoute = M645PathSearchForE320(controller, 0,
                    route);
                controller->segmentNo = 0x3C;
            }
            if (controller->currentRoute != 0) {
                fn_1_D8B0(&movement, route, controller->currentRoute,
                    (M645CharacterSetupWork *)controller);
                if (controller->currentRoute == route) {
                    isAtRoute = 1;
                }
            } else {
                controller->currentRoute = fn_1_D820_for_E320(controller, route);
                if (controller->currentRoute != 0) {
                    fn_1_D8B0(&movement, route, controller->currentRoute,
                        (M645CharacterSetupWork *)controller);
                }
            }
        }
    } else if (controller->assetSelection == 1) {
        if (controller->segmentNo == 0) {
            controller->currentRoute = M645PathSearchForE320(controller, (M645PathNode *)3,
                route);
            controller->segmentNo = 0x3C;
        }
        if (controller->currentRoute != 0) {
            fn_1_D8B0(&movement, route, controller->currentRoute, (M645CharacterSetupWork *)controller);
            if (route == controller->currentRoute) {
                isAtRoute = 1;
            }
        } else {
            controller->currentRoute = fn_1_D820_for_E320(controller, route);
            if (controller->currentRoute != 0) {
                fn_1_D8B0(&movement, route, controller->currentRoute,
                    (M645CharacterSetupWork *)controller);
            }
        }
    }

    if (controller->setupMode == 0) {
        isAtRoute = fn_1_78F4(controller, &movement, isAtRoute);
    } else if (controller->setupMode == 1) {
        fn_1_80CC(controller, &movement);
    }
    if ((controller->currentRoute == route &&
        fabs(lbl_1_data_24C[route->pathIndex].x -
            controller->position.x) < 10.0) ||
        index == M645RouteKeyForE320(controller->pairedPlayerNo) ||
        controller->collisionFlag != 0) {
        controller->currentRoute = NULL;
        controller->segmentNo = 0;
    }
    if ((controller->nextRoute == route &&
        fabs(lbl_1_data_24C[route->pathIndex].x -
            controller->position.x) < 10.0) ||
        index == M645RouteKeyForE320(controller->pairedPlayerNo) ||
        controller->collisionFlag != 0) {
        controller->nextRoute = NULL;
        controller->currentRoute = NULL;
    }
    return isAtRoute;
}

s32 fn_1_EB84(M645CharacterSetupWork *controller)
{
    HuVecF movement = { 0.0f, 0.0f, 0.0f };
    M645RouteEntry * activeRoute;
    s32 movementMode;
    s32 routeSlot;
    s32 routeIndex;
    s32 randomLane;
    s32 randomCharacter;
    s32 routeDistance;
    s32 returnValue = 0;
    movementMode = 0;
    for (routeSlot = 0; routeSlot < 25; routeSlot++)
    {
        if ((fn_1_401C(controller->routePlayerNo, routeSlot))->phase == 0)
        {
            movementMode++;
        }
    }
    routeIndex = M645RouteIndexGet(controller->playerNo);
    activeRoute = M645RouteGet(routeIndex);
    fn_1_DE48(controller);
    randomLane = (rand8() % 3) + 1;
    randomCharacter = rand8() % 4;
    if (controller->nextRoute == 0)
    {
        controller->nextRoute = M645RouteGet(randomLane + randomCharacter * 5);
    }
    if (controller->nextRoute)
    {
        if (controller->currentRoute == 0)
        {
            controller->currentRoute = M645RouteSearch(controller,
                controller->nextRoute, activeRoute,
                &routeDistance);
        }
        if (controller->currentRoute)
        {
            fn_1_D8B0(& movement, activeRoute, controller->currentRoute, controller);
            if (activeRoute == controller->currentRoute)
            {
                returnValue = 1;
            }
        }
        else
        {
            controller->nextRoute = 0;
        }
    }
    else
    {
        controller->currentRoute = 0;
    }
    if (controller->setupMode == 0)
    {
        returnValue = fn_1_78F4(controller, &movement, returnValue);
    }
    else
    {
        if (controller->setupMode == 1)
        {
            fn_1_80CC(controller, & movement);
        }
    }
    if ((controller->currentRoute == activeRoute &&
            fabs(lbl_1_data_24C[activeRoute->pathIndex].x - controller->position.x) < 10.0) ||
        routeIndex == M645RouteIndexGet(controller->pairedPlayerNo) || controller->collisionFlag != 0)
    {
        controller->currentRoute = 0;
    }
    if ((controller->nextRoute == activeRoute &&
            fabs(lbl_1_data_24C[activeRoute->pathIndex].x - controller->position.x) < 10.0) ||
        routeIndex == M645RouteIndexGet(controller->pairedPlayerNo) || controller->collisionFlag != 0)
    {
        controller->nextRoute = 0;
        controller->currentRoute = 0;
    }
    return returnValue;
}

s32 fn_1_F1BC(M645RouteSetupState *state)
{
    M645RouteEntry * currentRoute;
    s32 specialRoutes[25];
    s32 eligibleRoutes[25];
    HuVecF movement = { 0.0f, 0.0f, 0.0f };
    s32 routeIndex;
    s32 candidateCount;
    s32 specialCount;
    s32 currentIndex;
    s32 normalDistance;
    s32 specialDistance;
    s32 isAtRoute;
    isAtRoute = 0;
    currentIndex = M645RouteIndexGet(state->playerNo);
    currentRoute = M645RouteGet(currentIndex);
    fn_1_DE48(state);
    if (state->assetSelection == 0)
    {
        candidateCount = 0;
        for (routeIndex = 0; 25 > routeIndex; routeIndex += 1)
        {
            if ((fn_1_401C(state->routePlayerNo,
                routeIndex))->phase == 0)
            {
                specialRoutes[candidateCount] = routeIndex;
                candidateCount++;
            }
        }
        if (candidateCount == 0)
        {
            isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
            return isAtRoute;
        }
        if (state->nextRoute == 0)
        {
            state->nextRoute = M645RouteGet(specialRoutes[rand8() % candidateCount]);
        }
        if (state->nextRoute != 0)
        {
            if (state->currentRoute == 0)
            {
                state->currentRoute = M645RouteSearchForF1BC(
                    (M645CharacterSetupWork *)state, state->nextRoute,
                    currentRoute, &normalDistance);
            }
            if (state->currentRoute != 0)
            {
                fn_1_D8B0(& movement, currentRoute, state->currentRoute, (M645CharacterSetupWork *)state);
                if (currentRoute == state->currentRoute)
                {
                    isAtRoute = 1;
                }
                goto routeChosen;
            }
            state->nextRoute = NULL;
            isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
            return isAtRoute;
        }
        isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
        return isAtRoute;
    }
    if (state->assetSelection == 1)
    {
        specialCount = 0;
        for (routeIndex = 0; routeIndex < 25; routeIndex++)
        {
            if ((fn_1_401C(state->routePlayerNo,
                routeIndex))->phase == 3)
            {
                eligibleRoutes[specialCount] = routeIndex;
                specialCount++;
            }
        }
        if (specialCount == 0)
        {
            isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
            return isAtRoute;
        }
        if (state->nextRoute == 0)
        {
            state->nextRoute = M645RouteGet(eligibleRoutes[rand8() % specialCount]);
        }
        if (state->nextRoute != 0)
        {
            if (state->currentRoute == 0)
            {
                state->currentRoute = M645RouteSearchForF1BC(
                    (M645CharacterSetupWork *)state, state->nextRoute,
                    currentRoute, &specialDistance);
            }
            if (state->currentRoute != 0)
            {
                fn_1_D8B0(& movement, currentRoute, state->currentRoute, (M645CharacterSetupWork *)state);
                if (currentRoute == state->currentRoute)
                {
                    isAtRoute = 1;
                }
                goto routeChosen;
            }
            state->nextRoute = NULL;
            isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
            return isAtRoute;
        }
        isAtRoute = fn_1_EB84((M645CharacterSetupWork *)state);
        return isAtRoute;
    }
routeChosen:
    if (state->setupMode == 0) {
        isAtRoute = fn_1_78F4(state, &movement, isAtRoute);
    } else if (state->setupMode == 1) {
        fn_1_80CC(state, &movement);
    }
    if ((state->currentRoute == currentRoute &&
        fabs(lbl_1_data_24C[currentRoute->pathIndex].x - state->position.x) < 10.0) ||
        currentIndex == M645RouteIndexGet(state->pairedPlayerNo) || state->collisionFlag != 0) {
        if (rand8() % 2 == 0) {
            state->nextRoute = NULL;
        }
        state->currentRoute = NULL;
    }
    if ((state->nextRoute == currentRoute &&
        fabs(lbl_1_data_24C[currentRoute->pathIndex].x - state->position.x) < 10.0) ||
        currentIndex == M645RouteIndexGet(state->pairedPlayerNo)) {
        state->nextRoute = NULL;
    }
    return isAtRoute;
}

s32 fn_1_FA1C(M645CharacterSetupWork *controller)
{
    s32 durations[2] = { 0xB4, 0xB4 };
    s32 result;

    if (controller->setupMode == 0 || controller->setupMode == 1) {
        controller->setupState += 1;
    }
    if (controller->setupState > durations[controller->randomAngle]) {
        controller->setupState = 0;
        controller->randomAngle = (s16)((controller->randomAngle + 1) % 2);
    }
    if (controller->randomAngle == 0) {
        result = fn_1_E320(controller);
    } else {
        result = fn_1_F1BC(controller);
    }
    return result;
}

s32 fn_1_FAF8(M645CharacterSetupWork *controller)
{
    s32 durations[3] = { 0xB4, 0x168, 0x3C };
    s32 result;

    if (controller->setupMode == 0 || controller->setupMode == 1) {
        controller->setupState += 1;
    }
    if (controller->setupState > durations[controller->randomAngle]) {
        controller->setupState = 0;
        controller->randomAngle = (s16)((controller->randomAngle + 1) % 3);
    }
    if (controller->randomAngle == 0) {
        result = fn_1_F1BC(controller);
    } else if (controller->randomAngle == 1) {
        result = fn_1_E320(controller);
    } else {
        result = fn_1_EB84(controller);
    }
    return result;
}

s32 fn_1_FC04(M645CharacterSetupWork *object)
{
    s32 durations[2] = { 0xB4, 0xB4 };
    s32 result;

    if (object->setupMode == 0 || object->setupMode == 1) {
        object->setupState += 1;
    }
    if (object->setupState > durations[object->randomAngle]) {
        object->setupState = 0;
        object->randomAngle = (s16)((object->randomAngle + 1) % 2);
    }
    if (object->randomAngle == 0) {
        result = fn_1_F1BC((void *)object);
    } else {
        result = fn_1_EB84(object);
    }
    return result;
}
