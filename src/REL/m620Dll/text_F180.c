/* Trap Ease Artist collision checks and per-frame collision registration. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/object.h"
#include "game/charman.h"
#include "game/hu3d.h"
#include "game/gamework.h"
#include "game/wipe.h"
#include "game/mg/timer.h"

void fn_1_F190(OMOBJ *obj);

void fn_1_F180(OMOBJ *obj)
{
    obj->objFunc = fn_1_F190;
}

typedef struct M620Position_s {
    f32 x;
    f32 y;
    f32 z;
} M620Position;

typedef struct M620Motion_s {
    /* These bytes are not read by the motion-boundary helper in this file. */
    u8 unread_00_to_31[32];
    void (*callback)(void *motion, void *settings);
    M620Position *position;
} M620Motion;

typedef struct M620MotionSettings_s {
    /* The boundary helper does not inspect this prefix or the gap after centerX. */
    u8 unread_00_to_31[32];
    f32 centerX;
    u8 unread_36_to_39[4];
    f32 centerZ;
    f32 radiusX;
    f32 radiusZ;
    /* Zero selects the interior test; nonzero selects the exterior test. */
    s32 direction;
} M620MotionSettings;

typedef struct M620CollisionActor_s M620CollisionActor;
struct M620CollisionActor_s {
    /* The collision helpers use the callback, position, reference coordinates and flags below. */
    u8 unread_00_to_31[32];
    void (*collisionCallback)(void *self, void *other);
    M620Position *position;
    u8 unread_40_to_43[4];
    f32 collisionReferenceX;
    u8 unread_48_to_51[4];
    f32 collisionReferenceZ;
    f32 radius;
    f32 canMoveOnCollision;
};

typedef struct M620RingVolume_s {
    /* The ring-volume test reads the center position and the four extents below. */
    u8 unread_00_to_31[32];
    M620Position *position;
    f32 lowerHeight;
    f32 heightSpan;
    f32 innerRadius;
    f32 radialThickness;
} M620RingVolume;

typedef struct M620CollisionEntry_s {
    u32 flags;
    u32 mask;
    /* These words are not read by the collision manager in this file. */
    u32 unread_08;
    u32 unread_12;
    s32 state;
    void *primaryRelation;
    void *secondaryRelation;
    void (*invoke)(s32 argument);
} M620CollisionEntry;

typedef struct M620CollisionWork_s {
    /* The manager scans the primary list against itself and the secondary list. */
    M620CollisionEntry *entries[39];
    M620CollisionEntry *secondaryEntries[10];
} M620CollisionWork;

s32 fn_1_F3DC(M620CollisionActor *first, M620CollisionActor *second);
s32 fn_1_F78C(M620CollisionActor *first, M620RingVolume *second);
s32 fn_1_FC68(M620Motion *motion, M620MotionSettings *settings);

