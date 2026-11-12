// The Hu3D model id of an object that owns a model, for the calls that take the id (fn_800267CC's
// position, fn_8002741C's hook). player.cpp and model_pair_owner.cpp both use it.
#ifndef REL_M639DLL_MODEL_ID_H
#define REL_M639DLL_MODEL_ID_H

#include "REL/framework/model.h"

static inline short modelIdOf(Model *model)
{
    return *model;
}

#endif
