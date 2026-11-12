// A player's car: its models and six wheels, its drive from the two sticks, its engine sound and
// exhaust smoke, and the event callbacks it registers. The file name is descriptive: the game's
// assert texts name only dossun.cpp and player_table_1vs1.cpp in this module. This file links
// before dossun.cpp, and the game's files link in name order, so its name sorts before that one.
#include "dolphin/mtx.h"

#include "PowerPC_EABI_Support/Runtime/MWCPlusLib.h"

#include "dolphin/types.h"

#include "REL/m653DLL/exhaust_smoke.h"

#include "REL/framework/event_hm.h"

#include "REL/m653DLL/allocation.h"

#include "REL/m653DLL/random.h"

#include "REL/framework/model.h"

#include "REL/m653DLL/resource_handles.h"

#include "datadir_enum.h"

#include "game/hu3d.h"

#include "REL/m653DLL/car.h"

#include "REL/framework/p_chara.h"

#include "REL/m653DLL/boundary_plane.h"

#include "REL/framework/game_object_man.h"

#include "REL/m653DLL/motion_work.h"

#include "REL/m653DLL/effect_matrix.h"

#include "REL/m653DLL/model_matrix.h"

#include "REL/m653DLL/model_image.h"

#include "REL/framework/event_manager.h"

#include "REL/framework/category_sphere.h"

#include "REL/m653DLL.h"

#include "REL/framework/sound.h"

#include "REL/m653DLL/callback_payload.h"

#include "REL/framework/event_callback_inline.h"
extern "C" {
#include <math.h>
}

#include <functional>

void fn_1_1000();

void fn_1_1004();

extern "C" { void fn_1_2558(Car *); }

extern const s32 lbl_1_rodata_48[0xE] = {
    0x240, 0x228, 0x2B8, 0x318, 0x2E8, 0x1E0, 0x300,
    0x1F8, 0x2D0, 0x288, 0x210, 0x2A0, 0x270, 0x258
};

void fn_1_6C0(s32 soundIndex)
{
    soundPlay(lbl_1_rodata_48[soundIndex]);
}

static inline void createCharacterModel(Car *object)
{
    object->extra.reset(new OwnedReleasedModel(object->workSetting()->characterNo() + DATANUM(0x700000, 18)));
}

Car::Car(OwnedReleasedModel *primary,
                               OwnedReleasedModel *resource, void *asset,
                               PlayerInfo *setting, ExhaustSmoke *model)
    : GameObject(0), primary(primary), asset(asset), setting(setting),
      motion(new M653MotionWork(asset, setting)), callbacks(2), model(model)
{
    this->models[0].reset(resource);
    this->models[1].copy(resource);
    this->models[2].copy(resource);
    this->models[3].copy(resource);
    this->models[4].copy(resource);
    this->models[5].copy(resource);
    for (int index = 0; index < 6; ++index)
    {
        this->models[index].get()->setShadow();
        this->models[index].get()->setLayer(2);
    }
    fn_1_1CA0(&this->callbacks,
        std::bind1st(
            std::mem_fun(&Car::fn_1_17EC), this), 2);
    fn_1_1CA0(&this->callbacks,
        std::bind1st(
            std::mem_fun(&Car::fn_1_17F8), this), 3);
    this->flag10 = 0;
    createCharacterModel(this);
    this->primary.get()->setLayer(2);
    this->extra.get()->setLayer(2);
    this->wrappedX = 0.0f;
    this->wrappedY = 0.0f;
    this->fn_1_183C(0.0f, 0.0f);
    this->flag11 = 0;
}

void fn_1_FC4() {
    setPreUpdateHook(0,fn_1_1000);
    setPostUpdateHook(0,fn_1_1004);
}

void fn_1_1000() {}

void fn_1_1004() {}

void fn_1_1008(Car *source,Vec *position) {
    position->x=source->baseMatrix[0][3];
    position->y=source->baseMatrix[1][3];
    position->z=source->baseMatrix[2][3];
}

void fn_1_1024(const Car *source,Vec *direction) {
    direction->x=source->baseMatrix[0][2];
    direction->y=source->baseMatrix[1][2];
    direction->z=source->baseMatrix[2][2];
}

void fn_1_1040(const Car *source,Vec *axis) {
    axis->x=-source->baseMatrix[0][0];
    axis->y=-source->baseMatrix[1][0];
    axis->z=-source->baseMatrix[2][0];
}

struct OwnedModel;

extern "C" void fn_1_4320(void *);

void fn_1_1068(Car *self) { fn_1_4320(self->motionWork()); }

