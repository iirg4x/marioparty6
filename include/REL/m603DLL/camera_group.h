#ifndef REL_M603DLL_CAMERA_GROUP_H
#define REL_M603DLL_CAMERA_GROUP_H

#include "dolphin/mtx.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

struct CameraGroupBBC {
    Vec position, rotation, target;
    f32 left, top;
    u32 unknownValue;
    f32 width, height;
    u32 unknown38, unknown3C;
    u8 active, marked;
};
struct CameraOwnerBBC : RequestObject, MinigameSingleton<CameraOwnerBBC> {
 typedef void (CameraOwnerBBC::*Callback)();
 f32 insetX,insetY; u32 unknownValue; u8 transitionRequested;
 CameraGroupBBC cameras[4]; u8 menuState[0x24]; s32 elapsedFrames; Callback callback; s16 modelId; s32 selectedCamera;
 CameraOwnerBBC(); virtual ~CameraOwnerBBC(); virtual void flagEnabled(int); virtual void update();
 void fn_1_1944(); void fn_1_1C48(); void fn_1_1F4C(); void fn_1_2080(); void fn_1_251C();
 void fn_1_F30(int playerIndex);
 void fn_1_F58(s32 cameraIndex, f32 left, f32 top, f32 width, f32 height);
 void fn_1_12EC(f32,f32);
};

#endif
