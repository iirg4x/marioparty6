#include "REL/game_gravity.h"
#include "dolphin/types.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/m610DLL/module_types.h"

#include "REL/framework/random.h"

#include "REL/m610DLL/particle_emitter.h"

#include "REL/m610DLL/particle_game_object.hpp"

#include "REL/framework/particle.h"

#include "humath.h"
#include <math.h>

/* SDK RGBA color, copied into each particle record. */
static const GXColor defaultColor = { 0xEF, 0xFF, 0xFF, 0x00 };

/* Multiplier for the peak alpha calculation. */
static const f32 maximumScale = 3.0f;

/* Bias added to the alpha variation. */
static const f32 scaleBias = 0.5f;

/* Initial zero used for particle coordinates and velocity. */
static const f32 zero = 0.0f;

/* Base vertical velocity multiplied by the random velocity factor. */
static const f32 initialVerticalVelocity = 4.0f;

/* Base width multiplied by the random particle width. */
static const f32 minimumWidth = 100.0f;

/* Random width multiplier range. */
static const f32 widthRange = 0.2f;

/* Convert the sampled rotation from degrees to radians. */
static const f32 radiansPerDegree = MTXDegToRad(1.0f);

/* Range of the random initial particle rotation. */
static const f32 fullCircle = 360.0f;

void fn_1_12DF8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

/* Initialize a particle with a random lifetime, scale and velocity. */
inline void ParticleEmitterState::initializeParticleRecord(HU3D_PARTICLE_DATA *request)
{
    f32 scale;
    double velocityZ;
    double velocityX;
    f32 xScale;
    f32 zScale;

    request->time = 0;
    request->parManId = (s16) ((((s32) RandomUnsigned()) & 7) + 3);

    scale = maximumScale * (scaleBias + this->variation);
    if (scale > maximumScale)
        scale = maximumScale;

    xScale = this->xScale * scale;
    zScale = this->zScale * scale;
    velocityZ = zScale;
    velocityX = xScale;
    request->accel.x = velocityX;
    request->accel.y = zero;
    request->accel.z = velocityZ;
    request->vel.y = initialVerticalVelocity * gravityPerFrame;
    request->pos.x = this->x;
    request->pos.y = this->y;
    request->pos.z = this->depth;
    request->scale = minimumWidth * (widthRange + RandomFloat(widthRange));
    request->zRot = radiansPerDegree * RandomFloat(fullCircle);
    request->color = defaultColor;
}

/* Initialize the motion and color fields of a particle effect request. */
s32 ParticleEmitterState::fn_1_12850(HU3D_PARTICLE_DATA *request)
{
    f32 scale;
    double xScale;
    double zScale;

    request->time = 0;
    request->parManId = (s16) ((((s32) RandomUnsigned()) & 7) + 3);

    scale = maximumScale * (scaleBias + this->variation);
    if (scale > maximumScale)
        scale = maximumScale;

    xScale = (f32) (this->xScale * scale);
    zScale = (f32) (this->zScale * scale);
    request->accel.x = (f32) xScale;
    request->accel.y = zero;
    request->accel.z = (f32) zScale;
    request->vel.y = initialVerticalVelocity * gravityPerFrame;
    request->pos.x = this->x;
    request->pos.y = this->y;
    request->pos.z = this->depth;
    request->scale = minimumWidth * (widthRange + RandomFloat(widthRange));
    request->zRot = radiansPerDegree * RandomFloat(fullCircle);
    request->color = defaultColor;
    return 0;
}

/* Create the particle model and initialize all its records. */
ParticleEmitterState::ParticleEmitterState(ANIMDATA *animation) : GameObject(0x10)
{
    HU3D_PARTICLE_DATA *record;
    HU3D_PARTICLE_DATA *end;
    s32 index;
    s16 particleId;

    particleId = fn_8003216C(animation, 16);
    this->hookData = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[particleId].hookData);
    this->hookData->work = this;
    fn_800326D0((s16) particleId, fn_1_12DF8);
    fn_800325B0((s16) particleId, 0xC0, 0x50, 0x20);
    fn_8003269C((s16) particleId, 0);
    fn_80026B8C((s16) particleId, 5);
    fn_80026B64((s16) particleId, 1);
    fn_80026760((s16) particleId, 0.0f, 0.0f, 0.0f);
    this->particleId = (s16) particleId;
    this->x = 0.0f;
    this->y = 0.0f;
    this->depth = 2000.0f;
    this->xScale = 0.0f;
    this->unk2C = 0.0f;
    this->zScale = 0.0f;
    this->variation = 0.0f;

    record = this->hookData->data;
    end = this->hookData->data + this->hookData->maxCnt;
    for (index = 0; record != end; ++record, ++index)
    {
        initializeParticleRecord(record);
    }
}

