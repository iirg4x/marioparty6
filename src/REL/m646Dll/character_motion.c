/* Loads Hyper Sniper players, updates their aim, and animates shots and the finish sequence. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

#include "dolphin/types.h"
#include "dolphin/mtx/GeoTypes.h"

#define M646_HELD_GUN_P1_FILE 14
#define M646_HELD_GUN_P2_FILE 15
#define M646_HELD_GUN_P3_FILE 16
#define M646_HELD_GUN_P4_FILE 17
#define M646_AIM_MODEL_P1_FILE 22
#define M646_AIM_MODEL_P2_FILE 23
#define M646_AIM_MODEL_P3_FILE 24
#define M646_AIM_MODEL_P4_FILE 25
#define M646_PREPARE_MOTION_FILE 37
#define M646_SHOT_MOTION_FILE 38
#define M646_AIM_MOTION_FILE 39
#define M646_PLAYER_WORK_BYTES 384
#define M646_SHOT_SE_ID 1985
#define M646_PLAYER_COLLISION_ACTIVE_FLAG 0x1
#define M646_PLAYER_COLLISION_GROUP_MASK 0x7

Vec lbl_1_data_190[4] = {
    { 356.0f, 0.0f, 0.0f }, { 119.0f, 0.0f, 0.0f }, { -119.0f, 0.0f, 0.0f }, { -356.0f, 0.0f, 0.0f }
};
char lbl_1_data_1C0[] = "gun1p_null";
char lbl_1_data_1CB[] = "gun2p_null";
char lbl_1_data_1D6[] = "gun3p_null";
char lbl_1_data_1E1[] = "gun4p_null";
char *lbl_1_data_1EC[4] = {lbl_1_data_1C0, lbl_1_data_1CB, lbl_1_data_1D6, lbl_1_data_1E1};
s32 lbl_1_data_1FC[4] = { DATANUM(DATA_m646, M646_HELD_GUN_P1_FILE),
                          DATANUM(DATA_m646, M646_HELD_GUN_P2_FILE),
                          DATANUM(DATA_m646, M646_HELD_GUN_P3_FILE),
                          DATANUM(DATA_m646, M646_HELD_GUN_P4_FILE) };
s32 lbl_1_data_20C[4] = { DATANUM(DATA_m646, M646_AIM_MODEL_P1_FILE),
                          DATANUM(DATA_m646, M646_AIM_MODEL_P2_FILE),
                          DATANUM(DATA_m646, M646_AIM_MODEL_P3_FILE),
                          DATANUM(DATA_m646, M646_AIM_MODEL_P4_FILE) };
/* Signed character-index table; CHARNO_MAX is the actual fourteen-entry API domain. */
s8 lbl_1_data_21C[CHARNO_MAX] = {0, 0, 1, 1, 1, 1, 1};
/* Signed character-index table; CHARNO_MAX is the actual fourteen-entry API domain. */
s8 lbl_1_data_22A[CHARNO_MAX] = {0};
s32 lbl_1_data_238[7] = {1, 0, 0, 0, 0, 1, 1};
s32 lbl_1_data_254[7] = { CHARMOT_HSF_c000m1_300, CHARMOT_HSF_c000m1_306,
                          CHARMOT_HSF_c000m1_307, DATANUM(DATA_mario, M646_PREPARE_MOTION_FILE),
                          DATANUM(DATA_mario, M646_SHOT_MOTION_FILE),
                          DATANUM(DATA_mario, M646_AIM_MOTION_FILE), CHARMOT_HSF_c000m1_301 };
char lbl_1_data_270[] = "ske_head";
char lbl_1_data_279[] = "ske_R_arm1";

typedef struct M646ActorAnchor_s M646ActorAnchor;
typedef struct M646ModelCollider_s M646ModelCollider;
typedef struct M646ModelAttachment_s M646ModelAttachment;

typedef struct M646MeshEdge_s {
    Vec first; /* First endpoint in mesh coordinates. */
    Vec second; /* Second endpoint in mesh coordinates. */
    Mtx *matrix; /* Matrix transforming these endpoints into world coordinates. */
} M646MeshEdge;

typedef struct M646WorldEdge_s {
    Vec first; /* First endpoint in world coordinates. */
    Vec second; /* Second endpoint in world coordinates. */
    Vec normal; /* Edge normal; zero when its endpoints coincide, including model-target buffers. */
    f32 planeConstant; /* Signed plane offset in world units. */
} M646WorldEdge;

typedef struct M646CollisionRecord_s {
    u8 stageCollisionPrefix[32]; /* Collision header preceding the edge list. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* Edges used by this frame's collision tests. */
    M646WorldEdge *alternate; /* Spare world-edge buffer swapped during transforms. */
    M646MeshEdge *original; /* Endpoints retained in source mesh coordinates. */
    s32 transformCount; /* Number of source objects supplying transforms. */
    HSF_TRANSFORM **transforms; /* Source-object transforms. */
    Mtx *matrices; /* Matrices applied to the source edges. */
} M646CollisionRecord;

