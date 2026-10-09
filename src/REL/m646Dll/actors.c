/* Pools Hyper Sniper projectiles and updates their flight, target hits, and impact effects. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

/* Resource entries are selected by projectile effect and player slot. */
#define M646_PROJECTILE_P1_FILE 18
#define M646_PROJECTILE_P2_FILE 19
#define M646_PROJECTILE_P3_FILE 20
#define M646_PROJECTILE_P4_FILE 21
#define M646_CONTACT_MODEL_P1_FILE 26
#define M646_CONTACT_MODEL_P2_FILE 27
#define M646_CONTACT_MODEL_P3_FILE 28
#define M646_CONTACT_MODEL_P4_FILE 29
#define M646_CONTACT_MOTION_P1_FILE 30
#define M646_CONTACT_MOTION_P2_FILE 31
#define M646_CONTACT_MOTION_P3_FILE 32
#define M646_CONTACT_MOTION_P4_FILE 33
#define M646_EXTRA_CONTACT_MODEL_P1_FILE 34
#define M646_EXTRA_CONTACT_MODEL_P2_FILE 35
#define M646_EXTRA_CONTACT_MODEL_P3_FILE 36
#define M646_EXTRA_CONTACT_MODEL_P4_FILE 37
#define M646_BOUNDARY_IMPACT_MODEL_FILE 38
#define M646_EXTRA_CONTACT_MOTION_P1_FILE 39
#define M646_EXTRA_CONTACT_MOTION_P2_FILE 40
#define M646_EXTRA_CONTACT_MOTION_P3_FILE 41
#define M646_EXTRA_CONTACT_MOTION_P4_FILE 42
#define M646_BOUNDARY_IMPACT_MOTION_FILE 43
#define M646_FINAL_CONTACT_P1_FILE 44
#define M646_FINAL_CONTACT_P2_FILE 45
#define M646_FINAL_CONTACT_P3_FILE 46
#define M646_FINAL_CONTACT_P4_FILE 47

#define M646_PROJECTILE_WORK_BYTES 244
#define M646_COLLISION_ACTIVE_FLAG 0x1U
#define M646_PROJECTILE_COLLISION_GROUP_MASK 0xA

typedef struct M646ActorAnchor_s M646ActorAnchor;
typedef struct M646ModelCollider_s M646ModelCollider;
struct M646ActorAnchor_s {
    u32 kind; /* Collision registration and activity flags. */
    s32 group; /* Collision group bits tested against other registered objects. */
    s32 callbackType; /* Actor collision category used by contact handling. */
    s32 playerIndex; /* Collision player tag; projectile setup fixes it at zero. */
    s32 callbackMode; /* Manager mode that selects this anchor's collision path. */
    OMOBJ *owner; /* Object whose work record owns this anchor. */
    void (*update)(OMOBJ *); /* Callback that updates the anchor after collision checks. */
    s32 (*contact)(M646ActorAnchor *, M646ModelCollider *); /* Handles target contact. */
    Vec *position; /* Current actor position in stage units. */
    Vec *direction; /* Per-frame movement vector in stage units. */
    Vec savedPosition; /* Previous position used for swept collision checks. */
    f32 radius; /* Actor collision radius in stage units. */
    f32 responseScale; /* Scales movement added by actor-actor contact. */
};

