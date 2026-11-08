#ifndef REL_M648_CAMERA_EFFECT_OWNER_H
#define REL_M648_CAMERA_EFFECT_OWNER_H
#include "REL/framework/game_object.h"
#include "REL/m648DLL/node_owner_198.h"
#include "REL/m648DLL/released_model.h"
#include "dolphin/mtx.h"

struct CameraEffectOwner318 : NodeOwner305B0_198 {
    typedef void (CameraEffectOwner318::*Callback)();
    Callback callback;
    ReleasedModel454 modelIdA;
    ReleasedModel454 modelIdB;
    s32 audioHandle;
    u8 stopped;
    s8 complete;
    Vec startPosition;
    Vec position;
    Vec movement;
    Vec resetPosition;
    f32 fade;
    f32 angle;
    s32 timer;
    s32 updateCount;
    s32 groundHeight;
    s32 updatesRemaining;
    CameraEffectOwner318(Vec *position);
    void fn_1_BF0();
    void fn_1_BA8();
    void fn_1_8B4();
    void fn_1_E00();
    void fn_1_E4C();
};

#include "REL/m648DLL/camera_effect_functions.h"

#endif
