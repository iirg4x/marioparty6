/* Trap Ease Artist particle effects, path trails, and render callbacks. */
#include "dolphin/math.h"

#include "dolphin/types.h"

#include "dolphin/os.h"

#include "dolphin/mtx.h"

#include "dolphin/PPCArch.h"

#include "dolphin/os/OSCache.h"

#include "dolphin/gx.h"

#include "game/main.h"

#include "game/object.h"

#include "game/charman.h"

#include "game/hu3d.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/memory.h"

#include "game/mg/timer.h"

#include "game/frand.h"

#include "game/data.h"

#include "game/sprite.h"

#include "REL/m620Dll/module_types.h"

typedef struct Position {
    /* Position and offset vectors; these helpers do not inspect the intervening bytes. */
    HuVecF position;
    unsigned char unreadBeforeOffset[12];
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern s8 lbl_1_bss_218;

extern s8 lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void fn_1_B898(OMOBJ *obj);

void fn_1_C458(OMOBJ *object);

void fn_1_B8D0(OMOBJ *object);

void fn_1_BC04(OMOBJ *object);

void fn_1_BF14(OMOBJ *obj);

void fn_1_C358(OMOBJ *object);

extern OMOBJ *lbl_1_bss_650;

extern HUPROCESS *lbl_1_bss_654;

/* Called by fn_1_A0 to create the particle, ring, and trail effect models. */
void fn_1_B7D4(HUPROCESS *managerProcess)
{
    HUPROCESS *unusedProcess;
    OMOBJ *effectObject;
    void *objectData;

    lbl_1_bss_654 = managerProcess;
    effectObject = omAddObjEx(lbl_1_bss_654, 16, 5U, 0U, -1, fn_1_B898);
    lbl_1_bss_650 = effectObject;
    objectData = HuMemDirectMallocNum(HEAP_HEAP, 4, HU_MEMNUM_OVL);
    effectObject->data = objectData;
    memset(objectData, 0, 4);
    fn_1_B8D0(effectObject);
    fn_1_BC04(effectObject);
    fn_1_BF14(effectObject);
    fn_1_C358(effectObject);
}

void fn_1_B894(void)
{
}

void fn_1_B8A8(OMOBJ *obj);

void fn_1_B898(OMOBJ *obj)
{
    obj->objFunc = fn_1_B8A8;
}

void fn_1_B8A8(OMOBJ *object)
{
    fn_1_C458(object);
}

typedef struct M620Particle {
    s16 state; /* -1 is free, 0 starts the particle, and 1 updates it. */
    u8 unreadBeforeVelocity[18]; /* Other particle attributes are not used here. */
    Vec velocity; /* World-space displacement added each particle update. */
    u8 unreadBeforeScale[12]; /* Other particle attributes are not used here. */
    f32 scale; /* Sprite size, increased by four on each update. */
    u8 unreadBeforeRotation[4]; /* The separate Y scale is not used here. */
    f32 rotation; /* Rotation about the sprite's Z axis, in radians. */
    Vec position; /* World-space particle position. */
    u8 unreadBeforeAlpha[3]; /* RGB channels are not changed by this callback. */
    u8 alpha; /* Opacity; wrapping below zero marks the particle for release. */
} M620Particle;

extern s32 fn_1_FB8(s32 value);

extern void fn_1_B9F0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

extern void fn_1_BD10(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

/* Called by fn_1_B7D4 to load the falling-particle animation and register its callback. */
void fn_1_B8D0(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_FB8(22), HU_MEMNUM_OVL,
                                                    HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 160);
    object->mdlId[0] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleColSet(modelId, 218, 203, 158);
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_B9F0);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

/* Updates falling particles when the particle model's draw callback runs. */
void fn_1_B9F0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    M620Particle *entry = (M620Particle *)particle->data;
    s32 index;

    for (index = 0; index < particle->maxCnt; index++, entry++) {
        switch (entry->state) {
        case 0:
            entry->state++;
            entry->scale = (f32)(10.0 + 5.0 *
                (1.52587890625e-05 * (f64)(f32)(s32)(frand() & 0xFFFF)));
            entry->rotation = 0.0f;
            entry->alpha = 180;
            /* fall through */
        case 1:
            entry->scale += 4.0;
            entry->rotation += 0.008726646192371845;
            entry->alpha -= 4;
            /* Unsigned alpha wraps above 240 when the fade passes zero. */
            if (entry->position.y < -100.0 || entry->scale <= 0.0f ||
                entry->alpha > 240) {
                entry->state = -1;
                entry->scale = 0.0f;
            } else {
                PSVECAdd(&entry->position, &entry->velocity, &entry->position);
                entry->velocity.x *= 0.95;
                entry->velocity.y -= 0.05;
                entry->velocity.z *= 0.95;
            }
            break;
        }
    }

    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(M620Particle));
    PPCSync();
}

