/* Creates and updates the particle, ripple, and weather effects used during play. */
#include "dolphin/math.h"
#include "dolphin/PPCArch.h"
#include "dolphin/os/OSCache.h"
#include "REL/m626Dll/effects.h"
#include "game/memory.h"
#include "game/data.h"
#include "game/sprite.h"
#include "game/gamework.h"
#include <string.h>

#define M626_RANDOM_SIGNED_OFFSET 32768
#define M626_PARTICLE_RECORD_SIZE 72
#define M626_EFFECT_FAN_CENTER_COLOR 32896
#define M626_EFFECT_FAN_VERTEX_COLOR 32776
#define M626_SPARK_EFFECT_DATA_SIZE 16388
#define M626_SPARK_INITIAL_PARTICLE_COUNT 256
#define M626_SPARK_PARTICLE_COUNT 512
#define M626_SPARK_PARTICLE_LIFETIME 10
#define M626_ROTATION_AXIS_Z 'z'
#define M626_SPARK_CENTER_COLOR_BIT_PATTERN 0xFFFFFFFFU
#define M626_SPARK_VERTEX_COLOR_BIT_PATTERN 0xFFFFFF00U

extern s32 frand(void);
s32 fn_1_830(s32 resourceId);
void fn_1_BA34(HUPROCESS *process);
void fn_1_BADC(void);
void fn_1_BAE0(OMOBJ *object);
void fn_1_BB3C(OMOBJ *object);
void fn_1_BB74(OMOBJ *object);
void fn_1_BC80(HU3D_MODEL *context, HU3D_PARTICLE *system, Mtx matrix);
void fn_1_BE74(OMOBJ *object);
void fn_1_BF80(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);
void fn_1_C0FC(OMOBJ *object);
void fn_1_C278(HU3D_MODEL *unusedArgument, HU3D_PARTICLE *system, Mtx matrix);
void fn_1_C5D0(OMOBJ *object);
void fn_1_C70C(OMOBJ *object);
void fn_1_C8B0(HU3D_MODEL *model, Mtx *view);
void fn_1_CB70(OMOBJ *object);
void fn_1_CCAC(HU3D_MODEL *model, Mtx *view);
void fn_1_CF84(OMOBJ *object);
void fn_1_D1D8(OMOBJ *object);
void fn_1_D7AC(HU3D_MODEL *model, Mtx *view);
void fn_1_DBC0(Vec *position, Vec *velocity);
void fn_1_DC84(Vec *position, Vec *velocity);
void fn_1_DD48(s32 count, f32 radius, const Vec *position);
void fn_1_E068(const Vec *position);
void fn_1_E114(const Vec *position, f32 fanRadius);
void fn_1_E1D8(M626EffectRecord *effect);

/* Creates the effect manager and renderer when the scene starts its effect process. */
void fn_1_BA34(HUPROCESS *process) {
    HUPROCESS *unusedProcess;
    OMOBJ *manager;
    GXColor *weatherColor;
    lbl_1_bss_6CC = process;
    manager = omAddObjEx(lbl_1_bss_6CC, 16, 6, 0, -1, fn_1_BAE0);
    lbl_1_bss_6C8 = manager;
    weatherColor = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);
    manager->data = weatherColor;
    memset(weatherColor, 0, 4);
    fn_1_CB70(manager);
}

void fn_1_BADC(void)
{
}

/* Builds the effect models once, then switches the manager to its frame callback. */
void fn_1_BAE0(OMOBJ *object) {
    fn_1_BB74(object);
    fn_1_BE74(object);
    fn_1_C0FC(object);
    fn_1_C5D0(object);
    fn_1_CF84(object);
    object->objFunc = fn_1_BB3C;
}

/* Updates active ripples and spark particles from the effect manager each frame. */
void fn_1_BB3C(OMOBJ *object)
{
    fn_1_C70C(object);
    fn_1_D1D8(object);
}

