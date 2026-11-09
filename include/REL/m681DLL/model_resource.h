#ifndef REL_M681DLL_MODEL_RESOURCE_H
#define REL_M681DLL_MODEL_RESOURCE_H

#include "REL/m681DLL/owned_model.h"

struct ModelResource : OwnedModel {
    char *nodeName;
    ModelResource(s32 player);
    ~ModelResource();
};


#endif
