/* Runs Banana Shake's registered collision checks and contact callbacks. */
#include "REL/m675Dll/module_types.h"
#include "REL/m675Dll/actor_state.h"
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "game/charman.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/mg/timer.h"
#include "game/object.h"
#include "game/wipe.h"
#include "string.h"

s32 fn_1_F110(void) { return 0; }

typedef struct Position {
    HuVecF position;
    unsigned char unmappedBytes[12];
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;
extern float lbl_1_bss_1BC;
extern s8 lbl_1_bss_218, lbl_1_bss_219, lbl_1_data_2758;
s32 fn_1_367CC(s32 *player);

OMOBJ *lbl_1_bss_548;
void fn_1_F1A4(OMOBJ *obj);

/* Creates the collision manager and its registration table during minigame setup. */
void fn_1_F118(HUPROCESS *process)
{
    OMOBJ *object;
    void *managerData;

    object = fn_8003AB38(process, 18, 0, 0, -1, fn_1_F1A4);
    lbl_1_bss_548 = object;
    managerData = HuMemDirectMallocNum(HEAP_HEAP, 384, 0x10000000);
    object->data = managerData;
    memset(managerData, 0, 384);
}

void fn_1_F1A0(void)
{
}

void fn_1_F268(OMOBJ *obj);

/* The manager's first object update installs the per-frame collision callback. */
void fn_1_F1A4(OMOBJ *obj)
{
    obj->objFunc = fn_1_F268;
}

typedef struct M675EntryWork_s {
    s32 initializationState; /* Startup phase used by fn_1_F1B4. */
    s32 initializationTimer; /* Startup delay in object updates. */
    M675Collision *collisionEntries[84]; /* Moving-object registrations; NULL means free. */
    M675Collision *regionEntries[10]; /* Region registrations; NULL means free. */
} M675EntryWork;

typedef struct M675CollisionObject_s M675CollisionObject;
typedef struct M675CollisionRegion_s M675CollisionRegion;
typedef struct M675BoundaryObject_s M675BoundaryObject;
typedef struct M675EllipseBoundary_s M675EllipseBoundary;
s32 fn_1_F4FC(M675Collision *first, M675Collision *second);
s32 fn_1_F8AC(M675CollisionObject *object, M675CollisionRegion *region);
s32 fn_1_FE84(M675BoundaryObject *state, M675EllipseBoundary *parameters);

/* This alternate manager callback waits three updates, runs 180 contact passes, then one per
 * update. */
void fn_1_F1B4(OMOBJ *object)
{
    M675EntryWork *work;
    s32 index;

    work = object->data;
    switch (work->initializationState) {
    case 0:
        work->initializationState++;
        work->initializationTimer = 0;
    case 1:
        if (++work->initializationTimer >= 3) {
            work->initializationState++;
            for (index = 0; index < 180; index++) {
                fn_1_F268(object);
            }
        }
        break;
    default:
        fn_1_F268(object);
        break;
    }
}

static inline void DebugValue(int hasNonSphereShape)
{
}

/* Runs on each collision-manager update to compare registered pairs and call their handlers. */
void fn_1_F268(OMOBJ *obj)
{
    M675EntryWork *work;
    M675Collision *firstEntry;
    M675Collision *secondEntry;
    M675Collision *current;
    M675Collision *candidate;
    s32 entryIndex;
    s32 candidateIndex;

    work = obj->data;
    for (entryIndex = 0; entryIndex < 84; entryIndex++) {
        firstEntry = work->collisionEntries[entryIndex];
        if (firstEntry != NULL) {
            /* This preliminary shape check discards its result. */
            DebugValue(firstEntry->shapeType != 0);
        }
    }

    for (entryIndex = 0; entryIndex < 84; entryIndex++) {
        firstEntry = work->collisionEntries[entryIndex];
        if (firstEntry == NULL) {
            continue;
        }
        current = firstEntry;
        if ((current->flags & 1) == 0 || current->shapeType != 0) {
            continue;
        }
        for (candidateIndex = entryIndex + 1; candidateIndex < 84; candidateIndex++) {
            secondEntry = work->collisionEntries[candidateIndex];
            if (secondEntry == NULL) {
                continue;
            }
            candidate = secondEntry;
            if ((candidate->flags & 1) == 0 ||
                (current->collisionMask & candidate->collisionMask) == 0) {
                continue;
            }
            if (current->excludedOwner != NULL &&
                current->excludedOwner == candidate->owner) {
                continue;
            }
            if (candidate->excludedOwner != NULL &&
                candidate->excludedOwner == current->owner) {
                continue;
            }
            if (candidate->shapeType == 0) {
                fn_1_F4FC(firstEntry, secondEntry);
            }
        }

        if ((current->flags & 2) == 0) {
            for (candidateIndex = 0; candidateIndex < 10; candidateIndex++) {
                secondEntry = work->regionEntries[candidateIndex];
                if (secondEntry == NULL) {
                    continue;
                }
                candidate = secondEntry;
                if ((candidate->flags & 1) == 0 ||
                    (current->collisionMask & candidate->collisionMask) == 0) {
                    continue;
                }
                if (current->excludedOwner != NULL &&
                    current->excludedOwner == candidate->owner) {
                    continue;
                }
                if (candidate->excludedOwner != NULL &&
                    candidate->excludedOwner == current->owner) {
                    continue;
                }
                switch (candidate->shapeType) {
                case 10:
                    fn_1_F8AC((M675CollisionObject *)firstEntry,
                        (M675CollisionRegion *)secondEntry);
                    break;
                case 11:
                    fn_1_FE84((M675BoundaryObject *)firstEntry,
                        (M675EllipseBoundary *)secondEntry);
                    break;
                }
            }
        }
    }

    for (entryIndex = 0; entryIndex < 84; entryIndex++) {
        firstEntry = work->collisionEntries[entryIndex];
        if (firstEntry != NULL) {
            current = firstEntry;
            if (current->update != NULL) {
                current->update(current->owner);
            }
        }
    }
}

/* The collision manager separates overlapping spheres in X/Z, then notifies both owners. */
s32 fn_1_F4FC(M675Collision *first, M675Collision *second) {
    Vec *firstPosition;
    Vec *secondPosition;
    f32 x;
    f32 y;
    f32 z;
    f32 radius;
    f32 distance;
    f32 normalX;
    f32 normalZ;

    firstPosition = first->position;
    secondPosition = second->position;
    x = secondPosition->x - firstPosition->x;
    y = secondPosition->y - firstPosition->y;
    z = secondPosition->z - firstPosition->z;
    distance = sqrtf((z * z) + ((x * x) + (y * y)));
    radius = first->radius + second->radius;

    if (radius > distance) {
        if (distance == 0.0) {
            normalX = 0.0f;
            normalZ = -1.0f;
        } else {
            normalX = x / distance;
            normalZ = z / distance;
        }

        /* Both zero or both nonzero moves both spheres; otherwise only the nonzero one moves. */
        if ((first->yieldOnContact != 0.0) || (second->yieldOnContact == 0.0)) {
            first->position->x += x - normalX * radius;
            first->position->z += z - normalZ * radius;
        }
        if ((second->yieldOnContact != 0.0) || (first->yieldOnContact == 0.0)) {
            second->position->x += -x + (normalX * radius);
            second->position->z += -z + (normalZ * radius);
        }
        if (first->contact != NULL) {
            first->contact(first, second);
        }
        if (second->contact != NULL) {
            second->contact(second, first);
        }
        return 1;
    }
    return 0;
}

typedef struct M675CollisionObject_s {
    u32 flags;
    u8 collisionHeader[28]; /* Collision registration fields after flags. */
    void (*contactCallback)(void *, void *); /* Receives the object and contacted region. */
    Vec *position; /* Live world-space position adjusted by the response. */
    Vec *velocity; /* Live velocity; a top-face contact clears its Y component. */
    Vec positionCopy;
    f32 radius; /* Contact radius in world units. */
} M675CollisionObject;

typedef struct M675CollisionRegion_s {
    u8 collisionHeader[32]; /* Collision registration fields. */
    Vec center; /* World-space center of the region's ellipses. */
    f32 bottomOffsetY; /* Bottom-plane height relative to the center. */
    f32 height; /* Height from the bottom plane to the top plane. */
    f32 innerRadiusX; /* Inner ellipse's X semi-axis in world units. */
    f32 innerRadiusZ; /* Inner ellipse's Z semi-axis in world units. */
    f32 outerRadiusX; /* Outer ellipse's X semi-axis in world units. */
    f32 outerRadiusZ; /* Outer ellipse's Z semi-axis in world units. */
} M675CollisionRegion;

/* The manager resolves type-10 contacts against an elliptical ring's top or side, then notifies the
 * owner. */
s32 fn_1_F8AC(M675CollisionObject *object, M675CollisionRegion *region)
{
    Vec *position;
    Vec *center;
    f32 bottomY;
    f32 topY;
    f32 innerAxisScale;
    f32 outerAxisScale;
    f32 innerRadius;
    f32 outerRadius;
    f32 verticalDistance;
    f32 innerPenetration;
    f32 innerScaledX;
    f32 innerDeltaZ;
    f32 innerDistance;
    f32 outerScaledX;
    f32 outerDeltaZ;
    f32 outerDistance;
    f32 outerPenetration;
    f32 contactAngle;
    f32 edgeX;
    f32 edgeZ;
    position = object->position;
    center = &region->center;
    bottomY = center->y + region->bottomOffsetY;
    topY = bottomY + region->height;
    innerAxisScale = region->innerRadiusZ / region->innerRadiusX;
    outerAxisScale = region->outerRadiusZ / region->outerRadiusX;
    innerRadius = innerAxisScale * (region->innerRadiusX - object->radius);
    outerRadius = outerAxisScale * (region->outerRadiusX + object->radius);

    if (position->y < bottomY) {
        goto noCollision;
    }
    verticalDistance = topY - position->y;
    if (verticalDistance < 0.0f) {
        goto noCollision;
    }
    innerScaledX = innerAxisScale * (position->x - center->x);
    innerDeltaZ = position->z - center->z;
    innerDistance = sqrtf((innerScaledX * innerScaledX) + (innerDeltaZ * innerDeltaZ));
    innerPenetration = innerDistance - innerRadius;
    if (innerPenetration < 0.0f) {
        goto noCollision;
    }
    outerScaledX = outerAxisScale * (position->x - center->x);
    outerDeltaZ = position->z - center->z;
    outerDistance = sqrtf((outerScaledX * outerScaledX) +
        (outerDeltaZ * outerDeltaZ));
    outerPenetration = outerRadius - outerDistance;
    if (outerPenetration < 0.0f) {
        goto noCollision;
    }
    if (verticalDistance < innerPenetration && verticalDistance < outerPenetration &&
        (object->flags & 4) == 0) {
        position->y = topY;
        /* Contact with the top face stops the object's vertical motion. */
        object->velocity->y = 0.0f;
    } else {
        if (innerPenetration < outerPenetration) {
            edgeX = region->innerRadiusX - object->radius;
            edgeZ = region->innerRadiusZ - object->radius;
            contactAngle = (f32)(180.0 *
                (atan2((f64)innerScaledX, (f64)innerDeltaZ) / 3.141592653589793));
        } else {
            edgeX = region->outerRadiusX + object->radius;
            edgeZ = region->outerRadiusZ + object->radius;
            contactAngle = (f32)(180.0 *
                (atan2((f64)outerScaledX, (f64)outerDeltaZ) /
                    3.141592653589793));
        }
        position->x = (f32)((f64)center->x +
            ((f64)edgeX * sin((3.141592653589793 *
                (f64)contactAngle) / 180.0)));
        position->z = (f32)((f64)center->z +
            ((f64)edgeZ * cos((3.141592653589793 *
                (f64)contactAngle) / 180.0)));
    }
    if (object->contactCallback != NULL) {
        object->contactCallback(object, region);
    }
    return 1;
noCollision:
    return 0;
}

typedef struct M675BoundaryObject_s {
    u8 collisionHeader[32]; /* Collision registration fields. */
    void (*callback)(void *state, void *parameters); /* Receives the object and boundary record. */
    Vec *position; /* Live world-space position constrained by the boundary. */
} M675BoundaryObject;
typedef struct M675EllipseBoundary_s {
    u8 collisionHeader[32]; /* Collision registration fields. */
    Vec center; /* World-space ellipse center. */
    f32 radiusX; /* Ellipse X semi-axis in world units. */
    f32 radiusZ; /* Ellipse Z semi-axis in world units. */
    s32 direction; /* Nonzero confines positions inside; zero keeps them outside. */
} M675EllipseBoundary;

/* The collision manager uses type-11 regions to constrain an object's X/Z position to an
 * ellipse. */
s32 fn_1_FE84(M675BoundaryObject *state, M675EllipseBoundary *parameters)
{
    f32 x;
    f32 z;
    f32 distance;
    f32 angle;
    f32 radius;

    radius = parameters->radiusZ / parameters->radiusX;
    x = radius * (state->position->x - parameters->center.x);
    z = state->position->z - parameters->center.z;
    distance = sqrtf(x * x + z * z);

    if ((parameters->direction != 0 && distance > parameters->radiusZ) ||
        (parameters->direction == 0 && distance < parameters->radiusZ)) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / 3.141592653589793));
        state->position->x = parameters->center.x +
            parameters->radiusX * sin((3.141592653589793 * (f64)angle) / 180.0);
        state->position->z = parameters->center.z +
            parameters->radiusZ * cos((3.141592653589793 * (f64)angle) / 180.0);
        if (state->callback != NULL) {
            state->callback(state, parameters);
        }
        return 1;
    }
    return 0;
}

