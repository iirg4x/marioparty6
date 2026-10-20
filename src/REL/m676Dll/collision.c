/* Collision registration and contact resolution for scene objects. */
#include "dolphin/math.h"
#include "REL/m676Dll/collision_types.h"
#include "game/memory.h"
#include <string.h>

#define M676_COLLISION_MANAGER_PRIORITY 18
#define M676_COLLISION_WORK_SIZE 384

OMOBJ *lbl_1_bss_10B8;
void fn_1_5698(Vec *start, Vec *end, f32 factor, Vec *result);

s32 fn_1_1269C(void);
void fn_1_126A4(HUPROCESS *process);
void fn_1_1272C(void);
void fn_1_12730(OMOBJ *obj);
void fn_1_12740(OMOBJ *obj);
s32 fn_1_12E04(M676CollisionEntry *first, M676CollisionEntry *second);
s32 fn_1_131B4(M676CollisionEntry *object, M676CollisionEntry *region);
s32 fn_1_1378C(M676CollisionEntry *state, M676CollisionEntry *parameters);
s32 fn_1_13A78(M676CollisionEntry *object, M676CollisionEntry *bounds);
s32 fn_1_13C5C(s32 axis, M676CollisionEntry *object,
    M676CollisionEntry *bounds, f32 factor);
s32 fn_1_13E3C(M676CollisionEntry *owner, M676CollisionEntry *limits);
s32 fn_1_13EC4(M676CollisionEntry *registration);
int fn_1_13F54(void *registration);
void fn_1_13FE4(s32 index);
void *fn_1_14064(s32 index);
void *fn_1_140F0(u32 type, s32 id);

#define M676_FLOAT_EXPONENT_MASK 0x7F800000
#define M676_FLOAT_FRACTION_MASK 0x007FFFFF

/* Classifies a position component so the collision pass can reject invalid coordinates. */
static inline s32 M676FloatClass(f32 value)
{
    switch (*(s32 *)&value & M676_FLOAT_EXPONENT_MASK) {
    case M676_FLOAT_EXPONENT_MASK:
        if ((*(s32 *)&value & M676_FLOAT_FRACTION_MASK) != 0) {
            return 1;
        }
        return 2;
    case 0:
        if ((*(s32 *)&value & M676_FLOAT_FRACTION_MASK) != 0) {
            return 5;
        }
        return 3;
    default:
        return 4;
    }
}

/* Detects NaN or infinite coordinates before the owner update; contact callbacks have already
 * run. */
static inline s32 M676CollisionPositionInvalid(Vec *position)
{
    return M676FloatClass(position->x) == 1 ||
        M676FloatClass(position->x) == 2 ||
        M676FloatClass(position->y) == 1 ||
        M676FloatClass(position->y) == 2 ||
        M676FloatClass(position->z) == 1 ||
        M676FloatClass(position->z) == 2;
}

s32 fn_1_1269C(void) { return 0; }

/* Creates the collision manager during scene setup and clears its registration tables. */
void fn_1_126A4(HUPROCESS *process)
{
    OMOBJ *object;
    void *data;

    object = omAddObjEx(process, M676_COLLISION_MANAGER_PRIORITY, 0, 0, -1, fn_1_12730);
    lbl_1_bss_10B8 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, M676_COLLISION_WORK_SIZE, HU_MEMNUM_OVL);
    object->data = data;
    memset(data, 0, M676_COLLISION_WORK_SIZE);
}

void fn_1_1272C(void)
{
}

void fn_1_12730(OMOBJ *obj)
{
    obj->objFunc = fn_1_12740;
}

