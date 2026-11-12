#ifndef REL_M624DLL_SEQUENCE_MANAGER_H
#define REL_M624DLL_SEQUENCE_MANAGER_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/singleton.h"
#include <vector>

struct ActorSequence : MinigameSingleton<ActorSequence> {
    s32 spawnedCount;
    std::vector<Point3d> points;
    std::vector<void *> objects;
    ActorSequence();
    ~ActorSequence();
};

extern "C" void fn_1_2168(ActorSequence *owner);

#endif
