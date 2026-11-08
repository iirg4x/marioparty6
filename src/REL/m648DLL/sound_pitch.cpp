extern "C" {
#include "game/audio.h"
}


bool fn_1_FE80(int handle,short pitch)
{ return HuAudFXPitchSet(handle,pitch)==0; }
