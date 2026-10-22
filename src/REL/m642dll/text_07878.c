/* Particle effects, resource loading and grid animation helpers for Cashapult. */
#include "REL/m642dll/work.h"

#include "dolphin/os.h"

#include "string.h"

extern s16 fn_1_92D4(void);

void fn_1_7878(s32 playerIndex, s16 minigameCoins)
{
    GwPlayer[playerIndex].mgCoin = minigameCoins;
}

s16 fn_1_7890(void) { return GwMgNightF; }

typedef struct M642ParticleConfig_s {
    u32 flags; /* Particle behavior flags, including alternating-size animation. */
    GXColor endColor, startColor; /* Starting and target particle tints. */
    Vec velocity, decay; /* Initial velocity and per-frame velocity multipliers. */
    float gravity; /* Vertical velocity added each frame. */
    float scale; /* Base size configured for this particle. */
    float scaleDelta; /* Per-frame change to the particle's base size. */
    float alphaDelta; /* Per-frame change to particle transparency. */
    float colorTime; /* Interpolation amount between the configured tint colors. */
} M642ParticleConfig;

typedef struct M642ResourceRow_s {
    s32 dataNumber; /* Packed resource number, or a negative value for no resource. */
    s32 type; /* Resource kind understood by the scene loader. */
    s32 flags; /* Kind-specific model attributes or particle blend settings. */
    s16 count; /* Linked-model count or particle capacity, depending on type. */
    s16 modelIndex; /* Existing model slot used by joint-motion rows; read then overwritten by
                     * particle rows. */
} M642ResourceRow;

typedef struct M642ResourceTableView_s {
    s16 modelCount; /* Number of primary model slots needed by the scene. */
    s16 linkedModelCount; /* Additional model slots for linked models. */
    s32 motionCount; /* Motion slots needed by the scene object. */
    M642ResourceRow rows[16]; /* Ordered resource entries, ending with type 8. */
} M642ResourceTableView;

typedef struct M642ParticleAnimEntry {
    s32 resourceId; /* Packed animation resource number; a negative value ends the list. */
    ANIMDATA *anim; /* Loaded animation data. */
} M642ParticleAnimEntry;

/* Pool header holding the active and free item lists. */
typedef struct M642Pool {
    M642PoolNode *active, *free; /* Heads of the used-item and available-item lists. */
    u8 unknown08[8]; /* No Cashapult code reads these bytes of the pool header. */
} M642Pool;

typedef struct M642TrackKeyView {
    s32 frame; /* Animation frame; a negative value terminates the key list. */
    f32 values[3]; /* Up to three values sampled from this key. */
} M642TrackKeyView;

typedef struct M642TrackView {
    s16 kind; /* Interpolation mode used for this track. */
    s16 count; /* Number of values sampled from each key. */
    s16 end; /* Last frame at which the track is active. */
    M642TrackKeyView *keys; /* Ordered animation keys, terminated by a negative frame. */
} M642TrackView;

f32 fn_1_89E8(f32 startValue, f32 endValue, f32 fraction);

/* The particle model calls this hook each frame to update live Cashapult particles. */
void fn_1_78A0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M642ParticleConfig *config = particle->work;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    float alpha, alphaDeltaCopy;

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
        data->color.r =
            fn_1_89E8(config[i].startColor.r, config[i].endColor.r, config[i].colorTime);
        data->color.g =
            fn_1_89E8(config[i].startColor.g, config[i].endColor.g, config[i].colorTime);
        data->color.b =
            fn_1_89E8(config[i].startColor.b, config[i].endColor.b, config[i].colorTime);
        alpha = data->color.a;

        alphaDeltaCopy = config[i].alphaDelta;
        alpha = data->color.a + config[i].alphaDelta;
        if (alpha <= 0.0f) {
            data->scale = 0.0f;
        }
        data->color.a = alpha;
        if (data->scale) {
            if (config[i].flags & 0x1) {
                data->scale = data->scaleBase * (((data->time + i) & 1) ? 1.0 : 0.5);
            } else {
                data->scale = data->scaleBase;
            }
            data->scaleBase += config[i].scaleDelta;
            if (data->scaleBase <= 0.01f) {
                data->scale = 0.0f;
            }
        }
        data->time++;
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Called by Cashapult effect callbacks to emit a configured particle in world space. */
s16 fn_1_7D1C(M642ParticleOwner *owner, s16 camera, float x, float y, float z, float scale,
              M642ParticleConfig *config)
{
    s16 result = -1;
    s16 modelId = owner->model;
    HU3D_MODEL *model;
    HU3D_PARTICLE *particle;
    M642ParticleConfig *configs;
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
        data->cameraBit = camera;
        data->pos.x = x;
        data->pos.y = y;
        data->pos.z = z;
        data->vel = config->velocity;
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
    Hu3DModelLayerSet(modelId, 1);
    return result;
}

