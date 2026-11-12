#ifndef M624_HANDLE_LIST_OBJECT_H
#define M624_HANDLE_LIST_OBJECT_H
#include "REL/framework/list.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

// Object on m624's handle list, built on ListNode and taking an allocator handle (registryId).
// m624 links handle.cpp and does not link game_object2.cpp.
// The class and file names are descriptive.
struct HandleListObject : ListNode<HandleListObject>, OwnedModel {
    HandleListObject();
    virtual ~HandleListObject();
    virtual void update();
    virtual void draw(Mtx *matrix) {}
    virtual void start() {}
    virtual void finish() {}
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);

    unsigned long registryId;
};
#endif
