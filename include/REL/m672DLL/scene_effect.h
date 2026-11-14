#ifndef REL_M672DLL_SCENE_EFFECT_H
#define REL_M672DLL_SCENE_EFFECT_H

#include "REL/framework/game_object.h"
#include "REL/framework/model.h"

#include "REL/framework/event_hm.h"

struct SceneEffect;

typedef void (SceneEffect::*SceneEffectHandler)(EventMessage *);

#include "REL/m672DLL/member_callback.h"
typedef std::mem_fun1_t<void, SceneEffect, EventMessage *> SceneEffectMemberFunction;
typedef CallbackBinding<SceneEffect> SceneEffectBinding;

typedef void (SceneEffect::*SceneEffectUpdate)();
struct SceneEffect : GameObject, OwnedModel {
    int state;
    int nextState;
    float time;
    unsigned char active;
    EventHandlerManager child;
    SceneEffectUpdate updateCallback;
    // Select the update callback.
    void setUpdate(SceneEffectUpdate callback) { updateCallback = callback; }
    void fn_1_AF34(EventMessage *);
    void fn_1_AF78(EventMessage *);
    void fn_1_B050();
    void fn_1_B054();
    void fn_1_B140();
    SceneEffect(int firstMode, Vec *position);
    virtual ~SceneEffect();
    virtual void update();
};

#endif
