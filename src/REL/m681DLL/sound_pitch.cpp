
extern "C" {
#include "game/audio.h"
}

bool fn_1_A8A4(int handle, short pitch)
{
    return HuAudFXPitchSet(handle, pitch) == 0;
}
