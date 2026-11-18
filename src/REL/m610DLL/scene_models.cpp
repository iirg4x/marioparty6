#include "REL/game_gravity.h"
extern "C"
{
#include "game/gamework.h"
}
#include "REL/m610DLL/scene_callback.h"
#include "REL/framework/event_manager.h"
#include <new>

#include "REL/m610DLL/segment_spawner.h"

extern "C"
{
#include "dolphin/types.h"
}

#include "dolphin/mtx.h"

extern "C"
{
#include "dolphin/os.h"
}

#include "math.h"

#include "dolphin/mtx/GeoTypes.h"

#include "REL/framework/p_chara.h"

#include "REL/m610DLL/moving_model_owner.h"

#include "REL/framework/random.h"

#include "REL/framework/ground_sprite.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/framework/game_object.h"

#include "REL/framework/model.h"

#include "REL/m610DLL/scene_effect.h"

extern "C"
{
#include "game/data.h"
}

#include "REL/framework/sound.h"

#include "REL/framework/event.h"

#include "REL/m610DLL/collision_node.h"

#include "REL/m610DLL/module_types.h"

extern "C"
{
#include "game/charman.h"
}

#include "REL/framework/frame_time.h"

/* Create a falling obstacle, its collision sphere, shadow and optional light. */
MovingModelOwner::MovingModelOwner(const OwnedModel *source, ANIMDATA *animation,
                                   const Vec *position)
    : GameObject(1), registration(1, this), model(new OwnedModel(*source)), drawing(0)
{
    s32 nightMode;
    opacity = 0.2f;
    this->position.x = position->x;
    this->position.y = position->y;
    this->position.z = position->z;
    f32 collisionRadius = 270.0f;
    registration.radius = collisionRadius;
    Vec color = { 1.5f, 1.5f, 1.5f };
    model->setUniformScale(2.0f);
    model->setTransparency(opacity);
    model->setAmbient(&color);
    model->setAttribute(0x40000002);
    model->resetAttribute(1);
    model->setMotionTime(0.0f);
    model->setLayer(5);
    model->setMotionSpeed(2.0f);
    Vec drawingPosition;
    elapsedTime = 0.1f * (f32) ((s32) RandomUnsigned() & 0x1F);
    delay = RandomFloat(10.0f);
    rotation.x = 0.0f;
    rotation.y = 0.0f;
    rotation.z = 0.0f;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    f64 drawingX;
    f64 drawingZ = 20.0f + this->position.z;
    drawingX = this->position.x - 20.0f;
    drawingPosition.x = drawingX;
    drawingPosition.y = 0.0f;
    drawingPosition.z = drawingZ;
    drawing.reset(new GroundSprite(animation, &drawingPosition));
    nightMode = GWMgNightFGet();
    if (nightMode == 0)
    {
        Vec lightPosition = { 0.0f, 0.0f, 1000.0f };
        Vec lightDirection = { 0.0f, 0.0f, -1.0f };
        GXColor lightColor = { 255, 255, 255, 255 };
        s16 lightModelId = *model.get();
        light = fn_800288B4(lightModelId, &lightPosition, &lightDirection, &lightColor);
        fn_800296E0(lightModelId, light, 1);
        fn_80028B58(lightModelId, light);
    }
    MovingModelOwner::Action initialState = &MovingModelOwner::fn_1_CDC0;
    action = initialState;
}

/* Release the obstacle light in day mode and its owned models. */
MovingModelOwner::~MovingModelOwner()
{
    short nightMode = lbl_802C0278;
    if (nightMode == 0)
    {
        fn_80028CB8(*model, light);
    }
}

/* Add the supplied offset to a coordinate. */
static inline void addOffset(float &out, const float &value, float offset)
{
    out = value + offset;
}

/* Advance the obstacle state and update its model, sphere and ground shadow. */
void MovingModelOwner::update()
{
    if (action != 0)
        (this->*action)();
    model->setPosition(&position);
    model->setRotation(&rotation);
    registration.center.x = position.x;
    addOffset(registration.center.y, position.y, 200.0f);
    registration.center.z = position.z;
    float scale = 1.0f + (0.15f * (position.y / 500.0f));
    GroundSprite *shadowSprite = drawing.get();
    shadowSprite->setRadius(scale);
    float shadowAlpha = this->opacity;
    GroundSprite *sprite = drawing.get();
    sprite->setAlpha(shadowAlpha);
}

