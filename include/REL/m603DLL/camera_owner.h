#ifndef REL_M603_CAMERA_OWNER_H
#define REL_M603_CAMERA_OWNER_H
#include "dolphin/types.h"
#include "REL/framework/list_object.h"
extern "C" {
#include "REL/framework/assertion.h"
}
#include "REL/framework/model.h"
extern "C" {
#include "game/hu3d.h"
}
void operator delete(void *);
void *operator new(unsigned long);

#include "REL/framework/singleton.h"

#include <vector>

struct CameraModelPool : RequestObject, MinigameSingleton<CameraModelPool> {
    std::vector<s16> stateFields;
    OwnedModel modelId;
    CameraModelPool();
    virtual ~CameraModelPool();
    virtual void flagEnabled(int);
    virtual void update();
};

#endif
