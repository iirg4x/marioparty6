#ifndef M622_CAMERA_OWNER_H
#define M622_CAMERA_OWNER_H
#include "REL/framework/debug.h"
#include "REL/m622DLL/callback_object.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"

class CameraOwner : public RequestObject, public MinigameSingleton<CameraOwner> {
public:
    typedef void (CameraOwner::*Callback)();
    CameraFrame frame;
    s32 frameCount;
    Callback callback;
    CallbackObject *target;
    s16 motionModelId;
    CameraSetup setup;

    CameraOwner();
    virtual ~CameraOwner();
    virtual void flagEnabled(int mode);
    virtual void update();

    void fn_1_58DC();
    void fn_1_5974();
    void fn_1_59A4();
    void fn_1_59D4();
    void fn_1_5AE8(CallbackObject *target);
    void fn_1_5D38();
};
void fn_1_5CD0(CameraOwner *owner);
#endif
