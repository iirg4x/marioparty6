#ifndef REL_M652DLL_PLAYER_CAMERA_H
#define REL_M652DLL_PLAYER_CAMERA_H

#include "REL/framework/list_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/debug.h"
#include "REL/framework/event_packet.h"

class PlayerCamera : public RequestObject, public CameraFrame,
                  public EventListener {
public:
    typedef void (PlayerCamera::*Action)();
    PlayerCamera(unsigned int index);
    virtual ~PlayerCamera();
    virtual void flagEnabled(int mask);
    virtual void update();
    virtual void receive(const EventPacket *event);
    void fn_1_4118(); void fn_1_3E2C(); void fn_1_4458(); void fn_1_414C();
    Action action;
    int count;
    unsigned short modelId;
    CameraPose position;
    int frame;
    CameraSetup state;
};

#endif
