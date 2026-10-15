/* Shared camera, particle, resource-pool and sound helpers for T Minus Five. */
#include "dolphin/math.h"
#include "REL/m658dll/recovered.h"
#include "humath.h"
#include "game/object.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/frand.h"
#include "game/hsfex.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include "game/flag.h"
#include "dolphin/gx/GXVert.h"
#include "string.h"
#include "datadir_enum.h"

typedef struct M658ParticleAnimEntry { s32 resourceId; ANIMDATA *anim; } M658ParticleAnimEntry;
typedef struct M658PoolNode { struct M658PoolNode *prev; struct M658PoolNode *next; } M658PoolNode;
/* List operations leave the unused portion of the cleared pool header untouched. */
typedef struct M658Pool { M658PoolNode *active; M658PoolNode *free; u8 reserved08[8]; } M658Pool;
extern HUPROCESS *lbl_1_bss_0;

typedef struct M658RegistryDescriptorView {
    void (*render)(void);
    void (*update)(void *item, s32 event);
    s32 itemBytes;
    s32 count;
} M658RegistryDescriptorView;

typedef struct M658RegistryTemplate72 {
    u32 reservedWords[16];
} M658RegistryTemplate72;

#define M658_REGISTRY_ITEM_ACTIVE_FLAG (1 << 0)
#define M658_REGISTRY_ITEM_EVENT1_FLAG (1 << 1)
#define M658_REGISTRY_ITEM_FLAGS_MASK 3
#define M658_PARTICLE_SCALE_PULSE_FLAG (1 << 0)
#define M658_PARTICLE_OWNER_REUSE_RESOURCE_FLAG (1 << 0)
#define M658_PARTICLE_OWNER_REUSE_CONFIG_FLAG (1 << 1)
#define M658_RESOURCE_HAS_LINKED_MODELS_FLAG (1 << 0)

extern const M658RegistryDescriptorView lbl_1_data_494[];
extern void *fn_1_65E8(u8 *buffer, s32 bytes, s32 itemBytes);
extern void *fn_1_6680(M658Pool *pool);
extern void fn_1_140(OMOBJ *object, OMOBJ_FUNC update);
extern void fn_1_55FC(OMOBJ *object);

typedef struct M658RegistryItemView {
    u16 flags;
    s16 age;
} M658RegistryItemView;

extern void *fn_1_6764(void *pool);
extern void *fn_1_67A0(void *pool, void *item);

/* Runs each frame to render registry entries and update pending or active items. */
void fn_1_55FC(OMOBJ *object)
{
    s32 unusedValue;
    s32 index;
    M658RegistryItemView *item;
    M658RegistryEntry *entry;
    M658RegistryWork *work;
    void *pool;

    work = object->data;
    /* This assigned value is never read. */
    unusedValue = 15;
    index = 0;
    for (;;) {
        entry = &work->entries[index];
        if (entry->update == NULL) {
            break;
        }
        if (entry->render) {
            entry->render();
        }
        pool = entry->pool;
        item = fn_1_6764(pool);
        while (item) {
            if ((item->flags & M658_REGISTRY_ITEM_EVENT1_FLAG) != 0) {
                entry->update(item, 1);
                item->flags &= ~M658_REGISTRY_ITEM_FLAGS_MASK;
            }
            if ((item->flags & M658_REGISTRY_ITEM_ACTIVE_FLAG) != 0) {
                entry->update(item, 3);
                entry->update(item, 4);
                item->age++;
            }
            item = fn_1_67A0(pool, item);
        }
        index++;
    }
}

typedef struct M658ParticleConfig {
    u32 flags;
    GXColor endColor;
    GXColor startColor;
    Vec velocity;
    Vec decay;
    f32 gravity;
    f32 scale;
    f32 scaleDelta;
    f32 alphaDelta;
    f32 colorTime;
} M658ParticleConfig;

typedef struct M658ParticleOwner {
    u32 flags;
    s32 resource;
    s32 count;
    s32 blendMode;
    s32 model;
    ANIMDATA *anim;
    u8 reserved[4]; /* Particle setup leaves these bytes unchanged. */
} M658ParticleOwner;

f32 fn_1_69B8(f32 startValue, f32 endValue, f32 interpolation);

