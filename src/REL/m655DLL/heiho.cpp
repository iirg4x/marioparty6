#include "REL/m655DLL/game_constants.h"
#include "REL/m655DLL/rope_api.h"
#include "REL/m655DLL/goal.h"
#include "REL/m655DLL/player_entry.h"
#include "REL/framework/range.h"
#include "REL/framework/sound.h"
#include "REL/framework/frame_time.h"
#include <new>
#include "REL/m655DLL/sequence_event_object.h"
#include "REL/m655DLL/player_state.h"
#include "dolphin/mtx.h"
#include "dolphin/types.h"
#include "REL/m655DLL/target_effect.h"
#include "REL/framework/model.h"
#include "datadir_enum.h"
#include <vector>
#include "math.h"
#include "REL/framework/singleton.h"
#include "humath.h"
#include "REL/m655DLL/square.h"
#include "REL/m655DLL/effect_vector_inline.h"
#include "REL/framework/world_units.h"
#include "REL/m655DLL/heiho.h"
const u32 lbl_1_rodata_178[3] = { 0x720003, 0x720004, 0x720005 };
const u32 lbl_1_rodata_184[3] = { 0x720007, 0x720008, 0x720009 };
void fn_1_39C4(void)
{
    for (int index = 0; index < 3; ++index) {
        new SequenceEventObject(index);
    }
}
SequenceEventObject::SequenceEventObject(int index)
    : GameObject(index + 4), ModelMotion(DATANUM(0x720000, 2)),
      channelZeroHandlers(0), indexedChannelHandlers(1 + index),
      state(&SequenceEventObject::fn_1_4048), flag7(1),
      flag6(0), index(index), registeredValue(0)
{
    static Vec positions[3] = {
        {cmToWorld(-250.0f), cmToWorld(405.0f), cmToWorld(-630.0f)},
        {cmToWorld(0.0f), cmToWorld(405.0f), cmToWorld(-630.0f)},
        {cmToWorld(250.0f), cmToWorld(405.0f), cmToWorld(-630.0f)}
    };
    this->setPosition(&positions[index]);
    this->setLayer(1);
    this->setShadow();
    f32 height = 50.0f;
    this->registeredValue.radius = height;
    addMotions(lbl_1_rodata_178, 3);
    setMotion(DATANUM(0x720000, 4));
    this->setAttribute(0x40000001);
    fn_1_52B8(&this->channelZeroHandlers,
        sequenceBind(std::mem_fun(&SequenceEventObject::fn_1_3F54), this), 6);
    fn_1_52B8(&this->indexedChannelHandlers,
        sequenceBind(std::mem_fun(&SequenceEventObject::fn_1_3F40), this), 0);
    fn_1_52B8(&this->indexedChannelHandlers,
        sequenceBind(std::mem_fun(&SequenceEventObject::fn_1_3F68), this), 3);
    fn_1_52B8(&this->indexedChannelHandlers,
        sequenceBind(std::mem_fun(&SequenceEventObject::fn_1_3FBC), this), 4);
}
static inline void setSequenceState(SequenceEventObject *object, SequenceEventObject::State next)
{ object->state = next; }
SequenceEventObject::~SequenceEventObject() {}
void SequenceEventObject::fn_1_3F40(EventMessage *message)
{
    this->flag7 = 0;
}
void SequenceEventObject::fn_1_3F54(EventMessage *message)
{
    this->flag6 = 1;
}
void SequenceEventObject::fn_1_3F68(EventMessage *message)
{
    flag6 = 1;
    sendEvent(index + 1, 4, 0.3f);
}
void SequenceEventObject::fn_1_3FBC(EventMessage *message)
{
    registeredValue.clearFlag7();
}
/* Run the actor callback and apply its conditional motion update. */
void SequenceEventObject::update()
{
    (this->*state)();
    if (registeredValue.flag7) {
        this->getObjectPosition("itemhook_C", &registeredValue.center);
    }
}
void SequenceEventObject::fn_1_4048()
{
    fn_1_4538(this);
    if (!this->flag7) {
        shiftMotion(DATANUM(0x720000, 5),
                   (f32)secondsToFrames(0.3), 0.0f, 0);
        this->setMotionSpeed(0.5f);
        setSequenceState(this, &SequenceEventObject::fn_1_4194);
        return;
    }
    if (this->flag6) {
        shiftMotion(DATANUM(0x720000, 3),
                   (f32)secondsToFrames(0.3), 0.0f, 0x40000001);
        setSequenceState(this, &SequenceEventObject::fn_1_4318);
    }
}
void SequenceEventObject::fn_1_4194(void)
{
    fn_1_4538(this);
    if (getMotionTime() < 14.0f) {
        return;
    }
    setSequenceState(this, &SequenceEventObject::fn_1_4218);
}
void SequenceEventObject::fn_1_4218(void)
{
    if (this->isMotionEnd() != 0) {
        Vec position;
        this->getPosition(&position);
        position.x = (f32)((f64)position.x - 100.0);
        position.z = (f32)((f64)position.z - 50.0);
        this->setPosition(&position);
        setMotion(DATANUM(0x720000, 3));
        this->setMotionSpeed(1.0f);
        this->setAttribute(0x40000001);
        this->updateMotion();
        setSequenceState(this, &SequenceEventObject::fn_1_4318);
    }
}