/* Check registered pairs and invoke each primary entry's per-frame callback. */
void fn_1_F190(OMOBJ *obj)
{
    M620CollisionWork *work = obj->data;
    M620CollisionEntry *first;
    M620CollisionEntry *second;
    M620CollisionEntry *current;
    M620CollisionEntry *candidate;
    s32 i;
    s32 j;

    for (i = 0; i < 39; i++) {
        first = work->entries[i];
        if (first != NULL) {
            current = first;
        } else {
            continue;
        }
        if ((current->flags & 1) != 0 && current->state == 0) {
            for (j = i + 1; j < 39; j++) {
                second = work->entries[j];
                if (second != NULL) {
                    candidate = second;
                } else {
                    continue;
                }
                if ((candidate->flags & 1) != 0 &&
                    (current->mask & candidate->mask) != 0 &&
                    (current->secondaryRelation == NULL ||
                     current->secondaryRelation != candidate->primaryRelation) &&
                    (candidate->secondaryRelation == NULL ||
                     candidate->secondaryRelation != current->primaryRelation) &&
                    candidate->state == 0) {
                    fn_1_F3DC((M620CollisionActor *)first,
                              (M620CollisionActor *)second);
                }
            }
            if ((current->flags & 2) == 0) {
                for (j = 0; j < 10; j++) {
                    second = work->secondaryEntries[j];
                    if (second != NULL) {
                        candidate = second;
                    } else {
                        continue;
                    }
                    if ((candidate->flags & 1) != 0 &&
                        (current->mask & candidate->mask) != 0 &&
                        (current->secondaryRelation == NULL ||
                         current->secondaryRelation != candidate->primaryRelation) &&
                        (candidate->secondaryRelation == NULL ||
                         candidate->secondaryRelation != current->primaryRelation)) {
                        switch (candidate->state) {
                        case 10:
                            fn_1_F78C((M620CollisionActor *)first,
                                      (M620RingVolume *)second);
                            break;
                        case 11:
                            fn_1_FC68((M620Motion *)first,
                                      (M620MotionSettings *)second);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < 39; i++) {
        first = work->entries[i];
        if (first != NULL) {
            current = first;
            if (current->invoke != NULL) {
                current->invoke((s32)current->primaryRelation);
            }
        }
    }
}

/* Separate overlapping actors according to their movement flags and notify both callbacks. */
s32 fn_1_F3DC(M620CollisionActor *first, M620CollisionActor *second)
{
    M620Position *firstPosition;
    M620Position *secondPosition;
    f32 x;
    f32 y;
    f32 z;
    f32 reach;
    f32 distance;
    f32 xDirection;
    f32 zDirection;

    firstPosition = first->position;
    secondPosition = second->position;
    x = secondPosition->x - firstPosition->x;
    y = secondPosition->y - firstPosition->y;
    z = secondPosition->z - firstPosition->z;
    distance = sqrtf((z * z) + ((x * x) + (y * y)));
    reach = first->radius + second->radius;
    if (reach > distance) {
        if (0.0 == distance) {
            xDirection = 0.0f;
            zDirection = -1.0f;
        } else {
            xDirection = x / distance;
            zDirection = z / distance;
        }
        /* If both movement values are zero, both actors still move; otherwise only nonzero entries
         * move. */
        if ((0.0 != first->canMoveOnCollision) || (0.0 == second->canMoveOnCollision)) {
            first->position->x += x - (xDirection * reach);
            first->position->z += z - (zDirection * reach);
        }
        if ((0.0 != second->canMoveOnCollision) || (0.0 == first->canMoveOnCollision)) {
            second->position->x += -x + (xDirection * reach);
            second->position->z += -z + (zDirection * reach);
        }
        if (first->collisionCallback != NULL) {
            first->collisionCallback(first, second);
        }
        if (second->collisionCallback != NULL) {
            second->collisionCallback(second, first);
        }
        return 1;
    }
    return 0;
}

/* Within the ring's height band, push overlapping actors into the hole or outside the annulus. */
s32 fn_1_F78C(M620CollisionActor *first, M620RingVolume *second)
{
    M620Position *firstPosition;
    M620Position *secondPosition;
    f32 x;
    f32 z;
    f32 distance;
    f32 radialX;
    f32 radialZ;
    f32 angle;

    if (first->position->y < second->position->y + second->lowerHeight) {
        return 0;
    }
    if (first->position->y > second->position->y +
        second->lowerHeight + second->heightSpan) {
        return 0;
    }

    x = first->position->x - second->position->x;
    z = first->position->z - second->position->z;
    distance = sqrtf((x * x) + (z * z));

    if (distance < first->radius + second->innerRadius +
        second->radialThickness) {
        if (distance > second->innerRadius - first->radius) {
            radialX = first->collisionReferenceX - second->position->x;
            radialZ = first->collisionReferenceZ - second->position->z;
            distance = sqrtf((radialX * radialX) + (radialZ * radialZ));
            if (distance > second->innerRadius +
                (0.5 * (f64)second->radialThickness)) {
                distance = first->radius + (second->innerRadius +
                    second->radialThickness);
            } else {
                distance = second->innerRadius - first->radius;
            }
            angle = (f32)(180.0 * (atan2((f64)x, (f64)z) /
                3.141592653589793));
            first->position->x = (f32)((f64)second->position->x +
                ((f64)distance * sin((3.141592653589793 * (f64)angle) / 180.0)));
            first->position->z = (f32)((f64)second->position->z +
                ((f64)distance * cos((3.141592653589793 * (f64)angle) / 180.0)));
            if (first->collisionCallback != NULL) {
                first->collisionCallback(first, second);
            }
            return 1;
        }
    }
    return 0;
}

/* Constrain the moving point to the configured ellipse and call its collision callback. */
s32 fn_1_FC68(M620Motion *motion, M620MotionSettings *settings)
{
    f32 x;
    f32 z;
    f32 distance;
    f32 radius;
    f32 angle;

    radius = settings->radiusZ / settings->radiusX;
    x = radius * (motion->position->x - settings->centerX);
    z = motion->position->z - settings->centerZ;
    distance = sqrtf(x * x + z * z);
    if (((settings->direction != 0) && (distance > settings->radiusZ)) ||
        ((settings->direction == 0) && (distance < settings->radiusZ))) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / 3.141592653589793));
        motion->position->x = (f32)((f64)settings->centerX +
            ((f64)settings->radiusX * sin((3.141592653589793 * (f64)angle) / 180.0)));
        motion->position->z = (f32)((f64)settings->centerZ +
            ((f64)settings->radiusZ * cos((3.141592653589793 * (f64)angle) / 180.0)));
        if (motion->callback != NULL) {
            motion->callback(motion, settings);
        }
        return 1;
    }
    return 0;
}

extern OMOBJ *lbl_1_bss_660;

typedef struct M620CollisionEntryWork {
    void *entries[49];
} M620CollisionEntryWork;

/* Add a collision entry to the first free primary slot and return its slot number. */
int fn_1_FF54(void *value)
{
    OMOBJ *object = lbl_1_bss_660;
    M620CollisionEntryWork *data = object->data;
    void **entry = data->entries;
    int index;

    for (index = 0; index < 39; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index;
        }
    }
    return -1;
}

/* Add a collision entry to the first free secondary slot and return its combined index. */
int fn_1_FFCC(void *value)
{
    OMOBJ *object = lbl_1_bss_660;
    M620CollisionEntryWork *data = object->data;
    void **entry = &data->entries[39];
    int index;

    for (index = 0; index < 10; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index + 39;
        }
    }
    return -1;
}

