/* Sets up the minigame scene and drives its phases, props, and camera. */
#include "math.h"
#include "REL/m631Dll/utilities.h"
#include "dolphin/gx.h"
#include "game/main.h"
#include "game/gamework.h"
#include "game/gamemes.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/flag.h"
#include "game/process.h"
#include "string.h"
#include "datadir_enum.h"
#include "msm_stream.h"

/* Sound entries used by this minigame. */
#define MSM_SE_M631_01 1839
#define MSM_SE_M631_02 1840
#define MSM_SE_M631_03 1841
#define MSM_SE_M631_04 1842
#define MSM_SE_M631_05 1843
#define MSM_SE_M631_06 1844

typedef struct M631LightColor {
    u8 rgba[4]; /* Red, green, blue and alpha channels, 0 to 255. */
} M631LightColor;

typedef struct M631ActorDefaults {
    s32 lanePairs[6][2]; /* Allowed pairs of the five conveyor lanes. */
} M631ActorDefaults;

typedef struct M631CameraPoints {
    Point3d points[2]; /* Gameplay and results camera vectors, in that order. */
} M631CameraPoints;

typedef struct M631SearchIndexDefaults {
    s32 actorIndexes[3]; /* Group-one candidates, ordered with their distances. */
} M631SearchIndexDefaults;

typedef void (*VoidFunc)(void);

typedef struct M631VerticalPositions {
    f32 heights[3]; /* Effect heights for ground, middle and upper contact. */
} M631VerticalPositions;

HUPROCESS *lbl_1_bss_C; /* Object-manager process for this minigame. */
OMOBJ *lbl_1_bss_8; /* Scene object; its data owns the shared minigame state. */
s32 lbl_1_bss_4; /* Music channel handle used for the finish fade. */
s32 lbl_1_bss_0; /* Frames elapsed while waiting to announce the winning team. */

extern void CharMotionShiftSet(s16, s16, f32, f32, u32);

extern s16 Hu3DMotionIDGet(s16);

extern s16 Hu3DMotionShiftIDGet(s16);

extern void Hu3DMotionTimeSet(s16, f32);

extern s32 frandmod(s32 max);

extern const VoidFunc _ctors[];

extern const VoidFunc _dtors[];

extern s16 GwMgNightF;

extern void fn_1_A8F0(M631ActorWork *);
extern void fn_1_AA78(M631ActorWork *);
extern M631MotionPairNode *fn_1_B004(void);
extern void fn_1_AF94(M631ActorCallbackWork *);
extern void fn_1_AFF8(void *, f32, s32);
extern void fn_1_AF2C(void *, u32, u32, u32, u32, const HuVecF *);
extern void fn_1_5284(OMOBJ *);
extern void *fn_1_B0B8(s32 (*)(void *), void *);
extern void fn_1_AB4C(void);
extern void fn_1_AB60(void);
extern void fn_1_B014(void);
extern void fn_1_B1AC(void);
extern void fn_1_5AAC(M631ActorWork *, s32, s32);

/* Camera-selection masks, one bit per camera. */
const u16 lbl_1_rodata_10[4] = {1, 2, 4, 8};

int _prolog(void);
void _epilog(void);
void fn_1_A0(void);
void fn_1_1F8(s16 mode, s16 time);
void fn_1_248(s16 mode, s16 time);
void fn_1_4FC(s16 mode, s16 time);
void fn_1_560(s16 mode, s16 time);
void fn_1_644(s16 mode, s16 time);
void fn_1_990(s16 mode, s16 time);
void fn_1_10A0(s16 mode, s16 time);
void fn_1_1214(s16 mode, s16 time);
void fn_1_1218(s16 mode, s16 time);
void fn_1_121C(M631SceneWork *scene);
void fn_1_1374(M631SceneWork *scene);
void fn_1_14D0(M631SceneWork *scene);
void fn_1_162C(M631SceneWork *scene);
void fn_1_174C(OMOBJ *obj);
void fn_1_17C8(void);
void fn_1_1830(M631SceneWork *scene);
void fn_1_1F78(M631SceneWork *scene);
void fn_1_228C(HU3D_MODELID *models);
f32 fn_1_2528(void);
void fn_1_2588(M631ActionQueue *queue);
void fn_1_264C(M631ActionNode *work);
M631ActionNode *fn_1_2798(M631ActionQueue *owner);
void fn_1_27E8(M631ActionNode *node);
void fn_1_2868(void *callbackData);
s32 fn_1_2A84(void *callbackData);
void fn_1_2B60(void);
void fn_1_2B64(M631ActionQueue *work);
M631AnimationQueue *fn_1_2E9C(void);
void fn_1_2EC4(M631AnimationQueue *queue);
void fn_1_2F5C(M631AnimationNode *object);

M631AnimationNode *fn_1_31D8(M631AnimationQueue *owner);
void fn_1_3228(M631AnimationNode *node);
void fn_1_32C8(M631AnimationNode *inputWork);
s32 fn_1_3690(void *callbackData);
inline void fn_1_3704(M631AnimationNode *work, s32 positionIndex);
s32 fn_1_37FC(void *callbackData);
void fn_1_38E8(M631MotionPairNode *callback,
               M631MotionPairNode *context);
void fn_1_3B44(M631ActorWork *actor);
void fn_1_3B7C(HuVecF *position);
void fn_1_3BC4(void);
void fn_1_3BF4(M631ActorMotionState *hitEffect);
void fn_1_3D00(M631SceneWork *scene);
void fn_1_3E60(M631SceneWork *work);
void fn_1_40BC(M631SceneModelPosition *position, s32 index);
void fn_1_449C(M631SceneWork *work);
M631ActorWork *fn_1_469C(const HuVecF *position);
s32 fn_1_48D4(const HuVecF *position);
void fn_1_49B4(void);
void fn_1_4A34(const HuVecF *position, f32 radius, M631MotionPairNode *stop);
void fn_1_4DD4(const HuVecF *position, f32 radius);
void fn_1_4EA4(M631SceneListNode *node);
void fn_1_4F18(M631SceneListNode *callbackNode);
s32 fn_1_4F6C(M631SceneListNode *callbackNode);
void fn_1_5010(void);
void fn_1_5014(HuVecF *destination);
void fn_1_5054(void);
void fn_1_5084(void);
s16 fn_1_50B4(void);
void fn_1_50DC(HuVecF *direction, f32 magnitude);
s32 fn_1_51D0(f32 position);

/* Phase callbacks and a thirty-second play timer. */
MGSEQ_PARAM lbl_1_data_0 = {
    30, 0, fn_1_1F8, fn_1_248, fn_1_4FC, fn_1_560, fn_1_644, fn_1_990,
    fn_1_10A0, fn_1_1214, fn_1_1218
};
/* Arena, player attachment, obstacle and background model resources. */
s32 lbl_1_data_28[30] = {
    DATANUM(DATA_m631, 0), DATANUM(DATA_m631, 1), DATANUM(DATA_m631, 2), DATANUM(DATA_m631, 3),
    DATANUM(DATA_m631, 4), DATANUM(DATA_m631, 5), DATANUM(DATA_m631, 6), DATANUM(DATA_m631, 7),
    DATANUM(DATA_m631, 8), DATANUM(DATA_m631, 9), DATANUM(DATA_m631, 10), DATANUM(DATA_m631, 11),
    DATANUM(DATA_m631, 12), DATANUM(DATA_m631, 13), DATANUM(DATA_m631, 14), DATANUM(DATA_m631, 15),
    DATANUM(DATA_m631, 16), DATANUM(DATA_m631, 16), DATANUM(DATA_m631, 16), DATANUM(DATA_m631, 16),
    DATANUM(DATA_m631, 16), DATANUM(DATA_m631, 17), DATANUM(DATA_m631, 17), DATANUM(DATA_m631, 17),
    DATANUM(DATA_m631, 17), DATANUM(DATA_m631, 17), DATANUM(DATA_m631, 18), DATANUM(DATA_m631, 19),
    DATANUM(DATA_m631, 20), DATANUM(DATA_m631, 21)
};
/* Conveyor sound handle; -1 before playback starts. */
s32 lbl_1_data_A0 = -1;

/* Runs module constructors and starts Conveyor Bolt when the overlay loads. */
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

/* Runs module destructors when the overlay is unloaded. */
void _epilog(void)
{
    const VoidFunc *dtor = _dtors;
    while (*dtor != 0) {
        (*dtor)();
        dtor++;
    }
}

/* Creates the scene and four player objects, initializes the view and starts the minigame
 * sequence. */
