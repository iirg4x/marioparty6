#ifndef M634_CAMERA_H
#define M634_CAMERA_H
#include "REL/framework/game_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_inline.h"
#include <functional>
#include "dolphin/mtx.h"
#include "REL/framework/camera_frame.h"
struct SceneCameraMotion : CameraFrame {
    Vec initialPosition;
    Vec initialTarget;

    u8 unk3C[0x10]; /* No code names this field. */

    u8 active;

    // Returns the initial camera position.
    const Vec &getInitialPosition() const
    {
        return initialPosition;
    }
    // Returns the initial camera target.
    const Vec &getInitialTarget() const
    {
        return initialTarget;
    }
    // Stores the object position.
    void setPosition(const Vec &value)
    {
        position = value;
    }
    // Stores the camera target.
    void setTarget(const Vec &value)
    {
        target = value;
    }
};

struct SceneCameraDefaults {
    Vec initialPosition;
    Vec up;
    Vec initialTarget;
};

struct CameraSceneWork;

typedef void (CameraSceneWork::*MotionCallback)();

struct CameraSceneWork : GameObject, MinigameSingleton<CameraSceneWork> {
    SceneCameraMotion camera0;
    SceneCameraMotion camera;
    s16 motionId;

    u8 unkB2[2]; /* No code names this field. */

    MotionCallback initialCallback;
    EventHandlerManager registration;
    CameraSceneWork();
    virtual ~CameraSceneWork();
    virtual void update();
    void fn_1_11064(const EventPacket &event);
    void fn_1_10E0C();
    void fn_1_10DB8();
    void fn_1_10F8C(f32 amount);
    f32 fn_1_111E0();
    // Stores the next callback.
    void setCallback(MotionCallback next)
    {
        initialCallback = next;
    }
};

template <typename Payload> struct CallbackRecord : EventCallback {
    Payload callback;
    // Releases the object and its member-owned resources.
    virtual ~CallbackRecord() { }
    virtual void receive(const EventPacket *value);
    // Stores the callback payload.
    CallbackRecord(const Payload &payload)
        : EventCallback()
        , callback(payload)
    {
    }
};

// Invokes the stored event callback.
template <typename Payload> void CallbackRecord<Payload>::receive(const EventPacket *value)
{
    callback(*value);
}

// Registers a camera event callback for the requested key.
template <typename Payload>
void fn_1_112D4(EventHandlerManager *owner, const Payload &payload, s32 key)
{
    owner->addHandler(new CallbackRecord<Payload>(payload), key);
}

#endif
