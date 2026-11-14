#ifndef REL_M681DLL_MODEL_RESOURCE_H
#define REL_M681DLL_MODEL_RESOURCE_H

#include "REL/framework/model.h"

struct ModelResource : OwnedModel {
    char *nodeName;
    ModelResource(s32 player);
    ~ModelResource();
};

void fn_1_5168(Model *object);

extern "C" f32 (*fn_1_524C(ModelResource *, Mtx))[4];
#endif
