/* Coordinates player turns and chooses stage targets for human and computer players. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"

#define M646_COORDINATOR_WORK_BYTES 572

/* Absolute value keeps the random aim-tolerance sample nonnegative. */
extern int abs(int value);

typedef struct M646ActorAnchor_s M646ActorAnchor;
typedef struct M646ModelCollider_s M646ModelCollider;
typedef struct M646ModelAttachment_s M646ModelAttachment;

typedef struct M646MeshEdge_s {
    Vec first; /* First source-mesh endpoint in local coordinates. */
    Vec second; /* Second source-mesh endpoint in local coordinates. */
    Mtx *matrix; /* Matrix used to transform this edge into world coordinates. */
} M646MeshEdge;

typedef struct M646WorldEdge_s {
    Vec first; /* First endpoint in world coordinates. */
    Vec second; /* Second endpoint in world coordinates. */
    Vec normal; /* Plane normal used by contact and reflection tests. */
    f32 planeConstant; /* Signed plane offset in world units. */
} M646WorldEdge;

typedef struct M646CollisionRecord_s {
    u8 stageCollisionPrefix[32]; /* Collision header preceding the edge list. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* World edges used by this frame's contact tests. */
    M646WorldEdge *alternate; /* Spare world-edge buffer swapped during transforms. */
    M646MeshEdge *original; /* Endpoints retained in source-mesh coordinates. */
    s32 transformCount; /* Number of source objects supplying transforms. */
    HSF_TRANSFORM **transforms; /* Source-object transforms. */
    Mtx *matrices; /* Matrices applied to the source edges. */
} M646CollisionRecord;

struct M646ModelCollider_s {
    u32 kind; /* Collision registration and activity flags. */
    s32 group; /* Group bits used to filter projectile contacts. */
    s32 callbackType; /* Contact category selecting target points and impact sounds. */
    s32 unusedPlayerIndex; /* Player-index slot unused by stage targets. */
    s32 callbackMode; /* 11 selects the model-target contact test. */
    M646ModelAttachment *owner; /* Target attachment supplied to the callbacks. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(M646ModelCollider *, M646ActorAnchor *); /* Handles a projectile hit. */
    s32 edgeCount; /* Number of collider edges. */
    M646WorldEdge *current; /* Edges used by this frame's contact test. */
    M646WorldEdge *alternate; /* Spare world-edge buffer. */
    s32 transformCount; /* Number of collider transforms. */
    HSF_TRANSFORM pose; /* Target joint position, rotation, and scale. */
    Mtx *matrices; /* Matrices applied to collider geometry. */
    u8 contactEdgeStorage[4]; /* Storage for the edge selected during a contact test. */
    s16 category; /* Index selecting a category-specific contact distance. */
    HU3D_MODELID model; /* Carrier model containing the target joint. */
    char jointName[64]; /* Carrier joint followed by this target. */
    HSF_OBJECT *source; /* Hidden mesh used during collider setup. */
    s32 assigned; /* Nonzero while a computer player has reserved this target. */
};

struct M646ModelAttachment_s {
    HU3D_MODELID parentModel; /* Hook parent, or -1 after the first projectile contact. */
    HU3D_MODELID model; /* Visible target model. */
    HU3D_MOTIONID motion; /* Hit animation, paused until the first contact. */
    s16 state; /* Number of contacts; zero means still attached and unhit. */
    M646ModelCollider collider; /* Contact geometry and callbacks for this target. */
    Vec position; /* World position captured when the target detaches. */
};

typedef struct M646MovingStageState_s {
    HU3D_MODELID model; /* Visible layout hooked to its carrier. */
    HU3D_MODELID hiddenModel; /* Invisible model supplying collision meshes. */
    HU3D_MODELID parentModel; /* Carrier model holding this layout. */
    s16 modelPadding; /* Storage between the model IDs and the layout kind. */
    u32 stageKind; /* Descriptor index: 0-4 normal, 5 opening, 6 special. */
    M646ModelAttachment attachments[5][12]; /* Targets grouped by contact-distance category. */
    s16 jointIndex[5]; /* Next collider name to read in each target group. */
    u8 jointIndexPadding[2]; /* Storage after the collider-name indices. */
    M646CollisionRecord collision; /* Edge geometry for the layout's stage surface. */
    u8 reservedStageStorage[104]; /* Remaining storage not accessed by the stage callbacks. */
} M646MovingStageState;