/* Runs each frame from the collision manager callback to resolve contacts and update owners. */
void fn_1_12740(OMOBJ *obj)
{
    M676CollisionWork *work = obj->data;
    M676CollisionEntry *first;
    M676CollisionEntry *second;
    M676CollisionHeader *firstHeader;
    M676CollisionHeader *secondHeader;
    M676CollisionEntry *positionNode;
    Vec *position;
    s32 firstIndex;
    s32 secondIndex;

    for (firstIndex = 0; firstIndex < M676_COLLISION_OBJECT_COUNT; firstIndex++) {
        first = work->objects[firstIndex];
        if (first != NULL && first->header.state == 0) {
            first->shape.dynamic.active = 0;
        }
    }

    for (firstIndex = 0; firstIndex < M676_COLLISION_OBJECT_COUNT; firstIndex++) {
        first = work->objects[firstIndex];
        if (first == NULL) {
            continue;
        }
        firstHeader = &first->header;
        if ((firstHeader->flags & 1) == 0 || firstHeader->state != 0) {
            continue;
        }
        for (secondIndex = firstIndex + 1; secondIndex < M676_COLLISION_OBJECT_COUNT;
            secondIndex++) {
            second = work->objects[secondIndex];
            if (second == NULL) {
                continue;
            }
            secondHeader = &second->header;
            if ((secondHeader->flags & 1) == 0) {
                continue;
            }
            if ((firstHeader->collisionMask & secondHeader->collisionMask) == 0) {
                continue;
            }
            if (firstHeader->owner != NULL &&
                firstHeader->owner == secondHeader->callbackArgument) {
                continue;
            }
            if (secondHeader->owner != NULL &&
                secondHeader->owner == firstHeader->callbackArgument) {
                continue;
            }
            if (secondHeader->state == 0) {
                fn_1_12E04(first,
                    second);
            }
        }
        if ((firstHeader->flags & 2U) == 0) {
            for (secondIndex = 0; secondIndex < 10; secondIndex++) {
                second = work->targets[secondIndex];
                if (second == NULL) {
                    continue;
                }
                secondHeader = &second->header;
                if ((secondHeader->flags & 1) == 0) {
                    continue;
                }
                if ((firstHeader->collisionMask & secondHeader->collisionMask) == 0) {
                    continue;
                }
                if (firstHeader->owner != NULL &&
                    firstHeader->owner == secondHeader->callbackArgument) {
                    continue;
                }
                if (secondHeader->owner != NULL &&
                    secondHeader->owner == firstHeader->callbackArgument) {
                    continue;
                }
                switch (secondHeader->state) {
                case 10:
                    fn_1_131B4(first, second);
                    break;
                case 11:
                    fn_1_1378C(first, second);
                    break;
                case 12:
                    fn_1_13A78(first, second);
                    break;
                case 13:
                    fn_1_13E3C(first, second);
                    break;
                }
            }
        }
    }

    for (firstIndex = 0; firstIndex < M676_COLLISION_OBJECT_COUNT; firstIndex++) {
        first = work->objects[firstIndex];
        if (first == NULL) {
            continue;
        }

        firstHeader = &first->header;
        if (firstHeader->state == 0) {
            positionNode = first;
            position = positionNode->shape.dynamic.position;
            if (M676CollisionPositionInvalid(position)) {
                *positionNode->shape.dynamic.position = positionNode->shape.dynamic.currentPosition;
            }
        }
        if (firstHeader->updateCallback != NULL) {
            firstHeader->updateCallback(firstHeader->callbackArgument);
        }
        if (firstHeader->state == 0) {
            positionNode = first;
            positionNode->shape.dynamic.currentPosition = *positionNode->shape.dynamic.position;
        }
    }
}

/* Tests dynamic overlap with 3D distance but separates only in X/Z, using a -Z normal at zero
* distance. Each permitted mover receives the full correction. */
s32 fn_1_12E04(M676CollisionEntry *first, M676CollisionEntry *second)
{
    Vec *firstPosition;
    Vec *secondPosition;
    f32 x;
    f32 y;
    f32 z;
    f32 reach;
    f32 distance;
    f32 normalX;
    f32 normalZ;

    firstPosition = first->shape.dynamic.position;
    secondPosition = second->shape.dynamic.position;
    x = secondPosition->x - firstPosition->x;
    y = secondPosition->y - firstPosition->y;
    z = secondPosition->z - firstPosition->z;
    distance = sqrtf((z * z) + ((x * x) + (y * y)));
    reach = first->shape.dynamic.radius + second->shape.dynamic.radius;
    if (reach > distance) {
        if (0.0 == distance) {
            normalX = 0.0f;
            normalZ = -1.0f;
        } else {
            normalX = x / distance;
            normalZ = z / distance;
        }

        if ((0.0 != first->shape.dynamic.movable) || (0.0 == second->shape.dynamic.movable)) {
            first->shape.dynamic.position->x += x - (normalX * reach);
            first->shape.dynamic.position->z += z - (normalZ * reach);
    }
        if ((0.0 != second->shape.dynamic.movable) || (0.0 == first->shape.dynamic.movable)) {
            second->shape.dynamic.position->x += -x + (normalX * reach);
            second->shape.dynamic.position->z += -z + (normalZ * reach);
    }

        if (first->shape.dynamic.callback != NULL) {
            first->shape.dynamic.callback(first, second);
        }
        if (second->shape.dynamic.callback != NULL) {
            second->shape.dynamic.callback(second, first);
        }
        return 1;
    }
    return 0;
}

