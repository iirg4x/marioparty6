/* Updates collision entries and resolves segment, boundary, and mesh contacts. */
#include "dolphin/math.h"
#include "REL/m626Dll/collision.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include <string.h>

/* Creates the collision manager and its entry tables during minigame setup. */
void fn_1_E268(OMOBJMAN *objMan)
{
    OMOBJ *obj = omAddObjEx(objMan, 18, 0, 0, OM_GRP_NONE, fn_1_E2F4);
    void *data;
    lbl_1_bss_6D0 = obj;
    data = HuMemDirectMallocNum(HEAP_HEAP, 192, HU_MEMNUM_OVL);
    obj->data = data;
    memset(data, 0, 192);
}

void fn_1_E2F0(void)
{
}

void fn_1_E2F4(OMOBJ *obj)
{
    obj->objFunc = fn_1_E304;
}

/* Tests registered actors each manager tick, then invokes their update callbacks. */
void fn_1_E304(OMOBJ *obj)
{
    M626EntryWork *work = obj->data;
    void *first;
    void *second;
    M626Entry *current;
    M626Entry *candidate;
    s32 i;
    s32 j;

    for (i = 0; i < 38; i++) {
        first = work->entries[i];
        if (first != NULL) {
            current = first;
        } else {
            continue;
        }
        if ((current->flags & 1) != 0 && current->state == 0) {
            for (j = i + 1; j < 38; j++) {
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
                    fn_1_E56C(first, second);
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
                            fn_1_E994(first, second);
                            break;
                        case 11:
                            fn_1_EC80(first, second);
                            break;
                        case 12:
                            fn_1_ED80(first, second);
                            break;
                        }
                    }
                }
            }
        }
    }

    for (i = 0; i < 38; i++) {
        first = work->entries[i];
        if (first != NULL) {
            current = first;
            if (current->invoke != NULL) {
                current->invoke((s32)current->primaryRelation);
            }
        }
    }
}