/* Detach the particle hook and destroy its model. */
ParticleEmitterState::~ParticleEmitterState()
{
    hookData->work = 0;
    fn_800262D0(particleId);
}

/* Leave particle motion to the model hook. */
void ParticleEmitterState::update()
{
}

/* Forward the particle model hook to its owning object. */
void fn_1_12DF8(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    ParticleEmitterState *emitter = static_cast<ParticleEmitterState *>(particle->work);
    emitter->fn_1_12E48(model, particle, mtx);
}

/* Fade, recycle and integrate the attached particles. */
void ParticleEmitterState::fn_1_12E48(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    HU3D_PARTICLE_DATA *record;
    HU3D_PARTICLE_DATA *end;
    u8 colorLimit;

    record = particle->data;
    end = particle->data + particle->maxCnt;
    colorLimit = (u8) (200.0f * this->variation);
    if (colorLimit < 0x64U)
        colorLimit = 0x64;
    for (; record != end; ++record)
    {
        switch (record->time)
        {
        case 0:
            record->color.a += 8;
            if (record->color.a < colorLimit)
                break;
            record->color.a = colorLimit;
            ++record->time;
            /* Fall through to the fade step after reaching the target color. */
        case 1:
            record->color.a -= record->parManId;
            if (record->color.a > 0xCU)
                break;
            record->color.a = 0x00;
            ++record->time;
            /* Fall through to particle initialization after expiration. */
        case 2:
            if (this->variation != 0.0f)
            {
                initializeParticleRecord(record);
            }
            continue;
        default:
            break;
        }
        record->zRot += radiansPerDegree;
        record->scale += 3.0f;
        if (record->scale > 100.0f)
            record->scale = 100.0f;
        {
            f32 previousVelocity = record->vel.y;
            f32 minimumHeight;

            record->vel.y += -gravityPerFrame;
            previousVelocity = 0.5f * (previousVelocity + record->vel.y);
            record->pos.y += previousVelocity;
            minimumHeight = (0.2f * record->scale) + this->y;
            if (record->pos.y < minimumHeight)
                record->pos.y = minimumHeight;
        }
        record->pos.x += record->accel.x;
        record->pos.z += record->accel.z;
    }
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

/* Set the particle emitter position. */
void ParticleEmitterState::fn_1_13248(Vec *position)
{
    this->x = position->x;
    this->y = position->y;
    this->depth = position->z;
}

/* Set the particle emission direction and scale. */
void ParticleEmitterState::fn_1_13264(Vec *direction)
{
    this->xScale = direction->x;
    this->unk2C = direction->y;
    this->zScale = direction->z;
}

/* Scale peak alpha and recycle expired particles when the value is nonzero. */
void ParticleEmitterState::fn_1_13280(f32 value)
{
    this->variation = value;
}

/* Position a particle model on the effect layer and hide it before initialization. */
static inline void initializeParticleModel(Particle &model, Vec *position)
{
    model.setPosition(position);
    model.setLayer(4);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
}

/* Create a transparent expanding particle cloud at the supplied position. */
ParticleCloudObject::ParticleCloudObject(ANIMDATA *animation, Vec *position)
    : GameObject(0x10), model(fn_1_13414, animation, 32)
{
    initializeParticleModel(model, position);
}

/* Release the cloud particle model. */
ParticleCloudObject::~ParticleCloudObject()
{
}

/* An invalid particle count ends this object's updates. */
void ParticleCloudObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt < 0)
        delete this;
}

