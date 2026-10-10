/* Talkie Walkie particle, model, track, material, and spatial helpers. */
#include "dolphin/math.h"
#include "REL/m667dll/recovered.h"
#include "dolphin/mtx.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "msm.h"

void Hu3DMtxTransGet(Mtx src, HuVecF *dst);
void Hu3DMtxRotGet(Mtx src, HuVecF *dst);
void Hu3DMtxScaleGet(Mtx src, HuVecF *dst);

typedef struct M667ParticleConfigView {
    u32 flags;                         /* Bit 0 alternates scale between half and full size. */
    GXColor targetColor;               /* End color used by the particle's per-update color
                                        * blend. */
    GXColor startingColor;              /* Start color used by the particle's per-update color
                                         * blend. */
    HuVecF initialVelocity;            /* Initial displacement per particle update. */
    f32 velocityMultiplier[3];         /* Per-frame X, Y, and Z velocity multipliers. */
    f32 velocityYDelta;                /* Per-frame change to vertical velocity. */
    u8 reservedBytes[4];               /* Reserved bytes between velocity and scale settings. */
    f32 scaleRate;                     /* Per-frame change to base scale. */
    f32 alphaRate;                     /* Per-frame change to alpha. */
    f32 colorInterpolation;            /* Blend fraction from startingColor to targetColor. */
} M667ParticleConfigView;

#define PARTICLE_SCALE_ALTERNATE_FLAG (1U << 0)
#define PARTICLE_DESCRIPTOR_ANIMATION_READY_FLAG (1U << 0)
#define PARTICLE_DESCRIPTOR_WORK_READY_FLAG (1U << 1)

extern f32 fn_1_7EE0(f32, f32, f32);

/* The particle draw hook advances live slots' motion, color, and scale before rendering. */
void fn_1_6C88(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *data;
    s16 slotIndex;
    const M667ParticleConfigView *config;

    config = (const M667ParticleConfigView *)particle->work;
    /* A zero system count clears every slot's scale before any live-particle updates. */
    if (particle->count == 0) {
        data = particle->data;
        for (slotIndex = 0; slotIndex < particle->maxCnt; (void)(slotIndex++, data++)) {
            data->scale = 0.0f;
        }
    }

    data = particle->data;
    for (slotIndex = 0; slotIndex < particle->maxCnt; (void)(slotIndex++, data++)) {
        if (data->scale && data->parManId == -1) {
            data->vel.x *= config[slotIndex].velocityMultiplier[0];
            data->vel.y *= config[slotIndex].velocityMultiplier[1];
            data->vel.z *= config[slotIndex].velocityMultiplier[2];
            PSVECAdd(&data->vel, &data->pos, &data->pos);
            data->vel.y += config[slotIndex].velocityYDelta;
            data->color.r = (u8) fn_1_7EE0((f32) config[slotIndex].startingColor.r,
                                           (f32) config[slotIndex].targetColor.r,
                                           config[slotIndex].colorInterpolation);
            data->color.g = (u8) fn_1_7EE0((f32) config[slotIndex].startingColor.g,
                                           (f32) config[slotIndex].targetColor.g,
                                           config[slotIndex].colorInterpolation);
            data->color.b = (u8) fn_1_7EE0((f32) config[slotIndex].startingColor.b,
                                           (f32) config[slotIndex].targetColor.b,
                                           config[slotIndex].colorInterpolation);

            {
            f32 alpha = (f32)data->color.a;
            f32 alphaDelta = config[slotIndex].alphaRate;
            alpha = (f32)data->color.a + config[slotIndex].alphaRate;
            if (alpha <= 0.0f) {
                data->scale = 0.0f;
            }
            /* Alpha is converted without clamping even after a nonpositive value stops the
             * particle. */
            data->color.a = (u8)alpha;
            }

            if (data->scale) {
                if ((config[slotIndex].flags & PARTICLE_SCALE_ALTERNATE_FLAG) != 0) {
                f64 scaleFactor;
                    if (((data->time + slotIndex) & 1) != 0) {
                        scaleFactor = 1.0;
                    } else {
                        scaleFactor = 0.5;
                    }
                    data->scale = (f32)((f64)data->scaleBase * scaleFactor);
                } else {
                    data->scale = data->scaleBase;
                }
                data->scaleBase += config[slotIndex].scaleRate;
                if (data->scaleBase <= 0.01f) {
                    data->scale = 0.0f;
                }
            }
            data->time++;
        }
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(*particle->data));
}

