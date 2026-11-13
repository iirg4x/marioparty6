#include "REL/m604DLL/camera_masks.h"
#include "REL/framework/singleton.h"
#include "REL/m604DLL/camera_owner.h"
#include "REL/m604DLL/effect_calls.h"
#include "REL/m604DLL/sprite_parameters.h"
#include "REL/m604DLL/player_particle_effect.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
extern "C" {
#include "math.h"
#include "string.h"
#include "game/hu3d.h"
#include "game/data.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "game/frand.h"
#include "game/sprite.h"
}

#include "REL/m604DLL/player_position_history.h"

#include "REL/m604DLL/camera_hook_owner.h"

#include "REL/framework/linknode.h"

#include "REL/m604DLL/stage_effects.h"

void fn_1_2180(PlayerParticleEffect *effect, Mtx data);

void fn_1_1CEC(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx data)
{
    PlayerParticleEffect *effect = static_cast<PlayerParticleEffect *>(particle->work);
    fn_1_2180(effect, data);
}

void fn_1_1CEC(HU3D_MODEL *, HU3D_PARTICLE *callback, Mtx data);

/* Starts a transparent, additive particle effect on the selected cameras. */
PlayerParticleEffect::PlayerParticleEffect(const Vec *sourcePosition, s32 cameraMask)
{
    s16 particleModel;

    enabled = 0;
    finishing = 0;
    animation = HuSprAnimRead(fn_80008DD0(0x3F001A, 0x10000000, 2));
    HuSprAnimLock(animation);
    particleModel = fn_8003216C(animation, 0x20);
    particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[(s16)particleModel].hookData);
    particle->work = this;
    fn_800326D0((s16)particleModel, fn_1_1CEC);
    fn_800325B0((s16)particleModel, 0xFFU, 0xFFU, 0xFFU);
    fn_80032620((s16)particleModel, 0.0f);
    fn_8003269C((s16)particleModel, 1U);
    fn_800326D0((s16)particleModel, fn_1_1CEC);
    fn_80026B8C((s16)particleModel, 5);
    fn_80026B64((s16)particleModel, cameraMask);
    fn_80026760((s16)particleModel, 0.0f, 0.0f, 0.0f);
    fn_80026808((s16)particleModel, 0.0f, 0.0f, 0.0f);
    position.x = sourcePosition->x;
    position.y = sourcePosition->y;
    position.z = sourcePosition->z;
    storedVector.x = 0.0f;
    storedVector.y = 0.0f;
    storedVector.z = 0.0f;
    modelId = (s16)particleModel;
}

PlayerParticleEffect::~PlayerParticleEffect()
{
    particle->work = 0;
    fn_800262D0(modelId);
    HuSprAnimKill(animation);
}

// Starts one particle: fading in, within 50 units of the effect's position in x and z, a random size, angle and
// colour. The particle data's time counts its phase (0 fading in, 1 fading out, 0xFF gone) and parManId its fade
// step.
void fn_1_1F60(PlayerParticleEffect *effect, HU3D_PARTICLE_DATA *particle)
{
    u32 color;

    particle->time = 0;
    particle->parManId = (s16)((frand() & 0xF) + 4);
    particle->pos.x = effect->position.x + (50.0f * (0.000030517578f * ((f32)(u16)frand() - 32768.0f)));
    particle->pos.y = effect->position.y;
    particle->pos.z = effect->position.z + (50.0f * (0.000030517578f * ((f32)(u16)frand() - 32768.0f)));
    particle->scale = 100.0f * (0.2f + (0.2f * (0.000015258789f * (f32)(u16)frand())));
    particle->zRot = MTXDegToRad((360.0f * (0.000015258789f * (f32)(u16)frand())));
    color = frand();
    particle->color.r = color;
    particle->color.g = (color >> 8) & 0xFF;
    particle->color.b = (color >> 16) & 0xFF;
    particle->color.a = 0;
}

/* Advances each particle's fade and motion, then enables its model after all
 * particle slots have left the uninitialized state. */
