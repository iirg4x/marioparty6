/* Sets up and draws the Banana Shake particle and ground effects. */
#include "dolphin/math.h"

#include "dolphin/gx.h"

#include "dolphin/os.h"

#include "game/main.h"

#include "game/object.h"

#include "game/frand.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

#include "game/data.h"

#include <string.h>

#define M675_LARGE_PARTICLE_RESOURCE FILENUM(DATANUM(DATA_m675, 17))
#define M675_BURST_PARTICLE_RESOURCE FILENUM(DATANUM(DATA_m675, 18))

typedef struct M675EffectRecord {

    s32 active;

    f32 radius;

    Vec *position;

} M675EffectRecord;

typedef struct M675EffectCollection {

    M675EffectRecord *records[5];

    f32 xOffsets[10];

    f32 zOffsets[10];

} M675EffectCollection;

typedef struct M675EffectCollision {
    u32 flags, mask, type;
    s32 index; /* Owner-supplied record index. */
    s32 shapeType; /* Collision shape selector: 0 for sphere, 10 for ring, 11 for boundary. */
    OMOBJ *owner;
    void *excludedOwner;
    OMOBJ_FUNC update;
    s32 (*contact)(struct M675EffectCollision *, struct M675EffectCollision *);
    Vec *position, *velocity;
    Vec positionCopy;
    f32 radius;
    f32 yieldOnContact; /* Nonzero makes this sphere move against a zero-valued sphere. */
    s32 handled;
} M675EffectCollision;

/* Records updated by the three prop emitter managers and queried by computer-player input. */
typedef struct M675EffectRequest {

    s32 type;

    s32 enabled;

    s32 active;

    f32 timer;
    u32 unmappedWordAfterTimer;
    s32 phaseCounter, modelId;

    s32 index;

    Vec position;

    Vec rotation, velocity, rotationDelta;
    f32 scale, verticalOffset, heightLimit;
    Vec sourcePosition, projectedPosition;
    M675EffectCollision collision;

} M675EffectRequest;

/* Transform, motion and sequence state shared by prop emitter managers. */
typedef struct M675EmitterState {
    u8 unmappedBytesBeforePosition[96];
    Vec position, velocity, rotation;
    u8 unmappedBytesBeforeMotion[20];
    s32 motion, motionFrame, requestedMotion;
    f32 motionSpeed;
    s32 phase, phaseTimer;
    f32 countdown, targetAngle;
    Vec targetPosition;
    s32 actionPhase, actionTimer, soundTimer;
    u8 unmappedBytesBeforeMovement[4];
    s32 movementPhase, movementTimer;
    u8 unmappedBytesBeforeMotionTracking[32];
    s32 motionActive, motionTimer;
    u8 unmappedBytesBeforeAirborne[4];
    s32 airborne;
    u8 unmappedBytesBeforePositionSync[28];
    s32 positionSyncEnabled;
    Vec displayOffset;
    f32 movementSoundDistance;
    M675EffectRecord effectRegistration;
    u8 unmappedBytesAfterEffectRegistration[108];
} M675EmitterState;

typedef struct M675PropParameter M675PropParameter;

/* Prop callbacks allocate the record pool; computer-player input queries its active positions. */
typedef struct M675EffectRequestStorage {
    M675EmitterState state;
    M675PropParameter *parameters;
    s32 id, updateCount;
    Vec initialPosition;
    s32 sequencePhase, delay, sequenceTimer;
    u8 unmappedBytesBeforeControl[4];
    s32 controlValue, secondaryControlValue;
    f32 bounceScale, bounceAngle;
    u8 unmappedBytesBeforeRequests[152];
    M675EffectRequest *requests;
    s32 soundHandle, eventCount, shuffleIndex, effectActive;
    f32 effectMotionSpeed; /* Shared main/shape motion playback-speed multiplier. */
    s32 alternateMotionStart;
    u8 shuffledEffectTypes[20];
    Vec displayPosition;
} M675EffectRequestStorage;

typedef struct M675ParticleConfiguration {

    u8 bytes[4];

} M675ParticleConfiguration;

extern OMOBJ *lbl_1_bss_32C;

extern OMOBJ *lbl_1_bss_328;

extern OMOBJ *lbl_1_bss_324;

HUPROCESS *lbl_1_bss_53C;

OMOBJ *lbl_1_bss_538;

OMOBJ **lbl_1_data_848[4] = {

    &lbl_1_bss_32C,

    &lbl_1_bss_328,

    &lbl_1_bss_324,

    NULL,

};

extern OMOBJ **lbl_1_data_834[5];

