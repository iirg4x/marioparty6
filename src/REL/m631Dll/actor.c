/* Updates player input, character motions, and computer-controlled behavior. */
#include "dolphin/math.h"
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
#include "datadir_enum.h"
#include "msm_se.h"

#define MSM_SE_M631_07 1845

typedef struct M631AvoidanceDampingTable {
    f32 byDifficulty[4]; /* Direction damping for the four computer difficulty levels. */
} M631AvoidanceDampingTable;

typedef struct M631ActorChanceTable {
    s32 byDifficulty[4]; /* Percentage thresholds for the four computer difficulty levels. */
} M631ActorChanceTable;

/* Character motion resources, shared through the Mario motion archive. */
u32 lbl_1_data_100[8] = {
    DATANUM(DATA_mariomot, 0), DATANUM(DATA_mariomot, 1),
    DATANUM(DATA_mariomot, 2), DATANUM(DATA_mariomot, 55),
    DATANUM(DATA_mariomot, 20), DATANUM(DATA_mariomot, 24),
    DATANUM(DATA_mariomot, 39), DATANUM(DATA_mariomot, 40)
};

extern s32 frandmod(s32 max);

typedef struct M631S32Table4 {
    s32 byDifficulty[4]; /* Integer behavior settings, indexed by computer difficulty. */
} M631S32Table4;

typedef struct M631F32Table4 {
    f32 byDifficulty[4]; /* Squared avoidance-strength thresholds by computer difficulty. */
} M631F32Table4;

typedef struct M631ChanceLevelTable {
    s32 byDifficulty[4]; /* Waiting percentages for the four computer difficulty levels. */
} M631ChanceLevelTable;

extern s32 frandmod(s32 limit);

extern f32 fn_1_2528(void);
extern void fn_1_AFF8(void *, f32, s32);
extern void fn_1_AF2C(void *, u32, u32, u32, u32, const HuVecF *);
extern void *fn_1_B0B8(s32 (*)(void *), void *);
extern M631AnimationNode *fn_1_31D8(M631AnimationQueue *);
extern void fn_1_32C8(M631AnimationNode *);
extern s32 fn_1_37FC(void *);
extern void fn_1_38E8(M631MotionPairNode *, M631MotionPairNode *);
extern void fn_1_228C(HU3D_MODELID *);
extern M631AnimationQueue *fn_1_2E9C(void);
extern void fn_1_3B44(M631ActorWork *);
extern void fn_1_3B7C(HuVecF *);
extern void fn_1_3BC4(void);
extern void fn_1_3BF4(M631ActorMotionState *);
extern s32 fn_1_51D0(f32);
extern void fn_1_5014(HuVecF *);
extern s32 fn_1_AAD8(void *);
extern M631ActorWork *fn_1_469C(const HuVecF *);
extern s16 fn_1_50B4(void);
extern void fn_1_5054(void);
extern void fn_1_5084(void);
extern void fn_1_50DC(HuVecF *, f32);

void fn_1_5284(OMOBJ *object);
void fn_1_55CC(OMOBJ *object);
void fn_1_56B0(M631ActorWork *actor);
void fn_1_5870(M631ActorWork *actor);
void fn_1_5AAC(M631ActorWork *actor, s32 teamIndex, s32 isGroundPlayer);
void fn_1_5BBC(M631ActorWork *work);
void fn_1_5E5C(M631ActorWork *actor);
void fn_1_5F20(M631ActorWork *work);
void fn_1_6090(M631ActorWork *work);
void fn_1_636C(M631ActorWork *work);
void fn_1_6694(M631ActorWork *work);
void fn_1_66DC(M631ActorWork *work);
void fn_1_6724(M631ActorWork *state);
s32 fn_1_674C(M631ActorWork *callbackActor);
void fn_1_6A14(M631MotionPairNode *callback,
               M631MotionPairNode *context, f32 distance);
void fn_1_6C2C(s32 callbackToken);
s32 fn_1_6C3C(void *callbackData);
void fn_1_6D80(M631MotionPairNode *motion, M631MotionPairNode *target,
               f32 distance);
void fn_1_738C(M631ActorWork *actor);
s32 fn_1_7448(void *callbackData);
s32 fn_1_749C(void *callbackData);
void fn_1_75D0(M631ActorWork *actor);
void fn_1_7738(M631ActorWork *work);
void fn_1_80F8(M631ActorWork *work);
void fn_1_954C(M631ActorWork *work);
void fn_1_9A70(M631ActorWork *motion);
void fn_1_A2A4(void *callbackData);
void fn_1_A2F4(M631MotionController *state);
s32 fn_1_A334(M631MotionController *state);
s32 fn_1_A418(M631MotionController *state);
void fn_1_A4FC(M631MotionPairNode *callback,
               M631MotionPairNode *context, f64 distance);
void fn_1_A730(M631MotionPairNode *callback,
               M631MotionPairNode *context, f32 distance);
void fn_1_A8F0(M631ActorWork *actor);
s32 fn_1_A920(void *callbackData);
void fn_1_AA78(M631ActorWork *actor);

/* Returns an angle in radians for the horizontal movement calculations. */
static inline float atan2f(float y, float x)
{
    return (float)atan2((double)y, (double)x);
}

/* The player's first object update loads character motions, initializes movement state and
 * registers the actor. */
void fn_1_5284(OMOBJ *object)
{
    M631ActorWork *work =
        (M631ActorWork *)object->data;
    s32 motionIndex;

    work->object = object;
    work->playerNo = (s16)object->work[0];
    work->charNo = GwPlayerConf[work->playerNo].charNo;
    work->groupSelector = GwPlayerConf[work->playerNo].grpNo;
    work->padNo = GwPlayerConf[work->playerNo].padNo;
    work->isComputer = GwPlayerConf[work->playerNo].type;
    work->difficulty = GwPlayerConf[work->playerNo].comDif;

    *object->mdlId = CharModelCreate(work->charNo, 2);
    work->characterModelId = *object->mdlId;
    for (motionIndex = 0; motionIndex < 8; motionIndex++) {
        HU3D_MOTIONID motion =
            CharMotionCreate(work->charNo, lbl_1_data_100[motionIndex]);

        (&work->idleMotionId)[motionIndex] = motion;
        object->mtnId[motionIndex] = motion;
    }
    CharMotionSet(work->charNo, object->mtnId[0]);
    CharModelAttrReset(work->charNo, HU3D_MOTATTR_PAUSE);
    CharModelAttrSet(work->charNo, HU3D_MOTATTR_LOOP);
    CharModelAttrReset(work->charNo, HU3D_ATTR_DISPOFF);
    CharMotionDataClose(work->charNo);
    CharModelVoiceFlagSet(work->charNo, 1);
    CharEffectLayerSet(6);
    CharModelStepSet(work->charNo, 0);
    Hu3DModelLayerSet(work->characterModelId, 3);

    work->dispatchEnabled = 0;
    work->hit = 0;
    work->releaseCooldown = 0;
    work->releaseFrames = 0;
    work->position.x = 0.0f;
    work->position.y = 0.0f;
    work->position.z = 0.0f;
    work->rotation.x = 0.0f;
    work->rotation.y = 0.0f;
    work->rotation.z = 0.0f;
    work->behaviorAngle = 0.0f;
    work->direction.x = 0.0f;
    work->direction.y = 0.0f;
    work->direction.z = 0.0f;
    work->accelerationFrames = 0;
    work->movementRequested = 0;
    work->movementStrength = 0.0f;
    work->releaseRequested = 0;
    work->randomOffset =
        360.0f *
        ((f32)frandmod(10001) / 10000.0f);
    Hu3DModelPosSetV(work->characterModelId, &work->position);
    omSetTra(work->object, work->position.x, work->position.y,
             work->position.z);
    Hu3DModelRotSetV(work->characterModelId, &work->rotation);
    omSetRot(work->object, work->rotation.x, work->rotation.y,
             work->rotation.z);
    fn_1_3B44((M631ActorWork *)work);
    object->objFunc = fn_1_55CC;
}

/* Each enabled player update selects human input or computer behavior, then advances its arena
 * role. */