void SequenceEventObject::fn_1_4318()
{
    PlayerList elements;
    s32 count;
    s32 index;

    count = fn_1_7EF4(&elements);
    if (count <= 0) {
        return;
    }
    index = 0;
    while (index < count) {
        Sphere *element = static_cast<Sphere *>(elements[index]);
        f32 value = element->center.y;

        resolveSphereContact(&registeredValue, element,
                   0.0f, 0.1f);
        element->center.y = value;
        index += 1;
    }
}

static inline f32 maximumValue(f32 first, f32 second)
{
    return first > second ? first : second;
}

static inline f32 interpolateValue(f32 start, f32 end, f64 weight)
{
    return (f32)((1.0 - weight) * start + weight * end);
}

/* One flag byte and seven unknown zero-initialized bytes at .bss 0x68. */

// The target uses the shorter path around the circle.
// Its angles are expressed in degrees.

void fn_1_4538(SequenceEventObject *object)
{
    EffectVector leftOffset;
    EffectVector rightOffset;
    GameObject *model;
    EffectVector *offsetAddress;
    EffectVector *scaledOffset;

    object->getObjectPosition( "itemhook_L", &leftOffset);
    object->getObjectPosition( "itemhook_R", &rightOffset);
    PSVECAdd(&rightOffset, &leftOffset, &leftOffset);
    offsetAddress = &leftOffset;
    scaledOffset = offsetAddress;
    PSVECScale(scaledOffset, scaledOffset, 0.5f);
    model = fn_1_B140(object->index);
    fn_1_B8E0((Rope *)model, &leftOffset, &rightOffset, 1.0f);
}

TargetEffect::TargetEffect()
    : GameObject(3), ModelMotion(DATANUM(0x720000, 6)),
      targetState(&TargetEffect::fn_1_4A28), m_target_id(fn_1_7CB8()),
      timer(0), firstRegisteredValue(0), secondRegisteredValue(1), soundId(-1)
{

    PlayerWork *target;
    Vec *scenePosition;
    Vec *effectPosition;
    Vec *contextPosition;
    f32 endScale;

/* Retain assertion source location.
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 *
 */
    assert(m_target_id >= 0);
    this->setLayer(3);
    this->setShadow();
    target = fn_1_7E1C(this->m_target_id);
    scenePosition = &target->position.center;
    effectPosition = &(this->currentPosition = *scenePosition);
    this->startPosition = *effectPosition;

    contextPosition = MinigameSingleton<Goal>::getChecked().fn_1_3470();
    this->effectPosition = *contextPosition;
    this->effectPosition.y += target->height() + cmToWorld(50.0);
    this->effectPosition.z =
        (f32)((f64)this->effectPosition.z - 50.0);
    this->setPosition(&this->startPosition);
    addMotions(lbl_1_rodata_184, 3);
    setMotion(DATANUM(0x720000, 9));
    this->setAttribute(0x40000001);
    endScale = 75.0f;
    this->firstRegisteredValue.radius = endScale;
    this->secondRegisteredValue.radius = endScale;
    fn_1_9DC8(target);

}

static inline void setTargetState(TargetEffect *object, TargetEffect::State next)
{ object->targetState = next; }
TargetEffect::~TargetEffect() {}

void TargetEffect::update()
{
    f32 minimumHeight;

    (this->*targetState)();
    this->setPosition(&this->startPosition);
    minimumHeight = this->lowerHeight();
    RaisedPosition lowerPosition(this->startPosition.x,
        maximumValue(this->startPosition.y + minimumHeight,
                    minimumHeight + cmToWorld(-400.0f)),
        this->startPosition.z);
    this->firstRegisteredValue = lowerPosition;
    this->secondRegisteredValue = lowerPosition;
    fn_1_50C8(this);
}