/* Handles dynamic-object contact with a curved region during fn_1_12740's collision pass. */
s32 fn_1_131B4(M676CollisionEntry *object, M676CollisionEntry *region)
{
    Vec *position;
    Vec *center;
    f32 minimumY;
    f32 maximumY;
    f32 innerXScale;
    f32 outerXScale;
    f32 innerTestRadius;
    f32 outerTestRadius;
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

    position = object->shape.dynamic.position;
    center = &region->shape.region.center;
    minimumY = center->y + region->shape.region.lowerY;
    maximumY = minimumY + region->shape.region.verticalSpan;
    innerXScale = region->shape.region.innerRadiusZ / region->shape.region.innerRadiusX;
    outerXScale = region->shape.region.outerRadiusZ / region->shape.region.outerRadiusX;
    innerTestRadius =
        innerXScale * (region->shape.region.innerRadiusX - object->shape.dynamic.radius);
    outerTestRadius =
        outerXScale * (region->shape.region.outerRadiusX + object->shape.dynamic.radius);

    if (position->y < minimumY) {
        goto noCollision;
    }
    verticalDistance = maximumY - position->y;
    if (verticalDistance < 0.0f) {
        goto noCollision;
    }
    innerScaledX = innerXScale * (position->x - center->x);
    deltaZ = position->z - center->z;
    innerDistance = sqrtf((innerScaledX * innerScaledX) + (deltaZ * deltaZ));
    innerGap = innerDistance - innerTestRadius;
    if (innerGap < 0.0f) {
        goto noCollision;
    }
    outerScaledX = outerXScale * (position->x - center->x);
    secondDeltaZ = position->z - center->z;
    outerDistance = sqrtf((outerScaledX * outerScaledX) +
        (secondDeltaZ * secondDeltaZ));
    outerGap = outerTestRadius - outerDistance;
    if (outerGap < 0.0f) {
        goto noCollision;
    }
    if (verticalDistance < innerGap && verticalDistance < outerGap &&
        (object->header.flags & 4) == 0) {
        position->y = maximumY;
        object->shape.dynamic.velocity->y = 0.0f;
    } else {
        if (innerGap < outerGap) {
            edgeX = region->shape.region.innerRadiusX - object->shape.dynamic.radius;
            edgeZ = region->shape.region.innerRadiusZ - object->shape.dynamic.radius;
            contactAngle = (f32)(180.0 *
                (atan2((f64)innerScaledX, (f64)deltaZ) / 3.141592653589793));
        } else {
            edgeX = region->shape.region.outerRadiusX + object->shape.dynamic.radius;
            edgeZ = region->shape.region.outerRadiusZ + object->shape.dynamic.radius;
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
    if (object->shape.dynamic.callback != NULL) {
        object->shape.dynamic.callback(object, region);
    }
    return 1;
noCollision:
    return 0;
}

/* Keeps a dynamic object inside or outside an ellipse during fn_1_12740's collision pass. */
s32 fn_1_1378C(M676CollisionEntry *state, M676CollisionEntry *parameters)
{
    f32 x;
    f32 z;
    f32 distance;
    f32 angle;
    f32 xScale;

    xScale = parameters->shape.ellipse.radiusZ / parameters->shape.ellipse.radiusX;
    x = xScale * (state->shape.dynamic.position->x - parameters->shape.ellipse.center.x);
    z = state->shape.dynamic.position->z - parameters->shape.ellipse.center.z;
    distance = sqrtf(x * x + z * z);

    if ((parameters->shape.ellipse.direction != 0 &&
         distance > parameters->shape.ellipse.radiusZ) ||
        (parameters->shape.ellipse.direction == 0 &&
         distance < parameters->shape.ellipse.radiusZ)) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / 3.141592653589793));
        state->shape.dynamic.position->x = parameters->shape.ellipse.center.x +
            parameters->shape.ellipse.radiusX * sin((3.141592653589793 * (f64)angle) / 180.0);
        state->shape.dynamic.position->z = parameters->shape.ellipse.center.z +
            parameters->shape.ellipse.radiusZ * cos((3.141592653589793 * (f64)angle) / 180.0);
        if (state->shape.dynamic.callback != NULL) {
            state->shape.dynamic.callback(state, parameters);
        }
        return 1;
    }
    return 0;
}

