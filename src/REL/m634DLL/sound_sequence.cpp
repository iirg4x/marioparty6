#include "REL/framework/linknode.h"
#include "REL/framework/thread.h"
#include "REL/framework/game_object.h"
#include "REL/m634DLL/random.h"
extern "C" {
#include "dolphin/types.h"
}

extern "C" {
#include "dolphin/os.h"
}

#include "dolphin/mtx.h"

#include "math.h"

extern "C" {
#include "dolphin/gx.h"
}

#include "REL/m634DLL/camera.h"

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/charman.h"
}

#include "REL/framework/model.h"

#include "REL/framework/sound.h"

#include "REL/m634DLL/particle_effect.h"

extern "C" {
#include "game/main.h"
}

extern "C" {
#include "game/object.h"
}

extern "C" {
#include "game/gamework.h"
}

extern "C" {
#include "game/wipe.h"
}

extern "C" {
#include "game/mg/timer.h"
}

extern "C" {
#include "humath.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/gamemes.h"
}

extern "C" {
#include "game/hsfex.h"
}

extern "C" {
#include "game/data.h"
}

extern "C" {
#include "game/memory.h"
}

extern "C" {
#include "game/mg/seqman.h"
}

extern "C" {
#include "game/mg/score.h"
}

extern "C" {
#include "game/pad.h"
}

extern "C" {
#include "game/frand.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/board/object.h"
}

extern "C" {
#include "game/board/tutorial.h"
}

extern "C" {
#include "datadir_enum.h"
}

void operator delete(void *);

struct Data888Storage {
    f32 value;

    u8 unk04[0x18]; /* No code names this field. */
};

extern "C" {
Data888Storage lbl_1_data_888 = { 980.665f, { 0 } };
}

// Creates the particle model and installs its update hook.
ParticleModel634::ParticleModel634(ANIMDATA *animation, Vec *position, s32 particleCount)
    : ParticleObjectBase(0x50)
{
    s16 model;
    finished = 0;
    model = fn_8003216C(animation, (s16)particleCount);
    hookData = (HU3D_PARTICLE *)lbl_802C01C8[model].hookData;
    hookData->work = this;
    fn_8003269C((s16)model, 1);
    fn_80026B8C((s16)model, 7);
    fn_80026B64((s16)model, 1);
    fn_80026760((s16)model, position->x, position->y, position->z);
    fn_80026B64((s16)model, 2);
    modelId = (s16)model;
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    scale = 0.0f;
    initializedCount = 0;
    activeCount = 0;
}

// Clears the particle owner pointer and deletes the model.
ParticleModel634::~ParticleModel634()
{
    hookData->work = 0;
    fn_800262D0(modelId);
}

// Does nothing.
void ParticleModel634::fn_1_11914() { }

// Updates the particle model scale and visibility state.
void ParticleModel634::fn_1_11918()
{
    Callback resetValues;

    if (scale > 0.5f) {
        if ((activeCount & 7) == 0) {
            fn_80026A44(modelId, 1);
        } else {
            fn_800269E8(modelId, 1);
        }
        if (scale > 1.0f) {
            fn_80026A44(modelId, 1);
            resetValues = &ParticleModel634::fn_1_119FC;
            stateCallback = resetValues;
        }
    }
    scale += 0.016666666666666666;
}

// Does nothing.
void ParticleModel634::fn_1_119FC() { }

// Does nothing.
void ParticleModel634::fn_1_11A00() { }

// Selects the particle model state callback.
void ParticleModel634::mode(s32 mode)
{
    switch (mode) {
    case 0: {
        Callback values;

        scale = 0.0f;
        initializedCount = 0;
        values = &ParticleModel634::fn_1_11918;
        stateCallback = values;
        break;
    }
    case 3: {
        Callback values;

        values = &ParticleModel634::fn_1_11A00;
        stateCallback = values;
        break;
    }
    }
}

extern Vec lbl_1_data_8C8;

extern "C" {
const GXColor lbl_1_rodata_888[10] = { 102, 102, 68, 170, 136, 136, 102, 170, 170, 170, 136, 170,
    204, 204, 170, 170, 238, 238, 204, 170, 102, 102, 102, 170, 136, 136, 136, 170, 170, 170, 170,
    170, 204, 204, 204, 170, 238, 238, 238, 170 };
}

