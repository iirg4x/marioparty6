#ifndef REL_M624DLL_SEQUENCE_MANAGER_H
#define REL_M624DLL_SEQUENCE_MANAGER_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/singleton.h"
#include <vector>

struct ActorSequence : MinigameSingleton<ActorSequence> {
    s32 spawnedCount;
    std::vector<Point3d> points;
    std::vector<unsigned long> objects;
    ActorSequence();
    ~ActorSequence();
};

extern "C" void fn_1_2168(ActorSequence *owner);

class SequenceActor;
Vec *fn_1_22F0(SequenceActor *owner, Vec *position);
std::vector<unsigned long> *fn_1_2264(ActorSequence *manager);

#endif
