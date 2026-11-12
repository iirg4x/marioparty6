#include "REL/m671DLL/game_constants.h"
#include "REL/m671DLL/member_callback.h"
#include "REL/m671DLL/position_table.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
#include "dolphin/types.h"

#include "dolphin/mtx.h"

#include "math.h"

#include "REL/framework/linknode.h"

#include "REL/framework/game_object.h"

#include "REL/framework/minigame_core.h"

#include "REL/framework/event_hm.h"

#include "game/hu3d.h"

#include "game/data.h"

#include "REL/m671DLL/callback_receiver.h"
#include "REL/framework/model.h"

#include "REL/m671DLL/animation_owner.h"

#include "humath.h"

#include "REL/framework/sound.h"

#include "REL/framework/event_callback_inline.h"

#include "REL/framework/game_object2.h"

#include <vector>

#include <functional>

static const u32 lbl_1_rodata_298[8] = {
    DATANUM(0x820000, 12), DATANUM(0x820000, 13),
    DATANUM(0x820000, 14), DATANUM(0x820000, 15),
    DATANUM(0x820000, 19), DATANUM(0x820000, 16),
    DATANUM(0x820000, 17), DATANUM(0x820000, 18)
};

Vec fn_1_88B0(MotionPosition *owner, const Vec *target, float height)
{
    MotionPosition position;
    float verticalTravel;
    float duration;

    verticalTravel = height + (target->y - owner->value.y);
    duration = (float)((double)(2.0f * verticalTravel / gravityAcceleration) /
                       0.016666666666666666);
    position = *owner;
    PSVECSubtract(&position.value, target, &position.value);
    PSVECScale(&position.value, &position.value, 1.0f / duration);
    position.value.y = height;
    return position.value;
}

typedef std::binder1st<std::mem_fun1_t<void, CallbackReceiver, EventMessage *> > CallbackInit;

void fn_1_942C(CallbackReceiver *object, u32 motion, u8 enabled);

void fn_1_930C(CallbackReceiver *);

template<class Payload> void fn_1_B200(EventHandlerManager *owner, const Payload &payload, s32 key)
{
    owner->addHandler(new MemberCallback<CallbackReceiver>(payload), key);
}

CallbackReceiver::CallbackReceiver(unsigned int type, Vec *position, Vec *target)
    : GameObject(4), ModelMotion(type), MinigameSingleton<CallbackReceiver>(),
      position(*position), target(*target),
      lowerModel(new OwnedModel(DATANUM(0x820000, 8))), upperModel(new OwnedModel(DATANUM(0x820000, 8))),
      active(0), motion(-1), state(0), callbacks(0)
{
    s16 lightId;

    previousState = 0;
    setCamera(1);
    resetAttribute(((1 << 30) | (1 << 1)));
    resetShadow();
    setMotionTime(0.0f);
    setLayer(3);
    setPosition(position);
    setRotation(target);
    setUniformScale(1.0f);
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;

    Vec positionOffset = *position;
    positionOffset.y -= 50.0f;
    lowerModel->setPosition(&positionOffset);
    lowerModel->setLayer(3);
    upperModel->setShadow();
    upperModel->setAttribute(8);
    Vec targetOffset = positionOffset;
    targetOffset.y = 50.0f;
    upperModel->setPosition(&targetOffset);
    addMotions(lbl_1_rodata_298, 8);
    ::fn_1_942C(this, DATANUM(0x820000, 12), 1);
    setAttribute(((1 << 30) | (1 << 0)));
    index = 0;
    timer = 0.0f;
    frameCount = 0;
    fn_1_930C(this);

    fn_1_B200(&callbacks, std::bind1st(
        std::mem_fun(&CallbackReceiver::fn_1_A230), this), 0);
    fn_1_B200(&callbacks, std::bind1st(
        std::mem_fun(&CallbackReceiver::fn_1_A28C), this), 2);
    fn_1_B200(&callbacks, std::bind1st(
        std::mem_fun(&CallbackReceiver::fn_1_A3A4), this), 7);
    fn_1_B200(&callbacks, std::bind1st(
        std::mem_fun(&CallbackReceiver::fn_1_A2E0), this), 5);
    fn_1_B200(&callbacks, std::bind1st(
        std::mem_fun(&CallbackReceiver::fn_1_A530), this), 8);

    {
        Vec lightPosition = {0.0f, 0.0f, 1000.0f};
        Vec lightDirection = {0.0f, 0.0f, -1.0f};
        GXColor lightColor = {200, 200, 200, 255};

        lightId = *static_cast<const Model *>(this);
        light = fn_800288B4(lightId, &lightPosition, &lightDirection, &lightColor);
    }
    fn_800296E0(lightId, light, 1);
    fn_80028B58(lightId, light);
    setState(&CallbackReceiver::fn_1_94CC);
}

