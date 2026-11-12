#ifndef M639_CAMERA_OWNER_H
#define M639_CAMERA_OWNER_H
// The board camera.
#include "REL/framework/singleton.h"
#include "REL/framework/debug.h"
#include "REL/framework/game_object.h"
#include "REL/framework/list_object.h"

class CameraOwner : public RequestObject, public MinigameSingleton<CameraOwner> {
public:
    CameraFrame view;
    void (CameraOwner::*callback)();
    short state;
    int frame;
    int endPoseIndex;
    CameraSetup debugCamera;
    CameraOwner();
    virtual ~CameraOwner();
    virtual void flagEnabled(int mode);
    virtual void update();
    void fn_1_33BC();
    void applyView();
    void fn_1_3584();
    void fn_1_3690();
    void fn_1_3860();
    void fn_1_386C();
};

#endif
