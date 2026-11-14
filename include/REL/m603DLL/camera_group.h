#ifndef REL_M603DLL_CAMERA_GROUP_H
#define REL_M603DLL_CAMERA_GROUP_H

#include "dolphin/mtx.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/camera_frame.h"

/* One player camera frame and its viewport rectangle. */
struct CameraViewport : CameraFrame {
    f32 left, top;
    u32 unk_2C; /* No code names this field. */
    f32 width, height;
    u32 unk38; /* No code names this field. */
    u32 unk3C; /* No code names this field. */
    u8 active, marked;
};
/* Manage the four player viewports, transition camera and camera callbacks. */
struct CameraOwner : RequestObject, MinigameSingleton<CameraOwner> {
    typedef void (CameraOwner::*Callback)();
    f32 insetX, insetY;
    u32 unk_18; /* No code names this field. */
    u8 transitionRequested;
    CameraViewport cameras[4];
    CameraFrame menuState;
    s32 elapsedFrames;
    Callback callback;
    s16 modelId;
    s32 selectedCamera;

    CameraOwner();
    /* Release the camera owner and its base classes. */
    virtual ~CameraOwner() {}
    virtual void flagEnabled(int);
    virtual void update();
    void followActiveCamerasAtStart();
    void followActiveCamerasOnRequest();
    void fn_1_1F4C();
    void fn_1_2080();
    void fn_1_251C();
    void fn_1_14DC(s32 playerIndex, Vec *position, Vec *direction);
    void fn_1_15A4(int playerIndex, Vec *position, Vec *direction);
    void fn_1_161C(int playerIndex);
    void followActiveCameras();
    void fn_1_1FF0();
    void fn_1_26F8();
    void fn_1_F30(int playerIndex);
    void fn_1_F58(s32 cameraIndex, f32 left, f32 top, f32 width, f32 height);
    void fn_1_12EC(f32, f32);
};

s32 fn_1_3340(Vec *position, s32 cameraIndex, u8 widescreen);

#endif