/* Stores the requested player's coin bonus, replacing its previous value;
 * practice mode suppresses the write. */
void fn_1_571C(s32 player, s16 amount)
{
    if (_CheckFlag(FLAG_MG_PRACTICE) == 0) {
        GwPlayer[player].mgCoinBonus = amount;
    }
}

s16 fn_1_5770(void)
{
    return GwMgNightF;
}

/* Particle callback updates motion, sampled RGB, alpha and scale for each live particle. */
void fn_1_5780(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M658ParticleConfig *config = particle->work;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    f32 alpha;
    f32 unusedAlphaDelta;

    if (particle->count == 0) {
        data = particle->data;
        for (i = 0; i < particle->maxCnt; (void)(i++, data++)) {
            data->scale = 0.0f;
        }
    }
    data = particle->data;
    for (i = 0; i < particle->maxCnt; (void)(i++, data++)) {
        if (!data->scale || data->parManId != -1) {
            continue;
        }
        data->vel.x *= config[i].decay.x;
        data->vel.y *= config[i].decay.y;
        data->vel.z *= config[i].decay.z;
        VECAdd(&data->vel, &data->pos, &data->pos);
        data->vel.y += config[i].gravity;
        /* RGB samples the configured blend value; this callback does not advance colorTime. */
        data->color.r =
            fn_1_69B8(config[i].startColor.r, config[i].endColor.r, config[i].colorTime);
        data->color.g =
            fn_1_69B8(config[i].startColor.g, config[i].endColor.g, config[i].colorTime);
        data->color.b =
            fn_1_69B8(config[i].startColor.b, config[i].endColor.b, config[i].colorTime);
        /* The initial alpha copy is overwritten below before it is read. */
        alpha = data->color.a;
        /* This copied alpha delta is never read. */
        unusedAlphaDelta = config[i].alphaDelta;
        alpha = data->color.a + config[i].alphaDelta;
        if (alpha <= 0.0f) {
            data->scale = 0.0f;
        }
        data->color.a = alpha;
        if (data->scale) {
            if (config[i].flags & M658_PARTICLE_SCALE_PULSE_FLAG) {
                data->scale = data->scaleBase * (((data->time + i) & 1) ? 1.0 : 0.5);
            } else {
                data->scale = data->scaleBase;
            }
            data->scaleBase += config[i].scaleDelta;
            if (data->scaleBase <= 0.009999999776482582f) {
                data->scale = 0.0f;
            }
        }
        data->time++;
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Particle emission uses this to place an effect; it returns its slot or -1 when none is free. */
s16 fn_1_5BFC(M658ParticleOwner *owner, s16 cameraBit, f32 positionX, f32 positionY,
              f32 positionZ, f32 scale,
              M658ParticleConfig *config)
{
    s16 result = -1;
    s16 modelId = owner->model;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    M658ParticleConfig *configs;
    HU3D_PARTICLE_DATA *data;
    s16 i;

    if (modelId == -1) {
        return -1;
    }
    model = &Hu3DData[modelId];
    particle = model->hookData;
    configs = particle->work;
    data = &particle->data[particle->emitCnt];
    for (i = particle->emitCnt; i < particle->maxCnt; (void)(i++, data++)) {
        if (!data->scale) {
            break;
        }
    }
    if (i >= particle->maxCnt) {
        data = particle->data;
        for (i = 0; i < particle->maxCnt; (void)(i++, data++)) {
            if (!data->scale) {
                break;
            }
        }
    }
    if (i != particle->maxCnt) {
        configs[i] = *config;
        data->cameraBit = cameraBit;
        data->pos.x = positionX;
        data->pos.y = positionY;
        data->pos.z = positionZ;
        data->vel = config->velocity;
        /* Emission starts with the end color; later updates sample the start-to-end
         * color interpolation. */
        data->color.r = config->endColor.r;
        data->color.g = config->endColor.g;
        data->color.b = config->endColor.b;
        data->color.a = config->endColor.a;
        data->scaleBase = scale;
        data->scale = scale;
        data->time = 0;
        data->parManId = -1;
        particle->emitCnt = i;
        result = i;
    } else {
        result = -1;
    }
    /* Every valid emitter is moved to layer 1, even when emission found no free slot. */
    Hu3DModelLayerSet(modelId, 1);
    return result;
}
/* Counts model and motion slots before fn_1_6384 creates a resource object. */
void fn_1_5E48(M658ResourceEntry *entries, s32 *modelCountOut, s32 *motionCountOut)
{
    s32 modelCount = 0;
    s32 motionCount = 0;
    s32 index = 0;

    for (;;) {
        M658ResourceEntry *entry = &entries[index];
        /* This zero-initialized local is never read. */
        s32 rowState = 0;

        if (entry->type == 9) {
            break;
        }
        switch (entry->type) {
        case 0:
        case 2:
            modelCount = entry->linkedModelCount + modelCount;
            modelCount++;
            break;
        case 3:
            modelCount++;
            motionCount++;
            break;
        case 1:
        case 8:
            if (entry->targetModelIndex >= 0) {
                motionCount++;
            }
        case 4:
            /* Types 1 and 8 also fall through these assertions, which are omitted
             * outside _DEBUG. */
            ASSERT(FALSE);
        case 5:
            ASSERT(FALSE);
        case 6:
            ASSERT(FALSE);
        case 7:
            ASSERT(FALSE);
        }
        index++;
    }
    *modelCountOut = modelCount;
    *motionCountOut = motionCount;
}

extern s16 Hu3DModelCreate(void *);
extern s16 Hu3DModelLink(s16);
extern s16 Hu3DMotionCreate(void *);
extern void Hu3DCameraMotionOff(s16);
extern s16 Hu3DJointMotion(s16, void *);
extern s16 Hu3DMotionShapeIDGet(s16);
extern void Hu3DMotionSet(s16, s16);
extern void Hu3DMotionShapeSet(s16, s16);
extern void Hu3DModelAttrSet(s16, u32);
extern void Hu3DModelAttrFlagSet(s16, u32);
extern void Hu3DModelAttrFlagReset(s16, u32);
extern void Hu3DModelAttrFlagGet(s16);
extern void Hu3DModelAttrReset(s16, u32);
/* fn_1_6384 calls this after reserving model and motion slots to load a resource object's
 * assets. */
void fn_1_5EFC(OMOBJ *work, M658ResourceEntry *entries)
{
    s32 resourceCount = 0;
    s32 motionCount = 0;
    s32 entryIndex = 0;
    s32 linkIndex;
    M658ResourceEntry *entry;
    void *resource;
    s32 modelId;
    s32 rootModelId;
    s32 motionId;

    for (;;) {
        entry = &entries[entryIndex];
        resource = NULL;

        if (entry->type == 9) {
            break;
        }
        if (entry->resourceId >= 0) {
            resource = HuDataSelHeapReadNum(entry->resourceId, HU_MEMNUM_OVL, 2);
        }

        switch (entry->type) {
        case 0:
        case 2:
            rootModelId = Hu3DModelCreate(resource);
            modelId = rootModelId;
            work->mdlId[resourceCount++] = modelId;
            if (entry->type == 2) {
                Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
            }
            for (linkIndex = 0; linkIndex < entry->linkedModelCount; linkIndex++) {
                modelId = Hu3DModelLink(rootModelId);
                work->mdlId[resourceCount++] = modelId;
                if (entry->type == 2) {
                    Hu3DModelAttrSet(modelId, HU3D_ATTR_DISPOFF);
                }
            }
            if (entry->linkedModelCount > 0) {
                /* When linked instances exist, hide their source model even for type 0
                 * resources. */
                Hu3DModelAttrSet(rootModelId, HU3D_ATTR_DISPOFF);
                work->work[0] |= M658_RESOURCE_HAS_LINKED_MODELS_FLAG;
            }
            break;
        case 3:
            motionId = Hu3DMotionCreate(resource);
            work->mtnId[motionCount++] = motionId;
            modelId = Hu3DModelCameraCreate(motionId, 1);
            work->mdlId[resourceCount++] = modelId;
            Hu3DCameraMotionOff(modelId);
            break;
        case 1:
        case 8:
            if (entry->targetModelIndex >= 0) {
                modelId = work->mdlId[entry->targetModelIndex];
                motionId = Hu3DJointMotion(modelId, resource);
                switch (entry->type) {
                case 1:
                    if (Hu3DData[modelId].motId < 0) {
                        Hu3DMotionSet(modelId, motionId);
                    }
                    break;
                case 8:
                    if (Hu3DMotionShapeIDGet(modelId) < 0) {
                        Hu3DMotionSet(modelId, motionId);
                        Hu3DMotionShapeSet(modelId, motionId);
                    }
                    break;
                }
                if (entry->modelAttributes != 0) {
                    Hu3DModelAttrSet(modelId, entry->modelAttributes);
                }
                work->mtnId[motionCount++] = motionId;
            }
        case 4:
            /* Types 1 and 8 also fall through these assertions, which are omitted
             * outside _DEBUG. */
            ASSERT(FALSE);
        case 5:
            ASSERT(FALSE);
        case 6:
            ASSERT(FALSE);
        case 7:
            ASSERT(FALSE);
        }
        entryIndex++;
    }
}

void fn_1_5780(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

/* fn_1_64B4 calls this to build particle models and emitter buffers. Resource reuse retains
* the preceding animation settings. Configuration reuse leaves this iteration's local
* configuration-buffer pointer uninitialized. */
void fn_1_6180(M658ParticleOwner *owners)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    ANIMDATA *anim;
    /* This zero-initialized local is never read. */
    void *unusedValue = NULL;
    s32 i;
    s32 j;
    s32 count;
    s32 blendMode;

    for (i = 0;; i++) {
        void *file;
        M658ParticleOwner *owner;
        M658ParticleConfig *configs;

        owner = &owners[i];
        file = NULL;
        if (owner->resource < 0) {
            break;
        }
        if (!(owner->flags & M658_PARTICLE_OWNER_REUSE_RESOURCE_FLAG)) {
            file = HuDataSelHeapReadNum(owner->resource, HU_MEMNUM_OVL, 2);
            anim = owner->anim = HuSprAnimRead(file);
            count = owner->count;
            blendMode = owner->blendMode;
        }
        if (!(owner->flags & M658_PARTICLE_OWNER_REUSE_CONFIG_FLAG)) {
            configs = HuMemDirectMallocNum(0, count * sizeof(M658ParticleConfig), HU_MEMNUM_OVL);
            memset(configs, 0, count * sizeof(M658ParticleConfig));
        }
        owner->model = Hu3DParticleCreate(anim, count);
        Hu3DParticleBlendModeSet(owner->model, blendMode);
        Hu3DParticleHookSet(owner->model, fn_1_5780);
        particle = Hu3DData[owner->model].hookData;
        particle->emitCnt = 0;
        particle->work = configs;
        particle->count = 1;
        data = particle->data;
        for (j = 0; j < particle->maxCnt; (void)(j++, data++)) {
            data->scale = 0.0f;
        }
    }
}

void fn_1_62EC(M658ParticleAnimEntry *entries);
/* fn_1_64E8 calls this to load the particle animations listed in the supplied records. */
void fn_1_62EC(M658ParticleAnimEntry *entries)
{
    void *resourceData;
    ANIMDATA *anim;
    s32 index;
    /* This zero-initialized local is never read. */
    u32 unusedInitialWord = 0;
    M658ParticleAnimEntry *entry;

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

/* Creates an object for a resource list, reserves its model and motion slots, and loads it. */
OMOBJ *fn_1_6384(void *description)
{
    extern void fn_1_5E48(M658ResourceEntry *entries, s32 *models, s32 *motions);
    extern void fn_1_5EFC(OMOBJ *object, M658ResourceEntry *entries);
    s32 modelCount;
    s32 motionCount;
    OMOBJ *object;

    fn_1_5E48(description, &modelCount, &motionCount);
    object = omAddObjEx(lbl_1_bss_0, 32730, modelCount, motionCount, -1, NULL);
    object->stat |= OM_STAT_MODELPAUSE;
    fn_1_5EFC(object, description);
    return object;
}

/* The closing-scene transition calls this to kill a resource object's models and motions, then
 * delete the object. */
void fn_1_6414(OMOBJ *object) {
    s32 resourceIndex;
    /* This local is declared but never read. */
    s32 unusedValue;
    if (object != NULL)
    {
        resourceIndex = object->mdlcnt - 1;
        while (resourceIndex >= 0)
        {
            Hu3DModelKill(object->mdlId[resourceIndex]);
            resourceIndex -= 1;
        }
        resourceIndex = object->mtncnt - 1;
        while (resourceIndex >= 0)
        {
            Hu3DMotionKill(object->mtnId[resourceIndex]);
            resourceIndex -= 1;
        }
        omDelObjEx(lbl_1_bss_0, object);
    }
}

/* Initializes particle systems from the supplied owner records and returns those records. */
void *fn_1_64B4(void *work)
{
    extern void fn_1_6180(M658ParticleOwner *owners);
    fn_1_6180(work);
    return work;
}

/* Loads the particle animations listed in the supplied records and returns those records. */
void *fn_1_64E8(void *work)
{
    extern void fn_1_62EC(M658ParticleAnimEntry *entries);
    fn_1_62EC(work);
    return work;
}
/* Hides or shows every owned model when minigame code changes resource visibility. */
void fn_1_651C(OMOBJ *object, s32 visible)
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

void fn_1_65A0(void *item);
/* fn_1_66F4 calls this to unlink a pool node before returning its item to the free list. */
void fn_1_65A0(void *item)
{
    M658PoolNode *node;

    node = item;
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
}

void *fn_1_65E8(u8 *buffer, s32 bytes, s32 itemBytes);
/* fn_1_5430 calls this during registry setup to initialize its free-node list. */
void *fn_1_65E8(u8 *buffer, s32 bytes, s32 itemBytes)
{
    M658Pool *pool;
    M658PoolNode *node;
    s32 stride;

    memset(buffer, 0, bytes);
    pool = (M658Pool *)buffer;
    buffer += sizeof(M658Pool);
    bytes -= sizeof(M658Pool);
    while (bytes > 0) {
        node = (M658PoolNode *)buffer;
        if (pool->free) {
            pool->free->prev = node;
        }
        node->next = pool->free;
        pool->free = node;
        stride = itemBytes + sizeof(M658PoolNode);
        buffer += stride;
        bytes -= stride;
    }
    return pool;
}

void *fn_1_6680(M658Pool *pool);
/* fn_1_5430 calls this during registry setup to activate each initial item. */
void *fn_1_6680(M658Pool *pool)
{
    M658PoolNode *node;
    u8 *item;

    if (!pool->free) {
        return NULL;
    }
    node = pool->free;
    pool->free = pool->free->next;
    if (pool->active) {
        pool->active->prev = node;
    }
    node->prev = NULL;
    node->next = pool->active;
    pool->active = node;
    item = (u8 *)node;
    item += sizeof(M658PoolNode);
    return item;
}

void fn_1_66F4(M658Pool *pool, void *item);
void fn_1_65A0(void *item);
/* Pushes a released node onto the free list; it leaves the active-list head and
 * the node's previous link unchanged. */
void fn_1_66F4(M658Pool *pool, void *item)
{
    extern void fn_1_65A0(void *item);
    u8 *buffer;
    M658PoolNode *node;

    buffer = item;
    buffer -= sizeof(M658PoolNode);
    node = (M658PoolNode *)buffer;
    fn_1_65A0(node);
    if (pool->free) {
        pool->free->prev = node;
    }
    node->next = pool->free;
    pool->free = node;
}
/* Starts the active-item iteration used by registry and gameplay update loops. */
void *fn_1_6764(void *pool)
{
    M658PoolNode *node;
    u8 *data;

    node = ((M658Pool *)pool)->active;
    data = (u8 *)node;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M658PoolNode);
}
/* Advances active-item iteration used by registry and gameplay update loops. */
void *fn_1_67A0(void *pool, void *item)
{
    M658PoolNode *node;
    u8 *data;

    data = item;
    data -= sizeof(M658PoolNode);
    node = (M658PoolNode *)data;
    data = (u8 *)node->next;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M658PoolNode);
}

typedef struct M658TrackKey {
    s32 frame;
    f32 *values;
} M658TrackKey;

typedef struct M658Track {
    s16 kind;
    s16 count;
    s16 end;
    M658TrackKey *keys;
} M658Track;
/* Tests whether all four optional tracks are past their final frames. */
s32 fn_1_67E4(M658Track **tracks, s32 frame)
{
    s32 i;

    for (i = 0; i < 4; i++) {
        if (tracks[i] != NULL && frame <= tracks[i]->end) {
            return 0;
        }
    }
    return 1;
}

f32 fn_1_69B8(f32 startValue, f32 endValue, f32 interpolation);
/* Samples every value of a kind-0 track by keyframe interpolation; other kinds leave output
 * unchanged. */
void fn_1_6840(M658Track **tracks, s32 frame, s32 trackIndex, f32 *outValues)
{
    M658Track *track;
    M658TrackKey *keys;
    M658TrackKey *lowerKey;
    M658TrackKey *upperKey;
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
        interpolation = (f32)frameOffset / (f32)frameSpan;
    }
    for (valueIndex = 0; valueIndex < track->count; valueIndex++) {
        outValues[valueIndex] =
            fn_1_69B8(lowerKey->values[valueIndex], upperKey->values[valueIndex], interpolation);
    }
        break;
    }
}