/* Called by fn_1_B7D4 to load the burst animation and register its callback. */
void fn_1_BC04(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(HuDataSelHeapReadNum(fn_1_FB8(23), HU_MEMNUM_OVL,
                                                    HEAP_MODEL));
    modelId = Hu3DParticleCreate(animation, 100);
    object->mdlId[1] = modelId;
    particle = (HU3D_PARTICLE *)Hu3DData[(s16)modelId].hookData;
    Hu3DParticleBlendModeSet(modelId, 0);
    Hu3DParticleHookSet(modelId, fn_1_BD10);
    Hu3DModelLayerSet(modelId, 6);
    Hu3DModelPosSet(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

const GXColor lbl_1_rodata_3E0[8] = {
    {255, 255, 255, 255},
    {255, 255, 192, 255},
    {255, 192, 255, 255},
    {255, 192, 192, 255},
    {192, 255, 255, 255},
    {192, 255, 192, 255},
    {192, 192, 255, 255},
    {255, 255, 255, 255}
};

/* Advances burst particles when the particle model's draw callback runs. */
void fn_1_BD10(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry = particle->data;
    s32 index;
    s32 remaining;

    for (index = 0; index < particle->maxCnt; index++, entry++) {
        switch (entry->time) {
        case 0:
            entry->time++;
            entry->parManId = 0;
            entry->scale = (f32)(20.0 + 10.0 *
                (1.0 / 65536.0 * (f64)(f32)(s32)(frand() & 0xFFFF)));
            entry->zRot = (f32)(0.01745329238474369 *
                (360.0 * (1.0 / 65536.0 * (f64)(f32)(s32)(frand() & 0xFFFF))));
            entry->color = lbl_1_rodata_3E0[frand() & 7];
            /* fall through */
        case 1:
            /* The particle-manager ID is used here as an increasing fade step. */
            remaining = entry->color.a;
            if ((remaining -= entry->parManId) < 0) {
                entry->time = -1;
                entry->scale = 0.0f;
            } else {
                entry->color.a = remaining;
                entry->zRot += 0.08726646192371845;
                entry->parManId++;
            }
            break;
        }
    }

    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
    PPCSync();
}

typedef struct M620RingHookDataView {
    u8 unreadRecords[140]; /* Ring pointers are accessed through the draw callback's view. */
    f32 xOffsets[16]; /* Unit-circle X components used to build ring vertices. */
    f32 zOffsets[16]; /* Unit-circle Z components used to build ring vertices. */
} M620RingHookDataView;

void fn_1_C050(HU3D_MODEL *model, Mtx *view);

/* Called by fn_1_B7D4 to create the ring hook and precompute its directions. */
void fn_1_BF14(OMOBJ *owner)
{
    M620RingHookDataView *hookData;
    HU3D_MODEL *modelData;
    s16 modelId;
    f32 angle;
    s32 i;

    modelId = Hu3DHookFuncCreate(fn_1_C050);
    owner->mdlId[2] = modelId;
    Hu3DModelLayerSet(modelId, 4);
    modelData = &Hu3DData[modelId];
    hookData = HuMemDirectMallocNum(HEAP_HEAP, sizeof(*hookData), HU_MEMNUM_OVL);
    modelData->hookData = hookData;
    memset(hookData, 0, sizeof(*hookData));
    i = 0;
    angle = 0.0f;
    /* The 22-degree steps leave a 30-degree segment where the ring closes. */
    while (i < 16) {
        hookData->xOffsets[i] = (f32) sin(((3.141592653589793) * (f64) angle) / (180.0));
        hookData->zOffsets[i] = (f32) cos(((3.141592653589793) * (f64) angle) / (180.0));
        i++;
        angle += (22.0f);
    }
}

typedef struct M620EffectRecordView {
    s32 active; /* Zero suppresses drawing. */
    s32 blendMode; /* Zero uses ordinary alpha blending; other values add the glow. */
    f32 radius; /* Ring radius in world units. */
    Vec *position; /* Ring center; drawing uses only X and Z. */
} M620EffectRecordView;

typedef struct M620EffectCollectionView {
    M620EffectRecordView *records[35]; /* Registered ring records; NULL slots are free. */
    f32 xOffsets[16]; /* Unit-circle X components computed at setup. */
    f32 zOffsets[16]; /* Unit-circle Z components computed at setup. */
} M620EffectCollectionView;

#define M620_RING_CENTER_SOLID_BIT_PATTERN 0x00000080U
#define M620_RING_POINT_SOLID_BIT_PATTERN 0x00000018U
#define M620_RING_CENTER_GLOW_BIT_PATTERN 0x808000C0U
#define M620_RING_POINT_GLOW_BIT_PATTERN 0x80700000U

/* Hu3D invokes this hook to draw each ring from its center and radial points. */
void fn_1_C050(HU3D_MODEL *model, Mtx *view)
{
    u32 centerColorSolid;
    u32 pointColorSolid;
    u32 centerColorGlow;
    u32 pointColorGlow;
    M620EffectCollectionView *effects = model->hookData;
    Mtx matrix;
    M620EffectRecordView *record;
    f32 *xOffsets = effects->xOffsets;
    f32 *zOffsets = effects->zOffsets;
    s32 i;
    s32 vertex;
    f32 radius;
    u32 centerColor;
    u32 pointColor;

    centerColorSolid = M620_RING_CENTER_SOLID_BIT_PATTERN;
    pointColorSolid = M620_RING_POINT_SOLID_BIT_PATTERN;
    centerColorGlow = M620_RING_CENTER_GLOW_BIT_PATTERN;
    pointColorGlow = M620_RING_POINT_GLOW_BIT_PATTERN;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
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

    for (i = 0; i < 35; i++) {
        record = effects->records[i];
        if (record != NULL && record->active != 0) {
            if (record->blendMode == 0) {
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_CLEAR);
                centerColor = centerColorSolid;
                pointColor = pointColorSolid;
            } else {
                GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
                centerColor = centerColorGlow;
                pointColor = pointColorGlow;
            }
            radius = record->radius;
            /* Rings remain three units above the ground, ignoring the center's Y. */
            PSMTXTrans(matrix, record->position->x, (3.0f), record->position->z);
            PSMTXConcat(*view, matrix, matrix);
            GXLoadPosMtxImm(matrix, GX_PNMTX0);
            GXBegin(GX_TRIANGLEFAN, GX_VTXFMT0, 18);
            GXPosition3f32(0.0f, 0.0f, 0.0f);
            GXColor1u32(centerColor);
            for (vertex = 0; vertex < 16; vertex++) {
                GXPosition3f32(xOffsets[vertex] * radius, 0.0f,
                               zOffsets[vertex] * radius);
                GXColor1u32(pointColor);
            }
            GXPosition3f32(xOffsets[0] * radius, 0.0f,
                           zOffsets[0] * radius);
            GXColor1u32(pointColor);
        }
    }
}

#include <string.h>

s32 lbl_1_data_6D0[4] = {17, 18, 19, 20};

extern void fn_1_C4E0(HU3D_MODEL *model, Mtx *matrix);

typedef struct M620HookWork {
    ANIMDATA *animations[4]; /* Trail textures selected by each entry's source value. */
    M620ParticleBatch entries[8]; /* Trail slots; source -1 marks a free slot. */
} M620HookWork;

/* Called by fn_1_B7D4 to create the trail hook, load textures, and clear slots. */
void fn_1_C358(OMOBJ *object)
{
    s16 modelId;
    HU3D_MODEL *model;
    M620HookWork *work;
    M620ParticleBatch *entry;
    s32 index;

    modelId = Hu3DHookFuncCreate(fn_1_C4E0);
    object->mdlId[3] = modelId;
    Hu3DModelLayerSet(modelId, 6);
    model = &Hu3DData[modelId];
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M620HookWork), HU_MEMNUM_OVL);
    model->hookData = work;
    memset(work, 0, sizeof(M620HookWork));

    for (index = 0; index < 4; index++) {
        work->animations[index] = HuSprAnimRead(
            HuDataSelHeapReadNum(fn_1_FB8(lbl_1_data_6D0[index]),
                                 HU_MEMNUM_OVL, HEAP_MODEL));
    }

    for (index = 0; index < 8; index++) {
        entry = &work->entries[index];
        entry->source = -1;
    }
}

