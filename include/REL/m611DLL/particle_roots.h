#ifndef REL_M611DLL_PARTICLE_ROOTS_H
#define REL_M611DLL_PARTICLE_ROOTS_H

#include "REL/framework/list.h"

struct RadialBurstModel;
struct FadingParticleModel;
struct RotatingBurstModel;
struct EffectNode;
struct EffectModel;
struct RotatedSpriteEffect;
struct GroundSpriteEffect;
struct MotionParticleOwner;
struct ParticleStrip;
struct TimedFlash;

// Radial burst models.
extern ListRoot<RadialBurstModel> lbl_1_bss_AC;
// Active single fading particles.
extern ListRoot<FadingParticleModel> lbl_1_bss_C0;
// Rotating burst models.
extern ListRoot<RotatingBurstModel> lbl_1_bss_D4;
// Expanding ground effects.
extern ListRoot<EffectNode> lbl_1_bss_E8;
// Pulsing sprite effects.
extern ListRoot<EffectModel> lbl_1_bss_FC;
// Active rotated sprites.
extern ListRoot<RotatedSpriteEffect> lbl_1_bss_110;
// Ground sprite effects.
extern ListRoot<GroundSpriteEffect> lbl_1_bss_124;
// Active falling particle pools.
extern ListRoot<MotionParticleOwner> lbl_1_bss_138;
// Active position strips.
extern ListRoot<ParticleStrip> lbl_1_bss_15C;
// Timed flash effects.
extern ListRoot<TimedFlash> lbl_1_bss_170;

#endif