/* Separates two overlapping moving segments and calls both collision handlers. */
s32 fn_1_E56C(M626Entry *first, M626Entry *second)
{
    Vec firstStart;
    Vec firstEnd;
    Vec secondStart;
    Vec secondEnd;
    Vec firstClosest;
    Vec secondClosest;
    Vec difference;
    f32 firstAngle;
    f32 secondAngle;
    f32 distance;
    f32 radius;
    f32 normalX;
    f32 normalZ;

    if (first->matrix == NULL) {
        PSVECAdd(first->position, &first->segmentStart, &firstStart);
        PSVECAdd(first->position, &first->segmentEnd, &firstEnd);
    } else {
        PSMTXMultVec(*first->matrix, &first->segmentStart, &firstStart);
        PSVECAdd(&firstStart, first->position, &firstStart);
        PSMTXMultVec(*first->matrix, &first->segmentEnd, &firstEnd);
        PSVECAdd(&firstEnd, first->position, &firstEnd);
    }

    if (first->type == 1 && second->type == 1) {
        firstAngle = (f32)(180.0 *
            (atan2((f64)first->direction->x, (f64)first->direction->z) /
             3.141592653589793));
        secondAngle = (f32)(180.0 *
            (atan2((f64)second->direction->x, (f64)second->direction->z) /
             3.141592653589793));
        if (fabs(fn_1_3FC8(firstAngle, secondAngle)) < 30.0) {
            /* Similar-facing type-1 entries do not trigger this pair collision. */
            return 0;
        }
    }

    if (second->matrix == NULL) {
        PSVECAdd(second->position, &second->segmentStart, &secondStart);
        PSVECAdd(second->position, &second->segmentEnd, &secondEnd);
    } else {
        PSMTXMultVec(*second->matrix, &second->segmentStart, &secondStart);
        PSVECAdd(&secondStart, second->position, &secondStart);
        PSMTXMultVec(*second->matrix, &second->segmentEnd, &secondEnd);
        PSVECAdd(&secondEnd, second->position, &secondEnd);
    }

    fn_1_F264(&firstStart, &firstEnd, &secondStart, &secondEnd,
              &firstClosest, &secondClosest);
    PSVECSubtract(&secondClosest, &firstClosest, &difference);
    distance = PSVECMag(&difference);
    radius = first->radius + second->radius;
    if (radius > distance) {
        if (distance == 0.0) {
            normalX = 0.0f;
            normalZ = -1.0f;
        } else {
            normalX = difference.x / distance;
            normalZ = difference.z / distance;
        }
        if (first->collisionMode != 0.0 || second->collisionMode == 0.0) {
            first->position->x += difference.x - (normalX * radius);
            first->position->z += difference.z - (normalZ * radius);
        }
        if (second->collisionMode != 0.0 || first->collisionMode == 0.0) {
            second->position->x += -difference.x + (normalX * radius);
            second->position->z += -difference.z + (normalZ * radius);
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

/* Keeps a moving entry on the configured side of an ellipse during collision checks. */
s32 fn_1_E994(M626Entry *motion, M626EllipseBounds *settings)
{
    f32 x;
    f32 z;
    f32 distance;
    f32 radius;
    f32 angle;

    radius = settings->radiusZ / settings->radiusX;
    x = radius * (motion->position->x - settings->origin.x);
    z = motion->position->z - settings->origin.z;
    distance = sqrtf(x * x + z * z);
    if (((settings->direction != 0) && (distance > settings->radiusZ)) ||
        ((settings->direction == 0) && (distance < settings->radiusZ))) {
        angle = (f32)(180.0 * (atan2((f64)x, (f64)z) / 3.141592653589793));
        motion->position->x =
            (f32) ((f64) settings->origin.x +
                   ((f64) settings->radiusX * sin((3.141592653589793 * (f64) angle) / 180.0)));
        motion->position->z =
            (f32) ((f64) settings->origin.z +
                   ((f64) settings->radiusZ * cos((3.141592653589793 * (f64) angle) / 180.0)));
        if (motion->collisionCallback != NULL) {
            motion->collisionCallback(motion, settings);
        }
        return 1;
    }
    return 0;
}

/* Clamps an entry to the X/Z rectangle and reports whether its position changed. */
s32 fn_1_EC80(M626Entry *owner, M626RangeBounds *limits) {
    s32 changed;

    changed = 0;
    if (owner->position->x < limits->origin.x) {
        changed = 1;
        owner->position->x = limits->origin.x;
    }
    if (owner->position->x > limits->extent.x) {
        changed = 1;
        owner->position->x = limits->extent.x;
    }
    if (owner->position->z < limits->origin.z) {
        changed = 1;
        owner->position->z = limits->origin.z;
    }
    if (owner->position->z > limits->extent.z) {
        changed = 1;
        owner->position->z = limits->extent.z;
    }
    if ((changed != 0) && (owner->collisionCallback != NULL)) {
        owner->collisionCallback(owner, limits);
    }
    return changed;
}

/* Pushes a moving entry away from intersected mesh edges during the manager pass. */
s32 fn_1_ED80(M626Entry *body, M626SurfaceCollider *manager)
{
    s32 hit;
    M626WorldEdge * edge;
    s32 index;
    void(* bodyCallback)(void *, void *);
    void(* managerCallback)(void *, void *);
    Vec * collisionPosition;
    Vec * start;
    Vec * end;
    f32 radiusScale;
    f32 denominator;
    f32 side;
    f32 xOffset;
    f32 zOffset;
    f32 edgeDistance;
    f32 endDistance;
    f32 startDeltaZ;
    Vec point;
    bodyCallback = body->collisionCallback;
    managerCallback = manager->collisionCallback;
    hit = 0;
    fn_1_F190(body);
    edge = manager->current;
    collisionPosition = body->position;
    end = & body->worldEnd;
    start = & body->worldStart;
    if (body->type == 1)
    {
        radiusScale = (f32)(0.5 * (f64) body->radius);
    }
    else
    {
        radiusScale = (f32)(3.0 * (f64) body->radius);
    }
    for (index = 0; index < manager->edgeCount; index++, edge++)
    {
        endDistance = edge->planeConstant + (edge->normal.x * end->x + edge->normal.z * end->z);
        if (endDistance < 0.0f)
        {
            continue;
        }
        {
            xOffset = start->x - edge->normal.x * radiusScale;
            zOffset = start->z - edge->normal.z * radiusScale;
            denominator =
                edge->planeConstant + (edge->normal.x * xOffset + edge->normal.z * zOffset);
            side = edge->planeConstant + (edge->normal.x * start->x + edge->normal.z * start->z);
            if (denominator >= 0.0f && side >= 0.0f)
            {
                continue;
            }
            {
                side = -side;
                denominator = endDistance + side;
                if (denominator != 0.0)
                {
                    point.x = start->x + (side * (end->x - start->x)) / denominator;
                    point.z = start->z + (side * (end->z - start->z)) / denominator;
                    xOffset = point.x - edge->first.x;
                    zOffset = point.z - edge->first.z;
                    edgeDistance = point.x - edge->second.x;
                    startDeltaZ = point.z - edge->second.z;
                    if (xOffset * edgeDistance + zOffset * startDeltaZ > 0.0f)
                    {
                        continue;
                    }
                    {
                        point.x = start->x + 100.0 * (3.0 * edge->normal.x);
                        point.z = start->z + 100.0 * (3.0 * edge->normal.z);
                        endDistance = edge->planeConstant +
                            (edge->normal.x * point.x + edge->normal.z * point.z);
                        denominator = endDistance + side;
                        if (denominator != 0.0)
                        {
                            xOffset = (side * (point.x - start->x)) / denominator +
                                      edge->normal.x * radiusScale;
                            zOffset = (side * (point.z - start->z)) / denominator +
                                      edge->normal.z * radiusScale;
                            collisionPosition->x += xOffset;
                            collisionPosition->z += zOffset;
                            start->x += xOffset;
                            start->z += zOffset;
                            manager->hitEdge = edge;
                            hit = 1;
                            if (bodyCallback != NULL)
                            {
                                bodyCallback(body, manager);
                                /* Each handler is called at most once during this scan. */
                                bodyCallback = NULL;
                            }
                            if (managerCallback != NULL)
                            {
                                managerCallback(manager, body);
                                managerCallback = NULL;
                            }
                        }
                    }
                }
            }
        }
    }
    return hit;
}

/* Transforms the local segment endpoints and stores their midpoint for collision. */
void fn_1_F190(M626Entry *bounds) {
    Vec first;
    Vec second;
    Vec delta;

    if (bounds->matrix == NULL) {
        PSVECAdd(bounds->position, &bounds->segmentStart, &first);
        PSVECAdd(bounds->position, &bounds->segmentEnd, &second);
    } else {
        PSMTXMultVec(*bounds->matrix, &bounds->segmentStart, &first);
        PSVECAdd(&first, bounds->position, &first);
        PSMTXMultVec(*bounds->matrix, &bounds->segmentEnd, &second);
        PSVECAdd(&second, bounds->position, &second);
    }
    PSVECSubtract(&second, &first, &delta);
    PSVECScale(&delta, &delta, 0.5f);
    PSVECAdd(&first, &delta, &bounds->worldStart);
}

/* Finds the nearest points on two segments for the pairwise collision check. */
s32 fn_1_F264(Vec *firstStart, Vec *firstEnd, Vec *secondStart,
              Vec *secondEnd, Vec *firstClosest, Vec *secondClosest)
{
    Vec firstDirection;
    Vec secondDirection;
    Vec offset;
    f32 ratio = 0.0f;
    f32 firstLengthSquared;
    f32 secondLengthSquared;
    f32 directionDot;
    f32 denominator;
    f32 offsetDot;
    f32 reverseOffsetDot;
    f32 firstRatio;
    f32 secondRatio;

    PSVECSubtract(firstEnd, firstStart, &firstDirection);
    PSVECSubtract(secondEnd, secondStart, &secondDirection);
    firstLengthSquared = PSVECSquareMag(&firstDirection);
    secondLengthSquared = PSVECSquareMag(&secondDirection);
    if (firstLengthSquared < 0.010000001f) {
        *firstClosest = *firstStart;
        fn_1_F598(secondStart, &secondDirection, firstStart, 0,
                  secondClosest, &ratio);
    } else if (secondLengthSquared < 0.010000001f) {
        *secondClosest = *secondStart;
        fn_1_F598(firstStart, &firstDirection, secondStart, 0,
                  firstClosest, &ratio);
    } else {
        PSVECSubtract(secondStart, firstStart, &offset);
        directionDot = -PSVECDotProduct(&firstDirection, &secondDirection);
        denominator = (firstLengthSquared * secondLengthSquared) -
                      (directionDot * directionDot);
        if (fabs(denominator) < 0.10000000149011612) {
            fn_1_F73C(firstStart, firstEnd, &firstDirection,
                      secondStart, secondEnd, &secondDirection,
                      firstClosest, secondClosest);
        } else {
            offsetDot = PSVECDotProduct(&firstDirection, &offset);
            reverseOffsetDot = -PSVECDotProduct(&secondDirection, &offset);
            secondRatio = ((firstLengthSquared * reverseOffsetDot) -
                           (offsetDot * directionDot)) / denominator;
            firstRatio = (offsetDot - (directionDot * secondRatio)) /
                         firstLengthSquared;
            PSVECScale(&firstDirection, firstClosest, firstRatio);
            PSVECAdd(firstClosest, firstStart, firstClosest);
            PSVECScale(&secondDirection, secondClosest, secondRatio);
            PSVECAdd(secondClosest, secondStart, secondClosest);
            if (firstRatio < 0.0f || firstRatio > 1.0f ||
                secondRatio < 0.0f || secondRatio > 1.0f) {
                fn_1_FC4C(firstStart, &firstDirection, secondStart,
                          &secondDirection, firstRatio, secondRatio,
                          firstClosest, secondClosest);
            }
        }
    }
    return PSVECSquareDistance(firstClosest, secondClosest) <
           0.010000001f;
}

/* Projects a point onto a segment, optionally clamping its fraction to the endpoints. */
static inline void fn_1_F598(Vec *origin, Vec *direction, Vec *point, s32 clampResult,
                             Vec *result, f32 *ratio)
{
    Vec offset;
    f32 squaredLength;

    squaredLength = PSVECSquareMag(direction);
    if (squaredLength < 0.010000001f) {
        *result = *origin;
        return;
    }

    PSVECSubtract(point, origin, &offset);
    *ratio = PSVECDotProduct(direction, &offset) / squaredLength;
    if (clampResult == 0) {
        *ratio = 0.0f >
            (1.0f < *ratio ? 1.0f : *ratio) ?
            0.0f :
            (1.0f < *ratio ? 1.0f : *ratio);
    }
    PSVECScale(direction, result, *ratio);
    PSVECAdd(result, origin, result);
}

/* Resolves nearly parallel segments by testing their endpoint projections. */
void fn_1_F73C(Vec *firstStart, Vec *firstEnd, Vec *firstDirection,
               Vec *secondStart, Vec *secondEnd, Vec *secondDirection,
               Vec *firstClosest, Vec *secondClosest)
{
    Vec secondEndProjection;
    M626ProjectionRatios ratios;
    f32 middleRatio;

    fn_1_F598(firstStart, firstDirection, secondStart, 1, firstClosest,
              &ratios.start);
    fn_1_F598(firstStart, firstDirection, secondEnd, 1,
              &secondEndProjection, &ratios.end);

    if (ratios.start < 0.0f && ratios.end < 0.0f) {
        *firstClosest = *firstStart;
        if (ratios.start < ratios.end) {
            *secondClosest = *secondEnd;
        } else {
            *secondClosest = *secondStart;
        }
    } else if (ratios.start > 1.0f && ratios.end > 1.0f) {
        *firstClosest = *firstEnd;
        if (ratios.start < ratios.end) {
            *secondClosest = *secondStart;
        } else {
            *secondClosest = *secondEnd;
        }
    } else {
        middleRatio = 0.5f *
        ((0.0f > (1.0f < ratios.start ? 1.0f : ratios.start) ?
            0.0f : (1.0f < ratios.start ? 1.0f : ratios.start)) +
         (0.0f > (1.0f < ratios.end ? 1.0f : ratios.end) ?
            0.0f : (1.0f < ratios.end ? 1.0f : ratios.end)));
        PSVECScale(firstDirection, firstClosest, middleRatio);
        PSVECAdd(firstClosest, firstStart, firstClosest);

        fn_1_F598(secondStart, secondDirection, firstClosest, 1, secondClosest,
                  &middleRatio);
    }
}

/* Clamps out-of-range segment fractions and recomputes the closest points. */
void fn_1_FC4C(Vec *firstStart, Vec *firstDirection, Vec *secondStart,
               Vec *secondDirection, f32 firstRatio, f32 secondRatio,
               Vec *firstClosest, Vec *secondClosest) {
    if (((firstRatio < 0.0f) || (firstRatio > 1.0f)) &&
        ((secondRatio < 0.0f) || (secondRatio > 1.0f))) {
        firstRatio = 0.0f > (1.0f < firstRatio ? 1.0f : firstRatio) ?
            0.0f : (1.0f < firstRatio ? 1.0f : firstRatio);
        PSVECScale(firstDirection, firstClosest, firstRatio);
        PSVECAdd(firstClosest, firstStart, firstClosest);
        fn_1_F598(secondStart, secondDirection, firstClosest, 1,
                  secondClosest, &secondRatio);
        if ((secondRatio < 0.0f) || (secondRatio > 1.0f)) {
            secondRatio = 0.0f > (1.0f < secondRatio ? 1.0f : secondRatio) ?
                0.0f : (1.0f < secondRatio ? 1.0f : secondRatio);
            PSVECScale(secondDirection, secondClosest, secondRatio);
            PSVECAdd(secondClosest, secondStart, secondClosest);
            fn_1_F598(firstStart, firstDirection, secondClosest, 0,
                      firstClosest, &firstRatio);
            fn_1_F598(secondStart, secondDirection, firstClosest, 0,
                      secondClosest, &secondRatio);
        }
    } else {
        if ((firstRatio < 0.0f) || (firstRatio > 1.0f)) {
            firstRatio = 0.0f > (1.0f < firstRatio ? 1.0f : firstRatio) ?
                0.0f : (1.0f < firstRatio ? 1.0f : firstRatio);
            PSVECScale(firstDirection, firstClosest, firstRatio);
            PSVECAdd(firstClosest, firstStart, firstClosest);
            fn_1_F598(secondStart, secondDirection, firstClosest, 0,
                      secondClosest, &secondRatio);
        } else if ((secondRatio < 0.0f) || (secondRatio > 1.0f)) {
            secondRatio = 0.0f > (1.0f < secondRatio ? 1.0f : secondRatio) ?
                0.0f : (1.0f < secondRatio ? 1.0f : secondRatio);
            PSVECScale(secondDirection, secondClosest, secondRatio);
            PSVECAdd(secondClosest, secondStart, secondClosest);
            fn_1_F598(firstStart, firstDirection, secondClosest, 0,
                      firstClosest, &firstRatio);
        }
    }
}

/* Adds an actor or position record to the first free primary collision slot. */
int fn_1_10598(void *value) {
    OMOBJ *object = lbl_1_bss_6D0;
    M626EntryWork *data = object->data;
    void **entry = &data->entries[0];
    int index;

    for (index = 0; index < 38; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index;
        }
    }
    return -1;
}

/* Adds a bounds or mesh record and returns its combined-table index. */
s32 fn_1_10610(void *value)
{
    OMOBJ *object = lbl_1_bss_6D0;
    M626EntryWork *table = object->data;
    void **entry = table->secondaryEntries;
    s32 index;

    for (index = 0; index < 10; index++, entry++) {
        if (*entry == NULL) {
            *entry = value;
            return index + 38;
        }
    }
    return -1;
}

/* Removes a collision record from the primary or secondary table by index. */
void fn_1_10688(s32 index)
{
    OMOBJ *object = lbl_1_bss_6D0;
    M626EntryWork *table = object->data;

    if (index >= 0) {
        if (index < 38) {
            table->entries[index] = 0;
        }
        index -= 38;
        if (index < 10) {
            table->secondaryEntries[index] = 0;
        }
    }
}

/* Returns the collision record stored at a combined-table index. */
void *fn_1_106EC(s32 index)
{
    OMOBJ *object = lbl_1_bss_6D0;
    M626EntryWork *table = object->data;

    if (index < 0) {
        return 0;
    }
    if (index < 38) {
        return table->entries[index];
    }
    index -= 38;
    if (index < 10) {
        return table->secondaryEntries[index];
    }
    return 0;
}

/* Builds local quad edges and transform storage for a surface collider. */
void fn_1_1075C(M626SurfaceCollider *record, HSF_OBJECT *objects, s32 count)
{
    HSF_OBJECT *object;
    s32 length;
    M626MeshEdge *edge;
    HSF_FACE *face;
    Vec *vertices;
    Vec *first;
    Vec *second;
    HSF_TRANSFORM **transforms;
    s32 corner;
    s32 objectsRemaining;
    Mtx *matrix;
    s32 quadCount = 0;
    s32 nextCorner;

    object = objects;
    objectsRemaining = count;
    while (objectsRemaining--) {
        if (object->type == 2) {
            face = object->mesh.face->data;
            length = object->mesh.face->count;
            while (length--) {
                if (face->type == 3) {
                    quadCount++;
                }
                face++;
            }
        }
        object++;
    }

    length = quadCount * sizeof(M626MeshEdge);
    edge = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    record->original = edge;
    memset(edge, 0, length);
    length = quadCount * sizeof(M626WorldEdge);
    record->current = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    memset(record->current, 0, length);
    record->alternate = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    memset(record->alternate, 0, length);
    record->transformCount = count;
    length = record->transformCount * sizeof(HSF_TRANSFORM *);
    transforms = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    record->transforms = transforms;
    memset(transforms, 0, length);
    length = record->transformCount * sizeof(Mtx);
    matrix = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    record->matrices = matrix;
    memset(matrix, 0, length);

    object = objects;
    objectsRemaining = count;
    record->edgeCount = 0;
    while (objectsRemaining--) {
        if (object->type == 2) {
            vertices = object->mesh.vertex->data;
            face = object->mesh.face->data;
            length = object->mesh.face->count;
            while (length--) {
                if (face->type == 3) {
                    for (corner = 0; corner < 4; corner++) {
                        nextCorner = (corner + 1) & 3;
                        first = &vertices[face->index[lbl_1_data_908[corner]].vertex];
                        second = &vertices[face->index[lbl_1_data_908[nextCorner]].vertex];
                        if ((first->y < 0.0) && (second->y < 0.0)) {
                            /* Keep the first quad edge whose vertices lie below local Y zero. */
                            edge->first.x = first->x;
                            edge->first.z = first->z;
                            edge->second.x = second->x;
                            edge->second.z = second->z;
                            edge->matrix = matrix;
                            edge++;
                            record->edgeCount++;
                            break;
                        }
                    }
                }
                face++;
            }
        }
        *transforms = &object->mesh.base;
        transforms++;
        matrix++;
        object++;
    }
}

/* Builds surface collision data from every object in a model. */
void fn_1_10A1C(M626SurfaceCollider *collider, s16 modelNo) {
    HSF_DATA *hsf = Hu3DData[modelNo].hsf;
    fn_1_1075C(collider, hsf->object, hsf->objectNum);
}

/* Builds surface collision data from one model object. */
void fn_1_10A74(M626SurfaceCollider *collider, HSF_OBJECT *object) {
    fn_1_1075C(collider, object, 1);
}

/* Rebuilds transformed mesh edges and their X/Z plane normals during scenery updates. */
void fn_1_10AA8(M626SurfaceCollider *record)
{
    M626WorldEdge *edge;
    HSF_TRANSFORM **transforms;
    M626MeshEdge *original;
    s32 i;
    Mtx *matrix;
    Mtx temporary;
    Mtx rotation;
    Mtx transform;
    f64 zDifference;
    f64 xDifference;

    f64 magnitude;

    transforms = record->transforms;
    matrix = record->matrices;
    original = record->original;
    edge = record->alternate;
    record->alternate = record->current;
    record->current = edge;

    for (i = 0; i < record->transformCount; transforms++, matrix++, i++) {
        PSMTXRotRad(rotation, 'z', (0.01745329238474369f) * (*transforms)->rot.z);
        PSMTXRotRad(temporary, 'y', (0.01745329238474369f) * (*transforms)->rot.y);
        PSMTXConcat(rotation, temporary, transform);
        PSMTXRotRad(rotation, 'x', (0.01745329238474369f) * (*transforms)->rot.x);
        PSMTXConcat(transform, rotation, temporary);
        PSMTXTrans(rotation, (*transforms)->pos.x, (*transforms)->pos.y, (*transforms)->pos.z);
        PSMTXConcat(rotation, temporary, transform);
        PSMTXScale(rotation, (*transforms)->scale.x, (*transforms)->scale.y,
                   (*transforms)->scale.z);
        PSMTXConcat(transform, rotation, *matrix);
    }
    for (i = 0; i < record->edgeCount; edge++, original++, i++) {
        PSMTXMultVec(*original->matrix, &original->first, &edge->first);
        PSMTXMultVec(*original->matrix, &original->second, &edge->second);
        zDifference = edge->second.z - edge->first.z;
        xDifference = edge->second.x - edge->first.x;
        magnitude = sqrt(zDifference * zDifference + xDifference * xDifference);
        if (0.0 != magnitude) {
            zDifference /= magnitude;
            xDifference /= magnitude;
        }
        edge->normal.x = -zDifference;
        edge->normal.y = 0.0f;
        edge->normal.z = xDifference;
        edge->planeConstant = -((-zDifference * edge->first.x) +
                                 (xDifference * edge->first.z));
    }
}

/* Reports whether a point lies inside every current mesh-edge half-plane. */
s32 fn_1_10EA0(M626SurfaceCollider *work, Vec *position)
{
    M626WorldEdge *entry = work->current;
    s32 index;

    for (index = 0; index < work->edgeCount; index++, entry++) {
        float value =
            entry->planeConstant + (entry->normal.x * position->x + entry->normal.z * position->z);
        if (value < 0.0f) {
            return 0;
        }
    }
    return 1;
}

s32 lbl_1_data_908[4] = {0, 1, 3, 2};
OMOBJ *lbl_1_bss_6D0;
