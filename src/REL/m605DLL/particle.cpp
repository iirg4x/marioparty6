#include <new>
#include "REL/m605DLL/particle_objects.h"
#include "REL/framework/random.h"
#include "dolphin/mtx.h"

#include "dolphin/types.h"

#include "REL/framework/game_object.h"

#include <cmath>
#include "REL/framework/particle.h"

#include "game/hu3d.h"
#include <math.h>

void fn_1_3160(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

// Creates the particle model and places it on the effect layer.
ParticleGameObject::ParticleGameObject(ANIMDATA *animation, Vec *position)
    : GameObject(0x10), model(fn_1_3160, animation, 32)
{
    model.setPosition(position);
    model.setLayer(6);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
}

// Releases the particle object and its model.
ParticleGameObject::~ParticleGameObject() {}

// Deletes the object when its particle animation has ended.
void ParticleGameObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt < 0) delete this;
}

// Initializes a radial burst, then moves and fades its particles.
void fn_1_3160(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = particles->data + particles->maxCnt;
    HU3D_PARTICLE_DATA *first_entry;
    HU3D_PARTICLE_DATA *second_entry;
    int intensity;
    int active;

    if (particles->count == 0) {
        for (first_entry = particles->data;
             first_entry != end; ++first_entry) {
            double angle;
            float cosine;
            float sine;

            angle = RandomFloat(-(float) M_PI, (float) M_PI);
            cosine = (float)cos(angle);
            first_entry->vel.x = cosine;
            first_entry->vel.y = 0.0f;
            sine = (float)sin(angle);
            first_entry->vel.z = sine;
            first_entry->pos.x = 100.0f * first_entry->vel.x;
            first_entry->pos.y = 50.0f;
            first_entry->pos.z = 100.0f * first_entry->vel.z;
            first_entry->vel.x *= 10.0f;
            first_entry->vel.z *= 10.0f;
            first_entry->color.a = 0x80;
            first_entry->scale = 150.0f;
        }
    }

    active = 0;
    for (second_entry = particles->data;
         second_entry != end; ++second_entry) {
        if (second_entry->scale <= 0.0f)
            continue;
        PSVECAdd(&second_entry->vel, &second_entry->pos, &second_entry->pos);
        second_entry->vel.x *= 0.95;
        second_entry->vel.y += 0.2;
        second_entry->vel.z *= 0.95;
        intensity = second_entry->color.a;
        intensity -= 4;
        if (intensity < 0) {
            intensity = 0;
            second_entry->scale = 0.0f;
            particles->dataCnt = -1;
        }
        second_entry->color.a = (unsigned char)intensity;
        ++active;
    }
    particles->emitCnt = (short)active;
}

extern void fn_1_36E8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

extern void fn_1_3988(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

// Creates the two particle models and sets their drawing layers.
ParticlePairGameObject::ParticlePairGameObject(ANIMDATA *firstValue, ANIMDATA *secondValue)
    : GameObject(0x10), model(fn_1_36E8, firstValue, 32),
      secondModel(fn_1_3988, secondValue, 128)
{
    s16 risingModelId;
    s16 trailingModelId;
    risingModelId = (s16)model.modelId;
    fn_80026B8C(risingModelId, 6);
    trailingModelId = (s16)this->secondModel.modelId;
    fn_80026B8C(trailingModelId, 6);
}

// Releases the paired particle models.
ParticlePairGameObject::~ParticlePairGameObject() {}

// Positions both particle models ahead of the supplied point.
void fn_1_34D0(GameObject *object, const Vec *input)
{
    Vec position = *input;
    position.z += 350.0f;
    short risingModelId;
    risingModelId = (short)static_cast<const ParticlePairGameObject *>(object)->model.modelId;
    fn_80026790(risingModelId, &position);
    short trailingModelId;
    trailingModelId =
        (short)static_cast<const ParticlePairGameObject *>(object)->secondModel.modelId;
    fn_80026790(trailingModelId, &position);
}

// Deletes the paired object when its first particle animation has ended.
void ParticlePairGameObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt < 0) delete this;
}