void fn_1_2180(PlayerParticleEffect *effect, Mtx data)
{
    HU3D_MODEL *model;
    HU3D_PARTICLE *particles;
    HU3D_PARTICLE_DATA *particle;
    s32 particleIndex;
    u8 allGone;

    if (effect->enabled != 0) {
        model = &lbl_802C01C8[effect->modelId];
        particles = effect->particle;
        allGone = 1;
        for (particleIndex = 0, particle = particles->data;
             particleIndex < particles->maxCnt;
             particleIndex++, particle++) {
            if (particle->time == 0xFF) {
                if (effect->finishing != 0) {
                    continue;
                }
                fn_1_1F60(effect, particle);
            }
            if (particle->time == 0) {
                particle->color.a += 0x10;
                if (particle->color.a > 0xEE) {
                    particle->color.a = 0xFF;
                    particle->time++;
                }
            } else {
                particle->color.a -= particle->parManId;
                if (particle->color.a < 0x14) {
                    particle->color.a = 0;
                    particle->time = 0xFF;
                }
            }
            particle->zRot += MTXDegToRad(1.0f);
            particle->pos.y = (f32)((f64)particle->pos.y - 1.6666666666666667);
            allGone = 0;
        }
        if (allGone == 1) {
            effect->finishing = 1;
            fn_800269E8(effect->modelId, 1);
        }
    }
}

// Moves the effect to position and shows it, renewing its particles.
void fn_1_24C0(PlayerParticleEffect *effect, const Vec *position)
{
    effect->position.x = position->x;
    effect->position.y = position->y;
    effect->position.z = position->z;
    effect->enabled = 1;
    effect->finishing = 0;
    fn_80026A44(effect->modelId, 1U);
}

void fn_1_2528(PlayerParticleEffect *effect, const Vec *value)
{
    effect->storedVector.x = value->x;
    effect->storedVector.y = value->y;
    effect->storedVector.z = value->z;
}

// Starts the effect and every one of its particles.
void fn_1_2544(PlayerParticleEffect *effect)
{
    s32 particleIndex;
    HU3D_PARTICLE *particles;
    HU3D_PARTICLE_DATA *particle;

    particles = effect->particle;
    particle = particles->data;
    effect->enabled = 1;
    effect->finishing = 0;
    particleIndex = 0;
    while (particleIndex < particles->maxCnt) {
        fn_1_1F60(effect, particle);
        particleIndex += 1;
        particle++;
    }
}

// Lets the particles run out: none is renewed.
void fn_1_278C(PlayerParticleEffect *effect)
{
    effect->finishing = 1;
}

 void fn_1_2AA0(PlayerPositionHistory *owner, Mtx matrix);

void fn_1_2798(HU3D_MODEL *model, Mtx *matrix)
{
    PlayerPositionHistory *owner =
        static_cast<PlayerPositionHistory *>(model->hookData);
    fn_1_2AA0(owner, *matrix);
}

void fn_1_2CFC(PlayerPositionHistory *);

PlayerPositionHistory::PlayerPositionHistory(Vec *unused, s32 cameraMask)
{
    HU3D_MODEL *model;
    s16 handle;
    this->state = 0;
    this->countdown = 0;
    this->active = 0;
    handle = fn_800260F4(fn_1_2798);
    model = &lbl_802C01C8[handle];
    model->hookData = this;
    modelId = (s16) handle;
    animation = HuSprAnimRead(fn_80008DD0(0x3F0035, 0x10000000, 2));
    HuSprAnimLock(animation);
    fn_80026B8C((s16) handle, 5);
    fn_80026B64((s16) handle, cameraMask);
    fn_80026760((s16) handle, 0.0f, 0.0f, 0.0f);
    fn_80026808((s16) handle, 0.0f, 0.0f, 0.0f);
    this->translation.value.x = 0.0f;
    this->translation.value.y = 0.0f;
    this->translation.value.z = 0.0f;
    this->unknownScalarA = 0.0f;
    this->rotation = 0.0f;
    this->unknownScalarB = 0.0f;
    this->scale.x = this->scale.y = this->scale.z = 1.0f;
    fn_1_2CFC(this);
}

PlayerPositionHistory::~PlayerPositionHistory()
{
    fn_800262D0(modelId);
    HuSprAnimKill(animation);
}