void fn_1_A0(void) {
    OMOBJ * object;
    s32 playerIndex;
    void * scene;
    lbl_1_bss_C = omInitObjMan(10, 4096);
    omGameSysInit(lbl_1_bss_C);
    object = omAddObjEx(lbl_1_bss_C, 101, 0U, 0U, - 1, fn_1_174C);
    lbl_1_bss_8 = object;
    object->data = HuMemDirectMallocNum(HEAP_HEAP, 9148, HU_MEMNUM_OVL);
    scene = object->data;
    playerIndex = 0;
    while (playerIndex < 4)
    {
        object = omAddObjEx(lbl_1_bss_C, 100, 1U, 8U, 0, fn_1_5284);
        object->data = HuMemDirectMallocNum(HEAP_HEAP, 356, HU_MEMNUM_OVL);
        object->work[0] = (u32) playerIndex;
        playerIndex += 1;
    }
    Hu3DBGColorSet(0U, 0U, 0U);
    Hu3DNoSyncSet(1);
    fn_1_162C(scene);
    fn_1_121C(scene);
    fn_1_1374(scene);
    fn_1_14D0(scene);
    lbl_1_bss_0 = 0;
    MgSeqCreate(& lbl_1_data_0);
}

/* The sequence initialization phase assigns teams and prepares the reusable conveyor objects. */
void fn_1_1F8(s16 mode, s16 time) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    fn_1_1F78(scene);
    fn_1_2588(&scene->actionQueue);
    fn_1_2EC4(&scene->animationQueue);
    MgSeqModeNext();
}

/* The opening phase plays the camera animation, reveals the arena and starts each team's
 * entrance. */
