#ifndef REL_M681DLL_MODEL_PAIR_H
#define REL_M681DLL_MODEL_PAIR_H

#include "REL/framework/model.h"
#include "REL/framework/singleton.h"

struct ModelPair : MinigameSingleton<ModelPair> {
    ModelPair();
    OwnedModel firstModel;
    OwnedModel secondModel;
    ~ModelPair();
};

extern "C" {
Vec *fn_1_2FD4(ModelPair *, Vec *);
}

#endif