/* Creates the snow particle system when the effect manager is initialized. */
void fn_1_BB74(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_830(6), HU_MEMNUM_OVL,
                                                    HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 256);
    object->mdlId[0] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_BC80);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* Advances and fades the snow particles from the particle draw hook each frame. */
void fn_1_BC80(HU3D_MODEL *context, HU3D_PARTICLE *system, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particle;
    s32 index;
    s32 alpha;

    particle = system->data;
    index = 0;
    while (index < system->maxCnt) {
        switch (particle->time) {
        case 0:
            particle->time++;
            particle->vel.x = (f32)(3.0517578125e-05 *
                (f64)(f32)((frand() & 0xFFFF) - 32768));
            particle->vel.y = (f32)(2.0 +
                (1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF)));
            particle->vel.z = (f32)(3.0517578125e-05 *
                (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET));
            particle->color.r = 192;
            particle->color.g = 192;
            particle->color.b = 224;
            particle->color.a = 48;
            particle->scale = 50.0f;
            /* fall through */
        case 1:
            PSVECAdd(&particle->vel, &particle->pos,
                     &particle->pos);
            alpha = particle->color.a;
            alpha -= 4;
            if (alpha < 0) {
                alpha = 0;
                particle->scale = 0.0f;
                particle->time = -1;
            }
            particle->color.a = (u8)alpha;
            break;
        }
        index++;
        particle++;
    }
    DCStoreRangeNoSync(system->data, system->maxCnt * M626_PARTICLE_RECORD_SIZE);
    PPCSync();
}

/* Creates the second snow particle system when the effect manager is initialized. */
void fn_1_BE74(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_830(6), HU_MEMNUM_OVL,
                                                    HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 320);
    object->mdlId[1] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_BF80);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* Moves and fades the larger snow particles from their draw hook each frame. */
void fn_1_BF80(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry;
    s32 index;
    s32 alpha;

    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        switch (entry->time) {
        case 0:
            entry->time++;
            entry->color.r = 255;
            entry->color.g = 255;
            entry->color.b = 255;
            entry->color.a = 96;
            entry->scale = 60.0 +
                30.0 * (1.52587890625e-05 *
                    (f64)(f32)(s32)(frand() & 0xFFFF));
            /* fall through */
        case 1:
            PSVECAdd(&entry->pos, &entry->accel, &entry->pos);
            PSVECScale(&entry->accel, &entry->accel, 0.99f);
            alpha = entry->color.a;
            alpha -= 6;
            if (alpha < 0) {
                alpha = 0;
                entry->scale = 0.0f;
                entry->time = -1;
            }
            entry->color.a = (u8)alpha;
            break;
        }
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * M626_PARTICLE_RECORD_SIZE);
    PPCSync();
}

