// Three particle effects and their shared sprite animations.

extern "C" {
#include "game/sprite.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "dolphin/mtx/GeoTypes.h"
#include "dolphin/os/OSCache.h"
#include "game/hu3d.h"
#include "game/memory.h"
#include "game/data.h"
#include <math.h>
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "humath.h"
}

#include <new>
#include "REL/framework/list.h"
#include "REL/framework/vector3.h"
#include "REL/framework/list_object.h"
#include "REL/framework/linknode.h"
#include "REL/framework/model.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/singleton.h"
#include "REL/m623DLL/particle_animations.h"
#include "REL/framework/random.h"
#include "REL/m623DLL/particle_setup.h"
#include "REL/m623DLL/particle_object.h"
#include "REL/m623DLL/player_trail_particle.h"

void fn_1_5304(HU3D_MODEL *, HU3D_PARTICLE *particles, Mtx matrix);

// The three sprite animation resources.
const int lbl_1_rodata_190[3] = { DATANUM(0x520000, 51), DATANUM(0x520000, 54),
                                  DATANUM(0x520000, 53) };

void fn_1_457C();
void fn_1_4AF8();
void fn_1_5180();
void fn_1_464C();
void fn_1_4BC8();
void fn_1_5250();

// Loads the particle animation table.
ParticleAnimations::ParticleAnimations()
{
    int index;
    for (index = 0; index < 3; index++) {
        animations[index] = HuSprAnimDataRead(lbl_1_rodata_190[index]);
        HuSprAnimLock(animations[index]);
    }
}

// Destroys the three particle lists and releases their animations.
ParticleAnimations::~ParticleAnimations()
{
    fn_1_457C();
    fn_1_4AF8();
    fn_1_5180();
    for (int index = 0; index < 3; index++) {
        HuSprAnimKill(animations[index]);
    }
}

// Returns the particle animation at the supplied index.
ANIMDATA *fn_1_423C(unsigned int entryIndex)
{
    return ParticleAnimations::getChecked().animations[entryIndex];
}

// Clears the bit for the supplied request.
void ParticleAnimations::flagEnabled(int request)
{
    flags &= ~(1UL << request);
}

// Updates the three particle effect lists.
void ParticleAnimations::update()
{
    fn_1_464C();
    fn_1_4BC8();
    fn_1_5250();
}

extern ListRoot<ScatterParticles> lbl_1_bss_48C;

void fn_1_4700(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix);

// Creates the scatter effect at the supplied position.
// Nothing in this minigame calls this.
ScatterParticles::ScatterParticles(const Vector3 *source, int viewIndex)
{
    ANIMDATA *particleData;

    insert4(&lbl_1_bss_48C);
    Vector3 position = *source;
    position.z = (100.0f * RandomFloat(-1.0f, 1.0f)) + position.z;
    position.x += 100.0f * RandomFloat(-1.0f, 1.0f);
    particleData = fn_1_423C(0);
    modelId = fn_8003216C(particleData, 1);
    fn_80026790(modelId, &position);
    fn_80026B8C(modelId, 7);
    fn_800324E0(modelId, 100.0f);
    fn_800326D0(modelId, fn_1_4700);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    unk0C = 0;
    fn_80026B64(modelId, viewIndex == 0 ? 0x1U : 0x2U);
}

// Releases the scatter effect.
ScatterParticles::~ScatterParticles()
{
    fn_800262D0(modelId);
}

// Destroys the list of scatter effects.
void fn_1_457C()
{
    while (!lbl_1_bss_48C.empty())
        delete lbl_1_bss_48C.back();
}

void fn_1_467C(ScatterParticles *);

// Updates the list of scatter effects.
void fn_1_464C()
{
    lbl_1_bss_48C.visit(fn_1_467C);
}

// Removes a scatter effect after its particle data expires.
void fn_1_467C(ScatterParticles *owner)
{
    HU3D_PARTICLE *hookData = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);
    if (hookData->dataCnt < 0) {
        delete owner;
    }
}

extern ListRoot<ItemHitParticles> lbl_1_bss_4A0;
void fn_1_4C7C(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix);
// Initializes and advances the scatter effect's entries.
void fn_1_4700(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = particles->data + particles->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    s32 green;
    s32 blue;
    s32 alpha;
    s32 updatedCount;

    if (particles->count == 0) {
        for (initializingEntry = particles->data; initializingEntry != end; ++initializingEntry) {
            initializingEntry->vel.x = 2.5f * RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.y = 2.5f * RandomFloat(-1.0f, 1.0f);
            initializingEntry->vel.z = 2.5f * RandomFloat(-1.0f, 1.0f);
            initializingEntry->pos.x = initializingEntry->pos.y = initializingEntry->pos.z = 0.0f;
            initializingEntry->color.r = 0x80;
            initializingEntry->color.g = 0xFF;
            initializingEntry->color.b = 0xFF;
            initializingEntry->color.a = 0x80;
        }
    }

    updatedCount = 0;
    for (entry = particles->data; entry != end; ++entry) {
        PSVECScale(&entry->vel, &entry->vel, 0.8f);
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        entry->color.r = 0xFF;
        green = entry->color.g;
        green += 0x20;
        if (green > 0xFF) {
            green = 0xFF;
        }
        entry->color.g = green;
        blue = entry->color.b;
        blue += 0x20;
        if (blue > 0xFF) {
            blue = 0xFF;
        }
        entry->color.b = blue;
        alpha = entry->color.a;
        alpha -= 8;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            particles->dataCnt = -1;
        }
        entry->color.a = alpha;
        ++updatedCount;
    }

    particles->emitCnt = updatedCount;
}

