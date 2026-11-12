// One of the four camera hooks the game sequence builds: a Hu3D model drawn through a hook, placed each frame at
// its camera's record from the camera owner. The class name is descriptive.
#ifndef REL_M604DLL_CAMERA_HOOK_OWNER_H
#define REL_M604DLL_CAMERA_HOOK_OWNER_H

#include "REL/framework/list_object.h"
extern "C" {
#include "game/hu3d.h"
}

struct CameraHookOwner : RequestObject {
    CameraHookOwner(s32 mode);
    virtual ~CameraHookOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
    HU3D_MODELID modelId;

    Vec position;
    s32 mode; // The camera this hook follows (0-3).
    s16 nightMode;
};

#endif