unsigned char fn_1_10A4(Car *self)
{
    M653MotionWork *child = self->motion.value;
    return child->active_98;
}

extern "C" void fn_1_3EC4(M653MotionWork *, const Vec *);

void fn_1_10C0(Car *self, const Vec *position)
{
    M653MotionWork *work = self->motion.value;
    fn_1_3EC4(work, position);
}

extern "C" void fn_1_4300(M653MotionWork *, f32);

extern "C" void fn_1_4310(M653MotionWork *, f32);

extern "C" void fn_1_3EE0(M653MotionWork *);

extern "C" void fn_1_44E4(M653MotionWork *, f32);

extern "C" void fn_1_4558(M653MotionWork *);

void fn_1_1498(Car *);

extern u8 lbl_1_bss_64;

void Car::update()
{
    Vec modelPosition;
    Vec direction;
    f32 scale;
    f32 xInput;
    f32 zInput;
    f32 xMagnitude;
    f32 zMagnitude;
    f64 modelPositionXValue;
    f64 modelPositionYValue;
    f64 modelPositionZValue;

    if (this->flag11 == 0) {
        M653MotionWork *initialWork = this->motion.value;
        if (initialWork->active_98 != 0) {
            this->flag11 = 1;
            s32 mode;
            PlayerInfo *playerIndex = this->setting.value;
            mode = playerIndex->characterNo();
            fn_1_6C0(mode);
        }
    }
    if (lbl_1_bss_64 != 0) {
        this->fn_1_1904();
    }

    M653MotionWork *movementWork = this->motionWork();
    fn_1_4300(movementWork, 6.0f * this->rateX);
    fn_1_4310(movementWork, 6.0f * this->rateY);
    fn_1_3EE0(movementWork);
    fn_1_1498(this);
    fn_1_1508(this);

    modelPositionZValue = -130.0;
    modelPositionYValue = 170.0;
    modelPositionXValue = 45.0;
    modelPosition.x = (f32)modelPositionXValue;
    modelPosition.y = (f32)modelPositionYValue;
    modelPosition.z = (f32)modelPositionZValue;
    PSMTXMultVec(this->baseMatrix,
                 &modelPosition, &modelPosition);
    this->particleEffect()->fn_1_B2E8(&modelPosition);

    xInput = this->rateX;
    if (xInput < 0.0f) xMagnitude = -xInput;
    else xMagnitude = xInput;
    zInput = this->rateY;
    if (zInput < 0.0f) zMagnitude = -zInput;
    else zMagnitude = zInput;
    scale = 0.1f
        + (xMagnitude + zMagnitude) * 0.9f / 2.0f;
    this->particleEffect()->fn_1_B320(scale);

    M653MotionWork *soundWork = this->motionWork();
    if (lbl_1_bss_64 != 0) {
        if (scale <= 0.11f) {
            fn_1_4558(soundWork);
        } else {
            fn_1_44E4(soundWork, scale);
        }
    } else {
        fn_1_44E4(soundWork, 0.2f);
    }

    direction.x = this->baseMatrix[0][2];
    direction.y = this->baseMatrix[1][2];
    direction.z = this->baseMatrix[2][2];
    PSVECScale(&direction, &direction, -1.0f);
    direction.y += 0.6f;
    PSVECNormalize(&direction, &direction);
    this->particleEffect()->fn_1_B304(&direction);
}

extern "C" void fn_1_43FC(M653MotionWork *, const Mtx);

void fn_1_1424(Car *owner, const Mtx matrix)
{
    M653MotionWork *motion_work = owner->motion.value;
    fn_1_43FC(motion_work, matrix);
    owner->wrappedX += 5.0f;
    owner->wrappedY += 5.0f;
}

extern "C" void fn_1_4324(M653MotionWork *, const Mtx);

void fn_1_1498(Car *owner)
{
    M653MotionWork *motion_work = owner->motion.value;
    fn_1_4324(motion_work, owner->baseMatrix);
    Model *body = owner->primary.value;
    body->copy_matrix(owner->baseMatrix);
    Vec value;
    value.x = owner->baseMatrix[0][3];
    value.y = owner->baseMatrix[1][3];
    value.z = owner->baseMatrix[2][3];
}

extern "C" {
const Vec lbl_1_rodata_C0[6] = {
    { 84.0f, 40.0f, 98.0f },
    { 84.0f, 40.0f, 6.0f },
    { 84.0f, 40.0f, -82.0f },
    { -84.0f, 40.0f, 98.0f },
    { -84.0f, 40.0f, 6.0f },
    { -84.0f, 40.0f, -82.0f }
};
}