f32 fn_1_69B8(f32 startValue, f32 endValue, f32 interpolation)
{
    return startValue + (interpolation * (endValue - startValue));
}
/* Player state 13 uses this to interpolate the character's closing path between its endpoints. */
void fn_1_69C8(Point3d *start, Point3d *end, Point3d *out, f32 t)
{
    out->x = start->x + (t * (end->x - start->x));
    out->y = start->y + (t * (end->y - start->y));
    out->z = start->z + (t * (end->z - start->z));
}

extern f64 atan2(f64 y, f64 x);
extern f64 fmod(f64 x, f64 y);
extern f64 fn_1_6C54(f64 value);
f32 fn_1_6A20(HuVecF *direction)
{
    return (180.0 * atan2(direction->x, direction->z)) / 3.141592653589793;
}
/* Interpolates separately normalized angle endpoints using their raw difference,
 * without choosing the shortest turn across zero. */
f32 fn_1_6A78(f32 startDegrees, f32 endDegrees, f32 interpolation)
{
    f32 startAngle;
    f32 endAngle;
    f32 delta;

    startAngle = fmod(startDegrees, 360.0);
    if (startAngle < 0.0f) {
        startAngle += 360.0f;
    }
    endAngle = fmod(endDegrees, 360.0);
    if (endAngle < 0.0f) {
        endAngle += 360.0f;
    }
    delta = endAngle - startAngle;
    return startAngle + delta * interpolation;
}
/* Normalizes the direction. Near-zero input uses random X/Z and Y = -0.01 before
 * normalization, since frandmod(1) is always zero. */
