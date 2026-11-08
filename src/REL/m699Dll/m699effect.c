/* Dunk Bros. particle, ring, contact and camera-facing effects. */
#undef NDEBUG
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "math.h"
#include "REL/m699Dll/effect.h"

#include "REL/m699Dll/m699_natural_views.h"
#include "humath.h"
void fn_1_16CC8(void);
#include "game/object.h"
#include "game/memory.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/data.h"
extern int frand(void);
#include "string.h"

extern void fn_1_150D4(OMOBJ *obj);
extern void fn_1_16CC4(void);
extern OMOBJ *lbl_1_bss_A58;
extern OMOBJMAN *lbl_1_bss_A5C;

extern void fn_1_15130(OMOBJ *);
extern void fn_1_15170(OMOBJ *);
extern void fn_1_15470(OMOBJ *);
extern void fn_1_1568C(OMOBJ *);
extern void fn_1_174AC(OMOBJ *);
extern void fn_1_17AE0(OMOBJ *);

extern void fn_1_156E0(OMOBJ *obj);
extern void fn_1_175B0(OMOBJ *obj);
extern void fn_1_17B34(OMOBJ *obj);

extern s32 fn_1_10B8(s32 resource);
extern void fn_1_1527C(HU3D_MODEL *model, HU3D_PARTICLE *particle,
    Mtx matrix);

extern void fn_1_1557C(HU3D_MODEL *model, HU3D_PARTICLE *particle,
    Mtx matrix);

void fn_1_157C4(HU3D_MODEL *modelP, Mtx *mtx);

#include <math.h>
#include "dolphin/gx.h"
#include "dolphin/mtx.h"

#include "REL/m699Dll/m699_work_record_prefix_v3.h"

#include "dolphin/types.h"

extern Mtx *fn_1_14790(s32 index);

struct M699RingWork_s;
extern void fn_1_166F8(struct M699RingWork_s *work, s32 modelId);
extern void fn_1_17308(HU3D_MODEL *model, Mtx *view);
extern void fn_1_16D78(OMOBJ *object);

extern Vec lbl_1_bss_A40[2];

extern void *fn_1_14ACC(Vec *position, Vec *rotation);

extern void fn_1_1617C(M699RingContact *vertex);

extern void fn_1_17658(HU3D_MODEL *model, Mtx *matrix);
typedef union M699PackedColor_u {
    u32 packed;
    GXColor channels;
} M699PackedColor;

void fn_1_17B8C(HU3D_MODEL *model, Mtx *matrix);

















typedef struct M699EffectRecord {
    s16 active;
    u8 unk_2[6]; /* No code names this field. */
    f32 unknown_008;
    f32 unknown_00C;
    f32 unknown_010;
    Vec previousPosition;
    s32 unknown_020;
    u8 unk_24[0x14]; /* No code names this field. */
    Vec position;
    u8 unk_44[4]; /* No code names this field. */
} M699EffectRecord;

typedef struct M699EffectHookWork {
    u8 unk_0[0x30]; /* No code names this field. */
    s16 count;
    u8 unk_32[0x16]; /* No code names this field. */
    M699EffectRecord *records;
} M699EffectHookWork;
















extern void fn_1_16CCC(OMOBJ *obj);

extern OMOBJ *lbl_1_bss_A38[2];











extern Vec *fn_1_14778(s32 index);































static inline f32 RingDrop166F8(void)
{
    return 0.125f;
}



/* Create the effect controller and clear its work allocation. */
void M699EffectObjectInit(OMOBJMAN *objectManager) {
    OMOBJ *object;
    void *data;

    lbl_1_bss_A5C = objectManager;
    object = fn_8003AB38(lbl_1_bss_A5C, 0x10, 5, 0, -1, fn_1_150D4);
    lbl_1_bss_A58 = object;
    data = HuMemDirectMallocNum(HEAP_HEAP, 0xB0, 0x10000000);
    object->data = data;
    memset(data, 0, 0xB0);
    fn_1_16CC4();
}

void fn_1_150B4(s16 mode, s16 frameNo)
{
    fn_1_16CC8();
}

/* Initialize the effect services and install the update callback. */
void fn_1_150D4(OMOBJ *object) {
    fn_1_15170(object);
    fn_1_15470(object);
    fn_1_1568C(object);
    fn_1_174AC(object);
    fn_1_17AE0(object);
    object->objFunc = fn_1_15130;
}

void fn_1_15130(OMOBJ *obj) {
    fn_1_156E0(obj);
    fn_1_175B0(obj);
    fn_1_17B34(obj);
}

