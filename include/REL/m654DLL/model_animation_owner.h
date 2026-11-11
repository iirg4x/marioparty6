#ifndef M654_MODEL_ANIMATION_OWNER_H
#define M654_MODEL_ANIMATION_OWNER_H
// The map scene: its models, its lights and the door opening at the start of a round.
#include <vector>
#include "REL/framework/list_object.h"
#include "REL/framework/category_sphere.h"
#include "REL/framework/model.h"
#include "REL/framework/singleton.h"

class ModelAnimationOwner : public RequestObject, public MinigameSingleton<ModelAnimationOwner> {
public:
    void (ModelAnimationOwner::*callback)();
    OwnedModel animation;
    OwnedModel secondAnimation;
    OwnedModel secondaryAnimation0;
    OwnedModel secondaryAnimation1;
    float angle;
    OwnedModel additionalAnimation;
    std::vector<RegisteredValue> values;
    int elapsed;

    ModelAnimationOwner();
    virtual ~ModelAnimationOwner();
    virtual void flagEnabled(int event);
    virtual void update();
    void fn_1_20A0();
    void fn_1_246C();
    void fn_1_254C();
    void fn_1_2218();
    void fn_1_226C();
};

#endif
