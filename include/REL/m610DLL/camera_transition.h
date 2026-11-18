#ifndef REL_M610DLL_CAMERA_TRANSITION_H
#define REL_M610DLL_CAMERA_TRANSITION_H
#include "REL/framework/game_object.h"
#include "REL/framework/event_packet.h"
#include "REL/framework/debug.h"
#include "REL/framework/scene_shadow.h"
/* Own the gameplay camera and its transition between two perspectives. */
struct CameraTransitionState : GameObject, EventListener
{
    typedef void (CameraTransitionState::*Callback)();
    Callback callback;
    CameraFrame frame;
    CameraPerspective cameraPerspective, shadowPerspective;
    CameraSetup debug;
    /* Nothing in this module writes this camera pose. */
    CameraPose destination;
    int elapsedFrames;
    CameraTransitionState();
    virtual ~CameraTransitionState();
    virtual void update();
    virtual void receive(const EventPacket *);
    void fn_1_E38();
    void fn_1_F64();
    void fn_1_1240();
};
#endif