void fn_1_248(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    Point3d lightDirection = {0.0f, 450.0f, 1000.0f};
    Point3d lightNormal;
    M631LightColor lightColor = {{255, 255, 255, 255}};
    M631ActorWork *actor;
    s32 index;
    switch (scene->openingPhase) {
    case 0:
        MgSeqModeChangeOff();
        lightNormal.x = -lightDirection.x;
        lightNormal.y = -lightDirection.y;
        lightNormal.z = -lightDirection.z;
        PSVECNormalize(&lightNormal, &lightNormal);
        CharLightCreateV(&lightDirection, &lightNormal, (GXColor *)&lightColor);
        CharLightInfinitytSet();
        HuAudFXPlay(MSM_SE_M631_01);
        Hu3DCameraMotionStart(scene->sceneModels[13], lbl_1_rodata_10[0]);
        scene->openingFrames = 0;
        scene->openingPhase++;
        return;
    case 1:
        if (scene->openingFrames >= 87) {
            scene->openingPhase++;
        }
        /* fall through */
    case 2:
        scene->openingFrames++;
        if (scene->openingFrames >= 180) {
            Hu3DModelAttrSet(scene->indexedWorks[2].model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(scene->indexedWorks[2].model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(scene->indexedWorks[3].model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(scene->indexedWorks[3].model, HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(scene->indexedWorks[4].model, HU3D_ATTR_DISPOFF);
            Hu3DModelAttrSet(scene->indexedWorks[4].model, HU3D_MOTATTR_PAUSE);
            if (scene->isNight != 0) {
                Hu3DModelAttrSet(scene->sceneModels[7], HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrSet(scene->sceneModels[6], HU3D_ATTR_DISPOFF);
            }
            scene->openingPhase++;
            Hu3DCameraMotionOff(scene->sceneModels[13]);
            scene->cameraActive = 1;
            return;
        }
        break;
    case 3:
        for (index = 0; index < scene->motionGroups[0].actorCount; index++) {
            actor = scene->motionGroups[0].actorSlots[index];
            fn_1_A8F0(actor);
        }
        scene->openingFrames = 0;
        scene->openingPhase++;
        return;
    case 4:
        if (scene->openingFrames >= 35) {
            for (index = 0; index < scene->motionGroups[1].actorCount; index++) {
                actor = scene->motionGroups[1].actorSlots[index];
                fn_1_AA78(actor);
            }
            scene->openingPhase++;
        }
        /* fall through */
    case 5:
        scene->openingFrames++;
        if (scene->openingFrames >= 60) {
            MgSeqModeNext();
        }
        break;
    }
}

/* The start phase begins the game music once. */
void fn_1_4FC(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;

    switch (scene->startPhase) {
    case 0:
        lbl_1_bss_4 = HuAudBGMPlay(MSM_STREAM_MGMUS_19);
        scene->startPhase++;
        break;
    default:
        break;
    }
}

/* The play phase enables player updates and, by day, conveyor motion, ending when every ground
 * player is counted out. */
void fn_1_560(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    s32 actorIndex;

    switch (scene->playPhase) {
    case 0:
        if (scene->isNight == 0) {
            lbl_1_data_A0 = HuAudFXPlay(MSM_SE_M631_06);
            Hu3DModelAttrReset(scene->sceneModels[8], HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrReset(scene->sceneModels[9], HU3D_MOTATTR_PAUSE);
            scene->conveyorActive = 1;
        }
        for (actorIndex = 0; actorIndex < 4; actorIndex++) {
            scene->actorSlots[actorIndex]->dispatchEnabled = 1;
        }
        scene->playPhase++;
        /* fall through */
    case 1:
        if (scene->eliminatedCount >= scene->motionGroups[1].actorCount) {
            MgSeqModeNext();
        }
        break;
    default:
        break;
    }
}

/* The finish phase stops play, settles surviving players and reports the winning team after a short
 * delay. */
void fn_1_644(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    M631ActorWork *actor;
    s32 i;

    switch (scene->finishPhase) {
    case 0:
        scene->conveyorActive = 0;
        if (scene->isNight == 0) {
            HuAudFXStop(lbl_1_data_A0);
            Hu3DModelAttrSet(scene->sceneModels[8], HU3D_MOTATTR_PAUSE);
            Hu3DModelAttrSet(scene->sceneModels[9], HU3D_MOTATTR_PAUSE);
        }
        lbl_1_bss_0 = 0;
        for (i = 0; i < 4; i++) {
            actor = scene->actorSlots[i];
            actor->dispatchEnabled = 0;
            actor->movementRequested = 0;
            actor->motionController.finished = 0;
        }
        for (i = 0; i < scene->motionGroups[1].actorCount; i++) {
            actor = scene->motionGroups[1].actorSlots[i];
            if (actor->hit == 0 &&
                Hu3DMotionIDGet(actor->characterModelId) != actor->idleMotionId &&
                Hu3DMotionShiftIDGet(actor->characterModelId) != actor->idleMotionId) {
                CharMotionShiftSet(actor->charNo, actor->idleMotionId,
                                   0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
            }
        }
        for (i = 0; i < scene->motionGroups[0].actorCount; i++) {
            actor = scene->motionGroups[0].actorSlots[i];
            if (actor->releaseFrames > 0) {
                actor->releaseFrames = 0;
                Hu3DModelAttrSet(actor->setupWork.releaseModelId, HU3D_ATTR_DISPOFF);
            }
        }
        HuAudSStreamFadeOut(lbl_1_bss_4, 100);
        scene->finishPhase++;
        break;
    case 1:
        if (++lbl_1_bss_0 >= 30) {
            if (scene->eliminatedCount >= scene->motionGroups[1].actorCount) {
                scene->winnerGroup = 0;
            } else {
                scene->winnerGroup = 1;
            }
            if (scene->motionGroups[scene->winnerGroup].actorCount == 1) {
                MgSeqWinnerSet(
                    scene->motionGroups[scene->winnerGroup].actorSlots[0]->charNo,
                    -1, -1, -1);
            } else {
                MgSeqWinnerSet(scene->motionGroups[scene->winnerGroup].actorSlots[0]->charNo,
                               scene->motionGroups[scene->winnerGroup].actorSlots[1]->charNo,
                               scene->motionGroups[scene->winnerGroup].actorSlots[2]->charNo, -1);
            }
            scene->finishPhase++;
        }
    case 2:
        for (i = 0; i < scene->motionGroups[1].actorCount; i++) {
            actor = scene->motionGroups[1].actorSlots[i];
            if (actor->hit == 0 &&
                Hu3DMotionIDGet(actor->characterModelId) != actor->idleMotionId &&
                Hu3DMotionShiftIDGet(actor->characterModelId) != actor->idleMotionId) {
                CharMotionShiftSet(actor->charNo, actor->idleMotionId,
                                   0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
            }
        }
        break;
    default:
        break;
    }
}

/* The results transition hides active conveyor objects, arranges the players and wipes into the
 * results view. */
void fn_1_990(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    M631ActionNode *actorLink;
    M631ActorCallbackWork *callback;
    M631ActorCallbackWork *nextCallback;
    M631AnimationNode *animation;
    M631ActorWork *actor;
    s32 index;

    switch (scene->resultsTransitionPhase) {
    case 0:
        MgSeqModeChangeOff();
        if (WipeCheck() == 0) {
            WipeColorSet(255, 255, 255);
            WipeCreate(WIPE_MODE_OUT, WIPE_TYPE_NORMAL, 60);
        }
        scene->resultsTransitionPhase++;
        return;
    case 1:
        if (WipeCheck() == 0) {
            scene->resultsTransitionPhase++;
        }
        goto finished;
    case 2:
        scene->endingScene = 1;
        scene->primaryRotation = 0.0f;
        scene->secondaryRotation = 0.0f;
        callback = fn_1_B004();
        while (callback != 0) {
            nextCallback = callback->next;
            fn_1_AF94(callback);
            callback = nextCallback;
        }
        actorLink = scene->actionQueue.activeHead;
        while (actorLink != 0) {
            Hu3DModelAttrSet(actorLink->modelId, HU3D_ATTR_DISPOFF);
            actorLink = actorLink->next;
        }
        animation = scene->animationQueue.head;
        while (animation != 0) {
            Hu3DModelAttrSet(animation->modelId, HU3D_ATTR_DISPOFF);
            if (animation->soundId >= 0) {
                HuAudFXStop(animation->soundId);
                animation->soundId = -1;
            }
            animation = animation->next;
        }
        index = 0;
        while (index < scene->motionGroups[0].actorCount) {
            actor = scene->motionGroups[0].actorSlots[index];
            Hu3DModelHookReset(actor->setupWork.modelId);
            index++;
        }
        if (scene->isNight != 0) {
            if (scene->winnerGroup == 0) {
                index = 0;
                while (index < 3) {
                    actor = scene->motionGroups[0].actorSlots[index];
                    actor->position.x = 300.0f * (f32)index -
                                        300.0f;
                    actor->position.y = 0.0f;
                    actor->position.z = 150.0f;
                    Hu3DModelPosSet(actor->setupWork.modelId,
                                    50.0f + actor->position.x,
                                    50.0f,
                                    actor->position.z - 200.0f);
                    Hu3DModelRotSet(actor->setupWork.modelId, 0.0f,
                                    (-30.0f), 0.0f);
                    Hu3DModelShadowSet(actor->characterModelId);
                    index++;
                }
                actor = scene->motionGroups[1].actorSlots[0];
                actor->position.x = (-500.0f);
                actor->position.y = 0.0f;
                actor->position.z = (-250.0f);
            } else {
                index = 0;
                while (index < 3) {
                    actor = scene->motionGroups[0].actorSlots[index];
                    actor->position.x = 300.0f * (f32)index -
                                        300.0f;
                    actor->position.y = 0.0f;
                    actor->position.z = (-250.0f);
                    Hu3DModelAttrSet(actor->setupWork.modelId, HU3D_ATTR_DISPOFF);
                    Hu3DModelShadowSet(actor->characterModelId);
                    index++;
                }
                actor = scene->motionGroups[1].actorSlots[0];
                actor->position.x = 100.0f;
                actor->position.y = 0.0f;
                actor->position.z = 150.0f;
            }
        } else if (scene->winnerGroup == 0) {
            index = 0;
            while (index < 3) {
                actor = scene->motionGroups[1].actorSlots[index];
                actor->position.x =
                    (300.0f * (f32)index - 300.0f) -
                    150.0f;
                actor->position.y = 0.0f;
                actor->position.z = (-250.0f);
                index++;
            }
            actor = scene->motionGroups[0].actorSlots[0];
            actor->position.x = 350.0f;
            actor->position.y = 0.0f;
            actor->position.z = 150.0f;
            Hu3DModelPosSet(actor->setupWork.modelId,
                            100.0f + actor->position.x,
                            50.0f,
                            actor->position.z - 200.0f);
            Hu3DModelRotSet(actor->setupWork.modelId, 0.0f,
                            30.0f, 0.0f);
            Hu3DModelShadowSet(actor->characterModelId);
        } else {
            index = 0;
            while (index < 3) {
                actor = scene->motionGroups[1].actorSlots[index];
                actor->position.x = 300.0f * (f32)index -
                                    300.0f;
                actor->position.y = 0.0f;
                actor->position.z = 150.0f;
                index++;
            }
            actor = scene->motionGroups[0].actorSlots[0];
            actor->position.x = (-150.0f);
            actor->position.y = 0.0f;
            actor->position.z = (-250.0f);
            Hu3DModelAttrSet(actor->setupWork.modelId, HU3D_ATTR_DISPOFF);
            Hu3DModelShadowSet(actor->characterModelId);
        }
        index = 0;
        while (index < 4) {
            actor = scene->actorSlots[index];
            CharMotionSet(actor->charNo, actor->idleMotionId);
            CharModelAttrReset(actor->charNo, HU3D_MOTATTR_PAUSE);
            CharModelAttrSet(actor->charNo, HU3D_MOTATTR_LOOP);
            CharModelAttrReset(actor->charNo, HU3D_ATTR_DISPOFF);
            actor->rotation.y = 0.0f;
            Hu3DModelPosSetV(actor->characterModelId, &actor->position);
            omSetTra(actor->object, actor->position.x, actor->position.y,
                     actor->position.z);
            Hu3DModelRotSetV(actor->characterModelId, &actor->rotation);
            omSetRot(actor->object, actor->rotation.x, actor->rotation.y,
                     actor->rotation.z);
            index++;
        }
        scene->indexedWorks[0].position.x =
            (-3000.0f);
        scene->indexedWorks[0].position.y =
            (-1600.0f);
        scene->indexedWorks[0].position.z =
            (-12000.0f);
        scene->indexedWorks[1].position.x =
            2200.0f;
        scene->indexedWorks[1].position.y =
            200.0f;
        scene->indexedWorks[1].position.z =
            (-7000.0f);
        WipeCreate(WIPE_MODE_IN, WIPE_TYPE_NORMAL, 60);
        scene->resultsTransitionPhase++;
        return;
    case 3:
        if (WipeCheck() == 0) {
            MgSeqModeNext();
        }
        break;
    }
finished:
    return;
}

/* The results phase starts each team's outcome animation and awards ten coins outside practice
 * mode. */
void fn_1_10A0(s16 mode, s16 time)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    M631ActorWork *actor;
    s32 actorIndex;
    s32 playerNo;

    switch (scene->resultsPhase) {
    case 0:
        for (actorIndex = 0; actorIndex < scene->motionGroups[scene->winnerGroup].actorCount;
             actorIndex++) {
            actor = scene->motionGroups[scene->winnerGroup].actorSlots[actorIndex];
            CharMotionShiftSet(actor->charNo, actor->winMotionId, 0.0f, 5.0f, 0);
            playerNo = actor->playerNo;
            if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
                GwPlayer[playerNo].mgCoinBonus = 10;
            }
        }
        for (actorIndex = 0; actorIndex < scene->motionGroups[scene->winnerGroup ^ 1].actorCount;
             actorIndex++) {
            actor = scene->motionGroups[scene->winnerGroup ^ 1].actorSlots[actorIndex];
            CharMotionShiftSet(actor->charNo, actor->loseMotionId, 0.0f, 5.0f, 0);
        }
        scene->resultsPhase++;
        /* fall through */
    case 1:
    default:
        break;
    }
}

/* The sequence has no additional work during the fade-out phase. */
void fn_1_1214(s16 mode, s16 time)
{
}

/* The sequence has no additional work during its closing phase. */
void fn_1_1218(s16 mode, s16 time)
{
}

/* Creates and positions the gameplay camera during scene setup. */
void fn_1_121C(M631SceneWork *scene)
{
    HuVecF cameraPosition = {0.0f, 2000.0f, 2800.0f};
    HuVecF cameraTarget = {0.0f, 200.0f, -70.0f};
    HuVecF cameraForward;
    HuVecF computedUp;
    HuVecF cameraRight;
    HuVecF cameraUp = {0.0f, 1.0f, 0.0f};
    Hu3DCameraCreate(lbl_1_rodata_10[0]);
    Hu3DCameraPerspectiveSet(lbl_1_rodata_10[0], 25.0f, 20.0f, 50000.0f, 1.2f);
    PSVECSubtract(&cameraTarget, &cameraPosition, &cameraForward);
    PSVECNormalize(&cameraForward, &cameraForward);
    PSVECCrossProduct(&cameraUp, &cameraForward, &cameraRight);
    PSVECNormalize(&cameraRight, &cameraRight);
    PSVECCrossProduct(&cameraForward, &cameraRight, &computedUp);
    PSVECNormalize(&computedUp, &computedUp);
    Hu3DCameraPosSet(lbl_1_rodata_10[0], cameraPosition.x, cameraPosition.y, cameraPosition.z,
                     computedUp.x, computedUp.y, computedUp.z,
                     cameraTarget.x, cameraTarget.y, cameraTarget.z);
}

/* Sets the scene's directional light and ambient brightness for its day or night variant. */
void fn_1_1374(M631SceneWork *scene)
{
    HuVecF lightPosition = {0.0f, 10000.0f, 10000.0f};
    HuVecF lightDirection = {0.0f, 0.0f, 0.0f};
    HuVecF normalizedDirection;
    HU3D_LIGHTID lightId;
    PSVECSubtract(&lightDirection, &lightPosition, &normalizedDirection);
    PSVECNormalize(&normalizedDirection, &normalizedDirection);
    Hu3DLighInit();
    if (scene->isNight != 0) {
        lightId = Hu3DGLightCreate(lightPosition.x, lightPosition.y, lightPosition.z,
                                   normalizedDirection.x, normalizedDirection.y,
                                   normalizedDirection.z, 127, 127, 175);
        Hu3DAmbColorSet(0.5f, 0.5f, 0.5f);
    } else {
        lightId = Hu3DGLightCreate(lightPosition.x, lightPosition.y, lightPosition.z,
                                   normalizedDirection.x, normalizedDirection.y,
                                   normalizedDirection.z, 175, 175, 175);
        Hu3DAmbColorSet(1.0f, 1.0f, 1.0f);
    }
    Hu3DGLightStaticSet(lightId, 1);
    Hu3DGLightInfinitytSet(lightId);
}

/* Sets the overhead shadow view and day or night shadow tint during scene setup. */
void fn_1_14D0(M631SceneWork *scene)
{
    HuVecF computedUp;
    HuVecF cameraForward;
    HuVecF cameraRight;
    HuVecF cameraUp = {0.0f, 1.0f, 0.0f};
    HuVecF cameraPosition = {0.0f, 4000.0f, 0.0f};
    HuVecF cameraTarget = {0.0f, 0.0f, 0.1f};
    PSVECSubtract(&cameraTarget, &cameraPosition, &cameraForward);
    PSVECNormalize(&cameraForward, &cameraForward);
    PSVECCrossProduct(&cameraUp, &cameraForward, &cameraRight);
    PSVECNormalize(&cameraRight, &cameraRight);
    PSVECCrossProduct(&cameraForward, &cameraRight, &computedUp);
    PSVECNormalize(&computedUp, &computedUp);
    Hu3DShadowCreate(25.0f, 20.0f, 50000.0f);
    Hu3DShadowTPLvlSet(0.5f);
    Hu3DShadowPosSet(&cameraPosition, &computedUp, &cameraTarget);
    Hu3DShadowSizeSet(128);
    if (scene->isNight != 0) {
        Hu3DShadowColSet(15, 15, 95);
        return;
    }
    Hu3DShadowColSet(32, 48, 64);
}

/* Clears scene state and selects the arena variant from the minigame's night setting at startup. */
void fn_1_162C(M631SceneWork *scene)
{
    s16 nightValue;
    s32 i;

    for (i = 0; i < 30; i++) {
        scene->sceneModels[i] = -1;
    }
    for (i = 0; i < 1; i++) {
        scene->unusedSlots[i] = -1;
    }
    for (i = 0; i < 4; i++) {
        scene->actorSlots[i] = 0;
    }
    nightValue = GwMgNightF;
    scene->isNight = (nightValue == 1);
    scene->playPhase = 0;
    scene->openingPhase = 0;
    scene->startPhase = 0;
    scene->resultsTransitionPhase = 0;
    scene->finishPhase = 0;
    scene->resultsPhase = 0;
    scene->conveyorActive = 0;
    scene->eliminatedCount = 0;
    scene->winnerGroup = -1;
    scene->pursuitCount = 0;
    fn_1_AB4C();
}

/* The scene object's first update loads the arena, initializes effects and installs its recurring
 * update. */
void fn_1_174C(OMOBJ *obj) {
    M631SceneWork *scene = lbl_1_bss_8->data;

    fn_1_1830(scene);
    fn_1_B014();
    scene->cameraActive = 0;
    scene->endingScene = 0;
    scene->cameraAngle = 0.0f;
    fn_1_49B4();
    obj->objFunc = (OMOBJ_FUNC) fn_1_17C8;
}

/* The scene's recurring update advances camera movement, conveyor spawning, scenery and active
 * effects. */
void fn_1_17C8(void) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    fn_1_3E60(scene);
    if (scene->conveyorActive != 0) {
        fn_1_2B64(&scene->actionQueue);
    }
    fn_1_3D00(scene);
    fn_1_449C(scene);
    fn_1_AB60();
    fn_1_B1AC();
}

/* Loads or links arena models and places the background props on the scene object's first
 * update. */
void fn_1_1830(M631SceneWork *scene)
{
    HU3D_MODELID model;
    s32 index;

    if (scene->isNight != 0) {
        if (scene->sceneModels[5] < 0) {
            model = scene->sceneModels[5] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[5], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[5]);
        }
    } else {
        if (scene->sceneModels[4] < 0) {
            model = scene->sceneModels[4] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[4], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[4]);
        }
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (scene->isNight != 0) {
        if (scene->sceneModels[27] < 0) {
            model = scene->sceneModels[27] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[27], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[27]);
        }
    } else {
        if (scene->sceneModels[26] < 0) {
            model = scene->sceneModels[26] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[26], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[26]);
        }
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (scene->isNight != 0) {
        if (scene->sceneModels[7] < 0) {
            model = scene->sceneModels[7] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[7], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[7]);
        }
    } else {
        if (scene->sceneModels[6] < 0) {
            model = scene->sceneModels[6] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[6], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[6]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (scene->isNight != 0) {
        if (scene->sceneModels[29] < 0) {
            model = scene->sceneModels[29] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[29], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[29]);
        }
    } else {
        if (scene->sceneModels[28] < 0) {
            model = scene->sceneModels[28] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[28], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[28]);
        }
    }
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (scene->sceneModels[8] < 0) {
        model = scene->sceneModels[8] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[8], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[8]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(model);

    if (scene->sceneModels[9] < 0) {
        model = scene->sceneModels[9] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[9], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[9]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    if (scene->sceneModels[13] < 0) {
        model = scene->sceneModels[13] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[13], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[13]);
    }
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);

    Hu3DModelLayerSet(model, 0);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);

    for (index = 0; index < 5; index++) {
        fn_1_40BC(&scene->indexedWorks[index], index);
    }
    scene->primaryRotation = 0.0f;
    scene->secondaryRotation = 0.0f;
}

/* The initialization phase assigns the solo and three-player teams to their day or night roles and
 * positions. */
void fn_1_1F78(M631SceneWork *scene)
{
    M631ActorWork *actor;
    s32 otherGroupIndex = 0;
    s32 actorIndex;
    s32 primaryGroupCount;
    s32 secondaryGroupCount;

    for (actorIndex = 0; actorIndex < 4; actorIndex++) {
        actor = scene->actorSlots[actorIndex];
        if (actor->groupSelector == 0) {
            scene->motionGroups[scene->isNight].actorSlots[0] = actor;
            if (scene->isNight != 0) {
                actor->motionController.type = 1;
            } else {
                actor->motionController.type = 0;
            }
            fn_1_5AAC(actor, 0, scene->isNight);
        } else {
            scene->motionGroups[scene->isNight ^ 1].actorSlots[otherGroupIndex] = actor;
            if (scene->isNight != 0) {
                actor->motionController.type = 2;
            } else {
                actor->motionController.type = 3;
            }
            fn_1_5AAC(actor, otherGroupIndex, scene->isNight ^ 1);
            otherGroupIndex++;
        }
    }
    if (scene->isNight != 0) {
        primaryGroupCount = 3;
    } else {
        primaryGroupCount = 1;
    }
    scene->motionGroups[0].actorCount = primaryGroupCount;
    if (scene->isNight != 0) {
        secondaryGroupCount = 1;
    } else {
        secondaryGroupCount = 3;
    }
    scene->motionGroups[1].actorCount = secondaryGroupCount;
    for (actorIndex = 0; actorIndex < scene->motionGroups[0].actorCount; actorIndex++) {
        actor = scene->motionGroups[0].actorSlots[actorIndex];
        Hu3DModelHookSet(actor->setupWork.modelId, "db_ch", actor->characterModelId);
    }
    if (scene->isNight != 0) {
        actor = scene->motionGroups[0].actorSlots[0];
        actor->position.x = 350.0f;
        actor->position.y = 400.0f;
        actor->position.z = (-250.0f);
        actor = scene->motionGroups[0].actorSlots[1];
        actor->position.x = 350.0f;
        actor->position.y = 400.0f;
        actor->position.z = 0.0f;
        actor = scene->motionGroups[0].actorSlots[2];
        actor->position.x = 350.0f;
        actor->position.y = 400.0f;
        actor->position.z = 250.0f;
        actor = scene->motionGroups[1].actorSlots[0];
        actor->position.x = (-350.0f);
        actor->position.y = 0.0f;
        actor->position.z = 0.0f;
    } else {
        actor = scene->motionGroups[0].actorSlots[0];
        actor->position.x = 350.0f;
        actor->position.y = 400.0f;
        actor->position.z = 0.0f;
        actor = scene->motionGroups[1].actorSlots[0];
        actor->position.x = (-350.0f);
        actor->position.y = 0.0f;
        actor->position.z = (-250.0f);
        actor = scene->motionGroups[1].actorSlots[1];
        actor->position.x = (-350.0f);
        actor->position.y = 0.0f;
        actor->position.z = 0.0f;
        actor = scene->motionGroups[1].actorSlots[2];
        actor->position.x = (-350.0f);
        actor->position.y = 0.0f;
        actor->position.z = 250.0f;
    }
}

/* Creates a player's hooked model and hidden secondary animation model during actor setup. */
void fn_1_228C(HU3D_MODELID *models)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HU3D_MODELID model;

    if (scene->isNight != 0) {
        if (scene->sceneModels[1] < 0) {
            model = scene->sceneModels[1] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[1], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[1]);
        }
    } else {
        if (scene->sceneModels[0] < 0) {
            model = scene->sceneModels[0] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[0], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[0]);
        }
    }
    models[0] = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowSet(model);

    if (scene->isNight != 0) {
        if (scene->sceneModels[15] < 0) {
            model = scene->sceneModels[15] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[15], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[15]);
        }
    } else {
        if (scene->sceneModels[14] < 0) {
            model = scene->sceneModels[14] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[14], HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[14]);
        }
    }
    models[1] = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 6);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 6.14f);
}