/* Effect callbacks use this to wait until the emitted particles have all disappeared. */
s32 fn_1_7F68(M642ParticleOwner *owner)
{

    s16 noModelResult = -1;
    HU3D_MODEL *model;
    s16 modelId = owner->model;
    HU3D_PARTICLE *particle;
    void *particleWork;
    HU3D_PARTICLE_DATA *data;
    s16 i;
    if (modelId == -1) {
        return -1;
    }
    model = &Hu3DData[modelId];
    particle = model->hookData;

    particleWork = particle->work;
    data = particle->data;
    for (i = particle->emitCnt; i < particle->maxCnt; (void)(i++, data++)) {
        if (data->scale) {
            return 0;
        }
    }
    return 1;
}

/* Called by fn_1_84D8 to load the scene's model, motion and particle resources in table order. */
void fn_1_8024(OMOBJ *owner, M642ResourceTableView *resources)
{
    s32 modelCount = 0;
    s32 motionCount = 0;
    s32 index = 0;
    s32 linkIndex;
    ANIMDATA *particleAnimation;

    for (;;) {
        M642ResourceRow *row = &resources->rows[index];
        void *data = NULL;
        s32 model, motion;
        if (row->type == 8) {
            break;
        }
        if (row->dataNumber >= 0) {
            data = HuDataSelHeapReadNum(row->dataNumber, HU_MEMNUM_OVL, HEAP_MODEL);
        }
        switch (row->type) {
        case 0:
            model = Hu3DModelCreate(data);
            owner->mdlId[modelCount++] = model;
            if (row->count != 0) {
                for (linkIndex = 0; linkIndex < row->count; linkIndex++) {
                    owner->mdlId[modelCount++] = Hu3DModelLink(model);
                }
            }
            break;
        case 2:
            model = Hu3DModelCreate(data);
            owner->mdlId[modelCount++] = model;
            Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF);
            break;
        case 3:
            motion = Hu3DMotionCreate(data);
            owner->mtnId[motionCount++] = motion;
            model = Hu3DModelCameraCreate(motion, 1);
            owner->mdlId[modelCount++] = model;
            break;
        case 1:
            if (row->modelIndex >= 0) {
                model = owner->mdlId[row->modelIndex];
                motion = Hu3DJointMotion(model, data);
                if (Hu3DData[model].motId < 0) {
                    Hu3DMotionSet(model, motion);
                }
                if (row->flags != 0) {
                    Hu3DModelAttrSet(model, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP);
                }
                owner->mtnId[motionCount++] = motion;
            }
            break;
        case 6:
            particleAnimation = HuSprAnimRead(data);
            model = Hu3DParticleCreate(particleAnimation, row->count);
            Hu3DParticleBlendModeSet(model, row->flags & 0xFF);
            owner->mdlId[modelCount++] = model;
            break;
        case 7:
            if (particleAnimation) {
                model = owner->mdlId[row->modelIndex];
                model = Hu3DParticleCreate(particleAnimation, row->count);
                Hu3DParticleBlendModeSet(model, row->flags & 0xFF);
                owner->mdlId[modelCount++] = model;
            }
            break;
        case 4:
        case 5:
            break;
        }
        index++;
    }
}