/* Selects the night tint and creates the weather particle system at setup. */
void fn_1_C0FC(OMOBJ *object)
{
    ANIMDATA *animation;
    s16 night;
    GXColor *configuration;
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    HU3D_PARTICLE *particle;
    s32 index;

    configuration = object->data;
    night = GwMgNightF;
    if (night != 1) {
        *configuration = lbl_1_data_8C0[0];
    } else {
        *configuration = lbl_1_data_8C0[1];
    }
    animation = HuSprAnimRead(
        HuDataSelHeapReadNum(fn_1_830(7), HU_MEMNUM_OVL, HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 128);
    object->mdlId[2] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_C278);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f,
                    0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* Seeds weather particles and advances their tint, height, and drift each frame. */
void fn_1_C278(HU3D_MODEL *unusedModel, HU3D_PARTICLE *system, Mtx matrix)
{
    OMOBJ *emitter = lbl_1_bss_6C8;
    GXColor *color = emitter->data;
    HU3D_PARTICLE_DATA *record = system->data;
    s32 index = 0;

    while (index < system->maxCnt) {
        switch (record->time) {
        case 0: {
            record->time++;
            record->accel.x = (f32)((3.0517578125e-05 *
                                      (f32)((frand() & 0xFFFF) - 32768)) *
                                      0.6);

            record->accel.y = (f32)((1.52587890625e-05 *
                                      (f32)(frand() & 0xFFFF)) *
                                      0.2 + 2.0);

            record->accel.z = (f32)((3.0517578125e-05 *
                                      (f32)((frand() & 0xFFFF) - 32768)) *
                                      0.6);

            record->scale = (f32)((10.0 *
                                (1.52587890625e-05 *
                                 (f32)(frand() & 0xFFFF))) +
                                10.0);

            record->zRot = (f32)((1.52587890625e-05 *
                                     (f32)(frand() & 0xFFFF)) *
                                     360.0);
            record->color = *color;
            break;
        }
        case 1:
            break;
        default:
            goto next_record;
        }

        record->scale += 4.0f;
        record->color.a -= 4;
        if (record->color.a > 240) {
            record->time = -1;
            record->scale = 0.0f;
            goto next_record;
        }

        record->pos.x += record->accel.x;
        record->pos.y += record->accel.y;
        record->pos.z += record->accel.z;
        record->accel.x *= 0.9;
        record->accel.y -= 0.05;
        record->accel.z *= 0.9;

    next_record:
        index++;
        record++;
    }
    DCStoreRangeNoSync(system->data, system->maxCnt * sizeof(*record));
    PPCSync();
}

/* Creates the ripple renderer and its ten-angle lookup table at setup. */
void fn_1_C5D0(OMOBJ *object)
{
    M626RippleWork *hookData;
    HU3D_MODEL *modelData;
    s16 modelId;
    f32 angle;
    s32 i;

    modelId = Hu3DHookFuncCreate(fn_1_C8B0);
    object->mdlId[3] = modelId;
    Hu3DModelLayerSet(modelId, 3);
    modelData = &Hu3DData[modelId];
    hookData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*hookData), HU_MEMNUM_OVL);
    modelData->hookData = hookData;
    memset(hookData, 0, sizeof(*hookData));

    i = 0;
    angle = 0.0f;
    while (i < 10) {
        hookData->sineTable[i] = (f32)sin((3.141592653589793 * (f64)angle) / 180.0);
        hookData->cosineTable[i] = (f32)cos((3.141592653589793 * (f64)angle) / 180.0);
        i++;
        angle += 36.0f;
    }
}

/* Advances active ground ripples from the effect manager's per-frame callback. */
void fn_1_C70C(OMOBJ *object)
{
    s16 modelId;
    M626RippleWork *hookData;
    HU3D_MODEL *modelData;
    M626RippleRecord *record;
    s8 *channels;
    s32 activeCount;
    s32 index;

    modelId = object->mdlId[3];
    modelData = &Hu3DData[modelId];
    hookData = modelData->hookData;
    record = hookData->records;
    activeCount = 0;
    index = 0;
    while (index < 640) {
        switch (record->active) {
        case 0:
            break;
        case 1:
            record->active++;
            record->color.value = 65280;
            record->radius = 0.0f;
            /* fall through */
        case 2:
            record->radius += 0.2;
            channels = record->color.components;
            channels[3] = (s8)(16.0f * record->radius);
            if (!(record->radius < 1.0)) {
                record->active++;
            }
            break;
        case 3:
            record->radius = (f32)((f64)record->radius -
                                   0.0020833333333333333);
            channels = record->color.components;
            channels[3] = (s8)(16.0f * record->radius);
            if (!(record->radius > 0.0)) {
                record->active = 0;
            }
            break;
        }
        if (record->active != 0) {
            activeCount++;
        }
        index++;
        record++;
    }
}

