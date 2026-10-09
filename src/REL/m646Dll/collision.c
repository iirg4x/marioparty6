/* Builds and updates the collision edges used by Hyper Sniper actors and stage attachments. */
#include "dolphin/math.h"
#include "REL/m646Dll/module_types.h"
#include "REL/m646Dll/m646_float_classification.h"

#define M646_COLLISION_PRIORITY 70
#define M646_COLLISION_SLOT_BYTES 640

extern M646PositionTable lbl_1_bss_84;

typedef struct M646MeshEdge_s {
    Vec first; /* First endpoint in the source mesh's local coordinates. */
    Vec second; /* Second endpoint in the source mesh's local coordinates. */
    Mtx *matrix; /* Transform used to produce the world-space edge. */
} M646MeshEdge;

typedef struct M646WorldEdge_s {
    Vec first; /* First endpoint in world coordinates. */
    Vec second; /* Second endpoint in world coordinates. */
    Vec normal; /* Edge normal; zero when its endpoints coincide, including model-target buffers. */
    f32 planeConstant; /* Plane offset in world units. */
} M646WorldEdge;

typedef struct M646CollisionRecord_s {
    u8 stageCollisionPrefix[32]; /* Stage collision data before the edge list. */
    s32 edgeCount; /* Number of quad edges included in the collision sweep. */
    M646WorldEdge *current; /* Edges used by this frame's collision checks. */
    M646WorldEdge *alternate; /* Spare edge buffer swapped with current during transform updates. */
    M646MeshEdge *original; /* Source-mesh endpoints retained for transforms. */
    s32 transformCount; /* Number of source objects with transform records. */
    HSF_TRANSFORM **transforms; /* Source transforms for the HSF object list. */
    Mtx *matrices; /* Per-object matrices used to transform source edges. */
} M646CollisionRecord;

void fn_1_4044(M646CollisionRecord *record, HSF_OBJECT *objects, s32 count);

extern s32 lbl_1_data_11C[4];

typedef struct M646BodyAnchor_s M646BodyAnchor;
struct M646BodyAnchor_s {
    u32 kind; /* Collision participation and response flags. */
    u32 group; /* Group bits used to filter collision pairs. */
    u32 callbackType; /* Contact category used by the response callbacks. */
    s32 playerIndex; /* Player index associated with this actor. */
    s32 callbackMode; /* Selects actor versus attachment collision processing. */
    OMOBJ *owner; /* Object passed to the update callback. */
    void (*update)(OMOBJ *); /* Runs after the manager's collision pass. */
    s32 (*contact)(void *, void *); /* Optional contact response callback. */
    Vec *position; /* Current actor position in stage units. */
    Vec *direction; /* Movement vector changed by collision response. */
    Vec savedPosition; /* Previous frame position for swept contacts. */
    f32 radius; /* Collision radius in stage units. */
    f32 responseScale; /* Scales movement added by actor-actor contact. */
};

typedef struct M646CollisionHeader_s {
    u32 kind; /* Flags checked before this record enters collision processing. */
    u32 group; /* Collision group bits. */
    u32 callbackType; /* Contact category. */
    s32 playerIndex; /* Associated player index, when the record has one. */
    s32 callbackMode; /* Selects the record's collision callback path. */
    void *owner; /* Owning actor or attachment work record. */
    void (*update)(void *); /* Optional per-frame update callback. */
    s32 (*contact)(void *, void *); /* Optional collision callback. */
} M646CollisionHeader;

typedef struct M646SurfaceCollider_s {
    u32 kind; /* Collision participation and response flags. */
    u32 group; /* Group mask used to select projectile contacts. */
    u32 callbackType; /* Contact category passed to the projectile callback. */
    s32 playerIndex; /* Associated player index, when the surface has one. */
    s32 callbackMode; /* 10 selects the stage-edge collision test. */
    void *owner; /* Work record passed to the optional update callback. */
    void (*update)(void *); /* Runs after the manager's contact pass. */
    s32 (*contact)(void *, void *); /* Optional response; nonzero suppresses reflection. */
    s32 edgeCount; /* Number of collision edges. */
    M646WorldEdge *current; /* Active world-space edge buffer. */
    M646WorldEdge *alternate; /* Spare buffer swapped during transform updates. */
    M646MeshEdge *original; /* Source edge endpoints in local coordinates. */
    s32 transformCount; /* Number of transforms used by this collider. */
    HSF_TRANSFORM **transforms; /* Source model transforms. */
    Mtx *matrices; /* Matrices applied to source edge endpoints. */
    M646WorldEdge *contactEdge; /* Edge active during the contact callback. */
} M646SurfaceCollider;