void fn_1_15170(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(fn_80008DD0(fn_1_10B8(0xC), 0x10000000,
                                                    2));
    modelId = fn_8003216C(animation, 0x100);
    object->mdlId[0] = modelId;
    particle = lbl_802C01C8[(s16)modelId].hookData;
    fn_8003269C(modelId, 0);
    fn_800326D0(modelId, fn_1_1527C);
    fn_80026B8C(modelId, 6);
    fn_80026760(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

void fn_1_1527C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry = particle->data;
    s32 alpha;
    s32 index = 0;

    while (index < particle->maxCnt) {
        switch (entry->time) {
        case 0:
            entry->time++;
            entry->vel.x = (f32)(3.0517578125e-05 *
                (f32)((u16)frand() - 0x8000));
            entry->vel.y = (f32)(2.0 +
                (1.52587890625e-05 * (f32)(frand() & 0xFFFF)));
            entry->vel.z = (f32)(3.0517578125e-05 *
                (f32)((u16)frand() - 0x8000));
            entry->color.r = 0xC0;
            entry->color.g = 0xC0;
            entry->color.b = 0x60;
            entry->color.a = 0x60;
            entry->scale = 80.0f;
            /* fall through */
        case 1:
            PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
            alpha = entry->color.a;
            alpha -= 4;
            if (alpha < 0) {
                alpha = 0;
                entry->scale = 0.0f;
                entry->time = -1;
            }
            entry->color.a = (u8)alpha;
            break;
        }
        index++;
        entry++;
    }
    DCStoreRangeNoSync(particle->data,
                       particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
    PPCSync();
}

void fn_1_15470(OMOBJ *object)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_MODELID modelId;
    ANIMDATA *animation;
    HU3D_PARTICLE *particle;
    s32 index;

    animation = HuSprAnimRead(fn_80008DD0(fn_1_10B8(0xC), 0x10000000, 2));
    modelId = fn_8003216C(animation, 0x100);
    object->mdlId[1] = modelId;
    particle = (HU3D_PARTICLE *)lbl_802C01C8[(s16)modelId].hookData;
    fn_8003269C(modelId, 0);
    fn_800326D0(modelId, fn_1_1557C);
    fn_80026B8C(modelId, 6);
    fn_80026760(modelId, 0.0f, 0.0f, 0.0f);
    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        entry->time = -1;
        entry->scale = 0.0f;
    }
}

void fn_1_1557C(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry;
    s32 index;
    s32 alpha;

    entry = particle->data;
    for (index = 0; index < particle->maxCnt; index++, entry++) {
        switch (entry->time) {
        case 0:
            entry->time++;
            entry->color.r = 0xE0;
            entry->color.g = 0xE0;
            entry->color.b = 0x80;
            entry->color.a = 0x80;
            entry->scale = 50.0f;
            /* fall through */
        case 1:
            PSVECAdd(&entry->pos, &entry->accel, &entry->pos);
            alpha = entry->color.a;
            alpha -= 5;
            if (alpha < 0) {
                alpha = 0;
                entry->scale = 0.0f;
                entry->time = -1;
            }
            entry->color.a = (u8)alpha;
            break;
        }
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * 0x48);
    PPCSync();
}

void fn_1_1568C(OMOBJ *self)
{
    HU3D_MODELID hook = fn_800260F4(fn_1_157C4);

    self->mdlId[2] = hook;
    fn_80026B8C(hook, 6);
}

void fn_1_156E0(OMOBJ *self)
{
    M699RingDisplay *data;
    M699RingDisplay *record;
    s32 index;

    data = self->data;
    record = data;
    for (index = 0; index < 4; index++, record++) {
        switch (record->active) {
        case 0:
            break;
        case 1:
            record->active++;
            /* fall through */
        case 2:
            record->outerRadius += 20.0;
            record->innerRadius += 30.0;
            if ((record->opacity -= 0.05) > 0.0f) {
                continue;
            }
            record->active = 0;
            break;
        default:
            break;
        }
    }
}

