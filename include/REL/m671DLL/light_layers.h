#ifndef REL_M671DLL_LIGHT_LAYERS_H
#define REL_M671DLL_LIGHT_LAYERS_H
#include "REL/framework/game_object2.h"
struct LightLayer0 : RenderObject {
    LightLayer0();
    virtual ~LightLayer0();
    virtual void update();
    virtual void draw(Mtx *);
};
struct LightLayer4 : RenderObject {
    LightLayer4();
    virtual ~LightLayer4();
    virtual void update();
    virtual void draw(Mtx *);
};
#endif
