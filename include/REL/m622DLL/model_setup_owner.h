// The two background models and the motion the scene starts and stops with its movement.
#ifndef REL_M622DLL_MODEL_SETUP_OWNER_H
#define REL_M622DLL_MODEL_SETUP_OWNER_H

#include "REL/m622DLL/game_object.h"
#include "REL/framework/minigame_singleton.h"
#include "REL/m622DLL/owned_model.h"

class ModelSetupOwner : public GameObject, public MinigameSingleton<ModelSetupOwner> {
public:
    OwnedModel model;
    OwnedModel hookModel;

    ModelSetupOwner();
    virtual ~ModelSetupOwner();
    virtual void clearFlag(int bit);
    virtual void update();
    void fn_1_658C();
    void fn_1_65F8();
};

#endif
