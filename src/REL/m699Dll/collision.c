/* Dunk Bros. collision registration, tests and response. */
#include "dolphin/types.h"
#include "string.h"
#include "math.h"

#include "dolphin/mtx.h"
#include "REL/m699Dll/collision.h"
#include "game/object.h"
#include "game/memory.h"

#include "REL/m699Dll/m699_natural_views.h"
#include "humath.h"

extern void fn_1_1B770(OMOBJ *obj);

/* Create the collision manager and clear its registration slots. */
void fn_1_1B6E4(HUPROCESS *objectManager) {
    OMOBJ *object;
    void *data;

    object = fn_8003AB38(objectManager, 0x12, 0, 0, -1, fn_1_1B770);
    lbl_1_bss_A68 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 0xA0, 0x10000000);
    object->data = data;
    memset(data, 0, 0xA0);
}

void fn_1_1B76C(void) {
}

void fn_1_1B780(OMOBJ *obj);

void fn_1_1B770(OMOBJ *obj)
{
    obj->objFunc = fn_1_1B780;
}

typedef struct M699PointerSlots {
    void *values[40];
} M699PointerSlots;

static inline s32 M699CollisionPositionInvalid(Vec *position)
{
    return isnan(position->x) || isinf(position->x) ||
        isnan(position->y) || isinf(position->y) ||
        isnan(position->z) || isinf(position->z);
}