struct M646ModelCollider_s {
    u32 kind; /* Collision registration and activity flags. */
    s32 group; /* Group mask tested against projectiles. */
    s32 callbackType; /* Contact category selecting the target's reaction. */
    s32 unusedPlayerIndex; /* Player-index slot not used by target attachments. */
    s32 callbackMode; /* 11 selects the model-target contact test. */
    M646ModelAttachment *owner; /* Attachment supplied to the update callback. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(M646ModelCollider *, M646ActorAnchor *); /* Handles a projectile hit. */
    s32 edgeCount; /* Number of collider edges. */
    M646WorldEdge *current; /* Edges used by this frame's contact test. */
    M646WorldEdge *alternate; /* Spare edge buffer. */
    s32 transformCount; /* Number of collider transforms. */
    HSF_TRANSFORM pose; /* Target joint position, rotation, and scale. */
    Mtx *matrices; /* Allocated and zeroed matrix buffer; model-target setup and contact tests leave
                    * it unused. */
    u8 contactEdgeStorage[4]; /* Storage for the edge selected during a contact test. */
    s16 category; /* Index selecting a category-specific contact distance. */
    HU3D_MODELID model; /* Model containing the target joint. */
    char jointName[64]; /* Joint followed by this target. */
    HSF_OBJECT *source; /* Source mesh used during collider setup. */
    s32 assigned; /* Nonzero while a computer player has reserved this attachment. */
};

struct M646ModelAttachment_s {
    HU3D_MODELID parentModel; /* Stage model containing the attachment's hook. */
    HU3D_MODELID model; /* Model displaying the attachment animation. */
    HU3D_MOTIONID motion; /* Motion played after a hit. */
    s16 state; /* Zero makes this attachment available as a target. */
    M646ModelCollider collider; /* Registered target collider. */
    Vec position; /* Attachment position in world coordinates. */
};

typedef struct M646MovingStageState_s {
    HU3D_MODELID model; /* Moving stage model. */
    HU3D_MODELID hiddenModel; /* Hidden model supplying collision meshes. */
    HU3D_MODELID parentModel; /* Model carrying this stage's hook. */
    s16 modelPadding; /* Storage between the model IDs and the stage kind. */
    u32 stageKind; /* Index into the stage descriptor table. */
    M646ModelAttachment attachments[5][12]; /* Targets grouped by contact category. */
    s16 jointIndex[5]; /* Next collision-joint index in each attachment group. */
    u8 jointPadding[2]; /* Storage following the attachment joint counters. */
    M646CollisionRecord collision; /* Stage surface edges checked against projectiles. */
    u8 collisionTail[104]; /* Stage storage following the collision record. */
} M646MovingStageState;

typedef struct M646MovingStageGroup_s {
    char **attachmentNames; /* Hooks where this group's target models are attached. */
    char **colliderNames; /* Hidden-model joints supplying the target collider meshes. */
    s16 attachmentCount; /* Number of targets in this group. */
    u8 attachmentCountPadding[2]; /* Storage between attachment count and contact category. */
    s32 callbackType; /* Base contact category for targets in this group. */
} M646MovingStageGroup;

typedef struct M646MovingStageDescriptor_s {
    s32 modelFile; /* Resource number for the visible stage model. */
    s32 hiddenModelFile; /* Resource number for the collision model. */
    char *hiddenHook; /* Visible-model hook carrying the collision model. */
    M646MovingStageGroup groups[5]; /* Attachment groups in this stage. */
} M646MovingStageDescriptor;

typedef struct M646MovingStageWork_s {
    s32 state; /* Cleared at stage creation; not read or changed by the stage callbacks. */
    s32 stageOrder[6]; /* Descriptor index assigned to each stage slot. */
    M646MovingStageState stages[6]; /* Models, targets, and collision work for each slot. */
} M646MovingStageWork;

/* Player aiming state, item transforms, and target results shared by the callbacks below. */
typedef struct M646PlayerAnchor_s {
    s32 kind; /* Collision registration and activity flags. */
    s32 group; /* Collision group bits. */
    s32 callbackType; /* Contact category; player setup uses zero. */
    s32 playerIndex; /* Player associated with this anchor. */
    s32 callbackMode; /* Zero selects the actor collision path. */
    OMOBJ *owner; /* Character object owning this anchor. */
    s32 updateCallbackSlot; /* Cleared at setup to disable the collision update callback. */
    s32 contactCallbackSlot; /* Cleared at setup to disable the collision response callback. */
    Vec *position; /* Player position in world coordinates. */
    Vec *rotation; /* Player rotation vector supplied as the collision direction buffer. */
    Vec savedPosition; /* Saved position used when a collision update yields invalid coordinates. */
    f32 radius; /* Collision radius in world units. */
    f32 responseScale; /* Scales the displacement applied by actor contact handling. */
} M646PlayerAnchor;