// Initializes a particle position, velocity, lifetime, and color.
void ParticleEffect634::updateEntry(HU3D_PARTICLE_DATA *work)
{
    const GXColor *color;

    work->time = 0;
    work->parManId = (((s32)RandomUnsigned()) & 0xF) + 5;
    work->pos.x = RandomFloat(-5.0f, 5.0f);
    work->pos.y = RandomFloat(0.0f, 80.0f);
    work->pos.z = RandomFloat(-5.0f, 5.0f);
    lbl_1_data_8C8.x = (f32)(0.016666666666666666 * RandomFloat(-400.0f, 400.0f));
    lbl_1_data_8C8.y = (f32)(0.016666666666666666 * RandomFloat(0.0f, 100.0f));
    lbl_1_data_8C8.z = (f32)(0.016666666666666666 * RandomFloat(-400.0f, 400.0f));

    if (lbl_1_data_8C8.x >= 0.0f && lbl_1_data_8C8.x <= 1.5f) {
        lbl_1_data_8C8.x += 2.0f;
    }
    if (lbl_1_data_8C8.x < 0.0f && lbl_1_data_8C8.x >= (-1.5f)) {
        lbl_1_data_8C8.x -= 2.0f;
    }
    if (lbl_1_data_8C8.y >= 0.0f && lbl_1_data_8C8.y <= 0.2f) {
        lbl_1_data_8C8.y += 1.5f;
    }
    if (lbl_1_data_8C8.z >= 0.0f && lbl_1_data_8C8.z <= 0.5f) {
        lbl_1_data_8C8.z += 1.0f;
    }
    if (lbl_1_data_8C8.z < 0.0f && lbl_1_data_8C8.z >= (-1.5f)) {
        lbl_1_data_8C8.z -= 2.0f;
    }
    work->vel.x = lbl_1_data_8C8.x;
    work->vel.y = lbl_1_data_8C8.y;
    work->vel.z = lbl_1_data_8C8.z;

    color = lbl_1_rodata_888 + (((s32)RandomUnsigned()) % 10);
    work->color = *color;
    work->scale = 70.0f;
    work->zRot = MTXDegToRad(RandomFloat(360.0f));
}

Vec lbl_1_data_8C8 = { 0.0f, 0.0f, 0.0f };

// Initializes the effect particles and installs the effect callback.
ParticleEffect634::ParticleEffect634(
    ANIMDATA *animation, Vec *position, s32 particleCount, s32 layer)
    : ParticleModel634(animation, position, particleCount)
{
    s16 model;
    Callback cb;
    s32 index;
    HU3D_PARTICLE_DATA *entry, *lastEntry;
    model = (s16)modelId;
    hookData = (HU3D_PARTICLE *)lbl_802C01C8[model].hookData;
    hookData->work = this;
    fn_800326D0((s16)model, fn_1_122E8);
    fn_8003269C((s16)model, 0);
    fn_80026B8C((s16)model, (s16)layer);
    fn_80026760((s16)model, position->x, position->y, position->z);
    fn_80026808((s16)model, 20.0f, 0.0f, 0.0f);
    fn_80026B64((s16)model, 2);
    entry = hookData->data;
    lastEntry = hookData->data + hookData->maxCnt;
    for (index = 0; entry != lastEntry; index++) {
        updateEntry(entry);
        entry++;
    }
    x = 0.0f;
    y = 0.0f;
    z = 0.0f;
    scale = 0.0f;
    initializedCount = 0;
    activeCount = 0;
    cb = &ParticleModel634::fn_1_11914;
    callback = cb;
}

// Releases the object and its member-owned resources.
ParticleEffect634::~ParticleEffect634() { }

// Runs the effect callback and deletes the finished effect.
void ParticleEffect634::update()
{
    if (callback) {
        (this->*callback)();
    }
    if (finished == 1) {
        delete this;
    }
    activeCount++;
}