typedef struct M646MovingStageGroup_s {
    char **attachmentNames; /* Hook names for the visible targets. */
    char **colliderNames; /* Mesh names in the hidden collision model. */
    s16 attachmentCount; /* Number of targets created in this group. */
    u8 attachmentCountPadding[2]; /* Storage between target count and contact category. */
    s32 callbackType; /* Base contact category for targets in this group. */
} M646MovingStageGroup;

typedef struct M646MovingStageDescriptor_s {
    s32 modelFile; /* DATA_m646 resource number for the visible layout. */
    s32 hiddenModelFile; /* DATA_m646 resource number for its collision meshes. */
    char *hiddenHook; /* Visible-model joint holding the hidden collision model. */
    M646MovingStageGroup groups[5]; /* Target hooks and collision categories for this layout. */
} M646MovingStageDescriptor;

typedef struct M646MovingStageWork_s {
    s32 state; /* Cleared at stage startup; not read by stage callbacks. */
    s32 stageOrder[6]; /* Opening layout followed by five randomized layouts. */
    M646MovingStageState stages[6]; /* Loaded layouts and targets for the six carriers. */
} M646MovingStageWork;

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
    Vec *rotation; /* Player rotation supplied as the collision direction buffer. */
    Vec savedPosition; /* Previous position restored if a collision update produces invalid
                        * coordinates. */
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
    u8 itemWorkTail[12]; /* Storage between item animation and controller state. */
    s16 padIndex; /* Controller port used by this player. */
    u8 padIndexPadding[2]; /* Storage after the controller index. */
    f32 stickOffsetX; /* Horizontal aim displacement for this frame, in world units. */
    f32 stickOffsetY; /* Vertical aim displacement for this frame, in world units. */
    u8 buttonStorage[4]; /* Storage for the buttons pressed this frame. */
    s32 playerKind; /* Zero for a human player; nonzero for a computer player. */
    s16 comDifficulty; /* Computer difficulty, 0-3; -1 for a human player. */
    u8 difficultyPadding[2]; /* Storage between difficulty and targeting flags. */
    s32 targetReady; /* Nonzero while a computer player's target is active. */
    s32 motionReady; /* Nonzero while the computer aim is moving toward its target. */
    void *targetData; /* Chosen attachment, or NULL when aiming at stored coordinates. */
    f32 aimError; /* Extra arrival tolerance in world units. */
    f32 randomX; /* Horizontal error added to the target, in world units. */
    f32 randomY; /* Vertical error added to the target, in world units. */
    s16 targetDelayTicks; /* Remaining target-selection attempts to wait; not a frame count. */
    s16 shotCount; /* Number of shots fired by the computer player. */
    f32 targetX; /* Horizontal fallback aim coordinate in world units. */
    f32 targetY; /* Vertical fallback aim coordinate in world units. */
    M646PlayerAnchor anchor; /* Player's record registered with collision handling. */
    u8 playerWorkTail[104]; /* Storage following the collision anchor. */
} M646PlayerFullWork;

typedef struct M646TargetCandidate_s {
    M646ModelAttachment *attachment; /* Chosen target, or NULL for random free aim. */
    s16 category; /* Contact-distance category of the target. */
    u8 categoryPadding[2]; /* Storage between the category and fallback coordinates. */
    f32 x; /* Fallback X coordinate in world units; ignored when attachment is non-NULL. */
    f32 y; /* Fallback Y coordinate in world units; ignored when attachment is non-NULL. */
} M646TargetCandidate;

typedef struct M646AIProfile_s {
    s16 aimErrorScale; /* Arrival-tolerance scale in world units. */
    s16 delay; /* Frames between target selections for strategies that wait. */
    s16 unusedProfilePrefix; /* Zero in every profile; not read by these selectors. */
    s16 unusedProfileValue; /* Difficulty-specific value not read by these selectors. */
    s16 unusedProfileSuffix; /* Zero after the unused value; not read by these selectors. */
    s16 unusedProfileTail; /* Final zero entry; not read by these selectors. */
} M646AIProfile;