typedef struct M646PlayerAimState_s {
    M646PlayerWork motion; /* Character motion and turn-transition state. */
    f32 forceAngle; /* Forced head-joint Z rotation, in degrees. */
} M646PlayerAimState;

typedef struct M646PlayerFullWork_s {
    M646PlayerAimState aiming; /* Character motion and forced aim angle. */
    Vec modelPosition; /* Aim-model position; projectile target is 120 world units farther in Z. */
    Vec projectileDirection; /* Flight displacement per frame, with length 40 world units. */
    Vec projectilePosition; /* Gun-hook position where a shot starts. */
    Vec itemPosition; /* Detached gun position during the finish animation. */
    Vec itemRotation; /* Detached gun rotation in degrees. */
    f32 itemPhase; /* Frames elapsed in the detached-gun animation. */
    f32 itemDuration; /* Detached-gun animation duration in frames. */
    u8 itemWorkTail[12]; /* Player storage between item animation and controller state. */
    s16 padIndex; /* Controller port used by this player. */
    u8 padIndexPadding[2]; /* Storage after the controller index. */
    f32 stickOffsetX; /* Horizontal aim displacement for this frame, in world units. */
    f32 stickOffsetY; /* Vertical aim displacement for this frame, in world units. */
    u32 buttons; /* Buttons pressed this frame. */
    s32 playerKind; /* Zero for a human player; nonzero for a computer player. */
    s16 comDifficulty; /* Computer difficulty; -1 for a human player. */
    u8 difficultyPadding[2]; /* Storage between difficulty and targeting flags. */
    s32 targetReady; /* Nonzero while a computer player's target is active. */
    s32 motionReady; /* Nonzero while the computer aim is still moving toward its target. */
    M646ModelAttachment *targetData; /* Chosen target, or NULL when aiming at stored coordinates. */
    f32 aimError; /* Extra arrival tolerance in world units. */
    f32 randomX; /* Horizontal computer aim error added to the target, in world units. */
    f32 randomY; /* Vertical computer aim error added to the target, in world units. */
    s16 targetDelayTicks; /* Remaining target-selection attempts to wait; not a frame count. */
    s16 shotCount; /* Number of shots fired by the computer player. */
    f32 targetX; /* Horizontal fallback aim coordinate in world units. */
    f32 targetY; /* Vertical fallback aim coordinate in world units. */
    M646PlayerAnchor anchor; /* Player's record registered with collision handling. */
    u8 playerWorkTail[104]; /* Player storage following the collision anchor. */
} M646PlayerFullWork;

extern OMOBJ *lbl_1_bss_70[4];
extern s32 lbl_1_data_238[7];
extern char *lbl_1_data_1EC[4];
extern s32 lbl_1_data_20C[4];
extern s32 lbl_1_data_1FC[4];
extern s32 lbl_1_data_254[7];
extern Vec lbl_1_data_190[4];
extern Vec lbl_1_bss_84;
extern s8 lbl_1_data_21C[];
extern s8 lbl_1_data_22A[];
extern char lbl_1_data_270[];
extern char lbl_1_data_279[];

void fn_1_5A64(OMOBJ *obj, s16 motion);
OMOBJ *fn_1_23D0(Vec *position, f64 parameter, Vec *direction, OMOBJ *player);
s32 fn_1_B3C0(Vec *position, s32 sound);
void fn_1_6158(OMOBJ *obj);
void fn_1_680C(OMOBJ *obj);
f64 fn_1_2968(Vec lhs, Vec rhs);
void fn_1_6658(OMOBJ *obj);
void fn_1_6E20(OMOBJ *obj);
void fn_1_728C(OMOBJ *obj);
void fn_1_7858(OMOBJ *obj);
s32 fn_1_3E80(void *value);
void fn_1_A028(OMOBJ *obj);

void fn_1_543C(OMOBJMAN *manager);
void fn_1_5540(void);
void fn_1_5564(s16 player, s32 state);
void fn_1_55A0(void);
void fn_1_5634(OMOBJ *obj);
s32 fn_1_5694(OMOBJ *obj);
s32 fn_1_57DC(s16 player);
void fn_1_5850(s16 player);
void fn_1_589C(s16 player);
void fn_1_5964(s16 player, s32 resetAttribute);
void fn_1_59E0(s16 player, Vec *position);
void fn_1_5A64(OMOBJ *obj, s16 motion);
void fn_1_5BD4(OMOBJ *obj);
void fn_1_5C98(OMOBJ *obj);
void fn_1_5DAC(OMOBJ *obj);
void fn_1_5F80(OMOBJ *obj);
void fn_1_6158(OMOBJ *obj);
void fn_1_6308(OMOBJ *obj);
void fn_1_6538(OMOBJ *obj, s32 transitionCommand);
void fn_1_6554(OMOBJ *obj);
void fn_1_6658(OMOBJ *obj);
void fn_1_680C(OMOBJ *obj);
void fn_1_6B84(OMOBJ *obj);
s32 fn_1_6C48(OMOBJ *obj);
void fn_1_6E20(OMOBJ *obj);
void fn_1_728C(OMOBJ *obj);
void fn_1_7858(OMOBJ *obj);

