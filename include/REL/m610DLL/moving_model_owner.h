#ifndef REL_M610DLL_MOVING_MODEL_OWNER_H
#define REL_M610DLL_MOVING_MODEL_OWNER_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/octtree_man.h"
#include "REL/m610DLL/collision_node.h"
#include "game/animdata.h"
struct GroundSprite;

/* Animates a falling obstacle, its collision sphere, shadow and light. */
struct MovingModelOwner : GameObject
{
    /* Return the registered obstacle sphere. */
    const Sphere &collisionSphere() const
    {
        return registration;
    }
    typedef void (MovingModelOwner::*Action)();
    Action action;
    CollisionNode registration;
    Vec position;
    Vec rotation;
    Vec velocity;
    std::auto_ptr<OwnedModel> model;
    std::auto_ptr<GroundSprite> drawing;
    s16 light;
    f32 opacity;
    f32 elapsedTime;
    f32 delay;
    /* Return the falling obstacle model. */
    Model *getModel() const
    {
        return model.get();
    }
    MovingModelOwner(const OwnedModel *, ANIMDATA *, const Vec *);
    virtual ~MovingModelOwner();
    virtual void update();
    void fn_1_CDC0();
    void fn_1_CE98();
    void fn_1_CFB4();
    void fn_1_D110();
    void fn_1_D1D4();
    void fn_1_D2A4();
};
#endif
