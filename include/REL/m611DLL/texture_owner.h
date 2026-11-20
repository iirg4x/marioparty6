#ifndef REL_M611DLL_TEXTURE_OWNER_H
#define REL_M611DLL_TEXTURE_OWNER_H

#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include <vector>
extern "C" {
#include "game/hu3d.h"
#include "game/sprite.h"
}

// Owns the scene textures, image storage, and texture transforms.
struct TextureOwner : RequestObject, MinigameSingleton<TextureOwner>, OwnedModel {
    GXTexObj baseTexture;
    GXTexObj imageTexture;
    std::vector<s8> textureStorage;
    void *imageData;
    ANIMDATA *spriteAnimation;
    Mtx textureMatrices[4];
    f32 scrollX;
    f32 scrollY;
    TextureOwner();
    virtual ~TextureOwner();
    virtual void flagEnabled(int);
    virtual void update();
};

void fn_1_12CB4(HU3D_MODEL *model, Mtx *matrix);
void fn_1_12CEC(TextureOwner *owner, GXTexMapID map, u32 matrixId, Mtx inputMatrix);
void fn_1_12E04(TextureOwner *owner, GXTexMapID map, u32 matrixId);
void fn_1_12E50(TextureOwner *owner, s32 textureMap, u32 matrixId);
void fn_1_12F00(TextureOwner *owner, Mtx viewMatrix);
void fn_1_13510(TextureOwner *);

#endif