void fn_1_157C4(HU3D_MODEL *model, Mtx *matrix)
{
    M699RingDisplay *display;
    M699RingDisplay *ringList;
    Mtx transform;
    f32 angle;
    f32 nextAngle;
    f32 angleStep;
    s32 index;

    ringList = lbl_1_bss_A58->data;
    display = ringList;
    angleStep = 15.0f;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
    GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
    GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);

    for (index = 0; index < 4; index++, display++) {
        if (display->active != 0) {
            Mtx translation;
            f32 outerRadius = display->outerRadius;
            f32 innerRadius = display->innerRadius;
            M699PackedColor outerColor;
            M699PackedColor innerColor;

            outerColor.packed = display->color;
            outerColor.channels.a = (u8)(63.0 * (f64)display->opacity);
            innerColor.packed = display->color;
            innerColor.channels.a = (u8)(255.0 * (f64)display->opacity);
            PSMTXIdentity(transform);
            PSMTXTrans(transform, display->position.x, display->position.y, display->position.z);
            PSMTXConcat(*matrix, transform, transform);
            GXLoadPosMtxImm(transform, GX_PNMTX0);

            for (angle = 0.0f; (f64)angle < 360.0; angle += angleStep) {
                nextAngle = angle + angleStep;
                if (nextAngle >= 360.0) {
                    nextAngle = 0.0f;
                }
                GXBegin(GX_QUADS, GX_VTXFMT0, 4);
                GXPosition3f32(outerRadius * sin((M_PI * (f64)angle) / 180.0), 0.0f, outerRadius * cos((M_PI * (f64)angle) / 180.0));
                {
                    u32 packedColor = outerColor.packed;
                    GXColor1u32(packedColor);
                }
                GXPosition3f32(outerRadius * sin((M_PI * (f64)nextAngle) / 180.0), 0.0f, outerRadius * cos((M_PI * (f64)nextAngle) / 180.0));
                {
                    u32 packedColor = outerColor.packed;
                    GXColor1u32(packedColor);
                }
                GXPosition3f32(innerRadius * sin((M_PI * (f64)nextAngle) / 180.0), 0.0f, innerRadius * cos((M_PI * (f64)nextAngle) / 180.0));
                {
                    u32 packedColor = innerColor.packed;
                    GXColor1u32(packedColor);
                }
                GXPosition3f32(innerRadius * sin((M_PI * (f64)angle) / 180.0), 0.0f, innerRadius * cos((M_PI * (f64)angle) / 180.0));
                {
                    u32 packedColor = innerColor.packed;
                    GXColor1u32(packedColor);
                }
            }
        }
    }
}

void fn_1_15D08(M699MotionPointRecord *destination,
                const Point3d *source, s32 value) {
    destination->inactive = value;
    destination->position = *source;
    destination->previousPosition = *source;
    destination->velocity.x = 0.0f;
    destination->velocity.y = 0.0f;
    destination->velocity.z = 0.0f;
}

/* Advance an active point and apply its downward acceleration. */
void fn_1_15D70(M699MotionPointRecord *work) {
    if (work->inactive == 0) {
        PSVECAdd(&work->position, &work->velocity, &work->position);
        work->position.y =
            (f32)((f64)work->position.y - 0.27240694444444447);
    }
}

void fn_1_15DD0(M699MotionPointRecord *work, Vec *first,
                f32 distanceLimit, Vec *second)
{
    Vec difference;
    f32 distance;

    if (work->inactive != 0) {
        return;
    }
    PSVECSubtract(&work->position, first, &difference);
    distance = PSVECMag(&difference);
    if (distance >= distanceLimit) {
        return;
    }

    distance = (distanceLimit - distance) / distance;
    if (isnan(distance) || isinf(distance)) {
        return;
    }
    PSVECScale(&difference, &difference, distance);
    PSVECAdd(&work->position, &difference, &work->position);
    PSVECAdd(second, &difference, second);
}

/* Derive damped velocity from the previous position and save the current position. */
void fn_1_15F94(M699MotionPointRecord *work) {
    if (work->inactive == 0) {
        PSVECSubtract(&work->position, &work->previousPosition, &work->velocity);
        PSVECScale(&work->velocity, &work->velocity, 0.9f);
        work->previousPosition = work->position;
    }
}

void fn_1_16008(M699RingContact *result, M699MotionPointRecord *first,
                M699MotionPointRecord *second) {
    Vec difference;

    PSVECSubtract(&first->position, &second->position, &difference);
    result->first = first;
    result->second = second;
    result->distance = PSVECMag(&difference);
}

void fn_1_1606C(M699RingContact *work)
{
    Vec difference;
    f32 distance;
    M699MotionPointRecord *first;
    M699MotionPointRecord *second;

    first = work->first;
    second = work->second;
    PSVECSubtract(&second->position, &first->position, &difference);
    distance = PSVECMag(&difference);
    if (!(distance <= work->distance)) {
        distance = (distance - work->distance) / distance;
        PSVECScale(&difference, &difference, distance);
        if (first->inactive != 0) {
            PSVECSubtract(&second->position, &difference, &second->position);
            return;
        }
        if (second->inactive != 0) {
            PSVECAdd(&first->position, &difference, &first->position);
            return;
        }
        PSVECScale(&difference, &difference, 0.5f);
        PSVECAdd(&first->position, &difference, &first->position);
        PSVECSubtract(&second->position, &difference, &second->position);
    }
}