void PlayerPositionHistory::flagEnabled(int bit)
{
    flags &= ~(1 << bit);
}

void PlayerPositionHistory::update()
{
    if (active != 0) {
        countdown -= 2;
        if (countdown <= 0) {
            countdown = 0;
            state = 0;
            active = 0;
        }
    }
}

 void fn_1_3150(PlayerPositionHistory *);

void fn_1_2AA0(PlayerPositionHistory *owner, Mtx inputMatrix)
{
    Mtx transformMatrix;
    Mtx rotationMatrix;
    VectorHolder translation;

    translation = owner->translation;
    PSMTXRotRad(rotationMatrix, 0x79, MTXDegToRad(owner->rotation));
    PSMTXTrans(transformMatrix, translation.value.x, translation.value.y,
               translation.value.z);
    PSMTXConcat(transformMatrix, rotationMatrix, transformMatrix);
    PSMTXConcat(inputMatrix, transformMatrix, transformMatrix);
    GXLoadPosMtxImm(transformMatrix, GX_PNMTX0);
    HuSprTexLoad(owner->animation, 0, 0, (GXTexWrapMode)0, (GXTexWrapMode)0,
                 (GXTexFilter)1);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_ONE, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_TEXC, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    fn_1_3150(owner);
}

void fn_1_2CFC(PlayerPositionHistory *object) {
    s32 index;
    s32 side;
    Vec *vertex;

    vertex = object->vertices;
    index = 0;
    while (index < 16) {
        vertex->x = 0.0f;
        vertex->y = 0.0f;
        vertex->z = 0.0f;
        for (side = 0; side < 2; side++) {
            ((Vec *)object->borders[index][side])->x = 0.0f;
            ((Vec *)object->borders[index][side])->y = 0.0f;
            ((Vec *)object->borders[index][side])->z = 0.0f;
        }
        index++;
        vertex++;
    }
}

void fn_1_2DE8(PlayerPositionHistory *work, Vec *position) {
    s32 index;
    Vec *positions;

    positions = work->vertices;
    index = 0;
    while (index < 16) {
        positions->x = position->x;
        positions->y = position->y;
        positions->z = position->z;
        index += 1;
        positions++;
    }
    work->active = 1;
    work->state = 0;
    work->countdown = 0xFF;
}

void fn_1_2E50(PlayerPositionHistory *work)
{
    s32 currentIndex;
    s32 nextIndex;
    s32 currentSlot;
    s32 nextSlot;
    Vec currentVertex;
    Vec nextVertex;
    Vec edge;
    Vec normal;
    Vec positive;
    Vec negative;
    Vec * negativeVector;
    for (currentIndex = 0, nextIndex = 1; currentIndex < work->state; currentIndex++, nextIndex++)
    {
        currentSlot = currentIndex & 0xF;
        nextSlot = nextIndex & 0xF;
        currentVertex = work->vertices[currentSlot];
        nextVertex = work->vertices[nextSlot];
        PSVECSubtract(& currentVertex, & nextVertex, & edge);
        normal.x = edge.y;
        normal.y = - edge.x;
        normal.z = 0.0f;
        if ((normal.x == 0.0f && 0.0f == normal.y) || currentIndex == 0xF)
        {
            normal.x = 0.0f;
            normal.y = 1.0f;
            normal.z = 0.0f;
        }
        PSVECNormalize(& normal, & normal);
        negative = normal;
        negativeVector = & negative;
        Vec * copiedNegativeVector;
        copiedNegativeVector = negativeVector;
        positive.x = copiedNegativeVector->x;
        positive.y = copiedNegativeVector->y;
        positive.z = copiedNegativeVector->z;
        PSVECScale(& positive, & positive, 30.000002f);
        PSVECScale(& negative, & negative, - 30.000002f);
        PSVECAdd(& positive, & currentVertex, & positive);
        PSVECAdd(& negative, & currentVertex, & negative);
        (*(Vec *)work->borders[currentIndex][0]) = (*(Vec *)work->borders[nextIndex][0]) = positive;
        (*(Vec *)work->borders[currentIndex][1]) = (*(Vec *)work->borders[nextIndex][1]) = negative;
    }
}

