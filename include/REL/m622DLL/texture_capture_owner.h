// The two screen captures the shop window draws from.
#ifndef REL_M622DLL_TEXTURE_CAPTURE_OWNER_H
#define REL_M622DLL_TEXTURE_CAPTURE_OWNER_H

extern "C" {
#include "dolphin/gx.h"
}
#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"
#include <vector>

struct TextureCaptureOwner : RequestObject, MinigameSingleton<TextureCaptureOwner>, OwnedModel {
    u16 unknownValue;
    GXTexObj textures[2];
    std::vector<signed char> firstBuffer;
    std::vector<signed char> secondBuffer;

    TextureCaptureOwner();
    virtual ~TextureCaptureOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
};

#endif
