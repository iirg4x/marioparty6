#ifndef REL_M653DLL_CAR_H
#define REL_M653DLL_CAR_H

#include "REL/m653DLL/resource_handles.h"
#include "REL/framework/game_object.h"
#include "REL/framework/p_info.h"
#include "REL/m653DLL/motion_work.h"
#include "REL/framework/event_hm.h"

class ExhaustSmoke;
#include "REL/m653DLL/steering_work.h"

struct ModelHolder {
    OwnedModel *value;
    ModelHolder(OwnedModel *initial = 0) : value(initial) {}
    ~ModelHolder() { delete value; }
    OwnedModel *get() const { return value; }
    void copy(const OwnedModel *source) {
        reset(new OwnedModel(*source));
    }
    void reset(OwnedModel *replacement)
    {
        if (value != replacement) {
            delete value;
            value = replacement;
        }
    }
};

/* A player's car: its body, six wheels and the character riding it (models), its drive from the
 * two sticks (rateX, rateY turning the wheels, wrappedX, wrappedY), its motion work, steering
 * and exhaust smoke, and the event callbacks it registers. The class name is descriptive: the
 * game keeps none (its old name was its constructor's address). */
struct Car : GameObject {
    virtual ~Car();
    virtual void update();
    Car(OwnedModel *primary, OwnedModel *resource, void *asset, PlayerInfo *setting, ExhaustSmoke *smoke);
    void fn_1_17EC(EventMessage*);
    void fn_1_183C(float, float);
    void fn_1_1904();
    void fn_1_17F8(EventMessage*);
    unsigned char flag10;
    unsigned char flag11;
    unsigned char unknown12[2]; /* No code names this field. */
    Mtx baseMatrix;
    float rateX;
    float rateY;
    float wrappedX;
    float wrappedY;
    RegisteredValue points[6];
    ModelHolder primary;
    ModelHolder models[6];
    std::auto_ptr<void> asset;
    std::auto_ptr<PlayerInfo> setting;
    ModelHolder extra;
    std::auto_ptr<M653MotionWork> motion;
    EventHandlerManager callbacks;
    std::auto_ptr<M653SteeringWork> work;
    std::auto_ptr<ExhaustSmoke> smoke;
};

#include "REL/m653DLL/object_calls.h"

#endif