/* Wait for the obstacle delay before starting its drop animation. */
void MovingModelOwner::fn_1_CDC0()
{
    elapsedTime += 0.016666666666666666;
    if (elapsedTime < delay)
        return;

    model->setTransparency(0.2f);
    opacity = 0.2f;
    model->resetAttribute(0x40000002);
    MovingModelOwner::Action next = &MovingModelOwner::fn_1_CE98;
    action = next;
}

/* Fade in the obstacle and wait for its motion to reach the falling phase. */
void MovingModelOwner::fn_1_CE98()
{
    float value = opacity;
    value += 0.02f;
    if (value > 1.0f)
        value = 1.0f;

    model->setTransparency(value);
    opacity = value;

    if (model->getMotionTime() < 110.0f)
        return;

    float finishingSpeed = (1.0f / 3.0f);
    model->setMotionSpeed(finishingSpeed);
    MovingModelOwner::Action next = &MovingModelOwner::fn_1_CFB4;
    action = next;
}

/* Integrate the falling obstacle until it reaches the ground. */
void MovingModelOwner::fn_1_CFB4()
{
    float previousRate = velocity.y;
    velocity.y += 4.0f * -gravityPerFrame;
    float progressDelta = 0.5f * (previousRate + velocity.y);
    position.y += progressDelta;
    if (position.y > 0.0f)
        return;

    elapsedTime = 0.0f;
    position.y = 0.0f;
    velocity.x = 0.0f;
    velocity.y = 0.0f;
    velocity.z = 0.0f;
    model->setMotionSpeed(1.0f);
    MovingModelOwner::Action initialAction = &MovingModelOwner::fn_1_D110;
    action = initialAction;
}

/* Send the impact event and enter the grounded wait state. */
void MovingModelOwner::fn_1_D110()
{
    sendEvent(1, 0, &position, 0.0f);
    elapsedTime = 0.0f;
    model->resetAttribute(0x40000002);
    MovingModelOwner::Action callback = &MovingModelOwner::fn_1_D1D4;
    action = callback;
    if (lbl_1_bss_68 == 0)
        soundPlay(0x3AB);
}

/* Wait on the ground before starting the return animation. */
void MovingModelOwner::fn_1_D1D4()
{
    elapsedTime += 0.016666666666666666;
    if (elapsedTime < 2.0f)
        return;
    model->setMotionTime(0.0f);
    model->setAttribute(0x40000002);
    Action next = &MovingModelOwner::fn_1_D2A4;
    action = next;
}

/* Fade and raise the obstacle, then choose its next delay. */
void MovingModelOwner::fn_1_D2A4()
{
    float opacityValue = opacity;
    opacityValue -= 0.02f;
    if (opacityValue < 0.2f)
        opacityValue = 0.2f;
    model->setTransparency(opacityValue);
    opacity = opacityValue;

    position.y += (800.0 / 60.0);
    if (position.y < 500.0f)
        return;
    position.y = 500.0f;
    elapsedTime = 0.0f;
    delay = RandomFloat(5.0f);
    model->setMotionSpeed(2.0f);
    MovingModelOwner::Action initialAction = &MovingModelOwner::fn_1_CDC0;
    action = initialAction;
}

/* Day and night ambient color multipliers for the scenery model. */
struct SceneAmbientColors
{
    Vec values[2];
};

/* Create an animated scenery model and configure its day or night motion. */
SceneEffect::SceneEffect(const OwnedModel *sourceModel)
    : GameObject(2), animation(new OwnedModel(*sourceModel))
{
    void *motionData;
    int nightMode;

    motionData = fn_80008DD0(DATANUM(0x450000, 10), 0x10000000, 2);
    motion = fn_80030458(*animation, motionData);
    fn_8002C95C(*animation, motion);

    SceneAmbientColors ambientColors = { { { 1.7f, 1.7f, 1.7f }, { 1.5f, 1.5f, 1.5f } } };
    animation->setUniformScale(1.3f);
    animation->setShadow();
    animation->setAttribute(0x40000001);
    animation->resetAttribute(1);
    animation->setLayer(5);

    nightMode = GWMgNightFGet();
    animation->setAmbient(&ambientColors.values[nightMode]);
    animation->setMotionTime(RandomFloat(96.0f));
    animation->setMotionSpeed(RandomFloat(0.5f, 1.5f));
}