typedef struct M646CoordinatorWork_s {
    M646PlayerTicket *players[4]; /* Tickets connecting target selection to each player object. */
    s16 phase; /* Phase within the coordinator's current callback. */
    u8 phasePadding[2]; /* Storage between phase and the introduction frame counter. */
    s32 frames; /* Frames since the last introduction launch or placement. */
    M646TargetCandidate candidates[17]; /* Available attachment candidates for computer aim. */
    s32 candidateCount; /* Number of entries collected in the candidate array. */
    M646TargetCandidate *categories[5][12]; /* Candidates grouped by contact-distance category. */
    s32 categoryCount[5]; /* Candidate count in each category. */
    s32 completedTicketCount; /* Number of player targets assigned since the last collection. */
    s32 introComplete; /* Nonzero after all four opening shots have launched. */
    s32 resultTurnComplete; /* Nonzero after every player's result-facing turn finishes. */
} M646CoordinatorWork;

extern OMOBJ *lbl_1_bss_C0;
extern OMOBJ *lbl_1_bss_80;
extern M646MovingStageDescriptor lbl_1_data_1AB4[7];
extern M646AIProfile lbl_1_data_1DC8[4];
void fn_1_9978(OMOBJ *obj);
void fn_1_99A4(OMOBJ *obj);
void fn_1_9CD8(OMOBJ *obj);
void fn_1_9E3C(OMOBJ *obj);
void fn_1_9F80(OMOBJ *obj);
void fn_1_A124(void);
void fn_1_A158(void);
s32 fn_1_A34C(const M646PlayerTicket *ticket);
void fn_1_A4E8(M646PlayerTicket *ticket);
void fn_1_A85C(M646PlayerTicket *ticket, M646TargetCandidate *candidate);
M646TargetCandidate *fn_1_A8E0(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_ABB0(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_AE64(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_B07C(M646PlayerTicket *ticket);
f32 fn_1_B2BC(M646PlayerTicket *ticket);
f64 fn_1_2968(Vec lhs, Vec rhs);
void fn_1_1E4(s32 camera);
void fn_1_81FC(s16 index, Vec *out);
void fn_1_59E0(s16 player, Vec *position);
s32 fn_1_2CC(void);
void fn_1_5850(s16 player);
s32 fn_1_57DC(s16 player);
void fn_1_5964(s16 player, s32 resetAttribute);
void fn_1_589C(s16 player);
void fn_1_5634(OMOBJ *obj);
s32 fn_1_5694(OMOBJ *obj);
void fn_1_5A64(OMOBJ *obj, s16 motion);

void fn_1_55A0(void);
s32 fn_1_9380(void);

void fn_1_98E0(OMOBJMAN *objman);
void fn_1_9978(OMOBJ *obj);
void fn_1_99A4(OMOBJ *obj);
void fn_1_9CD8(OMOBJ *obj);
void fn_1_9E3C(OMOBJ *obj);
void fn_1_9F80(OMOBJ *obj);
void fn_1_A028(OMOBJ *obj);
s32 fn_1_A0D4(void);
s32 fn_1_A0FC(void);
void fn_1_A124(void);
void fn_1_A158(void);
s32 fn_1_A34C(const M646PlayerTicket *ticket);
void fn_1_A3AC(M646PlayerTicket *ticket, M646TargetCandidate *candidate);
void fn_1_A4E8(M646PlayerTicket *ticket);
void fn_1_A85C(M646PlayerTicket *ticket, M646TargetCandidate *candidate);
M646TargetCandidate *fn_1_A8E0(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_ABB0(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_AE64(M646PlayerTicket *ticket);
M646TargetCandidate *fn_1_B07C(M646PlayerTicket *ticket);
f32 fn_1_B2BC(M646PlayerTicket *ticket);

OMOBJ *lbl_1_bss_C0;

/* Per-difficulty delay and arrival tolerance, followed by unused profile entries. */
M646AIProfile lbl_1_data_1DC8[GW_PLAYER_COM_DIF_MAX] = {
    {50, 40, 0, 560, 0, 0},
    {60, 30, 0, 720, 0, 0},
    {60, 10, 0, 1661, 0, 0},
    {50, 5, 0, 2350, 0, 0},
};

/* Minigame setup creates the coordinator for the opening shots, computer aim, and result turns. */
void fn_1_98E0(OMOBJMAN *objman)
{
    OMOBJ *obj;
    void *work;
    obj = lbl_1_bss_C0 = omAddObjEx(objman, 70, 0, 0, OM_GRP_NONE, fn_1_9978);
    obj->stat |= OM_STAT_MODELPAUSE;
    work = obj->data = HuMemDirectMallocNum(HEAP_HEAP, M646_COORDINATOR_WORK_BYTES, HU_MEMNUM_OVL);
    memset(work, 0, M646_COORDINATOR_WORK_BYTES);
}

/* Coordinator startup resets its phase and installs the per-frame introduction callback. */
void fn_1_9978(OMOBJ *obj)
{
    M646CoordinatorWork *work = obj->data;
    work->phase = 0;
    obj->objFunc = fn_1_99A4;
}

/* Introduction callback aims all four players, launches their shots in order, then waits for
 * Start. */
void fn_1_99A4(OMOBJ *obj)
{
    M646CoordinatorWork *work = obj->data;
    s16 playerIndex;
    Vec position;
    switch (work->phase) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_FADEIN) {
                work->introComplete = 0;
                work->phase++;
            }
            break;
        case 1:
            fn_1_1E4(0);
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                fn_1_81FC(playerIndex, &position);
                fn_1_59E0(playerIndex, &position);
            }
            work->frames = 0;
            work->phase++;
            break;
        case 2:
            if (fn_1_2CC()) {
                work->phase++;
            }
            break;
        case 3:
            fn_1_5850(0);
            work->phase++;
            break;
        case 4:
            if (fn_1_57DC(0)) {
                fn_1_5964(0, 1);
                work->phase++;
            }
            break;
        case 5:
            fn_1_589C(0);
            work->frames = 0;
            work->phase++;
            break;
        case 6:
            if (work->frames > 22) {
                work->phase++;
            }
            break;
        case 7:
            fn_1_5850(1);
            work->phase++;
            break;
        case 8:
            if (fn_1_57DC(1)) {
                fn_1_5964(1, 1);
                work->phase++;
            }
            break;
        case 9:
            fn_1_589C(1);
            work->frames = 0;
            work->phase++;
            break;
        case 10:
            if (work->frames > 22) {
                work->phase++;
            }
            break;
        case 11:
            fn_1_5850(2);
            work->phase++;
            break;
        case 12:
            if (fn_1_57DC(2)) {
                fn_1_5964(2, 1);
                work->phase++;
            }
            break;
        case 13:
            fn_1_589C(2);
            work->frames = 0;
            work->phase++;
            break;
        case 14:
            if (work->frames > 22) {
                work->phase++;
            }
            break;
        case 15:
            fn_1_5850(3);
            work->phase++;
            break;
        case 16:
            if (fn_1_57DC(3)) {
                fn_1_5964(3, 1);
                work->phase++;
            }
            break;
        case 17:
            fn_1_589C(3);
            work->frames = 0;
            work->phase++;
            break;
        case 18:
            /* The final opening shot gets ten more wait frames than the first three. */
            if (work->frames > 32) {
                work->phase++;
            }
            break;
        case 19:
            work->introComplete = 1;
            work->phase++;
            break;
        case 20:
            if (MgSeqModeGet() == MGSEQ_MODE_START) {
                work->phase = 0;
                obj->objFunc = fn_1_9CD8;
            }
            break;
    }
    work->frames++;
}

/* Main-play callback collects targets for idle computer players and hands off to result turns. */
void fn_1_9CD8(OMOBJ *obj)
{
    M646CoordinatorWork *work = obj->data;
    M646PlayerTicket **slot = NULL;
    s32 playerIndex = 0;
    if (MgSeqModeGet() > MGSEQ_MODE_MAIN) {
        work->phase = 0;
        obj->objFunc = fn_1_9E3C;
    }
    switch (work->phase) {
        case 0:
            if (MgSeqModeGet() == MGSEQ_MODE_MAIN) {
                work->phase++;
            }
            break;
        case 1:
            fn_1_A124();
            fn_1_A158();
            work->phase++;
            break;
        case 2:
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                slot = &work->players[playerIndex];
                if ((*slot)->playerKind && fn_1_A34C(*slot)) {
                    fn_1_A4E8(*slot);
                }
            }
            work->phase++;
            break;
        case 3:
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                slot = &work->players[playerIndex];
                if ((*slot)->playerKind && fn_1_A34C(*slot)) {
                    work->phase = 1;
                }
            }
            break;
    }
}

