#ifndef REL_M603DLL_SHADOW_CAMERA_H
#define REL_M603DLL_SHADOW_CAMERA_H

#include "dolphin/mtx.h"

void configureShadowCamera(short mask, const Vec *direction, Vec *target, float width, float depth, float fov);

#endif