void fn_1_3150(PlayerPositionHistory *work)
{
    s32 index;
    s32 nextIndex;
    s32 lastIndex;
    s32 alpha0;
    s32 alpha1;
    f32 alphaStep;
    Vec currentPositive;
    Vec currentNegative;
    Vec nextPositive;
    Vec nextNegative;
    f32 fade0;
    f32 fade1;
    if (work->active != 0 && work->state >= 2 && work->countdown != 0) {
        fn_1_2E50(work);
        lastIndex = work->state - 1;
        alphaStep = (f32)(0x100 / work->state);
        GXBegin(GX_QUADS, GX_VTXFMT0, (u16)((work->state - 2) * 4));

        for (index = 0, nextIndex = 1;
             index < work->state - 2;
             ++index, ++nextIndex) {
            currentPositive = (*(Vec *)work->borders[index][0]);
            currentNegative = (*(Vec *)work->borders[index][1]);
            nextPositive = (*(Vec *)work->borders[nextIndex][0]);
            nextNegative = (*(Vec *)work->borders[nextIndex][1]);

            alpha0 = (s32)((f32)work->countdown -
                               alphaStep * (f32)(lastIndex - index));
            if (alpha0 < 0) {
                alpha0 = 0;
            }
            alpha1 = (s32)((f32)work->countdown -
                               alphaStep * (f32)(lastIndex - nextIndex));
            if (alpha1 < 0) {
                alpha1 = 0;
            }

            fade0 = 0.1f * (f32)((work->state - 3) - index);
            if (fade0 > 1.0f) {
                fade0 = 1.0f;
            }
            fade1 = 0.1f * (f32)((work->state - 2) - nextIndex);
            if (fade1 > 1.0f) {
                fade1 = 1.0f;
            }
            alpha0 = (s32)((f32)alpha0 * fade0);
            alpha1 = (s32)((f32)alpha1 * fade1);

            GXPosition3f32(currentPositive.x, currentPositive.y, currentPositive.z);
            GXColor4u8(0xFF, 0xFF, 0xFF, alpha0);
        GXTexCoord2f32(0.0f, 1.0f);
            GXPosition3f32(nextPositive.x, nextPositive.y, nextPositive.z);
            GXColor4u8(0xFF, 0xFF, 0xFF, alpha1);
        GXTexCoord2f32(0.0f, 0.0f);
            GXPosition3f32(nextNegative.x, nextNegative.y, nextNegative.z);
            GXColor4u8(0xFF, 0xFF, 0xFF, alpha1);
        GXTexCoord2f32(1.0f, 0.0f);
            GXPosition3f32(currentNegative.x, currentNegative.y, currentNegative.z);
            GXColor4u8(0xFF, 0xFF, 0xFF, alpha0);
        GXTexCoord2f32(1.0f, 1.0f);
        }
    }
}

void fn_1_37E4(PlayerPositionHistory *history, const Vec *position)
{
    if (history->state <= 14) {
        history->vertices[history->state] = *position;
        history->vertices[history->state + 1] = *position;
        history->state++;
    }
}

struct ParticleBurst;
void fn_1_4180(ParticleBurst *, Mtx);

void fn_1_386C(HU3D_MODEL *, HU3D_PARTICLE *callback, Mtx data)
{
    ParticleBurst *timingObject = static_cast<ParticleBurst *>(callback->work);
    fn_1_4180(timingObject, data);
}

struct ParticleBurst : RequestObject {
    ParticleBurst(const Vec *, s32, s32);
    virtual ~ParticleBurst();
    virtual void flagEnabled(int);
    virtual void update();
    s16 particleId;
    HU3D_PARTICLE *hookData;
    Vec position;
    Vec basePosition;
    s32 count;
    u8 dispatchCallback;
    s32 camera;

};

 extern void fn_1_3D64(ParticleBurst *, HU3D_PARTICLE_DATA *, s32, s32);

const f32 lbl_1_rodata_150[5][2] = { { 0.0f, 0.0f }, { 45.0f, 45.0f }, { 45.0f, -45.0f }, { -45.0f, -45.0f }, { -45.0f, 45.0f } };

