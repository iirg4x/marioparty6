#ifndef REL_M603_CAMERA_OWNER_H
#define REL_M603_CAMERA_OWNER_H
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}

#include "REL/framework/singleton.h"

#include <vector>
#include <new>

/* Own the shared UFO model and update the UFO list. */
struct CameraModelPool : RequestObject, MinigameSingleton<CameraModelPool> {
    /* Constructed and destroyed here; no module code accesses its elements. */
    std::vector<s16> stateFields;
    OwnedModel modelId;
    CameraModelPool();
    virtual ~CameraModelPool();
    virtual void flagEnabled(int);
    virtual void update();
};

#endif
