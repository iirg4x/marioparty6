#include "REL/m639DLL/pair_bounds.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/event_callback_inline.h"
#include <vector>
#include "REL/m639DLL/sphere.h"
#include "REL/m639DLL/line_object.h"
/* Releases the two model members and the containing owner. */

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/framework/game_object.h"
#include "REL/framework/sound.h"
extern "C" {
#include "game/data.h"
#include "REL/m639DLL.h"
#include "game/audio.h"
}

extern "C" {

int lbl_1_bss_E0 = minutesToFrames(5L);
int lbl_1_bss_E4 = secondsToFrames(30L);
float lbl_1_bss_E8 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
}

float RandomFloat(float scale);

struct ModelPairOwner;
typedef ModelPairOwner PairOwnerState;

extern "C" {
void fn_1_15DE0(void *, short);
void *fn_1_1AAF8(void *, short);
}
void operator delete(void *);
void *operator new(unsigned long);

struct PairSpawnDefinition {
    unsigned int resource;
    char *firstObject;
    char *secondObject;
};

extern "C" const PairSpawnDefinition lbl_1_rodata_530[4] = { { 0x620008, "fan1null", "null_nabiki1" }, { 0x620008, "fan2null", "null_nabiki2" }, { 0x620009, "fan3null", "null_nabiki3" }, { 0x620009, "fan4null", "null_nabiki4" } };

struct EventPacket;

struct PairPosition {
    float components[3];
};

struct ModelPairOwner : GameObject, EventListener, OwnedModel {
    int selection;
    PairPosition position;
    float *resourceValue;
    float scaleX, scaleY, scaleZ;
    unsigned char active : 1;
    unsigned char inactive : 7;
    int state;
    OwnedModel secondModel;
    ModelPairOwner(int modelIndex, unsigned int resource, Vec *position);

    virtual ~ModelPairOwner();
    void update();
    void receive(const EventPacket *event);
    void fn_1_E82C();
};

/* Creates four pair owners at the model objects named by the spawn table. */
void fn_1_DE1C(Model *model)
{
    for (int index = 0; index < 4; index++) {
        const PairSpawnDefinition *definition = &lbl_1_rodata_530[index];
        Vec secondPosition;
        Vec firstPosition;
        model->getObjectPosition(definition->firstObject, &firstPosition);
        model->getObjectPosition(definition->secondObject, &secondPosition);
        new ModelPairOwner(index, definition->resource, &firstPosition);
    }
}

extern "C" {

void fn_1_1F230(void *, short);
}

void collectGameObjects(int category, std::vector<GameObject *> *list);

extern "C" {
void fn_1_DECC(std::vector<Sphere> *spheres);
}

static inline short modelIdOf(Model *model)
{
    return *model;
}

/* Applies the shared position transform to the selected model pair. */
void fn_1_DECC(std::vector<Sphere> *spheres)
{
    std::vector<GameObject *> selected;
    int index;
    int loopLimit;
    ModelPairOwner *selection;

    collectGameObjects(2, &selected);
    loopLimit = selected.size();
    spheres->clear();
    index = 0;
    while (index < loopLimit) {
        Sphere value;
        selection = static_cast<ModelPairOwner *>(selected[index]);
        fn_800267CC(modelIdOf(selection), &value.center);
        {
            float radius = 220.0f;
            value.radius = radius;
        }
        spheres->push_back(value);
        index += 1;
    }
}

