#ifndef REL_M605DLL_MODEL_POSITION_ENGINE_H
#define REL_M605DLL_MODEL_POSITION_ENGINE_H

#include "dolphin/types.h"

extern "C" {
// m605's caller (scene_stage.cpp) passes the model identifier as a full word, while
// game/hu3d.h:401 declares the argument as HU3D_MODELID, a signed short.
void fn_80026760(s32 model, f32 x, f32 y, f32 z);
}

#endif