#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/gx/GXVert.h"
#include "dolphin/mtx.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamemes.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/memory.h"
#include "game/mg/actman.h"
#include "game/mg/seqman.h"
#include "game/mic.h"
#include "game/object.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "humath.h"
#include "string.h"

/* Particle emission requests fill a free slot from the last emission index, wrapping once.
 * Return the chosen slot index, or -1 if the model or a free slot is unavailable. */
s16 fn_1_7104(OMOBJ *poolItem, s16 cameraMask,
              f32 positionX, f32 positionY, f32 positionZ,
              f32 initialScale, const M667ParticleConfigView *config) {
    s16 result;
    s16 modelIndex;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *particleData;
    M667ParticleConfigView *configWork;
    s16 slot;

    result = -1;
    modelIndex = (s16) poolItem->mode;
    if (modelIndex == -1) {
        return -1;
    }

    model = &Hu3DData[modelIndex];
    particle = (HU3D_PARTICLE *) model->hookData;
    configWork = (M667ParticleConfigView *) particle->work;
    particleData = particle->data + particle->emitCnt;
    slot = particle->emitCnt;
    while (slot < particle->maxCnt) {
        if (!particleData->scale) {
            break;
        }
        (void)(slot++, particleData++);
    }

    if (slot >= particle->maxCnt) {
        particleData = particle->data;
        slot = 0;
        while (slot < particle->maxCnt) {
            if (!particleData->scale) {
                break;
            }
            (void)(slot++, particleData++);
        }
    }

    if (slot != particle->maxCnt) {
        configWork[slot] = *config;
        particleData->cameraBit = cameraMask;
        particleData->pos.x = positionX;
        particleData->pos.y = positionY;
        particleData->pos.z = positionZ;
        particleData->vel = config->initialVelocity;
        /* Emission starts with targetColor; the next particle update applies the configured
         * blend. */
        particleData->color = config->targetColor;
        particleData->scaleBase = initialScale;
        particleData->scale = initialScale;
        particleData->time = 0;
        particleData->parManId = -1;
        particle->emitCnt = slot;
        result = slot;
    } else {
        result = -1;
    }

    /* The model is raised to layer 1 even when no particle slot was available. */
    Hu3DModelLayerSet(modelIndex, 1);
    return result;
}

/* Descriptor-object creation counts model and motion slots before allocating the object. */
void fn_1_7350(void *description, s32 *modelSlotCount, s32 *motionSlotCount)
{
    M667ModelDescription *entry;
    s32 modelCount;
    s32 motionCount;
    s32 index;
    s32 descriptorScratch;

    modelCount = 0;
    motionCount = 0;
    for (index = 0;; index++) {
        entry = &((M667ModelDescription *)description)[index];
        descriptorScratch = 0;
        if (entry->kind == 8) {
            break;
        }
        switch (entry->kind) {
        case 5:
            break;
        case 0:
        case 2:
            if (entry->count > 0) {
                modelCount += entry->count;
            } else {
                modelCount++;
            }
            break;
        case 3:
            modelCount++;
            motionCount++;
            break;
        case 1:
            if (entry->modelIndex >= 0) {
                motionCount++;
            }
            break;
        }
    }
    *modelSlotCount = modelCount;
    *motionSlotCount = motionCount;
}

/* Descriptor-object creation loads resources into its allocated model and motion slots.
 * A failed resource read ends loading, leaving the remaining descriptors unprocessed. */
void fn_1_7424(OMOBJ *object, M667ModelDescription *description)
{
    s32 modelCount = 0;
    s32 motionCount = 0;
    s32 index = 0;
    s32 linkIndex;
    M667ModelDescription *entry;
    s32 baseModelId;
    s32 modelId;
    s32 motionId;
    void *data;

    for (;;) {
        entry = &description[index];
        data = NULL;
        if (entry->kind == 8) {
            break;
        }
        if (entry->dataId >= 0) {
            data = HuDataSelHeapReadNum(entry->dataId, HU_MEMNUM_OVL, HEAP_MODEL);
            if (data == NULL) {
                break;
            }
        }
        switch (entry->kind) {
        case 0:
        case 2:
            baseModelId = Hu3DModelCreate(data);
            modelId = baseModelId;
            object->mdlId[modelCount++] = modelId;
            if (entry->kind == 2) {
                Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
            }
            if (entry->count > 0) {
                for (linkIndex = 0; linkIndex < entry->count - 1; linkIndex++) {
                    modelId = Hu3DModelLink(baseModelId);
                    object->mdlId[modelCount++] = modelId;
                    if (entry->kind == 2) {
                        Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
                    }
                }
            }
            break;
        case 3:
            motionId = Hu3DMotionCreate(data);
            object->mtnId[motionCount++] = motionId;
            modelId = Hu3DModelCameraCreate(motionId, 1);
            object->mdlId[modelCount++] = modelId;
            Hu3DCameraMotionOff(modelId);
            break;
        case 1:
            if (entry->modelIndex >= 0) {
                modelId = object->mdlId[entry->modelIndex];
                motionId = Hu3DJointMotion(modelId, data);
                if (Hu3DData[modelId].motId < 0) {
                    Hu3DMotionSet(modelId, motionId);
                }
                if (entry->attr != 0) {
                    Hu3DModelAttrSet(modelId, entry->attr);
                }
                object->mtnId[motionCount++] = motionId;
            }
            break;
        case 5:
            break;
        }
        index++;
    }
}