s32 fn_1_6B58(Point3d *source, Point3d *destination)
{
    f32 vertical;
    s32 result;

    if (PSVECSquareMag(source) < 0.000001) {
        destination->x = 0.01f * ((f32)(u32)frandmod(20) - 10.0f);
        destination->z = 0.01f * ((f32)(u32)frandmod(20) - 10.0f);
        if (frandmod(1) != 0U) {
            vertical = 0.01f;
        } else {
            vertical = -0.01f;
        }
        destination->y = vertical;
        PSVECNormalize(destination, destination);
        result = 0;
    } else {
        PSVECNormalize(source, destination);
        result = 1;
    }
    return result;
}

extern s32 fn_1_6B58(Point3d *source, Point3d *destination);
/* Builds a horizontal camera-relative basis, discarding pitch and using the up
 * vector to choose vertical orientation and the near-vertical fallback. */
void fn_1_6CB0(Mtx matrix, s32 cameraMask)
{
    extern s32 fn_1_6B58(Point3d *source, Point3d *destination);
    HuVecF pos;
    HuVecF target;
    HuVecF up;
    HuVecF x;
    HuVecF y;
    HuVecF z;
    HuVecF horizontal;
    HuVecF direction;
    HuVecF cameraPos;
    HuVecF cameraTarget;
    HuVecF cameraUp;

    Hu3DCameraPosGet(cameraMask, &cameraPos, &cameraUp, &cameraTarget);
    pos.x = cameraPos.x;
    pos.y = cameraPos.y;
    pos.z = cameraPos.z;
    target.x = cameraTarget.x;
    target.y = cameraTarget.y;
    target.z = cameraTarget.z;
    up.x = cameraUp.x;
    up.y = cameraUp.y;
    up.z = cameraUp.z;
    x.x = 1.0f;
    x.y = 0.0f;
    x.z = 0.0f;
    y.x = 0.0f;
    y.y = 1.0f;
    y.z = 0.0f;
    z.x = 0.0f;
    z.y = 0.0f;
    z.z = 1.0f;
    PSVECSubtract(&target, &pos, &direction);
    horizontal = direction;
    horizontal.y = 0.0f;
    if (PSVECSquareMag(&horizontal) < 0.001) {
        horizontal = direction;
        direction = up;
        PSVECScale(&horizontal, &up, -1.0f);
    }
    direction.y = 0.0f;
    fn_1_6B58(&direction, &direction);
    if (PSVECDotProduct(&y, &up) < 0.0f) {
        y.y *= -1.0f;
        /* The later assignment of direction to z overwrites this Z sign change. */
        z.z *= -1.0f;
    }
    PSVECCrossProduct(&direction, &y, &x);
    z = direction;
    matrix[0][0] = x.x;
    matrix[0][1] = x.y;
    matrix[0][2] = x.z;
    matrix[0][3] = 0.0f;
    matrix[1][0] = y.x;
    matrix[1][1] = y.y;
    matrix[1][2] = y.z;
    matrix[1][3] = 0.0f;
    matrix[2][0] = z.x;
    matrix[2][1] = z.y;
    matrix[2][2] = z.z;
    matrix[2][3] = 0.0f;
}
/* Material diagnostics discard their sampled address in the game build. */
static inline void DebugValue(u32 address) { }