CallbackReceiver::~CallbackReceiver()
{
    const Model *modelIdPointer;
    s16 modelId;
    s16 lightId;

    modelIdPointer = this;
    modelId = modelIdPointer->modelId;
    lightId = modelId;
    fn_80028CB8(lightId, light);
}

void fn_1_57C4(AnimationOwner *);

void fn_1_930C(CallbackReceiver *owner)
{
    int index;
    AnimationOwner *resource;

    for (index = 0; index < 3; index++) {
        resource = new AnimationOwner(0x820004, index);
        owner->objects[index] = resource;
        fn_1_57C4(owner->objects[index]);
        owner->entryFlags[index] = 0;
    }
}

void CallbackReceiver::update()
{
    if (currentState) {
        (this->*currentState)();
    }
    if (motion != DATANUM(0x820000, 13)) {
        timer += 0.016666666666666666;
    }
}

inline void CallbackReceiver::setMotion(u32 value, u32 flags)
{
    if (motion != value) {
        shiftMotion(value, 8.0f, 0.0f, flags);
        motion = value;
    }
}

void fn_1_942C(CallbackReceiver *object, u32 motion, u8 enabled)
{
    u32 flags;

    if (object->motion != motion) {
        if (enabled == 0) {
            flags = 0;
        } else {
            flags = 0x40000001U;
        }
        object->shiftMotion(motion, 8.0f, 0.0f, flags);
        object->motion = motion;
    }
}

void CallbackReceiver::fn_1_94C0() {
    active = 1;
}

void CallbackReceiver::fn_1_94CC() {}

void fn_1_963C(CallbackReceiver *, const Vec *);

void fn_1_94D0(CallbackReceiver *camera, f32 angle)
{
    MotionPosition viewPosition;
    MotionPosition targetPosition;

    MotionPosition selectedPosition;
    AnimationOwner *selectedView = camera->objects[camera->index];

    selectedPosition = selectedView->position;
    {
        MotionPosition *selectedPositionOwner = &selectedPosition;
        viewPosition = *selectedPositionOwner;
    }
    camera->velocity.y = (f32)(0.3f * -sin(
        (M_PI * (180.0f * angle)) / 180.0) * 100.0);
    camera->position.value.y = 400.0f + camera->velocity.y;
    viewPosition.value.x = camera->position.value.x;
    viewPosition.value.z = camera->position.value.z;
    fn_1_963C(camera, &viewPosition.value);
    camera->setPosition(&camera->position.value);
    targetPosition = camera->position;
    targetPosition.value.y -= 50.0f;
    camera->lowerModel.get()->setPosition(&targetPosition.value);
}

void fn_1_5754(AnimationOwner *, const Vec *);

extern void fn_1_5824(AnimationOwner *camera, const Vec *position);

void fn_1_963C(CallbackReceiver *camera, const Vec *position)
{
    Mtx rotation;
    Vec offset = {0.0f, 0.0f, 70.0f};

    PSMTXRotRad(rotation, 'y', MTXDegToRad(camera->target.y));
    PSMTXMultVec(rotation, &offset, &offset);
    PSVECAdd(&offset, position, &offset);
    fn_1_5754(camera->objects[camera->index], &offset);
}

