// The four player cameras: each follows its player up (or down) the course, and the opening widens them from one
// view to four. The class name is descriptive.
#ifndef REL_M604DLL_CAMERA_OWNER_H
#define REL_M604DLL_CAMERA_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/camera_frame.h"

// One camera: the frame it shows and the frame it moves towards.
struct CameraView : CameraFrame {
    Vec desiredPosition;
    Vec desiredTarget;
    f32 unk3C[4]; /* No code names this field. */
    u8 active;
};

struct CameraOwner : RequestObject, MinigameSingleton<CameraOwner> {
    typedef void (CameraOwner::*Callback)();

    CameraOwner();
    virtual ~CameraOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
    void fn_1_7CE0();
    void fn_1_7D6C();
    void fn_1_81B4();
    void fn_1_84B4();
    Vec fn_1_878C(s32 index) const;

    CameraView cameras[4];
    f32 elapsed;
    u8 mode; // 1 until the round starts: the cameras do not follow the players yet.
    s32 state;
    Callback callback;
    u8 unk168[4]; /* No code names this field. */
};

void fn_1_8730(CameraOwner *owner, s32 index, const Vec *target, const Vec *position);
void fn_1_87B4(CameraOwner *owner, s32 index, f32 divisor);
f32 fn_1_8894(CameraOwner *owner, s32 index);

#endif
