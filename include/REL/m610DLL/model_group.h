#ifndef REL_M610DLL_MODEL_GROUP_H
#define REL_M610DLL_MODEL_GROUP_H
#include "REL/framework/category_sphere.h"
#include "REL/framework/p_chara.h"
#include "REL/framework/game_object.h"
#include "REL/framework/octtree_man.h"
#include "REL/framework/owned_pointer.h"

/* Owns the result playing piece and its six linked models. */
struct ResultModelOwner : GameObject
{
    Mtx transform;
    RegisteredValue origin;
    std::auto_ptr<OwnedModel> model;
    std::auto_ptr<OwnedModel> models[6];
    OwnedPointer<PlayerConfigSelection> playerConfig;
    ResultModelOwner(OwnedModel *, OwnedModel *, PlayerConfigSelection *);
    virtual ~ResultModelOwner();
    virtual void update();
    void fn_1_C358(const Vec *);
    /* Return the player configuration used by these models. */
    PlayerConfigSelection *playerConfiguration() const
    {
        return playerConfig.get();
    }
};

#endif