/* Pre-winner callback turns all players to +180 or -180 degrees and signals when every turn
 * ends. */
void fn_1_9E3C(OMOBJ *obj)
{
    M646CoordinatorWork *work = obj->data;
    s32 playerIndex;
    M646PlayerTicket **slot = NULL;
    s32 finishedPlayerCount;
    switch (work->phase) {
        case 0:
            work->resultTurnComplete = 0;
            if (MgSeqModeGet() == MGSEQ_MODE_PREWIN) {
                for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                    slot = &work->players[playerIndex];
                    fn_1_5634((*slot)->playerObject);
                }
                work->phase++;
            }
            break;
        case 1:
            finishedPlayerCount = 0;
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                slot = &work->players[playerIndex];
                if (fn_1_5694((*slot)->playerObject) == 0) {
                    finishedPlayerCount++;
                }
            }
            if (finishedPlayerCount == 4) {
                work->phase++;
            }
            break;
        case 2:
            for (playerIndex = 0; playerIndex < 4; playerIndex++) {
                slot = &work->players[playerIndex];
                fn_1_5A64((*slot)->playerObject, 0);
            }
            work->resultTurnComplete = 1;
            work->phase = 0;
            obj->objFunc = fn_1_9F80;
            break;
    }
}

/* Winner callback selects result motions and a score-dependent jingle when Decathlon is
 * inactive. */
