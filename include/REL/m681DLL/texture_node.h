#ifndef REL_M681DLL_TEXTURE_NODE_H
#define REL_M681DLL_TEXTURE_NODE_H

#include "REL/framework/game_object.h"
#include "REL/m681DLL/owned_model.h"
#include "REL/m681DLL/character_sequence.h"

struct RenderObject : GameObject, OwnedModel {
    explicit RenderObject(int category);
    virtual ~RenderObject();
    virtual void draw(Mtx *matrix) = 0;
    static void drawCallback(HU3D_MODEL *model, Mtx *matrix);
};

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
