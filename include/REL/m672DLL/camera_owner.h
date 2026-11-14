#ifndef REL_M672DLL_CAMERA_OWNER_H
#define REL_M672DLL_CAMERA_OWNER_H
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/event.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
#include "REL/framework/camera_frame.h"

typedef class CameraWork {
public:
    CameraFrame frame;
    EffectVector cameraPosition;
    EffectVector cameraTarget;
    // The game never reads this member.
    Vec unk3C;
    // The game never reads this member.
    u32 unk48;
    s8 enabled;
    // Copy the position coordinates.
    void setPosition(const Vec &value) {
        frame.position.x = value.x; frame.position.y = value.y; frame.position.z = value.z;
    }
    // Copy the camera target coordinates.
    void setTarget(const Vec &value) {
        frame.target.x = value.x; frame.target.y = value.y; frame.target.z = value.z;
    }
    // Copy the position coordinates.
    void setPosition(const EffectVector &target) {
        setPosition(target.getValue());
    }
    // Copy the camera target coordinates.
    void setTarget(const EffectVector &target) {
        setTarget(target.getValue());
    }
} CameraWork;

class CameraOwner;
typedef void (CameraOwner::*CameraUpdate)();
#include "REL/m672DLL/member_callback.h"
typedef std::mem_fun1_t<void, CameraOwner, EventMessage *> CameraMemberFunction;
typedef CallbackBinding<CameraOwner> CameraCallbackPayload;
typedef class CameraOwner : public GameObject {
public:
    OwnedModel model;
    CameraWork camera;
    CameraWork cameraCopy;
    f32 fade;
    EventHandlerManager callbackState;
    CameraUpdate updateCallback;
    // Select the update callback.
    void setUpdate(CameraUpdate callback) { updateCallback = callback; }
    CameraOwner();
    virtual ~CameraOwner();
    virtual void update();
    void fn_1_A6E8();
    void fn_1_A924(EventMessage *);
    void fn_1_A978(float amount);
    void fn_1_A6E4();
} CameraOwner;
#endif
