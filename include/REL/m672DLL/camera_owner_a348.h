#ifndef REL_M672DLL_CAMERA_OWNER_A348_H
#define REL_M672DLL_CAMERA_OWNER_A348_H
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "REL/m672DLL/event.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/m672DLL/model_bf64.h"
#include "REL/framework/camera_frame.h"

class CameraVector {
public:
    Vec value;
    const Vec &getValue() const;
};

typedef class CameraWork : public CameraFrame {
public:
    CameraVector cameraPosition;
    CameraVector cameraTarget;
    Vec unknown3C;
    u32 unknown48;
    s8 enabled;

    void setPosition(const Vec &value) {
        vectors[0].x = value.x; vectors[0].y = value.y; vectors[0].z = value.z;
    }
    void setTarget(const Vec &value) {
        vectors[2].x = value.x; vectors[2].y = value.y; vectors[2].z = value.z;
    }
    void setPosition(const CameraVector &target) {
        setPosition(target.getValue());
    }
    void setTarget(const CameraVector &target) {
        setTarget(target.getValue());
    }
} CameraWork;

class CameraOwner;
typedef void (CameraOwner::*CameraCallbackA348)();
#include "REL/m672DLL/early_callback_8980.h"
typedef std::mem_fun1_t<void, CameraOwner, EventMessage *> CameraMemberFunction;
typedef CallbackBinding<CameraOwner> CameraCallbackPayload;

static inline CameraMemberFunction CameraMember(void (CameraOwner::*fn)(EventMessage *)) {
    return CameraMemberFunction(fn);
}
static inline CameraCallbackPayload BindCamera(const CameraMemberFunction &fn,
                                         CameraOwner *owner) {
    return CameraCallbackPayload(fn, owner);
}

typedef class CameraOwner : public GameObject {
public:
    OwnedModel model;
    CameraWork camera;
    CameraWork cameraCopy;
    f32 fade;
    EventHandlerManager callbackState;
    CameraCallbackA348 updateCallback;
    void setUpdate(CameraCallbackA348 callback) { updateCallback = callback; }
    CameraOwner();
    virtual ~CameraOwner();
    virtual void update();
    void fn_1_A6E8();
    void fn_1_A924(EventMessage *);
    void fn_1_A978(float amount);
    void fn_1_A6E4();
} CameraOwner;
#endif
