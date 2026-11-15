#ifndef REL_M634_PARTICLE_EFFECT_H
#define REL_M634_PARTICLE_EFFECT_H

#include "REL/m634DLL/particle_object_base.h"
extern "C" {
#include "game/charman.h"
#include "game/hu3d.h"
}

void fn_1_122E8(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
class ParticleModel634 : public ParticleObjectBase {
public:
    s16 modelId;
    u8 unk12[6]; /* No code names this field. */
    HU3D_PARTICLE *hookData;
    f32 x, y, z;
    u8 finished;
    u8 unk29[3]; /* No code names this field. */
    f32 scale;
    s32 initializedCount, activeCount;
    ParticleModel634(ANIMDATA *, Vec *, s32);
    typedef void (ParticleModel634::*Callback)();
    Callback stateCallback;
    void fn_1_11914();
    void fn_1_11918();
    void fn_1_119FC();
    void fn_1_11A00();
    virtual ~ParticleModel634();
    virtual void update();
    virtual void mode(s32);
    virtual void updateEntry(HU3D_PARTICLE_DATA *);
    virtual void forward(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
};
class ParticleEffect634 : public ParticleModel634 {
public:
    typedef void (ParticleModel634::*Callback)();
    Callback callback;
    ParticleEffect634(ANIMDATA *, Vec *, s32, s32);
    virtual ~ParticleEffect634();
    virtual void update();
    virtual void updateEntry(HU3D_PARTICLE_DATA *);
    virtual void forward(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
};

class ParticleRadialEffect634 : public ParticleModel634 {
public:
    typedef void (ParticleModel634::*Callback)();
    Callback callback;
    f32 elapsed, rotation;
    s32 angleStep, modelIndex;
    u8 enabled;
    ParticleRadialEffect634(ANIMDATA *, Vec *, s32, s32, s32, f32);
    void fn_1_12FD8(HU3D_PARTICLE_DATA *);
    void fn_1_1315C(HU3D_PARTICLE_DATA *);
    virtual ~ParticleRadialEffect634();
    virtual void update();
    virtual void forward(HU3D_MODEL *, HU3D_PARTICLE *, Mtx);
};

#endif
