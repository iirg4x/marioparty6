#ifndef M681_VECTOR_STATE_H
#define M681_VECTOR_STATE_H

#include "REL/m681DLL/camera_pose.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"

struct CameraSetup {
    CameraFrame frame;
    unsigned char highFlag : 1;
    unsigned char middleFlag : 1;
    unsigned char lowFlag : 1;
    unsigned char remainingFlagBits : 5;
    CameraSetup();
};

class VectorStateComposite : public GameObject, public EventListener {
public:
    typedef void (VectorStateComposite::*Callback)();
    CameraFrame frame;
    CameraSetup debugCamera;
    int frameCount;
    Callback callback;
    VectorStateComposite();
    void fn_1_7C28();
    void fn_1_7C34();
    virtual ~VectorStateComposite();
    virtual void update();
    virtual void receive(const EventPacket *);
};

#endif