/* Called by fn_1_8568 during scene setup to load each particle owner's reusable buffers. */
void fn_1_82D4(M642ParticleOwner *owners)
{
    HU3D_PARTICLE *particle;
    HU3D_PARTICLE_DATA *data;
    ANIMDATA *anim;

    void *resourceBuffer = NULL;
    s32 i, j;
    s32 count, blendMode;
    for (i = 0; ; i++) {
        void *file;
        M642ParticleOwner *owner;
        M642ParticleConfig *configs;
        owner = &owners[i];
        file = NULL;
        if (owner->resource < 0) {
            break;
        }
        if (!(owner->flags & 1)) {
            file = HuDataSelHeapReadNum(owner->resource, HU_MEMNUM_OVL, HEAP_MODEL);
            anim = owner->animation = HuSprAnimRead(file);
            count = owner->count;
            blendMode = owner->blendMode;
        }
        if (!(owner->flags & 2)) {
            configs = HuMemDirectMallocNum(0, count * sizeof(M642ParticleConfig), HU_MEMNUM_OVL);
            memset(configs, 0, count * sizeof(M642ParticleConfig));
        }
        owner->model = Hu3DParticleCreate(anim, count);
        Hu3DParticleBlendModeSet(owner->model, blendMode);
        Hu3DParticleHookSet(owner->model, fn_1_78A0);
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

/* Called by fn_1_859C to load animations from the scene's resource list through its sentinel. */
void fn_1_8440(M642ParticleAnimEntry *entries)
{
    void *resourceData;
    ANIMDATA *anim;
    s32 index;

    u32 initialResourceMarker = 0;
    M642ParticleAnimEntry *entry;

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

/* Creates the scene's object slots, then loads the resources described by its table. */
OMOBJ *fn_1_84D8(M642ResourceTableView *capacity)
{
    OMOBJ *object;

    s32 initialObjectMarker = 1;
    s32 models = capacity->modelCount + capacity->linkedModelCount;
    s32 motions = capacity->motionCount;
    object = omAddObjEx(lbl_1_bss_0.manager, 32730, models, motions, -1, 0);
    fn_1_8024(object, capacity);
    return object;
}

/* Called during effect setup to load each particle owner's animation and buffers. */
void *fn_1_8568(void *work)
{
    extern void fn_1_82D4(M642ParticleOwner *owners);
    fn_1_82D4(work);
    return work;
}

/* Called during effect setup to load the shared particle animation list. */
void *fn_1_859C(void *work)
{
    extern void fn_1_8440(M642ParticleAnimEntry *entries);
    fn_1_8440(work);
    return work;
}

/* Adjusts neighboring links to detach an item before its caller returns it to a pool. */
void fn_1_85D0(void *item)
{
    M642PoolNode *node;

    node = item;
    if (node->prev) {
        node->prev->next = node->next;
    }
    if (node->next) {
        node->next->prev = node->prev;
    }
}

/* Initializes a fixed-size pool in caller-provided storage and links its free items. */
void *fn_1_8618(void *buffer, s32 bytes, s32 itemBytes)
{
    M642Pool *pool;
    M642PoolNode *node;
    s32 stride;

    memset(buffer, 0, bytes);
    pool = (M642Pool *)buffer;
    buffer = (u8 *)buffer + sizeof(M642Pool);
    bytes -= sizeof(M642Pool);
    while (bytes > 0) {
        node = (M642PoolNode *)buffer;
        if (pool->free) {
            pool->free->prev = node;
        }
        node->next = pool->free;
        pool->free = node;
        stride = itemBytes + sizeof(M642PoolNode);
        buffer = (u8 *)buffer + stride;
        bytes -= stride;
    }
    return pool;
}

/* Called by grid and effect callbacks when they need an item from a pool. */
void *fn_1_86B0(M642Pool *pool)
{
    M642PoolNode *node;
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
    item += sizeof(M642PoolNode);
    return item;
}

/* Returns a used item to its pool after the owning grid or effect is finished with it. */
void fn_1_8724(M642Pool *pool, void *item)
{
    extern void fn_1_85D0(void *item);
    u8 *buffer;
    M642PoolNode *node;

    buffer = item;
    buffer -= sizeof(M642PoolNode);
    node = (M642PoolNode *)buffer;
    fn_1_85D0(node);
    if (pool->free) {
        pool->free->prev = node;
    }
    node->next = pool->free;
    pool->free = node;
}

/* Starts traversal at the first active item in a pool. */
void *fn_1_8794(void *pool)
{
    M642PoolNode *node;
    u8 *data;

    node = ((M642Pool *)pool)->active;
    data = (u8 *)node;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M642PoolNode);
}

/* Advances traversal to the next active item in a pool. */
void *fn_1_87D0(void *pool, void *item)
{
    M642PoolNode *node;
    u8 *data;

    data = item;
    data -= sizeof(M642PoolNode);
    node = (M642PoolNode *)data;
    data = (u8 *)node->next;
    if (data == NULL) {
        return NULL;
    }
    return data + sizeof(M642PoolNode);
}

/* Effect animation callbacks use this to test whether all three tracks passed their end frames. */
s32 fn_1_8814(M642TrackView **tracks, s32 frame)
{
    s32 i;

    for (i = 0; i < 3; i++) {
        if (tracks[i] != NULL && frame <= tracks[i]->end) {
            return 0;
        }
    }
    return 1;
}

/* Effect animation callbacks use this to interpolate each value in a track at the current frame. */
void fn_1_8870(M642TrackView **tracks, s32 frame, s32 trackIndex, f32 *outValues)
{
    M642TrackView *track;
    M642TrackKeyView *keys;
    M642TrackKeyView *lowerKey;
    M642TrackKeyView *upperKey;
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
            outValues[valueIndex] = fn_1_89E8(lowerKey->values[valueIndex],
                                              upperKey->values[valueIndex], interpolation);
        }
        break;
    }
}

