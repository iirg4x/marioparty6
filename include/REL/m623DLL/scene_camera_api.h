#ifndef REL_M623DLL_SCENE_CAMERA_API_H
#define REL_M623DLL_SCENE_CAMERA_API_H
#include "dolphin/types.h"
class Model;

// Places the player model relative to camera 1.
void fn_1_C89C(s32 player, Model &model);

// Selects the idle camera state and restores the primary view bounds.
void fn_1_C1C4(Model *model);

// Starts a blend from the current team camera.
void fn_1_C0C8(void);

#endif
