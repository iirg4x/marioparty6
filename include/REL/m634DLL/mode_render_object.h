#ifndef REL_M634DLL_MODE_RENDER_OBJECT_H
#define REL_M634DLL_MODE_RENDER_OBJECT_H
#include "REL/framework/game_object2.h"
struct ModeRenderObject : RenderObject {
 explicit ModeRenderObject(int category):RenderObject(category) {}
 virtual ~ModeRenderObject() {}
 virtual void mode(int value) {}
};
#endif
