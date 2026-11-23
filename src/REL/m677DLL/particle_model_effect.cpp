// Implements the particle models and their per-frame updates.

#include "REL/m677DLL/game_constants.h"
#include "REL/m677DLL/particle_model_effect.h"

#include "REL/framework/random.h"
#include "REL/framework/vector_math.h"
#include "REL/framework/world_units.h"
#include "dolphin/os.h"

#include "REL/m677DLL/scale_hundred.h"

// Creates the particle model and connects its particle list to this object.
ParticleModelEffect::ParticleModelEffect(ANIMDATA *animation, const Vec &position, int capacity)
    : GameObject(10), finished(0)
{
    HU3D_MODELID id = fn_8003216C(animation, capacity);
    particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[id].hookData);
    particle->work = this;
    fn_8003269C((s16) id, 1);
    fn_80026B8C((s16) id, 5);
    fn_80026B64((s16) id, 1);
    fn_80026760((s16) id, position.x, position.y, position.z);
    modelId = (s16) id;
    unk2C = 0.0f;
    unk30 = 0;
    counter = 0;
}

// Disconnects the particle list and kills the model.
ParticleModelEffect::~ParticleModelEffect()
{
    particle->work = 0;
    fn_800262D0(modelId);
}

// Supplies the empty default frame callback.
void ParticleModelEffect::frameCallback()
{
}

// Supplies an empty secondary callback.
void ParticleModelEffect::fn_1_434C()
{
}

// Connects the model and the concrete effect's particle callback.
template <class Effect>
static inline void registerBurstModel(Effect *effect, s16 id, const Vec &position, int count)
{
    effect->particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[id].hookData);
    effect->particle->work = effect;
    effect->particleCount = count;
    fn_800326D0((s16) id, Effect::updateModelParticles);
    fn_8003269C((s16) id, 0);
    fn_80026B8C((s16) id, 5);
    fn_80026760((s16) id, position.x, position.y, position.z);
}

// Initializes the effect's settings and each entry in its particle list.
template <class Effect>
static inline void initializeBurstEntries(Effect *effect, float initialScale,
                                          float launchSpeedRange, int fadeStep)
{
    vectorSet(&effect->unk1C, 0.0f, 0.0f, 0.0f);
    effect->unk2C = 0.0f;
    effect->unk30 = 0;
    effect->counter = 0;
    effect->scaleStart = initialScale;
    effect->velocityRange = launchSpeedRange;
    effect->alphaStep = fadeStep;
    HU3D_PARTICLE_DATA *entry = effect->particle->data;
    HU3D_PARTICLE_DATA *end = effect->particle->data + effect->particle->maxCnt;
    for (int index = 0; entry != end; ++entry, ++index) {
        effect->initializeParticle(entry);
    }
    void (ParticleModelEffect::*onFrame)() = &ParticleModelEffect::frameCallback;
    effect->callback = onFrame;
}

// Sets up the model hook and initializes the gray particle entries.
GrayImpactParticles::GrayImpactParticles(ANIMDATA *animation, const Vec &position, int count,
                                         float initialScale, float launchSpeedRange, int fadeStep)
    : ParticleModelEffect(animation, position, count)
{
    s16 id = (s16) modelId;
    registerBurstModel(this, id, position, count);
    fn_80026808((s16) id, 10.0f, 0.0f, 0.0f);
    initializeBurstEntries(this, initialScale, launchSpeedRange, fadeStep);
}

// Releases the particle model through the base destructor.
GrayImpactParticles::~GrayImpactParticles()
{
}

