#include "REL/m672DLL/random.h"
#include "REL/m672DLL/particle_game_object.h"

void operator delete(void *);
void fn_1_FDB8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

ParticleGameObject::ParticleGameObject(ANIMDATA *animation, Vec *position)
    : GameObject(3), model(fn_1_FDB8, animation, 64)
{
    short particleId;
    model.setPosition(position);
    model.setLayer(2);
    model.SetScale(0.0f);
    model.SetTransparency(0.0f);
    particleId = (short)model.modelId;
    fn_8003269C(particleId, 0);
}

ParticleGameObject::~ParticleGameObject() {}

void ParticleGameObject::update()
{
    HU3D_PARTICLE *particle = model.GetParticle();
    if (particle->dataCnt <= 0) delete this;
}

extern "C" {
const GXColor lbl_1_rodata_66C = {250, 250, 230, 128};



}

static inline void fn_1_2794(HU3D_PARTICLE_DATA *entry)
{
    entry->pos.x = 100.0f * RandomFloat(-2.5f, 2.5f);
    entry->pos.y = 100.0f * RandomFloat(0.0f, 0.5f);
    entry->pos.z = 100.0f * RandomFloat(0.0f, 0.5f);
}

void fn_1_FDB8(HU3D_MODEL *unused, HU3D_PARTICLE *system, Mtx)
{
    HU3D_PARTICLE_DATA *limit;
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *activeEntry;
    s32 activeIndex;
    s32 alpha;
    f32 angle;

    limit = system->data + system->maxCnt;
    if (system->count == 0) {
        entry = system->data;
        while (entry != limit) {
            angle = RandomFloat(-3.1415927f, 3.1415927f);
            entry->vel.x = (f32)(10.0 * cos(angle));
            entry->vel.y = 0.33333334f;
            entry->vel.z = (f32)(10.0 * sin(angle));
            fn_1_2794(entry);
            entry->color = lbl_1_rodata_66C;
            entry->time = 0;
            entry->parManId = (s16)((RandomUnsigned() & 3) + 1);
            entry->scale = 150.0f;
            entry++;
        }
    }
    activeIndex = 0;
    activeEntry = system->data;
    while (activeEntry != limit) {
        if (!(activeEntry->scale <= 0.0f)) {
            PSVECAdd(&activeEntry->vel, &activeEntry->pos, &activeEntry->pos);
            activeEntry->vel.x *= 0.9f;
            activeEntry->vel.z *= 0.9f;
            activeEntry->scale += 0.16666666666666666;
            alpha = activeEntry->color.a;
            if (activeEntry->time == 0) {
                alpha += 8;
                if (alpha > 0x80) {
                    alpha = 0x80;
                    activeEntry->time++;
                }
            } else if (activeEntry->time == 1) {
                alpha -= activeEntry->parManId;
                if (alpha < 0) {
                    alpha = 0;
                    activeEntry->scale = 0.0f;
                    activeEntry->time++;
                }
            }
            activeEntry->color.a = (u8)alpha;
            activeIndex++;
        }
        activeEntry++;
    }
    system->dataCnt = activeIndex;
}

extern "C" { extern float lbl_1_data_3D0; }


extern "C" { float lbl_1_data_600 = 980.665f; }


extern "C" { float lbl_1_bss_60 = (1.0 / 60.0) * ((1.0 / 60.0) * lbl_1_data_600); }