void fn_1_9F80(OMOBJ *obj)
{
    M646CoordinatorWork *work = obj->data;
    s32 playerIndex = 0;

    switch (work->phase) {
    case 0:
        if (MgSeqModeGet() == MGSEQ_MODE_WINNER) {
            if (_CheckFlag(FLAG_INST_DECA) == 0) {
                fn_1_55A0();
                if (fn_1_9380() == 0) {
        HuAudJinglePlay(MSM_STREAM_MGMUS_17);
                } else {
        HuAudJinglePlay(MSM_STREAM_FILESEL);
                }
            }
            work->phase++;
        }
        break;
    case 1:
        break;
    }
}

/* Player initialization allocates its target-selection ticket while the coordinator phase is
 * zero. */
void fn_1_A028(OMOBJ *obj)
{
    M646PlayerTicket **ticket;
    M646PlayerFullWork *player;
    M646CoordinatorWork *coordinator;
    s32 *playerKind;

    player = obj->data;
    playerKind = &player->playerKind;
    coordinator = lbl_1_bss_C0->data;
    ticket = &coordinator->players[player->aiming.motion.playerIndex];
    if (coordinator->phase == 0) {
        /* The null comparison is discarded; a fresh ticket replaces any existing ticket. */
        (*ticket != NULL);
        *ticket = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M646PlayerTicket), HU_MEMNUM_OVL);
        if (*ticket) {
            (*ticket)->playerObject = obj;
            (*ticket)->playerKind = *playerKind;
        }
    }
}

/* The introduction sequence queries whether the four opening shots have finished their waits. */
s32 fn_1_A0D4(void)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    return work->introComplete;
}

/* The pre-winner sequence waits on this flag before displaying the winner message. */
s32 fn_1_A0FC(void)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    return work->resultTurnComplete;
}

/* Main-play target collection resets the candidate and assigned-ticket counts before scanning. */
void fn_1_A124(void)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    work->candidateCount = 0;
    work->completedTicketCount = 0;
}

