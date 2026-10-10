// Creates minigame particle effects and dispatches their updates before rendering.
static const char rcsid[] = "$Id: particle.cpp,v 1.22 2004/09/09 12:24:38 saf Exp $";

#include "REL/framework/particle.h"

// Effect setup creates a fixed-capacity particle model with normal blending and an update callback.
Particle::Particle(HU3D_PARTICLE_HOOK hook, ANIMDATA *animation, int capacity)
{
    short createdModel = Hu3DParticleCreate(animation, capacity);
    modelId = createdModel;
    updateHook = hook;
    Hu3DParticleHookSet(*this, Update);
    Hu3DParticleBlendModeSet(*this, HU3D_PARTICLE_BLEND_NORMAL);
    HU3D_PARTICLE *particle = GetParticle();
    particle->work = this;
}

// Effect cleanup kills the model; the base destructor then calls Replace(-1) with this ID still
// stored.
Particle::~Particle()
{
    Hu3DModelKill(*this);
}

// Effect callbacks and accessors use the engine's particle state attached to this model.
HU3D_PARTICLE *Particle::GetParticle()
{
    return static_cast<HU3D_PARTICLE *>(Hu3DData[*this].hookData);
}

// Effect updates use this to start iterating over the model's fixed-capacity particle array.
HU3D_PARTICLE_DATA *Particle::Begin()
{
    HU3D_PARTICLE *particle = GetParticle();
    return particle->data;
}

// Effect updates use this array limit, including entries whose zero scale makes them invisible.
HU3D_PARTICLE_DATA *Particle::End()
{
    HU3D_PARTICLE *particle = GetParticle();
    return particle->data + particle->maxCnt;
}

// The engine particle draw callback runs the effect's update hook before drawing the particle
// array.
void Particle::Update(HU3D_MODEL *model, HU3D_PARTICLE *particle, Mtx matrix)
{
    Particle *owner = static_cast<Particle *>(particle->work);
    // Effect setup must supply a valid hook: every dispatched update calls it unconditionally.
    owner->updateHook(model, particle, matrix);
    // Make the updated array available to the renderer without synchronizing the cache stores here.
    DCStoreRangeNoSync(particle->data, particle->maxCnt * sizeof(HU3D_PARTICLE_DATA));
}

// Effect setup or updates set every particle's display scale; zero hides the particles.
void Particle::SetScale(float scale)
{
    Hu3DParticleScaleSet(ParticleModelId(this), scale);
}

// Effect setup or fades set every particle's alpha: levels 0.0 and 1.0 produce alpha 0 and 255.
void Particle::SetTransparency(float level)
{
    Hu3DParticleTPLvlSet(ParticleModelId(this), level);
}