/* Player setup calls this to save actor material RGB values for the later flash effect. */
s8 *fn_1_6F70(s32 model)
{
    /* This zero-initialized local is never read. */
    s16 unusedValue = 0;
    HU3D_MODEL *modelData;
    s8 *result;
    s8 *out;
    s16 i;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    result = HuMemDirectMallocNum(HEAP_HEAP, modelData->hsf->materialNum * 3, HU_MEMNUM_OVL);
    out = result;
    DebugValue((u32)(i = 0, material = modelData->hsf->material));
    while (i < modelData->hsf->materialNum) {
        *out = (s8)material->color[0];
        out++;
        *out = (s8)material->color[1];
        out++;
        *out = (s8)material->color[2];
        out++;
        DebugValue((u32)(i++, material++));
    }
    return result;
}
/* The player's flash callback restores actor material RGB bytes when its timer expires. */
void fn_1_7048(s32 model, u8 *colors)
{
    /* This zero-initialized local is never read. */
    s16 unusedValue = 0;
    HU3D_MODEL *modelData;
    u8 *input;
    s16 i;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    input = colors;
    DebugValue((u32)(i = 0, material = modelData->hsf->material));
    while (i < modelData->hsf->materialNum) {
        material->color[0] = (u8)*input;
        input++;
        material->color[1] = (u8)*input;
        input++;
        material->color[2] = (u8)*input;
        input++;
        DebugValue((u32)(i++, material++));
    }
}
/* The flash callback alternates full saved RGB and black at four-frame intervals. */
void fn_1_70F8(s32 model, const s8 *colors, f32 scale)
{
    /* This zero-initialized local is never read. */
    s16 unusedValue = 0;
    HU3D_MODEL *modelData;
    const s8 *input;
    s16 i;
    HSF_MATERIAL *material;

    modelData = &Hu3DData[model];
    input = colors;
    DebugValue((u32)(i = 0, material = modelData->hsf->material));
    for (; i < modelData->hsf->materialNum; DebugValue((u32)(i++, material++))) {
        material->color[0] = *input * scale;
        ++input;
        material->color[1] = *input * scale;
        ++input;
        material->color[2] = *input * scale;
        ++input;
    }
}
/* Presentation updates copy a named attachment transform to a target model. */
void fn_1_7244(s32 modelId, char *objectName, s32 targetModelId)
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
/* Computer input timing uses summed random draws and cumulative weights to select
 * a timing variant; a zero sum selects slot zero. */