// Chooses the initial position, velocity, color, size, and rotation of a gray particle.
void GrayImpactParticles::initializeParticle(HU3D_PARTICLE_DATA *entry)
{
    static Vec velocity = { 0.0f, 0.0f, 0.0f };
    float negativeXRange;
    float positiveXRange;
    float negativeZRange;
    float positiveZRange;
    float startingScale;
    entry->time = 0;
    entry->parManId = 0;
    entry->pos.x = RandomFloat(-5.0f, 5.0f);
    entry->pos.y = 0.0f;
    entry->pos.z = RandomFloat(-5.0f, 5.0f);
    positiveXRange = velocityRange;
    float positiveXScale = 100.0f * positiveXRange;
    float positiveXLimit = positiveXScale;
    negativeXRange = velocityRange;
    float negativeXScale = 100.0f * negativeXRange;
    float negativeXLimit = negativeXScale;
    velocity.x = (1.0 / 60.0) * RandomFloat(-negativeXLimit, positiveXLimit);
    velocity.y = (1.0 / 60.0) * RandomFloat(0.0f, 100.0f);
    positiveZRange = velocityRange;
    float positiveZScale = 100.0f * positiveZRange;
    float positiveZLimit = positiveZScale;
    negativeZRange = velocityRange;
    float negativeZScale = 100.0f * negativeZRange;
    float negativeZLimit = negativeZScale;
    velocity.z = (1.0 / 60.0) * RandomFloat(-negativeZLimit, positiveZLimit);
    if (velocity.x >= 0.0f && velocity.x <= 1.0) {
        velocity.x += 2.0;
    }
    if (velocity.x < 0.0f && velocity.x >= -1.0) {
        velocity.x -= 2.0;
    }
    if (velocity.y >= 0.0f && velocity.y <= 0.2f) {
        velocity.y += 1.5f;
    }
    if (velocity.z >= 0.0f && velocity.z <= 1.0) {
        velocity.z += 2.0;
    }
    if (velocity.z < 0.0f && velocity.z >= -1.0) {
        velocity.z -= 2.0;
    }
    entry->vel.x = velocity.x;
    entry->vel.y = velocity.y;
    entry->vel.z = velocity.z;
    static const GXColor color = { 0x88, 0x88, 0x88, 0xAA };
    entry->color = color;
    startingScale = scaleStart;
    const float size = 100.0f * startingScale;
    entry->scale = cmToWorld(size);
    entry->zRot = MTXDegToRad(RandomFloat(360.0f));
}

// Runs the frame callback and deletes the effect once all particles have finished.
void GrayImpactParticles::update()
{
    if (callback) {
        (this->*callback)();
    }
    if (counter == particleCount) {
        delete this;
    }
}

// Dispatches the model hook to the owner of the particle list.
void GrayImpactParticles::updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    ParticleModelEffect *owner = static_cast<ParticleModelEffect *>(list->work);
    owner->updateParticles(model, list, matrix);
}

// Moves and fades active particles, then flushes the particle list.
void GrayImpactParticles::updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleEnd;
    HU3D_PARTICLE_DATA *entry;

    entry = list->data;
    particleEnd = list->data + list->maxCnt;
    while (entry != particleEnd) {
        if (entry->time == 0) {
            PSVECAdd(&entry->pos, &entry->vel, &entry->pos);
            entry->color.a = (u8) (entry->color.a - alphaStep);
            if (entry->color.a < 0xA2U) {
                entry->vel.x *= 0.96f;
                entry->vel.y = (float) ((double) entry->vel.y - 0.008333333333333333);
                entry->vel.z *= 0.96f;
            }
            if (entry->color.a <= alphaStep) {
                entry->time = 1;
                ++counter;
            }
        }
        ++entry;
    }
    DCStoreRangeNoSync(list->data, list->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Sets up the model hook and initializes the yellow particle entries.
YellowBurstParticles::YellowBurstParticles(ANIMDATA *animation, const Vec &position, int count,
                                           float initialScale, float launchSpeedRange)
    : ParticleModelEffect(animation, position, count)
{
    s16 id = (s16) modelId;
    particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[id].hookData);
    particle->work = this;
    fn_800326D0((s16) id, updateModelParticles);
    fn_8003269C((s16) id, 1);
    fn_80026B8C((s16) id, 5);
    fn_80026760((s16) id, position.x, position.y, position.z);
    fn_80026808((s16) id, 20.0f, 0.0f, 0.0f);
    vectorSet(&unk1C, 0.0f, 0.0f, 0.0f);
    unk2C = 0.0f;
    unk30 = 0;
    counter = 0;
    scaleStart = initialScale;
    velocityRange = launchSpeedRange;
    HU3D_PARTICLE_DATA *entry = particle->data;
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    for (int index = 0; entry != end; ++entry, ++index) {
        initializeParticle(entry);
    }
    void (ParticleModelEffect::*onFrame)() = &ParticleModelEffect::frameCallback;
    callback = onFrame;
}

