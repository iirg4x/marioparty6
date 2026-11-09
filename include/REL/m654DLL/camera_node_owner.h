#ifndef M654_CAMERA_NODE_OWNER_H
#define M654_CAMERA_NODE_OWNER_H
#include "REL/m654DLL/manager_event_base.h"
#include "REL/framework/event_packet.h"
#include "REL/m654DLL/camera_setup.h"
#include "REL/m654DLL/camera_transition.h"
#include "REL/framework/minigame_singleton.h"
struct CameraManager : ManagerEventBase, MinigameSingleton<CameraManager>, CameraFrame, EventListener {
    CameraManager();
    virtual ~CameraManager();
    virtual void handleEvent(int state);
    virtual void advanceFrame();
    virtual void receive(const EventPacket *packet);
    void (CameraManager::*callback)();
    unsigned int activeBit;
    CameraPose startPose;
    CameraPose targetPose;
    int duration;
    int elapsed;
    CameraSetup finalCamera;
    void run() { (this->*callback)(); }
    void fn_1_1948();
    void fn_1_1A38();
};

#endif