/* Returns the day or night collision radius used when actors register their movement callbacks. */
f32 fn_1_2528(void)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    f32 collisionRadius;

    if (scene->isNight != 0) {
        collisionRadius = 85.0f;
    } else {
        collisionRadius = 150.0f;
    }
    return collisionRadius;
}

/* Initializes the sixteen-node conveyor obstacle pool before play starts. */
void fn_1_2588(M631ActionQueue *queue)
{
    M631ActionNode *node;
    s32 i;

    queue->freeHead = queue->nodes;
    queue->activeHead = 0;
    node = queue->nodes;
    for (i = 0; i < 15; i++, node++) {
        fn_1_264C(node);
        node->owner = queue;
        node->previous = NULL;
        node->next = node + 1;
    }
    fn_1_264C(node);
    node->owner = queue;
    node->previous = NULL;
    node->next = NULL;
    queue->stage = 0;
    for (i = 0; i < 5; i++) {
        queue->modelCounts[i] = 0;
    }
}

/* Creates or links a hidden conveyor obstacle model while its pool is initialized. */
void fn_1_264C(M631ActionNode *work)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HU3D_MODELID model;

    if (scene->sceneModels[10] < 0) {
        model = scene->sceneModels[10] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[10], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[10]);
    }
    work->modelId = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DModelShadowMapSet(model);
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    work->completed = 0;
}