typedef struct M646ActorState_s {
    u8 actorWorkStart[8]; /* Actor work before its contact state. */
    s32 callbackType12Seen; /* Set when this actor contacts a type 12 target. */
    s32 callbackType2Seen; /* Set when this actor contacts a type 2 target. */
    s32 contactHandled; /* Prevents another stage-boundary reaction for this flight. */
    s32 activeAttachmentContact; /* Set when this projectile contacts an attachment that has already
                                  * been hit. */
    s32 boundaryEffectActive; /* Nonzero while the stage-boundary impact animation is playing. */
    s32 effectActive[3]; /* Nonzero while each target-contact effect animation is playing. */
    Vec position; /* Actor position in stage units. */
    Vec direction; /* Per-frame flight or fall movement in stage units. */
    s32 state; /* 0 enters flight updates, 1 checks boundaries, and 2 skips boundary checks. */
    s32 motionActive; /* Nonzero while following its launch direction. */
    OMOBJ *linkedObject; /* Player object that launched this pooled actor. */
    s16 playerIndex; /* Player whose model set is used by this actor. */
    u8 actorWorkBeforeAnchor[2]; /* Actor work between the player index and collision anchor. */
    M646ActorAnchor anchor; /* Projectile collision callbacks and movement references. */
    u8 actorWorkBeforeTarget[44]; /* Actor work between collision handling and target state. */
    s32 unusedWord; /* Cleared when allocated; projectile callbacks do not access this word. */
    s32 unusedInteger; /* Cleared when allocated; projectile callbacks do not access this word. */
    void *unusedPointer; /* Cleared when allocated; projectile callbacks do not access this pointer
                          * slot. */
    u8 actorWorkBeforeTargetCoordinates[16]; /* Actor work before target coordinates. */
    f32 unusedFloat; /* Cleared when allocated; projectile callbacks do not access this float-sized
                      * storage. */
    f32 unusedScalar; /* Cleared when allocated; projectile callbacks do not access this float-sized
                       * storage. */
    u8 actorWorkTail[24]; /* Actor work following the target coordinates. */
} M646ActorState;

typedef struct M646MeshEdge_s {
    Vec first; /* First endpoint in the source mesh's local coordinates. */
    Vec second; /* Second endpoint in the source mesh's local coordinates. */
    Mtx *matrix; /* Matrix used to transform this edge into world space. */
} M646MeshEdge;

typedef struct M646WorldEdge_s {
    Vec first; /* First endpoint in world coordinates. */
    Vec second; /* Second endpoint in world coordinates. */
    Vec normal; /* Edge normal; zero when its endpoints coincide, including model-target buffers. */
    f32 planeConstant; /* Plane offset in world units. */
} M646WorldEdge;

typedef struct M646CollisionRecord_s {
    u8 stageCollisionPrefix[32]; /* Stage collision data before the edge list. */
    s32 edgeCount; /* Number of edges included in the stage collision sweep. */
    M646WorldEdge *current; /* Edges used by the current collision pass. */
    M646WorldEdge *alternate; /* Spare edge buffer swapped during updates. */
    M646MeshEdge *original; /* Source mesh endpoints before world transforms. */
    s32 transformCount; /* Number of HSF object transforms. */
    HSF_TRANSFORM **transforms; /* Source transforms for the HSF object list. */
    Mtx *matrices; /* Per-object matrices applied to source edges. */
} M646CollisionRecord;

typedef struct M646ModelAttachment_s M646ModelAttachment;
struct M646ModelCollider_s {
    u32 kind; /* Collision participation flags. */
    s32 group; /* Group mask compared with the projectile group. */
    s32 callbackType; /* Target category selecting points, sound, and contact flags. */
    s32 unusedPlayerIndex; /* Player-index slot unused by target attachments. */
    s32 callbackMode; /* 11 selects model-target contact tests. */
    M646ModelAttachment *owner; /* Target attachment that owns this collider. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(M646ModelCollider *, M646ActorAnchor *); /* Handles the target reaction. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* Edges used by the current contact pass. */
    M646WorldEdge *alternate; /* Spare world-space edge buffer. */
    s32 transformCount; /* Number of collider transform matrices. */
    HSF_TRANSFORM pose; /* Target joint position, rotation, and scale. */
    Mtx *matrices; /* Transform matrices allocated for the collider. */
    u8 attachmentColliderPrefix[4]; /* Collider data before its category value. */
    s16 category; /* Selects the category-specific contact distance. */
    HU3D_MODELID model; /* Model containing the attached target joint. */
    char jointName[64]; /* Joint followed by the attachment. */
    HSF_OBJECT *source; /* HSF object selected during collider setup. */
    s32 assigned; /* Nonzero while a computer player's target selection has reserved this
                   * attachment. */
};