/* Builds the two positioned models and initializes their animation state. */
ModelPairOwner::ModelPairOwner(int modelIndex, unsigned int resource, Vec *position)
    : GameObject(2), OwnedModel(resource), selection(modelIndex + 1),
      position(*(const PairPosition *)position),
      resourceValue(&getObject("fan_axis")->mesh.base.rot.z),
      scaleX(0.0f), scaleY(0.0f), scaleZ(0.0f), active(1), state(-1),
      secondModel(DATANUM(0x620000, 6))
{
    registerChannel(modelIndex + 1);
    registerChannel(0);
    setLayer(4);
    setAttribute(((1 << 30) | (1 << 1)));
    setShadow();
    setPosition(position);
    setRotationY(-45.0f);
    secondModel.setLayer(4);
    secondModel.setAttribute(((1 << 30) | (1 << 0)));
    secondModel.setPosition(position + 1);
    secondModel.setRotationY(-45.0f);
    secondModel.setShadowMap();
    secondModel.setMotionTime(RandomFloat(secondModel.getMotionMaxTime()));
}

ModelPairOwner::~ModelPairOwner() {}

void sendEvent(int channel, unsigned long type, void *context, float delay);

static inline double pairTransitionProgress(float progress)
{
    if (0.0 > ((double)progress - 0.5)) {
        return 0.0;
    }
    return (double)progress - 0.5;
}

static inline void easePairProgress(float progress, float &eased)
{
    double transition = pairTransitionProgress(progress);
    eased = (float)(2.0 * transition);
}

template <class T> inline T pairLerp(T from, T to, double t)
{
    return (1.0 - t) * from + t * to;
}
/* Advances the model pair's transition and updates its position sound. */
void ModelPairOwner::update()
{
    * resourceValue = * resourceValue + scaleX;
    double reducedScale = (double) scaleX - framesToSeconds(2.0);
    storeLowerBoundPairValue(scaleX, 0.0, reducedScale);
    float clampedProgress = clampPairValue(scaleX / 29.0f,
                                           0.0f, 1.0f);
    float easedTransition;
    easePairProgress(clampedProgress, easedTransition);
    setMotionTime((float)(0.6 * easedTransition));
    if (clampedProgress > 0.0f)
    {
        if (state == - 1)
        {
            state = soundPlay(0x76C);
        }
        int soundDuration;
        soundDuration = (s16)pairLerp(0, 4000, clampedProgress);
        soundPanTo(1, state, (Point3d *) & position);
        if (soundSetPitch(state, soundDuration) == 0)
        {
            state = - 1;
        }
        ASSERT(soundDuration >= 0);
    }
    else if (state != - 1)
    {
        HuAudFXStop(state);
        state = - 1;
    }
    double reducedVertical = (double) scaleY - 1.0 / (double) secondsToFrames(2L);
    storeLowerBoundPairValue(scaleY, 0.0, reducedVertical);
    secondModel.setMotionSpeed((float)(2.0 * scaleY));
    secondModel.setTransparency(scaleY);
    if (active && scaleX > 0.0f)
    {
        scaleZ = scaleX / 29.0f;
        sendEvent(selection, 1, & scaleZ, 0.0f);
    }
}

/* Advances the object's position in state zero and clears its active flag in state two. */
void ModelPairOwner::receive(const EventPacket *event)
{
    switch (event->type) {
    case 2:
        active = 0;
        break;
    case 1:
        break;
    case 0: {
        float horizontal = 0.5f + scaleX;
        float adjustedHorizontal;
        adjustedHorizontal = (29.0f < horizontal) ? 29.0f : horizontal;
        float storedHorizontal = adjustedHorizontal;
        scaleX = storedHorizontal;

        double vertical = 0.1 + scaleY;
        double adjustedVertical;
        adjustedVertical = (1.0 < vertical) ? 1.0 : vertical;
        double storedVertical = adjustedVertical;
        scaleY = (float)storedVertical;
        fn_1_E82C();
    }
    }
}

// Builds the component's scaled position record and attaches it to the model.
void ModelPairOwner::fn_1_E82C()
{
    Vec values = { 0.0f, 0.0f, 0.0f };
    Vec position;
    values.x = (float)framesToSeconds(-0x1F4);
    values.y = scaleY *
               (float)framesToSeconds(0x190);
    values.z = (float)framesToSeconds(0x1F4);
    new LineObject(secondModel.getPosition(&position), &values);
}