struct ParticleHookInterface : Node {
    virtual ~ParticleHookInterface();
    virtual void update();
    virtual void mode(s32);
    virtual void updateEntry(HU3D_PARTICLE_DATA *);
    virtual void forward(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

    // Returns the object attached to the particle group.
    static ParticleHookInterface *get_owner(HU3D_PARTICLE *particle)
    {
        return static_cast<ParticleHookInterface *>(particle->work);
    }
};

// Forwards the particle hook to its owning model.
void fn_1_122E8(HU3D_MODEL *value, HU3D_PARTICLE *particle, Mtx flags)
{
    ParticleHookInterface::get_owner(particle)->forward(value, particle, flags);
}

void fn_1_12EE0(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix);

// Advances particle positions, scale, rotation, and opacity.
void ParticleEffect634::forward(HU3D_MODEL *unused, HU3D_PARTICLE *group, Mtx matrix)
{
    HU3D_PARTICLE_DATA *entry = group->data;
    HU3D_PARTICLE_DATA *end = group->data + group->maxCnt;

    while (entry != end) {
        entry->pos.x += entry->vel.x;
        entry->pos.y += entry->vel.y;
        entry->pos.z += entry->vel.z;
        if (entry->color.r < 0xFF) {
            entry->color.r++;
            entry->color.g++;
            entry->color.b++;
        }
        entry->color.a -= 2;
        entry->scale += 0.2f;
        if (entry->color.a < 0xA6) {
            entry->vel.x *= 0.9f;
            entry->vel.y -= 0.03333333333333333;
            entry->vel.z *= 0.9f;
        }
        if (entry->color.a < 2) {
            finished = 1;
        }
        entry++;
    }
}

extern "C" {
const GXColor lbl_1_rodata_8F8[4]
    = { 204, 204, 34, 22, 204, 204, 136, 8, 136, 204, 136, 136, 136, 136, 204, 136 };
}

// Initializes a particle with the requested age and model state.
void fn_1_12474(void *unused, HU3D_PARTICLE_DATA *object, s16 modelId, int value)
{
    u8 byteValue;
    byteValue = value;
    fn_80026A44(modelId, (1 << 0));
    object->time = byteValue;
    object->parManId = 0;
    if (object->time <= 35) {
        object->accel.y = (float)(byteValue * 10);
        object->accel.x = 50.0f;
        object->pos.x = (float)(object->accel.x * cos((M_PI * object->accel.y) / 180.0));
        object->pos.y = 0.0f;
        object->pos.z = (float)(object->accel.x * sin((M_PI * object->accel.y) / 180.0));
        object->color = lbl_1_rodata_8F8[0];
        object->scale = 50.0f;
        object->zRot = MTXDegToRad(RandomFloat(360.0f));
    } else {
        object->vel.x = RandomFloat(34.0f, 68.0f);
        object->vel.y = 0.0f;
        object->accel.x = 50.0f;
        object->accel.y = (float)((byteValue - 36) * 30);
        object->pos.x = (float)(object->accel.x * cos((M_PI * object->accel.y) / 180.0));
        object->pos.y = 0.0f;
        object->pos.z = (float)(object->accel.x * sin((M_PI * object->accel.y) / 180.0));
        object->color = lbl_1_rodata_8F8[1];
        object->scale = 50.0f;
    }
    object->vel.z = 0.0f;
    byteValue++;
    if (byteValue >= 48)
        byteValue = 0;
}

// Initializes the radial particles and their update hook.
ParticleRadialEffect634::ParticleRadialEffect634(
    ANIMDATA *animation, Vec *position, s32 modelIndex, s32 layer, s32 angleStep, float rotation)
    : ParticleModel634(animation, position, modelIndex)
{
    u8 index;

    this->elapsed = 0.0f;
    this->rotation = rotation;
    this->angleStep = angleStep;
    this->modelIndex = modelIndex;
    this->enabled = 0;

    const s16 model = (s16)this->modelId;
    this->hookData = (HU3D_PARTICLE *)lbl_802C01C8[model].hookData;
    this->hookData->work = this;
    fn_800326D0((s16)model, fn_1_12EE0);
    fn_8003269C((s16)model, 1);
    fn_800269E8((s16)model, (1 << 0));
    fn_80026B8C((s16)model, (s16)layer);
    fn_80026760((s16)model, position->x, position->y, position->z);
    float modelRotation = RandomFloat(0.0f, 360.0f);
    fn_80026808((s16)model, 20.0f, modelRotation, 0.0f);
    fn_80026B64((s16)model, (1 << 1));

    HU3D_PARTICLE_DATA *entry = this->hookData->data;
    HU3D_PARTICLE_DATA *end = this->hookData->data + this->hookData->maxCnt;
    index = 0;
    while (entry != end) {
        fn_1_12474(0, entry, (s16)model, index);
        entry++;
        index++;
    }

    this->x = 0.0f;
    this->y = 0.0f;
    this->z = 0.0f;
    this->scale = 0.0f;
    this->initializedCount = 0;
    this->activeCount = 0;
    Callback callback = &ParticleModel634::fn_1_11914;
    this->callback = callback;
}

// Releases the object and its member-owned resources.
ParticleRadialEffect634::~ParticleRadialEffect634() { }

// Updates the radial effect lifetime and enables its particles after the delay.
void ParticleRadialEffect634::update()
{
    if (callback != 0)
        (this->*callback)();
    if (modelIndex == 0) {
        delete this;
    }
    activeCount += 1;
    elapsed += 0.016666666666666666;
    if (rotation > elapsed)
        return;
    enabled = 1;
    fn_80026A44(modelId, 1);
}

// Forwards the radial particle hook to its owning model.
void fn_1_12EE0(HU3D_MODEL *value, HU3D_PARTICLE *particle, Mtx flags)
{
    ParticleHookInterface::get_owner(particle)->forward(value, particle, flags);
}

// Updates each enabled radial particle according to its age.
void ParticleRadialEffect634::forward(HU3D_MODEL *unused, HU3D_PARTICLE *group, Mtx matrix)
{
    HU3D_PARTICLE_DATA *record;
    HU3D_PARTICLE_DATA *end;

    record = group->data;
    end = group->data + group->maxCnt;
    if (enabled == 1) {
        while (record != end) {
            if (record->time <= 35) {
                fn_1_12FD8(record);
            } else {
                fn_1_1315C(record);
            }
            record++;
        }
    }
}

// Fades and expands a radial particle.
static inline void AdvanceParticle(ParticleRadialEffect634 *owner, HU3D_PARTICLE_DATA *particle)
{
    if (particle->color.a >= 1U) {
        particle->color.a--;
    }
    if (particle->accel.x <= 400.0f) {
        particle->accel.x += 10.0f;
    } else if (0.0f == particle->vel.z) {
        particle->color.a = 0;
        particle->vel.z = 1.0f;
        owner->modelIndex--;
    }
}

// Updates the radial particle orbit and expansion.
void ParticleRadialEffect634::fn_1_12FD8(HU3D_PARTICLE_DATA *particle)
{
    if (1.0f != particle->vel.z) {
        particle->parManId != 0 ? (void)0 : AdvanceParticle(this, particle);
        if (particle->accel.y >= 360.0f) {
            particle->accel.y = 0.0f;
        }
        particle->pos.x = particle->accel.x * cos(M_PI * particle->accel.y / 180.0);
        particle->pos.z = particle->accel.x * sin(M_PI * particle->accel.y / 180.0);
    }
}

// Updates a radial particle fade, scale, and rotation.
void ParticleRadialEffect634::fn_1_1315C(HU3D_PARTICLE_DATA *effect)
{
    if (1.0f != effect->vel.z) {
        if (0.0f == effect->vel.y) {
            effect->color.a += 8;
            if ((f32)effect->color.a >= effect->vel.x) {
                effect->vel.y = 1.0f;
            }
        } else if (1.0f == effect->vel.y) {
            effect->color.a -= 2;
            if (effect->color.a <= 2U) {
                effect->vel.z = 1.0f;
                effect->color.a = 0U;
                modelIndex = (s32)(modelIndex - 1);
            }
        }
        if (effect->accel.x <= 400.0f) {
            effect->accel.x += 4.0f;
        }
        if (effect->parManId == 0) {
            effect->scale += 20.0f;
            if (effect->scale >= 100.0f) {
                effect->parManId = 1;
            }
        }
        if (effect->parManId == 1) {
            effect->scale = (f32)(effect->scale - 20.0f);
            if (effect->scale <= 50.0f) {
                effect->parManId = 0;
            }
        }
        effect->pos.x = (f32)((f64)effect->accel.x * cos((M_PI * (f64)effect->accel.y) / 180.0));
        effect->pos.z = (f32)((f64)effect->accel.x * sin((M_PI * (f64)effect->accel.y) / 180.0));
    }
}

// Does nothing.
void ParticleModel634::updateEntry(HU3D_PARTICLE_DATA *) { }

// Does nothing.
void ParticleModel634::update() { }

// Does nothing.
void ParticleModel634::forward(HU3D_MODEL *, HU3D_PARTICLE *, Mtx) { }

extern "C" {
float lbl_1_bss_78 = (1.0 / 60.0) * ((1.0 / 60.0) * lbl_1_data_888.value);
}
