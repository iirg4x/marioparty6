#ifndef REL_M634DLL_OBJECT_WORK_H
#define REL_M634DLL_OBJECT_WORK_H
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
struct ObjectWork : GameObject, OwnedModel {
    s32 frame;
    typedef void (ObjectWork::*Callback)();
    Callback callback;
    explicit ObjectWork(s32 resource);
    virtual ~ObjectWork();
    virtual void update();
    void fn_1_11618();
    void fn_1_1161C();
};
#endif
