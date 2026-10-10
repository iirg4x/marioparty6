// Minigame objects that dispatch their drawing through a Hu3D model hook.
#ifndef REL_FRAMEWORK_GAME_OBJECT2_H
#define REL_FRAMEWORK_GAME_OBJECT2_H

#include "REL/framework/game_object.h"
#include "REL/framework/model.h"

// A manager-registered game object whose model hook calls its drawing method.
struct RenderObject : GameObject, OwnedModel {
    explicit RenderObject(int category);
    virtual ~RenderObject();
    virtual void draw(Mtx *matrix) = 0;
    virtual void drawPass() = 0;
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);
};

#endif
