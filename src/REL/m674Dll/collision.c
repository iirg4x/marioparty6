/* Collision registration, actor-pair checks, and annular and elliptical boundaries. */
#include "dolphin/math.h"
#include "game/main.h"
#include "game/memory.h"
#include "REL/m674Dll/collision.h"
#include "string.h"
OMOBJ *lbl_1_bss_6D0;
void fn_1_10244(OMOBJ *object);
void fn_1_10254(OMOBJ *object);
void fn_1_10308(OMOBJ *object);
s32 fn_1_105A0(M674Entry *first, M674Entry *second);
s32 fn_1_10950(M674Entry *actor, M674Entry *region);
s32 fn_1_10F2C(M674Entry *actor, M674Entry *boundary);

s32 fn_1_101B0(void) { return 0; }

/* Minigame setup calls this to create the collision manager and clear its entry tables. */
void fn_1_101B8(OMOBJMAN *objMan)
{
    OMOBJ *obj;
    void *work;

    obj = omAddObjEx(objMan, 18, 0, 0, -1, fn_1_10244);
    lbl_1_bss_6D0 = obj;
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M674EntryWork), HU_MEMNUM_OVL);
    obj->data = work;
    memset(work, 0, sizeof(M674EntryWork));
}

void fn_1_10240(void)
{
}

void fn_1_10244(OMOBJ *obj)
{
    obj->objFunc = fn_1_10254;
}

/* The object manager calls this each frame; after three frames it runs 100 passes, then one per
 * frame. */
void fn_1_10254(OMOBJ *obj)
{
    M674EntryWork *work;
    s32 i;

    work = obj->data;
    switch (work->state) {
    case 0:
        work->state++;
        work->timer = 0;
    case 1:
        if (++work->timer >= 3) {
            for (i = 0; i < 100; i++) {
                fn_1_10308(obj);
            }
            work->state++;
        }
        break;
    default:
        fn_1_10308(obj);
        break;
    }
}

/* The manager update calls this to check enabled entries, resolve contacts, and invoke
 * callbacks. */
void fn_1_10308(OMOBJ *obj)
{
    M674EntryWork *work;
    M674Entry *first;
    M674Entry *second;
    M674Entry *current;
    M674Entry *candidate;
    s32 i;
    s32 j;

    work = obj->data;
    for (i = 0; i < 31; i++) {
        first = work->entries[i];
        if (first != NULL && first->state == 0) {
            first->shape.actor.contactRegion = NULL;
        }
    }

    for (i = 0; i < 31; i++) {
        first = work->entries[i];
        if (first == NULL) {
            continue;
        }
        current = first;
        if ((current->flags & 1) == 0 || current->state != 0) {
            continue;
        }
        for (j = i + 1; j < 31; j++) {
            second = work->entries[j];
            if (second == NULL) {
                continue;
            }
            candidate = second;
            if ((candidate->flags & 1) == 0 ||
                (current->mask & candidate->mask) == 0) {
                continue;
            }
            if (current->secondaryRelation != NULL &&
                current->secondaryRelation == candidate->primaryRelation) {
                continue;
            }
            if (candidate->secondaryRelation != NULL &&
                candidate->secondaryRelation == current->primaryRelation) {
                continue;
            }
            if (candidate->state == 0) {
                fn_1_105A0(first, second);
            }
        }

        if ((current->flags & 2) == 0) {
            for (j = 0; j < 10; j++) {
                second = work->secondaryEntries[j];
                if (second == NULL) {
                    continue;
                }
                candidate = second;
                if ((candidate->flags & 1) == 0 ||
                    (current->mask & candidate->mask) == 0) {
                    continue;
                }
                if (current->secondaryRelation != NULL &&
                    current->secondaryRelation == candidate->primaryRelation) {
                    continue;
                }
                if (candidate->secondaryRelation != NULL &&
                    candidate->secondaryRelation == current->primaryRelation) {
                    continue;
                }
                switch (candidate->state) {
                case 10:
                    fn_1_10950(first,
                        second);
                    break;
                case 11:
                    fn_1_10F2C(first, second);
                    break;
                }
            }
        }
    }

    for (i = 0; i < 31; i++) {
        first = work->entries[i];
        if (first != NULL) {
            current = first;
            if (current->invoke != NULL) {
                current->invoke(current->primaryRelation);
            }
        }
    }
}

/* Entry processing measures actor contact in 3D, but corrects only X/Z; each movable actor
 * receives the full correction. */