/* Interpolates one animation channel between two key values. */
f32 fn_1_89E8(f32 startValue, f32 endValue, f32 fraction)
{
    return startValue + (fraction * (endValue - startValue));
}

/* Interpolates a three-dimensional position between animation keys. */
void fn_1_89F8(Point3d *start, Point3d *end, f32 t, Point3d *out)
{
    out->x = start->x + (t * (end->x - start->x));
    out->y = start->y + (t * (end->y - start->y));
    out->z = start->z + (t * (end->z - start->z));
}

/* Normalizes a direction, choosing a small random direction when the input is near zero. */
s32 fn_1_8A50(Point3d *source, Point3d *destination)
{
    f32 fallbackVertical;
    s32 usedSourceDirection;

    if (PSVECSquareMag(source) < 0.000001) {
        destination->x = 0.01f * ((f32) (u32) frandmod(20) - 10.0f);
        destination->z = 0.01f * ((f32) (u32) frandmod(20) - 10.0f);
        if (frandmod(1) != 0U) {
            fallbackVertical = 0.01f;
        } else {
            fallbackVertical = (-0.01f);
        }
        destination->y = fallbackVertical;
        PSVECNormalize(destination, destination);
        usedSourceDirection = 0;
    } else {
        PSVECNormalize(source, destination);
        usedSourceDirection = 1;
    }
    return usedSourceDirection;
}

/* Builds the camera-facing basis used to orient Cashapult effects on the board. */
void fn_1_8BA8(Mtx output, s32 cameraId)
{
    extern s32 fn_1_8A50(Point3d *source, Point3d *destination);
    HuVecF position;
    HuVecF target;
    HuVecF up;
    HuVecF right;
    HuVecF upAxis;
    HuVecF back;
    HuVecF flat;
    HuVecF direction;
    HuVecF cameraPosition;
    HuVecF cameraTarget;
    HuVecF cameraUp;

    Hu3DCameraPosGet(cameraId, &cameraPosition, &cameraUp, &cameraTarget);
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
    back.x = 0.0f;
    back.y = 0.0f;
    back.z = 1.0f;

    PSVECSubtract(&target, &position, &direction);
    flat = direction;
    flat.y = 0.0f;
    if (PSVECSquareMag(&flat) < 0.001) {
        flat = direction;
        direction = up;
        PSVECScale(&flat, &up, (-1.0f));
    }
    direction.y = 0.0f;
    fn_1_8A50(&direction, &direction);

    if (PSVECDotProduct(&upAxis, &up) < 0.0f) {
        upAxis.y *= (-1.0f);
        back.z *= (-1.0f);
    }
    PSVECCrossProduct(&direction, &upAxis, &right);
    back = direction;

    output[0][0] = right.x;
    output[0][1] = right.y;
    output[0][2] = right.z;
    output[0][3] = 0.0f;
    output[1][0] = upAxis.x;
    output[1][1] = upAxis.y;
    output[1][2] = upAxis.z;
    output[1][3] = 0.0f;
    output[2][0] = back.x;
    output[2][1] = back.y;
    output[2][2] = back.z;
    output[2][3] = 0.0f;
}