void fn_1_1617C(M699RingContact *pair)
{
    Vec *first;
    Vec *second;
    Vec cameraPosition;
    Vec cameraTarget;
    Vec direction;
    Vec cameraDirection;
    Vec firstSide;
    Vec secondSide;
    f32 crossMagnitude;

    first = &pair->first->position;
    second = &pair->second->position;
    fn_80027CD0(1, &cameraPosition, &cameraTarget, &cameraTarget);
    PSVECSubtract(second, first, &direction);
    PSVECNormalize(&direction, &direction);
    PSVECSubtract(first, &cameraPosition, &cameraDirection);
    PSVECNormalize(&cameraDirection, &cameraDirection);
    PSVECCrossProduct(&direction, &cameraDirection, &firstSide);
    crossMagnitude = PSVECMag(&firstSide);
    if (crossMagnitude <= 0.0f) {
        return;
    }
    PSVECScale(&firstSide, &firstSide, 1.5f / crossMagnitude);

    PSVECSubtract(second, &cameraPosition, &cameraDirection);
    PSVECNormalize(&cameraDirection, &cameraDirection);
    PSVECCrossProduct(&direction, &cameraDirection, &secondSide);
    if (crossMagnitude <= 0.0f) {
        return;
    }
    PSVECScale(&secondSide, &secondSide, 1.5f / crossMagnitude);

    GXBegin(GX_TRIANGLESTRIP, GX_VTXFMT0, 6);
    GXPosition3f32(first->x - firstSide.x, first->y - firstSide.y, first->z - firstSide.z);
    GXWGFifo.u16 = 0xA514;
    GXPosition3f32(second->x - secondSide.x, second->y - secondSide.y, second->z - secondSide.z);
    GXWGFifo.u16 = 0xA514;
    GXPosition3f32(first->x, first->y, first->z);
    GXWGFifo.u16 = 0xDEFB;
    GXPosition3f32(second->x, second->y, second->z);
    GXWGFifo.u16 = 0xDEFB;
    GXPosition3f32(first->x + firstSide.x, first->y + firstSide.y, first->z + firstSide.z);
    GXWGFifo.u16 = 0xA514;
    GXPosition3f32(second->x + secondSide.x, second->y + secondSide.y, second->z + secondSide.z);
    GXWGFifo.u16 = 0xA514;
}

void fn_1_16570(M699MotionPointRecord *records, s32 matrixIndex)
{
    Vec position;
    Mtx *matrix;
    f32 angle;
    s32 index;
    M699MotionPointRecord *record;

    matrix = fn_1_14790(matrixIndex);
    for (index = 0; index < 12; index++) {
        record = &records[index];
        angle = (f32)(((M_PI * 360.0 / 180.0) * (f64)index) / 12.0);
        position.x = (f32)(42.0 * sin((f64)angle));
        position.y = 0.0f;
        position.z = (f32)(42.0 * cos((f64)angle));
        PSMTXMultVec(*matrix, &position, &position);
        fn_1_15D08(record, &position, 1);
    }
}

void fn_1_166F8(M699RingWork *work, s32 modelId)
{
    Mtx *matrix;
    f32 radius;
    s32 ring;
    s32 index;
    f32 height;
    f32 angle;
    Vec position;
    M699MotionPointRecord *point;
    M699RingContact *link;
    M699RingContact *link_end;
    s32 linkRing;
    s32 linkIndex;

    matrix = fn_1_14790(modelId);
    radius = 33.6f;
    fn_1_16570(work->particles[0], modelId);

    for (ring = 1; ring < 9; ring++) {
        height = -(70.0f * ring) * RingDrop166F8();
        for (index = 0; index < 12; index++) {
            point = &work->particles[ring][index];
            angle = ((M_PI * 360.0 / 180.0) * (index - 0.5 * ring)) / 12.0;
            position.x = radius * sin(angle);
            position.y = height - 20.0;
            position.z = radius * cos(angle);
            PSMTXMultVec(*matrix, &position, &position);
            fn_1_15D08(point, &position, 0);
        }
        radius *= 0.98;
    }

    link = &work->contacts[0];
    link_end = link + 204;
    for (linkRing = 0; linkRing < 8; linkRing++) {
        for (linkIndex = 0; linkIndex < 12; linkIndex++) {
            M699MotionPointRecord *first = &work->particles[linkRing][linkIndex];
            M699MotionPointRecord *second = &work->particles[linkRing + 1][linkIndex];
            M699MotionPointRecord *diagonal = &work->particles[linkRing + 1][(linkIndex + 1) % 12];

            fn_1_16008(link++, first, second);
            fn_1_16008(link++, first, diagonal);
        }
    }

    for (linkIndex = 0; linkIndex < 12; linkIndex++) {
        M699MotionPointRecord *first = &work->particles[8][linkIndex];
        M699MotionPointRecord *second = &work->particles[8][(linkIndex + 1) % 12];

        fn_1_16008(link++, first, second);
    }

    assert(link == link_end);
    lbl_1_bss_A40[0].x = 0.0f;
    lbl_1_bss_A40[0].y = 0.0f;
    lbl_1_bss_A40[0].z = 0.0f;
    lbl_1_bss_A40[1].x = 0.0f;
    lbl_1_bss_A40[1].y = 0.0f;
    lbl_1_bss_A40[1].z = 0.0f;
}







