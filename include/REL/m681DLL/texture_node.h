#ifndef REL_M681DLL_TEXTURE_NODE_H
#define REL_M681DLL_TEXTURE_NODE_H

#include "REL/framework/game_object2.h"
#include <vector>

class FloorCircle : public RenderObject {
public:
    Vec position;
    f32 scale;
    u32 vertexCount;
    std::vector<signed char> textureBuffer;
    GXTexObj textureObject;
    FloorCircle(const Vec *position, f32 scale);
    virtual ~FloorCircle();
    virtual void update();
    virtual void draw(Mtx *matrix);
    void fn_1_4E8();
    void fn_1_944();
};

#endif
