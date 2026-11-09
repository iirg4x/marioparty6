#ifndef REL_M681DLL_MODEL_PAIR_H
#define REL_M681DLL_MODEL_PAIR_H

#include "REL/m681DLL/owned_model.h"
#include "REL/framework/minigame_singleton.h"


typedef Model ReleasedModel;

typedef OwnedModel ReleasedModel2E74;

struct ModelPair : MinigameSingleton<ModelPair> {
    ModelPair();
    ReleasedModel2E74 firstModel;
    ReleasedModel2E74 secondModel;
    ~ModelPair();
};


extern "C" {
Vec *fn_1_2FD4(ModelPair *, Vec *);
}

#endif