OMOBJ *lbl_1_bss_70[4];

/* Creates the character object and its per-player work during sequence setup. */
void fn_1_543C(OMOBJMAN *manager)
{
    OMOBJ *obj = NULL;
    M646PlayerFullWork *work = NULL;
    s32 player = 0;
    for (player = 0; player < 4; player++) {
        obj = lbl_1_bss_70[player] = omAddObjEx(manager, 110, 3, 7, -1, fn_1_6E20);
        work = HuMemDirectMallocNum(HEAP_HEAP, M646_PLAYER_WORK_BYTES, HU_MEMNUM_OVL);
        obj->data = work;
        memset(work, 0, M646_PLAYER_WORK_BYTES);
        work->aiming.motion.playerIndex = player;
        work->playerKind = GwPlayerConf[player].type;
        work->comDifficulty = -1;
        if (work->playerKind) {
            work->comDifficulty = GwPlayerConf[player].comDif;
        }
    }
}

/* Clears all character models during sequence teardown. */
void fn_1_5540(void)
{
    CharModelKill(-1);
}

/* Winner-sequence setup stores whether this player should receive the winning result motion. */
void fn_1_5564(s16 player, s32 state)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    M646PlayerWork *work = obj->data;
    work->motionState = state;
}

/* The target coordinator selects result motion 1 for marked players and motion 2 for the others. */
void fn_1_55A0(void)
{
    OMOBJ *obj = NULL;
    M646PlayerWork *work = NULL;
    s32 player = 0;
    s32 motion;
    for (player = 0; player < 4; player++) {
        obj = lbl_1_bss_70[player];
        work = obj->data;
        motion = 2;
        if (work->motionState != 0) {
            motion = 1;
        }
        fn_1_5A64(obj, motion);
    }
}

/* The target coordinator captures this player's heading to begin the result-facing turn. */
void fn_1_5634(OMOBJ *obj)
{
    M646PlayerWork *work = obj->data;
    work->rotationActive = 1;
    work->rotationStart = obj->rot.y;
    work->rotationPhase = (0.0f);
    fn_1_5A64(obj, 6);
}

/* The coordinator advances the result-facing turn toward +180 or -180 degrees over 30 frames. */
s32 fn_1_5694(OMOBJ *obj)
{
    f32 initialRotation;
    f32 interpolatedRotation;
    M646PlayerWork *work;
    work = obj->data;
    initialRotation = work->rotationStart;
    if (work->rotationActive != 0) {
        /* Phase 30 still reports an active turn; the next update clamps to 30 and clears the
         * flag. */
        if (work->rotationPhase > (30.0f)) {
            work->rotationPhase = (30.0f);
            work->rotationActive = 0;
        }
        if (work->rotationStart < (0.0f)) {
            interpolatedRotation =
                initialRotation + (((-180.0f) - initialRotation) * (work->rotationPhase / (30.0f)));
        } else {
            interpolatedRotation =
                initialRotation + (((180.0f) - initialRotation) * (work->rotationPhase / (30.0f)));
        }
        omSetRot(obj, (0.0f), interpolatedRotation, (0.0f));
        work->rotationPhase += (1.0f);
    }
    return work->rotationActive;
}

/* Checks whether a player's current character motion has finished. */
s32 fn_1_57DC(s16 player)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    M646PlayerWork *playerWork = obj->data;
    HU3D_MODELID model = obj->mdlId[0];
    if (Hu3DMotionEndCheck(model)) {
        return TRUE;
    }
    return FALSE;
}

/* Starts motion 3 for a player at coordinator phases 3, 7, 11, and 15. */
void fn_1_5850(s16 player)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    fn_1_5A64(obj, 3);
}

/* Human and computer firing updates start a projectile at the gun hook and play the shot sound. */
void fn_1_589C(s16 player)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    M646PlayerFullWork *work = obj->data;
    Vec position;
    Vec direction;
    fn_1_5A64(obj, 4);
    /* The launch helper ignores its phase argument; this caller also ignores its return value. */
    fn_1_23D0((position = work->projectilePosition, &position), (0.3),
              (direction = work->projectileDirection, &direction), obj);
    fn_1_B3C0(&work->projectilePosition, M646_SHOT_SE_ID);
}

/* Intro and winner-sequence callbacks show or hide the aim model using resetAttribute. */
void fn_1_5964(s16 player, s32 resetAttribute)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    HU3D_MODELID model = obj->mdlId[2];
    if (resetAttribute != 0) {
        Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    } else {
        Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    }
}