s32 fn_1_105A0(M674Entry *first, M674Entry *second)
{
    Vec *firstPosition;
    Vec *secondPosition;
    f32 x;
    f32 y;
    f32 z;
    f32 reach;
    f32 distance;
    f32 xDirection;
    f32 zDirection;

    firstPosition = first->shape.actor.position;
    secondPosition = second->shape.actor.position;
    x = secondPosition->x - firstPosition->x;
    y = secondPosition->y - firstPosition->y;
    z = secondPosition->z - firstPosition->z;
    distance = sqrtf((z * z) + ((x * x) + (y * y)));
    reach = first->shape.actor.radius + second->shape.actor.radius;
    if (reach > distance) {
        if (0.0 == distance) {
            /* Separate coincident centers along a fixed negative-Z direction. */
            xDirection = 0.0f;
            zDirection = -1.0f;
        } else {
            xDirection = x / distance;
            zDirection = z / distance;
        }
        if ((0.0 != first->shape.actor.collisionFlag) ||
            (0.0 == second->shape.actor.collisionFlag)) {
            first->shape.actor.position->x += x - (xDirection * reach);
            first->shape.actor.position->z += z - (zDirection * reach);
        }
        if ((0.0 != second->shape.actor.collisionFlag) ||
            (0.0 == first->shape.actor.collisionFlag)) {
            second->shape.actor.position->x += -x + (xDirection * reach);
            second->shape.actor.position->z += -z + (zDirection * reach);
        }
        if (first->shape.actor.callback != NULL) {
            first->shape.actor.callback(first, second);
        }
        if (second->shape.actor.callback != NULL) {
            second->shape.actor.callback(second, first);
        }
        return 1;
    }
    return 0;
}

/* Entry processing calls this for region pairs to constrain position and record landing contact. */
s32 fn_1_10950(M674Entry *actor, M674Entry *region)
{
    Vec *position;
    Vec *center;
    f32 bottomY;
    f32 topY;
    f32 innerXScale;
    f32 outerXScale;
    f32 innerLimit;
    f32 outerLimit;
    f32 verticalDistance;
    f32 innerGap;
    f32 innerScaledX;
    f32 deltaZ;
    f32 innerDistance;
    f32 outerScaledX;
    f32 secondDeltaZ;
    f32 outerDistance;
    f32 outerGap;
    f32 contactAngle;
    f32 edgeX;
    f32 edgeZ;

    position = actor->shape.actor.position;
    center = &region->shape.region.center;
    bottomY = center->y + region->shape.region.lowerY;
    topY = bottomY + region->shape.region.verticalSpan;
    innerXScale = region->shape.region.innerRadiusZ / region->shape.region.innerRadiusX;
    outerXScale = region->shape.region.outerRadiusZ / region->shape.region.outerRadiusX;
    innerLimit = innerXScale * (region->shape.region.innerRadiusX - actor->shape.actor.radius);
    outerLimit = outerXScale * (region->shape.region.outerRadiusX + actor->shape.actor.radius);

    if (position->y < bottomY) {
        goto noCollision;
    }
    verticalDistance = topY - position->y;
    if (verticalDistance < 0.0f) {
        goto noCollision;
    }
    innerScaledX = innerXScale * (position->x - center->x);
    deltaZ = position->z - center->z;
    innerDistance = sqrtf((innerScaledX * innerScaledX) + (deltaZ * deltaZ));
    innerGap = innerDistance - innerLimit;
    if (innerGap < 0.0f) {
        goto noCollision;
    }
    outerScaledX = outerXScale * (position->x - center->x);
    secondDeltaZ = position->z - center->z;
    outerDistance = sqrtf((outerScaledX * outerScaledX) +
        (secondDeltaZ * secondDeltaZ));
    outerGap = outerLimit - outerDistance;
    if (outerGap < 0.0f) {
        goto noCollision;
    }
    if (verticalDistance < innerGap && verticalDistance < outerGap &&
        (actor->flags & 4) == 0) {
        position->y = topY;
        actor->shape.actor.velocity->y = 0.0f;
        actor->shape.actor.contactRegion = region;
    } else {
        if (innerGap < outerGap) {
            edgeX = region->shape.region.innerRadiusX - actor->shape.actor.radius;
            edgeZ = region->shape.region.innerRadiusZ - actor->shape.actor.radius;
            contactAngle = (f32)(180.0 *
                (atan2((f64)innerScaledX, (f64)deltaZ) / 3.141592653589793));
        } else {
            edgeX = region->shape.region.outerRadiusX + actor->shape.actor.radius;
            edgeZ = region->shape.region.outerRadiusZ + actor->shape.actor.radius;
            contactAngle = (f32)(180.0 *
                (atan2((f64)outerScaledX, (f64)secondDeltaZ) /
                    3.141592653589793));
        }
        position->x = (f32)((f64)center->x +
            ((f64)edgeX * sin((3.141592653589793 *
                (f64)contactAngle) / 180.0)));
        position->z = (f32)((f64)center->z +
            ((f64)edgeZ * cos((3.141592653589793 *
                (f64)contactAngle) / 180.0)));
    }
    if (actor->shape.actor.callback != NULL) {
        actor->shape.actor.callback(actor, region);
    }
    return 1;
noCollision:
    return 0;
}