// Creates the particles emitted by an item hit.
ItemHitParticles::ItemHitParticles(Vector3 *position, int viewIndex)
{
    ANIMDATA *particleData;

    insert4(&lbl_1_bss_4A0);
    particleData = fn_1_423C(1);
    modelId = fn_8003216C(particleData, 16);
    fn_80026790(modelId, position);
    fn_80026B8C(modelId, 7);
    fn_800324E0(modelId, 70.0f + (50.0f * RandomFloat()));
    fn_800326D0(modelId, fn_1_4C7C);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    unk0C = 0;
    fn_80026B64(modelId, viewIndex == 0 ? 0x1U : 0x2U);
}

// Releases the item-hit particle model.
ItemHitParticles::~ItemHitParticles()
{
    fn_800262D0(modelId);
}

// Destroys the item-hit particles.
void fn_1_4AF8()
{
    while (!lbl_1_bss_4A0.empty())
        delete lbl_1_bss_4A0.back();
}

void fn_1_4BF8(ItemHitParticles *);

// Updates the item-hit particles.
void fn_1_4BC8()
{
    lbl_1_bss_4A0.visit(fn_1_4BF8);
}

// Removes an item-hit effect after its particles expire.
void fn_1_4BF8(ItemHitParticles *owner)
{
    HU3D_PARTICLE *hookData = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);
    if (hookData->dataCnt < 0) {
        delete owner;
    }
}

// The placement offset for the item-hit particles.
const Vec lbl_1_rodata_1BC = { 0.0f, 0.0f, 200.0f };
// The available colors for the item-hit particles.
const GXColor lbl_1_rodata_1C8[8] = { { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xE0, 0xFF },
                                      { 0xFF, 0xE0, 0xFF, 0xFF }, { 0xFF, 0xE0, 0xE0, 0xFF },
                                      { 0xE0, 0xFF, 0xFF, 0xFF }, { 0xE0, 0xFF, 0xE0, 0xFF },
                                      { 0xE0, 0xE0, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xFF } };