/* Intro setup places the aim model at the supplied X/Y while retaining its existing Z. */
void fn_1_59E0(s16 player, Vec *position)
{
    OMOBJ *obj = lbl_1_bss_70[player];
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[2];
    work->modelPosition.x = position->x;
    work->modelPosition.y = position->y;
    Hu3DModelPosSetV(model, &work->modelPosition);
}

/* Selects the character model's motion, blending changes over 16 frames and restarting the same
 * motion. The primary loop flag is set only for motions 0 and 5; blends also use the table's loop
 * flag. */
void fn_1_5A64(OMOBJ *obj, s16 motion)
{
    M646PlayerWork *work = obj->data;
    s16 character = work->character;
    HU3D_MODELID model = obj->mdlId[0];
    u32 attributes = 0;
    if (lbl_1_data_238[motion] != 0) {
        attributes = HU3D_MOTATTR_LOOP;
    }
    if (work->state != motion) {
        CharMotionShiftSet(character, work->motions[motion], (0.0f), (16.0f), attributes);
    } else {
        CharMotionSet(character, work->motions[motion]);
    }
    switch (motion) {
        case 0:
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            break;
        case 5:
            Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
            break;
        case 3:
            Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
            break;
        case 6:
            Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
            break;
        default:
            Hu3DModelAttrReset(model, HU3D_MOTATTR_LOOP);
            break;
    }
    work->state = motion;
}

/* Each human or computer aiming update turns the character toward its current aim point. */
void fn_1_5BD4(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    Vec direction;
    f32 angle;
    PSVECSubtract(&work->modelPosition, &work->aiming.motion.translation, &direction);
    PSVECNormalize(&direction, &direction);
    angle = (90.0) - (180.0) * (atan2(direction.z, direction.x) / (3.141592653589793));
    omSetRot(obj, (0.0f), angle, (0.0f));
}

/* Aiming updates sample the gun hook and store a 40-unit shot vector toward the aim point. */
void fn_1_5C98(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[0];
    Vec direction;
    Vec position;
    Hu3DModelObjPosGet(model, lbl_1_data_1EC[work->aiming.motion.playerIndex], &position);
    direction.x = work->modelPosition.x - position.x;
    direction.y = work->modelPosition.y - position.y;
    direction.z = ((120.0f) + work->modelPosition.z) - position.z;
    PSVECNormalize(&direction, &direction);
    work->projectilePosition = position;
    work->projectileDirection.x = (40.0f) * direction.x;
    work->projectileDirection.y = (40.0f) * direction.y;
    work->projectileDirection.z = (40.0f) * direction.z;
}

/* Reads human input and updates the player's aim during the active turn. */
void fn_1_5DAC(OMOBJ *obj)
{
    M646PlayerFullWork *playerWork = obj->data;
    if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
        fn_1_6658(obj);
    }
    fn_1_5BD4(obj);
    fn_1_5C98(obj);
    fn_1_6158(obj);
}

/* Updates computer targeting when both readiness flags are set, then updates facing, projectile
 * direction, and aim pose. */
void fn_1_5F80(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[0];
    if (work->targetReady != 0 && work->motionReady != 0) {
        fn_1_680C(obj);
    }
    fn_1_5BD4(obj);
    fn_1_5C98(obj);
    fn_1_6158(obj);
}

/* Each aiming update derives the forced head Z rotation from the current aim point. */
void fn_1_6158(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    M646PlayerAimState *aim = &work->aiming;
    HU3D_MODELID model = obj->mdlId[0];
    Mtx rotationMatrix;
    Mtx inverseMatrix;
    Vec position;
    Vec direction;
    Vec rotation;
    Hu3DModelObjPosGet(model, lbl_1_data_270, &position);
    PSVECSubtract(&work->modelPosition, &position, &direction);
    Hu3DModelRotGet(model, &rotation);
    mtxRot(rotationMatrix, rotation.x, rotation.y, rotation.z);
    PSMTXInverse(rotationMatrix, inverseMatrix);
    PSMTXMultVec(inverseMatrix, &direction, &direction);
    if (lbl_1_data_21C[aim->motion.character] == 0) {
        aim->forceAngle = (180.0) * (atan2(direction.y, -direction.z) / (3.141592653589793));
        Hu3DMotionForceSet(model, lbl_1_data_270, HU3D_CONST_FORCE_ROTZ, aim->forceAngle);
    } else {
        aim->forceAngle = (180.0) * (atan2(direction.z, direction.y) / (3.141592653589793));
        /* Peach and Daisy keep the forced head angle at or above 80 degrees. */
        if ((aim->motion.character == CHARNO_PEACH || aim->motion.character == CHARNO_DAISY) &&
            aim->forceAngle < (80.0f)) {
            aim->forceAngle = (80.0f);
        }
        Hu3DMotionForceSet(model, lbl_1_data_270, HU3D_CONST_FORCE_ROTZ, aim->forceAngle);
    }
}

