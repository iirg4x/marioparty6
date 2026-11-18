#include "REL/game_gravity.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/random.h"
#include "math.h"
#include "dolphin/os.h"
#include "REL/m610DLL/torch_effect.h"
#include "game/hu3d.h"

/* Reset a torch particle with a random position, lifetime and rotation. */
s32 fn_1_F42C(HU3D_PARTICLE_DATA *particle)
{
    particle->vel.x = 0.0f;
    particle->vel.y = 0.0f;
    particle->vel.z = 0.0f;
    particle->scale = 100.0f;
    particle->pos.x = RandomFloat(-40.0f, 40.0f);
    particle->pos.y = 0.25f * particle->scale;
    particle->pos.z = RandomFloat(-40.0f, 40.0f);
    particle->time = 0;
    particle->parManId = (s16) (0.5f * (RandomFloat(15.0f) + 7.0f));
    particle->zRot = RandomFloat(MTXDegToRad(360.0f));
    particle->color.r = 0xE0;
    particle->color.g = 0x60;
    particle->color.b = 0x30;
    particle->color.a = 0x00;
    return 0;
}

void fn_1_F800(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

/* Create and initialize the torch particle model. */
TorchEffect::TorchEffect(ANIMDATA *animation) : GameObject(0x10), Particle(fn_1_F800, animation, 32)
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
        fn_1_F42C(record);
        record->color.a = 0xFF;
        ++record;
    }
}

/* Release the torch particle model. */
TorchEffect::~TorchEffect()
{
}

/* Leave torch particle updates to the model hook. */
void TorchEffect::update()
{
}

/* Forward the torch model hook to the particle update routine. */
void fn_1_F800(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx mtx)
{
    /* The stored Particle part is passed to the hook without adjusting the pointer. */
    TorchEffect *effect = static_cast<TorchEffect *>(particle->work);
    effect->fn_1_F850(model, particle, mtx);
}

/* Recycle, fade and integrate the torch particles. */
void TorchEffect::fn_1_F850(HU3D_MODEL *, HU3D_PARTICLE *particles, Mtx)
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
        fn_1_F42C(entry);
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
            fn_1_F42C(updateEntry);
            updateEntry->time++;
            /* fallthrough */
        case 1:
            updateEntry->color.a += updateEntry->parManId;
            if (updateEntry->color.a < 0xE6U)
                break;
            updateEntry->color.a = 0xFFU;
            updateEntry->time++;
            /* fallthrough */
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
            (float) ((double) updateEntry->scale - ((double) (100.0f * 0.3f) / 60));
        if (updateEntry->scale < 0.0f)
        {
            updateEntry->scale = 0.0f;
        }
        previousVelocity = updateEntry->vel.y;
        updateEntry->vel.y = updateEntry->vel.y + (0.2f * gravityPerFrame);
        if (updateEntry->vel.y > (4.0f * gravityPerFrame))
        {
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