void fn_1_96E0(CallbackReceiver *camera, const Vec *position)
{
    Mtx rotation;
    Vec offset = {0.0f, 0.0f, 70.0f};

    PSMTXRotRad(rotation, 'y', MTXDegToRad(camera->target.y));
    PSMTXMultVec(rotation, &offset, &offset);
    PSVECAdd(&offset, position, &offset);
    fn_1_5824(camera->objects[camera->index], &offset);
}

static inline void updateFallingPosition(CallbackReceiver *owner, f32 blend)
{
    Vec sourcePosition = static_cast<AnimationOwner *>(owner->objects[owner->index])->getPosition();
    owner->velocity.y = (f32)(100.0 * ((double)0.3f * -sin(M_PI * (f64)(180.0f * blend) / 180.0)));
    owner->position.value.y = 400.0f + owner->velocity.y;
    sourcePosition.x = owner->position.value.x;
    sourcePosition.z = owner->position.value.z;
    fn_1_963C(owner, &sourcePosition);
    owner->setPosition(&owner->position.value);
    Vec lowerPosition = owner->position.value;
    lowerPosition.y -= 50.0f;
    owner->lowerModel.get()->setPosition(&lowerPosition);
}

void CallbackReceiver::fn_1_9784()
{
    Vec position = static_cast<AnimationOwner *>(this->objects[this->index])->getPosition();
    position.x = this->position.value.x;
    position.z = this->position.value.z;
    fn_1_963C(this, &position);
    if (this->motion == DATANUM(0x820000, 15))
    {
        if (fn_8002D75C(*static_cast<const Model *>(this)) == 0)
        {
            setMotion(DATANUM(0x820000, 19), ((1 << 30) | (1 << 0)));
        }
    }
    if (! (this->timer < 0.5f))
    {
        if (this->timer < 0.7f && this->index != 0)
        {
            updateFallingPosition(this, (this->timer - 0.5f) / 0.2f);
        }
        {
            f32 endProgress = 0.8f;
            if (this->index == 0)
            {
                endProgress = 2.0f;
            }
            if (! (this->timer < endProgress))
            {
                this->timer = 0.0f;
                soundPlayAt(1, 0x89C, &position);
                setMotion(DATANUM(0x820000, 14), 0);
                soundPlayAt(1, 0x3CE, &this->position.value);
                setState(&CallbackReceiver::fn_1_9C7C);
            }
        }
    }
}

Vec fn_1_9BFC(const CallbackReceiver *owner)
{
    Vec modelPosition;
    Hu3DModelObjPosGet(*static_cast<const Model *>(owner), "itemhook_R", &modelPosition);
    return modelPosition;
}

void fn_1_58E4(AnimationOwner *, const Vec *);

static inline Vec getTransitionPosition(AnimationOwner *entry)
{
    return entry->position.value;
}

static inline void setDescentSpeed(AnimationOwner *entry, f32 radius)
{
    entry->descentSpeed = radius;
}

static inline Vec &subtractVector(Vec *value, const Vec *other)
{
    PSVECSubtract(value, other, value);
    return *value;
}

static inline Vec &scaleVector(Vec *value, f32 scale)
{
    PSVECScale(value, value, scale);
    return *value;
}

static inline f32 vectorLength(const Vec *value)
{
    return PSVECMag(value);
}

static inline Vec calculateLaunchVelocity(const Vec *positionSource,
                                        const Vec *target, f32 height)
{
    f32 verticalTravel;
    f32 duration;

    verticalTravel = height + (target->y - positionSource->y);
    duration = (f32)((double)(2.0f * verticalTravel / gravityAcceleration) /
                     0.016666666666666666);
    Vec position = *positionSource;
    subtractVector(&position, target);
    scaleVector(&position, 1.0f / duration);
    position.y = height;
    return position;
}

