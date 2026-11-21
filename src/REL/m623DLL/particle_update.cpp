// Fire particle creation and simulation.

#include "REL/game_gravity.h"
#include "REL/framework/random.h"
#include "REL/m623DLL/fire_particle.h"

// Resets a particle with a random position, lifetime and rotation.
s32 fn_1_5ADC(HU3D_PARTICLE_DATA *particle)
{
    particle->vel.x = 0.0f;
    particle->vel.y = 0.0f;
    particle->vel.z = 0.0f;
    particle->scale = 50.0f;
    particle->pos.x = RandomFloat(-20.0f, 20.0f);
    particle->pos.y = 0.5f * particle->scale;
    particle->pos.z = RandomFloat(-20.0f, 20.0f);
    particle->time = 0;
    particle->parManId = 0.5f * (RandomFloat(15.0f) + 7.0f);
    particle->zRot = RandomFloat(MTXDegToRad(360.0f));
    particle->color.r = 0xE0;
    particle->color.g = 0x60;
    particle->color.b = 0x30;
    particle->color.a = 0x00;
    return 0;
}

void fn_1_5EB0(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix);

// Creates and initializes the particle model.
FireParticle::FireParticle(ANIMDATA *animation)
    : GameObject(0x10), Particle(fn_1_5EB0, animation, 32)
{
    HU3D_PARTICLE_DATA *end;
    fn_800325B0(ParticleModelId(this), 0xE0, 0x60, 0x30);
    fn_8003269C(ParticleModelId(this), 1U);
    setLayer(7);
    HU3D_PARTICLE_DATA *record = Begin();
    end = End();
    while (record != end) {
        record->time = 0;
        fn_1_5ADC(record);
        record->color.a = 0xFF;
        ++record;
    }
}

// Destroys the fire particle effect.
FireParticle::~FireParticle()
{
}

// Leaves particle updates to the model hook.
void FireParticle::update()
{
}

// Forwards the particle model hook to the particle update routine.
void fn_1_5EB0(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    FireParticle *effect = static_cast<FireParticle *>(particles->work);
    effect->fn_1_5F00(model, particles, matrix);
}

// Recycles, fades and integrates the particles.
void FireParticle::fn_1_5F00(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    float previousVelocity;
    int resetCount;
    HU3D_PARTICLE_DATA *updateEnd;
    HU3D_PARTICLE_DATA *initializeEnd;
    HU3D_PARTICLE_DATA *initializeEntry;
    HU3D_PARTICLE_DATA *updateEntry;
    HU3D_PARTICLE_DATA *entry;

    updateEntry = particles->data;
    updateEnd = particles->data + particles->maxCnt;
    resetCount = 10;
    initializeEntry = particles->data;
    initializeEnd = initializeEntry + particles->maxCnt;
    for (; initializeEntry != initializeEnd; ++initializeEntry) {
        if (initializeEntry->scale > 0.0f) {
            continue;
        }
        entry = initializeEntry;
        fn_1_5ADC(entry);
        resetCount -= 1;
        if (resetCount < 0) {
            break;
        }
    }
    for (; updateEntry != updateEnd; ++updateEntry) {
        switch (updateEntry->time) {
        case 0:
            fn_1_5ADC(updateEntry);
            updateEntry->time++;
            // fallthrough
        case 1:
            updateEntry->color.a += updateEntry->parManId;
            if (updateEntry->color.a < 0xE6U)
                break;
            updateEntry->color.a = 0xFFU;
            updateEntry->time++;
            // fallthrough
        case 2:
            updateEntry->color.a -= updateEntry->parManId;
            if (updateEntry->color.a <= 0xFU) {
                updateEntry->color.a = 0x00;
                updateEntry->time = 0;
            }
            break;
        }
        updateEntry->scale =
            (float) ((double) updateEntry->scale - ((double) (100.0f * 0.3f) / 60));
        if (updateEntry->scale < 0.0f) {
            updateEntry->scale = 0.0f;
        }
        previousVelocity = updateEntry->vel.y;
        updateEntry->vel.y = updateEntry->vel.y + (0.2f * gravityPerFrame);
        if (updateEntry->vel.y > (4.0f * gravityPerFrame)) {
            updateEntry->vel.y = 4.0f * gravityPerFrame;
        }
        previousVelocity = 0.5f * (previousVelocity + updateEntry->vel.y);
        updateEntry->pos.y += previousVelocity;
        updateEntry->pos.x *= 0.95f;
        updateEntry->pos.z *= 0.95f;
        updateEntry->color.r *= 0.999f;
        updateEntry->color.g *= 0.999f;
        updateEntry->color.b *= 0.999f;
    }
}