/* Maps a world position to stereo pan using its projected screen x coordinate. */
s16 fn_1_8E68(Vec *position, s32 camera, float nearPan, float farPan)
{
    Vec screen, world;
    s16 pan;
    s32 basePan;
    s32 cameraIndex;
    float fraction;

    cameraIndex = camera;
    basePan = cameraIndex == 1 ? 0 : 32;
    world = *position;
    world.z += 1000.0f;
    Hu3D3Dto2D(&world, cameraIndex, &screen);
    if (screen.x > farPan) {
        pan = basePan + 64;
    } else if (screen.x < nearPan) {
        pan = basePan + 32;
    } else {
        fraction = (screen.x - nearPan) / (farPan - nearPan);
        pan = (basePan + 32) + (s32)(32.0f * fraction);
    }
    return pan;
}

extern s16 fn_1_8E68(Vec *position, s32 camera, float nearPan, float farPan);

/* Plays a sound effect panned according to a world position and camera. */
s32 fn_1_8F9C(Vec *position, s32 camera, float nearPan, float farPan, s32 sound)
{
    extern s16 fn_1_8E68(Vec *position, s32 camera, float nearPan, float farPan);
    s16 pan, volume;
    pan = fn_1_8E68(position, camera, nearPan, farPan);
    volume = 127;
    return HuAudFXPlayVolPan(sound, volume, pan);
}

/* Plays a sound effect with pan derived from the projected world position. */
s32 fn_1_9008(HuVecF *input, f32 lower, f32 upper, s32 soundEffect)
{
    HuVecF projected;
    HuVecF input_copy;
    f32 ratio;
    s16 pan;
    s16 volume;
    s32 camera_bit;

    camera_bit = 1;
    input_copy = *input;
    Hu3D3Dto2D(&input_copy, camera_bit, &projected);

    if (projected.x > upper) {
        pan = 96;
    } else if (projected.x < lower) {
        pan = 32;
    } else {
        ratio = (projected.x - lower) / (upper - lower);
        pan = (s32)(64.0f * ratio) + 32;
    }

    volume = 127;
    return HuAudFXPlayVolPan(soundEffect, volume, pan);
}

/* Plays a sound effect centered between the speakers at full volume. */
s32 fn_1_9120(s32 soundId)
{
    return HuAudFXPlayVolPan(soundId, 127, 64);
}

/* Chooses a weighted entry after summing count draws from the shared game random stream. */
s32 fn_1_9150(s32 modulus, s32 count, s32 *weights)
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