// Initializes a rising particle with random position, velocity and rotation rate.
void fn_1_35E4(HU3D_PARTICLE_DATA *object)
{
    object->vel.x = RandomFloat(-(80.0f / 6.0f), (80.0f / 6.0f));
    object->vel.y = RandomFloat((80.0f / 6.0f), (100.0f / 6.0f));
    object->vel.z = RandomFloat((20.0f / 6.0f), 5.0f);
    object->pos.x = RandomFloat(-300.0f, 300.0f);
    object->pos.y = 0.0f;
    object->pos.z = 0.0f;
    object->parManId = (s16)RandomFloat(-4.0f, 4.0f);
    object->scale = 32.0f;
}

// Restarts expired particles and integrates their falling motion.
void fn_1_36E8(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *endForUpdate = particles->data + particles->maxCnt;
    void (*initialize)(HU3D_PARTICLE_DATA *) = fn_1_35E4;
    s32 retries = 2;
    HU3D_PARTICLE_DATA *candidate = particles->data;
    HU3D_PARTICLE_DATA *endForSelection = candidate + particles->maxCnt;

    while (candidate != endForSelection) {
        if (candidate->scale > 0.0f ||
            (initialize(candidate), retries--, retries >= 0)) {
            candidate++;
        } else {
            break;
        }
    }

    for (HU3D_PARTICLE_DATA *object = particles->data; object != endForUpdate; object++) {
        PSVECAdd(&object->vel, &object->pos, &object->pos);
        if (object->pos.y < -200.0f) {
            object->scale = 0.0f;
        } else {
            object->vel.x *= 0.998f;
            object->vel.z *= 0.998f;
            object->vel.y += (-10.0 / 6.0);
            object->zRot += 0.2f * (f32)object->parManId;
        }
    }
    DCStoreRangeNoSync(particles->data, (u32)particles->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Initializes a trailing particle with random velocity and rotation rate.
void fn_1_3878(HU3D_PARTICLE_DATA *object)
{
    object->vel.x = RandomFloat(-(50.0f / 6.0f), (50.0f / 6.0f));
    object->vel.y = RandomFloat(5.0f, (40.0f / 6.0f));
    object->vel.z = -RandomFloat((100.0f / 6.0f), (200.0f / 6.0f));
    object->pos.x = RandomFloat(-300.0f, 300.0f);
    object->pos.y = 0.0f;
    object->pos.z = 0.0f;
    object->color.a = 0xC0;
    object->parManId = (s16)RandomFloat(-4.0f, 4.0f);
    object->scale = 180.0f;
}

// Restarts trailing particles, then fades, expands and moves them.
void fn_1_3988(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *firstEnd;
    HU3D_PARTICLE_DATA *firstObject;
    void (*initializeObject)(HU3D_PARTICLE_DATA *);
    s32 remaining;
    HU3D_PARTICLE_DATA *object;
    HU3D_PARTICLE_DATA *end;

    firstEnd = particles->data + particles->maxCnt;
    initializeObject = fn_1_3878;
    remaining = 2;
    firstObject = particles->data;
    end = firstObject + particles->maxCnt;
    while (firstObject != end) {
        if (firstObject->scale > 0.0f) {
        } else {
            initializeObject(firstObject);
            remaining -= 1;
            if (remaining < 0) {
                break;
            }
        }
        ++firstObject;
    }

    object = particles->data;
    while (object != firstEnd) {
        s32 timer;
        timer = object->color.a;
        timer -= 4;
        if (timer < 0) {
            object->scale = 0.0f;
        } else {
            object->color.a = (u8)timer;
            object->scale += 2.0f;
            PSVECAdd(&object->vel, &object->pos, &object->pos);
            PSVECScale(&object->vel, &object->vel, 0.98f);
            object->zRot = object->zRot + (0.2f * (f32)object->parManId);
        }
        ++object;
    }
}