void CallbackReceiver::fn_1_9C7C()
{
    f32 ratio;
    f32 range;
    f32 speed;
    f64 horizontalOffset;
    f64 forwardOffset;
    Mtx matrix;
    if (this->timer < 0.2)
    {
        ratio = (f32)((f64) this->timer / 0.2);
        AnimationOwner *entry = static_cast<AnimationOwner *>(this->objects[this->index]);
        Vec position = getTransitionPosition(entry);
        this->velocity.y = (f32)(100.0 * ((double)0.3f * -sin(M_PI * (f64)(180.0f * ratio) / 180.0)));
        this->position.value.y = 400.0f + this->velocity.y;
        position.x = this->position.value.x;
        position.z = this->position.value.z;
        fn_1_963C(this, &position);
        return;
    }
    if (this->entryFlags[this->index] == 0)
    {
        range = 2.5f;
        if (! (this->index))
        {
            range = 1.5f;
        }
        AnimationOwner *entry = static_cast<AnimationOwner *>(this->objects[this->index]);
        Vec entryPosition = getTransitionPosition(entry);
        forwardOffset = 100.0f * RandomFloat(- range, range);
        horizontalOffset = 100.0f * RandomFloat(- range, range);
        Vec position;
        position.x = horizontalOffset;
        position.y = 0.0f;
        position.z = forwardOffset;
        PSMTXRotRad(matrix, 'Y', 0.7853982f);
        PSMTXMultVec(matrix, &position, &position);
        Vec velocity = calculateLaunchVelocity(&position, &entryPosition, 33.333332f);
        speed = vectorLength(& velocity);
        fn_1_58E4(static_cast<AnimationOwner *>(this->objects[this->index]), & velocity);
        setDescentSpeed(static_cast<AnimationOwner *>(this->objects[this->index]), speed);
        this->entryFlags[this->index] = 1;
    }
    if (! (this->timer < 1.5f))
    {
        this->timer = 0.0f;
        setMotion(DATANUM(0x820000, 12), ((1 << 30) | (1 << 0)));
        this->index++;
        if (this->index == 3)
        {
            setState(&CallbackReceiver::fn_1_94C0);
            return;
        }
        if (this->state == 1)
        {
            setState(&CallbackReceiver::fn_1_94C0);
            return;
        }
        {
            setState(&CallbackReceiver::fn_1_AE78);
        }
    }
}

void CallbackReceiver::fn_1_A230(EventMessage *)
{
    this->frameCount = 0;
    this->timer = 0.0f;
    {
        setState(&CallbackReceiver::fn_1_A794);
    }
}

void CallbackReceiver::fn_1_A28C(EventMessage *)
{
    this->timer = 0.0f;
    setState(&CallbackReceiver::fn_1_9784);
}

void CallbackReceiver::fn_1_A2E0(EventMessage *)
{

    this->state = 1;
    if (this->motion == DATANUM(0x820000, 12)) {
        setMotion(DATANUM(0x820000, 12), ((1 << 30) | (1 << 0)));
        setState(&CallbackReceiver::fn_1_94C0);
    }
}

/* Selects the victory motion and its dialogue based on the result state. */
void CallbackReceiver::fn_1_A3A4(EventMessage *input)
{
    u8 *state;

    state = static_cast<u8 *>(input->context);
    if (*state == 1) {
        setMotion(DATANUM(0x820000, 18), 0);
        soundPlayAt(1, 0x3CF, &this->position.value);
        soundPlayAtDelayed(1, 0x3C9, &this->position.value, 0x41);
        soundPlayAtDelayed(1, 0x3C9, &this->position.value, 0x5A);
        soundPlayAtDelayed(1, 0x3CC, &this->position.value, 0x78);
    } else {
        setMotion(DATANUM(0x820000, 17), 0);
        soundPlayAtDelayed(1, 0x3CB, &this->position.value, 0x1E);
        soundPlayAtDelayed(1, 0x3CD, &this->position.value, 0x6E);
    }
    setState(&CallbackReceiver::fn_1_94C0);
}

void CallbackReceiver::fn_1_A530(EventMessage *)
{

    if (this->motion == DATANUM(0x820000, 12)) {
        soundPlayAtDelayed(1, 0x3CB, &this->position.value, 0x26);
        setMotion(DATANUM(0x820000, 13), 0);
        this->previousState = this->currentState;
        setState(&CallbackReceiver::fn_1_A614);
    }
}

