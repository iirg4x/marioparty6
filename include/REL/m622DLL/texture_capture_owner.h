// The two screen captures the shop window draws from.
#ifndef REL_M622DLL_TEXTURE_CAPTURE_OWNER_H
#define REL_M622DLL_TEXTURE_CAPTURE_OWNER_H

extern "C" {
#include "dolphin/gx.h"
}
#include "REL/m622DLL/game_object.h"
#include "REL/framework/minigame_singleton.h"
#include "REL/m622DLL/owned_model.h"
#include <vector>

struct TextureCaptureOwner : GameObject, MinigameSingleton<TextureCaptureOwner>, OwnedModel {
    u16 unknownValue;
    GXTexObj textures[2];
    std::vector<signed char> firstBuffer;
    std::vector<signed char> secondBuffer;

    TextureCaptureOwner();
    virtual ~TextureCaptureOwner();
    virtual void clearFlag(int bit);
    virtual void update();
};

#endif
