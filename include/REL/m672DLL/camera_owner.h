#ifndef REL_M672DLL_CAMERA_OWNER_H
#define REL_M672DLL_CAMERA_OWNER_H
#include "dolphin/os.h"
#include "dolphin/mtx.h"
#include "REL/m672DLL/collision_object.h"
#include "REL/framework/event.h"
#include "REL/framework/game_object.h"
#include "REL/framework/event_hm.h"
#include "REL/framework/model.h"
#include "REL/framework/camera_work.h"

class CameraOwner;
typedef void (CameraOwner::*CameraUpdate)();
#include "REL/m672DLL/member_callback.h"
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
    void fn_1_A924(const EventPacket &);
    void fn_1_A978(float amount);
    void fn_1_A6E4();
} CameraOwner;
#endif