typedef struct M646ModelAttachment_s M646ModelAttachment;
typedef struct M646ModelCollider_s {
    u32 kind; /* Registration and active-collision flags. */
    s32 group; /* Collision group bits. */
    s32 callbackType; /* Target type used for score and sound reactions. */
    s32 unusedPlayerIndex; /* Player-index slot unused by target attachments. */
    s32 callbackMode; /* Manager mode for model attachment collision checks. */
    M646ModelAttachment *owner; /* Attachment work that owns this collider. */
    void (*update)(M646ModelAttachment *); /* Unused target-update callback; setup assigns an empty
                                            * function. */
    s32 (*contact)(void *, void *); /* Optional callback for a target contact. */
    s32 edgeCount; /* Number of edges in this collider. */
    M646WorldEdge *current; /* Active world-space edge buffer. */
    M646WorldEdge *alternate; /* Spare edge buffer. */
    s32 transformCount; /* Number of transforms applied to the collider. */
    HSF_TRANSFORM pose; /* Joint position and scale used by this attachment. */
    Mtx *matrices; /* Transform matrices for collider geometry. */
    M646WorldEdge *contactEdge; /* Edge supplied to the target's contact callback. */
    s16 category; /* Selects the category-specific contact distance. */
    HU3D_MODELID model; /* Model that supplies this attachment's joint position. */
    char jointName[64]; /* Joint used to follow the target model. */
    HSF_OBJECT *source; /* HSF object used to initialize this collider. */
    s32 assigned; /* Nonzero while a computer player's target selection has reserved this
                   * attachment. */
} M646ModelCollider;
extern f32 lbl_1_data_108[5];

extern OMOBJ *lbl_1_bss_60;

void fn_1_2BDC(OMOBJ *obj);

f32 fn_1_2594(const Vec *lhs, const Vec *rhs);
f32 fn_1_25C4(Vec *origin, Vec *direction, Vec *point);
void fn_1_263C(Vec *origin, Vec *endpoint, Vec *normal, Vec *planePoint, Vec *intersection);
void fn_1_27C4(Vec lhs, Vec rhs, Vec *result);
f64 fn_1_27E8(Vec point);
f64 fn_1_2968(Vec lhs, Vec rhs);
void fn_1_2B50(OMOBJMAN *objMan);
void fn_1_2BD8(void);
void fn_1_2BDC(OMOBJ *obj);
void fn_1_2BEC(OMOBJ *obj);
s32 fn_1_2EE0(M646BodyAnchor *first, M646BodyAnchor *second);
s32 fn_1_3350(M646BodyAnchor *first, M646SurfaceCollider *second);
s32 fn_1_3894(M646BodyAnchor *first, M646ModelCollider *second);
s32 fn_1_3E80(void *collisionResource);
s32 fn_1_3EF8(void *collisionResource);
void fn_1_3F70(s32 index);
void *fn_1_3FD4(s32 index);
void fn_1_4044(M646CollisionRecord *record, HSF_OBJECT *objects, s32 count);
void fn_1_4314(M646CollisionRecord *record, HU3D_MODELID model);
void fn_1_436C(M646CollisionRecord *record, HSF_OBJECT *object);
void fn_1_43A0(M646CollisionRecord *record);
s32 fn_1_4804(M646CollisionRecord *record, Vec *point);
s32 fn_1_48A4(const M646CollisionRecord *anchor, const Point3d *position);
s32 fn_1_48DC(M646ModelCollider *collider, HSF_OBJECT *objects, HU3D_MODELID model, char *name);
void fn_1_4DD0(M646ModelCollider *collider);