u8 lbl_1_data_858[8] = { 192, 192, 192, 96, 144, 144, 208, 96 };

extern u8 *fn_1_A38(s32 resourceId);

extern void DCStoreRangeNoSync(void *address, u32 length);

extern void PPCSync(void);

void fn_1_D6F4(OMOBJ *obj);

void fn_1_D708(OMOBJ *object);

void fn_1_D9A4(OMOBJ *object);

void fn_1_D704(OMOBJ *obj);

void fn_1_D840(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_DAC4(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

void fn_1_DCD8(OMOBJ *owner);

void fn_1_DE14(HU3D_MODEL *model, Mtx *view);

void fn_1_E434(Vec *position, Vec *displacement);

/* Finds the lowest active request position for an effect type and request index. */
/* Finds the lowest active request position for an effect type and request index. */
Vec *fn_1_D550(s32 type, s32 index)

{

    OMOBJ *manager;

    M675EffectRequestStorage *storage;

    M675EffectRequest *request;

    Vec *best = NULL;

    s32 requestLimit;

    s32 i;

    if (type < 0) {

        return NULL;

    }

    if ((u32) type >= 3U) {

        return NULL;

    }

    manager = *lbl_1_data_848[type];

    requestLimit = manager->work[0];

    storage = manager->data;

    request = storage->requests;

    for (i = 0; i < requestLimit; i++, request++) {

        if (request->active != 0 && request->enabled != 0 &&

            request->type == type && request->index == index) {

            if (best == NULL) {

                best = &request->position;

            } else if (request->position.y < best->y) {

                best = &request->position;

            }

        }

    }

    return best;

}

void fn_1_DE14(HU3D_MODEL *model, Mtx *view);

/* Creates the shared particle and ground-effect manager during minigame setup. */
void fn_1_D638(HUPROCESS *process)

{

    HUPROCESS *unusedProcessArgument;

    OMOBJ *manager;

    void *configurationData;

    lbl_1_bss_53C = process;

    manager = omAddObjEx(lbl_1_bss_53C, 16, 3U, 0U, -1, fn_1_D6F4);

    lbl_1_bss_538 = manager;

    configurationData = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);

    manager->data = configurationData;

    memset(configurationData, 0, 4);

    fn_1_D708(manager);

    fn_1_D9A4(manager);

    fn_1_DCD8(manager);

}

void fn_1_D6F0(void)

{

}

void fn_1_D6F4(OMOBJ *obj)

{

    obj->objFunc = fn_1_D704;

}

void fn_1_D704(OMOBJ *obj)

{

}

/* Loads the large falling particle effect and installs its frame callback. */
void fn_1_D708(OMOBJ *object)

{

    HU3D_PARTICLE_DATA *entry;

    HU3D_MODELID modelId;

    ANIMDATA *animation;

    HU3D_PARTICLE *particle;

    s32 index;

    M675ParticleConfiguration *configuration;

    configuration = (M675ParticleConfiguration *)object->data;

    *configuration = *(const M675ParticleConfiguration *)lbl_1_data_858;

    animation = HuSprAnimRead(HuDataSelHeapReadNum((s32) fn_1_A38(M675_LARGE_PARTICLE_RESOURCE),
                                                   HU_MEMNUM_OVL, HEAP_MODEL));

    modelId = Hu3DParticleCreate(animation, 128);

    object->mdlId[0] = modelId;

    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;

    Hu3DParticleBlendModeSet(modelId, 0);

    Hu3DParticleHookSet(modelId, fn_1_D840);

    Hu3DModelLayerSet(modelId, 6);

    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);

    entry = particle->data;

    for (index = 0; index < particle->maxCnt; index++, entry++) {

        entry->time = -1;

        entry->scale = 0.0f;

    }

}

/* Advances each active large banana particle during its per-frame update. */
void fn_1_D840(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)

{

    OMOBJ *globalManager = lbl_1_bss_538;

    HU3D_PARTICLE_DATA *record;

    s32 index;

    void *managerData[1];

    managerData[0] = globalManager->data;

    record = particle->data;

    index = 0;

    while (index < particle->maxCnt) {

        if (record->time == 0) {

            record->scale += 3.0f;

            record->color.a -= 2;

            if (record->color.a < 5) {

                record->time = -1;

                record->scale = 0.0f;

            } else {

                record->pos.x += record->accel.x;

                record->pos.y += record->accel.y;

                record->pos.z += record->accel.z;

                record->accel.x *= 0.9;

                record->accel.y -= 0.2;

                record->accel.z *= 0.9;

            }

        }

        index++;

        record++;

    }

    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));

    PPCSync();

}