void fn_1_CF8C(M620ParticleBatch *batch);

/* Updates active trail slots each frame from the effect object's update hook. */
void fn_1_C458(OMOBJ *object)
{
    s16 modelId = object->mdlId[3];
    HU3D_MODEL *model = &Hu3DData[modelId];
    M620HookWork *work = model->hookData;
    M620ParticleBatch *entry;
    s32 index;

    for (index = 0; index < 8; index++) {
        entry = &work->entries[index];
        if (entry->source != -1) {
            fn_1_CF8C(entry);
        }
    }
}

/* Called by fn_1_C4E0 and fn_1_C564 to set texture and GX state for a trail. */
void fn_1_D45C(const void *matrix, const M620HookWork *work,
               const M620ParticleBatch *entry);

void fn_1_D56C(void);

void fn_1_D60C(void);

void fn_1_D70C(Mtx matrix, M620ParticleBatch *batch);

/* Hu3D invokes this hook to draw each active trail. */
void fn_1_C4E0(HU3D_MODEL *model, Mtx *matrix)
{
    M620HookWork *work = model->hookData;
    M620ParticleBatch *entry;
    s32 index;

    for (index = 0; index < 8; index++) {
        entry = &work->entries[index];

        if (entry->source != -1) {
            fn_1_D45C(matrix, work, entry);
            fn_1_D56C();
            fn_1_D70C(*matrix, entry);
        }
    }
}

/* Draws active trails with the translucent effect render state. */
void fn_1_C564(HU3D_MODEL *model, Mtx *matrix)
{
    M620HookWork *work = model->hookData;
    M620ParticleBatch *entry;
    s32 index;

    for (index = 0; index < 8; index++) {
        entry = &work->entries[index];

        if (entry->source != -1) {
            fn_1_D45C(matrix, work, entry);
            fn_1_D60C();
            fn_1_D70C(*matrix, entry);
        }
    }
}

typedef struct M620C5E8Target {
    u8 unreadBeforePosition[32]; /* Registration state is not used for trail collision. */
    Vec *position; /* Collision center in world coordinates. */
    f32 lowerHeight; /* Height offset from the center to the collision region's base. */
    f32 heightSpan; /* Height added to the base to obtain the top surface. */
    f32 innerRadius; /* First contribution to the horizontal collision radius. */
    f32 radialThickness; /* Additional horizontal margin, in world units. */
} M620C5E8Target;

/* Measures horizontal distance between two points. */
static inline float fn_1_488C(Vec *firstPoint, Vec *secondPoint)
{
    float deltaX, deltaZ;
    deltaX = secondPoint->x - firstPoint->x;
    deltaZ = secondPoint->z - firstPoint->z;
    return sqrtf(deltaX * deltaX + deltaZ * deltaZ);
}

extern void *fn_1_10118(u32 type, s32 id);

/* Adjusts trail points against the linked object before the trail is smoothed. */
void fn_1_C5E8(M620ParticleBatch *work, s32 pointCount)
{
    M620C5E8Target * target;
    M620C5E8Target * targetData;
    Vec * targetPosition;
    M620VectorRecord *point;
    Vec * pointPosition;
    f32 floorHeight;
    f32 radius;
    f32 topHeight;
    f32 heightBase;
    f32 heightSlope;
    f32 dx;
    f32 dz;
    f32 pointHeight;
    f32 heightDifference;
    f32 angle;
    f32 distance;
    s32 index;
    if (work->resource != - 1)
    {
        target = fn_1_10118(3, work->resource);
        if (target != NULL)
        {
            targetData = target;
            targetPosition = targetData->position;
            radius = targetData->innerRadius + targetData->radialThickness;
            topHeight =
                targetData->heightSpan + (targetData->position->y + targetData->lowerHeight);
            heightBase = (f32)((f64) topHeight - 60.0);
            heightSlope = (heightBase - topHeight) / radius;
            floorHeight = 0.0f;
            for (index = 1; index < pointCount; index += 1)
            {
                point = &work->records[index];
                pointPosition = &point->origin;
                if (! (pointPosition->y > topHeight))
                {
                    dx = pointPosition->x - targetPosition->x;
                    dz = pointPosition->z - targetPosition->z;
                    distance = sqrtf((dx * dx) + (dz * dz));
                    pointHeight = topHeight + (heightSlope * distance);
                    heightDifference = pointHeight - pointPosition->y;
                    if (heightDifference > 0.0)
                    {
                        if (distance < radius)
                        {
                            if (heightDifference < (radius - distance))
                            {
                                pointPosition->y = pointHeight;
                            }
                            else
                            {
                                angle =
                                    (f32) (180.0 * (atan2((f64) dx, (f64) dz) / 3.141592653589793));
                                pointPosition->x =
                                    (f32) ((f64) targetData->position->x +
                                           ((f64) radius *
                                            sin((3.141592653589793 * (f64) angle) / 180.0)));
                                pointPosition->z =
                                    (f32) ((f64) targetData->position->z +
                                           ((f64) radius *
                                            cos((3.141592653589793 * (f64) angle) / 180.0)));
                            }
                        }
                    }
                    if (pointPosition->y <= floorHeight)
                    {
                        pointPosition->y = floorHeight;
                        PSVECScale(&point->displacement, &point->displacement, 0.8f);
                    }
                }
            }
        }
    }
}

