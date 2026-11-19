#ifndef REL_M605DLL_SCENE_CONTROLLER_H
#define REL_M605DLL_SCENE_CONTROLLER_H

// Stores camera transitions, saved views and the event listener.
#include "REL/framework/event_packet.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "REL/m605DLL/module_globals.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/camera_perspective.h"

#include "REL/framework/list_object.h"

// Interpolates between two values over a set number of frames.
template<class T> struct Interpolator {
    int duration;
    int elapsed;
    T value;
    T startValue;
    T targetValue;

    // Initializes the interpolated value to zero.
    Interpolator() { set(0.0f, 0); }
    T update();
    void set(T end, int frames);
    void set(T first, T end, int frames);
};

// Advances one frame and returns the interpolated value.
template<class T> T Interpolator<T>::update()
{
    T result = value;
    if (elapsed <= duration) {
        f64 frames = (f64)++elapsed;
        f64 fraction = frames / (f64)(duration + 1);
        result = (T)(startValue * (1.0 - fraction) + targetValue * fraction);
        value = result;
    }
    return result;
}

// Starts an interpolation from the current value.
template<class T> void Interpolator<T>::set(T end, int frames)
{
    set(value, end, frames);
}

// Sets interpolation endpoints and duration, handling an immediate change.
template<class T> void Interpolator<T>::set(T first, T end, int frames)
{
    duration = frames;
    elapsed = 0;
    if (frames == 0) {
        value = end;
        startValue = end;
        targetValue = end;
    } else {
        value = first;
        startValue = first;
        targetValue = end;
    }
}

#include "REL/framework/debug.h"

// Advances camera transitions and receives scene events.
struct SceneController : RequestObject, MinigameSingleton<SceneController>, EventListener {
    s32 transitionFrame;
    bool thresholdReached;
    bool elapsedThresholdReached;
    Interpolator<float> transition;
    Interpolator<float> noiseTransition;
    CameraFrame cameraWork;
    CameraPose savedPose;
    CameraPerspective perspective;
    CameraPerspective savedPerspective;
    CameraFrame savedCamera;
    Interpolator<float> secondTransition;
    CameraSetup cameraOutput;

    SceneController();
    virtual ~SceneController();
    virtual void flagEnabled(int step);
    virtual void update();
    virtual void receive(const EventPacket *packet);
};

void fn_1_23DC(SceneController *camera, f32 value, f32 duration);

void fn_1_2CB4();

#endif