/* Takes a free conveyor obstacle node and adds it to the active list for a caller to configure. */
M631ActionNode *fn_1_2798(M631ActionQueue *owner)
{
    M631ActionNode *node = 0;
    node = owner->freeHead;
    if (node) {
        owner->freeHead = node->next;
        node->next = owner->activeHead;
        if (node->next) {
            node->next->previous = node;
        }
        owner->activeHead = node;
    }
    return node;
}

/* Returns a conveyor obstacle to its free list and releases its lane occupancy. */
void fn_1_27E8(M631ActionNode *node)
{
    M631ActionQueue *owner;
    owner = node->owner;
    owner->modelCounts[node->modelIndex]--;
    if (node->previous != 0) {
        node->previous->next = node->next;
    } else {
        owner->activeHead = node->next;
    }
    if (node->next != 0) {
        node->next->previous = node->previous;
    }
    node->previous = 0;
    node->next = owner->freeHead;
    owner->freeHead = node;
}

/* The obstacle's movement callback carries it left and opens or closes its animation at the
 * conveyor ends. */
void fn_1_2868(void *callbackData)
{
    M631ActionNode *work;
    f32 progress;

    work = (M631ActionNode *)callbackData;
    fn_1_3B7C(&work->position);
    switch (work->state) {
    case 0:
        if (work->position.x <= 700.0f) {
            Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
            work->state++;
            work->frame = 0;
            HuAudFXPlayPan(MSM_SE_M631_04, 80);
            return;
        }
        break;
    case 1:
        work->frame++;
        progress = (f32)work->frame / 20.0f;
        Hu3DMotionTimeSet(work->modelId,
                          20.0f * progress);
        if (work->frame >= 20) {
            work->state++;
            work->frame++;
        }
        break;
    case 2:
        if (work->position.x <= (-633.2f)) {
            HuAudFXPlayPan(MSM_SE_M631_04, 80);
            work->state++;
            work->frame = 0;
            return;
        }
        break;
    case 3:
        work->frame++;
        progress = (f32)work->frame / 20.0f;
        Hu3DMotionTimeSet(work->modelId,
                          20.0f *
                              (1.0f - progress));
        if (work->frame >= 20) {
            work->completed = 1;
            Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
            work->frame++;
        }
        break;
    case 4:
        break;
    }
}

/* The obstacle's completion callback updates its model and recycles it after closing or leaving the
 * arena. */
s32 fn_1_2A84(void *callbackData)
{
    M631ActionNode *node;
    M631ActionQueue *owner;

    node = callbackData;
    Hu3DModelPosSetV(node->modelId, &node->position);
    if (node->completed != 0 || node->position.x < (-800.0f)) {
        owner = node->owner;
        owner->modelCounts[node->modelIndex]--;
        if (node->previous != 0) {
            node->previous->next = node->next;
        } else {
            owner->activeHead = node->next;
        }
        if (node->next != 0) {
            node->next->previous = node->previous;
        }
        node->previous = 0;
        node->next = owner->freeHead;
        owner->freeHead = node;
        return 1;
    }
    return 0;
}

/* The obstacle pair-contact callback has no additional response. */
void fn_1_2B60(void) { }

/* While the conveyor runs, periodically spawns obstacles in one lane or one of the allowed lane
 * pairs. */
void fn_1_2B64(M631ActionQueue *work)
{
    M631ActorDefaults defaults = {{{0, 2}, {0, 3}, {0, 4}, {1, 3}, {1, 4}, {2, 4}}};
    s32 selected[2];
    M631ActionNode *newNode;
    M631ActionNode *actor;
    HuVecF position;
    HuVecF *positionPointer;
    s32 count;
    s32 modelIndex;
    s32 i;
    switch (work->stage) {
    case 0:
        work->timer = 0;
        work->stage++;
        /* fall through */
    case 1:
        work->timer++;
        if (work->timer < 89) {
            return;
        }
        work->timer = 0;
        work->stage++;
        return;
    case 2:
    case 3:
    case 4:
        work->timer++;
        if (work->timer < 179) {
            return;
        }
        if (work->stage == 4) {
            modelIndex = frandmod(6);
            selected[0] = defaults.lanePairs[modelIndex][0];
            selected[1] = defaults.lanePairs[modelIndex][1];
            count = 2;
        } else {
            count = 1;
            selected[0] = frandmod(5);
        }
        for (i = 0; i < count; i++) {
            M631ActionNode *node = 0;

            node = work->freeHead;
            if (node != 0) {
                work->freeHead = node->next;
                node->next = work->activeHead;
                if (node->next != 0) {
                    node->next->previous = node;
                }
                work->activeHead = node;
            }
            newNode = node;
            actor = newNode;
            if (actor != 0) {
                actor->position.x = 800.0f;
                actor->position.y = 0.0f;
                actor->position.z = 100.0f + 200.0f * (f32)selected[i] - 500.0f;
                position = actor->position;
                positionPointer = &position;
                fn_1_4A34(positionPointer, 50.0f, 0);
                fn_1_AFF8(&actor->callbackWork, 50.0f, 3);
                fn_1_AF2C(&actor->callbackWork, (u32)actor, (u32)fn_1_2868,
                          (u32)fn_1_2A84, (u32)fn_1_2B60,
                          &actor->position);
                actor->modelIndex = selected[i];
                work->modelCounts[actor->modelIndex]++;
                Hu3DMotionTimeSet(actor->modelId, 0.0f);
                actor->state = 0;
                actor->completed = 0;
            }
        }
        work->timer = 0;
        work->stage++;
        if (work->stage == 5) {
            work->stage = 2;
        }
        break;
    default:
        break;
    }
}