// Releases the particle model through the base destructor.
YellowBurstParticles::~YellowBurstParticles()
{
}

// Randomizes the position, velocity, palette entry, and rotation of a yellow particle.
void YellowBurstParticles::initializeParticle(HU3D_PARTICLE_DATA *entry)
{
    entry->time = 0;
    entry->parManId = 5 + (RandomUnsigned() & 0xF);
    double x;
    double y;
    double z;
    z = RandomFloat(-5.0f, 5.0f);
    y = RandomFloat(-5.0f, 5.0f);
    x = RandomFloat(-5.0f, 5.0f);
    vectorSet(&entry->pos, x, y, z);

    Vec velocity;
    vectorSet(&velocity,
              (1.0 / 60.0) * RandomFloat(-scaleHundred(velocityRange), scaleHundred(velocityRange)),
              (1.0 / 60.0) * RandomFloat(-scaleHundred(velocityRange), scaleHundred(velocityRange)),
              (1.0 / 60.0) *
                  RandomFloat(-scaleHundred(velocityRange), scaleHundred(velocityRange)));
    if (velocity.x >= 0.0f && velocity.x <= 1.5f) {
        velocity.x += 2.0 / 60.0;
    }
    if (velocity.x < 0.0f && velocity.x >= -1.5f) {
        velocity.x -= 2.0 / 60.0;
    }
    if (velocity.y >= 0.0f && velocity.y <= 1.5f) {
        velocity.y += 2.0 / 60.0;
    }
    if (velocity.y < 0.0f && velocity.y >= -1.5f) {
        velocity.y -= 2.0 / 60.0;
    }
    if (velocity.z >= 0.0f && velocity.z <= 1.5f) {
        velocity.z += 2.0 / 60.0;
    }
    if (velocity.z < 0.0f && velocity.z >= -1.5f) {
        velocity.z -= 2.0 / 60.0;
    }

    entry->vel = velocity;
    static const GXColor palette[] = { { 0xFF, 0xFF, 0x22, 0xAA },
                                       { 0xFF, 0xFF, 0x44, 0xAA },
                                       { 0xFF, 0xFF, 0x66, 0xAA },
                                       { 0xFF, 0xFF, 0x88, 0xAA } };
    entry->color = palette[(s32) RandomUnsigned() % 4];
    entry->scale = scaleHundred(scaleStart);
    entry->zRot = MTXDegToRad(RandomFloat(360.0f));
}

// Runs the frame callback, handles completion, and advances the counter.
void YellowBurstParticles::update()
{
    if (callback) {
        (this->*callback)();
    }
    if (finished == 1) {
        delete this;
    }
    // The counter increments even on the path that deletes this object.
    ++counter;
}

// Dispatches the model hook to the owner of the particle list.
void YellowBurstParticles::updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    ParticleModelEffect *owner = static_cast<ParticleModelEffect *>(list->work);
    owner->updateParticles(model, list, matrix);
}