void fn_1_CC84(M620VectorRecord *destination, const Point3d *source);

s32 fn_1_CE14(M620ParticleBatch *work, Point3d *start, Point3d *end);

/* Called by fn_1_E570 to initialize a trail, then settle it for 300 update steps. */
void fn_1_C9D0(M620ParticleBatch *work, Point3d *pointB,
               Point3d *pointA, f32 trailLength, s32 sourceMode,
               s32 targetMode, s32 resourceId)
{
    Point3d position;
    Point3d endPosition;
    M620VectorRecord record;
    s32 frameIndex;
    f64 interpolationAmount;
    f32 pointBX;
    f32 pointAX;
    f32 pointBY;
    f32 pointAY;
    f32 pointBZ;
    f32 pointAZ;
    f64 x;
    f64 roundedX;
    f64 y;
    f64 roundedY;
    f64 z;
    f64 roundedZ;

    work->source = sourceMode;
    work->target = targetMode;
    work->resource = resourceId;
    work->trailLength = trailLength;
    work->active = 0;

    for (frameIndex = 0; frameIndex < 32; frameIndex++) {
        interpolationAmount = (f64)frameIndex / (31.0);
        pointAX = pointA->x;
        pointBX = pointB->x;
        x = (1.0 - interpolationAmount) * pointBX + interpolationAmount * pointAX;
        roundedX = x;
        position.x = (f32)roundedX;
        pointAY = pointA->y;
        pointBY = pointB->y;
        y = (1.0 - interpolationAmount) * pointBY + interpolationAmount * pointAY;
        roundedY = y;
        position.y = (f32)roundedY;
        pointAZ = pointA->z;
        pointBZ = pointB->z;
        z = (1.0 - interpolationAmount) * pointBZ + interpolationAmount * pointAZ;
        roundedZ = z;
        position.z = (f32)roundedZ;
        fn_1_CC84(&record, &position);
        work->records[frameIndex] = record;
    }

    for (frameIndex = 0; frameIndex < 300; frameIndex++) {
        fn_1_CE14(work, pointA, &endPosition);
        fn_1_CF8C(work);
    }
}

/* Called during trail creation and endpoint movement to reset one sample's displacement. */
void fn_1_CC84(M620VectorRecord *destination, const Point3d *source)
{
    destination->origin = *source;
    destination->previousPosition = *source;
    destination->displacement.x = 0.0f;
    destination->displacement.y = 0.0f;
    destination->displacement.z = 0.0f;
}

/* Called by fn_1_E8A8 to move a sample and its previous position by the same delta. */
void fn_1_CCE8(Point3d *positions, const Point3d *target, f32 scale)
{
    Point3d delta;

    PSVECSubtract(target, positions, &delta);
    PSVECScale(&delta, &delta, scale);
    PSVECAdd(positions, &delta, positions);
    PSVECAdd(positions + 1, &delta, positions + 1);
}

/* Lowers a trail sample and adds its result vector to the sample position. */
void fn_1_CD58(M620VectorRecord *record)
{
    record->origin.y -= (0.27240694444444447);
    PSVECAdd(&record->displacement, &record->origin, &record->origin);
}

/* Replaces the middle point with the first and stores the scaled segment delta. */
void fn_1_CDAC(Vec *work)
{
    PSVECSubtract(&work[0], &work[1], &work[2]);
    PSVECScale(&work[2], &work[2], (0.98f));
    work[1] = work[0];
}

/* Set the last sample to the requested endpoint, clamping its distance from the first
 * sample to the trail length. When already within range, return zero without writing end. */
s32 fn_1_CE14(M620ParticleBatch *work, Point3d *start, Point3d *end)
{
    Point3d difference;
    Point3d *origin = &work->records[0].origin;
    M620VectorRecord *result = &work->records[31];
    f32 distance;

    work->active = 1;
    PSVECSubtract(origin, start, &difference);
    distance = PSVECMag(&difference);
    if (distance <= work->trailLength) {
        fn_1_CC84(result, start);
        return 0;
    }

    PSVECScale(&difference, &difference,
               (distance - work->trailLength) / distance);
    PSVECAdd(start, &difference, end);
    fn_1_CC84(result, end);
    return 1;
}

extern void fn_1_C5E8(M620ParticleBatch *batch, s32 count);

extern void fn_1_D014(M620ParticleBatch *batch, s32 count);

extern void fn_1_D0B4(M620ParticleBatch *work, s32 count);

typedef M620ParticleBatch M620WorkD194;

typedef M620VectorRecord M620WorkFrameD194;

extern void fn_1_D194(M620WorkD194 *work);

/* Called by fn_1_C458 and fn_1_C9D0 to update one trail before it is drawn. */
void fn_1_CF8C(M620ParticleBatch *batch)
{
    s32 active;
    s32 count;

    if (batch->active != 0) {
        active = 1;
    } else {
        active = 0;
    }
    count = 32 - active;
    fn_1_D014(batch, count);
    fn_1_C5E8(batch, count);
    fn_1_D194(batch);
    fn_1_D0B4(batch, count);
    batch->active = 0;
}

/* Lowers and advances each interior trail sample during fn_1_CF8C. */
void fn_1_D014(M620ParticleBatch *batch, s32 count)
{
    s32 index;

    for (index = 1; index < count; index++) {
        fn_1_CD58(&batch->records[index]);
    }
}

/* Recomputes and damps the interior sample deltas during fn_1_CF8C. */
void fn_1_D0B4(M620ParticleBatch *work, s32 count)
{
    s32 index;

    for (index = 1; index < count; index++) {
        fn_1_CDAC(&work->records[index].origin);
    }
}