/* Loads the small burst particle effect and installs its frame callback. */
void fn_1_D9A4(OMOBJ *object)

{

    HU3D_PARTICLE_DATA *entry;

    HU3D_MODELID modelId;

    ANIMDATA *animation;

    HU3D_PARTICLE *particle;

    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum((s32) fn_1_A38(M675_BURST_PARTICLE_RESOURCE),
                                                   HU_MEMNUM_OVL, HEAP_MODEL));

    modelId = Hu3DParticleCreate(animation, 64);

    object->mdlId[1] = modelId;

    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;

    Hu3DParticleColSet(modelId, 218, 203, 158);

    Hu3DParticleBlendModeSet(modelId, 0);

    Hu3DParticleHookSet(modelId, fn_1_DAC4);

    Hu3DModelLayerSet(modelId, 6);

    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);

    entry = particle->data;

    for (index = 0; index < particle->maxCnt; index++, entry++) {

        entry->time = -1;

        entry->scale = 0.0f;

    }

}

/* Applies gravity, motion and fading to the small burst particles each frame. */
void fn_1_DAC4(HU3D_MODEL *model, HU3D_PARTICLE *manager, Mtx matrix)

{

    HU3D_PARTICLE_DATA *particle = manager->data;

    s32 index;

    for (index = 0; index < manager->maxCnt; index++, particle++) {

        switch (particle->time) {

        case 0:

            particle->time++;

            particle->scale = (f32)(10.0 + 10.0 *

                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));

            particle->zRot = 0.0f;

            particle->color.a = 180;

            /* The new particle starts moving and fading in this same update. */

        case 1:

            particle->scale += 4.0;

            particle->zRot += 0.008726646192371845;

            particle->color.a -= 4;

            /* Alpha fades through zero, then wraps above 240 on the next subtraction to retire the
             * particle. */
            if (particle->pos.y < -100.0 || particle->scale <= 0.0f ||

                particle->color.a > 240) {

                particle->time = -1;

                particle->scale = 0.0f;

            } else {

                PSVECAdd(&particle->pos, &particle->accel, &particle->pos);

                particle->accel.x *= 0.95;

                particle->accel.y -= 0.05;

                particle->accel.z *= 0.95;

            }

            break;

        }

    }

    DCStoreRangeNoSync(manager->data, manager->maxCnt * sizeof(HU3D_PARTICLE_DATA));

    PPCSync();

}

/* Creates the ground-ring model and caches its ten evenly spaced vertices. */
void fn_1_DCD8(OMOBJ *owner)

{

    M675EffectCollection *hookData;

    HU3D_MODEL *modelData;

    s16 modelId;

    f32 angle;

    s32 i;

    modelId = Hu3DHookFuncCreate(fn_1_DE14);

    owner->mdlId[2] = modelId;

    Hu3DModelLayerSet(modelId, 4);

    modelData = &Hu3DData[modelId];

    hookData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*hookData), HU_MEMNUM_OVL);

    modelData->hookData = hookData;

    memset(hookData, 0, sizeof(*hookData));

    i = 0;

    angle = 0.0f;

    while (i < 10) {

        hookData->xOffsets[i] = (f32) sin((3.141592653589793 * (f64) angle) / 180.0);

        hookData->zOffsets[i] = (f32) cos((3.141592653589793 * (f64) angle) / 180.0);

        i++;

        angle += 36.0f;

    }

}

/* Draws active ground rings for registered effect positions on the render pass. */
void fn_1_DE14(HU3D_MODEL *model, Mtx *view)

{

    M675EffectCollection *effects = model->hookData;

    Mtx matrix;

    M675EffectRecord *record;

    f32 *xOffsets;

    f32 *zOffsets;

    s32 i;

    s32 vertex;

    f32 radius;

    xOffsets = effects->xOffsets;

    zOffsets = effects->zOffsets;

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

    for (i = 0; i < 5; i++) {

        record = effects->records[i];

        if (record != NULL && record->active != 0) {

            radius = record->radius;

            PSMTXTrans(matrix, record->position->x,

                3.0 + record->position->y,

                record->position->z);

            PSMTXConcat(*view, matrix, matrix);

            GXLoadPosMtxImm(matrix, GX_PNMTX0);

            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 12);

            GXPosition3f32(0.0f, 0.0f, 0.0f);

            GXColor1u32(128);

            for (vertex = 0; vertex < 10; vertex++) {

                GXPosition3f32(xOffsets[vertex] * radius,

                    0.0f, zOffsets[vertex] * radius);

                GXColor1u32(24);

            }

            GXPosition3f32(xOffsets[0] * radius,

                0.0f, zOffsets[0] * radius);

            GXColor1u32(48);

        }

    }

}