void fn_1_6C88(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

/* Each particle-system descriptor records its resource, creation settings, and loaded handles. */
typedef struct M667ParticleDescription {
    u32 flags; /* Readiness bits used to skip animation and work-buffer setup. */
    s32 resourceId; /* Sprite-animation data number; a negative value ends the list. */
    s32 count; /* Maximum number of particle slots. */
    s32 blendMode; /* Blend mode passed to the particle renderer. */
    s32 modelId; /* Created particle-system model handle. */
    ANIMDATA *animation; /* Loaded animation used by the particle-system model. */
    u8 reservedAfterAnimation[4]; /* Reserved bytes after the loaded animation pointer. */
} M667ParticleDescription;

/* The particle-list initializer builds a system and per-slot settings for each descriptor. */
void fn_1_7664(void *work)
{
    s32 unusedWord = 0;
    void *resource;
    ANIMDATA *animation;
    s32 blendMode;
    s32 index;
    s32 particleIndex;
    s32 count;
    void *parameters;
    M667ParticleDescription *description;
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;

    index = 0;
    while (1) {
        description = &((M667ParticleDescription *)work)[index];
        resource = 0;
        if (description->resourceId < 0) {
            break;
        }
        /* Readiness flags skip assignments to local creation values; later creation still uses
         * those locals without loading a saved alternative. */
        if ((description->flags & PARTICLE_DESCRIPTOR_ANIMATION_READY_FLAG) == 0) {
            resource = HuDataSelHeapReadNum(description->resourceId, HU_MEMNUM_OVL, HEAP_MODEL);
            animation = (description->animation = HuSprAnimRead(resource));
            count = description->count;
            blendMode = description->blendMode;
        }
        if ((description->flags & PARTICLE_DESCRIPTOR_WORK_READY_FLAG) == 0) {
            parameters = HuMemDirectMallocNum(HEAP_HEAP, count * 56, HU_MEMNUM_OVL);
            memset(parameters, 0, count * 56);
        }
        description->modelId = Hu3DParticleCreate(animation, (s16)count);
        Hu3DParticleBlendModeSet((s16)description->modelId, (u8)blendMode);
        Hu3DParticleHookSet((s16)description->modelId, fn_1_6C88);
        particle = Hu3DData[description->modelId].hookData;
        particle->emitCnt = 0;
        particle->work = parameters;
        particle->count = 1;
        data = particle->data;
        particleIndex = 0;
        while (particleIndex < particle->maxCnt) {
            data->scale = 0.0f;
            (void)(particleIndex++, data++);
        }
        index++;
    }
}

/* The animation-list initializer loads each sprite animation up to the negative resource ID. */
void fn_1_77D0(M667ParticleAnimEntry *entries)
{
    void *resourceData;
    ANIMDATA *anim;
    s32 index;
    u32 unusedWord = 0;
    M667ParticleAnimEntry *entry;

    index = 0;
    while (1) {
        entry = &entries[index];
        resourceData = NULL;
        if (entry->resourceId < 0) {
            break;
        }
        resourceData = HuDataSelHeapReadNum(entry->resourceId, HU_MEMNUM_OVL, HEAP_MODEL);
        anim = entry->anim = HuSprAnimRead(resourceData);
        entry->anim = anim;
        index += 1;
    }
}

/* Scene setup counts descriptor slots, allocates the model object, then loads its resources. */
OMOBJ *fn_1_7868(void *description)
{
    extern void fn_1_7350(void *description, s32 *models, s32 *motions);
    extern void fn_1_7424(OMOBJ *object, M667ModelDescription *description);
    s32 modelCount;
    s32 motionCount;
    OMOBJ *object;

    fn_1_7350(description, &modelCount, &motionCount);
    object = omAddObjEx(lbl_1_bss_0, 32730, modelCount, motionCount, -1, NULL);
    object->stat |= OM_STAT_MODELPAUSE;
    fn_1_7424(object, description);
    return object;
}

/* Scene teardown releases a descriptor object's models, then its motions and object record. */
void fn_1_78F8(OMOBJ *object)
{
    s32 index;

    if (object != NULL) {
        index = object->mdlcnt - 1;
        while (index >= 0) {
            Hu3DModelKill(object->mdlId[index]);
            index--;
        }
        index = 0;
        while (index < object->mtncnt) {
            Hu3DMotionKill(object->mtnId[index]);
            index++;
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
}

/* Particle-list setup builds the described systems and returns the same list pointer. */
void *fn_1_7998(void *work)
{
    fn_1_7664(work);
    return work;
}

/* Animation-list setup loads the described animations and returns the same list pointer. */
void *fn_1_79CC(void *work)
{
    extern void fn_1_77D0(M667ParticleAnimEntry *entries);
    fn_1_77D0(work);
    return work;
}

/* This work item carries a sprite animation and a reserved leading word. */
typedef struct M667AnimView {
    u8 reservedWord[4];                /* Reserved word before the sprite animation pointer. */
    ANIMDATA *animation;               /* Sprite animation released with this work item. */
} M667AnimView;

/* Animation cleanup releases this work item's animation reference and clears its pointer. */
void fn_1_7A00(M667AnimView *work)
{
    if (work->animation) {
        HuSprAnimKill(work->animation);
        work->animation = NULL;
    }
}

/* Object visibility requests show or hide every model owned by the supplied object. */
void fn_1_7A44(OMOBJ *object, s32 visible)
{
    s32 index;

    index = 0;
    while (index < object->mdlcnt) {
        if (visible == 0) {
            Hu3DModelAttrSet(object->mdlId[index], HU3D_ATTR_DISPOFF);
        } else {
            Hu3DModelAttrReset(object->mdlId[index], HU3D_ATTR_DISPOFF);
        }
        index++;
    }
}

/* Returning a pool item patches its neighboring links before placing it on the free list. */
void fn_1_7AC8(void *item)
{
    M667PoolNode *node;

    node = item;
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
}

/* Effect-registry setup clears a fixed-size pool and links each item onto its free list. */
void *fn_1_7B10(u8 *buffer, s32 bytes, s32 itemBytes)
{
    M667Pool *pool;
    M667PoolNode *node;
    s32 stride;

    memset(buffer, 0, bytes);
    pool = (M667Pool *)buffer;
    buffer += sizeof(M667Pool);
    bytes -= sizeof(M667Pool);
    while (bytes > 0) {
        node = (M667PoolNode *)buffer;
        if (pool->free) {
            pool->free->prev = node;
        }
        node->next = pool->free;
        pool->free = node;
        stride = itemBytes + sizeof(M667PoolNode);
        buffer += stride;
        bytes -= stride;
    }
    return pool;
}

/* Effect-registry setup moves one free item to the active list and returns its payload. */
void *fn_1_7BA8(M667Pool *pool)
{
    M667PoolNode *node;
    u8 *item;

    if (!pool->free) {
        return NULL;
    }
    node = pool->free;
    /* The new free-list head keeps its prev link to the node being moved to the active list. */
    pool->free = pool->free->next;
    if (pool->active) {
        pool->active->prev = node;
    }
    node->prev = NULL;
    node->next = pool->active;
    pool->active = node;
    item = (u8 *)node;
    item += sizeof(M667PoolNode);
    return item;
}

/* Pool release requests return the supplied item to the free list. */
void fn_1_7C1C(M667Pool *pool, void *item)
{
    extern void fn_1_7AC8(void *item);
    u8 *buffer;
    M667PoolNode *node;

    buffer = item;
    buffer -= sizeof(M667PoolNode);
    node = (M667PoolNode *)buffer;
    /* Neighbor links are patched, but the pool's active head is left unchanged; the returned
     * node also keeps its previous prev pointer when it joins the free list. */
    fn_1_7AC8(node);
    if (pool->free) {
        pool->free->prev = node;
    }
    node->next = pool->free;
    pool->free = node;
}

/* Effect selection, update, drawing, and cleanup begin active-list traversal here.
 * Return the first payload, or null when the list is empty. */
void *fn_1_7C8C(void *pool)
{
    M667PoolNode *node;
    u8 *data;

    node = ((M667Pool *)pool)->active;
    data = (u8 *)node;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M667PoolNode);
}

/* Effect selection, update, drawing, and cleanup advance to the next active payload here. */
void *fn_1_7CC8(void *pool, void *item)
{
    M667PoolNode *node;
    u8 *data;

    data = item;
    data -= sizeof(M667PoolNode);
    node = (M667PoolNode *)data;
    data = (u8 *)node->next;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M667PoolNode);
}

/* Track completion queries return true only after the end frame of every occupied slot. */
s32 fn_1_7D0C(M667Track **tracks, s32 frame)
{
    s32 trackSlot;

    for (trackSlot = 0; trackSlot < 4; trackSlot++) {
        if (tracks[trackSlot] != NULL && frame <= tracks[trackSlot]->end) {
            return 0;
        }
    }
    return 1;
}

f32 fn_1_7EE0(f32 startValue, f32 endValue, f32 blend);

/* Track sampling interpolates the indexed linear track; other kinds leave output unchanged.
 * Linear tracks require an occupied slot and a frame at or after the first key; neither is
 * checked. A negative key ends the list, and later frames sample the last key's values. */
void fn_1_7D68(M667Track **tracks, s32 frame, s32 trackIndex, f32 *outValues) {
    M667Track *track;
    M667TrackKey *keys;
    M667TrackKey *lowerKey;
    M667TrackKey *upperKey;
    s32 keyIndex;
    s32 frameOffset;
    s32 frameSpan;
    s32 valueIndex;
    f32 interpolation;

    track = tracks[trackIndex];
    keys = track->keys;
    lowerKey = NULL;
    upperKey = NULL;
    for (keyIndex = 0; ; keyIndex++) {
        if (keys[keyIndex].frame < 0) {
            upperKey = lowerKey;
            break;
        }
        if (frame >= keys[keyIndex].frame) {
            lowerKey = &keys[keyIndex];
        } else {
            upperKey = &keys[keyIndex];
            break;
        }
    }
    switch (track->kind) {
    case 0:
        frameOffset = frame - lowerKey->frame;
        frameSpan = upperKey->frame - lowerKey->frame;
        if (frameSpan == 0) {
            interpolation = 0.0f;
        } else {
            interpolation = (f32) frameOffset / (f32) frameSpan;
        }
        for (valueIndex = 0; valueIndex < track->count; valueIndex++) {
            outValues[valueIndex] = fn_1_7EE0(lowerKey->values[valueIndex],
                                              upperKey->values[valueIndex], interpolation);
        }
        break;
    }
}

/* Particle colors and track values use this linear interpolation helper. */
f32 fn_1_7EE0(f32 startValue, f32 endValue, f32 blend)
{
    return startValue + (blend * (endValue - startValue));
}

/* Map effects use this to place a point between two positions during motion. */
void fn_1_7EF0(Point3d *start, Point3d *end, Point3d *out, f32 blend)
{
    out->x = start->x + (blend * (end->x - start->x));
    out->y = start->y + (blend * (end->y - start->y));
    out->z = start->z + (blend * (end->z - start->z));
}

extern f64 atan2(f64 y, f64 x);
extern f64 fmod(f64 x, f64 y);
extern f64 fn_1_6C54(f64 value);

/* Map and player movement convert an XZ direction to a yaw angle in degrees. */
f32 fn_1_7F48(HuVecF *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / 3.141592653589793;
}

/* Player rotation updates blend between wrapped yaw angles.
 * Any delta over 180 degrees in magnitude subtracts 360, even a negative delta below -180. */
f32 fn_1_7FA0(f32 start, f32 end, f32 blend)
{
    f32 startAngle;
    f32 endAngle;
    f32 delta;

    startAngle = fmod(start, 360.0);
    if (startAngle < 0.0f) {
        startAngle += 360.0f;
    }
    endAngle = fmod(end, 360.0);
    if (endAngle < 0.0f) {
        endAngle += 360.0f;
    }
    delta = endAngle - startAngle;
    if (fn_1_6C54(delta) > 180.0) {
        delta -= 360.0f;
    }
    return startAngle + delta * blend;
}

extern f32 fn_1_8E8C(f32 value);

/* Entry throw setup finds the horizontal distance to a unit quadratic's vertex.
 * The vertex height is calculated too, but only the horizontal distance is returned. */
f32 fn_1_80AC(HuVecF *start, HuVecF *end)
{
    HuVecF delta;
    f32 originDistance, originHeight, endDistance, endHeight;
    f32 peakDistance, peakHeight;
    f32 quadraticCoefficient, linearCoefficient, constantCoefficient;

    quadraticCoefficient = 1.0f;
    PSVECSubtract(end, start, &delta);
    originDistance = 0.0f;
    originHeight = 0.0f;
    endDistance = fn_1_8E8C(delta.x * delta.x + delta.z * delta.z);
    endHeight = end->y - start->y;
    linearCoefficient = (originHeight +
                         (originDistance * (-quadraticCoefficient * originDistance) +
                          endDistance * (quadraticCoefficient * endDistance)) -
                         endHeight) /
                        (originDistance - endDistance);
    constantCoefficient = originHeight - originDistance * (quadraticCoefficient * originDistance) -
                          linearCoefficient * originDistance;
    peakDistance = -(linearCoefficient / 2.0f * quadraticCoefficient);
    peakHeight = -(quadraticCoefficient * ((linearCoefficient * linearCoefficient -
                                            4.0f * quadraticCoefficient * constantCoefficient) /
                                           4.0f));
    return peakDistance;
}

extern f32 fn_1_8E8C(f32 value);

/* Entry throw setup fits negated vertical offsets against horizontal distance through the origin,
 * middle, and end points; entry movement subtracts the curve value from the starting height. */
void fn_1_8254(M667QuadraticView *out, HuVecF *origin, HuVecF *middle, HuVecF *end)
{
    HuVecF middleDelta, endDelta;
    f32 originSquaredDistance, middleSquaredDistance, endSquaredDistance, originDistanceCoefficient,
        middleDistanceCoefficient, endDistanceCoefficient, negatedOriginHeight, negatedMiddleHeight,
        negatedEndHeight;
    f32 originMiddleSquaredDifference, originMiddleDistanceDifference, originMiddleHeightDifference,
        middleEndSquaredDifference, middleEndDistanceDifference, middleEndHeightDifference;
    f32 eliminationDistanceTerm, eliminationHeightTerm;
    f32 quadraticCoefficient, linearCoefficient, constantCoefficient;
    f32 originDistance, middleDistance, endDistance, originHeight, middleHeight, endHeight;

    PSVECSubtract(middle, origin, &middleDelta);
    PSVECSubtract(end, origin, &endDelta);
    originDistance = 0.0f;
    originHeight = 0.0f;
    middleDistance = fn_1_8E8C(middleDelta.x * middleDelta.x + middleDelta.z * middleDelta.z);
    middleHeight = middleDelta.y;
    endDistance = fn_1_8E8C(endDelta.x * endDelta.x + endDelta.z * endDelta.z);
    endHeight = endDelta.y;
    /* Eliminate the constant term between adjacent points, then solve the two remaining terms. */
    originSquaredDistance = originDistance * originDistance;
    originDistanceCoefficient = originDistance;
    negatedOriginHeight = -originHeight;
    middleSquaredDistance = middleDistance * middleDistance;
    middleDistanceCoefficient = middleDistance;
    negatedMiddleHeight = -middleHeight;
    endSquaredDistance = endDistance * endDistance;
    endDistanceCoefficient = endDistance;
    negatedEndHeight = -endHeight;
    originMiddleSquaredDifference = originSquaredDistance - middleSquaredDistance;
    originMiddleDistanceDifference = originDistanceCoefficient - middleDistanceCoefficient;
    originMiddleHeightDifference = negatedOriginHeight - negatedMiddleHeight;
    middleEndSquaredDifference = middleSquaredDistance - endSquaredDistance;
    middleEndDistanceDifference = middleDistanceCoefficient - endDistanceCoefficient;
    middleEndHeightDifference = negatedMiddleHeight - negatedEndHeight;
    eliminationDistanceTerm =
        originMiddleDistanceDifference +
        originMiddleSquaredDifference * (-middleEndDistanceDifference / middleEndSquaredDifference);
    eliminationHeightTerm =
        originMiddleSquaredDifference * (middleEndHeightDifference / middleEndSquaredDifference) -
        originMiddleHeightDifference;
    linearCoefficient = -eliminationHeightTerm / eliminationDistanceTerm;
    quadraticCoefficient =
        (middleEndHeightDifference + -middleEndDistanceDifference * linearCoefficient) /
        middleEndSquaredDifference;
    constantCoefficient = negatedOriginHeight + (-originSquaredDistance * quadraticCoefficient -
                                                 originDistanceCoefficient * linearCoefficient);
    out->a = quadraticCoefficient;
    out->b = linearCoefficient;
    out->c = constantCoefficient;
}

/* Per-frame arc motion evaluates the stored height curve at its distance. */
f32 fn_1_84E8(M667QuadraticView *curve, f32 distance)
{
    f32 quadraticCoefficient, linearCoefficient, constantCoefficient;

    quadraticCoefficient = curve->a;
    linearCoefficient = curve->b;
    constantCoefficient = curve->c;
    return constantCoefficient +
           (distance * (quadraticCoefficient * distance) + linearCoefficient * distance);
}

#include "REL/m667dll/sqrtf.h"

/* Entry, player, and trail movement normalize a direction, using a random XZ fallback below
 * a squared length of 0.000001.
 * The frandmod(1) test always selects an initial Y component of -0.01 before normalization. */
s32 fn_1_8544(Point3d *source, Point3d *destination) {

    f32 fallbackY;
    s32 usedSourceVector;

    if (PSVECSquareMag(source) < (1e-06)) {
        destination->x = 0.01f * ((f32) (u32) frandmod(20) - (10.0f));
        destination->z = 0.01f * ((f32) (u32) frandmod(20) - (10.0f));
        if (frandmod(1) != 0U) {
            fallbackY = 0.01f;
        } else {
            fallbackY = (-0.01f);
        }
        destination->y = fallbackY;
        PSVECNormalize(destination, destination);
        usedSourceVector = 0;
    } else {
        PSVECNormalize(source, destination);
        usedSourceVector = 1;
    }
    return usedSourceVector;
}

/* Camera-orientation requests build axes from the first selected camera's horizontal facing
 * and up direction, without adding a translation. */
void fn_1_869C(Mtx output, s32 cameraMask)
{
    extern s32 fn_1_8544(Point3d *source, Point3d *destination);

    HuVecF position;
    HuVecF target;
    HuVecF up;
    HuVecF right;
    HuVecF upAxis;
    HuVecF forwardAxis;
    HuVecF flat;
    HuVecF direction;
    HuVecF cameraPosition;
    HuVecF cameraTarget;
    HuVecF cameraUp;

    Hu3DCameraPosGet(cameraMask, &cameraPosition, &cameraUp, &cameraTarget);
    position.x = cameraPosition.x;
    position.y = cameraPosition.y;
    position.z = cameraPosition.z;
    target.x = cameraTarget.x;
    target.y = cameraTarget.y;
    target.z = cameraTarget.z;
    up.x = cameraUp.x;
    up.y = cameraUp.y;
    up.z = cameraUp.z;

    right.x = 1.0f;
    right.y = 0.0f;
    right.z = 0.0f;
    upAxis.x = 0.0f;
    upAxis.y = 1.0f;
    upAxis.z = 0.0f;
    forwardAxis.x = 0.0f;
    forwardAxis.y = 0.0f;
    forwardAxis.z = 1.0f;

    PSVECSubtract(&target, &position, &direction);
    flat = direction;
    flat.y = 0.0f;
    /* For a nearly vertical view, use the camera's up vector as horizontal facing and
     * the negative view direction to choose the up-axis sign. */
    if (PSVECSquareMag(&flat) < (0.001)) {
        flat = direction;
        direction = up;
        PSVECScale(&flat, &up, (-1.0f));
    }
    direction.y = 0.0f;
    fn_1_8544(&direction, &direction);

    if (PSVECDotProduct(&upAxis, &up) < 0.0f) {
        upAxis.y *= (-1.0f);
        forwardAxis.z *= (-1.0f);
    }
    PSVECCrossProduct(&direction, &upAxis, &right);
    /* Copying direction overwrites the earlier forwardAxis.z sign change; the upAxis sign
     * remains. */
    forwardAxis = direction;

    output[0][0] = right.x;
    output[0][1] = right.y;
    output[0][2] = right.z;
    output[0][3] = 0.0f;
    output[1][0] = upAxis.x;
    output[1][1] = upAxis.y;
    output[1][2] = upAxis.z;
    output[1][3] = 0.0f;
    output[2][0] = forwardAxis.x;
    output[2][1] = forwardAxis.y;
    output[2][2] = forwardAxis.z;
    output[2][3] = 0.0f;
}

/* Player tile setup saves the model's material RGB bytes before tinting them. */
s8 *fn_1_895C(s32 model)
{
    s16 unusedWord = 0;
    HU3D_MODEL *modelData;
    s8 *savedColors;
    s8 *writeColor;
    s16 materialIndex;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    savedColors = HuMemDirectMallocNum(HEAP_HEAP, modelData->hsf->materialNum * 3, HU_MEMNUM_OVL);
    writeColor = savedColors;
    (void)(materialIndex = 0, material = modelData->hsf->material);
    while (materialIndex < modelData->hsf->materialNum) {
        *writeColor = (s8)material->color[0];
        writeColor++;
        *writeColor = (s8)material->color[1];
        writeColor++;
        *writeColor = (s8)material->color[2];
        writeColor++;
        (void)(materialIndex++, material++);
    }
    return savedColors;
}

/* Material-color restoration copies saved RGB bytes back into the supplied model. */
void fn_1_8A34(s32 model, s8 *colors)
{
    s16 unusedWord = 0;
    HU3D_MODEL *modelData;
    s8 *readColor;
    s16 materialIndex;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    readColor = colors;
    (void)(materialIndex = 0, material = modelData->hsf->material);
    while (materialIndex < modelData->hsf->materialNum) {
        material->color[0] = (u8)*readColor;
        readColor++;
        material->color[1] = (u8)*readColor;
        readColor++;
        material->color[2] = (u8)*readColor;
        readColor++;
        (void)(materialIndex++, material++);
    }
}

#include "dolphin/math.h"
#include "REL/m667dll/recovered.h"
#include "game/memory.h"

/* Player setup multiplies the saved signed RGB bytes by that player's tint factors.
 * Scaled channels are converted to material color bytes without clamping. */
void fn_1_8AE4(s32 model, const s8 *colors, f32 redScale, f32 greenScale, f32 blueScale)
{
    s16 unusedWord = 0;
    HU3D_MODEL *modelData;
    const s8 *savedColor;
    s16 materialIndex;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    savedColor = colors;
    (void)(materialIndex = 0, material = modelData->hsf->material);

    for (; materialIndex < modelData->hsf->materialNum; (void)(materialIndex++, material++)) {
        material->color[0] = *savedColor * redScale;
        ++savedColor;
        material->color[1] = *savedColor * greenScale;
        ++savedColor;
        material->color[2] = *savedColor * blueScale;
        ++savedColor;
    }
}

/* While an entry is held, the player update copies its character hook object's transform
 * to the entry model. */
void fn_1_8C30(s32 modelId, char *objectName, s32 targetModelId)
{
    Mtx objectMtx;
    HuVecF translation;
    HuVecF rotation;
    HuVecF scale;

    Hu3DModelObjMtxGet(modelId, objectName, objectMtx);
    Hu3DMtxTransGet(objectMtx, &translation);
    Hu3DMtxRotGet(objectMtx, &rotation);
    Hu3DMtxScaleGet(objectMtx, &scale);
    Hu3DModelPosSetV(targetModelId, &translation);
    Hu3DModelRotSetV(targetModelId, &rotation);
    Hu3DModelScaleSetV(targetModelId, &scale);
}

extern f64 fn_1_6C54(f64 value);
/* Entry landing, throwing, and impact sounds use screen X for pan and world Z for volume.
 * Within Z -700 through 700, volume falls with distance from Z 110; outside that interval
 * it jumps to 64 below -700 or 110 above 700. */

void fn_1_8CBC(HuVecF *worldPos, s32 soundId)
{
    HuVecF screenPos;
    f32 positionFactor;
    f32 depthDistance;
    s16 cameraBit;
    s32 pan;
    s32 volume;

    cameraBit = 1;
    Hu3D3Dto2D(worldPos, cameraBit, &screenPos);
    if (screenPos.x > 608.0f) {
        pan = MSM_PAN_RIGHT;
    } else if (screenPos.x < 32.0f) {
        pan = MSM_PAN_LEFT;
    } else {
        positionFactor = (screenPos.x - 32.0f) / 576.0f;
        pan = (s16)((s32)(64.0f * positionFactor) + MSM_PAN_LEFT);
    }

    if (worldPos->z > 700.0f) {
        volume = 110;
    } else if (worldPos->z < -700.0f) {
        volume = 64;
    } else {
        depthDistance = (f32)fn_1_6C54((f64)(worldPos->z - 110.0f));
        positionFactor = depthDistance / 1400.0f;
        volume = (s16)(110 - (s32)(46.0f * positionFactor));
    }
    HuAudFXPlayVolPan(soundId, volume, pan);
}

/* Scene updates request a centered sound at maximum volume and return its handle;
 * the entrance sound call discards that handle. */
s32 fn_1_8E5C(s32 soundId)
{
    return HuAudFXPlayVolPan(soundId, MSM_VOL_MAX, MSM_PAN_CENTER);
}
