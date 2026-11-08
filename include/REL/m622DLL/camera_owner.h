#ifndef M622_CAMERA_OWNER_H
#define M622_CAMERA_OWNER_H
#include "REL/m622DLL/camera_frame.h"
#include "REL/m622DLL/callback_object.h"
#include "REL/m622DLL/game_object.h"
#include "REL/framework/minigame_singleton.h"

class CameraOwner : public GameObject, public MinigameSingleton<CameraOwner> {
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
    virtual void clearFlag(int mode);
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
