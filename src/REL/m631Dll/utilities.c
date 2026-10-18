/* Maintains collision callbacks and the shared pool of scheduled jobs. */
#include "dolphin/math.h"
#include "REL/m631Dll/utilities.h"

/* The scheduled entrance job returns the ground player to its looping idle motion when the entrance
 * animation ends. */
s32 fn_1_AAD8(void *callbackData) {
    M631ActorWork *actor = (M631ActorWork *) callbackData;
    if (Hu3DMotionEndCheck(actor->characterModelId) != 0) {
        CharMotionSet(actor->charNo, actor->idleMotionId);
        CharModelAttrReset(actor->charNo, HU3D_MOTATTR_PAUSE);
        CharModelAttrSet(actor->charNo, HU3D_MOTATTR_LOOP);
        return 1;
    }
    return 0;
}

/* Scene initialization clears the list of active movement and contact callbacks. */
void fn_1_AB4C(void) { lbl_1_bss_10 = NULL; }

void fn_1_AF94(M631MotionPairNode *);

/* Each scene update runs movement callbacks, resolves horizontal overlaps and removes completed
 * entries. */
void fn_1_AB60(void)
{
    M631MotionPairNode *first;
    M631MotionPairNode *second;
    M631MotionPairNode *next;
    f32 distanceSquared;
    f32 radiusSquared;
    f32 distance;
    f32 radius;
    f32 deltaX;
    f32 deltaZ;

    first = (M631MotionPairNode *)lbl_1_bss_10;
    for (; first != 0; first = first->next) {
        first->previousPosition = *first->position;
        first->beforeUpdate(first->owner);
        first->deltaX = first->position->x - first->previousPosition.x;
        first->deltaZ = first->position->z - first->previousPosition.z;
    }

    /* Each ordered pair gets its own response, so the two owners may react differently. */
    for (first = (M631MotionPairNode *)lbl_1_bss_10; first != 0;
         first = first->next) {
        for (second = (M631MotionPairNode *)lbl_1_bss_10; second != 0;
             second = second->next) {
            if (first == second) {
                continue;
            }
            radiusSquared = (first->radius + second->radius) *
                            (first->radius + second->radius);
            radius = sqrtf(radiusSquared);
            deltaX = second->position->x - first->position->x;
            deltaZ = second->position->z - first->position->z;
            distanceSquared = deltaX * deltaX + deltaZ * deltaZ;
            if (distanceSquared < radiusSquared) {
                distance = sqrtf(distanceSquared);
                first->resolvePair(first, second, radius - distance);
            }
        }
    }

    first = (M631MotionPairNode *)lbl_1_bss_10;
    while (first != 0) {
        /* Completion may unlink the current entry. */
        next = first->next;
        first->position->x = first->previousPosition.x + first->deltaX;
        first->position->z = first->previousPosition.z + first->deltaZ;
        if (first->afterUpdate(first->owner) != 0) {
            fn_1_AF94(first);
        }
        first = next;
    }
}

/* Registers an actor or effect's position and callbacks at the head of the active contact list. */
void fn_1_AF2C(M631MotionPairNode *object, void *owner, void (*beforeUpdate)(void *),
               s32 (*afterUpdate)(void *),
               void (*resolvePair)(M631MotionPairNode *, M631MotionPairNode *, f32),
               HuVecF *position)
{
    object->next = lbl_1_bss_10;
    object->previous = 0;
    lbl_1_bss_10 = object;
    if (object->next != 0) {
        object->next->previous = object;
    }
    object->owner = owner;
    object->beforeUpdate = beforeUpdate;
    object->afterUpdate = afterUpdate;
    object->resolvePair = resolvePair;
    object->position = position;
    object->previousPosition = *position;
}

/* Removes an actor or effect's contact entry when it finishes or the results transition clears the
 * arena. */
void fn_1_AF94(M631MotionPairNode *object)
{
    if (object->previous) {
        object->previous->next = object->next;
    } else {
        lbl_1_bss_10 = object->next;
    }
    if (object->next) {
        object->next->previous = object->previous;
        object->next = NULL;
        if (object->previous) {
            object->previous = NULL;
        }
    }
}

/* Sets the contact radius and kind before an actor or effect is registered or changes its contact
 * behavior. */
void fn_1_AFF8(M631MotionPairNode *object, f32 radius, s32 contactKind)
{
    object->radius = radius;
    object->state = contactKind;
}

/* Returns the active contact list for nearby-object checks and the results transition. */
M631MotionPairNode *fn_1_B004(void) { return lbl_1_bss_10; }

/* The scene's first update initializes the pool of 256 scheduled callback jobs. */
void fn_1_B014(void)
{
    M631Job *node;
    M631Jobs *jobs;
    int nodeIndex;
    jobs = &lbl_1_bss_1414;
    node = lbl_1_bss_14;
    jobs->storage = node;
    jobs->freeHead = node;
    jobs->activeHead = NULL;
    for (nodeIndex = 0; nodeIndex < 255; nodeIndex++, node++) {
        node->owner = jobs;
        node->prev = NULL;
        node->next = node + 1;
        node->data = NULL;
        node->callback = NULL;
    }
    node->owner = jobs;
    node->prev = NULL;
    node->next = NULL;
    node->data = NULL;
    node->callback = NULL;
}

/* Schedules a callback with its data, returning its job or NULL when all jobs are in use. */
void *fn_1_B0B8(s32 (*callback)(void *), void *data)
{
    M631Job *node = NULL;
    M631Jobs *jobs;
    jobs = &lbl_1_bss_1414;
    node = jobs->freeHead;
    if (node) {
        jobs->freeHead = node->next;
        node->callback = callback;
        node->data = data;
        node->next = jobs->activeHead;
        if (node->next) {
            node->next->prev = node;
        }
        jobs->activeHead = node;
    }
    return node;
}

/* Returns a completed job to the free list and clears the caller's job pointer. */
void fn_1_B120(M631Job **job)
{
    M631Job *node;
    M631Jobs *jobs;
    node = *job;
    jobs = node->owner;
    if (node->prev) {
        node->prev->next = node->next;
    } else {
        jobs->activeHead = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
    node->prev = NULL;
    node->next = jobs->freeHead;
    node->callback = NULL;
    node->data = NULL;
    jobs->freeHead = node;
    *job = NULL;
}

/* Each scene update runs scheduled jobs and recycles those whose callbacks report completion. */
void fn_1_B1AC(void)
{
    M631Job *node;
    M631Jobs *jobs;
    M631Job *next;
    jobs = &lbl_1_bss_1414;
    node = jobs->activeHead;
    while (node) {
        /* Save the next job before the scheduler recycles a completed job. */
        next = node->next;
        if (node->callback && node->callback(node->data) != 0) {
            fn_1_B120(&node);
        }
        node = next;
    }
}

M631Jobs lbl_1_bss_1414; /* Active and free job lists. */
M631Job lbl_1_bss_14[256]; /* Reusable job storage. */
M631MotionPairNode *lbl_1_bss_10; /* Head of the active contact list. */