/* Main-play collection groups unhit, unreserved targets between Y=176 and Y=620 world units. */
void fn_1_A158(void)
{
    M646MovingStageWork *moving = lbl_1_bss_80->data;
    M646MovingStageState *stage = NULL;
    M646MovingStageDescriptor *descriptor = NULL;
    M646MovingStageGroup *descriptorGroup = NULL;
    M646ModelCollider *collider = NULL;
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    M646TargetCandidate *candidate = NULL;
    M646ModelAttachment *attachment;
    s32 category;
    s16 stageIndex;
    s16 jointIndex;
    s32 running = 1;
    memset(work->categories, 0, sizeof(work->categories));
    memset(work->categoryCount, 0, sizeof(work->categoryCount));
    while (running) {
        for (stageIndex = 1; stageIndex < 6; stageIndex++) {
            stage = &moving->stages[stageIndex];
            descriptor = &lbl_1_data_1AB4[stage->stageKind];
            for (category = 0; category < 5; category++) {
                descriptorGroup = &descriptor->groups[category];
                for (jointIndex = 0; jointIndex < stage->jointIndex[category]; jointIndex++) {
                    attachment = &stage->attachments[category][jointIndex];
                    if (attachment->state != 0 || attachment->collider.assigned != 0) {
                        continue;
                    }
                    candidate = &work->candidates[work->candidateCount];
                    candidate->attachment = attachment;
                    collider = &attachment->collider;
                    if (collider->pose.pos.y > (620.0f)) {
                        continue;
                    }
                    if (collider->pose.pos.y < (176.0f)) {
                        continue;
                    }
                    candidate->category = category;
                    candidate->attachment->collider.assigned = 0;
                    work->candidateCount++;
                    work->categories[category][work->categoryCount[category]] = candidate;
                    work->categoryCount[category]++;
                    if (work->candidateCount >= 17) {
                        /* This limit exits the current attachment loop; later groups still run. */
                        running = 0;
                        break;
                    }
                }
            }
        }
        running = 0;
    }
}

/* Main-play scheduling requests a target only when both of a computer player's active flags are
 * clear. */
s32 fn_1_A34C(const M646PlayerTicket *ticket)
{
    M646CoordinatorWork *coordinator = lbl_1_bss_C0->data;
    if (ticket->playerKind != 0) {
        M646PlayerFullWork *player = ticket->playerObject->data;
        if (player->targetReady == 0 && player->motionReady == 0) {
            return 1;
        }
    }
    return 0;
}

/* The two easier strategies use this free-aim target instead of following a stage attachment. */
void fn_1_A3AC(M646PlayerTicket *ticket, M646TargetCandidate *candidate)
{
    M646PlayerFullWork *player = ticket->playerObject->data;
    s32 limit;
    candidate->attachment = NULL;
    candidate->category = 0;
    limit = 813;
    candidate->x = frand() % limit;
    candidate->x += (-404.0f);
    limit = 412;
    candidate->y = frand() % limit;
    candidate->y += (192.0f);
    player->targetDelayTicks = lbl_1_data_1DC8[player->comDifficulty].delay;
    player->randomX = player->randomY = (0.0f);
    player->aimError = (10.0f);
}

/* Main-play scheduling selects a target by difficulty, using shot count to mix in free aim. */
void fn_1_A4E8(M646PlayerTicket *ticket)
{
    M646TargetCandidate *candidate = NULL;
    M646PlayerFullWork *player = ticket->playerObject->data;
    M646TargetCandidate randomCandidate = { 0 };
    s16 decision;
    /* A free-aim turn can be selected even while the two easier strategies' delay is nonzero. */
    switch (player->comDifficulty) {
        case 0:
            if (player->targetDelayTicks > 0) {
                player->targetDelayTicks--;
                candidate = NULL;
            }
            decision = player->shotCount % 5;
            if (decision == 0) {
                fn_1_A3AC(ticket, &randomCandidate);
                candidate = &randomCandidate;
            } else {
                candidate = fn_1_A8E0(ticket);
            }
            break;
        case 1:
            if (player->targetDelayTicks > 0) {
                player->targetDelayTicks--;
                candidate = NULL;
            }
            decision = player->shotCount % 3;
            if (decision == 0) {
                fn_1_A3AC(ticket, &randomCandidate);
                candidate = &randomCandidate;
            } else {
                candidate = fn_1_ABB0(ticket);
            }
            break;
        case 2:
            candidate = fn_1_AE64(ticket);
            break;
        case 3:
            candidate = fn_1_B07C(ticket);
            break;
    }
    if (candidate) {
        fn_1_A85C(ticket, candidate);
    }
}