/* Returns the scene's falling-object pool to actor setup. */
M631AnimationQueue *fn_1_2E9C(void)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    return &scene->animationQueue;
}

/* Initializes the thirty-two falling-object nodes and their models before play starts. */
void fn_1_2EC4(M631AnimationQueue *queue)
{
    s32 nodeIndex;
    M631AnimationNode *node;

    queue->freeHead = queue->nodes;
    queue->head = NULL;
    node = queue->nodes;
    nodeIndex = 0;
    while (nodeIndex < 31) {
        fn_1_2F5C(node);
        node->queueOwner = queue;
        node->previous = NULL;
        node->next = node + 1;
        nodeIndex += 1;
        node = node + 1;
    }
    fn_1_2F5C(node);
    node->queueOwner = queue;
    node->previous = NULL;
    node->next = NULL;
}

/* Creates or links a falling object's main model and hidden contact-effect model during pool
 * setup. */
void fn_1_2F5C(M631AnimationNode *object)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HU3D_MODELID model;

    if (scene->isNight != 0) {
        if (scene->sceneModels[3] < 0) {
            model = scene->sceneModels[3] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[3], HU_MEMNUM_OVL,
                                     HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[3]);
        }
    } else if (scene->sceneModels[2] < 0) {
        model = scene->sceneModels[2] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[2], HU_MEMNUM_OVL,
                                 HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[2]);
    }
    object->modelId = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionSpeedSet(model, 6.24f);

    if (scene->sceneModels[11] < 0) {
        model = scene->sceneModels[11] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[11], HU_MEMNUM_OVL,
                                 HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[11]);
    }
    object->effectModelId = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 3);
    Hu3DModelPosSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);

    object->position.x = 0.0f;
    object->position.y = 0.0f;
    object->position.z = 0.0f;
    object->soundId = -1;
}

/* Takes a free falling-object node and adds it to the active list when an actor launches an
 * object. */
M631AnimationNode *fn_1_31D8(M631AnimationQueue *owner)
{
    M631AnimationNode *node = 0;
    node = owner->freeHead;
    if (node) {
        owner->freeHead = node->next;
        node->next = owner->head;
        if (node->next) {
            node->next->previous = node;
        }
        owner->head = node;
    }
    return node;
}

/* Stops a falling object's sound and returns the node to its free list when it is removed. */
void fn_1_3228(M631AnimationNode *node) {
    M631AnimationQueue *queue;
    if (node->soundId >= 0) {
        HuAudFXStop(node->soundId);
        node->soundId = -1;
    }
    queue = node->queueOwner;
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        queue->head = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    }
    node->previous = NULL;
    node->next = queue->freeHead;
    queue->freeHead = node;
}

/* The falling object's movement callback grows and drops it, then times its disappearance or
 * conveyor exit. */
void fn_1_32C8(M631AnimationNode *inputWork)
{
    M631AnimationNode *work = inputWork;
    M631SceneWork *scene = lbl_1_bss_8->data;
    HuVecF position;
    HuVecF *positionPointer;
    f32 progress;

    switch (work->animationState) {
    case 0:
        HuAudFXPlayPan(MSM_SE_M631_02, fn_1_51D0(work->position.x));
        Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
        work->timer = 0;
        work->animationState++;
        /* fall through */
    case 1:
        work->timer++;
        progress = (f32)work->timer / 10.0f;
        if (scene->isNight != 0) {
            Hu3DModelScaleSet(work->modelId, 0.66f,
                              progress, 0.66f);
        } else {
            Hu3DModelScaleSet(work->modelId, 1.0f,
                              progress, 1.0f);
        }
        /* The first ten updates replace the release height with a fixed descent from 400 to 100
         * units. */
        work->position.y = 100.0f * progress +
                           400.0f * (1.0f - progress);
        if (work->timer >= 10) {
            work->animationState++;
        }
        break;
    case 2:
        if (work->position.y <= 0.0f) {
            fn_1_3704(work, 0);
            work->animationState++;
        }
        /* fall through */
    case 3:
        work->position.y -= 30.0f;
        if (work->position.y <= (-100.0f)) {
            work->soundId = HuAudFXPlayPan(MSM_SE_M631_03,
                                           fn_1_51D0(work->position.x));
            work->position.y = (-100.0f);
            work->callbackWork.state = 5;
            work->timer = 0;
            if (scene->isNight != 0) {
                work->animationState = 10;
            } else {
                position = work->position;
                positionPointer = &position;
                fn_1_4A34(positionPointer, 75.0f,
                          &work->callbackWork);
                work->animationState = 20;
            }
        }
        break;
    case 10:
        work->timer++;
        if (work->timer >= 90) {
            work->timer = 0;
            work->animationState++;
        }
        break;
    case 11:
        work->timer++;
        if ((work->timer & 1) != 0) {
            Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
        }
        if (work->timer >= 30) {
            HuAudFXStop(work->soundId);
            work->soundId = -1;
            work->animationState = 100;
        }
        break;
    case 20:
        if (work->position.x < (-525.0f)) {
            work->timer++;
            if ((work->timer & 1) != 0) {
                Hu3DModelAttrSet(work->modelId, HU3D_ATTR_DISPOFF);
            } else {
                Hu3DModelAttrReset(work->modelId, HU3D_ATTR_DISPOFF);
            }
        }
        HuAudFXPanning(work->soundId, fn_1_51D0(work->position.x));
        if (work->position.x < (-625.0f)) {
            HuAudFXStop(work->soundId);
            work->soundId = -1;
            work->animationState = 100;
        }
        fn_1_3B7C(&work->position);
        break;
        break;
    default:
        break;
    }
}

/* The scheduled contact-effect job follows the conveyor only for ground-level effects, then
 * hides the effect and finishes when its animation ends. */
s32 fn_1_3690(void *callbackData) {
    M631AnimationNode *work = (M631AnimationNode *) callbackData;
    if (work->effectHeightIndex == 0) {
        fn_1_3B7C(&work->targetPosition);
        Hu3DModelPosSetV(work->effectModelId, &work->targetPosition);
    }
    if (Hu3DMotionEndCheck(work->effectModelId) != 0) {
        Hu3DModelAttrSet(work->effectModelId, HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* Starts a falling object's contact-effect animation at one of three heights when a contact is
 * reported. */
inline void fn_1_3704(M631AnimationNode *work, s32 positionIndex)
{
    M631VerticalPositions yPositions = {{0.0f, 100.0f, 200.0f}};
    if (fn_1_B0B8(fn_1_3690, work) != 0U) {
        Hu3DModelAttrReset(work->effectModelId, HU3D_MOTATTR_PAUSE);
        Hu3DMotionTimeSet(work->effectModelId, 0.0f);
        work->targetPosition.x = work->position.x;
        work->targetPosition.y = yPositions.heights[positionIndex];
        work->targetPosition.z = work->position.z;
        Hu3DModelPosSetV(work->effectModelId, &work->targetPosition);
        Hu3DModelRotSet(work->effectModelId, 0.0f, 0.0f, 0.0f);
        Hu3DModelAttrReset(work->effectModelId, HU3D_ATTR_DISPOFF);
        work->effectHeightIndex = positionIndex;
    }
}

/* The falling object's completion callback updates its model and recycles it after its lifetime or
 * left exit. */
s32 fn_1_37FC(void *callbackData) {
    M631AnimationNode *node = (M631AnimationNode *) callbackData;
    M631AnimationQueue *queue;
    Hu3DModelPosSetV(node->modelId, &node->position);
    if ((node->position.x < (-800.0f)) || (node->animationState == 100)) {
        Hu3DModelAttrSet(node->modelId, HU3D_ATTR_DISPOFF);
        if (node->soundId >= 0) {
            HuAudFXStop(node->soundId);
            node->soundId = -1;
        }
        queue = node->queueOwner;
        if (node->previous) {
            node->previous->next = node->next;
        } else {
            queue->head = node->next;
        }
        if (node->next) {
            node->next->previous = node->previous;
        }
        node->previous = NULL;
        node->next = queue->freeHead;
        queue->freeHead = node;
        return 1;
    }
    return 0;
}

/* The pair-contact callback replaces a falling object with its effect when it reaches the contacted
 * height. */
void fn_1_38E8(M631MotionPairNode *callback,
               M631MotionPairNode *context)
{
    M631AnimationNode *work =
        (M631AnimationNode *)callback->owner;

    switch (context->state) {
    case 3: {

        if (work->position.y <= 100.0f) {
            HuAudFXPlayPan(MSM_SE_M631_05, fn_1_51D0(work->position.x));
            work->animationState = 100;
            fn_1_3704(work, 1);
        }
    }
    case 4:
        break;
    case 5: {

        if (work->position.y <= 200.0f) {
            HuAudFXPlayPan(MSM_SE_M631_05, fn_1_51D0(work->position.x));
            work->animationState = 100;
            fn_1_3704(work, 2);
        }
        break;
    }
    default:
        break;
    }
}

/* Actor initialization registers the player in the scene and copies its day or night setting. */
void fn_1_3B44(M631ActorWork *actor) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    scene->actorSlots[actor->playerNo] = actor;
    actor->isNight = scene->isNight;
}

/* Movement callbacks carry a position left by one conveyor step while conveyor motion is
 * enabled. */
void fn_1_3B7C(HuVecF *position)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;

    if (scene->conveyorActive != 0) {
        position->x -= 3.34f;
    }
}

/* An actor's elimination path increments the group-one completion count checked by the play
 * phase. */
void fn_1_3BC4(void) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    scene->eliminatedCount += 1;
}