void CallbackReceiver::fn_1_A614()
{
    PositionTable *playerStatus;
    s32 index;
    bool allPlayersReady;

    if (fn_8002D75C(*static_cast<Model *>(this)) == 0) {
        return;
    }
    setMotion(DATANUM(0x820000, 12), ((1 << 30) | (1 << 0)));
    playerStatus = &PositionTable::getChecked();
    allPlayersReady = 1;
    for (index = 0; index < 4; ++index) {
        if (playerStatus->activeFlags[index] != 0) {
            allPlayersReady = 0;
            break;
        }
    }
    if (allPlayersReady == 0) {
        this->currentState = this->previousState;
        return;
    }
    {

        setState(&CallbackReceiver::fn_1_94C0);
    }
}

extern void fn_1_5F6C(AnimationOwner *, f32);

void CallbackReceiver::fn_1_A794()
{
    f32 interpolation;
    f32 blend;
    f32 risingHeight;
    this->frameCount++;
    if ((f32) this->frameCount == 90.0f)
    {
        fn_1_96E0(this, & this->position.value);
        setMotion(DATANUM(0x820000, 15), 0);
    }
    if (this->motion == DATANUM(0x820000, 15))
    {
        if (fn_8002D75C(*static_cast<const Model *>(this)) == 0)
        {
            setMotion(DATANUM(0x820000, 19), ((1 << 30) | (1 << 0)));
        }
    }
    if ((f32) this->frameCount == 120.0f)
    {
        sendEvent(1, 0, 0.0f);
    }
    if (this->timer >= 2.0f && this->timer <= 2.2f)
    {
        blend = (this->timer - 2.0f) / 0.2f;
        updateFallingPosition(this, blend);
    }
    if ((f32) this->frameCount > 150.0f)
    {
        interpolation = 0.027777778f;
        this->target.y = (f32)((f64) this->target.y - 0.6666666666666666);
        if (this->target.y < - 45.0f)
        {
            this->target.y = - 45.0f;
        }
        this->position.value.x += (650.0f - this->position.value.x) * interpolation;
        this->position.value.z += (- 250.0f - this->position.value.z) * interpolation;
        this->setPosition(& this->position.value);
        this->setRotation(&this->target);
        Vec position = this->position.value;
        position.y -= 50.0f;
        this->lowerModel.get()->setPosition(&position);
        this->lowerModel.get()->setRotation(&this->target);
        Vec risingPosition = position;
        risingHeight = 4.6666665f * ((f32) this->frameCount - 150.0f);
        risingPosition.y = 50.0f + risingHeight;
        this->upperModel.get()->setPosition(& risingPosition);
        this->upperModel.get()->setRotation(&this->target);
        position = static_cast<AnimationOwner *>(this->objects[this->index])->getPosition();
        position.x = this->position.value.x;
        position.z = this->position.value.z;
        fn_1_963C(this, &position);
        fn_1_5F6C(this->objects[this->index], this->target.y);
    }
}

extern "C" { const f32 lbl_1_rodata_3A0[] = {0.0f, 7.0f, 0.0f, 0.0f}; }

void CallbackReceiver::fn_1_AE78()
{

    if (this->index != 3) {
        if (!(this->timer < lbl_1_rodata_3A0[this->index])) {
            if (this->index == 1) {
                this->timer = 0.0f;
                soundPlayAt(1, 0x3D1, &this->position.value);
                soundPlayAtDelayed(1, 0x3C9, &this->position.value, 0x14);
                setMotion(DATANUM(0x820000, 16), 0);
                setState(&CallbackReceiver::fn_1_B09C);
            } else {
                fn_1_96E0(this, &this->position.value);
                setMotion(DATANUM(0x820000, 15), 0);
                this->timer = 0.0f;
                setState(&CallbackReceiver::fn_1_9784);
            }
        }
    }
}

void CallbackReceiver::fn_1_B09C()
{

    this->timer += 0.016666666666666666;
    if (this->timer < 1.5f) {
        return;
    }
    this->timer = 0.0f;
    fn_1_96E0(this, &this->position.value);
    setMotion(DATANUM(0x820000, 15), 0);
    setState(&CallbackReceiver::fn_1_9784);
}

