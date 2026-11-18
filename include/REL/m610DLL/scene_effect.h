#ifndef M610_SCENE_EFFECT_H
#define M610_SCENE_EFFECT_H
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
#include "REL/framework/octtree_man.h"
/* Owns an animated day or night scenery model. */
struct SceneEffect : GameObject
{
    Vec position;
    std::auto_ptr<OwnedModel> animation;
    short motion;
    explicit SceneEffect(const OwnedModel *);
    virtual ~SceneEffect();
    virtual void update();
    /* Return the animated scenery model. */
    Model *get()
    {
        return animation.get();
    }
};
#endif
