// The two background models and the motion the scene starts and stops with its movement.
#ifndef REL_M622DLL_MODEL_SETUP_OWNER_H
#define REL_M622DLL_MODEL_SETUP_OWNER_H

#include "REL/framework/list_object.h"
#include "REL/framework/singleton.h"
#include "REL/framework/model.h"

class ModelSetupOwner : public RequestObject, public MinigameSingleton<ModelSetupOwner> {
public:
    OwnedModel model;
    OwnedModel hookModel;

    ModelSetupOwner();
    virtual ~ModelSetupOwner();
    virtual void flagEnabled(int bit);
    virtual void update();
    void fn_1_658C();
    void fn_1_65F8();
};

#endif
