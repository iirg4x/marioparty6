#ifndef REL_M623DLL_PARTICLE_SETUP_H
#define REL_M623DLL_PARTICLE_SETUP_H
#include "REL/framework/list.h"
#include "REL/framework/vector3.h"

// Owns a scattered particle model.
struct ScatterParticles : ListNode<ScatterParticles> {
    ScatterParticles(const Vector3 *source, int viewIndex);
    ~ScatterParticles();
    s16 modelId;
    // The game's code never reads this field.
    int unk0C;
};

#endif
