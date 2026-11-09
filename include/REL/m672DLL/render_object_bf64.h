#ifndef REL_M672DLL_RENDER_OBJECT_BF64_H
#define REL_M672DLL_RENDER_OBJECT_BF64_H
#include "REL/framework/game_object.h"
#include "REL/m672DLL/model_bf64.h"
// Dispatches a manager-owned object's drawing through its model hook.
struct RenderObject : GameObject, OwnedModel {
    explicit RenderObject(int category);
    virtual ~RenderObject();
    virtual void draw(Mtx *matrix) = 0;
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);
};
#endif
