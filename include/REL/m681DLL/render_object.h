#ifndef REL_M681DLL_RENDER_OBJECT_H
#define REL_M681DLL_RENDER_OBJECT_H

#include "REL/framework/game_object.h"
#include "REL/m681DLL/owned_model.h"

struct RenderObject : GameObject, OwnedModel {
    explicit RenderObject(int category);
    virtual ~RenderObject();
    virtual void draw(Mtx *matrix) = 0;
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);
};

#endif