ParticleBurst::ParticleBurst(const Vec *position, s32 count, s32 camera) : RequestObject(), position(*position), basePosition(*position)
{
    s16 particleId;
    WeatherParticles *resources;
    s32 rotationIndex;
    s32 index;
    HU3D_PARTICLE_DATA *record;
    this->count = count;
    this->dispatchCallback = 0;
    this->camera = camera;
    resources = &static_cast<WeatherParticles &>(WeatherParticles::getChecked());
    particleId = fn_8003216C(resources->getAnimation(), 0x40);
    this->hookData = (HU3D_PARTICLE *) lbl_802C01C8[(s16) particleId].hookData;
    this->hookData->work = this;
    fn_800326D0((s16) particleId, (HU3D_PARTICLE_HOOK) fn_1_386C);
    fn_800325B0((s16) particleId, 0xFF, 0xFF, 0xFF);
    fn_80032620((s16) particleId, 0.0f);
    fn_8003269C((s16) particleId, 1);
    fn_800326D0((s16) particleId, (HU3D_PARTICLE_HOOK) fn_1_386C);
    fn_80026B8C((s16) particleId, 5);
    fn_80026B64((s16) particleId, (u16) cameraMasks[camera]);
    fn_80026760((s16) particleId, 0.0f, 0.0f, 0.0f);
    rotationIndex = (frand() % 5 < 0) ? - (frand() % 5) : frand() % 5;
    fn_80026808((s16) particleId, 0.0f, lbl_1_rodata_150[rotationIndex][0], lbl_1_rodata_150[rotationIndex][1]);
    this->particleId = (s16) particleId;
    index = 0;
    record = this->hookData->data;
    for (; index < this->hookData->maxCnt;)
    {
        fn_1_3D64(this, record, index, this->hookData->maxCnt);
        index++;
        record++;
    }

}

extern "C" { void fn_800262D0(s16 modelId); }

ParticleBurst::~ParticleBurst()
{
    hookData->work = 0;
    fn_800262D0(particleId);
}

void ParticleBurst::flagEnabled(int bitIndex) {
    flags &= ~(1 << bitIndex);
}

struct ParticleColorScale {
    f32 values[3];
};

extern "C" { u32 frand(void); }

void ParticleBurst::update()
{

    CameraOwner *records;

    records = &MinigameSingleton<CameraOwner>::getChecked();
    Vec recordPosition = records->fn_1_878C(camera);
    this->position.y = basePosition.y + 0.8f * recordPosition.y;
    this->position.x = basePosition.x + recordPosition.x;
    fn_80026760(particleId, this->position.x, this->position.y, this->position.z);
    if (dispatchCallback != 0) {
        delete this;
    }
}

extern "C" {
const ParticleColorScale lbl_1_rodata_17C[4] = { { { 1.0f, 0.7f, 0.7f } }, { { 0.7f, 1.0f, 0.7f } }, { { 0.7f, 0.7f, 1.0f } }, { { 1.0f, 1.0f, 1.0f } } };

}

void fn_1_3D64(ParticleBurst *owner, HU3D_PARTICLE_DATA *particle,
               s32 phase, s32 duration)
{
    s32 phaseShifted;
    f32 radius;
    f32 angle;
    u32 randomValue;
    s32 colorScaleIndex;

    phaseShifted = phase >> 4;
    particle->time = 0;
    particle->parManId = (s16)((frand() & 7) + 2);
    particle->pos.x = 0.0f;
    particle->pos.y = 0.0f;
    particle->pos.z = 0.0f;

    particle->scale = 2.5f *
        (100.0f * (0.1f +
            0.2f *
                (1.5258789e-05f * (f32)(u16)frand())));

    particle->zRot = MTXDegToRad((360.0f * (1.5258789e-05f * (f32)(u16)frand())));

    radius = 0.5f *
             ((0.5f +
               (f32)((phase * 2) / 16) / 4.0f) /
              2.0f);
    angle = MTXDegToRad((10.0f * (f32)phaseShifted +
         ((360.0f * (f32)phase) /
          ((f32)duration / 4.0f))));
    particle->vel.x = (f32)(radius *
        (7.627394444444445 * sin((f64)angle)));
    particle->vel.y = (f32)(radius *
        (7.627394444444445 * cos((f64)angle)));
    particle->vel.z = 0.0f;

    randomValue = frand();
    particle->color.r = (u8)randomValue;
    particle->color.g = (u8)((randomValue >> 8) & 0xFF);
    particle->color.b = (u8)((randomValue >> 16) & 0xFF);
    particle->color.a = 0;

    colorScaleIndex = owner->count;
    particle->color.r = (u8)((f32)particle->color.r *
                          lbl_1_rodata_17C[colorScaleIndex].values[0]);
    particle->color.g = (u8)((f32)particle->color.g *
                            lbl_1_rodata_17C[colorScaleIndex].values[1]);
    particle->color.b = (u8)((f32)particle->color.b *
                           lbl_1_rodata_17C[colorScaleIndex].values[2]);
}

