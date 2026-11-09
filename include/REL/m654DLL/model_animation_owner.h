#ifndef M654_MODEL_ANIMATION_OWNER_H
#define M654_MODEL_ANIMATION_OWNER_H
// The map scene: its models, its lights and the door opening at the start of a round.
#include <vector>
#include "REL/m654DLL/manager_event_base.h"
#include "REL/m654DLL/registered_value.h"
#include "REL/m654DLL/owned_model.h"
#include "REL/framework/minigame_singleton.h"

class ModelAnimationOwner : public ManagerEventBase, public MinigameSingleton<ModelAnimationOwner> {
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
    virtual void handleEvent(int event);
    virtual void advanceFrame();
    void fn_1_20A0();
    void fn_1_246C();
    void fn_1_254C();
    void fn_1_2218();
    void fn_1_226C();
};

#endif