void fn_1_16CC4(void) {
}







void fn_1_16CC8(void) {
}







void fn_1_16CCC(OMOBJ *object)
{
    HU3D_MODELID modelId;
    HU3D_MODEL *model;
    void *work;

    modelId = fn_800260F4(fn_1_17308);
    model = &lbl_802C01C8[(s16)modelId];
    work = HuMemDirectMallocNum(HEAP_HEAP, 0x1A70, 0x10000000);
    fn_80026B8C((s16)modelId, 3);
    object->work[1] = (s16)modelId;
    model->hookData = work;
    fn_1_166F8(work, (s16)modelId);
    object->objFunc = fn_1_16D78;
}







void fn_1_16D78(OMOBJ *object)
{
    s16 modelIndex;
    M699RingWork *work;
    Vec cameraPosition;
    Vec *center;
    M699MotionPointRecord *points;
    M699MotionPointRecord *particle;
    M699MotionPointRecord *particleEnd;
    M699RingContact *contact;
    M699RingContact *contactEnd;
    s16 side;
    HU3D_MODEL *model;

    modelIndex = (s16)object->work[1];
    side = modelIndex & 1;
    model = &lbl_802C01C8[modelIndex];
    work = model->hookData;
    center = &lbl_1_bss_A40[side];
    fn_1_14ACC(&cameraPosition, NULL);
    fn_1_16570(work->particles[0], (s16)side);

    points = &work->particles[0][0];
    particleEnd = points + 9 * 12;
    contact = work->contacts;
    contactEnd = contact + 204;
    particle = points;
    while (particle != particleEnd) {
        fn_1_15D70(particle);
        particle++;
    }

    center->x = 0.0f;
    center->y = 0.0f;
    center->z = 0.0f;
    particle = points;
    while (particle != particleEnd) {
        fn_1_15DD0(particle, &cameraPosition,
                   36.0f, center);
        particle++;
    }
    center->x *= -0.1;
    center->y *= -0.02;
    center->z *= -0.1;

    while (contact != contactEnd) {
        fn_1_1606C(contact);
        contact++;
    }

    particle = points;
    while (particle != particleEnd) {
        fn_1_15F94(particle);
        particle++;
    }
}

const GXVtxDescList lbl_1_rodata_8D0[3] = {
    { GX_VA_POS, GX_DIRECT }, { GX_VA_CLR0, GX_DIRECT }, { GX_VA_NULL,
    GX_NONE }
};
const GXVtxAttrFmtList lbl_1_rodata_8E8[3] = {
    { GX_VA_POS, GX_POS_XYZ, GX_F32, 0 },
    { GX_VA_CLR0, GX_CLR_RGB, GX_RGB565, 0 },
    { GX_VA_NULL, 0, 0, 0 }
};







void fn_1_17308(HU3D_MODEL *model, Mtx *view)
{
    M699RingWork *work;
    M699RingContact *vertex;
    M699RingContact *end;

    GXLoadPosMtxImm(*view, GX_PNMTX0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0, GX_DISABLE, GX_SRC_REG, GX_SRC_VTX,
                  GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL,
    GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_RASC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_KONST);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetCullMode(GX_CULL_NONE);
    GXSetColorUpdate(GX_TRUE);
    GXSetAlphaUpdate(GX_FALSE);
    GXSetZCompLoc(GX_TRUE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0xFF, GX_AOP_OR, GX_ALWAYS, 0xFF);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_NOOP);
    GXClearVtxDesc();
    GXSetVtxDescv(lbl_1_rodata_8D0);
    GXSetVtxAttrFmtv(GX_VTXFMT0, lbl_1_rodata_8E8);

    work = model->hookData;
    vertex = work->contacts;
    end = vertex + 192;
    while (vertex != end) {
        fn_1_1617C(vertex);
        vertex++;
    }
}







