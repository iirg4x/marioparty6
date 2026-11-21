#include "REL/game_gravity.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/random.h"
#include "math.h"
#include "dolphin/os.h"
#include "REL/framework/particle.h"
#include "REL/m605DLL/particle_effect_object.h"
#include "game/hu3d.h"

// Resets an orange particle with random position, rotation and fade rate.
s32 fn_1_B42C(HU3D_PARTICLE_DATA *particle)
{
    particle->vel.x = 0.0f;
    particle->vel.y = 0.0f;
    particle->vel.z = 0.0f;
    particle->scale = 50.0f;
    particle->pos.x = RandomFloat(-20.0f, 20.0f);
    particle->pos.y = 0.5f * particle->scale;
    particle->pos.z = RandomFloat(-20.0f, 20.0f);
    particle->time = 0;
    particle->parManId = (s16) (0.5f * (RandomFloat(15.0f) + 7.0f));
    particle->zRot = RandomFloat(MTXDegToRad(360.0f));
    particle->color.r = 0xE0;
    particle->color.g = 0x60;
    particle->color.b = 0x30;
    particle->color.a = 0x00;
    return 0;
}

void fn_1_B800(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

// Creates the orange particle effect and initializes every particle.
OrangeParticles::OrangeParticles(ANIMDATA *animation)
    : GameObject(0x10), Particle(fn_1_B800, animation, 32)
{
    HU3D_PARTICLE_DATA *end;

    fn_800325B0(ParticleModelId(this), 0xE0, 0x60, 0x30);
    fn_8003269C(ParticleModelId(this), 1U);
    this->setLayer(5);

    HU3D_PARTICLE_DATA *record = this->Begin();
    end = this->End();
    while (record != end)
    {
        record->time = 0;
        fn_1_B42C(record);
        record->color.a = 0xFF;
        ++record;
    }
}

// Releases the particle effect and its base objects.
OrangeParticles::~OrangeParticles()
{
}

// Leaves particle simulation to the model callback.
void OrangeParticles::update()
{
}

// Dispatches the particle simulation using the model work pointer.
void fn_1_B800(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    // The stored Particle part is passed to the hook without adjusting the pointer.
    OrangeParticles *effect = static_cast<OrangeParticles *>(particle->work);
    effect->fn_1_B850(model, particle, mtx);
}

// Restarts, fades and lifts the orange particles while reducing their scale.
void OrangeParticles::fn_1_B850(HU3D_MODEL *, HU3D_PARTICLE *particles, Mtx)
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
    for (; initializeEntry != initializeEnd; ++initializeEntry)
    {
        if (initializeEntry->scale > 0.0f)
        {
            continue;
        }
        entry = initializeEntry;
        fn_1_B42C(entry);
        resetCount -= 1;
        if (resetCount < 0)
        {
            break;
        }
    }
    for (; updateEntry != updateEnd; ++updateEntry)
    {
        switch (updateEntry->time)
        {
        case 0:
            fn_1_B42C(updateEntry);
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
            if (updateEntry->color.a <= 0xFU)
            {
                updateEntry->color.a = 0x00;
                updateEntry->time = 0;
            }
            break;
        }
        updateEntry->scale =
            (float) ((double) updateEntry->scale - ((double) (50.0f * 0.2f) / 60));
        if (updateEntry->scale < 0.0f)
        {
            updateEntry->scale = 0.0f;
        }
        previousVelocity = updateEntry->vel.y;
        updateEntry->vel.y = updateEntry->vel.y + (0.1f * gravityPerFrame);
        if (updateEntry->vel.y > (2.0f * gravityPerFrame))
        {
            updateEntry->vel.y = 2.0f * gravityPerFrame;
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