/* Smooths trail spacing in forward and reverse passes during fn_1_CF8C. */
void fn_1_D194(M620WorkD194 *work)
{
    Vec originalFirst;
    Vec originalLast;
    Vec deltaForward;
    Vec deltaReverse;
    M620WorkFrameD194 *frame;
    M620WorkFrameD194 *reverseFrame;
    s32 divisor;
    s32 frameOffset;
    s32 passCount;
    M620WorkFrameD194 *frameLimit;
    s32 pass;
    s32 frameIndex;
    f32 threshold;
    f32 forwardDistance;
    f32 reverseDistance;

    originalFirst = work->records[0].origin;
    originalLast = work->records[31].origin;
    threshold = work->trailLength / (31.0f);
    if (work->active != 0) {
        divisor = 2;
    } else {
        divisor = 1;
    }
    passCount = 9 / divisor;

    for (pass = 0; pass < passCount; pass++) {
        frame = &work->records[0];
        if (work->active != 0) {
            frameOffset = 2;
        } else {
            frameOffset = 1;
        }
        frameLimit = &work->records[32 - frameOffset];

        while (frame != frameLimit) {
            s32 forwardMode = work->target;

            PSVECSubtract(&frame[1].origin, &frame[0].origin, &deltaForward);
            forwardDistance = PSVECMag(&deltaForward);
            if (forwardMode != 0) {
                if (forwardDistance == threshold ||
                     0.0f == forwardDistance) {
                    goto forward_next;
                }
            } else if (forwardDistance <= threshold) {
                goto forward_next;
            }
            PSVECScale(&deltaForward, &deltaForward,
                       ((forwardDistance - threshold) / forwardDistance) / 2.0f);
            PSVECAdd(&frame[0].origin, &deltaForward, &frame[0].origin);
            PSVECSubtract(&frame[1].origin, &deltaForward, &frame[1].origin);
forward_next:
            frame++;
        }

        if (work->active != 0) {
            reverseFrame = &work->records[0];
            for (frameIndex = 31; frameIndex >= 1; frameIndex--) {
                s32 reverseMode = work->target;

                PSVECSubtract(
                    &reverseFrame[frameIndex - 1].origin,
                    &reverseFrame[frameIndex].origin,
                    &deltaReverse);
                reverseDistance = PSVECMag(&deltaReverse);
                if (reverseMode != 0) {
                    if (reverseDistance == threshold ||
                         0.0f == reverseDistance) {
                        continue;
                    }
                } else if (reverseDistance <= threshold) {
                    continue;
                }
                PSVECScale(&deltaReverse, &deltaReverse,
                           ((reverseDistance - threshold) / reverseDistance) / 2.0f);
                PSVECAdd(
                    &reverseFrame[frameIndex].origin,
                    &deltaReverse,
                    &reverseFrame[frameIndex].origin);
                PSVECSubtract(
                    &reverseFrame[frameIndex - 1].origin,
                    &deltaReverse,
                    &reverseFrame[frameIndex - 1].origin);
            }
        }
        /* The source stays fixed; endpoint movement also pins the last sample. */
        work->records[0].origin = originalFirst;
        if (work->active != 0) {
            work->records[31].origin = originalLast;
        }
    }
}

extern int HuSprTexLoad(int texture, int x, int y, int width, int height, int format);

const GXVtxDescList lbl_1_rodata_484[3] = {
    {9, 1}, {13, 1}, {255, 0}
};

const GXVtxAttrFmtList lbl_1_rodata_49C[3] = {
    {9, 1, 4, 0}, {13, 1, 4, 0}, {255, 0, 0, 0}
};

/* Called by trail draw hooks to select the texture and configure GX state. */
void fn_1_D45C(const void *matrix, const M620HookWork *work,
               const M620ParticleBatch *entry)
{
    GXLoadPosMtxImm(matrix, 0);
    GXSetNumChans(0);
    HuSprTexLoad((u32)work->animations[entry->source], 0, 0, 0, 0, 1);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(0, 1, 4, GX_IDENTITY, 0, GX_PTIDENTITY);
    GXSetCullMode(2);
    GXSetColorUpdate(1);
    GXSetAlphaUpdate(0);
    GXSetZCompLoc(0);
    GXSetZMode(1, 1, 1);
    GXSetAlphaCompare(6, 192, 0, 3, 255);
    GXSetBlendMode(1, 4, 5, 5);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_484);
    GXSetVtxAttrFmtv(0, lbl_1_rodata_49C);
}

/* Called by fn_1_C4E0 to select the textured combiner for opaque trails. */
void fn_1_D56C(void)
{
    GXSetNumTevStages(1U);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0U,
                    GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0U,
                    GX_TEVPREV);
}

const GXColor lbl_1_rodata_4CC = {0, 0, 0, 102};

/* Called by fn_1_C564 to select the tinted combiner for translucent trails. */
void fn_1_D60C(void)
{
    GXColor tevColor = lbl_1_rodata_4CC;

    GXSetTevColor(GX_TEVREG0, tevColor);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_A0);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, 0, GX_TEVPREV);
}

