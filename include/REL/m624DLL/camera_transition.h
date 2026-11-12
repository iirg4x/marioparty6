#ifndef M624_CAMERA_TRANSITION_H
#define M624_CAMERA_TRANSITION_H
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/debug.h"

// The game camera: it moves between three poses, and shakes for a while after a hit.
class GameCamera : public RequestObject, public MinigameSingleton<GameCamera> {
public:
    typedef void (GameCamera::*Callback)();

    GameCamera();
    virtual ~GameCamera();
    virtual void flagEnabled(int bit);
    virtual void update();
    void fn_1_4228();
    void fn_1_43C0();
    void fn_1_4634();
    void fn_1_47F0();
    void fn_1_47FC();
    void run() { (this->*callback)(); }

    CameraFrame camera;
    Callback callback;
    s16 state;
    u16 stateData;
    s32 frameCount;
    s32 frameLimit;
    CameraSetup setupCamera;
};

void fn_1_45E8(GameCamera *);
void fn_1_40F4(GameCamera *, f32);
#endif