s32 fn_1_72D0(s32 modulus, s32 count, s32 *weights)
{
    s32 index;
    u32 random;
    u32 cumulative;

    random = 0;
    for (index = 0; index < count; index++) {
        random += frandmod(modulus);
    }
    cumulative = 0;
    for (index = 0; ; index++) {
        cumulative += weights[index];
        if (cumulative >= random) {
            return index;
        }
    }
}

/* Game events use this for spatial effects; it projects the source and pans at full volume. */
void fn_1_736C(HuVecF *worldPosition, int soundId)
{
    HuVecF projected;
    f32 ratio;
    s16 pan;
    s16 volume;
    s32 cameraBit;

    cameraBit = 1;
    Hu3D3Dto2D(worldPosition, cameraBit, &projected);
    if (projected.x > 608.0f) {
        pan = 96;
    } else if (projected.x < 32.0f) {
        pan = 32;
    } else {
        ratio = (projected.x - 32.0f) / 576.0f;
        pan = (s32) (64.0f * ratio) + 32;
    }
    volume = 127;
    HuAudFXPlayVolPan(soundId, volume, pan);
}

s32 fn_1_7468(s32 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, 64);
}

extern unsigned int CharSeTable[];
extern unsigned int CharVoiceSeTable[];
/* When a minigame plays a character sound, this maps its shared ID to that character. */
s32 fn_1_7498(s16 charNo, s32 seId)
{
    if (omSysExitReq) {
        return 0;
    }
    if (seId < MSM_SE_CHARVOICE_MARIO) {
        seId -= MSM_SE_CHAR_MARIO;
        seId += CharSeTable[charNo];
    } else {
        seId -= MSM_SE_CHARVOICE_MARIO;
        seId += CharVoiceSeTable[charNo];
    }
    return HuAudFXPlay(seId);
}