struct M646ModelAttachment_s {
    HU3D_MODELID parentModel; /* Model containing the attached target joint. */
    HU3D_MODELID model; /* Model used by the attachment animation. */
    HU3D_MOTIONID motion; /* Joint motion played for the attachment. */
    s16 state; /* Zero enables score-target contact handling. */
    M646ModelCollider collider; /* Collision data registered with the manager. */
    Vec position; /* Current attachment position in stage units. */
};

typedef struct M646ActorStageSource_s {
    u8 stageWorkPrefix[11304]; /* Stage work before its moving collision record. */
    M646CollisionRecord collision; /* World-space edges for this moving stage piece. */
    u8 stageWorkTail[104]; /* Stage work after the collision record. */
} M646ActorStageSource;

typedef struct M646ActorStageWork_s {
    s32 state; /* Cleared at stage creation; not read or changed by the stage callbacks. */
    s32 stageOrder[6]; /* Layout descriptor index assigned to each of the six stage slots. */
    M646ActorStageSource stages[6]; /* Collision records for the six stage pieces. */
} M646ActorStageWork;

typedef struct M646ActorPlayerTarget_s {
    u8 playerTargetPrefix[172]; /* Player work before the target-active flag. */
    s32 active; /* Cleared after a stage-boundary contact. */
} M646ActorPlayerTarget;

extern OMOBJ *lbl_1_bss_30[4][3];
extern s16 lbl_1_bss_28[4];
extern OMOBJ *lbl_1_bss_80;
extern Vec lbl_1_data_38;
extern Vec lbl_1_data_44;
extern s32 lbl_1_data_50[4];
extern s32 lbl_1_data_60[4];
extern s32 lbl_1_data_70[4];
extern s32 lbl_1_data_80[4];
extern s32 lbl_1_data_90[4];
extern s32 lbl_1_data_A0[4];
extern s32 lbl_1_data_B0[4];
extern s32 lbl_1_data_C0[4];
void fn_1_1630(OMOBJ *obj);
void fn_1_1B08(OMOBJ *obj);
void fn_1_1F24(OMOBJ *obj);
s32 fn_1_1F80(M646ActorAnchor *actor, M646ModelCollider *target);
s32 fn_1_48A4(const M646CollisionRecord *anchor, const Point3d *position);
s32 fn_1_3E80(void *collisionResource);
void fn_1_8F04(s32 player, s32 score);
void fn_1_8F5C(s32 player);
s32 fn_1_B3C0(HuVecF *position, s32 soundId);

typedef struct M646ActorPlayerSource_s {
    M646PlayerWork motion; /* Launching player identity and character motion state. */
    u8 playerWorkBeforeTarget[308]; /* Player work between motion state and target state. */
} M646ActorPlayerSource;

#define M646_TARGET_CONTACT_SE_ID 1986
#define M646_TARGET_SCORE_SE_ID 1987
#define M646_TARGET_SCORE_CLEAR_SE_ID 1988
#define M646_STAGE_FALL_SE_ID 1989

void fn_1_AF4(void);
void fn_1_AF8(void);
void fn_1_AFC(OMOBJMAN *objman, s16 player);
void fn_1_BF4(void);
void fn_1_BF8(OMOBJ *obj);
void fn_1_D8C(OMOBJ *obj);
void fn_1_E98(OMOBJ *obj);
void fn_1_F68(OMOBJ *obj);
void fn_1_FDC(OMOBJ *obj);
void fn_1_1290(OMOBJ *obj);
void fn_1_1430(OMOBJ *obj);
void fn_1_1514(OMOBJ *obj);
void fn_1_1570(OMOBJ *obj, s32 active);
void fn_1_15A4(OMOBJ *obj, s32 sound);
void fn_1_1630(OMOBJ *obj);
void fn_1_1B08(OMOBJ *obj);
void fn_1_1F24(OMOBJ *obj);
s32 fn_1_1F80(M646ActorAnchor *actor, M646ModelCollider *target);
OMOBJ *fn_1_23D0(Vec *position, f64 phase, Vec *direction, OMOBJ *player);

Vec lbl_1_data_38 = {0.0f, 0.0f, 0.0f};

Vec lbl_1_data_44 = {0.0f, 0.0f, 0.0f};