void fn_1_174AC(OMOBJ *object)
{
    HU3D_MODELID modelId;
    HU3D_MODEL *model;
    M699CaptureBuffer *work;
    u32 textureSize;

    modelId = fn_800260F4(fn_1_17658);
    object->mdlId[3] = modelId;
    fn_80026B8C(modelId, 7);
    model = &lbl_802C01C8[(s16)modelId];
    work = HuMemDirectMallocNum(HEAP_HEAP, sizeof(M699CaptureBuffer),
    0x10000000);
    model->hookData = work;
    memset(work, 0, sizeof(M699CaptureBuffer));
    work->state = work->capturePending = 0;
    work->positionX = work->positionY = 0.0f;
    work->scaleX = work->scaleY = 1.0f;
    work->texture = HuMemDirectMallocNum(
        HEAP_HEAP, GXGetTexBufferSize(0x280, 0x1E0, GX_TF_RGB565, 0, 0),
    0x10000000);
}







void fn_1_175B0(OMOBJ *object)
{
    HU3D_MODELID modelId = object->mdlId[3];
    HU3D_MODEL *model = &lbl_802C01C8[(s32)modelId];
    M699CaptureBuffer *hook = model->hookData;

    hook->positionX = (float)(hook->positionX +
                           0.3 * (hook->positionY - hook->positionX));
    hook->scaleX = (float)(hook->scaleX +
                           0.1 * (hook->scaleY - hook->scaleX));
    hook->capturePending = 1;
}







void fn_1_17658(HU3D_MODEL *model, Mtx *matrix)
{
    M699CaptureBuffer *buffer;
    GXTexObj texture;
    Mtx projectionMatrix;
    Mtx textureMatrix;
    Mtx translationMatrix;
    s32 color;

    buffer = model->hookData;
    if (buffer->positionX <= 0.01 &&
        buffer->positionY <= 0.01) {
        buffer->state = 0;
        buffer->positionX = buffer->positionY = 0.0f;
        return;
    }

    if (buffer->state != 0) {
        color = (u32)(255.0f * buffer->positionX) - 0x100;
        GXInitTexObj(&texture, buffer->texture, 0x280, 0x1E0,
                     GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_FALSE);
        GXLoadTexObj(&texture, GX_TEXMAP0);
        PSMTXTrans(textureMatrix, 0.5f, 0.5f, 0.0f);
        PSMTXScaleApply(textureMatrix, textureMatrix, buffer->scaleX,
                        buffer->scaleX, 1.0f);
        PSMTXTrans(translationMatrix, -0.5f, -0.5f, 0.0f);
        PSMTXConcat(textureMatrix, translationMatrix, textureMatrix);
        GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);
        GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                          GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
        C_MTXOrtho(projectionMatrix, 0.0f, 480.0f, 0.0f, 640.0f,
                   0.0f, 100.0f);
        GXSetProjection(projectionMatrix, GX_ORTHOGRAPHIC);
        PSMTXIdentity(textureMatrix);
        GXLoadPosMtxImm(textureMatrix, GX_PNMTX0);
        GXSetCullMode(GX_CULL_NONE);
        GXSetZMode(GX_FALSE, GX_LEQUAL, GX_FALSE);
        GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA,
                       GX_BL_INVSRCALPHA, GX_LO_CLEAR);
        GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
        GXSetNumChans(1);
        GXSetNumTexGens(1);
        GXSetNumTevStages(1);
        GXSetTevOp(GX_TEVSTAGE0, GX_MODULATE);
        GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
        GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX,
                      GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
        GXClearVtxDesc();
        GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
        GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
        GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_S, GX_RGBX8, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
        GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_RGBA6, 0);
        GXBegin(GX_QUADS, GX_VTXFMT0, 4);
        GXPosition2s16(0, 0);
        GXColor1u32(color);
        GXTexCoord2f32(0.0f, 0.0f);
        GXPosition2s16(0x280, 0);
        GXColor1u32(color);
        GXTexCoord2f32(1.0f, 0.0f);
        GXPosition2s16(0x280, 0x1E0);
        GXColor1u32(color);
        GXTexCoord2f32(1.0f, 1.0f);
        GXPosition2s16(0, 0x1E0);
        GXColor1u32(color);
        GXTexCoord2f32(0.0f, 1.0f);
    }

    if (buffer->capturePending != 0) {
        fn_8002BCDC(0, 0, 0x280, 0x1E0, GX_TF_RGB565,
                       0, buffer->texture);
        buffer->state = 1;
        buffer->capturePending = 0;
    }
}