typedef struct M620PointerSlots {
    void *primary[39];
    void *secondary[10];
} M620PointerSlots;

/* Clear the registered entry selected by its primary or combined secondary index. */
void fn_1_10044(s32 registrationIndex)
{
    M620PointerSlots *slots;
    OMOBJ *object = lbl_1_bss_660;

    slots = object->data;
    if (registrationIndex >= 0) {
        if (registrationIndex < 39) {
            slots->primary[registrationIndex] = NULL;
        }
        /* A primary index also reaches this reindexed secondary access, clearing the same slot
         * again. */
        registrationIndex -= 39;
        if (registrationIndex < 10) {
            slots->secondary[registrationIndex] = NULL;
        }
    }
}

/* Return the registered entry selected by its primary or combined secondary index. */
void *fn_1_100A8(s32 registrationIndex)
{
    M620PointerSlots *slots;
    OMOBJ *object = lbl_1_bss_660;

    slots = object->data;
    if (registrationIndex < 0) {
        return NULL;
    }
    if (registrationIndex < 39) {
        return slots->primary[registrationIndex];
    }
    registrationIndex -= 39;
    if (registrationIndex < 10) {
        return slots->secondary[registrationIndex];
    }
    return NULL;
}

typedef struct M620Registration {
    /* Registration lookup uses the type and identifier after this prefix. */
    u8 unread_00_to_07[8];
    u32 type;
    s32 id;
} M620Registration;

/* Find a secondary registration with the requested type and identifier. */
void *fn_1_10118(u32 type, s32 id)
{
    OMOBJ *object;
    M620PointerSlots *slots;
    M620Registration *entry;
    M620Registration *record;
    s32 index;

    object = lbl_1_bss_660;
    slots = object->data;
    for (index = 0; index < 10; index++) {
        entry = slots->secondary[index];
        if (entry != NULL) {
            record = entry;
            if (record->type == type && record->id == id) {
                return entry;
            }
        }
    }
    return NULL;
}