s32 lbl_1_data_50[4] = {
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MODEL_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MODEL_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MODEL_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MODEL_FILE)
};

s32 lbl_1_data_60[4] = {
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MOTION_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MOTION_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MOTION_FILE),
    DATANUM(DATA_m646, M646_BOUNDARY_IMPACT_MOTION_FILE)
};

s32 lbl_1_data_70[4] = {
    DATANUM(DATA_m646, M646_CONTACT_MODEL_P1_FILE), DATANUM(DATA_m646, M646_CONTACT_MODEL_P2_FILE),
    DATANUM(DATA_m646, M646_CONTACT_MODEL_P3_FILE), DATANUM(DATA_m646, M646_CONTACT_MODEL_P4_FILE)
};

s32 lbl_1_data_80[4] = {
    DATANUM(DATA_m646, M646_CONTACT_MOTION_P1_FILE),
    DATANUM(DATA_m646, M646_CONTACT_MOTION_P2_FILE),
    DATANUM(DATA_m646, M646_CONTACT_MOTION_P3_FILE),
    DATANUM(DATA_m646, M646_CONTACT_MOTION_P4_FILE)
};

s32 lbl_1_data_90[4] = {
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MODEL_P1_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MODEL_P2_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MODEL_P3_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MODEL_P4_FILE)
};

s32 lbl_1_data_A0[4] = {
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MOTION_P1_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MOTION_P2_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MOTION_P3_FILE),
    DATANUM(DATA_m646, M646_EXTRA_CONTACT_MOTION_P4_FILE)
};

s32 lbl_1_data_B0[4] = {
    DATANUM(DATA_m646, M646_FINAL_CONTACT_P1_FILE), DATANUM(DATA_m646, M646_FINAL_CONTACT_P2_FILE),
    DATANUM(DATA_m646, M646_FINAL_CONTACT_P3_FILE), DATANUM(DATA_m646, M646_FINAL_CONTACT_P4_FILE)
};

s32 lbl_1_data_C0[4] = {
    DATANUM(DATA_m646, M646_PROJECTILE_P1_FILE), DATANUM(DATA_m646, M646_PROJECTILE_P2_FILE),
    DATANUM(DATA_m646, M646_PROJECTILE_P3_FILE), DATANUM(DATA_m646, M646_PROJECTILE_P4_FILE)
};

/* Three reusable projectile objects per player and each player's next pool slot. */

OMOBJ *lbl_1_bss_30[4][3];

s16 lbl_1_bss_28[4];

void fn_1_AF4(void)
{
}

void fn_1_AF8(void)
{
}

/* Sequence setup allocates three reusable projectiles and resets the player's next pool slot. */
void fn_1_AFC(OMOBJMAN *objman, s16 player)
{
    OMOBJ *obj;
    M646ActorState *work;
    s32 actorIndex = 0;
    for (actorIndex = 0; actorIndex < 3; actorIndex++) {
        obj = omAddObjEx(objman, 40, 6, 4, OM_GRP_NONE, fn_1_1630);
        lbl_1_bss_30[player][actorIndex] = obj;
        obj->stat |= OM_STAT_MODELPAUSE;
        work = HuMemDirectMallocNum(HEAP_HEAP, M646_PROJECTILE_WORK_BYTES, HU_MEMNUM_OVL);
        obj->data = work;
        memset(work, 0, M646_PROJECTILE_WORK_BYTES);
        work->playerIndex = player;
    }
    lbl_1_bss_28[player] = 0;
}

void fn_1_BF4(void)
{
}

/* Projectile startup creates its hidden flight model and registers its collision callbacks. */
void fn_1_BF8(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    M646ActorAnchor *anchor;
    HU3D_MODELID model;
    work->position = lbl_1_data_38;
    work->direction = lbl_1_data_44;
    model = obj->mdlId[0] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_C0[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (-10000.0f));
    Hu3DModelLayerSet(model, 5);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    anchor = &work->anchor;
    anchor->kind = 0;
    anchor->group = M646_PROJECTILE_COLLISION_GROUP_MASK;
    anchor->callbackType = 1;
    anchor->playerIndex = 0;
    anchor->callbackMode = 0;
    anchor->owner = obj;
    anchor->update = fn_1_1F24;
    anchor->contact = fn_1_1F80;
    anchor->position = &work->position;
    anchor->direction = &work->direction;
    anchor->savedPosition = work->position;
    anchor->radius = (40.0f);
    anchor->responseScale = (1.0f);
    fn_1_3E80(anchor);
}