/* Optional arm-aim helper forces the right-arm rotation from the current aim point. */
void fn_1_6308(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[0];
    Mtx rotationMatrix;
    Mtx inverseMatrix;
    Vec position;
    Vec direction;
    Vec rotation;
    f32 angleY;
    f32 angleX;
    Hu3DModelObjPosGet(model, lbl_1_data_279, &position);
    PSVECSubtract(&work->modelPosition, &position, &direction);
    mbObjRotGet((MBMODELID)model, &rotation);
    mtxRot(rotationMatrix, rotation.x, rotation.y, rotation.z);
    PSMTXInverse(rotationMatrix, inverseMatrix);
    PSMTXMultVec(inverseMatrix, &direction, &direction);
    if (lbl_1_data_22A[work->aiming.motion.character] == 0) {
        /* The current table selects this Y-only branch for every character; angleX is computed
         * but unused here. */
        angleX = -((180.0) * (atan2(direction.x, direction.z) / (3.141592653589793)));
        angleY = -((180.0) * (atan2(direction.y, direction.z) / (3.141592653589793)));
        Hu3DMotionForceSet(model, lbl_1_data_279, HU3D_CONST_FORCE_ROTY, angleY);
    } else {
        angleX = (180.0) * (atan2(direction.x, direction.z) / (3.141592653589793));
        angleY = (180.0) * (atan2(direction.z, direction.y) / (3.141592653589793));
        if ((work->aiming.motion.character == CHARNO_PEACH ||
             work->aiming.motion.character == CHARNO_DAISY) &&
            angleY < (80.0f)) {
            angleY = (80.0f);
        }
        Hu3DMotionForceSet(model, lbl_1_data_279, HU3D_CONST_FORCE_ROTX, angleX);
        Hu3DMotionForceSet(model, lbl_1_data_279, HU3D_CONST_FORCE_ROTZ, angleY);
    }
}

/* Stores the transition command used by the player's per-frame callback. */
void fn_1_6538(OMOBJ *obj, s32 transitionCommand)
{
    M646PlayerWork *work = obj->data;
    work->transitionCommand = transitionCommand;
}

/* Character initialization creates the hidden aim model at the stage's aiming depth. */
void fn_1_6554(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_20C[work->aiming.motion.playerIndex], HU_MEMNUM_OVL,
                             HEAP_MODEL));
    Hu3DModelLayerSet(model, 7);
    Hu3DModelAttrSet(obj->mdlId[2], HU3D_ATTR_DISPOFF);
    work->modelPosition.x = (0.0f);
    work->modelPosition.y = (0.0f);
    work->modelPosition.z = lbl_1_bss_84.z - (120.0f);
    Hu3DModelScaleSet(model, (1.0f), (1.0f), (1.0f));
    Hu3DModelPosSetV(model, &work->modelPosition);
}

/* The human update moves the aim model from stick input and clamps it to the target area. */
void fn_1_6658(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID model = obj->mdlId[2];
    work->stickOffsetX = (0.15f) * -HuPadStkX[work->padIndex];
    work->stickOffsetY = (0.15f) * HuPadStkY[work->padIndex];
    work->modelPosition.x += work->stickOffsetX;
    work->modelPosition.y += work->stickOffsetY;
    if (work->modelPosition.x > (425.0f)) {
        work->modelPosition.x = (425.0f);
    }
    if (work->modelPosition.x < (-420.0f)) {
        work->modelPosition.x = (-420.0f);
    }
    if (work->modelPosition.y > (620.0f)) {
        work->modelPosition.y = (620.0f);
    }
    if (work->modelPosition.y < (176.0f)) {
        work->modelPosition.y = (176.0f);
    }
    Hu3DModelPosSetV(model, &work->modelPosition);
}

