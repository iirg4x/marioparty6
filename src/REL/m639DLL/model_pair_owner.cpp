#include "REL/framework/event.h"
#include "REL/framework/game_object_man.h"
#include "REL/m639DLL/pair_bounds.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/framework/event_callback_inline.h"
#include <vector>
#include "REL/m639DLL/sphere.h"
#include "REL/m639DLL/line_object.h"

extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
// Model supplies its own conversion to the Hu3D model id.
#include "REL/framework/game_object.h"
#include "REL/framework/sound.h"
extern "C" {
#include "game/data.h"
#include "game/audio.h"
}
#include "REL/m639DLL/player.h"

#include "REL/m639DLL/game_constants.h"

#include "REL/framework/random.h"

struct ModelPairOwner;

struct PairSpawnDefinition {
    unsigned int resource;
    char *firstObject;
    char *secondObject;
};

extern "C" const PairSpawnDefinition lbl_1_rodata_530[4] = { { 0x620008, "fan1null", "null_nabiki1" }, { 0x620008, "fan2null", "null_nabiki2" }, { 0x620009, "fan3null", "null_nabiki3" }, { 0x620009, "fan4null", "null_nabiki4" } };

struct EventPacket;

struct ModelPairOwner : GameObject, EventListener, OwnedModel {
    int selection;
    Vec position;
    float *resourceValue;
    float fanSpeed, modelOpacity, charge;
    unsigned char active : 1;
    int soundHandle;
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
        Vec positions[2];
        model->getObjectPosition(definition->firstObject, &positions[0]);
        model->getObjectPosition(definition->secondObject, &positions[1]);
        new ModelPairOwner(index, definition->resource, positions);
    }
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
        fn_800267CC(*selection, &value.center);
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
      position(*position),
      resourceValue(&getObject("fan_axis")->mesh.base.rot.z),
      fanSpeed(0.0f), modelOpacity(0.0f), charge(0.0f), active(1), soundHandle(-1),
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

/* Releases the two model members and the containing owner. */
ModelPairOwner::~ModelPairOwner() {}

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
    * resourceValue = * resourceValue + fanSpeed;
    double reducedSpeed = (double) fanSpeed - framesToSeconds(2.0);
    storeLowerBoundPairValue(fanSpeed, 0.0, reducedSpeed);
    float clampedProgress = clampPairValue(fanSpeed / 29.0f,
                                           0.0f, 1.0f);
    float easedTransition;
    easePairProgress(clampedProgress, easedTransition);
    setMotionTime((float)(0.6 * easedTransition));
    if (clampedProgress > 0.0f)
    {
        if (soundHandle == - 1)
        {
            soundHandle = soundPlay(0x76C);
        }
        int soundPitch;
        soundPitch = (s16)pairLerp(0, 4000, clampedProgress);
        soundPanTo(1, soundHandle, &position);
        if (soundSetPitch(soundHandle, soundPitch) == 0)
        {
            soundHandle = - 1;
        }
        ASSERT(FALSE);
    }
    else if (soundHandle != - 1)
    {
        HuAudFXStop(soundHandle);
        soundHandle = - 1;
    }
    double reducedOpacity = (double) modelOpacity - 1.0 / (double) secondsToFrames(2L);
    storeLowerBoundPairValue(modelOpacity, 0.0, reducedOpacity);
    secondModel.setMotionSpeed((float)(2.0 * modelOpacity));
    secondModel.setTransparency(modelOpacity);
    if (active && fanSpeed > 0.0f)
    {
        charge = fanSpeed / 29.0f;
        sendEvent(selection, 1, & charge, 0.0f);
    }
}

/* Event zero raises fan speed and model opacity; event two clears the active flag. */
void ModelPairOwner::receive(const EventPacket *event)
{
    switch (event->type) {
    case 2:
        active = 0;
        break;
    case 1:
        break;
    case 0: {
        fanSpeed = Min(29.0f, 0.5f + fanSpeed);

        modelOpacity = (float)Min(1.0, 0.1 + modelOpacity);
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
    values.y = modelOpacity *
               (float)framesToSeconds(0x190);
    values.z = (float)framesToSeconds(0x1F4);
    new LineObject(secondModel.getPosition(&position), &values);
}