M642ResourceTableView lbl_1_data_4F0 = {
    1, 0, 1, {
        {DATANUM(DATA_m642, 45), 3, 0, 0, -1},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_5F8 = {
    3, 1, 1, {
        {DATANUM(DATA_m642, 1), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 0), 2, 0, 0, -1},
        {DATANUM(DATA_m642, 2), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 3), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 19), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_700 = {
    3, 1, 1, {
        {DATANUM(DATA_m642, 11), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 0), 2, 0, 0, -1},
        {DATANUM(DATA_m642, 12), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 13), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 20), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_808 = {
    1, 0, 5, {
        {DATANUM(DATA_m642, 5), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 26), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 27), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 28), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 29), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 30), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 31), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 32), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_910 = {
    1, 0, 5, {
        {DATANUM(DATA_m642, 15), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 26), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 27), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 28), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 29), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 30), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 31), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 32), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_A18 = {
    1, 0, 5, {
        {DATANUM(DATA_m642, 4), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 21), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 22), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 23), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 24), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 25), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_B20 = {
    1, 0, 5, {
        {DATANUM(DATA_m642, 14), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 21), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 22), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 23), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 24), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 25), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_C28 = {
    1, 0, 6, {
        {DATANUM(DATA_m642, 6), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 33), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 34), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 35), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 36), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 37), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 38), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 39), 1, 0, 0, 0},
        {DATANUM(DATA_m642, 40), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_D30 = {
    1, 0, 2, {
        {DATANUM(DATA_m642, 7), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 41), 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP, 0, 0},
        {DATANUM(DATA_m642, 42), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_E38 = {
    1, 0, 2, {
        {DATANUM(DATA_m642, 17), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 41), 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP, 0, 0},
        {DATANUM(DATA_m642, 42), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_F40 = {
    1, 0, 2, {
        {DATANUM(DATA_m642, 8), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 43), 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP, 0, 0},
        {DATANUM(DATA_m642, 44), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_1048 = {
    1, 0, 2, {
        {DATANUM(DATA_m642, 16), 0, 0, 0, -1},
        {DATANUM(DATA_m642, 43), 1, HU3D_ATTR_DISPOFF | HU3D_MOTATTR_LOOP, 0, 0},
        {DATANUM(DATA_m642, 44), 1, 0, 0, 0},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_1150 = {
    41, 0, 0, {
        {DATANUM(DATA_m642, 10), 0, 0, 40, -1},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_1258 = {
    41, 0, 0, {
        {DATANUM(DATA_m642, 9), 0, 0, 40, -1},
        {0, 8, 0, -1, -1},
    }
};

M642ResourceTableView lbl_1_data_1360 = {
    41, 0, 0, {
        {DATANUM(DATA_m642, 18), 0, 0, 40, -1},
        {0, 8, 0, -1, -1},
    }
};

void *lbl_1_data_1468[9] = {
    &lbl_1_data_4F0, &lbl_1_data_5F8, &lbl_1_data_808, &lbl_1_data_A18,
    &lbl_1_data_C28, &lbl_1_data_D30, &lbl_1_data_F40, &lbl_1_data_1150,
    &lbl_1_data_1258
};

void *lbl_1_data_148C[9] = {
    &lbl_1_data_4F0, &lbl_1_data_700, &lbl_1_data_910, &lbl_1_data_B20,
    &lbl_1_data_C28, &lbl_1_data_E38, &lbl_1_data_1048, &lbl_1_data_1150,
    &lbl_1_data_1360
};

M642ParticleOwner lbl_1_data_14B0[3] = {
    {0, DATANUM(DATA_m642, 46), 40, 0, 0, NULL, {0}},
    {1, 0, 0, 0, -1, NULL, {0}},
    {0, -1, 0, 0, -1, NULL, {0}},
};

M642ParticleOwner lbl_1_data_1504[3] = {
    {0, DATANUM(DATA_m642, 47), 40, 0, 0, NULL, {0}},
    {1, 0, 0, 0, -1, NULL, {0}},
    {0, -1, 0, 0, -1, NULL, {0}},
};

void *lbl_1_data_1558[1] = { lbl_1_data_14B0 };

void *lbl_1_data_155C[1] = { lbl_1_data_1504 };
#define fn_1_91EC fn_1_91EC
#define fn_1_92D4 fn_1_92D4
#define lbl_1_data_1468 lbl_1_data_1468
#define lbl_1_data_148C lbl_1_data_148C

/* Selects the day or night grid resource table for a scene index. */
void *fn_1_91EC(s32 index)
{
    if (fn_1_92D4() == 0) return lbl_1_data_1468[index];
    return lbl_1_data_148C[index];
}

#define fn_1_9250 fn_1_9250
#define fn_1_92D4 fn_1_92D4
#define lbl_1_data_1558 lbl_1_data_1558
#define lbl_1_data_155C lbl_1_data_155C

/* Selects the day or night particle owner list for a scene index. */
void *fn_1_9250(s32 index)
{
    if (fn_1_92D4() == 0) return lbl_1_data_1558[index];
    return lbl_1_data_155C[index];
}

M642ParticleAnimEntry lbl_1_data_1560[3] = {
    {DATANUM(DATA_mgconst, 28), NULL},
    {DATANUM(DATA_mgconst, 70), NULL},
    {-1, NULL},
};

/* Supplies the shared particle animation resource list during effect setup. */
void *fn_1_92B4(void)
{
    return lbl_1_data_1560;
}

s32 lbl_1_data_1578[5][9][9] = {
    {
        {1, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 0, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 1, 0, 1, 0, 0},
    },
    {
        {1, 0, 1, 0, 0, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 1, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 1},
    },
    {
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 0, 1, 0, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
    },
    {
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 0, 1, 0, 1, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
    },
    {
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {1, 0, 1, 0, 1, 0, 1, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 0, 1, 0, 1, 0, 1, 0, 1},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
        {0, 1, 0, 1, 0, 1, 0, 1, 0},
    },
};

/* Returns the checkerboard pattern used for one Cashapult round. */
void *fn_1_92C0(s32 index)
{
    return lbl_1_data_1578[index];
}

s16 fn_1_92D4(void) { return GwMgNightF; }