void fn_1_55CC(OMOBJ *object)
{
    M631ActorWork *state = (M631ActorWork *)object->data;

    if (state->dispatchEnabled != 0) {
        if (state->isGroundPlayer != 0) {
            if (state->hit == 0) {
                if (state->isComputer != 0) {
                    fn_1_66DC(state);
                } else {
                    fn_1_636C(state);
                }
                fn_1_5F20(state);
                fn_1_5870(state);
                fn_1_3B7C(&state->position);
            }
        } else {
            if (state->isComputer != 0) {
                fn_1_6694(state);
            } else {
                fn_1_6090(state);
            }
            fn_1_5BBC(state);
            fn_1_5E5C(state);
            fn_1_56B0(state);
        }
    } else if (state->isGroundPlayer == 0) {
        fn_1_5E5C(state);
    }
}

/* The airborne player's update turns its model toward the requested heading by at most nine
 * degrees. */
void fn_1_56B0(M631ActorWork *actor) {
    f32 turnDegrees;

    while (actor->behaviorAngle > 180.0f) {
        actor->behaviorAngle -= 360.0f;
    }
    while (actor->behaviorAngle < (-180.0f)) {
        actor->behaviorAngle += 360.0f;
    }
    while (actor->rotation.y > 180.0f) {
        actor->rotation.y -= 360.0f;
    }
    while (actor->rotation.y < (-180.0f)) {
        actor->rotation.y += 360.0f;
    }
    turnDegrees = actor->behaviorAngle - actor->rotation.y;
    while (turnDegrees > 180.0f) {
        turnDegrees -= 360.0f;
    }
    while (turnDegrees < (-180.0f)) {
        turnDegrees += 360.0f;
    }
    if (turnDegrees > 0.0f) {
        if (turnDegrees > 9.0f) {
            turnDegrees = 9.0f;
        }
    } else if ((turnDegrees < 0.0f) && (turnDegrees < (-9.0f))) {
        turnDegrees = (-9.0f);
    }
    actor->rotation.y += turnDegrees;
}

/* The ground player's update turns its model toward the requested heading with separate human and
 * computer limits. */
void fn_1_5870(M631ActorWork *actor) {
    f32 turnDegrees;

    while (actor->behaviorAngle > 180.0f) {
        actor->behaviorAngle -= 360.0f;
    }
    while (actor->behaviorAngle < (-180.0f)) {
        actor->behaviorAngle += 360.0f;
    }
    while (actor->rotation.y > 180.0f) {
        actor->rotation.y -= 360.0f;
    }
    while (actor->rotation.y < (-180.0f)) {
        actor->rotation.y += 360.0f;
    }
    turnDegrees = actor->behaviorAngle - actor->rotation.y;
    while (turnDegrees > 180.0f) {
        turnDegrees -= 360.0f;
    }
    while (turnDegrees < (-180.0f)) {
        turnDegrees += 360.0f;
    }
    if (actor->isComputer != 0) {
        /* Positive turns through twelve degrees are kept; larger ones become six. */
        if (turnDegrees > 12.0f) {
            if (turnDegrees > 6.0f) {
                turnDegrees = 6.0f;
            }
        } else if ((turnDegrees < 0.0f) && (turnDegrees < (-6.0f))) {
            turnDegrees = (-6.0f);
        }
    } else if (turnDegrees > 0.0f) {
        if (turnDegrees > 12.0f) {
            turnDegrees = 12.0f;
        }
    } else if ((turnDegrees < 0.0f) && (turnDegrees < (-12.0f))) {
        turnDegrees = (-12.0f);
    }
    actor->rotation.y += turnDegrees;
}

/* The initialization phase assigns an actor's team slot and registers its airborne or ground-player
 * callbacks. */
void fn_1_5AAC(M631ActorWork *actor, s32 teamIndex, s32 isGroundPlayer)
{
    actor->teamIndex = (s16)teamIndex;
    actor->isGroundPlayer = (s16)isGroundPlayer;
    if (isGroundPlayer != 0) {
        fn_1_AFF8(&actor->callbackWork, 50.0f, 2);
        fn_1_AF2C(&actor->callbackWork, (u32)actor, (u32)fn_1_6C2C,
                  (u32)fn_1_6C3C, (u32)fn_1_6D80, &actor->position);
        Hu3DModelShadowSet(actor->characterModelId);
        fn_1_3BF4(&actor->hitEffect);
    } else {
        fn_1_AFF8(&actor->callbackWork, fn_1_2528(), 0);
        fn_1_AF2C(&actor->callbackWork, (u32)actor, (u32)fn_1_6724,
                  (u32)fn_1_674C, (u32)fn_1_6A14, &actor->position);
        fn_1_228C(&actor->setupWork.modelId);
        CharMotionSet(actor->charNo, actor->airborneMotionId);
        actor->setupWork.animationQueue = fn_1_2E9C();
    }
    if (actor->isComputer != 0) {
        fn_1_75D0(actor);
    }
}

/* Each airborne-player update applies horizontal inertia and advances the falling-object release
 * and cooldown. */
void fn_1_5BBC(M631ActorWork *work)
{
    M631AnimationNode *node;

    if (work->direction.x > 5.0f) {
        work->direction.x = 5.0f;
    }
    if (work->direction.x < (-5.0f)) {
        work->direction.x = (-5.0f);
    }
    if (work->direction.z > 5.0f) {
        work->direction.z = 5.0f;
    }
    if (work->direction.z < (-5.0f)) {
        work->direction.z = (-5.0f);
    }
    work->position.x += work->direction.x;
    work->position.z += work->direction.z;
    work->direction.x *= 0.99f;
    work->direction.z *= 0.99f;

    if (work->releaseCooldown == 0) {
        if (work->releaseFrames == 0) {
            if (work->releaseRequested != 0) {
                fn_1_AFF8(&work->callbackWork, fn_1_2528(), 1);
                work->releaseFrames = 1;
            }
        } else {
            work->releaseFrames += 1;
            if (work->releaseFrames >= 10) {
                Hu3DModelAttrSet(work->setupWork.releaseModelId, HU3D_ATTR_DISPOFF);
                node = fn_1_31D8(work->setupWork.animationQueue);
                if (node != 0) {
                    omVibrate(work->playerNo, 20, 4, 4);
                    node->position.x = work->position.x;
                    node->position.y = work->position.y;
                    node->position.z = work->position.z;
                    if (work->isNight != 0) {
                        fn_1_AFF8(&node->callbackWork, 25.0f, 4);
                    } else {
                        fn_1_AFF8(&node->callbackWork, 75.0f, 4);
                    }
                    fn_1_AF2C(&node->callbackWork, (u32)node,
                              (u32)fn_1_32C8, (u32)fn_1_37FC,
                              (u32)fn_1_38E8, &node->position);
                    node->animationState = 0;
                    work->releaseCooldown = 1;
                    fn_1_AFF8(&work->callbackWork, fn_1_2528(), 0);
                    work->releaseFrames = 0;
                }
            } else {
                Hu3DModelPosSetV(work->setupWork.releaseModelId, &work->position);
                Hu3DModelRotSetV(work->setupWork.releaseModelId, &work->rotation);
                Hu3DModelAttrReset(work->setupWork.releaseModelId, HU3D_ATTR_DISPOFF);
            }
        }
    } else {
        work->releaseCooldown += 1;
        if (work->releaseCooldown >= 70) {
            work->releaseCooldown = 0;
        }
    }
}

/* Each airborne-player update applies a sine bob around a height of 400 units, including before
 * play starts. */
void fn_1_5E5C(M631ActorWork *actor) {
    actor->randomOffset += 4.5f;
    while (actor->randomOffset > 180.0f) {
        actor->randomOffset -= 360.0f;
    }
    actor->position.y =
        (f32) (400.0 + (15.0 * sin((3.141592653589793 * (f64) actor->randomOffset) / 180.0)));
}

/* Each ground-player update selects an idle, slower or faster movement animation and clears the
 * movement request. */
void fn_1_5F20(M631ActorWork *work)
{
    if (work->movementRequested != 0) {
        if (work->movementStrength < 0.25f) {
            if (work->slowMoveMotionId != Hu3DMotionIDGet(work->characterModelId) &&
                work->slowMoveMotionId != Hu3DMotionShiftIDGet(work->characterModelId)) {
                CharMotionShiftSet(work->charNo, work->slowMoveMotionId,
                                   0.0f, 5.0f,
                                   HU3D_MOTATTR_LOOP);
            }
        } else if (work->fastMoveMotionId != Hu3DMotionIDGet(work->characterModelId) &&
                   work->fastMoveMotionId != Hu3DMotionShiftIDGet(work->characterModelId)) {
            CharMotionShiftSet(work->charNo, work->fastMoveMotionId,
                               0.0f, 5.0f,
                               HU3D_MOTATTR_LOOP);
        }
    } else if (work->idleMotionId != Hu3DMotionIDGet(work->characterModelId) &&
               work->idleMotionId != Hu3DMotionShiftIDGet(work->characterModelId)) {
        CharMotionShiftSet(work->charNo, work->idleMotionId,
                           0.0f, 5.0f,
                           HU3D_MOTATTR_LOOP);
    }
    work->movementRequested = 0;
}

