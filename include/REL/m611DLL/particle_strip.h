#ifndef REL_M611DLL_PARTICLE_STRIP_H
#define REL_M611DLL_PARTICLE_STRIP_H

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/list.h"
#include "dolphin/mtx.h"

// A shaded strip joining the crane joint to the moving effect.
struct ParticleStrip : ListNode<ParticleStrip> {
    OwnedModel model;
    Vec start;
    Vec end;
    ParticleStrip();
    ~ParticleStrip();
};

void fn_1_8954(ParticleStrip *strip);
void fn_1_897C(ParticleStrip *);

void fn_1_8980(ParticleStrip *strip, Mtx sourceMatrix);
void fn_1_8800(HU3D_MODEL *model, Mtx *matrix);

#endif