/* Initialize and advance the expanding cloud particles until they fade out. */
void fn_1_13414(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = particles->data + particles->maxCnt;
    HU3D_PARTICLE_DATA *firstEntry;
    HU3D_PARTICLE_DATA *secondEntry;
    int intensity;
    int active;

    if (particles->count == 0)
    {
        for (firstEntry = particles->data; firstEntry != end; firstEntry++)
        {
            float angle = RandomFloat(-(float) M_PI, (float) M_PI);
            firstEntry->vel.x = (float) cos(angle);
            firstEntry->vel.y = 0.0f;
            firstEntry->vel.z = (float) sin(angle);
            firstEntry->pos.x = 100.0f * firstEntry->vel.x;
            firstEntry->pos.y = 50.0f;
            firstEntry->pos.z = 100.0f * firstEntry->vel.z;
            firstEntry->vel.x *= 20.0f;
            firstEntry->vel.z *= 20.0f;
            firstEntry->color.a = 0x80;
            firstEntry->scale = 300.0f;
        }
    }

    active = 0;
    for (secondEntry = particles->data; secondEntry != end; secondEntry++)
    {
        if (secondEntry->scale <= 0.0f)
        {
            continue;
        }
        PSVECAdd(&secondEntry->vel, &secondEntry->pos, &secondEntry->pos);
        secondEntry->vel.x *= 0.95;
        secondEntry->vel.y += 0.2;
        secondEntry->vel.z *= 0.95;
        intensity = secondEntry->color.a;
        intensity -= 0x04;
        if (intensity < 0x00)
        {
            intensity = 0x00;
            secondEntry->scale = 0.0f;
            particles->dataCnt = -1;
        }
        secondEntry->color.a = (u8) intensity;
        ++active;
    }
    particles->emitCnt = active;
}

/* Create and initialize the colored impact particles. */
ParticleGameObject::ParticleGameObject(ANIMDATA *animation, Vec *position)
    : GameObject(0x10), model(fn_1_13A9C, animation, 32)
{
    initializeParticleModel(model, position);
    short particleId = (short) model.modelId;
    fn_800325B0(particleId, 0xF0, 0xE6, 0x82);
    fn_1_137EC();
}

/* Release the impact particle model. */
ParticleGameObject::~ParticleGameObject()
{
}

/* This update variant also ends the object when its particle count is zero. */
void ParticleGameObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt <= 0)
        delete this;
}

/* Seed each particle with a point on the circle and its starting motion values. */
void ParticleGameObject::fn_1_137EC()
{
    HU3D_PARTICLE *state = model.GetParticle();
    HU3D_PARTICLE_DATA *element = state->data;
    s32 index = 0;

    while (index < state->maxCnt)
    {
        element->vel.x =
            (f32) (0.6f *
                   sin((M_PI * (double) ((360.0f * (f32) index) / (f32) state->maxCnt)) / 180.0));
        element->vel.y = element->accel.y = 1.5f * (gravityPerFrame * (30.0f + RandomFloat(10.0f)));
        element->vel.z =
            (f32) (0.6f *
                   cos((M_PI * (double) ((360.0f * (f32) index) / (f32) state->maxCnt)) / 180.0));
        element->scale = 80.0f;
        element->pos.x = RandomFloat(-100.0f, 100.0f);
        element->pos.y = -100.0f;
        element->pos.z = RandomFloat(-100.0f, 100.0f);
        element->color.a = 0x00;
        element->time = 0;
        element->parManId = (s16) (((s32) RandomUnsigned() & 0xF) + 0xF);
        element->zRot = (f32) ((M_PI * RandomFloat(360.0f)) / 180.0);
        state->dataCnt += 1;
        index += 1;
        ++element;
    }
}

/* Update the animation state and position of each active scene particle. */
void fn_1_13A9C(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    f32 previousVelocity;
    f32 midpoint;
    s32 elementIndex;
    HU3D_PARTICLE_DATA *element;

    element = particles->data;
    particles->dataCnt = 0;
    elementIndex = 0;
    while (elementIndex < particles->maxCnt)
    {
        if (element->time != 2)
        {
            if (element->time == 0)
            {
                element->color.a = (u8) (element->color.a + element->parManId);
                if (element->color.a >= 0xA0U)
                {
                    element->color.a = 0xA0U;
                    ++element->time;
                }
            }
            else if (element->color.a > 6)
            {
                element->color.a -= 4;
            }
            else
            {
                element->color.a = 0x00;
                ++element->time;
            }
            element->scale += (100.0 / 60.0);
            previousVelocity = element->vel.y;
            element->vel.y = (f32) (element->vel.y + -(1.5f * gravityPerFrame));
            midpoint = 0.5f * (previousVelocity + element->vel.y);
            element->pos.x =
                (f32) ((f64) element->pos.x +
                       (0.016666666666666666 * (f64) (100.0f * (2.0f * element->vel.x))));
            element->pos.y = (f32) (element->pos.y + midpoint);
            element->pos.z =
                (f32) ((f64) element->pos.z +
                       (0.016666666666666666 * (f64) (100.0f * (2.0f * element->vel.z))));
            ++particles->dataCnt;
        }
        elementIndex += 1;
        ++element;
    }
    DCStoreRangeNoSync(particles->data, particles->maxCnt * sizeof(*element));
}