/* Builds and submits the triangle strips for one trail draw callback. */
void fn_1_D70C(Mtx matrix, M620ParticleBatch *batch)
{
    Vec vertices[32 * 3];
    Mtx inverseMatrix;
    Vec samples[3];
    Mtx rotation;
    Vec center = {0.0f, 0.0f, 0.0f};
    Vec nextSegment;
    Vec previousSegment;
    Vec radial;
    Vec direction;
    Vec axis;
    s32 index;
    s32 previousIndex;
    s32 currentIndex;
    s32 nextIndex;
    s32 vertex;
    s32 stripVertex;
    f32 textureStartF;
    f32 textureEndF;
    f32 texturePosition;
    Vec *stripPoint;
    Vec *nextStripPoint;

    PSMTXInverse(matrix, inverseMatrix);
    PSMTXMultVec(inverseMatrix, &center, &center);

    for (index = 0; index < 32; index++) {
        previousIndex = (index - 1 < 0) ? 0 :
            ((index - 1 > 31) ? 31 : index - 1);
        currentIndex = (index < 0) ? 0 : ((index > 31) ? 31 : index);
        nextIndex = (index + 1 < 0) ? 0 :
            ((index + 1 > 31) ? 31 : index + 1);
        samples[0] = batch->records[previousIndex].origin;
        samples[1] = batch->records[currentIndex].origin;
        samples[2] = batch->records[nextIndex].origin;
        PSVECSubtract(&samples[1], &samples[0], &previousSegment);
        PSVECSubtract(&samples[2], &samples[1], &nextSegment);
        if (PSVECSquareMag(&previousSegment) <= 0.0f) {
            previousSegment = nextSegment;
        } else if (PSVECSquareMag(&nextSegment) <= 0.0f) {
            nextSegment = previousSegment;
        }
        PSVECAdd(&previousSegment, &nextSegment, &axis);
        if (PSVECSquareMag(&axis) <= 0.0f) {
            PSVECSubtract(&samples[1], &samples[0], &axis);
            if (PSVECSquareMag(&axis) <= 0.0f) {
                axis.y = (1.0f);
            }
        }
        PSVECNormalize(&axis, &axis);
        PSVECSubtract(&samples[1], &center, &direction);
        PSVECNormalize(&direction, &direction);
        PSVECCrossProduct(&axis, &direction, &radial);
        PSVECNormalize(&radial, &radial);
        PSVECScale(&radial, &radial, (9.0f));
        PSMTXRotAxisRad(rotation, &axis, (0.34906584f));
        PSMTXMultVecSR(rotation, &radial, &radial);
        PSMTXRotAxisRad(rotation, &axis, (1.2217305f));

        for (vertex = 0; vertex < 3; vertex++) {
            PSVECAdd(&samples[1], &radial, vertices + 3 * index + vertex);
            PSMTXMultVecSR(rotation, &radial, &radial);
        }
    }

    for (index = 0; index < 31; index++) {
        textureStartF = (index == 30) ? (0.6) : (0.2);
        textureEndF = (f32)((0.4) + (f64)textureStartF);
        GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 6U);
        for (stripVertex = 0; stripVertex < 3; stripVertex++) {
            stripPoint = &vertices[3 * index + stripVertex];
            nextStripPoint = &vertices[3 * index + stripVertex + 3];
            texturePosition = (f32)stripVertex / 2.0f;
            GXPosition3f32(stripPoint->x, stripPoint->y, stripPoint->z);
            GXTexCoord2f32(textureStartF, texturePosition);
            GXPosition3f32(nextStripPoint->x, nextStripPoint->y, nextStripPoint->z);
            GXTexCoord2f32(textureEndF, texturePosition);
        }
    }
}

typedef struct M620ParticleRecord {
    s16 state; /* Negative values mark a free particle. */
    u8 unreadBeforeVelocity[18]; /* Other particle attributes are not set by this helper. */
    Vec velocity; /* Displacement consumed by the falling-particle callback. */
    u8 unreadBeforePosition[24]; /* Size and rotation are initialized by the callback. */
    Vec position; /* Initial world-space position. */
    u8 unreadColor[4]; /* Color is not set by this helper. */
} M620ParticleRecord;

typedef struct M620ParticleWork {
    u8 unreadBeforeCount[48]; /* Particle-model settings are not inspected here. */
    s16 count; /* Number of particle slots to search. */
    u8 unreadBeforeRecords[22]; /* Counters and animation settings are not inspected here. */
    M620ParticleRecord *records; /* Particle slots owned by the model. */
} M620ParticleWork;

/* Called by fn_1_DD18 to start the next free particle at its position and velocity. */
void fn_1_DC54(const Vec *position, const Vec *velocity)
{
    OMOBJ *object = lbl_1_bss_650;
    s16 modelId = object->mdlId[0];
    M620ParticleWork *work = Hu3DData[modelId].hookData;
    M620ParticleRecord *record = work->records;
    s32 index;

    for (index = 0; index < work->count; index++, record++) {
        if (record->state < 0) {
            record->state = 0;
            record->position = *position;
            record->velocity = *velocity;
            break;
        }
    }
}

/* Called by fn_1_73A0 on landing to scatter particles around the launch prop. */
void fn_1_DD18(const Vec *origin, f32 radius, s32 count)
{
    s32 index;

    for (index = 0; index < count; index++) {
        Vec position;
        Vec velocity;
        f32 sine;
        f32 cosine;
        f32 angle;

        angle = (f32)((f64)(f32)((s32)frand() & 0xFFFF) *
                       1.52587890625e-05 * (240.0) + (240.0));
        sine = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        cosine = (f32)cos(((3.141592653589793) * (f64)angle) / (180.0));

        position.x = (f32)((f64)(origin->x + sine * radius) +
            (f64)(f32)(((s32)frand() & 0xFFFF) - 32768) * (3.0517578125e-05) *
                (30.0));
        position.y = (f32)((f64)origin->y +
            (f64)(f32)((s32)frand() & 0xFFFF) * 1.52587890625e-05 *
                (20.0) + 10.0);
        position.z = (f32)((f64)(origin->z + cosine * radius) +
            (f64)(f32)(((s32)frand() & 0xFFFF) - 32768) * (3.0517578125e-05) *
                (30.0));

        velocity.x = (f32)((f64)(f32)(((s32)frand() & 0xFFFF) - 32768) *
            (3.0517578125e-05) + (3.0) * sine);
        velocity.y = (f32)((f64)(f32)((s32)frand() & 0xFFFF) *
            1.52587890625e-05 * (0.3) + (0.3));
        velocity.z = (f32)((f64)(f32)(((s32)frand() & 0xFFFF) - 32768) *
            (3.0517578125e-05) + (3.0) * cosine);

        fn_1_DC54(&position, &velocity);
    }
}

typedef struct M620EffectRecordE1F4 {
    s16 state; /* Negative values mark a free burst-particle slot. */
    u8 unreadBeforePosition[54]; /* Other particle attributes are initialized by the callback. */
    Vec position; /* Initial world-space position. */
    u8 unreadColor[4]; /* Color is selected by the burst-particle callback. */
} M620EffectRecordE1F4;

