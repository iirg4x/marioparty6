#ifndef M654_CAMERA_NODE_OWNER_H
#define M654_CAMERA_NODE_OWNER_H
#include "REL/framework/list_object.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/debug.h"
#include "REL/m654DLL/camera_transition.h"
#include "REL/framework/singleton.h"
struct CameraManager : RequestObject, MinigameSingleton<CameraManager>, CameraFrame, EventListener {
    CameraManager();
    virtual ~CameraManager();
    virtual void flagEnabled(int state);
    virtual void update();
    virtual void receive(const EventPacket *packet);
    void (CameraManager::*callback)();
    unsigned int activeBit;
    CameraPose startPose;
    CameraPose targetPose;
    int duration;
    int elapsed;
    CameraSetup debugCamera;
    // Invokes the current camera state.
    void run() { (this->*callback)(); }
    void fn_1_1948();
    void fn_1_1A38();
};

#endif