/* Projectile startup loads the shared boundary-impact model and leaves its motion stopped. */
void fn_1_D8C(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[2] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_50[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelLayerSet(model, 7);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    motion = obj->mtnId[0] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(lbl_1_data_60[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(model, (0.0f));
    work->boundaryEffectActive = 0;
}

/* The flight update starts the boundary-impact animation at projectile X, Y-20, and Z-40. */
void fn_1_E98(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    f32 modelDepth = (-40.0f) + work->position.z;
    HU3D_MODELID model = obj->mdlId[2];
    HU3D_MOTIONID motion = obj->mtnId[0];
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(model, work->position.x, (-20.0f) + work->position.y, modelDepth);
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (1.0f));
    work->boundaryEffectActive = 1;
}

/* Each projectile frame hides the boundary-impact model when its animation finishes. */
void fn_1_F68(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID model;
    if (work->boundaryEffectActive != 0) {
        model = obj->mdlId[2];
        if (Hu3DMotionEndCheck(model) != 0) {
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            work->boundaryEffectActive = 0;
        }
    }
}

/* Projectile startup loads three player-specific target-contact effects with stopped motions. */
void fn_1_FDC(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID model;
    HU3D_MOTIONID motion;
    model = obj->mdlId[3] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_70[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelLayerSet(model, 7);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    motion = obj->mtnId[1] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(lbl_1_data_80[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(model, (0.0f));
    work->effectActive[0] = 0;
    model = obj->mdlId[4] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_90[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelLayerSet(model, 7);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    motion = obj->mtnId[2] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(lbl_1_data_A0[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(model, (0.0f));
    work->effectActive[1] = 0;
    /* The final contact resource supplies both its model and its motion. */
    model = obj->mdlId[5] = Hu3DModelCreate(
        HuDataSelHeapReadNum(lbl_1_data_B0[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DModelPosSet(model, (0.0f), (0.0f), (0.0f));
    Hu3DModelLayerSet(model, 7);
    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
    motion = obj->mtnId[3] = Hu3DJointMotion(
        model, HuDataSelHeapReadNum(lbl_1_data_B0[work->playerIndex], HU_MEMNUM_OVL, HEAP_MODEL));
    Hu3DMotionSpeedSet(model, (0.0f));
    work->effectActive[2] = 0;
}

/* A target-contact callback shows all three impact effects and restarts their motions. */
void fn_1_1290(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    f32 modelDepth = (-40.0f) + work->position.z;
    HU3D_MODELID model = obj->mdlId[3];
    HU3D_MOTIONID motion = obj->mtnId[1];
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(model, work->position.x, (-20.0f) + work->position.y, modelDepth);
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (1.0f));
    model = obj->mdlId[4];
    motion = obj->mtnId[2];
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(model, work->position.x, (-20.0f) + work->position.y, modelDepth);
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (1.0f));
    model = obj->mdlId[5];
    motion = obj->mtnId[3];
    Hu3DModelAttrReset(model, HU3D_ATTR_DISPOFF);
    Hu3DModelPosSet(model, work->position.x, (-20.0f) + work->position.y, modelDepth);
    Hu3DMotionSet(model, motion);
    Hu3DMotionSpeedSet(model, (1.0f));
    work->effectActive[0] = 1;
    work->effectActive[1] = 1;
    work->effectActive[2] = 1;
}

/* Each projectile frame hides completed target-contact effects independently. */
void fn_1_1430(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID model;
    if (work->effectActive[0] != 0) {
        model = obj->mdlId[3];
        if (Hu3DMotionEndCheck(model) != 0) {
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            work->effectActive[0] = 0;
        }
    }
    if (work->effectActive[1] != 0) {
        model = obj->mdlId[4];
        if (Hu3DMotionEndCheck(model) != 0) {
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            work->effectActive[1] = 0;
        }
    }
    if (work->effectActive[2] != 0) {
        model = obj->mdlId[5];
        if (Hu3DMotionEndCheck(model) != 0) {
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            work->effectActive[2] = 0;
        }
    }
}

/* Pool reuse shows the flight model and enables projectile collision checks. */
void fn_1_1514(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID primaryModel = obj->mdlId[0];
    Hu3DModelAttrReset(primaryModel, HU3D_ATTR_DISPOFF);
    work->anchor.kind |= M646_COLLISION_ACTIVE_FLAG;
}

/* Boundary and target contacts select flight or falling movement and disable collision checks. */
void fn_1_1570(OMOBJ *obj, s32 active)
{
    M646ActorState *work = obj->data;
    HU3D_MODELID primaryModel = obj->mdlId[0];
    /* Both movement modes stop further contacts for this flight. */
    work->motionActive = active;
    work->anchor.kind &= ~M646_COLLISION_ACTIVE_FLAG;
}

/* Contact handlers play a positional sound at the offset used by the projectile's impact
 * effects. */
void fn_1_15A4(OMOBJ *obj, s32 sound)
{
    M646ActorState *work = obj->data;
    HuVecF position;
    f32 modelDepth = (-40.0f) + work->position.z;
    position.x = work->position.x;
    position.y = (-20.0f) + work->position.y;
    position.z = modelDepth;
    fn_1_B3C0(&position, sound);
}

/* The projectile's first object callback loads its models and installs the flight update. */
void fn_1_1630(OMOBJ *obj)
{
    fn_1_BF8(obj);
    fn_1_D8C(obj);
    fn_1_FDC(obj);
    obj->objFunc = fn_1_1B08;
}

/* Each projectile frame checks the shared stage depth, advances flight or falling, and retires
 * effects. */
void fn_1_1B08(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    s32 stage;
    switch (work->state) {
        case 0:
            work->state++;
        case 1:
            for (stage = 0; stage < 6; stage++) {
                M646ActorStageWork *stageWork = lbl_1_bss_80->data;
                M646ActorStageSource *stageData = &stageWork->stages[stage];
                if (work->contactHandled == 0 &&
                    fn_1_48A4(&stageData->collision, &work->position)) {
                    /* Each stage uses the same sampled depth threshold in this test. */
                    OMOBJ *player = work->linkedObject;
                    M646PlayerRuntimeWork *playerWork = player->data;
                    M646ActorPlayerTarget *target = playerWork->targetData;
                    if (target) {
                        target->active = 0;
                    }
                    fn_1_E98(obj);
                    fn_1_15A4(obj, M646_STAGE_FALL_SE_ID);
                    fn_1_1570(obj, 0);
                    if (work->activeAttachmentContact == 0) {
                        Hu3DModelLayerSet(obj->mdlId[0], 7);
                        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_ZCMP_OFF);
                    }
                    break;
                }
            }
            break;
        case 2:
            break;
    }
    if (work->motionActive != 0) {
        work->position.x += work->direction.x;
        work->position.y += work->direction.y;
        /* Flight subtracts the stored Z step; falling adds that same component. */
        work->position.z -= work->direction.z;
    } else {
        work->direction.x *= (0.99);
        work->direction.y -= (1.3333334028720856);
        work->direction.z *= (0.8);
        PSVECAdd(&work->position, &work->direction, &work->position);
        if (work->position.y < (-115.0f)) {
            HU3D_MODELID model = obj->mdlId[0];
            work->direction.x = work->direction.y = work->direction.z = (0.0f);
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            Hu3DModelPosSet(model, (0.0f), (-1000.0f), (0.0f));
        }
    }
    fn_1_F68(obj);
    fn_1_1430(obj);
}

/* After collision checks, the manager places the flight model and saves its next swept-test
 * origin. */
void fn_1_1F24(OMOBJ *obj)
{
    M646ActorState *work = obj->data;
    Hu3DModelPosSetV(obj->mdlId[0], &work->position);
    work->anchor.savedPosition = work->position;
}

/* The collision manager reports target hits here; target category selects points, sound, and
 * effects. */
s32 fn_1_1F80(M646ActorAnchor *actor, M646ModelCollider *target)
{
    OMOBJ *obj = actor->owner;
    M646ActorState *work = obj->data;
    OMOBJ *player = work->linkedObject;
    M646PlayerWork *playerWork = NULL;
    M646ModelAttachment *attachment = target->owner;
    if (player == NULL) {
        return 0;
    }
    playerWork = player->data;
    if (attachment->state == 0) {
        switch (target->callbackType) {
            case 12:
                work->callbackType12Seen = 1;
                break;
            case 2:
                work->callbackType2Seen = 1;
                break;
            case 8:
            case 9:
            case 10:
            case 11:
                work->contactHandled = 1;
                break;
            case 3:
                fn_1_8F04(playerWork->playerIndex, 10);
                work->contactHandled = 1;
                break;
            case 4:
                fn_1_8F04(playerWork->playerIndex, 30);
                work->contactHandled = 1;
                break;
            case 5:
                fn_1_8F04(playerWork->playerIndex, 50);
                work->contactHandled = 1;
                break;
            case 6:
                fn_1_8F04(playerWork->playerIndex, 100);
                work->contactHandled = 1;
                break;
            case 7:
                fn_1_8F5C(playerWork->playerIndex);
                work->contactHandled = 1;
                break;
        }
        switch (target->callbackType) {
            case 3:
            case 4:
            case 5:
            case 8:
            case 9:
            case 10:
            case 11:
                fn_1_15A4(obj, M646_TARGET_CONTACT_SE_ID);
                break;
            case 6:
                fn_1_15A4(obj, M646_TARGET_SCORE_SE_ID);
                break;
            case 7:
                fn_1_15A4(obj, M646_TARGET_SCORE_CLEAR_SE_ID);
                break;
        }
        Hu3DModelLayerSet(obj->mdlId[0], 7);
        Hu3DModelAttrSet(obj->mdlId[0], HU3D_ATTR_ZCMP_OFF);
        fn_1_1290(obj);
        fn_1_1570(obj, 1);
    } else {
        /* A target already reacting consumes the shot without awarding another score. */
        work->anchor.kind &= ~M646_COLLISION_ACTIVE_FLAG;
        fn_1_1570(obj, 1);
        work->contactHandled = 1;
        work->activeAttachmentContact = 1;
    }
    return 1;
}

/* Reuses the next projectile at the launch position; phase is unused. A supplied direction is
 * copied with Z negated; NULL keeps the previous direction. */
OMOBJ *fn_1_23D0(Vec *position, f64 phase, Vec *direction, OMOBJ *player)
{
    OMOBJ *playerObject = player;
    M646ActorPlayerSource *playerWork = playerObject->data;
    M646PlayerWork *motion = &playerWork->motion;
    OMOBJ *actor = lbl_1_bss_30[motion->playerIndex][lbl_1_bss_28[motion->playerIndex]];
    M646ActorState *work = actor->data;
    HU3D_MODELID model = actor->mdlId[0];

    work->position = *position;
    Hu3DModelPosSetV(model, &work->position);
    if (direction != NULL) {
        work->direction = *direction;
        work->direction.z *= (-1.0f);
    }
    work->linkedObject = player;
    lbl_1_bss_28[motion->playerIndex]++;
    if (lbl_1_bss_28[motion->playerIndex] >= 3) {
        lbl_1_bss_28[motion->playerIndex] = 0;
    }
    fn_1_1514(actor);
    work->motionActive = 1;
    work->callbackType12Seen = 0;
    work->callbackType2Seen = 0;
    work->contactHandled = 0;
    work->activeAttachmentContact = 0;
    Hu3DModelLayerSet(model, 5);
    Hu3DModelAttrReset(model, HU3D_ATTR_ZCMP_OFF);
    omVibrate(playerWork->motion.playerIndex, 20, 4, 4);
    /* The launch callback receives the player object back, rather than the projectile. */
    return work->linkedObject;
}
