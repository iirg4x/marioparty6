#include <new>
#include "REL/m672DLL/resources.h"
#include "PowerPC_EABI_Support/Msl/MSL_C/MSL_Common/assert.h"
#include "REL/framework/sound.h"
#include "REL/m672DLL/particle_game_object.h"
#include "REL/m672DLL/scene_effect.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/random.h"
#include "dolphin/types.h"

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "math.h"
#include "game/animdata.h"
}

#include "dolphin/mtx.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/charman.h"
}

#include "humath.h"

#include "game/main.h"

#include "game/object.h"

#include "game/gamework.h"

#include "game/wipe.h"

#include "game/mg/timer.h"

extern "C" {
#include "game/data.h"
}

#include "REL/m672DLL/collision_object.h"

extern "C" {
#include "game/flag.h"
}

extern "C" {
#include "game/sprite.h"
}

extern "C" {
#include "game/audio.h"
}

extern "C" {
#include "game/window.h"
}

#include "REL/m672DLL/hanging_cage.h"
#include "REL/m672DLL/particle_groups.h"

#include "REL/m672DLL/member_handler.h"

// Initializes the model state, position, child callbacks, and idle update callback.
SceneEffect::SceneEffect(int firstMode, Vec *position)
    : GameObject(4), OwnedModel((unsigned int)firstMode), state(0), nextState(0),
      time(0.0f), active(0), child(0)
{
    setLayer(1);
    setPosition(position);
    setAttribute(((1 << 30) | (1 << 1)));
    registerMemberHandler<SceneEffect>(&child,
             bindMember(std::mem_fun(&SceneEffect::fn_1_AF34), this), 1);
    registerMemberHandler<SceneEffect>(&child,
             bindMember(std::mem_fun(&SceneEffect::fn_1_AF78), this), 3);
    setUpdate(&SceneEffect::fn_1_B050);
}

// Performs no additional work when the scene effect is destroyed.
SceneEffect::~SceneEffect() {}

// Invokes the currently selected scene effect update callback, when present.
void SceneEffect::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
}

// Selects the timed scene effect update callback.
void SceneEffect::fn_1_AF34(EventMessage *)
{
    setUpdate(&SceneEffect::fn_1_B054);
}

extern "C" const EffectVector lbl_1_rodata_4A4 = {{0.0f, 0.0f, -2900.0f}};

// Adjusts model motion, plays the associated sound, and selects the particle update callback.
void SceneEffect::fn_1_AF78(EventMessage *)
{
    EffectVector position;

    fn_8002D730(*this, 0.0f, 118.0f);
    position = lbl_1_rodata_4A4;
    soundPlayAt(1, 0x8A8, &position.value);
    setUpdate(&SceneEffect::fn_1_B140);
}

// Empty scene effect update callback used as an idle state.
void SceneEffect::fn_1_B050()
{
}

// Advances elapsed time, changes model attributes after two seconds, and returns to idle.
void SceneEffect::fn_1_B054()
{
    this->time += (1.0 / 60.0);
    if (this->time < 2.0f) {
        return;
    }

    resetAttribute(((1 << 30) | (1 << 1)));
    fn_8002D730(*this, 0.0f, 59.0f);

    setUpdate(&SceneEffect::fn_1_B050);
}
extern "C" {

const s32 lbl_1_rodata_4C8[3] = {40, 53, 58};

// This motion vector has the same components as lbl_1_rodata_4A4 above.
const EffectVector lbl_1_rodata_4D4 = {{0.0f, 0.0f, -2900.0f}};
}

// Advances the state counter, spawns particle effects at thresholds, and stops after three states.
void SceneEffect::fn_1_B140()
{
    EffectVector movement;
    Resources *resourceRoot;
    EffectVector position;

    this->state += 1;
    if (this->state >= lbl_1_rodata_4C8[this->nextState]) {
        if (this->active == 0) {
            movement = lbl_1_rodata_4D4;
            soundPlayAt(1, 0x8A9, &movement.value);
            this->active = 1;
        }

        resourceRoot = &Resources::getChecked();
        position.value.x = 0.0f;
        position.value.y = 0.0f;
        position.value.z = -2900.0f;
        new ParticleGameObject(resourceRoot->animation(0), &position.value);

        this->nextState += 1;
        if (this->nextState >= 3) {
            setUpdate(&SceneEffect::fn_1_B050);
        }
    }
}
// Creates the cage model and rising-particle child, then chooses an allowed starting angle.
HangingCage::HangingCage(s32 type, const EffectVector *position)
    : GameObject(0), position(*position), motion(new OwnedModel(type)), secondModel(0), child(0),
      angle(0.0f), rotation(0.0f), frame(0), fade(0.0f)
{
    f32 angle;

    motion->setLayer(4);
    motion->setPosition(const_cast<Vec *>(&position->value));
    motion->setAttribute(((1 << 30) | (1 << 1)));
    Resources &resources = Resources::getChecked();
    child.reset(new RisingParticles(resources.animation(0), 0x1E, 5, *position));

    for (;;) {
        angle = RandomFloat(0.0f, 359.9f);
        this->angle = angle;
        if ((angle < 45.0f) ||
            ((angle >= 135.0f) && (angle <= 225.0f)) ||
            (angle > 315.0f)) {
            break;
        }
    }
    if ((angle >= 135.0f) && (angle <= 225.0f)) {
        this->rotation = 180.0f;
    }
}

// Performs no additional work when the hanging cage is destroyed.
HangingCage::~HangingCage() {}

// Updates chain rotations/fade and moves the rising-particle child from the combined transforms.
void HangingCage::update()
{
    EffectVector origin;
    Mtx firstMatrix;
    Mtx secondMatrix;
    Mtx translation;
    HSF_OBJECT *firstChain;
    HSF_OBJECT *secondChain;

    ++this->frame;
    this->fade = (float)(1.0 - ((1.0 / 60.0) * (0.027f * (float)this->frame)));
    if (this->fade < 0.0f) {
        this->fade = 0.0f;
    }
    firstChain = motion->getObject("kusari");
    this->angle += 2.0;
    if (this->angle > 360.0f) {
        this->angle -= 360.0f;
    }
    firstChain->mesh.base.rot.z = this->fade * (6.0f * sinf(MTXDegToRad(this->angle)));
    secondChain = motion->getObject("ori_top01");
    if ((this->angle >= 45.0f && this->angle < 135.0f) ||
        (this->angle > 225.0f && this->angle <= 315.0f)) {
        this->rotation += 4.0;
        if (this->rotation > 360.0f) {
            this->rotation -= 360.0f;
        }
        secondChain->mesh.base.rot.z = this->fade * (3.0f * sinf(MTXDegToRad(this->rotation)));
    }
    origin.value.x = 0.0f;
    origin.value.y = 0.0f;
    origin.value.z = 0.0f;
    PSMTXRotRad(firstMatrix, 'z', MTXDegToRad(firstChain->mesh.base.rot.z));
    PSMTXTrans(translation, 0.0f, -1794.0f, 0.0f);
    PSMTXConcat(firstMatrix, translation, firstMatrix);
    PSMTXRotRad(secondMatrix, 'z', MTXDegToRad(secondChain->mesh.base.rot.z));
    PSMTXTrans(translation, 0.0f, -130.0f, 0.0f);
    PSMTXConcat(secondMatrix, translation, secondMatrix);
    PSMTXConcat(firstMatrix, secondMatrix, firstMatrix);
    PSMTXMultVec(firstMatrix, &origin.value, &origin.value);
    PSVECAdd(&origin.value, &this->position.value, &origin.value);
    child->setPosition(origin);
}
