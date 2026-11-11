#ifndef REL_M681DLL_TEXTURE_NODE_H
#define REL_M681DLL_TEXTURE_NODE_H

#include "REL/framework/game_object2.h"
#include "REL/m681DLL/character_sequence.h"

struct PositionVector { Vec value; };
typedef std::vector<signed char> TextureBuffer0;

class TextureNode0 : public RenderObject {
public:
    PositionVector position;
    f32 scale;
    u32 vertexCount;
    TextureBuffer0 textureBuffer;
    GXTexObj textureObject;
    TextureNode0(const PositionVector *position, f32 scale);
    virtual ~TextureNode0();
    virtual void update();
    virtual void draw(Mtx *matrix);
    void fn_1_4E8();
    void fn_1_944();
};

#endif