/* The human airborne-player update accelerates along its facing direction and requests a release on
 * the A button. */
void fn_1_6090(M631ActorWork *work)
{
    f32 stickX = HuPadStkX[work->padNo];
    f32 stickY = -HuPadStkY[work->padNo];
    f32 stickMagnitude = (stickX * stickX) + (stickY * stickY);
    f32 angle;
    f32 atanAngle;
    f32 magnitude;

    if (stickMagnitude > 0.010000001f) {
        work->accelerationFrames += 1;
        if (work->accelerationFrames > 120) {
            work->accelerationFrames = 120;
        }
        magnitude = (f32)work->accelerationFrames / 120.0f;
        if (0.0f == stickY) {
            if (stickX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            atanAngle = (f32)atan2((f64)stickX, (f64)stickY);
            angle = atanAngle;
        }
        work->behaviorAngle =
            (f32)((f64)(180.0f * angle) / 3.141592653589793);
        work->direction.x = (f32)((f64)work->direction.x +
            ((f64)(5.0f * magnitude) *
             sin((3.141592653589793 * (f64)work->rotation.y) /
                 180.0)));
        work->direction.z = (f32)((f64)work->direction.z +
            ((f64)(5.0f * magnitude) *
             cos((3.141592653589793 * (f64)work->rotation.y) /
                 180.0)));
    } else {
        work->accelerationFrames = 0;
    }
    work->releaseRequested = 0;
    if ((HuPadBtnDown[work->padNo] & PAD_BUTTON_A) != 0) {
        work->releaseRequested = 1;
    }
}

/* The human ground-player update moves from the stick direction and requests an animation based on
 * stick strength. */
void fn_1_636C(M631ActorWork *work)
{
    f32 stickX = HuPadStkX[work->padNo];
    f32 stickY = -HuPadStkY[work->padNo];
    f32 stickMagnitude = (stickX * stickX) + (stickY * stickY);
    f32 adjustedAngle;
    f32 angle;
    f32 atanAngle;

    if (stickMagnitude > 0.010000001f) {
        work->movementStrength = sqrtf(stickMagnitude) / 56.0f;
        if (work->movementStrength > 1.0f) {
            work->movementStrength = 1.0f;
        }
        if (0.0f == stickY) {
            if (stickX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            atanAngle = (f32)atan2((f64)stickX, (f64)stickY);
            angle = atanAngle;
        }
        adjustedAngle = angle;
        work->position.x = (f32)((f64)work->position.x +
            ((f64)(10.0f * work->movementStrength) *
             sin((f64)adjustedAngle)));
        work->position.z = (f32)((f64)work->position.z +
            ((f64)(10.0f * work->movementStrength) *
             cos((f64)adjustedAngle)));
        work->behaviorAngle =
            (f32)((f64)(180.0f * adjustedAngle) /
                  3.141592653589793);
        work->movementRequested = 1;
    }
}

/* The airborne computer update selects the solo or three-player behavior for the arena variant. */
void fn_1_6694(M631ActorWork *work) {
    if (work->motionController.type == 0) {
        fn_1_7738(work);
        return;
    }
    fn_1_80F8(work);
}

/* The ground computer update selects the solo or three-player behavior for the arena variant. */
void fn_1_66DC(M631ActorWork *work) {
    if (work->motionController.type == 1) {
        fn_1_954C(work);
        return;
    }
    fn_1_9A70(work);
}

/* The airborne player's pre-movement callback clears the contact and ground-player proximity
 * indications. */
void fn_1_6724(M631ActorWork *state)
{
    M631ActorWork *callbackState = state;
    callbackState->motionController.state = 0;
    callbackState->motionController.alertState = 0;
}

/* The airborne player's completion callback pushes it away from arena edges, clamps its position
 * and places its model. */
s32 fn_1_674C(M631ActorWork *callbackActor) {
    M631ActorWork *actor;

    actor = callbackActor;
    if (actor->position.x < (-600.0f)) {
        actor->direction.x =
            (f32) (actor->direction.x + (0.125f * ((-600.0f) - actor->position.x)));
        actor->position.x = (f32) (actor->position.x + (0.125f * ((-600.0f) - actor->position.x)));
    }
    if (actor->position.x > 600.0f) {
        actor->direction.x = (f32) (actor->direction.x + (0.125f * (600.0f - actor->position.x)));
        actor->position.x = (f32) (actor->position.x + (0.125f * (600.0f - actor->position.x)));
    }
    if (actor->position.z < (-400.0f)) {
        actor->direction.z =
            (f32) (actor->direction.z + (0.125f * ((-400.0f) - actor->position.z)));
        actor->position.z = (f32) (actor->position.z + (0.125f * ((-400.0f) - actor->position.z)));
    }
    if (actor->position.z > 400.0f) {
        actor->direction.z = (f32) (actor->direction.z + (0.125f * (400.0f - actor->position.z)));
        actor->position.z = (f32) (actor->position.z + (0.125f * (400.0f - actor->position.z)));
    }
    if (actor->position.x < (-650.0f)) {
        actor->position.x = (f32) (-650.0f);
    }
    if (actor->position.x > 650.0f) {
        actor->position.x = (f32) 650.0f;
    }
    if (actor->position.z < (-450.0f)) {
        actor->position.z = (f32) (-450.0f);
    }
    if (actor->position.z > 450.0f) {
        actor->position.z = (f32) 450.0f;
    }
    Hu3DModelPosSetV(actor->setupWork.modelId, &actor->position);
    Hu3DModelRotSetV(actor->setupWork.modelId, &actor->rotation);
    return 0;
}

/* The airborne player's contact callback separates airborne players and notes proximity to a ground
 * player. */
void fn_1_6A14(M631MotionPairNode *callback,
               M631MotionPairNode *context, f32 distance)
{
    M631ActorWork *node = callback->owner;
    f32 xDelta;
    f32 zDelta;
    f32 atanAngle;
    f32 angle;
    f32 motionAngle;

    switch (context->state) {
    case 0:
        zDelta = context->position->z - callback->position->z;
        xDelta = context->position->x - callback->position->x;
        if (0.0f == zDelta) {
            if (xDelta >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            atanAngle = (f32)atan2((f64)xDelta, (f64)zDelta);
            angle = atanAngle;
        }
        motionAngle = angle;
        callback->deltaX = (f32)((f64)callback->deltaX -
                                 ((f64)(0.5f * distance) *
                                  sin((f64)motionAngle)));
        callback->deltaZ = (f32)((f64)callback->deltaZ -
                                 ((f64)(0.5f * distance) *
                                  cos((f64)motionAngle)));
        node->direction.x =
            (f32)((f64)node->direction.x -
                  ((f64)(0.2f * distance) *
                   sin((f64)motionAngle)));
        node->direction.z =
            (f32)((f64)node->direction.z -
                  ((f64)(0.2f * distance) *
                   cos((f64)motionAngle)));
        node->motionController.state = 1;
        return;
    case 2:
        node->motionController.alertState = 1;
        break;
    case 1:
    case 3:
    case 4:
    case 5:
        break;
    }
}

/* The ground player's registered pre-movement callback leaves its state unchanged. */
void fn_1_6C2C(s32 callbackToken) {
    s32 ignoredToken;

    ignoredToken = callbackToken;
}

/* The ground player's completion callback clamps and places the character, then starts elimination
 * when it was hit. */
s32 fn_1_6C3C(void *callbackData) {
    M631ActorWork *actor = (M631ActorWork *) callbackData;
    if (actor->position.x < (-625.0f)) actor->position.x = (-625.0f);
    if (actor->position.x > 625.0f) actor->position.x = 625.0f;
    if (actor->position.z < (-450.0f)) actor->position.z = (-450.0f);
    if (actor->position.z > 450.0f) actor->position.z = 450.0f;
    Hu3DModelPosSetV(actor->characterModelId, &actor->position);
    omSetTra(actor->object, actor->position.x, actor->position.y, actor->position.z);
    Hu3DModelRotSetV(actor->characterModelId, &actor->rotation);
    omSetRot(actor->object, actor->rotation.x, actor->rotation.y, actor->rotation.z);
    if (actor->hit != 0) {
        CharFXPlayPan(actor->charNo, CHARVOICEID(3), (s16) fn_1_51D0(actor->position.x));
        fn_1_738C(actor);
        return 1;
    }
    return 0;
}

/* The ground player's contact callback separates players or prepares an upward knockback when a
 * hazard hits. */
void fn_1_6D80(M631MotionPairNode *motion, M631MotionPairNode *target,
               f32 distance)
{
    M631ActorWork *owner = motion->owner;
    f32 angleForTrig;

    switch (target->state) {
    case 2: {
        f32 deltaX;
        f32 deltaZ;
        f32 angle;
        f32 computedAngle;

        deltaZ = target->position->z - motion->position->z;
        deltaX = target->position->x - motion->position->x;
        if (0.0f == deltaZ) {
            if (deltaX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            computedAngle = (f32)atan2((f64)deltaX, (f64)deltaZ);
            angle = computedAngle;
        }
        angleForTrig = angle;
        motion->deltaX = (f32)((f64)motion->deltaX -
                                    ((f64)(0.5f * distance) *
                                     sin((f64)angleForTrig)));
        motion->deltaZ = (f32)((f64)motion->deltaZ -
                                    ((f64)(0.5f * (f32)distance) *
                                     cos((f64)angleForTrig)));
        return;
    }
    case 3:
        CharModelVoiceFlagSet(owner->charNo, 0);
        {
            f32 deltaX;
            f32 deltaZ;
            f32 angle;
            f32 computedAngle;

            deltaZ = target->position->z - motion->position->z;
            deltaX = target->position->x - motion->position->x;
            if (0.0f == deltaZ) {
                if (deltaX >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                computedAngle = (f32)atan2((f64)deltaX, (f64)deltaZ);
                angle = computedAngle;
            }
            angleForTrig = angle;
            owner->hitEffect.position.x = (f32)((f64)owner->position.x +
                                             (distance * sin((f64)angleForTrig)));
            owner->hitEffect.position.y = 100.0f;
            owner->hitEffect.position.z = 50.0f + owner->position.z;
            owner->direction.x = (f32)(-distance * sin((f64)angleForTrig));
            owner->direction.y = 30.0f;
            owner->direction.z = (f32)(-distance * cos((f64)angleForTrig));
        }
        owner->hit = 1;
        return;
    case 4: {
        M631AnimationNode *targetOwner = target->owner;
        if (targetOwner->position.y <= 150.0f) {
            f32 deltaX;
            f32 deltaZ;
            f32 angle;
            f32 computedAngle;

            deltaZ = target->position->z - motion->position->z;
            deltaX = target->position->x - motion->position->x;
            if (0.0f == deltaZ) {
                if (deltaX >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                computedAngle = (f32)atan2((f64)deltaX, (f64)deltaZ);
                angle = computedAngle;
            }
            angleForTrig = angle;
            owner->hitEffect.position.x = (f32)((f64)owner->position.x +
                                             (distance * sin((f64)angleForTrig)));
            owner->hitEffect.position.y = 100.0f;
            owner->hitEffect.position.z = 50.0f + owner->position.z;
            owner->direction.x = (f32)(-distance * sin((f64)angleForTrig));
            owner->direction.y = 30.0f;
            owner->direction.z = (f32)(-distance * cos((f64)angleForTrig));
            if (owner->hit == 0) {
                HuAudFXPlayPan(MSM_SE_M631_07,
                               (s16)fn_1_51D0(owner->position.x));
            }
            owner->hit = 1;
        }
        break;
    }
    case 5:
        {
            f32 deltaX;
            f32 deltaZ;
            f32 angle;
            f32 computedAngle;

            deltaZ = target->position->z - motion->position->z;
            deltaX = target->position->x - motion->position->x;
            if (0.0f == deltaZ) {
                if (deltaX >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                computedAngle = (f32)atan2((f64)deltaX, (f64)deltaZ);
                angle = computedAngle;
            }
            angleForTrig = angle;
            owner->hitEffect.position.x = (f32)((f64)owner->position.x +
                                             (distance * sin((f64)angleForTrig)));
            owner->hitEffect.position.y = 100.0f;
            owner->hitEffect.position.z = 50.0f + owner->position.z;
            owner->direction.x = (f32)(-distance * sin((f64)angleForTrig));
            owner->direction.y = 30.0f;
            owner->direction.z = (f32)(-distance * cos((f64)angleForTrig));
        }
        if (owner->hit == 0) {
            HuAudFXPlayPan(MSM_SE_M631_07, (s16)fn_1_51D0(owner->position.x));
        }
        owner->hit = 1;
        break;
    }
}

/* On elimination, counts the ground player out and starts the rising character and contact-effect
 * jobs. */
void fn_1_738C(M631ActorWork *actor) {
    fn_1_3BC4();
    omVibrate(actor->playerNo, 20, 20, 0);
    fn_1_B0B8(fn_1_749C, actor);
    CharMotionShiftSet(actor->charNo, actor->eliminatedMotionId, 0.0f, 5.0f, HU3D_MOTATTR_LOOP);
    if (fn_1_B0B8(fn_1_7448, &actor->hitEffect)) {
        Hu3DModelAttrReset(actor->hitEffect.modelId, HU3D_MOTATTR_PAUSE);
        Hu3DModelAttrReset(actor->hitEffect.modelId, HU3D_ATTR_DISPOFF);
        Hu3DModelPosSetV(actor->hitEffect.modelId, &actor->hitEffect.position);
    }
}

/* The elimination effect job hides its model and finishes after the animation ends. */
s32 fn_1_7448(void *callbackData) {
    M631ActorMotionState *state = (M631ActorMotionState *) callbackData;
    if (Hu3DMotionEndCheck(state->modelId) != 0) {
        Hu3DModelAttrSet(state->modelId, HU3D_ATTR_DISPOFF);
        return 1;
    }
    return 0;
}

/* The eliminated player's job moves and spins the character upward, hiding it once it passes 1000
 * units. */
s32 fn_1_749C(void *callbackData) {
    M631ActorWork *actor;

    actor = callbackData;
    actor->position.x = (f32) (actor->position.x + actor->direction.x);
    actor->position.y = (f32) (actor->position.y + actor->direction.y);
    actor->position.z = (f32) (actor->position.z + actor->direction.z);
    actor->rotation.y -= 1.0f;
    while (actor->rotation.y < (-180.0f)) {
        actor->rotation.y += 360.0f;
    }
    Hu3DModelPosSetV(actor->characterModelId, &actor->position);
    omSetTra(actor->object, actor->position.x, actor->position.y, actor->position.z);
    Hu3DModelRotSetV(actor->characterModelId, &actor->rotation);
    omSetRot(actor->object, actor->rotation.x, actor->rotation.y, actor->rotation.z);
    /* The character keeps its upward velocity until this height cutoff. */
    if (actor->position.y > 1000.0f) {
        CharMotionSet(actor->charNo, actor->idleMotionId);
        CharModelAttrSet(actor->charNo, HU3D_ATTR_DISPOFF);
        CharModelAttrSet(actor->charNo, HU3D_MOTATTR_PAUSE);
        return 1;
    }
    return 0;
}

/* Actor setup registers computer avoidance callbacks and initializes the movement controller for
 * its arena role. */
void fn_1_75D0(M631ActorWork *actor)
{
    M631AvoidanceDampingTable dampingFactors = {{0.5f, 0.7f, 0.8f, 0.9f}};
    M631MotionController *node;
    node = &actor->motionController;
    if (actor->isGroundPlayer != 0) {
        node->sourcePosition = actor->position;
        fn_1_AFF8(node, 150.0f, 6);
        if (actor->motionController.type == 1) {
            fn_1_AF2C(node, (u32)node, (u32)fn_1_A2F4, (u32)fn_1_A418,
                      (u32)fn_1_A730, &node->sourcePosition);
            actor->motionController.scale =
                dampingFactors.byDifficulty[actor->difficulty];
        } else {
            fn_1_AF2C(node, (u32)node, (u32)fn_1_A2A4, (u32)fn_1_A334,
                      (u32)fn_1_A4FC, &node->sourcePosition);
        }
        node->state = 0;
        node->stateFrames = 0;
        node->finished = 0;
        node->avoidanceDirection.x = 0.0f;
        node->avoidanceDirection.y = 0.0f;
        node->avoidanceDirection.z = 0.0f;
        node->behaviorState = 0;
    } else {
        node->behaviorState = 0;
    }
}

/* The day solo airborne computer chooses waits, target pursuit and cross-arena sweeps while timing
 * releases. */
void fn_1_7738(M631ActorWork *work)
{
    M631ActorChanceTable waitProbability = {{30, 25, 10, 0}};
    M631ActorChanceTable sweepProbability = {{50, 50, 50, 50}};
    M631ActorWork *target;
    f32 xDelta;
    f32 zDelta;
    f32 distanceSquared;
    f32 step;

    work->releaseRequested = 0;
    switch (work->motionController.behaviorState) {
    case 0: {
        HuVecF position;
        const HuVecF *searchPosition;
        s32 chance = frandmod(100);

        if (chance < waitProbability.byDifficulty[work->difficulty]) {
            work->motionController.behaviorState = 10;
            return;
        }
        position = work->position;
        searchPosition = (const HuVecF *)&position;
        target = fn_1_469C(searchPosition);
        if (target == 0) return;
        xDelta = target->position.x - work->position.x;
        zDelta = target->position.z - work->position.z;
        distanceSquared = (xDelta * xDelta) + (zDelta * zDelta);
        if (distanceSquared < 90000.0f) {
            work->motionController.behaviorState = 20;
            work->target = target;
            return;
        }
        chance = frandmod(100);
        if (work->position.x > 350.0f &&
            chance < sweepProbability.byDifficulty[work->difficulty]) {
            work->motionController.behaviorState = 30;
            return;
        }
        work->motionController.behaviorState = 40;
        return;
    }
    case 10:
        work->motionController.stateFrames = 0;
        work->motionController.behaviorState++;
        /* fall through */
    case 11:
        work->motionController.stateFrames++;
        if (work->motionController.stateFrames >= 30) {
            work->motionController.behaviorState = 0;
        }
        if (work->motionController.alertState != 0) {
            work->releaseRequested = 1;
            return;
        }
        break;
    case 20: {
        f32 angle;
        if (work->target->hit != 0) {
            work->motionController.behaviorState = 0;
            return;
        }
        xDelta = work->target->position.x - work->position.x;
        zDelta = work->target->position.z - work->position.z;
        distanceSquared = (xDelta * xDelta) + (zDelta * zDelta);
        if (distanceSquared > 160000.0f) {
            work->motionController.behaviorState = 0;
        } else if (distanceSquared < 2500.0f) {
            work->accelerationFrames = 0;
            work->motionController.behaviorState = 0;
        } else {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (zDelta == 0.0f) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
        }
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            work->releaseRequested = 1;
            return;
        }
        break;
    }
    case 30:
        if (work->position.z < 0.0f) {
            work->motionController.destination.x = work->position.x;
            work->motionController.destination.z = 400.0f;
        } else {
            work->motionController.destination.x = work->position.x;
            work->motionController.destination.z = -400.0f;
        }
        work->motionController.behaviorState++;
        /* fall through */
    case 31: {
        f32 angle;
        xDelta = work->motionController.destination.x - work->position.x;
        zDelta = work->motionController.destination.z - work->position.z;
        distanceSquared = (xDelta * xDelta) + (zDelta * zDelta);

        if (distanceSquared > 2500.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (zDelta == 0.0f) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
        } else {
            work->accelerationFrames = 0;
            work->motionController.behaviorState = 0;
        }
        if (work->releaseCooldown == 0 && work->releaseFrames == 0) {
            work->releaseRequested = 1;
            return;
        }
        break;
    }
    case 40: {
        HuVecF position;
        const HuVecF *searchPosition;

        position = work->position;
        searchPosition = (const HuVecF *)&position;
        target = fn_1_469C(searchPosition);
        if (target != 0) {
            work->target = target;
            work->motionController.destination = target->position;
            work->motionController.destination.x += 50.0f;
            if (work->motionController.destination.x > 600.0f) {
                work->motionController.destination.x = 600.0f;
            }
            work->motionController.behaviorState++;
        } else {
            work->motionController.behaviorState = 0;
            return;
        }
        /* fall through */
    }
    case 41: {
        f32 angle;
        if (work->target->hit != 0) {
            work->motionController.behaviorState = 0;
            return;
        }
        xDelta = work->motionController.destination.x - work->position.x;
        zDelta = work->motionController.destination.z - work->position.z;
        distanceSquared = (xDelta * xDelta) + (zDelta * zDelta);
        if (distanceSquared > 10000.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (zDelta == 0.0f) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
        } else {
            work->accelerationFrames = 0;
            work->motionController.behaviorState = 0;
        }
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            work->releaseRequested = 1;
        }
    }
    }
}

/* A night airborne computer chooses pursuit, approaches and repositioning, sharing the pursuit
 * count with its teammates. */
void fn_1_80F8(M631ActorWork *work)
{
    M631ActorWork *target;
    M631ActorChanceTable waitThreshold = {{50, 45, 10, 0}};
    M631ActorChanceTable chaseThreshold = {{60, 70, 50, 70}};
    M631ActorChanceTable approachThreshold = {{95, 95, 80, 95}};
    M631ActorChanceTable repositionProbability = {{10, 20, 50, 60}};
    M631ActorChanceTable releaseProbability = {{30, 50, 90, 100}};
    f32 xDelta;
    f32 zDelta;
    f32 step;
    f32 distanceOrRadius;
    f32 randomAngle;
    HuVecF headingTarget;

    work->releaseRequested = 0;
    switch (work->motionController.behaviorState) {
    case 0: {
        s32 chance = frandmod(100);
        s16 behaviorChoice;

        if (chance < waitThreshold.byDifficulty[work->difficulty]) {
            work->motionController.behaviorState = 10;
            return;
        }
        if (chance < chaseThreshold.byDifficulty[work->difficulty]) {
            if (work->difficulty < 2) {
                work->motionController.behaviorState = 20;
                return;
            }
            behaviorChoice = fn_1_50B4();
            switch (behaviorChoice) {
            case 0:
                work->motionController.behaviorState = 20;
                return;
            case 1:
                if (frandmod(100) < 20) {
                    work->motionController.behaviorState = 20;
                    return;
                }
                work->motionController.behaviorState = 50;
                return;
            case 2:
                work->motionController.behaviorState = 50;
                return;
            default:
                work->motionController.behaviorState = 30;
                return;
            }
        }
        if (chance < approachThreshold.byDifficulty[work->difficulty]) {
            work->motionController.behaviorState = 30;
        } else {
            work->motionController.behaviorState = 40;
        }
        break;
    }
    case 10:
        work->motionController.stateFrames = 0;
        work->motionController.behaviorState++;
        /* fall through */
    case 11:
        work->motionController.stateFrames++;
        if (work->motionController.stateFrames >= 30) {
            work->motionController.behaviorState = 0;
            return;
        }
        if (work->motionController.state != 0) {
            work->motionController.behaviorState = 0;
        }
        if (work->motionController.alertState != 0 &&
            frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
            work->releaseRequested = 1;
            return;
        }
        break;
    case 20: {
        HuVecF targetSearchPosition;
        const HuVecF *searchPosition;

        targetSearchPosition = work->position;
        searchPosition = (const HuVecF *)&targetSearchPosition;
        target = fn_1_469C(searchPosition);
        if (target == NULL) {
            work->motionController.behaviorState = 0;
            return;
        }
        fn_1_5054();
        work->target = target;
        work->motionController.behaviorState++;
        /* fall through */
    }
    case 21: {
        f32 angle;
        if (work->target->hit != 0) {
            fn_1_5084();
            work->motionController.behaviorState = 0;
            return;
        }
        if (work->motionController.state != 0) {
            fn_1_5084();
            work->motionController.behaviorState = 0;
        }
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            fn_1_5084();
            if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
        }
        xDelta = work->target->position.x - work->position.x;
        zDelta = work->target->position.z - work->position.z;
        distanceOrRadius = (xDelta * xDelta) + (zDelta * zDelta);
        if (distanceOrRadius > 2500.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (0.0f == zDelta) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            return;
        }
        work->accelerationFrames = 0;
        if (work->releaseCooldown == 0 && work->releaseFrames == 0) {
            fn_1_5084();
            if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
            return;
        }
        break;
    }
    case 30: {
        HuVecF approachSearchPosition;
        const HuVecF *searchPosition;

        approachSearchPosition = work->position;
        searchPosition = (const HuVecF *)&approachSearchPosition;
        target = fn_1_469C(searchPosition);
        if (target == NULL) {
            work->motionController.behaviorState = 0;
            return;
        }
        work->target = target;
        work->motionController.behaviorState++;
        /* fall through */
    }
    case 31: {
        f32 angle;
        if (work->target->hit != 0) {
            work->motionController.behaviorState = 0;
            return;
        }
        if (work->motionController.state != 0) {
            work->motionController.behaviorState = 0;
        }
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
        }
        xDelta = work->target->position.x - work->position.x;
        zDelta = work->target->position.z - work->position.z;
        distanceOrRadius = (xDelta * xDelta) + (zDelta * zDelta);

        if (distanceOrRadius > 40000.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (0.0f == zDelta) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            return;
        } else {
            work->accelerationFrames = 0;
        }
        if (work->releaseCooldown == 0 && work->releaseFrames == 0) {
            work->releaseRequested = 1;
            if (work->motionController.behaviorState != 0) {
                if (frandmod(100) < repositionProbability.byDifficulty[work->difficulty]) {
                    work->motionController.behaviorState++;
                    return;
                }
                work->motionController.behaviorState = 0;
                return;
            }
        }
        break;
    }
    case 32: {
        /* This temporary holds a radius here and a squared distance during travel. */
        distanceOrRadius = 150.0f;
        randomAngle = (f32)(2.0 *
            (3.141592653589793 *
             ((f32)frandmod(10001) / 10000.0f)));

        work->motionController.destination.x = (f32)((f64)work->position.x +
                                     (distanceOrRadius * sin((f64)randomAngle)));
        work->motionController.destination.z = (f32)((f64)work->position.z +
                                     (distanceOrRadius * cos((f64)randomAngle)));
        if (work->motionController.destination.x < -600.0f)
            work->motionController.destination.x = -600.0f;
        if (work->motionController.destination.x > 600.0f)
            work->motionController.destination.x = 600.0f;
        if (work->motionController.destination.z < -400.0f)
            work->motionController.destination.z = -400.0f;
        if (work->motionController.destination.z > 400.0f)
            work->motionController.destination.z = 400.0f;
        work->motionController.behaviorState++;
        /* fall through */
    }
    case 33: {
        f32 angle;

        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
        }
        xDelta = work->motionController.destination.x - work->position.x;
        zDelta = work->motionController.destination.z - work->position.z;
        distanceOrRadius = xDelta * xDelta + zDelta * zDelta;
        if (distanceOrRadius > 2500.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) work->accelerationFrames = 120;
            step = (f32)work->accelerationFrames / 120.0f;
            if (0.0f == zDelta) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            return;
        }
        work->accelerationFrames = 0;
        if (work->releaseCooldown == 0 && work->releaseFrames == 0) {
            work->releaseRequested = 1;
                if (frandmod(100) < repositionProbability.byDifficulty[work->difficulty]) {
                work->motionController.behaviorState = 32;
                return;
            }
            work->motionController.behaviorState = 0;
            return;
        }
        break;
    }
    case 40: {
        HuVecF orbitSearchPosition;
        const HuVecF *searchPosition;

        orbitSearchPosition = work->position;
        searchPosition = (const HuVecF *)&orbitSearchPosition;
        target = fn_1_469C(searchPosition);
        if (target == NULL) {
            work->motionController.behaviorState = 0;
            return;
        }
        {
            /* Choose a destination on a circle around the target player. */
            distanceOrRadius = 300.0f;
            randomAngle = (f32)(2.0 *
                (3.141592653589793 *
                 ((f32)frandmod(10001) / 10000.0f)));
            work->motionController.destination.x = (f32)((f64)target->position.x +
                ((f64)distanceOrRadius * sin((f64)randomAngle)));
            work->motionController.destination.z = (f32)((f64)target->position.z +
                ((f64)distanceOrRadius * cos((f64)randomAngle)));
        }
        if (work->motionController.destination.x < -600.0f)
            work->motionController.destination.x = -600.0f;
        if (work->motionController.destination.x > 600.0f)
            work->motionController.destination.x = 600.0f;
        if (work->motionController.destination.z < -400.0f)
            work->motionController.destination.z = -400.0f;
        if (work->motionController.destination.z > 400.0f)
            work->motionController.destination.z = 400.0f;
        work->motionController.behaviorState++;
        /* fall through */
    }
    case 41: {
        f32 angle;
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
                if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
        }
        xDelta = work->motionController.destination.x - work->position.x;
        zDelta = work->motionController.destination.z - work->position.z;
        distanceOrRadius = (xDelta * xDelta) + (zDelta * zDelta);
        if (distanceOrRadius > 10000.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) {
                work->accelerationFrames = 120;
            }
            step = (f32)work->accelerationFrames / 120.0f;
            if (0.0f == zDelta) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
        } else {
            work->accelerationFrames = 0;
            work->motionController.behaviorState = 0;
        }
        if (work->releaseCooldown == 0 && work->releaseFrames == 0 &&
            frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
            work->releaseRequested = 1;
            return;
        }
        break;
    }
    case 50:
        work->motionController.behaviorState++;
        /* fall through */
    case 51: {
        f32 angle;

        /* This is a point measured from the arena origin along the target's heading. */
        fn_1_50DC(&headingTarget, 300.0f);
        if (headingTarget.x < -600.0f) headingTarget.x = -600.0f;
        if (headingTarget.x > 600.0f) headingTarget.x = 600.0f;
        if (headingTarget.z < -400.0f) headingTarget.z = -400.0f;
        if (headingTarget.z > 400.0f) headingTarget.z = 400.0f;
        if (work->motionController.alertState != 0 && work->releaseCooldown == 0 &&
            work->releaseFrames == 0) {
            if (frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
                work->releaseRequested = 1;
            }
            work->motionController.behaviorState = 0;
        }
        xDelta = headingTarget.x - work->position.x;
        zDelta = headingTarget.z - work->position.z;
        distanceOrRadius = xDelta * xDelta + zDelta * zDelta;
        if (distanceOrRadius > 10000.0f) {
            work->accelerationFrames++;
            if (work->accelerationFrames > 120) work->accelerationFrames = 120;
            step = (f32)work->accelerationFrames / 120.0f;
            if (0.0f == zDelta) {
                if (xDelta >= 0.0f) {
                    angle = 1.5707964f;
                } else {
                    angle = (-1.5707964f);
                }
            } else {
                angle = atan2f(xDelta, zDelta);
            }
            work->behaviorAngle = (f32)((f64)(180.0f * angle) /
                                         3.141592653589793);
            work->direction.x = (f32)((f64)work->direction.x +
                (f64)(5.0f * step) *
                    sin((3.141592653589793 * (f64)work->rotation.y) / 180.0));
            work->direction.z = (f32)((f64)work->direction.z +
                (f64)(5.0f * step) *
                    cos((3.141592653589793 * (f64)work->rotation.y) / 180.0));
        } else {
            work->accelerationFrames = 0;
            work->motionController.behaviorState = 0;
        }
        if (work->releaseCooldown == 0 && work->releaseFrames == 0 &&
            frandmod(100) < releaseProbability.byDifficulty[work->difficulty]) {
            work->releaseRequested = 1;
        }
        break;
    }
    }
}