/* Release the animated scenery model. */
SceneEffect::~SceneEffect()
{
}
/* Apply the scenery model position. */
void SceneEffect::update()
{
    animation->setPosition(&position);
}

/* Return the centered position and reduce the endpoint count. */
static inline f32 centeredSpacing(s32 index, s32 &count, f32 spacing)
{
    --count;
    return spacing * index - spacing * count / 2.0f;
}

/* Center a floating point row using the distance between adjacent members. */
f32 fn_1_D6BC(s32 index, s32 count, f32 spacing)
{
    return centeredSpacing(index, count, spacing);
}

/* Create five registered collision segments for the named model parts. */
SegmentSpawner::SegmentSpawner(OwnedModel *target) : GameObject(3), resource(target)
{
    s32 index;

    SegmentContactPayload *newNode;

    resource->setShadow();
    resource->setAttribute(0x40000001);
    resource->setLayer(5);
    resource->setMotionSpeed(0.1f);
    resource->setShadow();

    for (index = 0; index < 5; ++index)
    {
        newNode = new SegmentContactPayload;
        segments[index] = newNode;
    }
}

/* Model-part names for the two endpoints of each collision segment. */
struct SegmentPartNames
{
    char *first;
    char *second;
};

/* Endpoint model-part names, in segment order. */
SegmentPartNames lbl_1_data_2E0[5] = { { "bar03a", "bar03b" },
                                       { "bar04a", "bar04b" },
                                       { "bar05ad", "bar05b" },
                                       { "bar07a", "bar07b" },
                                       { "bar08a", "bar08b" } };

/* Refresh segment bounds and periodically play a sound at the next position. */
void SegmentSpawner::update()
{
    Vec difference;
    Vec spawnPosition;
    s32 index;
    s32 count;
    s32 spawnIndex;
    f32 radius;
    double spawnX;
    double spawnZ;

    static s32 frameCounter = 1;

    for (index = 0; index < 5; ++index)
    {
        SegmentContactPayload *firstSegment =
            static_cast<SegmentContactPayload *>(segments[index].value);
        Vec &firstPosition = firstSegment->firstPosition;
        SegmentContactPayload *secondSegment =
            static_cast<SegmentContactPayload *>(segments[index].value);
        Vec &secondPosition = secondSegment->secondPosition;
        SegmentContactPayload *boundsSegment =
            static_cast<SegmentContactPayload *>(segments[index].value);
        Sphere &bounds = *boundsSegment;
        f32 length;
        resource->getObjectPosition(lbl_1_data_2E0[index].first, &firstPosition);
        resource->getObjectPosition(lbl_1_data_2E0[index].second, &secondPosition);
        bounds.center.x = 0.5f * (firstPosition.x + secondPosition.x);
        bounds.center.y = 0.5f * (firstPosition.y + secondPosition.y);
        bounds.center.z = 0.5f * (firstPosition.z + secondPosition.z);
        PSVECSubtract(&firstPosition, &secondPosition, &difference);
        length = PSVECMag(&difference);
        radius = 0.5f * length;
        bounds.radius = radius;
    }

    if ((frameCounter % secondsToFrames<long>(2)) == 0)
    {
        static s32 nextSpawn = 0;
        spawnZ = 2800.0;
        count = 4;
        spawnIndex = nextSpawn;
        spawnX = centeredSpacing(spawnIndex, count, 800.0f);
        spawnPosition.x = spawnX;
        spawnPosition.y = 0.0f;
        spawnPosition.z = spawnZ;
        soundPlayAt(1, nextSpawn + 0x672, &spawnPosition);
        ++nextSpawn;
        nextSpawn %= 4;
    }
    frameCounter += 1;
}

/* Release the segment registrations and their model. */
SegmentSpawner::~SegmentSpawner()
{
}
