#include "REL/m672DLL/sound_pitch.h"
extern "C" {
#include "REL/framework/assertion.h"
#include "game/charman.h"
#include "game/data.h"
#include "game/hu3d.h"
#include "game/mg/timer.h"
#include "game/flag.h"
#include "game/sprite.h"
#include "game/audio.h"
#include "game/window.h"
#include "game/gamework.h"
#include "game/memory.h"
}

#include "REL/m672DLL/framework/model.h"

bool fn_1_12488(int handle,short pitch)
{ return HuAudFXPitchSet(handle,pitch)==0; }
