#ifndef REL_M648_CAMERA_MANAGER_H
#define REL_M648_CAMERA_MANAGER_H
#include "REL/framework/minigame_singleton.h"
#include "REL/m648DLL/list_object.h"
#include "REL/m648DLL/camera_view.h"
class CameraManager : public RequestObject, public MinigameSingleton<CameraManager> {
public:
    CameraManager();
    virtual ~CameraManager();
    virtual void clearRequest(int mode);
    virtual void update();
    CameraView view;
    s32 motionFrame;
    s32 timer;
    void (CameraManager::*callback)();
    Node *target;
    s16 modelId;
    u8 listenerActive;
    u8 byteValue;
    u8 cameraSetup[0x28];
    void fn_1_89D4();
    void fn_1_8A68();
    void fn_1_8ACC();
    void fn_1_8AFC();
    void fn_1_8B2C();
    void fn_1_8C40(Node *target);
    void fn_1_8F04();
    void fn_1_8F6C();
};
#endif
