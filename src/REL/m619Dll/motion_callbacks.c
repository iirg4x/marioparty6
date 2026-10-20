/* Resolve linked motion contacts and run the minigame's callback jobs. */
#include "dolphin/math.h"
#include "REL/m619Dll/motion_system.h"

void fn_1_E4A4(M619MotionNode *node);

void fn_1_E05C(void) {
    lbl_1_bss_10 = NULL;
}

/* The scene object's per-frame update calls this to update participants and resolve overlaps. */
void fn_1_E070(void) {
    M619MotionNode *first;
    M619MotionNode *second;
    M619MotionNode *next;
    f32 distanceSquared;
    f32 radiusSquared;
    f32 distance;
    f32 radius;
    f32 deltaX;
    f32 deltaZ;

    first = lbl_1_bss_10;
    for (; first != 0; first = first->next) {
        first->previousPosition = *first->position;
        first->beforeUpdate(first->owner);
        first->deltaX = first->position->x - first->previousPosition.x;
        first->deltaZ = first->position->z - first->previousPosition.z;
    }

    for (first = lbl_1_bss_10; first != 0; first = first->next) {
        for (second = lbl_1_bss_10; second != 0; second = second->next) {
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

    first = lbl_1_bss_10;
    while (first != 0) {
        next = first->next;
        first->position->x = first->previousPosition.x + first->deltaX;
        first->position->z = first->previousPosition.z + first->deltaZ;
        if (first->afterUpdate(first->owner) != 0) {
            fn_1_E4A4(first);
        }
        first = next;
    }
}

/* Called when a minigame object joins collision handling; links its callbacks and position. */
void fn_1_E43C(M619MotionNode *node, void *owner,
               void (*beforeUpdate)(void *), s32 (*afterUpdate)(void *),
               void (*resolvePair)(M619MotionNode *, M619MotionNode *, f32),
               HuVecF *position) {
    node->next = lbl_1_bss_10;
    node->previous = NULL;
    lbl_1_bss_10 = node;
    if (node->next != 0) {
        node->next->previous = node;
    }
    node->owner = owner;
    node->beforeUpdate = beforeUpdate;
    node->afterUpdate = afterUpdate;
    node->resolvePair = resolvePair;
    node->position = position;
    node->previousPosition = *position;
}

/* Unlinks a collision participant when its movement or effect ends. */
void fn_1_E4A4(M619MotionNode *node) {
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        lbl_1_bss_10 = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
        node->next = NULL;
        if (node->previous) {
            node->previous = NULL;
        }
    }
}

void fn_1_E508(M619MotionNode *node, f32 radius, s32 kind) {
    node->radius = radius;
    node->kind = kind;
}

/* Scene initialization calls this to build the fixed pool for scene and player callbacks. */
void fn_1_E514(void) {
    M619Job *node;
    M619Jobs *jobs;
    int i;

    jobs = &lbl_1_bss_1414;
    node = lbl_1_bss_14;
    jobs->storage = node;
    jobs->freeHead = node;
    jobs->activeHead = NULL;
    for (i = 0; i < 255; i++, node++) {
        node->owner = jobs;
        node->previous = NULL;
        node->next = node + 1;
        node->data = NULL;
        node->callback = NULL;
    }
    node->owner = jobs;
    node->previous = NULL;
    node->next = NULL;
    node->data = NULL;
    node->callback = NULL;
}

/* Queues a scene or player callback when phase logic schedules an action. */
s32 fn_1_E5B8(s32 (*callback)(void *), void *data) {
    M619Job *node = NULL;
    M619Jobs *jobs;

    jobs = &lbl_1_bss_1414;
    node = jobs->freeHead;
    if (node) {
        jobs->freeHead = node->next;
        node->callback = callback;
        node->data = data;
        node->next = jobs->activeHead;
        if (node->next) {
            node->next->previous = node;
        }
        jobs->activeHead = node;
    }
    return (s32)node;
}

/* Returns a completed callback job to the free pool during job processing. */
void fn_1_E620(M619Job **job) {
    M619Job *node;
    M619Jobs *jobs;

    node = *job;
    jobs = node->owner;
    if (node->previous) {
        node->previous->next = node->next;
    } else {
        jobs->activeHead = node->next;
    }
    if (node->next) {
        node->next->previous = node->previous;
    }
    node->previous = NULL;
    node->next = jobs->freeHead;
    node->callback = NULL;
    node->data = NULL;
    jobs->freeHead = node;
    *job = NULL;
}

/* The scene object's per-frame update calls this and frees jobs whose callbacks are complete. */
void fn_1_E6AC(void) {
    M619Job *node;
    M619Jobs *jobs;
    M619Job *next;

    jobs = &lbl_1_bss_1414;
    node = jobs->activeHead;
    while (node) {
        next = node->next;
        if (node->callback && node->callback(node->data) != 0) {
            fn_1_E620(&node);
        }
        node = next;
    }
}

M619Jobs lbl_1_bss_1414;
M619Job lbl_1_bss_14[256];
M619MotionNode *lbl_1_bss_10;
