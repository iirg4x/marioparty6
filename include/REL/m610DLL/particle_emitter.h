#ifndef REL_M610DLL_PARTICLE_EMITTER_H
#define REL_M610DLL_PARTICLE_EMITTER_H
#include "REL/m610DLL/module_types.h"
#include "game/hu3d.h"
#include "REL/framework/game_object.h"
/* Controls the particles emitted from a moving playing piece. */
struct ParticleEmitterState : GameObject
{
    explicit ParticleEmitterState(ANIMDATA *);
    virtual ~ParticleEmitterState();
    virtual void update();
    void initializeParticleRecord(HU3D_PARTICLE_DATA *);
    s32 fn_1_12850(HU3D_PARTICLE_DATA *);
    void fn_1_12E48(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
    void fn_1_13248(Vec *);
    void fn_1_13264(Vec *);
    void fn_1_13280(f32);
    s16 particleId;
    u8 unk12[6]; /* No code names this field. */
    HU3D_PARTICLE *hookData;
    f32 x;
    f32 y;
    f32 depth;
    f32 xScale;
    f32 unk2C; /* The game's code never reads this field. */
    f32 zScale;
    f32 variation;
};

void fn_1_13414(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
void fn_1_13A9C(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);

#endif
