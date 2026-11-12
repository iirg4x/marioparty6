#ifndef REL_M671_CAMERA_MOTION_H
#define REL_M671_CAMERA_MOTION_H
#include "REL/framework/model.h"
#include "dolphin/types.h"
#include "dolphin/os.h"
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/camera_frame.h"
#include <functional>

struct CameraVector {
    Vec value;
    const Vec &getValue() const;
};
typedef struct CameraWork {
    CameraFrame frame;
    CameraVector cameraPosition;
    CameraVector cameraTarget;
    u32 unknown_03C[3]; /* No code names this field. */
    u32 unknown48;
    s8 enabled;

    void setPosition(const Vec &value) {
        frame.position.x = value.x;
        frame.position.y = value.y;
        frame.position.z = value.z;
    }
    void setTarget(const Vec &value) {
        frame.target.x = value.x;
        frame.target.y = value.y;
        frame.target.z = value.z;
    }
    void setPosition(const CameraVector &target) {
        setPosition(target.getValue());
    }
    void setTarget(const CameraVector &target) {
        setTarget(target.getValue());
    }
} CameraWork;
struct CameraOwner;
typedef void (CameraOwner::*CameraCallback)();
typedef void (CameraOwner::*CameraEvent)(EventMessage *);
typedef struct CameraOwner : GameObject {
    CameraOwner();
    virtual ~CameraOwner();
    virtual void update();
    OwnedModel model;
    CameraWork camera;
    CameraWork cameraCopy;
    f32 fade;
    EventHandlerManager callbackState;
    CameraCallback updateCallback;
    void setUpdate(CameraCallback callback) { updateCallback = callback; }
    void fn_1_BA5C(EventMessage *);
    void fn_1_B81C();
    void fn_1_B820();
} CameraOwner;

#endif