void fn_1_17AE0(OMOBJ *self)
{
    HU3D_MODELID hook = fn_800260F4(fn_1_17B8C);

    self->mdlId[4] = hook;
    fn_80026B8C(hook, 6);
}







void fn_1_17B34(OMOBJ *object)
{
    M699TransitionWork *work = object->data;
    M699TransitionState *transition = &work->transition;

    switch (transition->active) {
    case 0:
        break;
    case 1:
        if (--transition->mode <= 0) {
            transition->active = 0;
        }
        break;
    default:
        break;
    }
}







void fn_1_17B8C(HU3D_MODEL *owner, Mtx *matrix)
{
    M699TransitionWork *objectData;
    M699TransitionState *work;
    Vec origin;
    Vec extent;
    Mtx transform;
    f32 halfWidth;
    u32 color;

    objectData = lbl_1_bss_A58->data;
    work = &objectData->transition;
    if (work->active == 0) {
        return;
    }

    halfWidth = 10.0f;
    color = 0x4040C0C0;
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LESS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
    GX_LO_CLEAR);
    GXSetAlphaCompare(GX_GREATER, 0, GX_AOP_AND, GX_LEQUAL, 0xFF);
    GXSetNumChans(1);
    GXSetNumTexGens(0);
    GXSetNumTevStages(1);
    GXSetTevOp(GX_TEVSTAGE0, GX_PASSCLR);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD_NULL, GX_TEXMAP_NULL, GX_COLOR0A0);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_NONE, GX_AF_NONE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_TEX_ST, GX_RGBA6, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_TEX_ST, GX_RGBA8, 0);
    GXSetLineWidth(0xC, GX_TO_ZERO);

    origin = work->start;
    extent = work->destination;
    while (origin.y > 0.0) {
        PSMTXIdentity(transform);
        PSMTXTrans(transform, origin.x, origin.y, origin.z);
        PSMTXConcat(*matrix, transform, transform);
        GXLoadPosMtxImm(transform, GX_PNMTX0);
        GXBegin(GX_LINESTRIP, GX_VTXFMT0, 5);
        GXPosition3f32(-halfWidth, -halfWidth, 0.0f);
        GXColor1u32(color);
        GXPosition3f32(halfWidth, -halfWidth, 0.0f);
        GXColor1u32(color);
        GXPosition3f32(halfWidth, halfWidth, 0.0f);
        GXColor1u32(color);
        GXPosition3f32(-halfWidth, halfWidth, 0.0f);
        GXColor1u32(color);
        GXPosition3f32(-halfWidth, -halfWidth, 0.0f);
        GXColor1u32(color);
        extent.y = (f32)((f64)extent.y - 1.0896277777777779);
        PSVECAdd(&origin, &extent, &origin);
    }
}







void fn_1_17EB0(Vec *position, Vec *previousPosition)
{
    OMOBJ *object;
    HU3D_MODELID modelId;
    M699EffectHookWork *work;
    M699EffectRecord *record;
    s32 index;

    object = lbl_1_bss_A58;
    modelId = object->mdlId[0];
    work = lbl_802C01C8[modelId].hookData;
    record = work->records;
    for (index = 0; index < work->count; index++, record++) {
        if (record->active < 0) {
            record->active = 0;
            record->position = *position;
            record->previousPosition = *previousPosition;
            break;
        }
    }
}







void fn_1_17F74(Vec *position, Vec *previousPosition)
{
    OMOBJ *object;
    HU3D_MODELID modelId;
    M699EffectHookWork *work;
    M699EffectRecord *record;
    s32 index;

    object = lbl_1_bss_A58;
    modelId = object->mdlId[1];
    work = lbl_802C01C8[modelId].hookData;
    record = work->records;
    for (index = 0; index < work->count; index++, record++) {
        if (record->active < 0) {
            record->active = 0;
            record->position = *position;
            record->previousPosition = *previousPosition;
            break;
        }
    }
}







void fn_1_18038(Vec *position)
{
    M699RingDisplay *initialRecords;
    M699RingDisplay *initialRecord;
    Vec samplePosition;
    Vec difference;
    s32 index;

    initialRecords = lbl_1_bss_A58->data;
    initialRecord = initialRecords;
    for (index = 0; index < 4; index++, initialRecord++) {
        if (initialRecord->active == 0) {
            initialRecord->active++;
            initialRecord->position = *position;
            initialRecord->color = -1;
            initialRecord->outerRadius = 10.0f;
            initialRecord->innerRadius = 50.0f;
            initialRecord->opacity = 1.0f;
            break;
        }
    }

    for (index = 0; index < 0x80; index++) {
        samplePosition.x = (f32)((f64)position->x +
            (400.0 * (3.0517578125e-05 *
                (f32)((u16)frand() - 0x8000))));
        samplePosition.y = (f32)((f64)100.0 +
            ((f64)position->y +
                (200.0 * (1.52587890625e-05 * (f32)(frand() & 0xFFFF)))));
        samplePosition.z = (f32)((f64)position->z +
            (400.0 * (3.0517578125e-05 *
                (f32)((u16)frand() - 0x8000))));
        PSVECSubtract(position, &samplePosition, &difference);
        PSVECScale(&difference, &difference, 0.03f);

        fn_1_17F74(&samplePosition, &difference);
    }
}