/* Draws active ripple fans from the model hook during the scene render. */
void fn_1_C8B0(HU3D_MODEL *model, Mtx *view)
{
    M626RippleWork *hookData = model->hookData;
    Mtx matrix;
    M626RippleRecord *record;
    f32 *xOffsets;
    f32 *zOffsets;
    s32 i;
    s32 vertex;
    f32 radius;
    u32 color;

    record = hookData->records;
    xOffsets = hookData->sineTable;
    zOffsets = hookData->cosineTable;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 255);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    for (i = 0; i < 640; i++, record++) {
        if (record->active >= 2) {
            radius = record->fanRadius;
            color = record->color.value;
            PSMTXTrans(matrix, record->position.x, record->position.y, record->position.z);
            PSMTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 12);
            GXPosition3f32(0.0f, 0.0f, 0.0f);
            GXColor1u32(color);
            for (vertex = 0; vertex < 10; vertex++) {
                GXPosition3f32(xOffsets[vertex] * radius, 0.0f,
                               zOffsets[vertex] * radius);
                GXColor1u32(color);
            }
            GXPosition3f32(xOffsets[0] * radius, 0.0f,
                           zOffsets[0] * radius);
            GXColor1u32(color);
        }
    }
}

/* Creates the queued effect renderer and its ten-angle lookup table at setup. */
void fn_1_CB70(OMOBJ *object)
{
    M626EffectCollection *hookData;
    HU3D_MODEL *modelData;
    s16 modelId;
    f32 angle;
    s32 i;

    modelId = Hu3DHookFuncCreate(fn_1_CCAC);
    object->mdlId[4] = modelId;
    Hu3DModelLayerSet(modelId, 4);
    modelData = &Hu3DData[modelId];
    hookData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*hookData), HU_MEMNUM_OVL);
    modelData->hookData = hookData;
    memset(hookData, 0, sizeof(*hookData));

    i = 0;
    angle = 0.0f;
    while (i < 10) {
        hookData->sineTable[i] = (f32)sin((3.141592653589793 * (f64)angle) / 180.0);
        hookData->cosineTable[i] = (f32)cos((3.141592653589793 * (f64)angle) / 180.0);
        i++;
        angle += 36.0f;
    }
}

/* Draws active queued effect fans from the renderer's model hook. */
void fn_1_CCAC(HU3D_MODEL *model, Mtx *view)
{
    M626EffectCollection *effects = model->hookData;
    Mtx matrix;
    M626EffectRecord *record;
    f32 *xOffsets;
    f32 *zOffsets;
    s32 i;
    s32 vertex;
    f32 radius;

    xOffsets = effects->sineTable;
    zOffsets = effects->cosineTable;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    for (i = 0; i < 30; i++) {
        record = effects->records[i];
        if (record != NULL && record->active != 0) {
            /* These fans draw at Y=3 regardless of the tracked position's height. */
            radius = record->radius;
            PSMTXTrans(matrix, record->position->x, 3.0f,
                       record->position->z);
            PSMTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 12);
            GXPosition3f32(0.0f, 0.0f, 0.0f);
            GXColor1u32(M626_EFFECT_FAN_CENTER_COLOR);
            for (vertex = 0; vertex < 10; vertex++) {
                GXPosition3f32(xOffsets[vertex] * radius, 0.0f,
                               zOffsets[vertex] * radius);
                GXColor1u32(M626_EFFECT_FAN_VERTEX_COLOR);
            }
            GXPosition3f32(xOffsets[0] * radius, 0.0f,
                           zOffsets[0] * radius);
            GXColor1u32(M626_EFFECT_FAN_VERTEX_COLOR);
        }
    }
}

