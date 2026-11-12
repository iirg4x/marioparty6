#ifndef M653_CAMERA_CONTROLLER_H
#define M653_CAMERA_CONTROLLER_H
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/debug.h"
#include "REL/framework/frame_interpolator.h"
#include "REL/framework/camera_frame.h"
struct CameraController653 : GameObject, EventListener {
    typedef void (CameraController653::*Callback)();
    Callback callback;
    CameraFrame frame;
    CameraSetup debugCamera;
    unsigned char unknown6C[0x1C]; /* No code names this field. */
    s16 cameraModelId;
    s16 motionId;
    FrameInterpolator<float> shake;
    CameraController653();
    virtual ~CameraController653();
    virtual void update();
    virtual void receive(EventMessage *);
    void fn_1_334();
    void fn_1_3C8();
    void fn_1_3CC();
    void setCallback(Callback function) { callback = function; }
};

void fn_1_410(CameraController653 *owner);
void fn_1_48C(CameraController653 *work);
#endif