typedef struct M620EffectWorkE1F4 {
    u8 beforeCount[48]; /* Particle-model settings are not inspected here. */
    s16 count; /* Number of particle slots to search. */
    u8 beforeRecords[22]; /* Counters and animation settings are not inspected here. */
    M620EffectRecordE1F4 *records; /* Burst-particle slots owned by the model. */
} M620EffectWorkE1F4;

typedef struct M620EffectHookRecordView {
    s16 active; /* Negative values mark a free burst-particle slot. */
    u8 unreadBeforePosition[54]; /* Other particle attributes are initialized by the callback. */
    Vec position; /* Initial world-space position. */
    u8 unreadColor[4]; /* Color is selected by the burst-particle callback. */
} M620EffectHookRecordView;

typedef struct M620EffectHookWorkView {
    u8 unreadBeforeCount[48]; /* Particle-model settings are not inspected here. */
    s16 count; /* Number of particle slots to search. */
    u8 unreadBeforeRecords[22]; /* Counters and animation settings are not inspected here. */
    M620EffectHookRecordView *records; /* Burst-particle slots owned by the model. */
} M620EffectHookWorkView;

/* Starts the first free burst particle at the supplied position, if a slot is available. */
void fn_1_E148(const Vec *position)
{
    OMOBJ *object = lbl_1_bss_650;
    s16 modelId = object->mdlId[1];
    M620EffectHookWorkView *work = Hu3DData[modelId].hookData;
    M620EffectHookRecordView *record = work->records;
    s32 index;

    for (index = 0; index < work->count; index++, record++) {
        if (record->active < 0) {
            record->active = 0;
            record->position = *position;
            return;
        }
    }
}

/* Called by fn_1_E1F4 to start one burst particle, if a slot is available. */
static inline void M620EffectPositionSet(const Vec *position)
{
    OMOBJ *object = lbl_1_bss_650;
    s16 modelId = object->mdlId[1];
    M620EffectWorkE1F4 *work = Hu3DData[modelId].hookData;
    M620EffectRecordE1F4 *record = work->records;
    s32 index;

    for (index = 0; index < work->count; index++, record++) {
        if (record->state < 0) {
            record->state = 0;
            record->position = *position;
            return;
        }
    }
}

/* Called during actor movement to scatter burst particles around the supplied position. */
void fn_1_E1F4(const Vec *origin, f32 radius, s32 count)
{
    Vec position;
    f32 angle;
    f32 sine;
    f32 cosine;
    s32 index;

    for (index = 0; index < count; index++) {
        angle = (f32)((f64)(f32)(s32)(frand() & 0xFFFF) *
                       1.52587890625e-05 * (360.0));
        sine = (f32)sin(((3.141592653589793) * (f64)angle) / (180.0));
        cosine = (f32)cos(((3.141592653589793) * (f64)angle) / (180.0));
        position.x = (f32)(origin->x + sine * radius +
                           (3.0517578125e-05) *
                               (f64)(f32)(s32)((frand() & 0xFFFF) - 32768) *
                               (20.0));
        position.y = (f32)(origin->y +
                           1.52587890625e-05 *
                               (f64)(f32)(s32)(frand() & 0xFFFF) *
                                (20.0) + 10.0);
        position.z = (f32)(origin->z + cosine * radius +
                           (3.0517578125e-05) *
                               (f64)(f32)(s32)((frand() & 0xFFFF) - 32768) *
                               (20.0));

        M620EffectPositionSet(&position);
    }
}

typedef struct M620EffectView_s {
    s32 active, blendMode; /* Drawing enable and ring blend selection. */
    u8 unreadAfterBlendMode[16]; /* Remaining record fields are not read by registration. */
    s32 unreadTail; /* This word is not read by registration. */
} M620EffectView;

typedef struct M620EffectSlotsView_s {
    M620EffectView *entries[5]; /* Pointer storage scanned by the ring registration helper. */
} M620EffectSlotsView;

/* Called by actor setup to add its ring record to the first available draw slot. */
void fn_1_E4E0(M620EffectView *effect)
{
    M620EffectView **entry;
    s32 index;
    OMOBJ *object;
    HU3D_MODEL *model;
    s16 modelId;
    M620EffectSlotsView *work;

    object = lbl_1_bss_650;
    modelId = object->mdlId[2];
    model = &Hu3DData[modelId];
    work = model->hookData;
    entry = work->entries;
    for (index = 0; index < 35; index++, entry++) {
        if (*entry == NULL) {
            *entry = effect;
            break;
        }
    }
}

extern void fn_1_C9D0(M620ParticleBatch *work, Point3d *pointB,
                      Point3d *pointA, f32 trailLength, s32 sourceMode,
                      s32 targetMode, s32 resourceId);

/* Called by fn_1_7130 to reserve and seed a trail; returns -1 when all slots are used. */
s32 fn_1_E570(Point3d *pointB, Point3d *pointA, f32 trailLength,
              s32 sourceMode, s32 targetMode, s32 resourceId)
{
    OMOBJ *object;
    s16 modelId;
    HU3D_MODEL *model;
    M620HookWork *work;
    s32 index;

    object = lbl_1_bss_650;
    modelId = object->mdlId[3];
    model = &Hu3DData[modelId];
    work = model->hookData;
    for (index = 0; index < 8; index++) {
        M620ParticleBatch *entry = &work->entries[index];

        if (entry->source == -1) {
            fn_1_C9D0(entry, pointB, pointA, trailLength, sourceMode & 3,
                      targetMode, resourceId);
            return index;
        }
    }
    return -1;
}

/* Called by fn_1_73A0 each frame to move a trail's first point with its source. */
void fn_1_E640(s32 index, const Vec *position)
{
    OMOBJ *object;
    s16 modelId;
    HU3D_MODEL *model;
    M620HookWork *work;
    M620ParticleBatch *entry;

    if (index >= 0) {
        if (index < 8) {
            object = lbl_1_bss_650;
            modelId = object->mdlId[3];
            model = &Hu3DData[modelId];
            work = model->hookData;
            entry = &work->entries[index];
            entry->records[0].origin = *position;
        }
    }
}

