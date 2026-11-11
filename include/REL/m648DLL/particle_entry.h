#ifndef REL_M648_PARTICLE_ENTRY_H
#define REL_M648_PARTICLE_ENTRY_H
#include "REL/framework/list.h"
#include "dolphin/mtx.h"

// A particle model whose particles fall and fade to white; it deletes itself when they are gone.
struct WhiteParticles : ListNode<WhiteParticles> {
    s16 modelId;
    s32 state;
    WhiteParticles(Vec *position);
    ~WhiteParticles();
};

// A particle model whose particles take colours from a table and spin; it deletes itself when
// they are gone.
struct ColorParticles : ListNode<ColorParticles> {
    s16 modelId;
    s32 state;
    ColorParticles(Vec *position);
    ~ColorParticles();
};
#endif