/* Check each eligible actor/ellipse pair and clamp a position that violates the configured side
 * of the ellipse. */
s32 fn_1_10F2C(M674Entry *actor, M674Entry *boundary)
{
    f32 x;
    f32 z;
    f32 distance;
    f32 xScale;
    f32 angle;

    /* Scale X into the ellipse's Z-radius space for the circle distance test. */
    xScale = boundary->shape.ellipse.radiusZ / boundary->shape.ellipse.radiusX;
    x = xScale * (actor->shape.actor.position->x - boundary->shape.ellipse.center.x);
    z = actor->shape.actor.position->z - boundary->shape.ellipse.center.z;
    distance = sqrtf(x * x + z * z);
    if (((boundary->shape.ellipse.direction != 0) &&
         (distance > boundary->shape.ellipse.radiusZ)) ||
        ((boundary->shape.ellipse.direction == 0) &&
         (distance < boundary->shape.ellipse.radiusZ))) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / 3.141592653589793));
        actor->shape.actor.position->x = (f32) ((f64) boundary->shape.ellipse.center.x +
                                                  ((f64) boundary->shape.ellipse.radiusX *
                                                  sin((3.141592653589793 * (f64) angle) / 180.0)));
        actor->shape.actor.position->z = (f32) ((f64) boundary->shape.ellipse.center.z +
                                                  ((f64) boundary->shape.ellipse.radiusZ *
                                                  cos((3.141592653589793 * (f64) angle) / 180.0)));
        if (actor->shape.actor.callback != NULL) {
            actor->shape.actor.callback(actor, boundary);
        }
        return 1;
    }
    return 0;
}

/* Adds an entry to the first free primary slot and returns its slot index. */
s32 fn_1_11218(M674Entry *registration)
{
    OMOBJ *object;
    M674EntryWork *work;
    M674Entry **entry;
    s32 index;

    if (lbl_1_bss_6D0 != NULL) {
        object = lbl_1_bss_6D0;
        work = object->data;
        entry = work->entries;
        for (index = 0; index < 31; index++, entry++) {
            if (*entry == NULL) {
                *entry = registration;
                return index;
            }
        }
    }
    return -1;
}

/* Adds an entry to the first free secondary slot and returns its combined index. */
s32 fn_1_112A8(M674Entry *entry)
{
    OMOBJ *obj;
    M674EntryWork *work;
    M674Entry **slot;
    s32 i;

    if (lbl_1_bss_6D0 != NULL) {
        obj = lbl_1_bss_6D0;
        work = obj->data;
        slot = work->secondaryEntries;
    for (i = 0; i < 10; i++, slot++) {
        if (*slot == NULL) {
            *slot = entry;
                return i + 31;
            }
        }
    }
    return -1;
}

/* Clear the indexed slot; primary indices also clear the same slot again through a negative
 * secondary-array index after subtracting 31. */
void fn_1_11338(s32 index)
{
    OMOBJ *obj;
    M674EntryWork *work;

    if (lbl_1_bss_6D0 != NULL) {
        obj = lbl_1_bss_6D0;
        work = obj->data;
        if (index >= 0) {
            if (index < 31) {
                work->entries[index] = NULL;
            }
            index -= 31;
            if (index < 10) {
                work->secondaryEntries[index] = NULL;
            }
        }
    }
}

/* Returns the entry at a valid primary or secondary index, or NULL. */
M674Entry *fn_1_113B8(s32 index)
{
    if (lbl_1_bss_6D0 != NULL) {
        OMOBJ *object = lbl_1_bss_6D0;
        M674EntryWork *table = object->data;

        if (index < 0) {
            return NULL;
        }
        if (index < 31) {
            return table->entries[index];
        }
        index -= 31;
        if (index < 10) {
            return table->secondaryEntries[index];
        }
    }
    return NULL;
}

/* Finds a secondary entry with the requested category and ID. */
M674Entry *fn_1_11444(u32 type, s32 id)
{
    OMOBJ *object;
    M674EntryWork *work;
    M674Entry *entry;
    M674Entry *record;
    s32 index;

    if (lbl_1_bss_6D0 != NULL) {
        object = lbl_1_bss_6D0;
        work = object->data;
        for (index = 0; index < 10; index++) {
            entry = work->secondaryEntries[index];
            if (entry != NULL) {
                record = entry;
                if (record->type == type && record->id == id) {
                    return entry;
                }
            }
        }
    }
    return NULL;
}