/* After strategy selection, starts the player's aim and reserves its chosen attachment if
 * present. */
void fn_1_A85C(M646PlayerTicket *ticket, M646TargetCandidate *candidate)
{
    M646CoordinatorWork *coordinator = lbl_1_bss_C0->data;
    M646PlayerFullWork *player = ticket->playerObject->data;
    M646ModelAttachment *target = candidate->attachment;
    player->targetData = candidate->attachment;
    player->targetReady = 1;
    player->motionReady = 1;
    /* Attachment targets follow their joint position and ignore these fallback coordinates. */
    player->targetX = candidate->x;
    player->targetY = candidate->y;
    if (target) {
        target->collider.assigned = 1;
    }
    coordinator->completedTicketCount++;
}

/* Easy target selection waits out its delay, then tries low categories first using X/Y distance. */
M646TargetCandidate *fn_1_A8E0(M646PlayerTicket *ticket)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    M646TargetCandidate *selected = NULL;
    M646PlayerFullWork *player = ticket->playerObject->data;
    M646ModelCollider *collider = NULL;
    s16 category = 0;
    s16 joint = 0;
    f64 bestDistance = 10000.0;
    s16 index;
    s16 filteredCount = 0;
    s16 shotParity = player->shotCount % 2;
    Vec playerPosition;
    Vec targetPosition;
    M646TargetCandidate *filtered[12];
    f64 distance;
    if (player->targetDelayTicks > 0) {
        return NULL;
    }
    HuCopyVecF(&playerPosition, &player->modelPosition);
    for (category = 0; category < 4; category++) {
        selected = NULL;
        for (joint = 0; joint < work->categoryCount[category]; joint++) {
            filtered[filteredCount] = work->categories[category][joint];
            if (filtered[filteredCount]->attachment->collider.assigned == 0) {
                filteredCount++;
            }
        }
        for (index = 0; index < filteredCount; index++) {
            collider = &filtered[index]->attachment->collider;
            HuCopyVecF(&targetPosition, &collider->pose.pos);
            distance = fn_1_2968(targetPosition, playerPosition);
            if (distance < bestDistance) {
                bestDistance = distance;
                selected = filtered[index];
            }
        }
        if (selected) {
            player->aimError = fn_1_B2BC(ticket);
            player->targetDelayTicks = lbl_1_data_1DC8[player->comDifficulty].delay;
            player->randomX = frand() % 50;
            player->randomY = frand() % 40;
            return selected;
        }
    }
    return NULL;
}

/* Normal target selection tries low categories first, with less random aim offset than Easy. */
M646TargetCandidate *fn_1_ABB0(M646PlayerTicket *ticket)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    M646TargetCandidate *selected = NULL;
    M646PlayerFullWork *player = ticket->playerObject->data;
    M646ModelCollider *collider = NULL;
    s16 category = 0;
    s16 joint = 0;
    f64 bestDistance = 10000.0;
    s16 index;
    s16 filteredCount = 0;
    Vec playerPosition;
    Vec targetPosition;
    M646TargetCandidate *filtered[12];
    f64 distance;
    if (player->targetDelayTicks > 0) {
        return NULL;
    }
    HuCopyVecF(&playerPosition, &player->modelPosition);
    for (category = 0; category < 4; category++) {
        selected = NULL;
        for (joint = 0; joint < work->categoryCount[category]; joint++) {
            filtered[filteredCount] = work->categories[category][joint];
            if (filtered[filteredCount]->attachment->collider.assigned == 0) {
                filteredCount++;
            }
        }
        for (index = 0; index < filteredCount; index++) {
            collider = &filtered[index]->attachment->collider;
            HuCopyVecF(&targetPosition, &collider->pose.pos);
            distance = fn_1_2968(targetPosition, playerPosition);
            if (distance < bestDistance) {
                bestDistance = distance;
                selected = filtered[index];
            }
        }
        if (selected) {
            player->aimError = fn_1_B2BC(ticket);
            player->targetDelayTicks = lbl_1_data_1DC8[player->comDifficulty].delay;
            player->randomX = frand() % 30;
            player->randomY = frand() % 30;
            return selected;
        }
    }
    return NULL;
}

