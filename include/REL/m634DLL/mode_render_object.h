#ifndef REL_M634DLL_MODE_RENDER_OBJECT_H
#define REL_M634DLL_MODE_RENDER_OBJECT_H
#include "REL/framework/game_object2.h"
struct ModeRenderObject : RenderObject {
    // Initializes the object category.
    explicit ModeRenderObject(int category)
        : RenderObject(category)
    {
    }
    // Releases the object and its member-owned resources.
    virtual ~ModeRenderObject() { }
    // Does nothing.
    virtual void mode(int value) { }
};
#endif