/* Allocates and seeds the spark field when the effect manager is initialized. */
void fn_1_CF84(OMOBJ *object)
{
    M626SparkParticle *particles;
    s32 index;
    HU3D_MODEL *modelData;
    HU3D_MODELID modelId;
    M626SparkParticle *particle;
    Vec *position;
    Vec *velocity;

    modelId = Hu3DHookFuncCreate(fn_1_D7AC);
    object->mdlId[5] = modelId;
    Hu3DModelLayerSet(modelId, 7);
    modelData = &Hu3DData[modelId];
    particles = HuMemDirectMallocNum(HEAP_HEAP, M626_SPARK_EFFECT_DATA_SIZE, HU_MEMNUM_OVL);
    modelData->hookData = particles;
    memset(particles, 0, M626_SPARK_EFFECT_DATA_SIZE);
    particle = particles;
    for (index = 0; index < M626_SPARK_INITIAL_PARTICLE_COUNT; index++, particle++) {
        particle->active = 1;
        particle->life = M626_SPARK_PARTICLE_LIFETIME;
        position = &particle->position;
        velocity = &particle->velocity;
        position->x = (f32)(1200.0 *
            (3.0517578125e-05 *
                (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET)));
        position->y = (f32)(100.0 +
            (1000.0 * (1.52587890625e-05 *
                (f64)(f32)(frand() & 0xFFFF))));
        position->z = (f32)(1000.0 *
            (3.0517578125e-05 *
                (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET)));
        velocity->x = 0.0f;
        velocity->y = (f32)(-(3.0 *
            (1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF))));
        velocity->z = 0.0f;
    }
}

/* Updates spark positions and lifetimes from the manager callback each frame. */
void fn_1_D1D8(OMOBJ *object)
{
    HU3D_MODEL *modelData;
    M626SparkEffectData *effects;
    M626SparkParticle *particle;
    Vec *position;
    Vec *velocity;
    s16 modelId = object->mdlId[5];
    s32 index;
    s32 initialized;
    s32 updated;
    f32 x;
    f32 z;
    f32 angle;
    f32 magnitude;

    modelData = &Hu3DData[modelId];
    effects = modelData->hookData;
    particle = effects->particles;
    initialized = 0;
    updated = 0;
    effects->angle += 3.0;
    for (index = 0; index < M626_SPARK_PARTICLE_COUNT; index++, particle++) {
        position = &particle->position;
        velocity = &particle->velocity;
        if (particle->active == 0) {
            if (initialized > 3) {
                continue;
            }
            particle->active = 1;
            particle->life = M626_SPARK_PARTICLE_LIFETIME;
            position->x = (f32)(1200.0 *
                (3.0517578125e-05 * (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET)));
            position->y = 1000.0f;
            position->z = (f32)(1000.0 *
                (3.0517578125e-05 * (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET)));
            velocity->x = 0.0f;
            velocity->y = (f32)(-(5.0 *
                (1.52587890625e-05 * (f64)(f32)(frand() & 0xFFFF))) -
                5.0);
            velocity->z = 0.0f;
            initialized++;
        }

        if (position->y < 0.0f ||
            position->y > 1010.0) {
            particle->active = 0;
        } else {
            if ((particle->life = particle->life - 1) == 0) {
                particle->life = 4;
                if (position->y > 500.0) {
                    velocity->y *= 0.999f;
                } else {
                    velocity->y = (f32)((f64)velocity->y +
                        (0.2 * (3.0517578125e-05 *
                         (f64)(f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET)) -
                         0.1));
                }
            }

            x = position->x;
            z = position->z;
            angle = (f32)(180.0 *
                (atan2((f64)x, (f64)z) / 3.141592653589793));
            magnitude = sqrtf((x * x) + (z * z));
            angle += 85.0;
            magnitude *= 0.005;
            velocity->x = (f32)((f64)magnitude *
                sin((3.141592653589793 * (f64)angle) / 180.0));
            velocity->z = (f32)((f64)magnitude *
                cos((3.141592653589793 * (f64)angle) / 180.0));
            PSVECAdd(position, velocity, position);
            if (position->x > 1200.0) {
                position->x = -position->x;
            }
            if (position->x < -1200.0) {
                position->x = -position->x;
            }
            if (position->z > 1000.0) {
                position->z = -position->z;
            }
            if (position->z < -1000.0) {
                position->z = -position->z;
            }
            updated++;
        }
    }
}