// Moves, grows, and fades the particles and flags the effect when the alpha runs out.
void YellowBurstParticles::updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleEnd;
    HU3D_PARTICLE_DATA *entry;

    entry = list->data;
    particleEnd = list->data + list->maxCnt;
    while (entry != particleEnd) {
        PSVECAdd(&entry->pos, &entry->vel, &entry->pos);
        entry->color.a -= 2;
        entry->scale += 0.01f;
        if (entry->color.b < 0xFFU) {
            entry->color.b += 1;
        }
        if (entry->color.a < 0xA2U) {
            PSVECScale(&entry->vel, &entry->vel, 0.95f);
        }
        if (entry->color.a < 2U) {
            finished = 1;
        }
        ++entry;
    }
    DCStoreRangeNoSync(list->data, list->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Sets up the model hook and initializes the tan particle entries.
TanBurstParticles::TanBurstParticles(ANIMDATA *animation, const Vec &position, int count,
                                     float initialScale, float launchSpeedRange, int fadeStep)
    : ParticleModelEffect(animation, position, count)
{
    s16 id = (s16) modelId;
    registerBurstModel(this, id, position, count);
    fn_80026808((s16) id, 20.0f, 20.0f, 0.0f);
    initializeBurstEntries(this, initialScale, launchSpeedRange, fadeStep);
}

// Releases the particle model through the base destructor.
TanBurstParticles::~TanBurstParticles()
{
}

// Chooses the position and velocity of a tan particle, with a wider vertical velocity range.
void TanBurstParticles::initializeParticle(HU3D_PARTICLE_DATA *entry)
{
    entry->time = 0;
    entry->parManId = 0;
    double x;
    double y;
    double z;
    z = RandomFloat(-5.0f, 5.0f);
    y = RandomFloat(-5.0f, 5.0f);
    x = RandomFloat(-5.0f, 5.0f);
    vectorSet(&entry->pos, x, y, z);

    Vec velocity;
    vectorSet(&velocity,
              (1.0 / 60.0) * RandomFloat(-scaleHundred(velocityRange / 2.0f),
                                         scaleHundred(velocityRange / 2.0f)),
              (1.0 / 60.0) *
                  RandomFloat(-scaleHundred(velocityRange), scaleHundred(2.0f * velocityRange)),
              (1.0 / 60.0) * RandomFloat(-scaleHundred(velocityRange / 2.0f),
                                         scaleHundred(velocityRange / 2.0f)));
    if (velocity.x >= 0.0f && velocity.x <= 1.0)
        velocity.x += 2.0;
    if (velocity.x < 0.0f && velocity.x >= -1.0)
        velocity.x -= 2.0;
    if (velocity.y >= 0.0f && velocity.y <= 1.0)
        velocity.y += 2.0;
    if (velocity.y < 0.0f && velocity.y >= -1.0)
        velocity.y -= 2.0;
    if (velocity.z >= 0.0f && velocity.z <= 1.0)
        velocity.z += 2.0;
    if (velocity.z < 0.0f && velocity.z >= -1.0)
        velocity.z -= 2.0;

    entry->vel = velocity;
    static const GXColor palette[] = { { 0x88, 0x88, 0x66, 0xAA },
                                       { 0xAA, 0xAA, 0x88, 0xAA },
                                       { 0xCC, 0xCC, 0xAA, 0xAA } };
    entry->color = palette[(s32) RandomUnsigned() % 3];
    entry->scale = scaleHundred(scaleStart);
    entry->zRot = MTXDegToRad(RandomFloat(360.0f));
}

// Runs the frame callback and deletes the effect once all particles have finished.
void TanBurstParticles::update()
{
    if (callback) {
        (this->*callback)();
    }
    if (counter == particleCount) {
        delete this;
    }
}

// Dispatches the model hook to the owner of the particle list.
void TanBurstParticles::updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    ParticleModelEffect *owner = static_cast<ParticleModelEffect *>(list->work);
    owner->updateParticles(model, list, matrix);
}

