#ifndef REL_M603_CAMERA_OWNER_H
#define REL_M603_CAMERA_OWNER_H
#include "dolphin/types.h"
#include "REL/m603DLL/callback_owner.h"
extern "C" {
#include "REL/framework/assertion.h"
}
#define M603_OWNED_MODEL_DECLARATION
#include "REL/m603DLL/model.h"
void operator delete(void *);
void *operator new(unsigned long);

#include "REL/framework/minigame_singleton.h"

#include <vector>

struct CameraModelPool : CallbackOwner, MinigameSingleton<CameraModelPool> {
    std::vector<s16> stateFields;
    OwnedModel modelId;
    CameraModelPool();
    virtual ~CameraModelPool();
    virtual void update(unsigned long);
    virtual void extra();
};

#endif