f32 fn_1_2594(const Vec *lhs, const Vec *rhs);
f32 fn_1_25C4(Vec *origin, Vec *direction, Vec *point);
void fn_1_263C(Vec *origin, Vec *endpoint, Vec *normal, Vec *planePoint, Vec *intersection);
void fn_1_27C4(Vec lhs, Vec rhs, Vec *result);
f64 fn_1_27E8(Vec point);
f64 fn_1_2968(Vec lhs, Vec rhs);
void fn_1_2B50(OMOBJMAN *objMan);
void fn_1_2BD8(void);
void fn_1_2BDC(OMOBJ *obj);
void fn_1_2BEC(OMOBJ *obj);
s32 fn_1_2EE0(M646BodyAnchor *first, M646BodyAnchor *second);
s32 fn_1_3350(M646BodyAnchor *first, M646SurfaceCollider *second);
s32 fn_1_3894(M646BodyAnchor *first, M646ModelCollider *second);
s32 fn_1_3E80(void *collisionResource);
s32 fn_1_3EF8(void *collisionResource);
void fn_1_3F70(s32 index);
void *fn_1_3FD4(s32 index);
void fn_1_4044(M646CollisionRecord *record, HSF_OBJECT *objects, s32 count);
void fn_1_4314(M646CollisionRecord *record, HU3D_MODELID model);
void fn_1_436C(M646CollisionRecord *record, HSF_OBJECT *object);
void fn_1_43A0(M646CollisionRecord *record);
s32 fn_1_4804(M646CollisionRecord *record, Vec *point);
s32 fn_1_48A4(const M646CollisionRecord *anchor, const Point3d *position);
s32 fn_1_48DC(M646ModelCollider *collider, HSF_OBJECT *objects, HU3D_MODELID model, char *name);
void fn_1_4DD0(M646ModelCollider *collider);

OMOBJ *lbl_1_bss_60;

/* X/Y target contact radii, in world units, indexed by attachment category. */
f32 lbl_1_data_108[5] = {100.0f, 75.0f, 55.0f, 35.0f, 100.0f};

/* Mesh-edge extraction walks each quad in this corner order. */
s32 lbl_1_data_11C[4] = {0, 1, 3, 2};
/* Alternate quad-corner order with a trailing zero entry. */
u8 lbl_1_data_12C[20] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 3, 0, 0, 0, 2, 0, 0, 0, 0};

/* Returns the 3D dot product used by the segment and plane calculations below. */
f32 fn_1_2594(const Vec *lhs, const Vec *rhs)
{
    return (lhs->z * rhs->z) + ((lhs->x * rhs->x) + (lhs->y * rhs->y));
}

/* The line/plane helper uses this signed origin-minus-point projection; direction is normalized in
 * place. */
f32 fn_1_25C4(Vec *origin, Vec *direction, Vec *point)
{
    Vec difference;
    PSVECNormalize(direction, direction);
    PSVECSubtract(origin, point, &difference);
    return (direction->z * difference.z) +
           ((direction->x * difference.x) + (direction->y * difference.y));
}

/* Line/plane utility offsets origin along the endpoint direction using origin-minus-planePoint.
 * It neither clamps to the segment nor guards a zero direction projection. */
void fn_1_263C(Vec *origin, Vec *endpoint, Vec *normal, Vec *planePoint, Vec *intersection)
{
    f64 planeDistance;
    f64 directionDistance;
    Vec difference;
    PSVECNormalize(normal, normal);
    planeDistance = fn_1_25C4(origin, normal, planePoint);
    PSVECSubtract(endpoint, origin, &difference);
    directionDistance = fn_1_2594(normal, &difference);
    intersection->x = origin->x + (difference.x * planeDistance / directionDistance);
    intersection->y = origin->y + (difference.y * planeDistance / directionDistance);
    intersection->z = origin->z + (difference.z * planeDistance / directionDistance);
}

/* Target range checks subtract X and Y here; the destination's Z component is left untouched. */
void fn_1_27C4(Vec lhs, Vec rhs, Vec *result)
{
    result->x = lhs.x - rhs.x;
    result->y = lhs.y - rhs.y;
}