void TargetEffect::fn_1_4A28(void)
{
    f32 distance = (f32)framesToSeconds(0x12C);
    f32 previousHeight = startPosition.y;
    f32 boundary = -100.0f;
    f32 lowerHeight = startPosition.y + distance;
    f32 parameter =
        (lowerHeight - currentPosition.y) /
        (effectPosition.y - currentPosition.y);
    f64 weight = square(square(square(parameter)));
    f64 targetX;
    f64 targetZ;
    targetZ = interpolateValue(currentPosition.z, effectPosition.z, weight);
    targetX = interpolateValue(currentPosition.x, effectPosition.x, weight);
    EffectVector offset;
    bool enteredLowerRegion;

    offset.x = targetX;
    offset.y = (f64)lowerHeight;
    offset.z = targetZ;
    PSVECSubtract(&offset, &startPosition, &offset);
    offset.normalize() *= distance;
    PSVECAdd(&offset, &startPosition, &startPosition);

    if (previousHeight < boundary && startPosition.y >= boundary) {
        sendEvent(0, 5, 0.0f);
    }

    enteredLowerRegion = startPosition.y >= effectPosition.y;
    if (enteredLowerRegion) {
        startPosition.x = effectPosition.x;
        startPosition.y = effectPosition.y;
        startPosition.z = effectPosition.z;
        s32 timer = secondsToFrames<long>(2);
        this->timer = timer;
        shiftMotion(DATANUM(0x720000, 7),
                   (f32)secondsToFrames(0.3), 0.0f,
                   0x40000001);
        setTargetState(this, &TargetEffect::fn_1_4DA8);
    }
    fn_1_5044();
    if (enteredLowerRegion) {
        fn_1_9ED8(fn_1_7E1C(m_target_id));
    }
}

void TargetEffect::fn_1_4DA8(void)
{
    u8 expired = 0;

    if (timer > 0 && --timer == 0) {
        expired = 1;
    }
    if (expired != 0) {
        firstRegisteredValue.clearFlag7();
        secondRegisteredValue.clearFlag7();
        shiftMotion(DATANUM(0x720000, 8),
                   (f32)secondsToFrames(0.3), 0.0f,
                   0x40000001);
        setTargetState(this, &TargetEffect::fn_1_4E8C);
    }
}

void TargetEffect::fn_1_4E8C(void)
{
    EffectVector offset;
    EffectVector *offsetAddress;
    EffectVector *scaledOffset;
    f32 scale;
    f32 rate;
    f64 horizontalDelta;
    f64 verticalOffset;
    f64 depthOffset;

    scale = (f32)framesToSeconds(0x12C);
    depthOffset = 0.1 * (f64)(this->effectPosition.z - this->currentPosition.z);
    verticalOffset = framesToSeconds(10.0);
    horizontalDelta = this->effectPosition.x - this->currentPosition.x;
    offset.x = horizontalDelta;
    offset.y = (f32)verticalOffset;
    offset.z = (f32)depthOffset;
    PSVECNormalize(&offset, &offset);
    offsetAddress = &offset;
    scaledOffset = offsetAddress;
    PSVECScale(scaledOffset, scaledOffset, scale);
    PSVECAdd(&offset, &this->startPosition, &this->startPosition);
    rate = (f32)(180.0 *
                 (atan2((f64)offset.x, (f64)offset.z) /
                  M_PI));
    Range<float> range(0.0f, 360.0f, this->getRotationY());
    this->setRotationY(range.step(rate, 0.1));
}

void TargetEffect::fn_1_5044()
{
    Vec bounds;

    getObjectPosition("itemhook_C",
                                &bounds);
    bounds.y -= 50.0;
    bounds.z += 50.0;
    fn_1_9E40(fn_1_7E1C(m_target_id), &bounds);
}

void fn_1_50C8(TargetEffect *owner)
{
    f32 yOffset;

    if (owner->soundId == -1) {
        owner->soundId = soundPlay(0x3C3);
    }
    Vec position = owner->startPosition;
    yOffset = owner->secondRegisteredValue.radius;
    position.y += yOffset;
    soundSetVolumeAt(1, owner->soundId, &position);
    soundPanTo(1, owner->soundId, &position);
}