void fn_1_1B780(OMOBJ *obj)
{
    M699PointerSlots *work;
    M699CollisionRecord *first;
    M699CollisionRecord *second;
    M699CollisionRecord *firstHeader;
    M699CollisionRecord *secondHeader;
    M699CollisionRecord *positionNode;
    Vec *position;
    s32 i;
    s32 j;

    work = obj->data;
    for (i = 0; i < 20; i++) {
        first = work->values[i];
        if (first != NULL && first->state == 0) {
            first->shape.actor.ready = 0;
        }
    }
    for (j = 0; j < 20; j++) {
        second = (&work->values[20])[j];
        if (second != NULL) {
            secondHeader = second;
            secondHeader->status = 0;
        }
    }

    for (i = 0; i < 20; i++) {
        first = work->values[i];
        if (first == NULL) {
            continue;
        }
        firstHeader = first;
        if ((firstHeader->flags & 1) == 0 || firstHeader->state != 0) {
            continue;
        }
        {
            for (j = i + 1; j < 20; j++) {
                second = work->values[j];
                if (second == NULL) {
                    continue;
                }
                secondHeader = second;
                if ((secondHeader->flags & 1) == 0) {
                    continue;
                }
                if ((firstHeader->mask & secondHeader->mask) == 0) {
                    continue;
                }
                if (firstHeader->excludedIdentity != NULL && firstHeader->excludedIdentity == secondHeader->identity) {
                    continue;
                }
                if (secondHeader->excludedIdentity != NULL && secondHeader->excludedIdentity == firstHeader->identity) {
                    continue;
                }
                if (secondHeader->state == 0) {
                    fn_1_1BED0(first, second);
                }
            }
            if ((firstHeader->flags & 2U) == 0) {
                for (j = 0; j < 20; j++) {
                    second = (&work->values[20])[j];
                    if (second == NULL) {
                        continue;
                    }
                    secondHeader = second;
                    if ((secondHeader->flags & 1) == 0) {
                        continue;
                    }
                    if ((firstHeader->mask & secondHeader->mask) == 0) {
                        continue;
                    }
                    if (firstHeader->excludedIdentity != NULL && firstHeader->excludedIdentity == secondHeader->identity) {
                        continue;
                    }
                    if (secondHeader->excludedIdentity != NULL && secondHeader->excludedIdentity == firstHeader->identity) {
                        continue;
                    }
                    {
                        switch (secondHeader->state) {
                        case 10:
                            if (fn_1_1C280(first, second) != 0) {
                                secondHeader->status = 1;
                            }
                            break;
                        case 11:
                            if (fn_1_1C4D4(first, second) != 0) {
                                secondHeader->status = 1;
                            }
                            break;
                        case 12:
                            if (fn_1_1CA4C(first, second) != 0) {
                                secondHeader->status = 1;
                            }
                            break;
                        case 13:
                            if (fn_1_1CD80(first, second) != 0) {
                                secondHeader->status = 1;
                            }
                            break;
                        case 14:
                            if (fn_1_1CE08(first, second) != 0) {
                                secondHeader->status = 1;
                            }
                            break;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < 20; i++) {
        first = work->values[i];
        if (first == NULL) {
            continue;
        }
        firstHeader = first;
        {
            if (firstHeader->state == 0) {
                positionNode = first;
                position = positionNode->shape.actor.position;
                if (M699CollisionPositionInvalid(position)) {
                    *positionNode->shape.actor.position = positionNode->shape.actor.savedPosition;
                }
            }
            if (firstHeader->update != NULL) {
                firstHeader->update((s32)firstHeader->identity);
            }
            if (firstHeader->state == 0) {
                positionNode = first;
                positionNode->shape.actor.savedPosition = *positionNode->shape.actor.position;
            }
        }
    }
}

s32 fn_1_1BED0(M699CollisionRecord *first, M699CollisionRecord *second)
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
    reach = first->shape.actor.extent.x + second->shape.actor.extent.x;
    if (reach > distance) {
        if (0.0 == distance) {
            xDirection = 0.0f;
            zDirection = -1.0f;
        } else {
            xDirection = x / distance;
            zDirection = z / distance;
        }
        if ((0.0 != first->shape.actor.collisionScale) || (0.0 == second->shape.actor.collisionScale)) {
            first->shape.actor.position->x += x - (xDirection * reach);
            first->shape.actor.position->z += z - (zDirection * reach);
        }
        if ((0.0 != second->shape.actor.collisionScale) || (0.0 == first->shape.actor.collisionScale)) {
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

s32 fn_1_1C280(M699CollisionRecord *actor, M699CollisionRecord *volume)
{
    Vec *actorPosition;
    Vec *volumePosition;
    f32 xDistance;
    f32 zDistance;
    f32 distance;
    f32 reach;

    actorPosition = actor->shape.actor.position;
    volumePosition = &volume->shape.box.minimum;
    if (!((actorPosition->y - actor->shape.actor.extent.x) < volumePosition->y) &&
        !((actorPosition->y + actor->shape.actor.extent.x) > (volumePosition->y + volume->shape.box.maximum.y))) {
        xDistance = actorPosition->x - volumePosition->x;
        zDistance = actorPosition->z - volumePosition->z;
        distance = sqrtf((xDistance * xDistance) + (zDistance * zDistance));

        reach = actor->shape.actor.extent.x + volume->shape.box.maximum.x;
        if (!(distance > reach)) {
            if (0.0 == distance) {
                xDistance = 0.0f;
                zDistance = -1.0f;
            } else {
                xDistance /= distance;
                zDistance /= distance;
            }
            actorPosition->x = volumePosition->x + (xDistance * reach);
            actorPosition->z = volumePosition->z + (zDistance * reach);
            return 1;
        }
    }
    return 0;
}

s32 fn_1_1C4D4(M699CollisionRecord *motion, M699CollisionRecord *settings)
{
    f32 radius;
    f32 x;
    f32 z;
    f32 distance;
    f32 angle;

    radius = settings->shape.ellipse.radiusZ / settings->shape.ellipse.radiusX;
    x = radius * (motion->shape.actor.position->x - settings->shape.ellipse.center.x);
    z = motion->shape.actor.position->z - settings->shape.ellipse.center.z;
    distance = sqrtf(x * x + z * z);
    if (((settings->shape.ellipse.direction != 0) && (distance > settings->shape.ellipse.radiusZ)) || ((settings->shape.ellipse.direction == 0) && (distance < settings->shape.ellipse.radiusZ))) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / M_PI));
        motion->shape.actor.position->x = settings->shape.ellipse.center.x +
            settings->shape.ellipse.radiusX * sin((M_PI * (f64)angle) / 180.0);
        motion->shape.actor.position->z = settings->shape.ellipse.center.z +
            settings->shape.ellipse.radiusZ * cos((M_PI * (f64)angle) / 180.0);
        if (motion->shape.actor.callback != NULL) {
            motion->shape.actor.callback(motion, settings);
        }
        return 1;
    }
    return 0;
}

s32 fn_1_1C7C0(M699CollisionRecord *work, M699CollisionRecord *bounds)
{
    s32 axis;
    f32 axisCenter;
    f32 otherCenter;
    f32 positiveExtent;
    f32 negativeExtent;
    f32 lowerBound;
    f32 upperBound;
    f32 separation;
    f32 movement;
    f32 *axisPositions;
    f32 *positionCoordinates;
    f32 *positiveExtentValues;
    f32 *negativeExtentValues;
    f32 *minimumValues;
    f32 *maximumValues;

    axisPositions = &work->shape.actor.position->x;
    positionCoordinates = &work->shape.actor.savedPosition.x;
    positiveExtentValues = work->shape.actor.positiveExtent;
    negativeExtentValues = work->shape.actor.negativeExtent;
    minimumValues = &bounds->shape.box.minimum.x;
    maximumValues = &bounds->shape.box.maximum.x;
    positiveExtentValues[0] = work->shape.actor.extent.x;
    positiveExtentValues[1] = work->shape.actor.extent.z;
    positiveExtentValues[2] = work->shape.actor.extent.x;
    negativeExtentValues[0] = -work->shape.actor.extent.x;
    negativeExtentValues[1] = work->shape.actor.extent.y;
    negativeExtentValues[2] = -work->shape.actor.extent.x;
    for (axis = 0; axis < 3; axis++) {
        axisCenter = axisPositions[axis];
        otherCenter = positionCoordinates[axis];
        positiveExtent = positiveExtentValues[axis];
        negativeExtent = negativeExtentValues[axis];
        lowerBound = minimumValues[axis];
        upperBound = maximumValues[axis];
        if (!((otherCenter + positiveExtent) > lowerBound) &&
            !((axisCenter + positiveExtent) <= lowerBound)) {
            separation = axisCenter - otherCenter;
            if (!(separation <= 0.0f)) {
                movement = lowerBound - (otherCenter + positiveExtent);
                if (!(movement < 0.0f) &&
                    fn_1_1CB70(axis, work, bounds, movement / separation) != 0) {
                    if (axis == 1 && work->mode != 1 &&
                        work->shape.actor.velocity->y > 0.0) {
                        work->shape.actor.velocity->y *= 0.8;
                    }
                    return 1;
                }
            }
        }
        if (!((otherCenter + negativeExtent) < upperBound) &&
            !((axisCenter + negativeExtent) >= upperBound)) {
            separation = otherCenter - axisCenter;
            if (!(separation <= 0.0f)) {
            movement = (otherCenter + negativeExtent) - upperBound;
            if (!(movement < 0.0f) &&
                fn_1_1CB70(axis, work, bounds, movement / separation) != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Resolve the bounds collision, nudge an unmoved actor and notify its callback. */
s32 fn_1_1CA4C(M699CollisionRecord *work, M699CollisionRecord *bounds) {
    Vec *previousPosition;
    Vec *position;
    s32 collided;
    collided = fn_1_1C7C0(work, bounds);
    if (work->mode == 1U && (collided != 0))
    {
        position = work->shape.actor.position;
        previousPosition = &work->shape.actor.savedPosition;
        if (position->x == previousPosition->x && position->y == previousPosition->y &&
            position->z == previousPosition->z)
        {
            if (position->x < 0.0)
            {
                work->shape.actor.position->x += 2.0;
            }
            else
            {
                work->shape.actor.position->x = (f32)((f64)work->shape.actor.position->x - 2.0);
            }
        }
    }
    if ((collided != 0) && work->shape.actor.callback != NULL)
    {
        work->shape.actor.callback(work, bounds);
    }
    return collided;
}

extern void fn_1_539C(Point3d *start, Point3d *end, f32 t, Point3d *out);

s32 fn_1_1CB70(s32 axis, M699CollisionRecord *work,
               const M699CollisionRecord *bounds, f32 fraction)
{
    Vec position;
    const Vec *minimumBounds;
    const Vec *maximumBounds;

    minimumBounds = &bounds->shape.box.minimum;
    maximumBounds = &bounds->shape.box.maximum;
    fn_1_539C(&work->shape.actor.savedPosition, work->shape.actor.position, fraction, &position);
    if (axis != 0) {
        if (position.x + work->shape.actor.positiveExtent[0] < minimumBounds->x) {
            return 0;
        }
        if (position.x + work->shape.actor.negativeExtent[0] > maximumBounds->x) {
            return 0;
        }
        position.x = work->shape.actor.position->x;
    }
    if (axis != 1) {
        if (position.y + work->shape.actor.positiveExtent[1] < minimumBounds->y) {
            return 0;
        }
        if (position.y + work->shape.actor.negativeExtent[1] > maximumBounds->y) {
            return 0;
        }
        position.y = work->shape.actor.position->y;
    }
    if (axis != 2) {
        if (position.z + work->shape.actor.positiveExtent[2] < minimumBounds->z) {
            return 0;
        }
        if (position.z + work->shape.actor.negativeExtent[2] > maximumBounds->z) {
            return 0;
        }
        position.z = work->shape.actor.position->z;
    }

    *work->shape.actor.position = position;
    if (work->mode == 1) {
        switch (axis) {
        case 0:
            work->shape.actor.velocity->x *= -0.7;
            break;
        case 1:
            work->shape.actor.velocity->y *= -0.7;
            break;
        case 2:
            work->shape.actor.velocity->z *= -0.7;
            break;
        }
    }
    return 1;
}

s32 fn_1_1CD80(M699CollisionRecord *positionRecord,
               const M699CollisionRecord *boundsRecord) {
    if (positionRecord->shape.actor.position->x < boundsRecord->shape.box.minimum.x) {
        positionRecord->shape.actor.position->x = boundsRecord->shape.box.minimum.x;
    }
    if (positionRecord->shape.actor.position->x > boundsRecord->shape.box.maximum.x) {
        positionRecord->shape.actor.position->x = boundsRecord->shape.box.maximum.x;
    }
    if (positionRecord->shape.actor.position->z < boundsRecord->shape.box.minimum.z) {
        positionRecord->shape.actor.position->z = boundsRecord->shape.box.minimum.z;
    }
    if (positionRecord->shape.actor.position->z > boundsRecord->shape.box.maximum.z) {
        positionRecord->shape.actor.position->z = boundsRecord->shape.box.maximum.z;
    }
    return 0;
}

s32 fn_1_1CE08(M699CollisionRecord *work,
               M699CollisionRecord *other)
{
    Vec offset;
    Vec *direction;
    s32 moved = 0;

    if (work->mode == 1) {
        if (fn_1_1CF50(work, other, &offset) != 0) {
            PSVECAdd(work->shape.actor.velocity, &offset, work->shape.actor.velocity);
            moved = 1;
        }
        other->shape.circle.collisionState = fn_1_1D3E4(work, other);
        if (other->shape.circle.collisionState != 0) {
            work->shape.actor.collisionScale = 0.0f;
        }
        if (other->shape.circle.collisionState != 0) {
            work->shape.actor.velocity->x *= 0.8;
            work->shape.actor.velocity->z *= 0.8;
        }
        direction = fn_1_19080(&work->shape.actor.position->x);
        PSVECAdd(work->shape.actor.velocity, direction, work->shape.actor.velocity);
    }
    if (moved != 0 && work->shape.actor.callback != NULL) {
        work->shape.actor.callback(work, other);
    }
    return moved;
}

s32 fn_1_1CF50(M699CollisionRecord *first, M699CollisionRecord *second,
               Point3d *separation)
{
    Point3d direction;
    Point3d displacement;
    f32 distance;

    PSVECSubtract(first->shape.actor.position, second->shape.circle.position, &direction);
    direction.y = 0.0f;
    PSVECScale(&direction, &direction,
               second->shape.circle.radius / PSVECMag(&direction));
    PSVECAdd(second->shape.circle.position, &direction, &direction);
    PSVECSubtract(first->shape.actor.position, &direction, &displacement);
    distance = PSVECMag(&displacement);
    if (distance > 0.0f && distance < first->shape.actor.extent.x) {
        PSVECScale(&displacement, separation,
                   ((first->shape.actor.extent.x + second->shape.circle.extraRadius) - distance) /
                       distance);
        if ((isnan(separation->x) || isinf(separation->x) ||
                  isnan(separation->y) || isinf(separation->y) ||
                  isnan(separation->z) || isinf(separation->z)) != 0) {
            separation->x = 0.0f;
            separation->y = 0.0f;
            separation->z = 0.0f;
            return 0;
        }
        return 1;
    }
    return 0;
}

s32 fn_1_1D3E4(M699CollisionRecord *work, M699CollisionRecord *bounds)
{
    f32 planeY;
    f32 startY;
    f32 endY;
    f64 fraction;
    f64 horizontalX;
    f64 horizontalZ;

    planeY = bounds->shape.circle.position->y;
    startY = work->shape.actor.savedPosition.y;
    endY = work->shape.actor.position->y;
    if ((startY >= planeY) && (endY < planeY)) {
        fraction = ((f64)startY - (f64)planeY) / ((f64)startY - (f64)endY);
        horizontalX = ((fraction * (f64)work->shape.actor.position->x) + ((1.0 - fraction) * (f64)work->shape.actor.savedPosition.x)) - (f64)bounds->shape.circle.position->x;
        horizontalZ = ((fraction * (f64)work->shape.actor.position->z) + ((1.0 - fraction) * (f64)work->shape.actor.savedPosition.z)) - (f64)bounds->shape.circle.position->z;
        if (sqrt((horizontalX * horizontalX) +
                 (horizontalZ * horizontalZ)) <
            (bounds->shape.circle.radius - bounds->shape.circle.extraRadius)) {
            return 1;
        }
    }
    return 0;
}

/* Register an actor in the first free slot, or return -1 when full. */
int fn_1_1D6BC(void *value) {
    OMOBJ *object = lbl_1_bss_A68;
    M699PointerSlots *work = object->data;
    void **entry = work->values;
    int index;

    for (index = 0; index < 20; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index;
        }
    }
    return -1;
}

/* Register a volume in the second bank of slots, or return -1 when full. */
int fn_1_1D734(void *value) {
    OMOBJ *object = lbl_1_bss_A68;
    M699PointerSlots *work = object->data;
    void **entry = &work->values[20];
    int index;

    for (index = 0; index < 20; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index + 0x14;
        }
    }
    return -1;
}

void fn_1_1D7AC(s32 registrationIndex)
{
    M699PointerSlots *slots;
    OMOBJ *object = lbl_1_bss_A68;

    slots = object->data;
    if (registrationIndex >= 0) {
        if (registrationIndex < 20) {
            slots->values[registrationIndex] = NULL;
        }
        registrationIndex -= 20;
        if (registrationIndex < 20) {
            (&slots->values[20])[registrationIndex] = NULL;
        }
    }
}

void *fn_1_1D810(s32 registrationIndex)
{
    M699PointerSlots *slots;
    OMOBJ *object = lbl_1_bss_A68;

    slots = object->data;
    if (registrationIndex < 0) {
        return NULL;
    }
    if (registrationIndex < 20) {
        return slots->values[registrationIndex];
    }
    registrationIndex -= 20;
    if (registrationIndex < 20) {
        return (&slots->values[20])[registrationIndex];
    }
    return NULL;
}

void *fn_1_1D880(u32 type, s32 id)
{
    OMOBJ *object;
    M699PointerSlots *slots;
    M699CollisionRecord *entry;
    M699CollisionRecord *record;
    s32 index;

    if (lbl_1_bss_A68 != NULL) {
        object = lbl_1_bss_A68;
        slots = object->data;
        for (index = 0; index < 20; index++) {
            entry = (&slots->values[20])[index];
            if (entry != NULL) {
                record = entry;
                if (record->mode == type && record->id == id) {
                    return entry;
                }
            }
        }
    }
    return NULL;
}

OMOBJ *lbl_1_bss_A68;