/* Sweeps a dynamic object's radius against box faces during fn_1_12740's collision pass. */
s32 fn_1_13A78(M676CollisionEntry *object, M676CollisionEntry *bounds)
{
    Vec *livePosition;
    Vec *sampledPosition;
    Vec *minimum;
    Vec *maximum;
    s32 axis;

    livePosition = object->shape.dynamic.position;
    sampledPosition = &object->shape.dynamic.currentPosition;
    minimum = &bounds->shape.bounds.minimum;
    maximum = &bounds->shape.bounds.maximum;
    for (axis = 0; axis < 3; axis++) {
        f32 liveComponent;
        f32 sampledComponent;
        f32 lower;
        f32 upper;
        f32 distance;
        f32 penetration;

        liveComponent = ((f32 *)livePosition)[axis];
        sampledComponent = ((f32 *)sampledPosition)[axis];
        lower = ((f32 *)minimum)[axis];
        upper = ((f32 *)maximum)[axis];
        if (!((sampledComponent + object->shape.dynamic.radius) > lower) &&
            !((liveComponent + object->shape.dynamic.radius) <= lower)) {
            distance = liveComponent - sampledComponent;
            if (!(distance <= 0.0f)) {
                penetration = lower - (sampledComponent + object->shape.dynamic.radius);
                if (!(penetration < 0.0f) &&
                    fn_1_13C5C(axis, object, bounds,
                               penetration / distance) != 0) {
                    return 1;
                }
            }
        }
        if (!((sampledComponent - object->shape.dynamic.radius) < upper) &&
            !((liveComponent - object->shape.dynamic.radius) >= upper)) {
            distance = sampledComponent - liveComponent;
            if (!(distance <= 0.0f)) {
                penetration = (sampledComponent - object->shape.dynamic.radius) - upper;
                if (!(penetration < 0.0f) &&
                    fn_1_13C5C(axis, object, bounds,
                               penetration / distance) != 0) {
                    return 1;
                }
            }
        }
    }
    return 0;
}

/* Interpolates the swept position, rejects overlap outside the other bounds, then moves the object
 * and reverses the tested velocity component to 80%. */
s32 fn_1_13C5C(s32 axis, M676CollisionEntry *object,
    M676CollisionEntry *bounds, f32 factor)
{
    Vec position;
    Vec *minimum;
    Vec *maximum;

    minimum = &bounds->shape.bounds.minimum;
    maximum = &bounds->shape.bounds.maximum;
    fn_1_5698(&object->shape.dynamic.currentPosition, object->shape.dynamic.position,
              factor, &position);
    if (axis != 0) {
        if (position.x + object->shape.dynamic.radius < minimum->x) {
            return 0;
        }
        if (position.x - object->shape.dynamic.radius > maximum->x) {
            return 0;
        }
    }
    if (axis != 1) {
        if (position.y + object->shape.dynamic.radius < minimum->y) {
            return 0;
        }
        if (position.y - object->shape.dynamic.radius > maximum->y) {
            return 0;
        }
    }
    if (axis != 2) {
        if (position.z + object->shape.dynamic.radius < minimum->z) {
            return 0;
        }
        if (position.z - object->shape.dynamic.radius > maximum->z) {
            return 0;
        }
    }

    *object->shape.dynamic.position = position;
    switch (axis) {
    case 0:
        object->shape.dynamic.velocity->x *= -0.8;
        break;
    case 1:
        object->shape.dynamic.velocity->y *= -0.8;
        break;
    case 2:
        object->shape.dynamic.velocity->z *= -0.8;
        break;
    }
    return 1;
}