/* The night solo ground computer follows its avoidance direction or alternates random movement and
 * waits. */
void fn_1_954C(M631ActorWork *work)
{
    M631S32Table4 stopProbability = {{75, 50, 25, 0}};
    M631S32Table4 turnProbability = {{20, 50, 100, 100}};
    M631S32Table4 turnDuration = {{60, 60, 30, 30}};
    M631F32Table4 speedThreshold = {{10000.0f, 10000.0f, 0.01f, 0.01f}};
    f32 velocityX;
    f32 velocityZ;
    f32 angle;
    f32 angleForTrig;
    f32 speedSquared;
    f32 headingAngle;
    f32 computedAngle;
    if (work->motionController.avoidanceDirection.x > 1000.0f) {
        work->motionController.avoidanceDirection.x = 1000.0f;
    }
    if (work->motionController.avoidanceDirection.x < (-1000.0f)) {
        work->motionController.avoidanceDirection.x = (-1000.0f);
    }
    if (work->motionController.avoidanceDirection.z > 1000.0f) {
        work->motionController.avoidanceDirection.z = 1000.0f;
    }
    if (work->motionController.avoidanceDirection.z < (-1000.0f)) {
        work->motionController.avoidanceDirection.z = (-1000.0f);
    }

    velocityX = work->motionController.avoidanceDirection.x;
    velocityZ = work->motionController.avoidanceDirection.z;
    work->motionController.previousAngle = work->behaviorAngle;
    speedSquared = velocityX * velocityX + velocityZ * velocityZ;
    if (speedSquared >
            speedThreshold.byDifficulty[work->difficulty] &&
        frandmod(100) < turnProbability.byDifficulty[work->difficulty]) {
        work->movementStrength = 1.0f;
        if (0.0f == velocityZ) {
            if (velocityX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            computedAngle = (f32)atan2((f64)velocityX, (f64)velocityZ);
            angle = computedAngle;
        }
        angleForTrig = angle;
        work->position.x = (f32)((f64)work->position.x +
                                 (f64)(10.0f * work->movementStrength) *
                                     sin((f64)angleForTrig));
        work->position.z = (f32)((f64)work->position.z +
                                 (f64)(10.0f * work->movementStrength) *
                                     cos((f64)angleForTrig));
        headingAngle = (f32)((f64)(180.0f * angleForTrig) /
                             3.141592653589793);
        work->behaviorAngle = 0.5f *
                        (headingAngle + work->motionController.previousAngle);
        work->movementRequested = 1;
        work->motionController.behaviorState = 0;
    } else {
        switch (work->motionController.behaviorState) {
        case 0:
            work->motionController.stateFrames = 0;
            if (frandmod(100) < stopProbability.byDifficulty[work->difficulty]) {
                work->movementStrength = 0.0f;
                work->motionController.behaviorState = 2;
                break;
            } else {
                work->movementStrength = 1.0f;
                work->behaviorAngle =
                    (10.0f * (f32)frandmod(36)) - 180.0f;
                work->motionController.behaviorState = 1;
                /* fall through */
            }
        case 1:
            work->motionController.stateFrames++;
            if (work->motionController.stateFrames >= 30) {
                work->motionController.behaviorState = 0;
            }
            angleForTrig = (f32)((3.141592653589793 * (f64)work->behaviorAngle) /
                                 180.0);
            work->position.x = (f32)((f64)work->position.x +
                                     (f64)(10.0f *
                                           work->movementStrength) *
                                         sin((f64)angleForTrig));
            work->position.z = (f32)((f64)work->position.z +
                                     (f64)(10.0f *
                                           work->movementStrength) *
                                         cos((f64)angleForTrig));
            work->movementRequested = 1;
            break;
        case 2:
            work->motionController.stateFrames++;
            if (work->motionController.stateFrames >=
                turnDuration.byDifficulty[work->difficulty]) {
                work->motionController.behaviorState = 0;
            }
            break;
        }
    }
    work->motionController.sourcePosition = work->position;
}

/* A day ground computer follows avoidance or chooses short moves. In a shared quadrant, its
 * fallback heading uses the negative airborne position measured from the arena origin. */
void fn_1_9A70(M631ActorWork *motion)
{
    M631ChanceLevelTable waitProbability = {{50, 30, 10, 0}};
    f32 headingX;
    f32 headingZ;
    f32 speedSquared;
    f32 angle;
    f32 angleForTrig;
    f32 headingAngle;

    if (motion->motionController.avoidanceDirection.x > 1000.0f) {
        motion->motionController.avoidanceDirection.x = 1000.0f;
    }
    if (motion->motionController.avoidanceDirection.x < (-1000.0f)) {
        motion->motionController.avoidanceDirection.x = (-1000.0f);
    }
    if (motion->motionController.avoidanceDirection.z > 1000.0f) {
        motion->motionController.avoidanceDirection.z = 1000.0f;
    }
    if (motion->motionController.avoidanceDirection.z < (-1000.0f)) {
        motion->motionController.avoidanceDirection.z = (-1000.0f);
    }

    headingX = motion->motionController.avoidanceDirection.x;
    headingZ = motion->motionController.avoidanceDirection.z;
    motion->motionController.previousAngle = motion->behaviorAngle;
    speedSquared = (headingX * headingX) + (headingZ * headingZ);
    if (speedSquared > 0.01f) {
        motion->movementStrength = 1.0f;
        if (0.0f == headingZ) {
            if (headingX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            angle = atan2f(headingX, headingZ);
        }
        angleForTrig = angle;
        motion->position.x = (f32)((f64)motion->position.x +
                                  ((f64)(10.0f * motion->movementStrength) *
                                   sin((f64)angleForTrig)));
        motion->position.z = (f32)((f64)motion->position.z +
                                  ((f64)(10.0f * motion->movementStrength) *
                                   cos((f64)angleForTrig)));
        headingAngle = (f32)((f64)(180.0f * angleForTrig) /
                             3.141592653589793);
        motion->behaviorAngle =
            0.5f * (headingAngle + motion->motionController.previousAngle);
        motion->movementRequested = 1;
        motion->motionController.behaviorState = 0;
    } else {
        switch (motion->motionController.behaviorState) {
        case 0:
        motion->motionController.stateFrames = 0;
        motion->movementStrength = 1.0f;
        if (frandmod(100) < waitProbability.byDifficulty[motion->difficulty]) {
            motion->motionController.behaviorState = 2;
            break;
        } else {
            if (motion->position.x < (-350.0f)) {
                if (frandmod(15) == 0) {
                    motion->behaviorAngle =
                        45.0f * (f32)(frandmod(3) + 5);
                } else {
                    motion->behaviorAngle =
                        45.0f * (f32)frandmod(5);
                }
            } else if (frandmod(5) == 0) {
                motion->behaviorAngle =
                    45.0f * (f32)(frandmod(3) + 5);
            } else {
                motion->behaviorAngle =
                    45.0f * (f32)frandmod(5);
            }
            motion->motionController.behaviorState = 1;
        }
        /* fall through */
        case 1: {
        HuVecF throwerPosition;
        f32 rightSideEscapeX;
        f32 rightSideEscapeZ;
        f32 leftSideEscapeX;
        f32 leftSideEscapeZ;
        f32 rightSideEscapeAngle;
        f32 leftSideEscapeAngle;
        motion->motionController.stateFrames++;
        if (motion->motionController.stateFrames >= 30) {
            motion->motionController.behaviorState = 0;
        }
        fn_1_5014(&throwerPosition);
        if ((throwerPosition.x > 0.0f) &&
            (motion->position.x > 0.0f)) {
            if (((throwerPosition.z > 0.0f) &&
                 (motion->position.z > 0.0f)) ||
                ((throwerPosition.z < 0.0f) &&
                 (motion->position.z < 0.0f))) {
                rightSideEscapeZ = -throwerPosition.z;
                rightSideEscapeX = -throwerPosition.x;
                if (0.0f == rightSideEscapeZ) {
                    if (rightSideEscapeX >= 0.0f) {
                        rightSideEscapeAngle = 1.5707964f;
                    } else {
                        rightSideEscapeAngle = (-1.5707964f);
                    }
                } else {
                    rightSideEscapeAngle =
                        atan2f(rightSideEscapeX, rightSideEscapeZ);
                }
                motion->behaviorAngle =
                    (f32)((f64)(180.0f * rightSideEscapeAngle) /
                          3.141592653589793);
            }
        } else if ((throwerPosition.x < 0.0f) &&
                   (motion->position.x < 0.0f) &&
                   (((throwerPosition.z > 0.0f) &&
                     (motion->position.z > 0.0f)) ||
                    ((throwerPosition.z < 0.0f) &&
                     (motion->position.z < 0.0f)))) {
            leftSideEscapeZ = -throwerPosition.z;
            leftSideEscapeX = -throwerPosition.x;
            if (0.0f == leftSideEscapeZ) {
                if (leftSideEscapeX >= 0.0f) {
                    leftSideEscapeAngle = 1.5707964f;
                } else {
                    leftSideEscapeAngle = (-1.5707964f);
                }
            } else {
                leftSideEscapeAngle =
                    atan2f(leftSideEscapeX, leftSideEscapeZ);
            }
            motion->behaviorAngle =
                (f32)((f64)(180.0f * leftSideEscapeAngle) /
                      3.141592653589793);
        }
        angleForTrig = (f32)((3.141592653589793 *
                              (f64)motion->behaviorAngle) /
                             180.0);
        motion->position.x = (f32)((f64)motion->position.x +
                                  (f64)(10.0f * motion->movementStrength) *
                                  sin((f64)angleForTrig));
        motion->position.z = (f32)((f64)motion->position.z +
                                  (f64)(10.0f * motion->movementStrength) *
                                  cos((f64)angleForTrig));
        motion->movementRequested = 1;
            break;
        }
        case 2:
        motion->motionController.stateFrames++;
        if (motion->motionController.stateFrames >= 30) {
            motion->motionController.behaviorState = 0;
        }
            break;
        }
    }
    motion->motionController.sourcePosition = motion->position;
}

/* Before computer avoidance contacts are processed, halves the accumulated direction and clears the
 * contact state. */
void fn_1_A2A4(void *callbackData)
{
    M631MotionController *node = (M631MotionController *) callbackData;

    node->avoidanceDirection.x *= 0.5f;
    node->avoidanceDirection.z *= 0.5f;
    node->state = 0;
}

/* Before solo computer avoidance contacts are processed, damps the accumulated direction according
 * to difficulty. */
void fn_1_A2F4(M631MotionController *state)
{
    M631MotionController *motion = state;
    motion->avoidanceDirection.x *= motion->scale;
    motion->avoidanceDirection.z *= motion->scale;
    motion->state = 0;
}

/* The computer avoidance completion callback steers its accumulated direction inward near the arena
 * edges. */
s32 fn_1_A334(M631MotionController *state)
{
    M631MotionController *motion = state;

    if (motion->sourcePosition.x < (-600.0f)) {
        motion->avoidanceDirection.x += 100.0f;
    } else if (motion->sourcePosition.x > 600.0f) {
        motion->avoidanceDirection.x -= 100.0f;
    }
    if (motion->sourcePosition.z < (-400.0f)) {
        motion->avoidanceDirection.z += 100.0f;
    } else if (motion->sourcePosition.z > 400.0f) {
        motion->avoidanceDirection.z -= 100.0f;
    }
    return 0;
}

/* The solo computer avoidance completion callback steers its accumulated direction inward near the
 * arena edges. */
s32 fn_1_A418(M631MotionController *state)
{
    M631MotionController *motion = state;

    if (motion->sourcePosition.x < (-600.0f)) {
        motion->avoidanceDirection.x += 100.0f;
    } else if (motion->sourcePosition.x > 600.0f) {
        motion->avoidanceDirection.x -= 100.0f;
    }
    if (motion->sourcePosition.z < (-400.0f)) {
        motion->avoidanceDirection.z += 100.0f;
    } else if (motion->sourcePosition.z > 400.0f) {
        motion->avoidanceDirection.z -= 100.0f;
    }
    return 0;
}

/* The three-player computer contact callback adds avoidance, turning sideways while the player
 * is left of the hazard's right-side margin. */
void fn_1_A4FC(M631MotionPairNode *callback,
               M631MotionPairNode *context, f64 distance)
{
    M631MotionController *node = callback->owner;
    f32 xDeltaInitial;
    f32 zDeltaInitial;
    f32 xDelta;
    f32 zDelta;
    f32 atanAngle;
    f32 angle;
    f32 adjustedAngle;

    switch (context->state) {
    case 3:
    case 4:
    case 5:
    case 7:
        xDeltaInitial = context->position->x - callback->position->x;
        zDeltaInitial = context->position->z - callback->position->z;
        zDelta = context->position->z - callback->position->z;
        xDelta = context->position->x - callback->position->x;
        if (0.0f == zDelta) {
            if (xDelta >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            atanAngle = (f32)atan2((f64)xDelta, (f64)zDelta);
            angle = atanAngle;
        }
        adjustedAngle = angle;
        /* Turn sideways while left of the hazard's right-side margin. */
        if (xDeltaInitial > (-25.0f) - context->radius) {
            if (zDeltaInitial < 0.0f) {
                adjustedAngle += 1.5707963267948966;
            } else {
                adjustedAngle -= 1.5707963267948966;
            }
        }
        node->avoidanceDirection.x = (f32)((f64)node->avoidanceDirection.x -
                                 (distance * sin((f64)adjustedAngle)));
        node->avoidanceDirection.z = (f32)((f64)node->avoidanceDirection.z -
                                 (distance * cos((f64)adjustedAngle)));
        break;
    }
}

/* The solo computer contact callback adds a direction away from releasing airborne players and
 * hazards. */
void fn_1_A730(M631MotionPairNode *callback,
               M631MotionPairNode *context, f32 distance)
{
    f32 angle;
    f32 angleForTrig;
    f32 deltaX;
    f32 deltaZ;
    f32 initialDeltaX;
    f32 initialDeltaZ;
    f32 computedAngle;
    M631MotionController *node;

    node = callback->owner;
    switch (context->state) {
    case 1:
    case 3:
    case 4:
    case 5:
        initialDeltaX = context->position->x - callback->position->x;
        initialDeltaZ = context->position->z - callback->position->z;
        deltaZ = context->position->z - callback->position->z;
        deltaX = context->position->x - callback->position->x;
        if (0.0f == deltaZ) {
            if (deltaX >= 0.0f) {
                angle = 1.5707964f;
            } else {
                angle = (-1.5707964f);
            }
        } else {
            computedAngle = (f32)atan2((f64)deltaX, (f64)deltaZ);
            angle = computedAngle;
        }
        angleForTrig = angle;
        node->avoidanceDirection.x = (f32)((f64)node->avoidanceDirection.x -
                                 (distance * sin((f64)angleForTrig)));
        node->avoidanceDirection.z = (f32)((f64)node->avoidanceDirection.z -
                                 (distance * cos((f64)angleForTrig)));
        break;
    default:
        break;
    }
}

/* The opening sequence schedules the airborne player's initial falling-object release. */
void fn_1_A8F0(M631ActorWork *actor)
{
    fn_1_B0B8(fn_1_A920, actor);
}

/* The opening release job shows the secondary model for nine updates, then hides it and attempts
 * a release on update ten, retrying until a node is available. */
s32 fn_1_A920(void *callbackData)
{
    M631ActorWork *actor = (M631ActorWork *)callbackData;
    M631AnimationNode *node;

    actor->releaseFrames++;
    if (actor->releaseFrames >= 10) {
        Hu3DModelAttrSet(actor->setupWork.releaseModelId, HU3D_ATTR_DISPOFF);
        node = fn_1_31D8(actor->setupWork.animationQueue);
        if (node) {
            omVibrate(actor->playerNo, 20, 4, 4);
            node->position.x = actor->position.x;
            node->position.y = actor->position.y;
            node->position.z = actor->position.z;
            if (actor->isNight != 0) {
                fn_1_AFF8(&node->callbackWork, 25.0f, 4);
            } else {
                fn_1_AFF8(&node->callbackWork, 75.0f, 4);
            }
            fn_1_AF2C(&node->callbackWork, (u32)node,
                      (u32)fn_1_32C8, (u32)fn_1_37FC, (u32)fn_1_38E8,
                      &node->position);
            node->animationState = 0;
            actor->releaseCooldown = 0;
            fn_1_AFF8(&actor->callbackWork, fn_1_2528(), 0);
            actor->releaseFrames = 0;
            return 1;
        }
    } else {
        Hu3DModelPosSetV(actor->setupWork.releaseModelId, &actor->position);
        Hu3DModelRotSetV(actor->setupWork.releaseModelId, &actor->rotation);
        Hu3DModelAttrReset(actor->setupWork.releaseModelId, HU3D_ATTR_DISPOFF);
    }
    return 0;
}

/* The opening sequence starts a ground player's entrance animation and schedules the job that
 * restores idle when it ends. */
void fn_1_AA78(M631ActorWork *actor) {
    CharMotionShiftSet(actor->charNo, actor->entranceMotionId, 0.0f, 5.0f, HU3D_MOTATTR_NONE);
    fn_1_B0B8(fn_1_AAD8, actor);
}