/* Player and prop setup add a collision record to the first free slot, or receive -1 when full. */
s32 fn_1_10170(void *registration)
{
    OMOBJ *object;
    M675EntryWork *work;
    M675Collision **entry;
    s32 index;

    if (lbl_1_bss_548 != NULL) {
        object = lbl_1_bss_548;
        work = object->data;
        entry = work->collisionEntries;
        for (index = 0; index < 84; index++, entry++) {
            if (*entry == NULL) {
                *entry = registration;
                return index;
            }
        }
    }
    return -1;
}

/* Region setup takes the first free region slot, returning 84-93 or -1 when unavailable. */
int fn_1_10200(void *registration)
{
    if (lbl_1_bss_548 != NULL) {
        OMOBJ *object = lbl_1_bss_548;
        M675EntryWork *table = object->data;
        M675Collision **entry = table->regionEntries;
        s32 index;

        for (index = 0; index < 10; index++, entry++) {
            if (*entry == NULL) {
                *entry = registration;
                return index + 84;
            }
        }
    }
    return -1;
}

/* Registration owners remove a collision or region record using its returned slot index. */
void fn_1_10290(s32 index)
{
    if (lbl_1_bss_548 != NULL) {
        OMOBJ *object = lbl_1_bss_548;
        M675EntryWork *table = object->data;

        if (index >= 0) {
            if (index < 84) {
                table->collisionEntries[index] = NULL;
            }
            /* For collision slots, this second write addresses the same slot from the region
             * base. */
            index -= 84;
            if (index < 10) {
                table->regionEntries[index] = NULL;
            }
        }
    }
}

/* Returns the registration stored at a collision or region slot index. */
void *fn_1_10310(s32 index)
{
    if (lbl_1_bss_548 != NULL) {
        OMOBJ *object = lbl_1_bss_548;
        M675EntryWork *table = object->data;

        if (index < 0) {
            return NULL;
        }
        if (index < 84) {
            return table->collisionEntries[index];
        }
        index -= 84;
        if (index < 10) {
            return table->regionEntries[index];
        }
    }
    return NULL;
}

/* Finds a region whose collision-category word and owner-supplied record index match. */
void *fn_1_1039C(u32 type, s32 index)
{
    OMOBJ *object;
    M675EntryWork *table;
    M675Collision *entry;
    M675Collision *record;
    s32 slot;

    if (lbl_1_bss_548 != NULL) {
        object = lbl_1_bss_548;
        table = object->data;
        for (slot = 0; slot < 10; slot++) {
            entry = table->regionEntries[slot];
            if (entry != NULL) {
                record = entry;
                if (record->type == type && record->index == index) {
                    return entry;
                }
            }
        }
    }
    return NULL;
}
