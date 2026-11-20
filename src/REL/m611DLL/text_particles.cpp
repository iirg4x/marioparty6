#include "REL/m611DLL/effect_model.h"
#include "REL/m611DLL/particle_roots.h"
#include "REL/m611DLL/list_rotation.h"
#include <new>
#include "REL/m611DLL/animation_owner.h"

extern "C" {
#include "game/data.h"
#include "game/sprite.h"
#include "game/hu3d.h"
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include <math.h>
#include "humath.h"
}

#include "REL/m611DLL/positioned_particle_effect.h"
#include "REL/framework/random.h"

#include "REL/framework/phase_duration_statics.h"
#include "REL/framework/seconds_to_frames_inline.h"

// Animation shared by the positioned particles.
extern ANIMDATA *lbl_1_bss_184;

// Loads and locks the animation shared by the particle effects.
AnimationOwner::AnimationOwner()
{
    lbl_1_bss_184 = HuSprAnimDataRead(DATANUM(0x460000, 25));
    HuSprAnimLock(lbl_1_bss_184);
}

// Deletes the particle effects and releases their shared animation.
AnimationOwner::~AnimationOwner()
{
    fn_1_B210();
    HuSprAnimKill(lbl_1_bss_184);
}

// Clears the requested bit in the animation owner state.
void AnimationOwner::flagEnabled(int bit)
{
    flags &= ~(1UL << bit);
}

// Updates the timed particle effects.
void AnimationOwner::update()
{
    fn_1_B2E0();
}

// Creates a positioned particle model and attaches the selected update hook.
// Nothing in this minigame calls this.
PositionedParticleEffect::PositionedParticleEffect(Vec *initialPosition, s32 variant)
{
    this->elapsedFrames = 0;
    this->insert4(&lbl_1_bss_194);
    this->particleModelId = fn_8003216C(lbl_1_bss_184, 32);
    fn_80026790(this->particleModelId, initialPosition);
    fn_80026B8C(this->particleModelId, 6);
    if (variant == 0) {
        fn_800324E0(this->particleModelId, 20.0f);
        fn_800326D0(this->particleModelId, fn_1_B378);
    } else {
        fn_800324E0(this->particleModelId, 30.0f);
        fn_800326D0(this->particleModelId, fn_1_B5C8);
    }
    fn_80032620(this->particleModelId, 0.0f);
    fn_8003269C(this->particleModelId, 0);
    // The particle variant sets the maximum scale of the pulsing sprite.
    new EffectModel(initialPosition, static_cast<f32>((variant == 0) ? 200 : 400));
}

// Kills the owned particle model.
PositionedParticleEffect::~PositionedParticleEffect()
{
    fn_800262D0(particleModelId);
}

// Deletes all timed particle effects.
void fn_1_B210()
{
    deleteEffects(lbl_1_bss_194);
}

// Visits and updates the timed particle effects.
void fn_1_B2E0()
{
    lbl_1_bss_194.visit(fn_1_B310);
}

// Deletes a timed particle effect after its lifetime expires.
void fn_1_B310(PositionedParticleEffect *effect)
{
    if (++effect->elapsedFrames > 64) {
        delete effect;
    }
}

// Sets the position, velocity and color of a newly emitted particle.
static inline void initializeTextParticle(HU3D_PARTICLE_DATA *particle, f32 radius,
                                              f32 height, f32 cosine, f32 sine)
{
    particle->pos.x = cosine * radius;
    particle->pos.y = height;
    particle->pos.z = sine * radius;
    particle->vel.x = 3.0f * cosine;
    particle->vel.y = static_cast<f32>((40.0 * (980.665 / 60.0 / 60.0)) *
        (0.5 +
         0.5 * RandomFloat()));
    particle->vel.z = 3.0f * sine;
    particle->color.r = 224;
    particle->color.g = 240;
    particle->color.b = 255;
    particle->color.a = 255;
}

// Moves the particle and shrinks it to zero once its alpha falls below zero.
static inline void advanceTextParticle(HU3D_PARTICLE_DATA *particle)
{
    s32 alpha;

    particle->vel.y = static_cast<f32>(particle->vel.y - (980.665 / 60.0 / 60.0));
    PSVECScale(&particle->vel, &particle->vel,
               0.9f);
    PSVECAdd(&particle->vel, &particle->pos,
             &particle->pos);
    alpha = particle->color.a;
    alpha -= 4;
    if (alpha < 0) {
        alpha = 0;
        particle->scale = 0.0f;
    }
    particle->color.a = static_cast<u8>(alpha);
}

// Initializes and advances the smaller particle set.
void fn_1_B378(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx)
{
    void *hookData;
    HU3D_PARTICLE_DATA *initialParticle;
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *end;
    s32 updatedCount;
    f32 radius;

    end = &particle->data[particle->maxCnt];
    updatedCount = 0;
    if (particle->count == 0) {
        // Reads the model hook data and discards the value.
        hookData = model->hookData;
        radius = 10.0f;
        for (initialParticle = particle->data; initialParticle != end; ++initialParticle) {
            f32 angle;
            f32 cosine;
            f32 sine;

            angle = static_cast<f32>(M_PI *
                          (RandomFloat() * 360.0f -
                           180.0f) /
                          180.0);
            cosine = cos(angle);
            sine = sin(angle);
            initializeTextParticle(initialParticle, radius, 10.0f, cosine, sine);
        }
    }

    for (entry = particle->data; entry != end; ++entry) {
        advanceTextParticle(entry);
        ++updatedCount;
    }
    particle->emitCnt = updatedCount;
}

// Initializes and advances the larger particle set.
void fn_1_B5C8(HU3D_MODEL *, HU3D_PARTICLE *particle, Mtx)
{
    HU3D_PARTICLE_DATA *initialParticle;
    HU3D_PARTICLE_DATA *entry;
    HU3D_PARTICLE_DATA *end;
    s32 updatedCount;
    f32 radius;

    end = &particle->data[particle->maxCnt];
    updatedCount = 0;
    if (particle->count == 0) {
        radius = 30.0f;
        for (initialParticle = particle->data; initialParticle != end; ++initialParticle) {
            f32 angle;
            f32 cosine;
            f32 sine;

            angle = static_cast<f32>(M_PI *
                          (RandomFloat() * 360.0f -
                           180.0f) /
                          180.0);
            cosine = cos(angle);
            sine = sin(angle);
            initializeTextParticle(initialParticle, radius, 15.0f, cosine, sine);
        }
    }

    for (entry = particle->data; entry != end; ++entry) {
        advanceTextParticle(entry);
        ++updatedCount;
    }
    particle->emitCnt = updatedCount;
}

// Animation shared by the positioned particles.
ANIMDATA *lbl_1_bss_184;

// Active positioned particle effects.
ListRoot<PositionedParticleEffect> lbl_1_bss_194;