M658ResourceEntry lbl_1_data_5E8[] = {
    { DATANUM(DATA_m658, 35), 3, 0, 0, -1 },
    { DATANUM(DATA_m658, 36), 3, 0, 0, -1 },
    { DATANUM(DATA_m658, 37), 3, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_628[] = {
    { DATANUM(DATA_m658, 20), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 21), 0, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_658[] = {
    { DATANUM(DATA_m658, 0), 2, 0, 0, -1 },
    { DATANUM(DATA_m658, 1), 2, 0, 0, -1 },
    { DATANUM(DATA_m658, 2), 2, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_698[] = {
    { DATANUM(DATA_m658, 3), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 4), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 5), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 6), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 7), 1, 0, 0, 0 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_6F8[] = {
    { DATANUM(DATA_m658, 10), 0, 0, 10, -1 },
    { DATANUM(DATA_m658, 11), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 12), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 13), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 14), 1, 0, 0, 0 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_758[] = {
    { DATANUM(DATA_m658, 15), 0, 0, 1, -1 },
    { DATANUM(DATA_m658, 16), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 17), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 18), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 19), 1, 0, 0, 0 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_7B8[] = {
    { DATANUM(DATA_m658, 8), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 9), 0, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_7E8[] = {
    { DATANUM(DATA_m658, 22), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 23), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 24), 1, 0, 0, 0 },
    { DATANUM(DATA_m658, 25), 1, 0, 0, 1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_838[] = {
    { DATANUM(DATA_m658, 26), 0, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_858[] = {
    { DATANUM(DATA_m658, 27), 0, 0, 2, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_878[] = {
    { DATANUM(DATA_m658, 28), 0, 0, 2, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_898[] = {
    { DATANUM(DATA_m658, 30), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 31), 1, 0, 0, 0 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_8C8[] = {
    { DATANUM(DATA_m658, 32), 0, 0, 0, -1 },
    { DATANUM(DATA_m658, 33), 1, 0, 0, 0 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_8F8[] = {
    { DATANUM(DATA_m658, 29), 0, 0, 2, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry lbl_1_data_918[] = {
    { DATANUM(DATA_m658, 34), 0, 0, 0, -1 },
    { 0, 9, 0, -1, -1 },
};

M658ResourceEntry *lbl_1_data_938[] = {
    lbl_1_data_6F8, lbl_1_data_758, lbl_1_data_698, lbl_1_data_658,
    lbl_1_data_7B8, lbl_1_data_858, lbl_1_data_8F8, lbl_1_data_878,
    lbl_1_data_628, lbl_1_data_5E8, lbl_1_data_898, lbl_1_data_8C8,
    lbl_1_data_918, lbl_1_data_7E8, lbl_1_data_838
};

M658ResourceEntry *fn_1_753C(s32 index) { return lbl_1_data_938[index]; }