void fn_1_4180(ParticleBurst *owner, Mtx data) {
    HU3D_MODEL *model;
    HU3D_PARTICLE *list;
    HU3D_PARTICLE_DATA *entry;
    s32 index;

    model = &lbl_802C01C8[owner->particleId];
    list = owner->hookData;
    owner->dispatchCallback = 1;
    index = 0;
    entry = list->data;
    while (index < list->maxCnt) {
        if (entry->time != 0xFF) {
            switch (entry->time) {
            case 0:
                entry->color.a += 0x10;
                if (entry->color.a >= 0xEB) {
                    entry->color.a = 0xFF;
                    entry->time++;
                    /* Fall through into the shrinking state. */
            case 1:
                    entry->color.a -= entry->parManId;
                    if (entry->color.a <= 0x14) {
                        entry->color.a = 0;
                        entry->time = 0xFF;
                    }
                }
                break;
            }
            entry->zRot += MTXDegToRad(1.0f);
            PSVECScale(&entry->vel, &entry->vel, 0.99f);
            PSVECAdd(&entry->pos, &entry->vel, &entry->pos);
            owner->dispatchCallback = 0;
        }
        index++;
        entry++;
    }
}

WeatherParticles::WeatherParticles()
{
    weatherState = 0;
    interval = 0;
    frame = 0;
    elapsed = 0.0f;
    animation = HuSprAnimRead(fn_80008DD0(0x3F001A, 0x10000000, 2));
    HuSprAnimLock(animation);
}

WeatherParticles::~WeatherParticles()
{
    HuSprAnimKill(animation);
}

void WeatherParticles::flagEnabled(int bitIndex) {
    flags &= ~(1 << bitIndex);
}

class CameraOwner;

/* Updates the four board position records and spawns effects for active ones. */
void WeatherParticles::update()
{
    CameraOwner *recordOwner;

    recordOwner = &MinigameSingleton<CameraOwner>::getChecked();

    for (s32 recordIndex = 0; recordIndex < 4; recordIndex++) {
        Vec recordPosition = recordOwner->fn_1_878C(recordIndex);
        Vec effectPosition;
        if (recordPosition.y > 15000.0f) {
            switch (weatherState) {
            case 0: {
                interval = ((frand() % 35 < 0)
                    ? -(frand() % 35) : frand() % 35) + 0x28;
                frame = 0;
                weatherState++;
            }
                /* The first active frame also advances the timer below. */
            case 1:
                frame++;
                if ((u32)frame >= (u32)interval) {

                    effectPosition.x = 100.0f *
                        (3.0f * (3.0517578e-05f *
                            ((f32)(frand() & 0xFFFF) - 32768.0f)));
                    effectPosition.y = 4000.0f +
                        100.0f * (3.0f *
                            (3.0517578e-05f *
                                ((f32)(frand() & 0xFFFF) - 32768.0f)));
                    effectPosition.z = -2000.0f +
                        100.0f * (4.0f *
                            (3.0517578e-05f *
                                ((f32)(frand() & 0xFFFF) - 32768.0f)));

                    new ParticleBurst(&effectPosition, frand() & 3, recordIndex);
                    weatherState = 0;
                }
                break;
            }
        }
    }

    frame++;
    elapsed += 0.016666666666666666;
    if (!(elapsed < 5.0f)) {
        delete this;
    }
}

