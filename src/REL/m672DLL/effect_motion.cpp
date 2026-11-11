#include <vector>
#include "REL/m672DLL/particle_groups.h"
#include "REL/m672DLL/member_callback.h"
#include "REL/m672DLL/spinner.h"
#include "dolphin/mtx.h"
extern "C" {
#include <stdio.h>
#include <stdlib.h>
}
#include "REL/framework/event_hm.h"
#include "REL/framework/event_callback_inline.h"
#include "humath.h"
void *operator new(unsigned long);
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"
void operator delete(void *);

#include "REL/framework/linknode.h"
#include "REL/m672DLL/collision_object.h"
#include "math.h"
void operator delete(void *);

template<class Receiver>
void fn_1_8980(EventHandlerManager *, const CallbackBinding<Receiver> &, int);

typedef std::mem_fun1_t<void, Spinner, EventMessage *> SpinnerMemberFunction;
static inline SpinnerMemberFunction SpinnerMember(void (Spinner::*fn)(EventMessage *)) {
    return SpinnerMemberFunction(fn);
}
static inline CallbackBinding<Spinner> BindSpinner(const SpinnerMemberFunction &fn,
                                               Spinner *object) {
    return CallbackBinding<Spinner>(fn, object);
}
extern "C" {
}
template<class Time> Time secondsToFrames(Time);
Spinner::Spinner(s32 setupMode, WorldPosition position, ANIMDATA *, s32, s32, ANIMDATA *, s32,
                 ANIMDATA *texture, s32 count, s32 layer, s32 itemIndex)
    : OwnedModel(setupMode), GameObject(2), motion(150.0f),
      progress(0.0f), velocity(0.5f), mode(0), angle(0.0f), angleDelta(0.0f),
      amplitude(10.0f), active(0),
      remainingFrames((unsigned int)secondsToFrames(1.6f)),
      duration((unsigned int)secondsToFrames(0.8f)),
      output(new CollidingParticles(texture, count, layer, position)),
      firstCallback(0), secondCallback(1)
{
    motion.setPosition(position.value);
    motion.active = 1;
    motion.state = 1;
    output->setPosition(fn_1_88D0(itemIndex));
    setShadow();
    setLayer(3);
    setPosition(&motion.position.value);
    fn_1_8980<Spinner>(&firstCallback,
        BindSpinner(SpinnerMember(&Spinner::fn_1_8814), this), 3);
    fn_1_8980<Spinner>(&secondCallback,
        BindSpinner(SpinnerMember(&Spinner::fn_1_8820), this), 4);
}

Spinner::~Spinner() {}

void Spinner::update()
{
    float direction;
    float progressDelta;
    float previousProgress;
    float nextProgress;
    CollidingParticles *startOutput;
    CollidingParticles *endOutput;

    if (this->active == 0) {
        return;
    }
    if (this->remainingFrames != 0) {
        EffectVector position = this->motion.position;
        if (this->remainingFrames < this->duration) {
            this->angleDelta = (float)(this->amplitude *
                sin((M_PI * (double)this->angle) / 180.0));
            position.value.x += this->angleDelta;
            this->angle += 60.0f;
            this->amplitude *= 0.9f;
        }
        this->remainingFrames -= 1;
        if (this->remainingFrames == 0) {
            setPosition(&this->motion.position.value);
        } else {
            setPosition(&position.value);
        }
        return;
    }
    if (0.0f != this->motion.position.value.x) {
    progressDelta = (float)((double)this->velocity / 0.038333332538604735);
    direction = 1.0f;
    setRotationY(this->progress);
    previousProgress = this->progress;
    startOutput = output.get();
    startOutput->angle = previousProgress;
    if (((this->motion.position.value.x > 0.0f) &&
         (this->motion.position.value.z > -1500.0f)) ||
        ((this->motion.position.value.z < -1500.0f) &&
         (this->motion.position.value.x < 0.0f))) {
        direction *= -1.0f;
    }
    this->progress = this->progress + (this->velocity * direction);
    nextProgress = this->progress + (progressDelta * direction);
    endOutput = output.get();
    endOutput->emitAngle = nextProgress;
    if (this->velocity < 3.0f) {
        this->velocity += 0.08333333333333333;
        if (this->velocity > 3.0f) {
            this->velocity = 3.0f;
        }
    }
    }
}

void Spinner::fn_1_8814(EventMessage *) {
    active = 1;
}

extern "C" void fn_1_3350(CollidingParticles *);
void Spinner::fn_1_8820(EventMessage *) { fn_1_3350(output.get()); }

Vec Spinner::fn_1_885C(Vec *source) {
    Vec offset = {0.0f, 100.0f, 50.0f};
    PSVECAdd(source, &offset, &offset);
    return offset;
}

WorldPosition Spinner::fn_1_88D0(s32 index)
{
    WorldPosition table[5] = {
        {{{0.0f, 209.99998f, 80.0f}}},
        {{{0.0f, 200.0f, 100.0f}}},
        {{{0.0f, 180.0f, 100.0f}}},
        {{{-40.0f, 200.0f, 80.0f}}},
        {{{0.0f, 200.0f, 130.0f}}},
    };

    return table[index];
}

template<class Receiver>
void fn_1_8980(EventHandlerManager *owner, const CallbackBinding<Receiver> &payload, int key)
{
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}
