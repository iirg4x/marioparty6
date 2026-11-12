// A game object holding one sprite animation, loaded and locked for as long as it lives.
#include "REL/m604DLL/animation_menu.h"
#include "game/data.h"

// The data numbers of the animations.
const int lbl_1_rodata_9F0[1] = {0x3F0003};

ParticleAnimations::ParticleAnimations()
{
    int index;
    for (index = 0; index < 1; index++) {
        animations[index] = HuSprAnimRead(fn_80008DD0(lbl_1_rodata_9F0[index], 0x10000000, 2));
        HuSprAnimLock(animations[index]);
    }
}

ParticleAnimations::~ParticleAnimations()
{
    int spriteIndex;
    for (spriteIndex = 0; spriteIndex < 1; spriteIndex++) {
        HuSprAnimKill(animations[spriteIndex]);
    }
}

void ParticleAnimations::flagEnabled(int bit) { flags &= ~(1 << bit); }
void ParticleAnimations::update() {}