/* The target-distance helper measures the magnitude of an X/Y displacement in world units. */
f64 fn_1_27E8(Vec point)
{
    return sqrt((point.x * point.x) + (point.y * point.y));
}

/* Target selection and model-contact checks use this X/Y distance, ignoring depth. */
f64 fn_1_2968(Vec lhs, Vec rhs)
{
    Vec difference;
    fn_1_27C4(lhs, rhs, &difference);
    return fn_1_27E8(difference);
}

/* Creates the collision manager and its model-contact slots during sequence setup. */
void fn_1_2B50(OMOBJMAN *objMan)
{
    OMOBJ *obj = omAddObjEx(objMan, M646_COLLISION_PRIORITY, 0, 0, OM_GRP_NONE, fn_1_2BDC);
    void *data;
    lbl_1_bss_60 = obj;
    data = HuMemDirectMallocNum(HEAP_HEAP, M646_COLLISION_SLOT_BYTES, HU_MEMNUM_OVL);
    obj->data = data;
    memset(data, 0, M646_COLLISION_SLOT_BYTES);
}

/* Empty collision utility entry point. */
void fn_1_2BD8(void)
{
}

/* The manager startup callback installs the per-frame collision check. */
void fn_1_2BDC(OMOBJ *obj)
{
    obj->objFunc = fn_1_2BEC;
}

/* Checks registered actors against the current stage surfaces and model attachments each frame. */
void fn_1_2BEC(OMOBJ *obj)
{
    void **slots = obj->data;
    void *firstRecord;
    void *secondRecord;
    M646CollisionHeader *first;
    M646CollisionHeader *second;
    s32 i;
    s32 j;
    for (i = 0; i < 30; i++) {
        firstRecord = slots[i];
        if (firstRecord == NULL) {
            continue;
        }
        first = firstRecord;
        if (!(first->kind & 0x1) || first->callbackMode != 0) {
            continue;
        }
        for (j = i + 1; j < 30; j++) {
            secondRecord = slots[j];
            if (secondRecord == NULL) {
                continue;
            }
            second = secondRecord;
            if (!(second->kind & 0x1) || second->callbackMode != 0) {
                continue;
            }
            /* The actor-pair scan discards this mask comparison and performs no contact test. */
            (first->group & second->group) == 0;
        }
        if (first->kind & 0x2) {
            continue;
        }
        for (j = 0; j < 130; j++) {
            secondRecord = (slots + 30)[j];
            if (secondRecord == NULL) {
                continue;
            }
            second = secondRecord;
            if (!(second->kind & 0x1) || !(first->kind & 0x1) ||
                !(first->group & second->group)) {
                continue;
            }
            switch (second->callbackMode) {
                case 10:
                    fn_1_3350(firstRecord, secondRecord);
                    break;
                case 11:
                    fn_1_3894(firstRecord, secondRecord);
                    break;
            }
        }
    }
    for (i = 0; i < 30; i++) {
        firstRecord = slots[i];
        if (firstRecord == NULL) {
            continue;
        }
        first = firstRecord;
        if (first->callbackMode == 0) {
            M646BodyAnchor *body = firstRecord;
            if (isnan(body->position->x) || isnan(body->position->z)) {
                *body->position = body->savedPosition;
            }
        }
        if (first->update != NULL) {
            first->update(first->owner);
        }
    }
}

/* Actor-contact utility separates overlapping X/Z circles and adjusts their movement before
 * callbacks. */