// Moves and fades active particles, counts finished entries, and flushes the particle list.
void TanBurstParticles::updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleEnd;
    HU3D_PARTICLE_DATA *entry;

    entry = list->data;
    particleEnd = list->data + list->maxCnt;
    while (entry != particleEnd) {
        if (entry->time == 0) {
            PSVECAdd(&entry->pos, &entry->vel, &entry->pos);
            entry->color.a = (u8) (entry->color.a - alphaStep);
            if (entry->color.a < 0xA2U) {
                PSVECScale(&entry->vel, &entry->vel, 0.96f);
            }
            if (entry->color.a <= alphaStep) {
                entry->time = 1;
                ++counter;
            }
        }
        ++entry;
    }
    DCStoreRangeNoSync(list->data, list->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Scales the size limit and initializes the entries for the rising particles.
RisingYellowParticles::RisingYellowParticles(ANIMDATA *animation, const Vec &position, int count,
                                             float peakScale)
    : ParticleModelEffect(animation, position, count)
{
    s16 id = (s16) modelId;
    particle = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[id].hookData);
    particle->work = this;
    particleCount = count;
    fn_800326D0((s16) id, updateModelParticles);
    fn_8003269C((s16) id, 1);
    fn_80026B8C((s16) id, 5);
    fn_80026760((s16) id, position.x, position.y, position.z);
    fn_80026808((s16) id, 10.0f, 0.0f, 0.0f);
    unk2C = 0.0f;
    unk30 = 0;
    counter = 0;
    const float scaledMaximum = 100.0f * peakScale;
    maximumScale = scaledMaximum;
    HU3D_PARTICLE_DATA *entry = particle->data;
    HU3D_PARTICLE_DATA *end = particle->data + particle->maxCnt;
    for (int index = 0; entry != end; ++entry, ++index) {
        initializeParticle(entry);
    }
    void (ParticleModelEffect::*onFrame)() = &ParticleModelEffect::frameCallback;
    callback = onFrame;
}

// Releases the particle model through the base destructor.
RisingYellowParticles::~RisingYellowParticles()
{
}

// Randomizes a rising particle's position, palette entry, and rotation.
void RisingYellowParticles::initializeParticle(HU3D_PARTICLE_DATA *entry)
{
    entry->time = 0;
    entry->parManId = 0;
    double x;
    double y;
    double z;
    z = RandomFloat(-50.0f, 50.0f);
    y = RandomFloat(-50.0f, 50.0f);
    x = RandomFloat(-50.0f, 50.0f);
    vectorSet(&entry->pos, x, y, z);
    vectorSet(&entry->vel, 0.0, cmToWorld(50.0) / 60.0, 0.0);

    static const GXColor palette[] = { { 0xFF, 0xFF, 0x66, 0xAA },
                                       { 0xFF, 0xFF, 0x88, 0xAA },
                                       { 0xFF, 0xFF, 0xAA, 0xAA },
                                       { 0xFF, 0xFF, 0xCC, 0xAA } };
    entry->color = palette[(s32) RandomUnsigned() % 4];
    entry->scale = 5.0f;
    entry->zRot = MTXDegToRad(RandomFloat(360.0f));
}

// Runs the frame callback and deletes the effect after the last particle has finished.
void RisingYellowParticles::update()
{
    if (callback) {
        (this->*callback)();
    }
    if (counter >= particleCount) {
        delete this;
    }
}

// Dispatches the model hook to the owner of the particle list.
void RisingYellowParticles::updateModelParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    ParticleModelEffect *owner = static_cast<ParticleModelEffect *>(list->work);
    owner->updateParticles(model, list, matrix);
}

// Moves the particles upward, grows and shrinks them, and counts finished entries.
void RisingYellowParticles::updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
    HU3D_PARTICLE_DATA *particleEnd;
    HU3D_PARTICLE_DATA *entry;

    entry = list->data;
    particleEnd = list->data + list->maxCnt;
    while (entry != particleEnd) {
        if (entry->time <= 1) {
            entry->pos.y = entry->pos.y + entry->vel.y;
        }
        if (entry->time == 0) {
            entry->scale += 1.0;
            if (entry->scale >= maximumScale) {
                entry->time = 1;
            }
        }
        if (entry->time == 1) {
            entry->scale -= 1.0;
            if (entry->scale <= 1.0) {
                entry->time = 2;
                entry->color.a = 0;
                ++counter;
            }
        }
        ++entry;
    }
    DCStoreRangeNoSync(list->data, list->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Supplies the empty default particle initializer.
void ParticleModelEffect::initializeParticle(HU3D_PARTICLE_DATA *entry)
{
}

// Supplies the empty default object update.
void ParticleModelEffect::update()
{
}

// Supplies the empty default particle model hook.
void ParticleModelEffect::updateParticles(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix)
{
}