void fn_1_4BD4(CameraHookOwner *work, Mtx input);

void fn_1_48C8(HU3D_MODEL *model, Mtx *data) {
    CameraHookOwner *hookData = static_cast<CameraHookOwner *>(model->hookData);
    fn_1_4BD4(hookData, *data);
}

// Creates the hooked model for camera `mode` and files the hook under the game objects.
CameraHookOwner::CameraHookOwner(s32 mode) : mode(mode)
{
    s16 created = fn_800260F4(fn_1_48C8);
    fn_80026B8C((s16)created, 0);
    fn_80026B64((s16)created, (u16)cameraMasks[mode]);
    HU3D_MODEL *model = &lbl_802C01C8[created];
    model->hookData = this;
    modelId = (s16)created;
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    position.z = -4000.0f;
    nightMode = GWMgNightFGet();
}

CameraHookOwner::~CameraHookOwner()
{
    fn_800262D0(modelId);
}

// Takes the handled request bit down.
void CameraHookOwner::flagEnabled(int bit)
{
    flags &= ~(1 << bit);
}

// Follows the camera's record: its position, lowered to three quarters and kept below 15000, in front of the scene.
void CameraHookOwner::update()
{
    const CameraOwner &records = MinigameSingleton<CameraOwner>::getChecked();
    position = records.fn_1_878C(mode);
    position.y = 0.75f * position.y;
    position.z = -4000.0f;
    if (position.y > 15000.0f) {
        position.y = 15000.0f;
    }
}

const GXColor lbl_1_rodata_1F4[2] = { { 0x0, 0x0, 0xcc, 0x0 }, { 0x0, 0x0, 0x56, 0x0 } };
const GXColor lbl_1_rodata_1FC[2] = { { 0x65, 0xff, 0xe5, 0x0 }, { 0x0, 0x9f, 0x70, 0x0 } };

void fn_1_4BD4(CameraHookOwner *work, Mtx input)
{
    Mtx translated;
    Vec translation = work->position;
    s32 colorIndex;
    u8 red0, green0, blue0;
    u8 red1, green1, blue1;
    u8 red2, green2, blue2;
    u8 red3, green3, blue3;

    PSMTXTrans(translated, translation.x, translation.y,
               translation.z);
    PSMTXConcat(input, translated, translated);
    GXLoadPosMtxImm(translated, GX_PNMTX0);
    GXSetZMode(GX_ENABLE, GX_LEQUAL, GX_DISABLE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_DISABLE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL,
                  GX_DF_CLAMP, GX_AF_NONE);
    GXSetNumTexGens(1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE,
                      GX_PTIDENTITY);
    GXSetNumTevStages(1);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0A0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_RASC, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_RASA, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_FALSE, GX_TEVPREV);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxDesc(GX_VA_CLR0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_CLR0, GX_CLR_RGBA, GX_RGBA8, 0);
    GXSetCullMode(GX_CULL_NONE);
    colorIndex = work->nightMode;
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(-2500.0f, 10000.0f, 0.0f);
    GXColor4u8(lbl_1_rodata_1F4[colorIndex].r, lbl_1_rodata_1F4[colorIndex].g, lbl_1_rodata_1F4[colorIndex].b, 0xFF);

    GXPosition3f32(2500.0f, 10000.0f, 0.0f);
    GXColor4u8(lbl_1_rodata_1F4[colorIndex].r, lbl_1_rodata_1F4[colorIndex].g, lbl_1_rodata_1F4[colorIndex].b, 0xFF);

    GXPosition3f32(2500.0f, -2500.0f, 0.0f);
    GXColor4u8(lbl_1_rodata_1FC[colorIndex].r, lbl_1_rodata_1FC[colorIndex].g, lbl_1_rodata_1FC[colorIndex].b, 0xFF);

    GXPosition3f32(-2500.0f, -2500.0f, 0.0f);
    GXColor4u8(lbl_1_rodata_1FC[colorIndex].r, lbl_1_rodata_1FC[colorIndex].g, lbl_1_rodata_1FC[colorIndex].b, 0xFF);
}
