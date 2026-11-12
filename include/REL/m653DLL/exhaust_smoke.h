#ifndef REL_M653DLL_EXHAUST_SMOKE_H
#define REL_M653DLL_EXHAUST_SMOKE_H
#include "REL/framework/game_object.h"
extern "C" {
#include "game/hu3d.h"
}

u32 RandomUnsigned();
float RandomFloat(float);

extern "C" {
extern float lbl_1_bss_C0;
extern const GXColor lbl_1_rodata_3D0;
}

static inline void SetAccelerationXZ(HU3D_PARTICLE_DATA *output, double x, double z)
{
    output->accel.x = x;
    output->accel.y = 0.0f;
    output->accel.z = z;
}

/* A player's exhaust smoke: one particle emitter per car (ResourceOwner::fn_1_7370). Each frame
 * the car moves it to a point behind and above itself (fn_1_B2E8), aims it back and up from its
 * heading (fn_1_B304) and sets how hard it puffs from how far the two sticks are pushed
 * (fn_1_B320, kept in `variation`); the engine sound takes the same value. The particles' draw
 * callback (fn_1_AEA8) hands each frame back to it (fn_1_AEF8).
 * The class name is descriptive: the game keeps no name for it (no RTTI, no symbol). */
class ExhaustSmoke : public GameObject {
public:
    ExhaustSmoke(ANIMDATA *animation);
    s16 particleId;
    unsigned char unknown12[6]; /* No code names this field. */
    HU3D_PARTICLE *particles;
    Vec position;
    Vec direction;
    float variation;
    virtual ~ExhaustSmoke();
    virtual void update();
    int fn_1_A900(HU3D_PARTICLE_DATA *output);
    void fn_1_AEF8(HU3D_MODEL *model, HU3D_PARTICLE *list, Mtx matrix);
    void fn_1_B2E8(Vec *position);
    void fn_1_B304(Vec *direction);
    void fn_1_B320(float variation);
};

#endif
