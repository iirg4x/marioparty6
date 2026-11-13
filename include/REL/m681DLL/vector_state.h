#ifndef M681_VECTOR_STATE_H
#define M681_VECTOR_STATE_H

#include "REL/framework/camera_frame.h"
#include "REL/framework/debug.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

class GameCamera : public GameObject, public EventListener {
public:
    typedef void (GameCamera::*Callback)();
    CameraFrame frame;
    CameraSetup debugCamera;
    int frameCount;
    Callback callback;
    GameCamera();
    void fn_1_7C28();
    void fn_1_7C34();
    virtual ~GameCamera();
    virtual void update();
    virtual void receive(const EventPacket *);
};

#endif