// Initializes and advances the particles emitted by an item hit.
void fn_1_4C7C(HU3D_MODEL *model, HU3D_PARTICLE *particles, Mtx matrix)
{
    HU3D_PARTICLE_DATA *end = particles->data + particles->maxCnt;
    HU3D_PARTICLE_DATA *initializingEntry;
    HU3D_PARTICLE_DATA *entry;
    Mtx rotationX;
    Mtx rotationY;
    Vec transformedPosition;
    s32 alpha;
    f32 frameSeconds;

    if (particles->count == 0) {
        for (initializingEntry = particles->data; initializingEntry != end; ++initializingEntry) {
            frameSeconds = 1.0f / 60.0f;
            MTXRotDeg(rotationX, 'X', RandomFloat() * 180.0f - 90.0f);
            MTXRotDeg(rotationY, 'Y', RandomFloat() * 360.0f - 180.0f);
            PSMTXConcat(rotationY, rotationX, rotationX);
            PSMTXMultVec(rotationX, &lbl_1_rodata_1BC, &transformedPosition);
            initializingEntry->vel.x = transformedPosition.x * frameSeconds;
            initializingEntry->vel.y = transformedPosition.y * frameSeconds;
            initializingEntry->vel.z = transformedPosition.z * frameSeconds;
            initializingEntry->pos.x = transformedPosition.x;
            initializingEntry->pos.y = transformedPosition.y;
            initializingEntry->pos.z = transformedPosition.z;
            initializingEntry->color = lbl_1_rodata_1C8[RandomUnsigned() & 7];
            initializingEntry->zRot = (f32) (M_PI * (f64) (360.0f * RandomFloat()) / 180.0);
        }
    }

    for (entry = particles->data; entry != end; ++entry) {
        PSVECAdd(&entry->vel, &entry->pos, &entry->pos);
        alpha = entry->color.a;
        alpha -= particles->emitCnt;
        if (alpha < 0) {
            alpha = 0;
            entry->scale = 0.0f;
            particles->dataCnt = -1;
        }
        entry->color.a = alpha;
        entry->zRot = (f32) (M_PI * (5.0 + 180.0 * ((f64) entry->zRot / M_PI)) / 180.0);
    }

    particles->emitCnt += 1;
    DCStoreRangeNoSync(particles->data, particles->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

extern ListRoot<PlayerTrailParticles> lbl_1_bss_4B4;

// Creates the player trail at the supplied position.
PlayerTrailParticles::PlayerTrailParticles(const Vector3 *position, int viewIndex)
{
    ANIMDATA *particleAnimation;

    insert4(&lbl_1_bss_4B4);
    particleAnimation = fn_1_423C(2);
    modelId = fn_8003216C(particleAnimation, 8);

    Vector3 particlePosition = *position;
    if (viewIndex == 0) {
        particlePosition.x -= 200.0f;
    } else {
        particlePosition.x -= 100.0f;
    }
    fn_80026790(modelId, &particlePosition);
    fn_80026B8C(modelId, 7);
    fn_800324E0(modelId, 20.0f + 20.0f * RandomFloat());
    fn_800326D0(modelId, fn_1_5304);
    fn_80032620(modelId, 0.0f);
    fn_8003269C(modelId, 0);
    unk0C = 0;

    fn_80026B64(modelId, viewIndex == 0 ? 0x1U : 0x2U);
}

// Releases the player trail model.
PlayerTrailParticles::~PlayerTrailParticles()
{
    fn_800262D0(modelId);
}

// Destroys the player trail particles.
void fn_1_5180()
{
    while (!lbl_1_bss_4B4.empty())
        delete lbl_1_bss_4B4.back();
}

void fn_1_5280(PlayerTrailParticles *);

// Updates the player trail particles.
void fn_1_5250()
{
    lbl_1_bss_4B4.visit(fn_1_5280);
}

// Active scatter effects.
ListRoot<ScatterParticles> lbl_1_bss_48C;
// Active item-hit effects.
ListRoot<ItemHitParticles> lbl_1_bss_4A0;
// Active player trails.
ListRoot<PlayerTrailParticles> lbl_1_bss_4B4;

// Removes a player trail after its particles expire.
void fn_1_5280(PlayerTrailParticles *owner)
{
    HU3D_PARTICLE *hookData = static_cast<HU3D_PARTICLE *>(lbl_802C01C8[owner->modelId].hookData);
    if (hookData->dataCnt < 0) {
        delete owner;
    }
}

// An unused position offset.
const Vec lbl_1_rodata_220 = { 0.0f, 0.0f, 75.0f };
// The player trail colors with their starting opacities.
static const GXColor fadeColors[8] = {
    { 0xFF, 0xFF, 0xFF, 0xFF }, { 0xFF, 0xFF, 0xFF, 0xE0 }, { 0xFF, 0xFF, 0xFF, 0xC0 },
    { 0xFF, 0xFF, 0xFF, 0xA0 }, { 0xFF, 0xFF, 0xFF, 0x80 }, { 0xFF, 0xFF, 0xFF, 0x5C },
    { 0xFF, 0xFF, 0xFF, 0x40 }, { 0xFF, 0xFF, 0xFF, 0x20 },
};

// Initializes and advances the player trail particles.
void fn_1_5304(HU3D_MODEL *, HU3D_PARTICLE *particles, Mtx matrix)
{
    s32 index;
    HU3D_PARTICLE_DATA *end = particles->data + particles->maxCnt;

    if (particles->count == 0) {
        f32 initialValue = 10.0f + 30.0f * RandomFloat();
        HU3D_PARTICLE_DATA *record;

        index = 0;
        record = particles->data;

        while (record != end) {
            f32 rotationSide;
            f32 side;
            static f32 frameTime = 1.0f / 60.0f;
            if ((index & 1) == 0) {
                side = 1.0f;
            } else {
                side = -1.0f;
            }
            rotationSide = side;

            record->vel.x = (2500.0f + 500.0f * RandomFloat()) * frameTime;
            record->vel.y = (400.0f + 200.0f * RandomFloat()) * frameTime;
            record->vel.z = rotationSide * ((300.0f + 500.0f * RandomFloat()) * frameTime);
            record->pos.x = 0.0f;
            record->pos.y = 0.0f;
            record->pos.z = rotationSide * (120.0 * ((f64) RandomFloat() - 0.5));

            record->color = fadeColors[RandomUnsigned() & 7];

            record->zRot = (f32) ((M_PI * (360.0f * RandomFloat())) / 180.0);
            record++;
            index++;
        }
    }

    HU3D_PARTICLE_DATA *record;

    index = 0;
    record = particles->data;

    while (record != end) {
        f32 side;
        f32 rotationSide;
        s32 alpha;

        PSVECAdd(&record->vel, &record->pos, &record->pos);
        if ((index & 1) == 0) {
            side = 1.0f;
        } else {
            side = -1.0f;
        }
        rotationSide = side;
        alpha = record->color.a;
        alpha -= particles->emitCnt;
        if (alpha < 0) {
            alpha = 0;
            record->scale = 0.0f;
            particles->dataCnt = -1;
        }
        record->color.a = alpha;
        record->zRot = (f32) ((M_PI * ((180.0 * ((f64) record->zRot / M_PI)) -
                                       (f64) (((f32) alpha * rotationSide) / 10.0f))) /
                              180.0);
        record++;
        index++;
    }

    particles->emitCnt++;
    DCStoreRangeNoSync(particles->data, particles->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}
