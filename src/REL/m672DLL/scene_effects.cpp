#include "REL/m672DLL/resources.h"
#include "REL/framework/sound.h"
#include "REL/m672DLL/particle_game_object.h"
#include "REL/m672DLL/ac4c_owner.h"
#include "REL/framework/event_callback_inline.h"
#include "REL/framework/camera_frame.h"
class Resources;
#include "REL/m672DLL/random.h"
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

#include "REL/m672DLL/position_bf64.h"

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

void *operator new(unsigned long);

void operator delete(void *);

#include "REL/m672DLL/hanging_cage.h"
#include "REL/m672DLL/particle_groups.h"

template<class Receiver> void fn_1_BBD8(EventHandlerManager *, const AC4CPayload &, int);

static inline AC4CMemberFunction AC4COwnerMember(AC4CCallback fn) {
    return AC4CMemberFunction(fn);
}

static inline AC4CPayload BindAC4COwner(const AC4CMemberFunction &fn, AC4COwner *owner) {
    return AC4CPayload(fn, owner);
}

AC4COwner::AC4COwner(int firstMode, const float *position)
    : GameObject(4), OwnedModel((unsigned int)firstMode), state(0), nextState(0),
      time(0.0f), active(0), child(0)
{
    setLayer(1);
    setPosition((Vec *)position);
    setAttribute(((1 << 30) | (1 << 1)));
    fn_1_BBD8<AC4COwner>(&child,
             BindAC4COwner(AC4COwnerMember(&AC4COwner::fn_1_AF34), this), 1);
    fn_1_BBD8<AC4COwner>(&child,
             BindAC4COwner(AC4COwnerMember(&AC4COwner::fn_1_AF78), this), 3);
    setUpdate(&AC4COwner::fn_1_B050);
}

AC4COwner::~AC4COwner() {}

void AC4COwner::update()
{
    if (updateCallback) {
        (this->*updateCallback)();
    }
}

extern "C" {

typedef struct {
    Vec value;
} MotionVector;

void AC4COwner::fn_1_AF34(EventMessage *)
{
    {
        AC4CUpdate value = &AC4COwner::fn_1_B054;
        this->updateCallback = value;
    }
}

}

extern "C" {


const MotionVector lbl_1_rodata_4A4 = {{0.0f, 0.0f, -2900.0f}};

void AC4COwner::fn_1_AF78(EventMessage *)
{
    MotionVector position;
    AC4CUpdate movement;
    HU3D_MODELID motionModelId;
    s16 *modelIdAddress = (s16 *)this;
    s16 *modelIdSlot;
    s16 modelId;

    if (modelIdAddress != 0) {
        modelIdAddress += 8;
    }
    modelIdSlot = modelIdAddress;
    motionModelId = (s16)(*modelIdSlot);
    modelId = motionModelId;
    fn_8002D730(modelId, 0.0f,
                          118.0f);
    position = lbl_1_rodata_4A4;
    soundPlayAt(1, 0x8A8, &position.value);
    movement = &AC4COwner::fn_1_B140;
    this->updateCallback = movement;
}

}

extern "C" {

/* Position and offset vectors separated by an unknown interval. */
typedef struct Position {
    HuVecF position;
    unsigned char unk_0C[12]; /* No code names this field. */
    HuVecF offset;
} Position;

extern s32 lbl_1_data_F18;

extern float lbl_1_bss_1BC;

extern unsigned char lbl_1_bss_218;

extern unsigned char lbl_1_bss_219;

extern s8 lbl_1_data_2758;

s32 fn_1_367CC(s32 *player);

void AC4COwner::fn_1_B050()
{
}

}

extern "C" {

extern void resetAttribute__5ModelFUl(void *, u32);

}

void AC4COwner::fn_1_B054()
{
    s16 *modelIdAddress;
    s16 *modelIdSlot;
    s16 modelId;
    s16 motionModelId;

    this->time += 0.016666666666666666;
    if (this->time < 2.0f) {
        return;
    }

    resetAttribute(((1 << 30) | (1 << 1)));
    modelIdAddress = (s16 *)this;
    if (modelIdAddress != 0) {
        modelIdAddress += 8;
    }
    modelIdSlot = modelIdAddress;
    modelId = (s16)(*modelIdSlot);
    motionModelId = modelId;
    fn_8002D730(motionModelId, 0.0f,
                          59.0f);
    {
        AC4CUpdate callback;
        callback = &AC4COwner::fn_1_B050;
        this->updateCallback = callback;
    }
}
extern "C" {

extern void __msl_assertion_failed(const char *expression, const char *file, int line);

const s32 lbl_1_rodata_4C8[3] = {40, 53, 58};

const MotionVector lbl_1_rodata_4D4 = {{0.0f, 0.0f, -2900.0f}};

}