/* Starts one large particle at a supplied effect position when a slot is free. */
void fn_1_E0F0(Vec *origin)

{

    OMOBJ *manager = lbl_1_bss_538;

    GXColor *color = manager->data;

    HU3D_MODELID modelId = manager->mdlId[0];

    HU3D_PARTICLE *work = Hu3DData[modelId].hookData;

    HU3D_PARTICLE_DATA *slot = work->data;

    s32 index = 0;

    while (index < work->maxCnt) {

        if (slot->time < 0) {

            f32 randomX;

            f32 randomZ;

            slot->time = 0;

        randomX = (f32)(50.0 * (0.000030517578125 * (f32)((u16)frand() - 32768)));

            randomZ = (f32)(50.0 * (0.000030517578125 * (f32)((u16)frand() - 32768)));

            slot->pos.x = origin->x + randomX;

            slot->pos.y =
                (f32) (origin->y + (20.0 * (1.52587890625e-05 * (f32) (s32) (frand() & 0xFFFF))));

            slot->pos.z = origin->z + randomZ;

            slot->accel.x = (f32)(0.05 * (f64)randomX);

            slot->accel.y =
                (f32) (3.0 + (0.2 * (1.52587890625e-05 * (f64) (f32) (s32) (frand() & 0xFFFF))));

            slot->accel.z = (f32)(0.05 * (f64)randomZ);

            slot->scale =
                (f32) (20.0 + (10.0 * (1.52587890625e-05 * (f64) (f32) (s32) (frand() & 0xFFFF))));

            slot->zRot = (f32)(360.0 * (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));

            slot->color = *color;

            return;

        }

        index++;

        slot++;

    }

}

/* Gives a free burst particle its initial position and per-frame displacement. */
void fn_1_E434(Vec *position, Vec *displacement)

{

    OMOBJ *object = lbl_1_bss_538;

    HU3D_MODELID modelId = object->mdlId[1];

    HU3D_PARTICLE *work = Hu3DData[modelId].hookData;

    HU3D_PARTICLE_DATA *record = work->data;

    s32 index;

    for (index = 0; index < work->maxCnt; index++, record++) {

        if (record->time < 0) {

            record->time = 0;

            record->pos = *position;

            record->accel = *displacement;

            break;

        }

    }

}

/* Emits a randomized ring of small particles around a point during an effect. */
void fn_1_E4F8(const Vec *position, f32 radius, s32 count)

{

    s32 index;

    for (index = 0; index < count; index++) {

        Vec start;

        Vec velocity;

        f32 sine;

        f32 cosine;

        f32 angle;

        angle = (f32)(360.0 * (1.52587890625e-05 * (f32)(s32)(frand() & 0xFFFF)));

        sine = sin((3.141592653589793 * angle) / 180.0);

        cosine = cos((3.141592653589793 * angle) / 180.0);

        start.x = (f32)(position->x + (sine * radius) +

            (10.0 * (0.000030517578125 * (f32)((u16)frand() - 32768))));

        start.y = (f32)(position->y + (3.0 *

            (1.52587890625e-05 * (f32)(s32)(frand() & 0xFFFF))) + 3.0);

        start.z = (f32)(position->z + (cosine * radius) +

            (10.0 * (0.000030517578125 * (f32)((u16)frand() - 32768))));

        velocity.x = (f32)((2.0 * (f64)sine) +

            (0.000030517578125 * (f32)((u16)frand() - 32768)));

        velocity.y = (f32)(0.3 + (0.3 *

            (1.52587890625e-05 * (f32)(s32)(frand() & 0xFFFF))));

        velocity.z = (f32)((2.0 * (f64)cosine) +

            (0.000030517578125 * (f32)((u16)frand() - 32768)));

        fn_1_E434(&start, &velocity);

    }

}

/* Player and prop setup register a live position/radius record in the first free ground-effect
 * slot. */
void fn_1_E918(M675EffectRecord *effect)

{

    M675EffectRecord **entry;

    s32 index;

    OMOBJ *object = lbl_1_bss_538;

    HU3D_MODEL *model;

    s16 modelId;

    M675EffectCollection *work;

    modelId = object->mdlId[2];

    model = &Hu3DData[modelId];

    work = model->hookData;

    entry = work->records;

    for (index = 0; index < 5; index++, entry++) {

        if (*entry == NULL) {

            *entry = effect;

            break;

        }

    }

}