void fn_1_1508(M653EffectMatrixState *work)
{
    Mtx yawRotation;
    Mtx pitchRotation;
    Mtx transform;
    s32 index = 0;
    f32 angle;
    f32 pointDistance;
    f32 finalTranslationY;
    f32 finalTranslationZ;
    f32 translationX;
    f32 translationY;
    f32 translationZ;
    f32 pitch;
    ModelMatrix1CBC4 *modelNumber;
    ModelMatrix1CBC4 *finalModelNumber;
    ModelMatrix1CBC4 *finalModelNumberForTransform;

    while (index < 6) {
        PSMTXRotRad(yawRotation, 0x59,
                    MTXDegToRad((f32)(index >= 3 ? 0xB4 : 0)));
        angle = index >= 3 ? -work->wrappedY : work->wrappedX;
        PSMTXRotRad(pitchRotation, 0x58, MTXDegToRad(angle));
        PSMTXConcat(yawRotation, pitchRotation, transform);
        translationZ = lbl_1_rodata_C0[index].z;
        translationY = lbl_1_rodata_C0[index].y;
        translationX = lbl_1_rodata_C0[index].x;
        PSMTXTransApply(transform, transform,
                        translationX, translationY, translationZ);
        PSMTXConcat(work->baseMatrix, transform, transform);
        modelNumber = work->models[index].value;
        modelNumber->copy_matrix(transform);
        work->points[index].center.x = 0.0f;
        ((Vec &)work->points[index]).y = 0.0f;
        ((Vec &)work->points[index]).z = 0.0f;
        PSMTXMultVec(transform,
                     &work->points[index].center,
                     &work->points[index].center);
        pointDistance = 150.0f;
        work->points[index].setRadius(pointDistance);
        ++index;
    }
    PSMTXRotRad(transform, 0x59, 0.0f);
    finalTranslationZ = 0.0f;
    finalTranslationY = 61.0f;
    PSMTXTransApply(transform, transform, 0.0f,
                    finalTranslationY, finalTranslationZ);
    PSMTXConcat(work->baseMatrix, transform, transform);
    finalModelNumber = work->extra.value;
    if (finalModelNumber != 0) {
        finalModelNumberForTransform = work->extra.value;
        finalModelNumberForTransform->copy_matrix(transform);
    }
}

void Car::fn_1_17EC(EventMessage*) { flag10=1; }

void Car::fn_1_17F8(EventMessage*) {
    flag10=0;
    M653MotionWork *child=motion.value;
    child->fn_1_4454();
}

void Car::fn_1_183C(float x,float y) {
    rateX=x;
    rateY=y;
    wrappedX+=5.0f*x;
    wrappedY+=5.0f*y;
    wrappedX=(float)fmod(wrappedX,360.0);
    wrappedY=(float)fmod(wrappedY,360.0);
}

void Car::fn_1_1904() {
    PlayerInfo *predicate,*first,*second;
    float x,y;
    fn_1_183C(0.0f,0.0f);
    predicate=setting.value;
    if(predicate->isComputer()) {
        fn_1_2558(this);
        return;
    }
    first=setting.value;
    y=first->subStickY();
    second=setting.value;
    x=second->stickY();
    rateX=x;
    rateY=y;
    wrappedX=wrappedX+5.0f*x;
    wrappedY=wrappedY+5.0f*y;
    wrappedX=(float)fmod((double)wrappedX,360.0);
    wrappedY=(float)fmod((double)wrappedY,360.0);
}

Car::~Car()
{
}

/* An event callback that hands each event to the functor it holds (here a member function of
 * the car, bound to the car). The class name is descriptive: no framework template does this,
 * and the game keeps no name for it. */
template<class Functor> struct FunctorCallback : EventCallback {
    struct Payload : Functor {
        Payload(const Functor &source) : Functor(source) {}
        void call(EventMessage *event) { op(value, event); }
    };
    Payload payload;
    virtual ~FunctorCallback() {}
    virtual void receive(const EventPacket *value) { payload.call(value); }
    FunctorCallback(const Functor &value)
        : EventCallback(), payload(value) {}
};

void *operator new(unsigned long);

template<class Functor> void fn_1_1CA0(EventHandlerManager *owner, const Functor &payload, int key)
{ owner->addHandler(new FunctorCallback<Functor>(payload),key); }

/* Never read in this module. */
extern "C" unsigned char lbl_1_bss_28[4];
unsigned char lbl_1_bss_28[4];