void AC4COwner::fn_1_B140()
{
    MotionVector movement;
    Resources *resourceRoot;
    MotionVector position;
    AC4CUpdate callback;

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
            callback = &AC4COwner::fn_1_B050;
            this->updateCallback = callback;
        }
    }
}
HangingCage::HangingCage(s32 type, const WorldPosition *position)
    : GameObject(0), position(*position), motion(new OwnedModel(type)), secondModel(0), child(0),
      angle(0.0f), rotation(0.0f), frame(0), fade(0.0f)
{
    f32 angle;

    motion->setLayer(4);
    motion->setPosition((Vec *)position);
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

extern "C" unsigned char lbl_1_bss_218, lbl_1_bss_219;

inline
void OwnedModel::apply_defaults()
{
    if(lbl_1_bss_218) setAttribute((1 << 14));
    if(lbl_1_bss_219) {
        setAttribute(((1 << 30) | (1 << 6)));
        setAttribute(((1 << 30) | (1 << 0)));
    }
}

/* The standalone variant-to-mask helper is retained at fn_1_336F0.
 * Its inline return is visible in the character construction path. */
static inline int model_variant_mask(int variant)
{
    int mask=0;
    switch(variant) {
    case 0: mask=(1 << 0); break;
    case 1: mask=(1 << 1); break;
    case 2: mask=(1 << 2); break;
    case 3: mask=(1 << 3); break;
    }
    return mask;
}

static inline float SinRadians(float radians)
{
    return (float)sin(radians);
}

HangingCage::~HangingCage() {}

void HangingCage::update()
{
    WorldPosition origin;
    Mtx firstMatrix;
    Mtx secondMatrix;
    Mtx translation;
    HSF_OBJECT *firstChain;
    HSF_OBJECT *secondChain;

    ++this->frame;
    this->fade = (float)(1.0 - (0.016666666666666666 * (0.027f * (float)this->frame)));
    if (this->fade < 0.0f) {
        this->fade = 0.0f;
    }
    firstChain = motion->getObject("kusari");
    this->angle += 2.0;
    if (this->angle > 360.0f) {
        this->angle -= 360.0f;
    }
    firstChain->mesh.base.rot.z = this->fade * (6.0f * SinRadians(MTXDegToRad(this->angle)));
    secondChain = motion->getObject("ori_top01");
    if ((this->angle >= 45.0f && this->angle < 135.0f) ||
        (this->angle > 225.0f && this->angle <= 315.0f)) {
        this->rotation += 4.0;
        if (this->rotation > 360.0f) {
            this->rotation -= 360.0f;
        }
        secondChain->mesh.base.rot.z = this->fade * (3.0f * SinRadians(MTXDegToRad(this->rotation)));
    }
    origin.value.value.x = 0.0f;
    origin.value.value.y = 0.0f;
    origin.value.value.z = 0.0f;
    PSMTXRotRad(firstMatrix, 'z', MTXDegToRad(firstChain->mesh.base.rot.z));
    PSMTXTrans(translation, 0.0f, -1794.0f, 0.0f);
    PSMTXConcat(firstMatrix, translation, firstMatrix);
    PSMTXRotRad(secondMatrix, 'z', MTXDegToRad(secondChain->mesh.base.rot.z));
    PSMTXTrans(translation, 0.0f, -130.0f, 0.0f);
    PSMTXConcat(secondMatrix, translation, secondMatrix);
    PSMTXConcat(firstMatrix, secondMatrix, firstMatrix);
    PSMTXMultVec(firstMatrix, &origin.value.value, &origin.value.value);
    PSVECAdd(&origin.value.value, &this->position.value.value, &origin.value.value);
    child->setPosition(origin);
}

template<class Receiver>
void fn_1_BBD8(EventHandlerManager *owner, const AC4CPayload &payload, int key) {
    owner->addHandler(new MemberCallbackRecord<Receiver>(payload), key);
}