/* Draws the active spark field from the custom model hook during rendering. */
void fn_1_D7AC(HU3D_MODEL *model, Mtx *view)
{
    M626SparkEffectData *effects = model->hookData;
    M626SparkParticle *particle;
    Point3d rotated[5];
    Mtx rotation;
    f32 baseX;
    f32 baseY;
    f32 baseZ;
    Vec *position;
    u32 centerColor;
    u32 vertexColor;
    s32 i;

    particle = effects->particles;
    centerColor = M626_SPARK_CENTER_COLOR_BIT_PATTERN;
    vertexColor = M626_SPARK_VERTEX_COLOR_BIT_PATTERN;
    GXLoadPosMtxImm(*view, GX_PNMTX0);
    PSMTXRotRad(rotation, M626_ROTATION_AXIS_Z, 0.017453292f * effects->angle);
    PSMTXMultVecArray(rotation, lbl_1_data_8C8, rotated, 5);

    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);

    for (i = 0; i < M626_SPARK_PARTICLE_COUNT; i++, particle++) {
        if (particle->active != 0) {
            position = &particle->position;
            baseX = position->x;
            baseY = position->y;
            baseZ = position->z;
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 7);
            GXPosition3f32(baseX, baseY, baseZ);
            GXColor1u32(centerColor);
            GXPosition3f32(baseX + rotated[0].x, baseY + rotated[0].y,
                           baseZ + rotated[0].z);
            GXColor1u32(vertexColor);
            GXPosition3f32(baseX + rotated[1].x, baseY + rotated[1].y,
                           baseZ + rotated[1].z);
            GXColor1u32(vertexColor);
            GXPosition3f32(baseX + rotated[2].x, baseY + rotated[2].y,
                           baseZ + rotated[2].z);
            GXColor1u32(vertexColor);
            GXPosition3f32(baseX + rotated[3].x, baseY + rotated[3].y,
                           baseZ + rotated[3].z);
            GXColor1u32(vertexColor);
            GXPosition3f32(baseX + rotated[4].x, baseY + rotated[4].y,
                           baseZ + rotated[4].z);
            GXColor1u32(vertexColor);
            GXPosition3f32(baseX + rotated[0].x, baseY + rotated[0].y,
                           baseZ + rotated[0].z);
            GXColor1u32(vertexColor);
        }
    }
}

/* Adds a snow particle at the supplied position with the supplied velocity. */
void fn_1_DBC0(Vec *position, Vec *velocity)
{
    OMOBJ *object;
    HU3D_MODELID modelId;
    HU3D_PARTICLE *work;
    HU3D_PARTICLE_DATA *record;
    s32 index;

    object = lbl_1_bss_6C8;
    modelId = object->mdlId[0];
    work = Hu3DData[modelId].hookData;
    record = work->data;
    for (index = 0; index < work->maxCnt; index++, record++) {
        if (record->time < 0) {
            record->time = 0;
            record->pos = *position;
            record->accel = *velocity;
            break;
        }
    }
}

/* Adds a large snow particle at the supplied position with its initial velocity. */
void fn_1_DC84(Vec *position, Vec *velocity)
{
    OMOBJ *object;
    HU3D_MODELID modelId;
    HU3D_PARTICLE *work;
    HU3D_PARTICLE_DATA *record;
    s32 index;

    object = lbl_1_bss_6C8;
    modelId = object->mdlId[1];
    work = Hu3DData[modelId].hookData;
    record = work->data;
    for (index = 0; index < work->maxCnt; index++, record++) {
        if (record->time < 0) {
            record->time = 0;
            record->pos = *position;
            record->accel = *velocity;
            break;
        }
    }
}