typedef struct M620EffectData {
    u8 unreadAnimations[16]; /* Four animation pointers precede the trail records. */
    M620ParticleBatch records[1]; /* Trail records indexed by the selected slot. */
} M620EffectData;

typedef struct M620EffectModelView {
    u8 unreadBeforeHookData[288]; /* Other model state is not accessed here. */
    M620EffectData *effectData; /* Trail-model hook data. */
    u8 unreadAfterHookData[16]; /* Other model callbacks and counters. */
} M620EffectModelView;

typedef struct M620EffectObject {
    u8 unreadBeforeModelIds[64]; /* Other object state is not accessed here. */
    s16 *modelIds; /* Effect model IDs. */
} M620EffectObject;

/* Called by fn_1_73A0 to place the last sample at the requested endpoint,
 * limited to the trail length from the first sample. */
s32 fn_1_E6D0(s32 recordIndex, Vec *position, Vec *adjustedPosition)
{
    OMOBJ *object;
    s16 modelId;
    HU3D_MODEL *model;
    M620EffectData *effectData;
    M620ParticleBatch *record;

    if (recordIndex >= 0 && recordIndex < 8) {
        object = lbl_1_bss_650;
        modelId = object->mdlId[3];
        model = &Hu3DData[modelId];
        effectData = model->hookData;
        record = &effectData->records[recordIndex];
        return fn_1_CE14(record, position, adjustedPosition);
    }

    return 0;
}

typedef struct M620EffectModel {
    u8 unreadBeforeHookData[288]; /* Other model state is not accessed here. */
    u8 *effectData; /* Trail-model hook data. */
} M620EffectModel;

typedef struct M620EffectVectorFrame {
    Vec positions[3]; /* Current position, previous position, and carried displacement. */
} M620EffectVectorFrame;

typedef struct M620EffectBatch {
    u8 unreadBeforeFrames[20]; /* Trail settings precede the sample vectors. */
    M620EffectVectorFrame frames[32]; /* Ordered samples from the source to the endpoint. */
} M620EffectBatch;

static inline f64 m620_interpolate_component(f64 start, f64 end, f64 amount)
{
    return (1.0 - amount) * start + amount * end;
}

/* Called by fn_1_E8A8 to interpolate coordinates between the endpoint samples. */
static inline void m620_interpolate_vector(const Vec *start, const Vec *end,
                                           f64 amount, Vec *result)
{
    result->x = (f32)m620_interpolate_component(start->x, end->x, amount);
    result->y = (f32)m620_interpolate_component(start->y, end->y, amount);
    result->z = (f32)m620_interpolate_component(start->z, end->z, amount);
}

/* Called by fn_1_73A0 to straighten interior samples between the two endpoints. */
void fn_1_E8A8(s32 effectIndex)
{
    Vec interpolated;
    OMOBJ *object;
    Vec *frame;
    s32 step;
    M620EffectBatch *trail;
    M620EffectVectorFrame *firstFrame;
    M620EffectVectorFrame *lastFrame;
    s16 modelId;
    M620EffectModel *model;
    u8 *effectData;
    f64 interpolationAmount;

    if (effectIndex < 0) {
        return;
    }
    if (effectIndex >= 8) {
        return;
    }
    object = lbl_1_bss_650;
    modelId = object->mdlId[3];
    model = (M620EffectModel *)&Hu3DData[modelId];
    effectData = model->effectData;
    trail = (M620EffectBatch *)(effectData + effectIndex * sizeof(M620EffectBatch) + 16);
    firstFrame = &trail->frames[0];
    lastFrame = &trail->frames[31];
    for (step = 1; step < 31; step += 1) {
        frame = trail->frames[step].positions;
        interpolationAmount = (f64)(s32)step / 31.0;
        m620_interpolate_vector(firstFrame->positions, lastFrame->positions, interpolationAmount,
                                &interpolated);

        fn_1_CCE8(frame, &interpolated, 1.0f);
    }
}

typedef struct M620EffectPoint {
    u8 unreadBeforePosition[20]; /* This helper reads only the selected sample's position. */
    Vec position; /* World-space position displaced by the radial arc. */
    u8 unreadAfterPosition[4]; /* Not accessed by this helper. */
} M620EffectPoint;

typedef struct M620EffectPointBatch {
    u8 unreadBeforePoints[16]; /* Not accessed by this helper. */
    M620EffectPoint points[31]; /* Samples indexed while the endpoints remain fixed. */
    u8 unreadAfterPoints[40]; /* Not accessed by this helper. */
} M620EffectPointBatch;

typedef struct M620PointModelView {
    u8 unreadBeforeHookData[288]; /* Other model state is not accessed here. */
    M620EffectPointBatch *hookData; /* Trail-model hook data. */
    u8 unreadAfterHookData[16]; /* Other model callbacks and counters. */
} M620PointModelView;

/* Called by fn_1_73A0 at launch to displace interior trail points along a radial arc. */
void fn_1_EB28(s32 recordIndex, f32 amount)
{
    OMOBJ *object;
    s16 modelId;
    HU3D_MODEL *model;
    M620EffectPointBatch *effectData;
    M620EffectPoint *points;
    Vec *position;
    f32 angle;
    s32 pointIndex;

    if (recordIndex >= 0 && recordIndex < 8) {
        object = lbl_1_bss_650;
        modelId = object->mdlId[3];
        model = &Hu3DData[modelId];
        effectData = model->hookData;
        points = effectData[recordIndex].points;
        angle = (f32)(60.0 + 90.0 * (f64)recordIndex);
        pointIndex = 1;
        while (pointIndex < 31) {
            position = &points[pointIndex].position;
            position->x += amount *
                sin((3.141592653589793 * (f64)angle) / 180.0);
            position->z += amount *
                cos((3.141592653589793 * (f64)angle) / 180.0);
            pointIndex++;
            angle += 30.0;
        }
    }
}