/* Computer updates move toward the target and finish aiming within 3+aimError world units. */
void fn_1_680C(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    M646ModelAttachment *attachment = NULL;
    M646ModelCollider *collider = NULL;
    HU3D_MODELID model = obj->mdlId[2];
    Vec position;
    Vec direction;
    Vec target = { 0, 0, 0 };
    f32 radius;
    f64 distance;
    attachment = work->targetData;
    if (attachment) {
        collider = &work->targetData->collider;
        target.x = collider->pose.pos.x + work->randomX;
        target.y = (50.0f + collider->pose.pos.y) + work->randomY;
        target.z = collider->pose.pos.z;
        /* Reject high or already-hit attachments by clearing readiness and delay, but still
         * finish this frame's movement toward their sampled position. */
        if (collider->pose.pos.y > (560.0f)) {
            work->targetReady = 0;
            work->motionReady = 0;
            work->targetDelayTicks = 0;
        }
        if (work->targetData->state > 0) {
            work->targetReady = 0;
            work->motionReady = 0;
            work->targetDelayTicks = 0;
        }
        direction.x = target.x - work->modelPosition.x;
        direction.y = target.y - work->modelPosition.y;
        direction.z = target.z - ((120.0f) + work->modelPosition.z);
    } else {
        target.x = work->targetX;
        target.y = work->targetY;
        target.z = (120.0f) + work->modelPosition.z;
        direction.x = target.x - work->modelPosition.x;
        direction.y = target.y - work->modelPosition.y;
        direction.z = (0.0f);
    }
    PSVECNormalize(&direction, &direction);
    direction.x *= (6.5f);
    direction.y *= (6.5f);
    direction.z *= (6.5f);
    work->modelPosition.x += direction.x;
    work->modelPosition.y += direction.y;
    if (work->modelPosition.x > (425.0f)) {
        work->modelPosition.x = (425.0f);
    }
    if (work->modelPosition.x < (-420.0f)) {
        work->modelPosition.x = (-420.0f);
    }
    if (work->modelPosition.y > (620.0f)) {
        work->modelPosition.y = (620.0f);
    }
    if (work->modelPosition.y < (176.0f)) {
        work->modelPosition.y = (176.0f);
    }
    position.x = work->modelPosition.x;
    position.y = work->modelPosition.y;
    position.z = (120.0f) + work->modelPosition.z;
    radius = (3.0f);
    radius += work->aimError;
    distance = fn_1_2968(position, target);
    if (distance < radius) {
        work->motionReady = 0;
    }
    Hu3DModelPosSetV(model, &work->modelPosition);
}

/* At finish, the player update detaches the gun and starts its 60-frame shrink-and-spin
 * animation. */
void fn_1_6B84(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    HU3D_MODELID item = obj->mdlId[1];
    char *hook = NULL;
    HU3D_MODELID model = obj->mdlId[0];
    s16 character = work->aiming.motion.character;
    hook = CharModelItemHookGet(character, 4, 0);
    work->itemPhase = (0.0f);
    work->itemDuration = (60.0f);
    Hu3DModelObjPosGet(model, hook, &work->itemPosition);
    Hu3DModelRotGet(model, &work->itemRotation);
    Hu3DModelHookReset(model);
    Hu3DModelPosSetV(item, &work->itemPosition);
    Hu3DModelRotSet(item, work->itemRotation.x, work->itemRotation.y, work->itemRotation.z);
}

/* Finish updates shrink the gun, then hide it; head rotation tends toward 180 or 90 degrees. */
s32 fn_1_6C48(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    M646PlayerAimState *aim = &work->aiming;
    HU3D_MODELID model = obj->mdlId[0];
    HU3D_MODELID item = obj->mdlId[1];
    f32 scale;
    f32 angle;
    scale = (1.0f) - work->itemPhase / work->itemDuration;
    Hu3DModelScaleSet(item, scale, scale, scale);
    work->itemPosition.y += (1.5f);
    Hu3DModelPosSetV(item, &work->itemPosition);
    Hu3DModelRotSet(item, work->itemRotation.x, work->itemRotation.y, work->itemRotation.z);
    work->itemRotation.x += (20.0f);
    if ((0.0f) == scale) {
        Hu3DModelAttrSet(item, HU3D_ATTR_DISPOFF);
        return 1;
    }
    work->itemPhase += (1.0f);
    if (lbl_1_data_21C[aim->motion.character] == 0) {
        angle =
            aim->forceAngle + work->itemPhase * (((180.0f) - aim->forceAngle) / work->itemDuration);
        Hu3DMotionForceSet(model, lbl_1_data_270, HU3D_CONST_FORCE_ROTZ, angle);
    } else {
        angle =
            aim->forceAngle - work->itemPhase * ((aim->forceAngle - (90.0f)) / work->itemDuration);
        Hu3DMotionForceSet(model, lbl_1_data_270, HU3D_CONST_FORCE_ROTZ, angle);
    }
    return 0;
}

