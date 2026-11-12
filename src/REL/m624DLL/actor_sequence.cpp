#include "REL/m624DLL/game_constants.h"
#include "REL/framework/world_units.h"
#include "REL/m624DLL/scene_sequence_contract.h"
#include "REL/m624DLL/handle_list_object.h"
#include <math.h>

extern "C" {
#include "dolphin.h"
#include "humath.h"
#include "game/hu3d.h"
#include "game/mg/actman.h"
#include "REL/m624DLL.h"
}

float RandomFloat();
u32 RandomUnsigned();

// One of the actors the sequence manager spawns on its circle: it swings while it walks and falls
// back to its start when it drops off the map.
class SequenceActor : public HandleListObject {
public:
    typedef void (SequenceActor::*Callback)();

    SequenceActor(Vec *position, s32 param, s32 data);
    virtual ~SequenceActor();
    virtual void update();
    void fn_1_2710();
    BOOL fn_1_2768();
    void DebugValue(u8 value) {}

    Callback callback;
    OwnedModel modelStorage;
    MGACTOR *actor;
    Point3d position;
    u8 updateDisabled;
    u8 state;
    FrameInterpolator<float> interpolation;
    f32 oscillationAngle;
};

// The centre of the circle the actors start from.
static Vec circleCenter = { cmToWorld(701.0f), cmToWorld(130.0f), cmToWorld(87.0f) };

#include "REL/m624DLL/tail_types.h"
extern "C" {
}

// Places one point on the circle of radius `radius` around circleCenter, at `angle`; the second
// point sits 400 units higher than the first.
static inline const Vec &calcPoint(Vec &position, f64 radius, f64 angle, s32 index)
{
    s32 height;

    position.x = (f32)(radius * sin(angle));
    height = ((index * 4) + 0xA) * 0x64;
    position.y = (f32)height;
    position.z = (f32)(radius * cos(angle));
    PSVECAdd(&circleCenter, &position, &position);
    return position;
}

// Registers the manager as the current one and fills `points` with two points on opposite sides
// of the circle, starting at a random angle.
ActorSequence::ActorSequence() : spawnedCount(0)
{
    Vec position;
    s32 index;
    f64 radius;
    f64 angle;

    radius = 190.0;
    angle = M_PI * RandomFloat();
    for (index = 0; index < 2; angle += M_PI) {
        points.push_back(calcPoint(position, radius, angle, index));
        index++;
    }
}

ActorSequence::~ActorSequence() {}

// Spawns an actor at every point, in random order, and keeps its resource in `objects`;
// `spawnedCount` counts the actors spawned.
void fn_1_2168(ActorSequence *owner)
{
    while (!owner->points.empty()) {
        unsigned int index = RandomUnsigned() % owner->points.size();
        SequenceActor *object =
            new SequenceActor(&owner->points.at(index), owner->spawnedCount + 4, 0x530007);
        owner->objects.push_back(fn_1_5E1C(object));
        owner->points.erase(&owner->points.at(index));
        owner->spawnedCount++;
    }
}

extern "C" {
}
extern "C" {

}

std::vector<void *> *fn_1_2264(ActorSequence *manager)
{
    std::vector<void *>::iterator current = manager->objects.begin();
    std::vector<void *>::iterator last = manager->objects.end();
    while (current != last) {
        if (fn_1_5E24(*current) == 0) {
            manager->objects.erase(current);
        }
        ++current;
    }
    return &manager->objects;
}

Point3d *fn_1_22F0(SequenceActor *owner, Point3d *position)
{
    fn_80086BC8(owner->actor, position);
    return position;
}

extern "C" {

int fn_1_27BC(COL_NARROW_PARAM *, COL_NARROW_PARAM *);
extern const MGACTOR_PARAM lbl_1_rodata_128 = {
    0.0f, 0.0f, 0, 1, fn_1_27BC, 0, 0, 0
};
}

SequenceActor::SequenceActor(Vec *position, s32 param, s32 data)
    : callback(&SequenceActor::fn_1_2710),
      modelStorage(data),
      updateDisabled(0),
      state(1),
      interpolation(0.0f),
      oscillationAngle(0.0f)
{
    MGACTOR_PARAM actorParameters = lbl_1_rodata_128;
    actorParameters.height = 40.0f;
    actorParameters.radius = 40.0f;
    actorParameters.param = param;
    actor = fn_80080B04(&actorParameters, modelStorage);
    actor->gravity = 75.0f;
    fn_80080DD4(actor, 2);
    modelStorage.setLayer(4);
    modelStorage.setAttribute(((1 << 30) | (1 << 1)));
    modelStorage.setShadow();
    this->position.x = position->x;
    this->position.y = position->y;
    this->position.z = position->z;
    fn_80086AA8(actor, &this->position);
}

SequenceActor::~SequenceActor()
{
    fn_80080F38(actor);
}

extern "C" {

s32 fn_1_C774(void *);
}
void SequenceActor::update() {
    Point3d position;
    f32 amplitude;
    f64 angle;
    BOOL state;

    (this->*callback)();
    if (updateDisabled) {
        delete this;
        return;
    }
    fn_80086BC8(actor, &position);
    state = fn_1_2768();
    if (this->state == 0) {
        DebugValue(this->state);
    }
    this->state = state;
    angle = 180.0 *
            (atan2((f64)(position.x - circleCenter.x), (f64)(position.z - circleCenter.z)) / M_PI);
    angle = (f64)(f32)angle;
    oscillationAngle = (f32)((f64)oscillationAngle + framesToSeconds(94.24777960769379));
    interpolation.update();
    amplitude = interpolation.value;
    fn_80086CA8(actor, (f32)(angle + ((f64)amplitude * sin((f64)oscillationAngle))));
    if (modelStorage.isMotionEnd() != 0) {
        modelStorage.setAttribute(((1 << 30) | (1 << 1)));
        modelStorage.setMotionTime(0.0f);
    }
}

void SequenceActor::fn_1_2710()
{
    Point3d position;
    fn_80086BC8(actor, &position);
    if (position.y < -1000.0f) {
        fn_80086AA8(actor, &this->position);
    }
}

BOOL SequenceActor::fn_1_2768()
{
    u32 collisionCode = 0;
    fn_80086DD0(actor, &collisionCode);
    return (collisionCode & 0x407F) != 0;
}

int fn_1_27BC(COL_NARROW_PARAM *request, COL_NARROW_PARAM *other)
{
    std::vector<void *> *array;
    void **current;
    void **end;
    SequenceActor *selected;
    SequenceActor *candidate;

    if (request->type != 1 || other->type != 0) {
        return 0;
    }
    array = fn_1_2264(&ActorSequence::getChecked());
    current = array->begin();
    end = array->end();
    selected = 0;
    while (current != end) {
        candidate = (SequenceActor *)fn_1_5E24(*current);
        if (request->paramB == candidate->actor->param) {
            selected = candidate;
            break;
        } else {
            current++;
        }
    }
    if (selected == 0) {
        return 0;
    }
    selected->modelStorage.resetAttribute(((1 << 30) | (1 << 1)));
    selected->interpolation.set(10.0f, 0.0f, secondsToFrames(1L));
    return 1;
}
