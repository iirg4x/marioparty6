#include "REL/framework/frame_time.h"
#include <new>
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
#include "REL/framework/game_object.h"
#include "REL/framework/model.h"

#include "REL/framework/linknode.h"
#include "REL/m672DLL/collision_object.h"
#include "math.h"

#include "REL/m672DLL/member_handler.h"

typedef std::mem_fun1_t<void, Spinner, EventMessage *> SpinnerMemberFunction;
// Initializes motion/output state, positions the output, and registers two event callbacks.
Spinner::Spinner(s32 setupMode, EffectVector position, ANIMDATA *, s32, s32, ANIMDATA *, s32,
                 ANIMDATA *texture, s32 count, s32 layer, s32 itemIndex)
    : OwnedModel(setupMode), GameObject(2), motion(150.0f),
      progress(0.0f), velocity(0.5f), mode(0), angle(0.0f), angleDelta(0.0f),
      amplitude(10.0f), active(0),
      remainingFrames((unsigned int)secondsToFrames(1.6f)),
      duration((unsigned int)secondsToFrames(0.8f)),
      output(new CollidingParticles(texture, count, layer, position)),
      firstCallback(0), secondCallback(1)
{
    motion.setPosition(position);
    motion.active = 1;
    motion.state = 1;
    output->setPosition(fn_1_88D0(itemIndex));
    setShadow();
    setLayer(3);
    setPosition(&motion.position.value);
    registerMemberHandler<Spinner>(&firstCallback,
        bindMember(std::mem_fun(&Spinner::fn_1_8814), this), 3);
    registerMemberHandler<Spinner>(&secondCallback,
        bindMember(std::mem_fun(&Spinner::fn_1_8820), this), 4);
}

// Performs no additional work when the spinner is destroyed.
Spinner::~Spinner() {}

// Advances startup movement or active spin and updates output angles and capped velocity.
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

// Enables the spinner update path.
void Spinner::fn_1_8814(EventMessage *) {
    active = 1;
}

// Starts the associated colliding-particle output.
void Spinner::fn_1_8820(EventMessage *) { fn_1_3350(output.get()); }

// Adds a fixed vector to source and returns the resulting offset.
Vec Spinner::fn_1_885C(Vec *source) {
    Vec offset = {0.0f, 100.0f, 50.0f};
    PSVECAdd(source, &offset, &offset);
    return offset;
}

// Returns the indexed world position from the five-entry position table.
EffectVector Spinner::fn_1_88D0(s32 index)
{
    EffectVector table[5] = {
        {{0.0f, 209.99998f, 80.0f}},
        {{0.0f, 200.0f, 100.0f}},
        {{0.0f, 180.0f, 100.0f}},
        {{-40.0f, 200.0f, 80.0f}},
        {{0.0f, 200.0f, 130.0f}},
    };

    return table[index];
}