/* Emits snow around a position when player or scenery code requests a burst. */
void fn_1_DD48(s32 count, f32 radius, const Vec *position)
{
    s32 index;

    for (index = 0; index < count; index++) {
        Vec start;
        Vec velocity;
        s32 slot;
        OMOBJ *manager;
        s16 modelIndex;
        HU3D_PARTICLE *model;
        HU3D_PARTICLE_DATA *particle;

        start.x = (f32)((f64)position->x + radius *
            ((f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET) * 3.0517578125e-05));
        start.y = (f32)((f64)position->y +
            (f32)(frand() & 0xFFFF) * 1.52587890625e-05 * 20.0 +
            10.0);
        start.z = (f32)((f64)position->z + radius *
            ((f32)((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET) * 3.0517578125e-05));
        velocity.x =
            (f32) ((f32) ((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET) * 3.0517578125e-05 * 3.0);
        velocity.y = (f32)((f32)(frand() & 0xFFFF) * 1.52587890625e-05 *
                           3.0 + 2.0);
        velocity.z =
            (f32) ((f32) ((frand() & 0xFFFF) - M626_RANDOM_SIGNED_OFFSET) * 3.0517578125e-05 * 3.0);

        manager = lbl_1_bss_6C8;
        modelIndex = manager->mdlId[1];
        model = Hu3DData[modelIndex].hookData;
        particle = model->data;

        for (slot = 0; slot < model->maxCnt; slot++, particle++) {
            if (particle->time < 0) {
                particle->time = 0;
                particle->pos = start;
                particle->accel = velocity;
                break;
            }
        }
    }
}

/* Queues one weather particle at the supplied position when scenery emits it. */
void fn_1_E068(const Vec *position)
{
    OMOBJ *object = lbl_1_bss_6C8;
    s16 modelId = object->mdlId[2];
    HU3D_PARTICLE *work = Hu3DData[modelId].hookData;
    HU3D_PARTICLE_DATA *record = work->data;
    s32 index;

    for (index = 0; index < work->maxCnt; index++, record++) {
        if (record->time < 0) {
            record->time = 0;
            record->pos = *position;
            return;
        }
    }
}

/* Starts a ground ripple at the supplied position with its requested fan radius. */
void fn_1_E114(const Vec *position, f32 fanRadius)
{
    OMOBJ *boardState = lbl_1_bss_6C8;
    s16 modelId;
    HU3D_MODEL *model;
    M626RippleRecord *records;
    M626RippleRecord *record;
    s32 index;

    modelId = boardState->mdlId[3];
    model = &Hu3DData[modelId];
    records = model->hookData;
    record = records;

    for (index = 0; index < 640; index++, record++) {
        if (record->active == 0) {
            record->active++;
            record->position.x = position->x;
            record->position.y = 3.0 + position->y;
            record->position.z = position->z;
            record->fanRadius = fanRadius;
            return;
        }
    }
}

/* Adds an effect record to the renderer queue when scenery activates it. */
void fn_1_E1D8(M626EffectRecord *effect)
{
    OMOBJ *object = lbl_1_bss_6C8;
    s16 modelId = object->mdlId[4];
    HU3D_MODEL *model = &Hu3DData[modelId];
    M626EffectCollection *work = model->hookData;
    M626EffectRecord **entry = work->records;
    s32 i;

    for (i = 0; i < 30; i++, entry++) {
        if (*entry == NULL) {
            *entry = effect;
            break;
        }
    }
}

HUPROCESS *lbl_1_bss_6CC;
OMOBJ *lbl_1_bss_6C8;
GXColor lbl_1_data_8C0[2] = {{255, 255, 255, 128}, {144, 144, 208, 128}};
Vec lbl_1_data_8C8[5] = {
    {0.0f, 10.0f, 0.0f},
    {9.51056957f, 3.09016991f, 0.0f},
    {5.87785006f, -8.09016991f, 0.0f},
    {-5.87785006f, -8.09016991f, 0.0f},
    {-9.51056957f, 3.09016991f, 0.0f},
};
