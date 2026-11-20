#ifndef REL_M611DLL_SCENE_CAMERA_H
#define REL_M611DLL_SCENE_CAMERA_H
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/debug.h"
#include "REL/framework/singleton.h"
struct PlayerState;

// Updates the scene camera and its requested view transition.
struct SceneCamera : RequestObject, MinigameSingleton<SceneCamera> {
    SceneCamera();
    virtual ~SceneCamera();
    virtual void flagEnabled(int);
    virtual void update();
    CameraFrame camera;
    s32 frame;
    void (SceneCamera::*updateCallback)();
    PlayerState *targetPlayer;
    s16 cameraModelId;
    CameraSetup cameraView;

    void fn_1_2D8C();
    void fn_1_3244();
    void fn_1_30F0();
    void fn_1_3364();
};

// Loads and starts the camera motion.
void fn_1_3660(SceneCamera *camera);

// Stops the camera motion and clears its handle.
void fn_1_36C8(SceneCamera *camera);

#endif