/* Initializes each player's character models, callbacks, and collision anchor. */
void fn_1_6E20(OMOBJ *obj)
{
    M646PlayerFullWork *work = NULL;
    s32 motionIndex = 0;
    s16 player;
    HU3D_MODELID model;
    s16 character;
    HU3D_MODELID item;
    char *hook;
    M646PlayerAnchor *anchor;

    work = obj->data;
    player = work->aiming.motion.playerIndex;
    character = work->aiming.motion.character = GwPlayerConf[player].charNo;
    work->padIndex = GwPlayerConf[player].padNo;
    model = obj->mdlId[0] = CharModelCreate(work->aiming.motion.character, 4);
    for (motionIndex = 0; motionIndex < 7; motionIndex++) {
        work->aiming.motion.motions[motionIndex] =
            CharMotionCreate(character, lbl_1_data_254[motionIndex]);
    }
    CharMotionDataClose(character);
    fn_1_5A64(obj, 5);
    work->stickOffsetX = work->stickOffsetY = (0.0f);
    work->aiming.motion.translation.x = work->aiming.motion.translation.y =
        work->aiming.motion.translation.z = (0.0f);
    work->aiming.motion.rotation.x = work->aiming.motion.rotation.y =
        work->aiming.motion.rotation.z = (0.0f);
    work->aiming.motion.setupRotation.x = work->aiming.motion.setupRotation.y =
        work->aiming.motion.setupRotation.z = (0.0f);
    work->aiming.motion.translation.x = lbl_1_data_190[work->aiming.motion.playerIndex].x;
    work->aiming.motion.translation.y = lbl_1_data_190[work->aiming.motion.playerIndex].y;
    work->aiming.motion.translation.z = lbl_1_data_190[work->aiming.motion.playerIndex].z;
    omSetTra(obj, work->aiming.motion.translation.x, work->aiming.motion.translation.y,
             work->aiming.motion.translation.z);
    omSetRot(obj, work->aiming.motion.setupRotation.x, work->aiming.motion.setupRotation.y,
             work->aiming.motion.setupRotation.z);
    Hu3DModelAttrSet(model, HU3D_MOTATTR_LOOP);
    Hu3DModelLayerSet(model, 4);
    Hu3DModelShadowSet(model);
    item = obj->mdlId[1] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_1FC[work->aiming.motion.playerIndex], HU_MEMNUM_OVL,
                             HEAP_MODEL));
    hook = CharModelItemHookGet(character, 4, 0);
    Hu3DModelHookSet(obj->mdlId[0], hook, item);
    Hu3DModelLayerSet(item, 4);
    work->modelPosition.x = work->modelPosition.y = work->modelPosition.z = (0.0f);
    fn_1_6554(obj);
    anchor = &work->anchor;
    anchor->kind = M646_PLAYER_COLLISION_ACTIVE_FLAG;
    anchor->group = M646_PLAYER_COLLISION_GROUP_MASK;
    anchor->callbackType = 0;
    anchor->playerIndex = work->aiming.motion.playerIndex;
    anchor->callbackMode = 0;
    anchor->owner = obj;
    anchor->updateCallbackSlot = 0;
    anchor->contactCallbackSlot = 0;
    anchor->position = &work->aiming.motion.translation;
    anchor->rotation = &work->aiming.motion.rotation;
    anchor->savedPosition = *anchor->position;
    anchor->radius = (40.0f);
    anchor->responseScale = (0.5f);
    fn_1_3E80(anchor);
    fn_1_A028(obj);
    if (work->playerKind == 0) {
        obj->objFunc = fn_1_728C;
    } else {
        obj->objFunc = fn_1_7858;
    }
}

/* Runs the human player's per-frame update and result transition. */
void fn_1_728C(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    if (MgSeqModeGet() <= MGSEQ_MODE_MAIN) {
        fn_1_5DAC(obj);
    }
    switch (work->aiming.motion.transitionCommand) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 1:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                work->aiming.motion.transitionCommand++;
            } else {
                break;
            }
        case 2:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                work->buttons = HuPadBtnDown[work->padIndex];
                if (work->buttons & PAD_BUTTON_A) {
                    fn_1_589C(work->aiming.motion.playerIndex);
                    work->aiming.motion.transitionCommand++;
                }
            } else {
                work->aiming.motion.transitionCommand = 4;
            }
            break;
        case 3:
            if (fn_1_57DC(work->aiming.motion.playerIndex)) {
                work->aiming.motion.transitionCommand--;
            }
            break;
        case 4:
            if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
                fn_1_5A64(obj, 0);
                fn_1_6B84(obj);
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 5:
            if (fn_1_6C48(obj)) {
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 6:
            if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
                work->aiming.motion.transitionCommand++;
            }
            break;
    }
}

/* Runs the computer player's per-frame update and result transition. */
void fn_1_7858(OMOBJ *obj)
{
    M646PlayerFullWork *work = obj->data;
    if (MgSeqModeGet() <= MGSEQ_MODE_MAIN) {
        fn_1_5F80(obj);
    }
    switch (work->aiming.motion.transitionCommand) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 1:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                work->aiming.motion.transitionCommand++;
            } else {
                break;
            }
        case 2:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                if (work->targetReady != 0 && work->motionReady == 0) {
                    fn_1_589C(work->aiming.motion.playerIndex);
                    work->shotCount++;
                    work->aiming.motion.transitionCommand++;
                }
            } else {
                work->aiming.motion.transitionCommand = 4;
            }
            break;
        case 3:
            if (fn_1_57DC(work->aiming.motion.playerIndex)) {
                work->targetReady = 0;
                work->aiming.motion.transitionCommand--;
            }
            break;
        case 4:
            if (MgSeqModeGet() == MGSEQ_MODE_FINISH) {
                fn_1_5A64(obj, 0);
                fn_1_6B84(obj);
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 5:
            if (fn_1_6C48(obj)) {
                work->aiming.motion.transitionCommand++;
            }
            break;
        case 6:
            if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
                work->aiming.motion.transitionCommand++;
            }
            break;
    }
}
