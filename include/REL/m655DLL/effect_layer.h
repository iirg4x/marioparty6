#ifndef M655DLL_EFFECT_LAYER_H
#define M655DLL_EFFECT_LAYER_H
#include "REL/m655DLL/goal.h"
#include "REL/framework/timer.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/game_object_man.h"
#include "REL/m655DLL/effect_model.h"
struct EffectLayer {
    EventManager eventManager;
    GameObjectManager layer;
    TimerOwner state;
    Goal goal;
    EffectModel model;
    unsigned char active : 1;
    unsigned char flags6_0 : 7;
    EffectLayer() : state(0), active(0) {}
};
#endif
