extern "C" {
#include "game/data.h"
#include "game/sprite.h"
}

// Loads and locks a sprite animation.
ANIMDATA *fn_1_B90C(unsigned long dataNum)
{
    ANIMDATA *anim = HuSprAnimRead(fn_80008DD0(dataNum, 0x10000000, 2));
    HuSprAnimLock(anim);
    return anim;
}