s32 fn_1_2EE0(M646BodyAnchor *first, M646BodyAnchor *second)
{
    Vec *firstPosition = first->position;
    Vec *secondPosition = second->position;
    f32 dx = firstPosition->x - secondPosition->x;
    f32 dz = firstPosition->z - secondPosition->z;
    f32 distance;
    f32 scale;
    f32 radius;
    Vec midpoint;
    distance = sqrtf(dx * dx + dz * dz);
    radius = first->radius + second->radius;
    if (radius > distance) {
        if (0.0 == distance) {
            dx = (0.0f);
            dz = (-1.0f);
        } else {
            dx /= distance;
            dz /= distance;
        }
        midpoint.x = firstPosition->x + 0.5 * (secondPosition->x - firstPosition->x);
        midpoint.z = firstPosition->z + 0.5 * (secondPosition->z - firstPosition->z);
        distance = 0.5 * radius;
        firstPosition->x = midpoint.x + dx * distance;
        firstPosition->z = midpoint.z + dz * distance;
        secondPosition->x = midpoint.x - dx * distance;
        secondPosition->z = midpoint.z - dz * distance;
        /* These paired callback types still separate, but do not push each other. */
        if (!(first->callbackType == 0 && second->callbackType == 1 && (first->kind & 0x10)) &&
            !(second->callbackType == 0 && first->callbackType == 1 && (second->kind & 0x10))) {
            scale = (0.10000000149011612f);
            if ((second->kind & 0x4) && !(first->kind & 0x8)) {
                scale = (0.5f);
            }
            first->direction->x += first->responseScale * (scale * (dx * distance));
            first->direction->z += first->responseScale * (scale * (dz * distance));
            scale = (0.10000000149011612f);
            if ((first->kind & 0x4) && !(second->kind & 0x8)) {
                scale = (0.5f);
            }
            second->direction->x -= second->responseScale * (scale * (dx * distance));
            second->direction->z -= second->responseScale * (scale * (dz * distance));
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

/* The frame collision pass separates actors from stage edges in X/Z and reflects their movement.
 * A nonzero surface callback suppresses reflection while preserving the contact result. */
s32 fn_1_3350(M646BodyAnchor *first, M646SurfaceCollider *second)
{
    s32 result = 0;
    M646WorldEdge *edge = second->current;
    Vec *previous = &first->savedPosition;
    Vec *position = first->position;
    f32 projectedDistance;
    f32 currentDistance;
    f32 previousDistance;
    f32 radius = first->radius;
    f32 speed;
    f32 dx;
    f32 dz;
    f32 scale;
    f32 endX;
    f32 endZ;
    Vec intersection;
    s32 i;
    for (i = 0; i < second->edgeCount; i++, edge++) {
        previousDistance =
            edge->planeConstant + (edge->normal.x * previous->x + edge->normal.z * previous->z);
        if (previousDistance < (0.0f)) {
            continue;
        }
        dx = position->x - edge->normal.x * radius;
        dz = position->z - edge->normal.z * radius;
        projectedDistance = edge->planeConstant + (edge->normal.x * dx + edge->normal.z * dz);
        currentDistance =
            edge->planeConstant + (edge->normal.x * position->x + edge->normal.z * position->z);
        if (projectedDistance >= (0.0f) && currentDistance >= (0.0f)) {
            continue;
        }
        currentDistance = -currentDistance;
        projectedDistance = previousDistance + currentDistance;
        if (0.0 == projectedDistance) {
            continue;
        }
        intersection.x =
            position->x + currentDistance * (previous->x - position->x) / projectedDistance;
        intersection.z =
            position->z + currentDistance * (previous->z - position->z) / projectedDistance;
        dx = intersection.x - edge->first.x;
        dz = intersection.z - edge->first.z;
        endX = intersection.x - edge->second.x;
        endZ = intersection.z - edge->second.z;
        if (dx * endX + dz * endZ > (0.0f)) {
            continue;
        }
        intersection.x = position->x + (100.0) * (3.0 * edge->normal.x);
        intersection.z = position->z + (100.0) * (3.0 * edge->normal.z);
        previousDistance = edge->planeConstant +
                           (edge->normal.x * intersection.x + edge->normal.z * intersection.z);
        projectedDistance = previousDistance + currentDistance;
        if (0.0 == projectedDistance) {
            continue;
        }
        position->x += currentDistance * (intersection.x - position->x) / projectedDistance +
                       edge->normal.x * radius;
        position->z += currentDistance * (intersection.z - position->z) / projectedDistance +
                       edge->normal.z * radius;
        result = 1;
        if (first->contact != NULL) {
            first->contact(first, second);
        }
        if (second->contact != NULL) {
            second->contactEdge = edge;
            if (second->contact(second, first) != 0) {
                continue;
            }
        }
        speed = PSVECMag(first->direction);
        if (0.0 == speed) {
            continue;
        }
        if (speed > (100.00000521540642)) {
            speed = (100.00000762939453f);
        }
        C_VECReflect(first->direction, &edge->normal, &intersection);
        if (isnan(intersection.x) || isnan(intersection.z)) {
            continue;
        }
        scale = (0.8) * speed;
        first->direction->x = intersection.x * scale;
        first->direction->z = intersection.z * scale;
    }
    return result;
}

/* The frame collision pass tests a model target's depth and X/Y radius using its first edge.
 * A handled target callback or zero speed returns 0 even after contact callbacks have run. */
s32 fn_1_3894(M646BodyAnchor *first, M646ModelCollider *second)
{
    s32 result = 0;
    f32 speed;
    f32 radius;
    f32 scale;
    f64 distance = 0.0;
    M646WorldEdge *edge = second->current;
    Vec *previous = &first->savedPosition;
    Vec *position = first->position;
    f32 previousDistance;
    f32 projectedX;
    f32 projectedY;
    f32 projectedZ;
    f32 zDistance;
    f32 threshold;
    Vec reflected;
    Vec colliderPosition;
    radius = first->radius;
    if (!(second->kind & 0x1)) {
        return 0;
    }
    previousDistance = edge->planeConstant + (edge->normal.z * previous->z +
                       (edge->normal.x * previous->x + edge->normal.y * previous->y));
    if (previousDistance < (0.0f)) {
        return 0;
    }
    /* Only the projected depth is used; range is measured from the unprojected X/Y position. */
    projectedX = position->x - edge->normal.x * radius;
    projectedY = position->y - edge->normal.y * radius;
    projectedZ = position->z - edge->normal.z * radius;
    HuCopyVecF(&colliderPosition, &second->pose.pos);
    zDistance = projectedZ - colliderPosition.z;
    if (zDistance < (0.0f)) {
        return 0;
    }
    threshold = lbl_1_data_108[second->category];
    distance = fn_1_2968(*position, colliderPosition);
    if (distance > threshold) {
        return 0;
    }
    result = 1;
    if (first->contact != NULL) {
        first->contact(first, second);
    }
    if (second->contact != NULL) {
        second->contactEdge = edge;
        if (second->contact(second, first) != 0) {
            return 0;
        }
    }
    speed = PSVECMag(first->direction);
    if (0.0 == speed) {
        return 0;
    }
    if (speed > (100.00000521540642)) {
        speed = (100.00000762939453f);
    }
    C_VECReflect(first->direction, &edge->normal, &reflected);
    if (!isnan(reflected.x) && !isnan(reflected.z)) {
        scale = (0.8) * speed;
        first->direction->x = reflected.x * scale;
        first->direction->z = reflected.z * scale;
    }
    return result;
}

/* Actor setup registers collision work in the first free body slot, returning -1 if all 30 are
 * full. */
s32 fn_1_3E80(void *collisionResource)
{
    OMOBJ *object = lbl_1_bss_60;
    void *data = object->data;
    void **entry = (void **)data;
    s32 index;

    for (index = 0; index < 30; index++, entry++) {
        if (*entry == NULL) {
            *entry = collisionResource;
            return index;
        }
    }
    return -1;
}

/* Stage setup registers a surface or target in the 130 attachment slots, returning its combined
 * slot. */
s32 fn_1_3EF8(void *collisionResource)
{
    OMOBJ *object = lbl_1_bss_60;
    void *data = object->data;
    void **entry = (void **)data + 30;
    s32 index;

    for (index = 0; index < 130; index++, entry++) {
        if (*entry == NULL) {
            *entry = collisionResource;
            return index + 30;
        }
    }
    return -1;
}

/* Collision-resource release clears its manager slot; body slots are harmlessly cleared twice. */
void fn_1_3F70(s32 index)
{
    OMOBJ *object = lbl_1_bss_60;
    void **slots = (void **)object->data;

    if (index < 0) {
        return;
    }
    if (index < 30) {
        slots[index] = NULL;
    }
    index -= 30;
    if (index < 130) {
        (slots + 30)[index] = NULL;
    }
}

/* Returns a registered collision resource by slot during contact checks. */
void *fn_1_3FD4(s32 index)
{
    OMOBJ *object = lbl_1_bss_60;
    void **slots = (void **)object->data;

    if (index < 0) {
        return NULL;
    }
    if (index < 30) {
        return slots[index];
    }
    index -= 30;
    if (index < 130) {
        return (slots + 30)[index];
    }
    return NULL;
}

/* Model and stage setup allocate edge buffers, keeping only the first edge below Y=50 in each
 * quad. */
void fn_1_4044(M646CollisionRecord *record, HSF_OBJECT *objects, s32 count)
{
    HSF_OBJECT *object;
    s32 length;
    M646MeshEdge *edge;
    HSF_FACE *face;
    Vec *first;
    Vec *second;
    HSF_TRANSFORM **transforms;
    s32 corner;
    s32 objectsRemaining;
    Mtx *matrix;
    s32 quadCount = 0;
    Vec *vertices;
    s32 nextCorner;

    object = objects;
    objectsRemaining = count;
    while (objectsRemaining--) {
        if (object->type == HSF_OBJ_MESH) {
            face = object->mesh.face->data;
            length = object->mesh.face->count;
            while (length--) {
                if (face->type == HSF_FACE_QUAD) {
                    quadCount++;
                }
                face++;
            }
        }
        object++;
    }

    length = quadCount * sizeof(M646MeshEdge);
    edge = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    record->original = edge;
    memset(edge, 0, length);
    length = quadCount * sizeof(M646WorldEdge);
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
        if (object->type == HSF_OBJ_MESH) {
            vertices = object->mesh.vertex->data;
            face = object->mesh.face->data;
            length = object->mesh.face->count;
            while (length--) {
                if (face->type == HSF_FACE_QUAD) {
                    for (corner = 0; corner < 4; corner++) {
                        nextCorner = (corner + 1) & 0x3;
                        first = &vertices[face->index[lbl_1_data_11C[corner]].vertex];
                        second = &vertices[face->index[lbl_1_data_11C[nextCorner]].vertex];
                        if ((first->y < (50.0)) && (second->y < (50.0))) {
                            HuCopyVecF(&edge->first, first);
                            HuCopyVecF(&edge->second, second);
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

/* Starts the stage edge build from every HSF object in a loaded model. */
void fn_1_4314(M646CollisionRecord *record, HU3D_MODELID model)
{
    HSF_DATA *hsf = Hu3DData[model].hsf;
    fn_1_4044(record, hsf->object, hsf->objectNum);
}

/* Attachment setup calls this for one world mesh to build its collision edges. */
void fn_1_436C(M646CollisionRecord *record, HSF_OBJECT *object)
{
    fn_1_4044(record, object, 1);
}

/* Stage setup swaps edge buffers and transforms mesh edges into world-space collision planes. */
void fn_1_43A0(M646CollisionRecord *record)
{
    M646WorldEdge *edge;
    HSF_TRANSFORM **transforms;
    M646MeshEdge *original;
    s32 i;
    Mtx *matrix;
    Mtx temporary;
    Mtx rotation;
    Mtx transform;
    f64 zDifference;
    f64 xDifference;
    f64 yDifference;
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
        yDifference = edge->second.y - edge->first.y;
        xDifference = edge->second.x - edge->first.x;
        magnitude = sqrt(xDifference * xDifference +
                         (zDifference * zDifference + yDifference * yDifference));
        if (0.0 != magnitude) {
            zDifference /= magnitude;
            yDifference /= magnitude;
            xDifference /= magnitude;
        }
        edge->normal.x = -zDifference;
        edge->normal.y = yDifference;
        edge->normal.z = xDifference;
        edge->planeConstant = -(xDifference * edge->first.z +
                                 (-zDifference * edge->first.x + yDifference * edge->first.y));
    }
}

/* Point-containment utility returns false when any current edge plane places the point outside. */
s32 fn_1_4804(M646CollisionRecord *record, Vec *point)
{
    M646WorldEdge *edge = record->current;
    s32 i;
    f32 planeDistance;

    for (i = 0; i < record->edgeCount; i++, edge++) {
        planeDistance = edge->planeConstant +
            (edge->normal.z * point->z +
             (edge->normal.x * point->x + edge->normal.y * point->y));
        if (planeDistance < (0.0f)) {
            return 0;
        }
    }
    return 1;
}

/* Projectile frames compare only the first sampled stage depth; the supplied collider's edges are
 * unused. */
s32 fn_1_48A4(const M646CollisionRecord *anchor, const Point3d *position)
{
    M646WorldEdge *edge = anchor->current;
    if (position->z < lbl_1_bss_84.points[0].z) {
        return 0;
    }
    return 1;
}

/* Target setup records its model joint and registers empty collision buffers.
 * It derives the first plane from zeroed endpoints without extracting mesh edges or returning a
 * value. */
s32 fn_1_48DC(M646ModelCollider *collider, HSF_OBJECT *objects, HU3D_MODELID model, char *name)
{
    HSF_OBJECT *object;
    s32 length;
    M646WorldEdge *edge;
    HSF_FACE *face;
    s32 objectsRemaining;
    s32 quadCount = 0;
    Vec position;
    HSF_TRANSFORM transform;
    Mtx *matrix;
    f64 zDifference;
    f64 xDifference;
    f64 yDifference;
    f64 magnitude;
    collider->model = model;
    memset(collider->jointName, 0, 64);
    strcpy(collider->jointName, name);
    object = objects;
    collider->source = object;
    objectsRemaining = 1;
    while (objectsRemaining--) {
        if (object->type == HSF_OBJ_MESH) {
            face = object->mesh.face->data;
            length = object->mesh.face->count;
            while (length--) {
                if (face->type == HSF_FACE_QUAD) {
                    quadCount++;
                }
                face++;
            }
        }
        object++;
    }
    length = quadCount * sizeof(M646WorldEdge);
    collider->current = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    memset(collider->current, 0, length);
    collider->alternate = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
    memset(collider->alternate, 0, length);
    collider->transformCount = 1;
    length = collider->transformCount * sizeof(Mtx);
    {
        void *matrixBuffer;
        collider->matrices = matrixBuffer = HuMemDirectMallocNum(HEAP_HEAP, length, HU_MEMNUM_OVL);
        memset(matrixBuffer, 0, length);
    }
    Hu3DModelObjPosGet(model, name, &position);
    HuCopyVecF(&collider->pose.pos, &position);
    collider->pose.scale.x = collider->pose.scale.y = collider->pose.scale.z = 1.0f;
    transform = collider->pose;
    matrix = collider->matrices;
    edge = collider->alternate;
    collider->alternate = collider->current;
    collider->current = edge;
    zDifference = edge->second.z - edge->first.z;
    yDifference = edge->second.y - edge->first.y;
    xDifference = edge->second.x - edge->first.x;
    magnitude =
        sqrt(xDifference * xDifference + (zDifference * zDifference + yDifference * yDifference));
    if (0.0 != magnitude) {
        zDifference /= magnitude;
        yDifference /= magnitude;
        xDifference /= magnitude;
    }
    edge->normal.x = -zDifference;
    edge->normal.y = yDifference;
    edge->normal.z = xDifference;
    edge->planeConstant = -(xDifference * edge->first.z +
                            (-zDifference * edge->first.x + yDifference * edge->first.y));
    fn_1_3EF8((void *)collider);
}

/* Each moving-stage update refreshes a target collider's position from its named model joint. */
void fn_1_4DD0(M646ModelCollider *collider)
{
    Point3d position;
    M646ModelAttachment *owner;
    owner = collider->owner;
    Hu3DModelObjPosGet(collider->model, collider->jointName, &position);
    collider->pose.pos.x = position.x;
    collider->pose.pos.y = position.y;
    collider->pose.pos.z = position.z;
}