/* Hard target selection counts down its delay, then finds the nearest target to the X/Y origin. */
M646TargetCandidate *fn_1_AE64(M646PlayerTicket *ticket)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    M646TargetCandidate *selected = NULL;
    M646PlayerFullWork *player = ticket->playerObject->data;
    Vec origin = { 0, 0, 0 };
    s16 category = 0;
    s16 joint = 0;
    M646ModelCollider *collider = NULL;
    f64 bestDistance = 10000.0;
    M646TargetCandidate *candidate;
    Vec targetPosition;
    f64 distance;
    if (player->targetDelayTicks > 0) {
        player->targetDelayTicks--;
        if (player->targetDelayTicks < 0) {
            player->targetDelayTicks = 0;
        }
        return NULL;
    }
    for (category = 3; category >= 0; category--) {
        for (joint = 0; joint < work->categoryCount[category]; joint++) {
            candidate = work->categories[category][joint];
            if (candidate->attachment->collider.assigned == 0) {
                collider = &candidate->attachment->collider;
                HuCopyVecF(&targetPosition, &collider->pose.pos);
                distance = fn_1_2968(targetPosition, origin);
                if (distance < bestDistance) {
                    bestDistance = distance;
                    selected = candidate;
                }
            }
        }
    }
    if (selected) {
        player->aimError = fn_1_B2BC(ticket);
        player->targetDelayTicks = lbl_1_data_1DC8[player->comDifficulty].delay;
        player->randomX = (0.0f);
        player->randomY = (0.0f);
        return selected;
    }
    return NULL;
}

/* Very Hard selection ignores its delay counter and tries high categories first using X/Y
 * distance. */
M646TargetCandidate *fn_1_B07C(M646PlayerTicket *ticket)
{
    M646CoordinatorWork *work = lbl_1_bss_C0->data;
    M646TargetCandidate *selected = NULL;
    M646PlayerFullWork *player = ticket->playerObject->data;
    M646ModelCollider *collider = NULL;
    s16 category = 0;
    s16 joint = 0;
    f64 bestDistance = 10000.0;
    s16 index;
    s16 filteredCount = 0;
    Vec playerPosition;
    Vec targetPosition;
    M646TargetCandidate *filtered[12];
    f64 distance;
    HuCopyVecF(&playerPosition, &player->modelPosition);
    for (category = 3; category >= 0; category--) {
        selected = NULL;
        for (joint = 0; joint < work->categoryCount[category]; joint++) {
            filtered[filteredCount] = work->categories[category][joint];
            if (filtered[filteredCount]->attachment->collider.assigned == 0) {
                filteredCount++;
            }
        }
        for (index = 0; index < filteredCount; index++) {
            collider = &filtered[index]->attachment->collider;
            HuCopyVecF(&targetPosition, &collider->pose.pos);
            distance = fn_1_2968(targetPosition, playerPosition);
            if (distance < bestDistance) {
                bestDistance = distance;
                selected = filtered[index];
            }
        }
        if (selected) {
            player->aimError = fn_1_B2BC(ticket);
            player->targetDelayTicks = lbl_1_data_1DC8[player->comDifficulty].delay;
            player->randomX = (0.0f);
            player->randomY = (0.0f);
            return selected;
        }
    }
    return NULL;
}

/* Attachment selection samples an arrival tolerance from 0.0 to 0.9 times the difficulty scale. */
f32 fn_1_B2BC(M646PlayerTicket *ticket)
{
    M646PlayerFullWork *player = ticket->playerObject->data;
    s32 randomSample = abs(frand());
    f32 arrivalTolerance;
    f32 toleranceFraction;
    randomSample %= 10;
    toleranceFraction = 0.1f * randomSample;
    arrivalTolerance = toleranceFraction * lbl_1_data_1DC8[player->comDifficulty].aimErrorScale;
    return arrivalTolerance;
}