/* Actor setup creates or links a hidden secondary model for its motion state. */
void fn_1_3BF4(M631ActorMotionState *hitEffect)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HU3D_MODELID model;

    if (scene->sceneModels[12] < 0) {
        model = scene->sceneModels[12] = Hu3DModelCreate(
            HuDataSelHeapReadNum(lbl_1_data_28[12], HU_MEMNUM_OVL, HEAP_MODEL));
    } else {
        model = Hu3DModelLink(scene->sceneModels[12]);
    }
    hitEffect->modelId = model;
    Hu3DModelAttrSet(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelLayerSet(model, 6);
    Hu3DModelPosSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelRotSet(model, 0.0f, 0.0f, 0.0f);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
}

/* Each scene update slowly rotates both background models, wrapping their angles in degrees. */
void fn_1_3D00(M631SceneWork *scene) {
    scene->primaryRotation -= 0.02f;
    while (scene->primaryRotation < (-180.0f)) {
        scene->primaryRotation += 360.0f;
    }
    scene->secondaryRotation -= 0.03f;
    while (scene->secondaryRotation < (-180.0f)) {
        scene->secondaryRotation += 360.0f;
    }
    if (scene->isNight != 0) {
        Hu3DModelRotSet(scene->sceneModels[5], 0.0f, scene->primaryRotation, 0.0f);
        Hu3DModelRotSet(scene->sceneModels[27], 0.0f, scene->secondaryRotation, 0.0f);
        return;
    }
    Hu3DModelRotSet(scene->sceneModels[4], 0.0f, scene->primaryRotation, 0.0f);
    Hu3DModelRotSet(scene->sceneModels[26], 0.0f, scene->secondaryRotation, 0.0f);
}

/* After the opening camera animation, each scene update applies the gameplay or results camera
 * bob. */
void fn_1_3E60(M631SceneWork *work)
{
    M631CameraPoints eyePositions = {{{0.0f, 2000.0f, 2800.0f}, {0.0f, 1000.0f, 2600.0f}}};
    M631CameraPoints lookAtPositions = {{{0.0f, 200.0f, -70.0f}, {0.0f, 250.0f, 0.0f}}};
    Point3d direction;
    Point3d forward;
    Point3d right;
    Point3d up = {0.0f, 1.0f, 0.0f};
    Point3d *lookAt;
    Point3d *eye;
    if (work->cameraActive != 0) {
        work->cameraAngle += 1.0f;
        while (work->cameraAngle > 180.0f) {
            work->cameraAngle -= 360.0f;
        }
        eye = &eyePositions.points[work->endingScene];
        lookAt = &lookAtPositions.points[work->endingScene];
        /* Both additions affect the same eye-position Y component. */
        eye->y += 50.0 *
                     sin((3.141592653589793 * (f64)work->cameraAngle) /
                         180.0);
        eye->y += 50.0 *
                     sin((3.141592653589793 * (f64)work->cameraAngle) /
                         180.0);
        PSVECSubtract(lookAt, eye, &direction);
        PSVECNormalize(&direction, &direction);
        PSVECCrossProduct(&up, &direction, &right);
        PSVECNormalize(&right, &right);
        PSVECCrossProduct(&direction, &right, &forward);
        PSVECNormalize(&forward, &forward);
        Hu3DCameraPosSet(lbl_1_rodata_10[0], eye->x, eye->y, eye->z,
                         forward.x, forward.y, forward.z,
                         lookAt->x, lookAt->y, lookAt->z);
    }
}

/* Scene setup creates or links one background prop and applies its placement, scale and animation
 * offset. */
void fn_1_40BC(M631SceneModelPosition *position, s32 index)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HuVecF positions[5] = {
        {-3000.0f, -3700.0f, -12000.0f},
        {2500.0f, -2000.0f, -7000.0f},
        {9000.0f, 0.0f, -4000.0f},
        {3000.0f, 0.0f, 4000.0f},
        {6000.0f, 0.0f, 8000.0f},
    };
    f32 scales[5] = {0.7f, 0.92f, 0.9f, 0.83f, 0.75f};
    f32 motionTimes[5] = {0.0f, 60.0f, 120.0f, 60.0f, 120.0f};
    HU3D_MODELID model;

    if (scene->isNight != 0) {
        if (scene->sceneModels[index + 21] < 0) {
            model = scene->sceneModels[index + 21] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[index + 21], HU_MEMNUM_OVL,
                                     HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[index + 21]);
        }
    } else {
        if (scene->sceneModels[index + 16] < 0) {
            model = scene->sceneModels[index + 16] = Hu3DModelCreate(
                HuDataSelHeapReadNum(lbl_1_data_28[index + 16], HU_MEMNUM_OVL,
                                     HEAP_MODEL));
        } else {
            model = Hu3DModelLink(scene->sceneModels[index + 16]);
        }
    }

    position->model = model;
    Hu3DModelAttrReset(model, HU3D_MOTATTR_PAUSE);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 0);
    position->position = positions[index];
    if (index == 0) {
        position->motionTime = 60.0f;
        {
            f32 angle = (f32)((3.141592653589793 * (f64)position->motionTime) /
                              180.0);
            Hu3DModelPosSet(
                model,
                (f32)((f64)position->position.x +
                      200.0 * sin((f64)angle)),
                (f32)((f64)position->position.y +
                      100.0 *
                          sin((f64)(2.0f * angle))),
                position->position.z);
        }
    } else {
        position->motionTime = 0.0f;
        Hu3DModelPosSetV(model, &position->position);
    }
    /* This final placement also replaces the first prop's sine-offset position. */
    Hu3DModelPosSetV(model, &position->position);
    Hu3DModelRotSet(model, 0.0f, 0.0f,
                    0.0f);
    Hu3DModelScaleSet(model, scales[index], scales[index], scales[index]);
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DMotionTimeSet(model, motionTimes[index]);
}

/* Each scene update moves the first two background props along their horizontal and vertical sine
 * paths. */
void fn_1_449C(M631SceneWork *work)
{
    HuVecF position;
    f32 angle;

    work->indexedWorks[0].motionTime += 1.0f;
    while (work->indexedWorks[0].motionTime > 180.0f) {
        work->indexedWorks[0].motionTime -= 360.0f;
    }
    work->indexedWorks[1].motionTime += 1.0f;
    while (work->indexedWorks[1].motionTime > 180.0f) {
        work->indexedWorks[1].motionTime -= 360.0f;
    }

    angle = (f32)((3.141592653589793 * (f64)work->indexedWorks[0].motionTime) /
                  180.0);
    position.x = (f32)((f64)work->indexedWorks[0].position.x +
                       (200.0 * sin((f64)angle)));
    position.y = (f32)((f64)work->indexedWorks[0].position.y +
                       (100.0 *
                        sin((f64)(2.0f * angle))));
    position.z = work->indexedWorks[0].position.z;
    Hu3DModelPosSetV(work->indexedWorks[0].model, &position);

    angle = (f32)((3.141592653589793 * (f64)work->indexedWorks[1].motionTime) /
                  180.0);
    position.x = (f32)((f64)work->indexedWorks[1].position.x +
                       (200.0 * sin((f64)angle)));
    position.y = (f32)((f64)work->indexedWorks[1].position.y +
                       (100.0 *
                        sin((f64)(2.0f * angle))));
    position.z = work->indexedWorks[1].position.z;
    Hu3DModelPosSetV(work->indexedWorks[1].model, &position);
}

/* Actor behavior chooses the nearest eligible group-one player using squared distance in the arena
 * plane. */