/* Create the ring-update object and store it in the player pair slot. */
void fn_1_18300(u32 playerIndex) {
    OMOBJ *obj = fn_8003AB38(lbl_1_bss_A5C, 0x10, 0, 0, -1, fn_1_16CCC);

    lbl_1_bss_A38[playerIndex & 1] = obj;
    obj->work[0] = playerIndex;
}

s32 fn_1_18378(Vec *start, Vec *end, Vec *plane)
{
    f32 planeHeight;
    f32 endHeight;
    f32 startHeight;
    f64 crossingFraction;
    f64 horizontalX;
    f64 horizontalZ;

    planeHeight = plane->y;
    if (end->y >= planeHeight && start->y <= planeHeight) {
        endHeight = end->y;
        startHeight = start->y;
        crossingFraction = (f64)endHeight /
            ((f64)endHeight - (f64)startHeight);
        horizontalX = (crossingFraction * (f64)start->x +
            ((1.0 - crossingFraction) * (f64)end->x)) - (f64)plane->x;
        horizontalZ = (crossingFraction * (f64)start->z +
            ((1.0 - crossingFraction) * (f64)end->z)) - (f64)plane->z;
        if (sqrt((horizontalX * horizontalX) +
                 (horizontalZ * horizontalZ)) < 37.0) {
            return 1;
        }
    }
    return 0;
}

s32 fn_1_18640(Vec *position, Vec *separation, s32 playerIndex)
{
    Vec center;
    Vec direction;
    Vec displacement;
    f32 distance;

    center = *fn_1_14778(playerIndex);
    PSVECSubtract(position, &center, &direction);
    direction.y = 0.0f;
    PSVECScale(&direction, &direction, (f32)(42.0 / PSVECMag(&direction)));
    PSVECAdd(&center, &direction, &direction);
    PSVECSubtract(position, &direction, &displacement);
    distance = PSVECMag(&displacement);
    if (distance > 0.0f && distance < 30.0) {
        PSVECScale(&displacement, separation,
                   (f32)((35.0 - (f64)distance) / (f64)distance));
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







void fn_1_18B0C(Vec *velocity)
{
    Vec cameraPosition;
    Vec separation;
    s32 index;

    assert(velocity != NULL);
    velocity->x = 0.0f;
    velocity->y = 0.0f;
    velocity->z = 0.0f;
    fn_1_14ACC(&cameraPosition, NULL);
    for (index = 0; index < 2; index++) {
        PSVECAdd(velocity, &lbl_1_bss_A40[index], velocity);
        if (fn_1_18640(&cameraPosition, &separation, index)) {
            PSVECAdd(velocity, &separation, velocity);
            return;
        }
    }
}

/* Select one of the two ring vectors from the sign of the supplied value. */
Vec *fn_1_19080(const f32 *value) {
    s32 vectorIndex;

    if (*value < 0.0) {
        vectorIndex = 0;
    } else {
        vectorIndex = 1;
    }

    return &lbl_1_bss_A40[vectorIndex];
}

void fn_1_190C8(f32 firstValue, f32 secondValue)
{
    OMOBJ *object = lbl_1_bss_A58;
    s16 modelId = object->mdlId[3];
    HU3D_MODEL *model = &lbl_802C01C8[modelId];
    M699CaptureBuffer *hookData = model->hookData;

    hookData->positionY = firstValue;
    hookData->scaleY = secondValue;
}

void fn_1_1912C(s32 mode, Vec *start, Vec *destination)
{
    OMOBJ *object = lbl_1_bss_A58;
    M699TransitionWork *work = object->data;
    M699TransitionState *transition = &work->transition;

    if (transition->active == 0) {
        transition->active++;
    }
    transition->mode = mode;
    transition->start = *start;
    transition->destination = *destination;
}

OMOBJMAN *lbl_1_bss_A5C;
OMOBJ *lbl_1_bss_A58;
Vec lbl_1_bss_A40[2];
OMOBJ *lbl_1_bss_A38[2];