/* Clamps a dynamic object's X and Z position to bounds during fn_1_12740's pass. */
s32 fn_1_13E3C(M676CollisionEntry *owner, M676CollisionEntry *limits) {
    if (owner->shape.dynamic.position->x < limits->shape.bounds.minimum.x) {
        owner->shape.dynamic.position->x = limits->shape.bounds.minimum.x;
    }
    if (owner->shape.dynamic.position->x > limits->shape.bounds.maximum.x) {
        owner->shape.dynamic.position->x = limits->shape.bounds.maximum.x;
    }
    if (owner->shape.dynamic.position->z < limits->shape.bounds.minimum.z) {
        owner->shape.dynamic.position->z = limits->shape.bounds.minimum.z;
    }
    if (owner->shape.dynamic.position->z > limits->shape.bounds.maximum.z) {
        owner->shape.dynamic.position->z = limits->shape.bounds.maximum.z;
    }
    return 0;
}

/* Registers a dynamic collider in the manager table when scene objects are created. */
s32 fn_1_13EC4(M676CollisionEntry *registration)
{
    OMOBJ *object;
    M676CollisionWork *work;
    M676CollisionEntry **entry;
    s32 index;

    if (lbl_1_bss_10B8 != NULL) {
        object = lbl_1_bss_10B8;
        work = object->data;
        entry = work->objects;
        for (index = 0; index < M676_COLLISION_OBJECT_COUNT; index++, entry++) {
            if (*entry == NULL) {
                *entry = registration;
                return index;
            }
        }
    }
    return -1;
}

/* Registers a static collision target and returns its manager-table index. */
int fn_1_13F54(void *registration)
{
    OMOBJ *object;
    M676CollisionWork *work;
    M676CollisionEntry **entry;
    s32 index;

    if (lbl_1_bss_10B8 != NULL) {
        object = lbl_1_bss_10B8;
        work = object->data;
        entry = work->targets;
        for (index = 0; index < 10; index++, entry++) {
            if (*entry == NULL) {
                *entry = registration;
                return index + M676_COLLISION_OBJECT_COUNT;
            }
        }
    }
    return -1;
}

/* Removes a registration; dynamic indices also clear the same objects slot through targets[index -
 * 84]. */
void fn_1_13FE4(s32 index)
{
    if (lbl_1_bss_10B8 != NULL) {
        OMOBJ *object = lbl_1_bss_10B8;
        M676CollisionWork *table = object->data;

        if (index >= 0) {
            if (index < M676_COLLISION_OBJECT_COUNT) {
                table->objects[index] = NULL;
            }
            index -= M676_COLLISION_OBJECT_COUNT;
            if (index < 10) {
                table->targets[index] = NULL;
            }
        }
    }
}

/* Returns the registered collider or target at a manager-table index. */
void *fn_1_14064(s32 index)
{
    if (lbl_1_bss_10B8 != NULL) {
        OMOBJ *object = lbl_1_bss_10B8;
        M676CollisionWork *table = object->data;

        if (index < 0) {
            return NULL;
        }
        if (index < M676_COLLISION_OBJECT_COUNT) {
            return table->objects[index];
        }
        index -= M676_COLLISION_OBJECT_COUNT;
        if (index < 10) {
            return table->targets[index];
        }
    }
    return NULL;
}

/* Finds a static target by type and ID for scene code that needs that registration. */
void *fn_1_140F0(u32 type, s32 id)
{
    OMOBJ *object;
    M676CollisionWork *table;
    M676CollisionEntry *entry;
    M676CollisionEntry *record;
    s32 index;

    if (lbl_1_bss_10B8 != NULL) {
        object = lbl_1_bss_10B8;
        table = object->data;
        for (index = 0; index < 10; index++) {
            entry = table->targets[index];
            if (entry != NULL) {
                record = entry;
                if (record->header.type == type && record->header.id == id) {
                    return entry;
                }
            }
        }
    }
    return NULL;
}