M631ActorWork *fn_1_469C(const HuVecF *position)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    M631SearchIndexDefaults indexes = {{0, 1, 2}};
    f32 distances[3];
    s32 candidateCount;
    s32 index;
    M631ActorWork *result;
    if (scene->motionGroups[1].actorCount == 1) {
        if (scene->motionGroups[1].actorSlots[0]->hit == 0) {
            result = scene->motionGroups[1].actorSlots[0];
            return result;
        }
        return 0;
    }
    candidateCount = 0;
    index = 0;
    while (index < 3) {
        if (scene->motionGroups[1].actorSlots[index]->hit == 0) {
            indexes.actorIndexes[candidateCount] = index;
            distances[candidateCount] =
                (position->x -
                 scene->motionGroups[1].actorSlots[index]->position.x) *
                    (position->x -
                     scene->motionGroups[1].actorSlots[index]->position.x) +
                (position->z -
                 scene->motionGroups[1].actorSlots[index]->position.z) *
                    (position->z -
                     scene->motionGroups[1].actorSlots[index]->position.z);
            candidateCount++;
        }
        index++;
    }
    if (candidateCount == 0) {
        return 0;
    }
    if (candidateCount == 1) {
        result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[0]];
        return result;
    }
    if (candidateCount == 2) {
        if (distances[0] < distances[1]) {
            result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[0]];
        } else {
            result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[1]];
        }
        return result;
    }
    if (distances[0] < distances[1]) {
        if (distances[0] < distances[2]) {
            result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[0]];
        } else {
            result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[2]];
        }
    } else if (distances[1] < distances[2]) {
        result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[1]];
    } else {
        result = scene->motionGroups[1].actorSlots[indexes.actorIndexes[2]];
    }
    return result;
}

/* At x >= -350 and outside the open -250 < z < 250 strip, checks for an unhit ground player
 * more than 100 units to the left. */
s32 fn_1_48D4(const HuVecF *position)
{
    M631SceneWork *sceneData = lbl_1_bss_8->data;
    s32 i;

    if (position->x < (-350.0f) ||
        (position->z > (-250.0f) && position->z < 250.0f)) {
        return 0;
    }
    for (i = 0; i < 3; i++) {
        if (sceneData->motionGroups[1].actorSlots[i]->hit == 0 &&
            sceneData->motionGroups[1].actorSlots[i]->position.x <
                position->x - 100.0f) {
            return 1;
        }
    }
    return 0;
}

/* The scene's first update builds the thirty-two-node free list used for contact responses. */
void fn_1_49B4(void)
{
    s32 nodeIndex;
    M631SceneWork *scene;
    M631SceneListNode *node;

    scene = lbl_1_bss_8->data;
    scene->sceneListFreeHead = scene->sceneListNodes;
    scene->sceneListHead = NULL;
    node = scene->sceneListNodes;
    nodeIndex = 0;
    while (nodeIndex < 31) {
        node->previous = NULL;
        node->next = node + 1;
        nodeIndex += 1;
        node = node + 1;
    }
    node->previous = NULL;
    node->next = NULL;
}

/* When an object is placed or lands, queues responses for nearby active contacts and the arena's
 * side edges. */
void fn_1_4A34(const HuVecF *position, f32 radius, M631MotionPairNode *stop)
{
    M631MotionPairNode *entry;
    HuVecF midpoint;
    HuVecF collisionResponse;
    HuVecF upperBoundaryResponse;
    HuVecF lowerBoundaryResponse;
    HuVecF *upperResponsePosition;
    HuVecF *lowerResponsePosition;
    f32 xDelta;
    f32 zDelta;
    f32 distanceSquared;
    f32 collisionLimitSquared;

    entry = fn_1_B004();
    while (entry != 0) {
        if (entry != stop) {
            switch (entry->state) {
            case 3:
            case 5:
            xDelta = position->x - entry->position->x;
            zDelta = position->z - entry->position->z;
            distanceSquared = xDelta * xDelta + zDelta * zDelta;
            collisionLimitSquared = (50.0f + radius + entry->radius) *
                                    (50.0f + radius + entry->radius);
            if (distanceSquared < collisionLimitSquared) {
                midpoint.x = (position->x + entry->position->x) * 0.5f;
                midpoint.y = 0.0f;
                midpoint.z = (position->z + entry->position->z) * 0.5f;
                collisionResponse = midpoint;
                fn_1_4DD4(&collisionResponse,
                          0.5f * sqrtf(distanceSquared));
            }
                break;
            default:
                break;
            }
        }
        entry = entry->next;
    }
    if (position->z + radius > 450.0f) {
        midpoint.x = position->x;
        midpoint.y = 0.0f;
        midpoint.z = 500.0f;
        upperBoundaryResponse = midpoint;
        upperResponsePosition = &upperBoundaryResponse;
        fn_1_4DD4(upperResponsePosition,
                  500.0f - position->z);
    }
    if (position->z - radius < (-450.0f)) {
        midpoint.x = position->x;
        midpoint.y = 0.0f;
        midpoint.z = (-500.0f);
        lowerBoundaryResponse = midpoint;
        lowerResponsePosition = &lowerBoundaryResponse;
        fn_1_4DD4(lowerResponsePosition,
                  500.0f + position->z);
    }
}

/* Allocates a contact-response node and registers its movement and completion callbacks. */
void fn_1_4DD4(const HuVecF *position, f32 radius)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    M631SceneListNode *node = scene->sceneListFreeHead;

    if (node) {
        scene->sceneListFreeHead = node->next;
        node->next = scene->sceneListHead;
        if (node->next) {
            node->next->previous = node;
        }
        scene->sceneListHead = node;
        node->position = *position;
        fn_1_AFF8(&node->callbackWork, radius, 7);
        fn_1_AF2C(&node->callbackWork, (u32)node,
                  (u32)fn_1_4F18, (u32)fn_1_4F6C, (u32)fn_1_5010,
                  &node->position);
    }
}

/* Returns a contact-response node to the scene's free list when it is explicitly removed. */
void fn_1_4EA4(M631SceneListNode *node) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        scene->sceneListHead = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    }
    node->previous = NULL;
    node->next = scene->sceneListFreeHead;
    scene->sceneListFreeHead = node;
}

/* The contact-response movement callback carries its position left while the conveyor is
 * running. */
void fn_1_4F18(M631SceneListNode *callbackNode)
{
    M631SceneListNode *node = callbackNode;
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    if (scene->conveyorActive != 0) {
        node->position.x -= 3.34f;
    }
}

/* The contact-response completion callback recycles its node after it leaves the left edge. */
s32 fn_1_4F6C(M631SceneListNode *callbackNode) {
    M631SceneListNode *node;
    M631SceneWork *scene;

    node = callbackNode;
    if (node->position.x < (-700.0f)) {
        scene = (M631SceneWork *) lbl_1_bss_8->data;
        if (node->previous) {
            node->previous->next = node->next;
        } else {
            scene->sceneListHead = node->next;
        }
        if (node->next) {
            node->next->previous = node->previous;
        }
        node->previous = NULL;
        node->next = scene->sceneListFreeHead;
        scene->sceneListFreeHead = node;
        return 1;
    }
    return 0;
}

/* The contact-response pair callback has no additional response. */
void fn_1_5010(void) { }

/* Actor behavior reads the first group-zero player's position for its movement decision. */
void fn_1_5014(HuVecF *destination)
{
    M631SceneWork *sceneValues = (M631SceneWork *)lbl_1_bss_8->data;
    *destination = sceneValues->motionGroups[0].actorSlots[0]->position;
}

/* Actor behavior increments the shared pursuit count when it commits to chasing a target. */
void fn_1_5054(void) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    scene->pursuitCount += 1;
}

/* Actor behavior decrements the shared pursuit count when a chase ends or is interrupted. */
void fn_1_5084(void) {
    M631SceneWork *scene = (M631SceneWork *) lbl_1_bss_8->data;
    scene->pursuitCount -= 1;
}

/* Returns the shared pursuit count to actor behavior when it chooses its next action. */
s16 fn_1_50B4(void)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    return scene->pursuitCount;
}

/* Actor behavior gets a horizontal vector of the requested length along the first group-one
 * player's heading. */
void fn_1_50DC(HuVecF *direction, f32 magnitude)
{
    M631SceneWork *scene = (M631SceneWork *)lbl_1_bss_8->data;
    HuVecF result;
    f32 angle = scene->motionGroups[1].actorSlots[0]->behaviorAngle;

    result.x = (f32)(magnitude * sin((3.141592653589793 * (f64)angle) / 180.0));
    result.y = 0.0f;
    result.z = (f32)(magnitude * cos((3.141592653589793 * (f64)angle) / 180.0));
    *direction = result;
}

/* Sound callers map the arena's X coordinate to a clamped stereo pan from 48 to 80. */
s32 fn_1_51D0(f32 positionX)
{
    f32 panFraction = (700.0f + positionX) / 1400.0f;
    s32 pan;
    if (panFraction > 1.0f) {
        panFraction = 1.0f;
    }
    if (panFraction < 0.0f) {
        panFraction = 0.0f;
    }
    pan = (s32)(48.0f + 32.0f * panFraction);
    return pan;
}
