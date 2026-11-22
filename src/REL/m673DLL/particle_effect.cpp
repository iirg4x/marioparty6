// Updates the model-backed particle effects.

extern "C" {
#include "game/hu3d.h"
#include "math.h"
}
#include "REL/game_gravity.h"
#include "REL/framework/random.h"
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "REL/framework/game_object.h"
#include "REL/framework/particle.h"
#include "REL/m673DLL/particle_game_object.h"

// Initial colour assigned to each particle.
static const GXColor color = { 0xFA, 0xFA, 0xE6, 0x80 };

// Creates and positions the particle model, then initializes its particles.
ParticleGameObject::ParticleGameObject(ANIMDATA *animation, Vec *position)
    : GameObject(4), model(fn_1_1170C, animation, 64)
{
    HU3D_PARTICLE *particles;
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *end;
    s16 modelId;

    model.setPosition(position);
    model.setLayer(2);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
    modelId = (s16)model.modelId;
    fn_8003269C(modelId, 0);
    particles = model.GetParticle();
    entry = particles->data;
    end = particles->data + particles->maxCnt;
    while (entry != end) {
        f32 angle = RandomFloat(-M_PI, M_PI);
        entry->vel.x = (f32)(14.0 * cos(angle));
        entry->vel.y = 20.0f / 60.0f;
        entry->vel.z = (f32)(14.0 * sin(angle));
        entry->pos.x = 2.0f * entry->vel.x;
        entry->pos.y = 100.0f * RandomFloat(0.0f, 0.5f);
        entry->pos.z = 2.0f * entry->vel.z;
        entry->color = color;
        entry->time = 0;
        entry->parManId = (s16)((RandomUnsigned() & 3) + 2);
        entry->scale = 150.0f;
        entry++;
    }
    particles->dataCnt = 1;
}

// Releases the particle model.
ParticleGameObject::~ParticleGameObject()
{
}

// Deletes the particle object when the particle callback's live count reaches zero.
void ParticleGameObject::update()
{
    HU3D_PARTICLE *particles = model.GetParticle();
    if (particles->dataCnt <= 0) {
        delete this;
    }
}

// Advances and fades the active particles.
void fn_1_1170C(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *end;
    s32 liveCount;

    end = particles->data + particles->maxCnt;
    liveCount = 0;
    entry = particles->data;
    while (entry != end) {
        s32 opacity;

        if (!(entry->scale <= 0.0f)) {
            PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
            entry->vel.x *= 0.9f;
            entry->vel.z *= 0.9f;
            entry->scale += 10.0 / 60.0;
            opacity = entry->color.a;
            if (entry->time == 0) {
                opacity += 8;
                if (opacity > 0x80) {
                    opacity = 0x80;
                    ++entry->time;
                }
            } else if (entry->time == 1) {
                opacity -= entry->parManId;
                if (opacity < 0) {
                    opacity = 0;
                    entry->scale = 0.0f;
                    ++entry->time;
                }
            }
            entry->color.a = opacity;
            liveCount++;
        }
        entry++;
    }
    particles->dataCnt = liveCount;
}
