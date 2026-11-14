#include "REL/framework/square.h"
#include <new>
#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/framework/random.h"
#include "REL/framework/event.h"
// The map scene: builds its models and lights, then runs the door opening when the round starts.
#include "REL/m654DLL/model_animation_owner.h"
#include "datadir_enum.h"
#include "REL/framework/scene_shadow.h"
#include "REL/framework/world_units.h"

extern "C" {
#include "game/hu3d.h"
#include "game/mg/actman.h"
}
// One-minute time limit and gravity in world units per frame squared.
int lbl_1_bss_50 = minutesToFrames(1L);
float lbl_1_bss_54 =
    0.016666666666666666 *
    (0.016666666666666666 *
     cmToWorld(980.665f));
Vec lbl_1_data_118[6] = {
    {cmToWorld(-500.0f), cmToWorld(100.0f), cmToWorld(-350.0f)},
    {cmToWorld(500.0f), cmToWorld(100.0f), cmToWorld(-350.0f)},
    {cmToWorld(-500.0f), cmToWorld(100.0f), cmToWorld(750.0f)},
    {cmToWorld(500.0f), cmToWorld(100.0f), cmToWorld(750.0f)},
    {cmToWorld(-850.0f), cmToWorld(800.0f), cmToWorld(-950.0f)},
    {cmToWorld(850.0f), cmToWorld(800.0f), cmToWorld(-950.0f)}
};
#include "REL/m654DLL.h"

#include <cmath>

void fn_1_22C4(void *);

// Builds the scene models, lights, shadow, collision spheres and switch panels.
ModelAnimationOwner::ModelAnimationOwner()
    : callback(&ModelAnimationOwner::fn_1_20A0),
      model(DATANUM(0x710000, 0)),
      secondModel(DATANUM(0x710000, 1)),
      secondaryModel0(DATANUM(0x710000, 2)),
      secondaryModel1(DATANUM(0x710000, 3)),
      angle(0.0f),
      additionalModel(DATANUM(0x710000, 10)),
      spheres(lbl_1_data_118, lbl_1_data_118 + 6),
      elapsed(0)
{
    Vec offset = {0.0f, 0.0f, 0.0f};
    Vec lightDirection;
    Vec lightPosition;
    GXColor lightColor;
    int lights;

    // The map model on layer 1, and the two secondary models, both moved 200 along z; the first is on
    // layer 1 too.
    offset.z = 200.0f;
    model.setLayer(1);
    model.setAttribute(0x40000002);
    secondaryModel0.setLayer(1);
    secondaryModel0.setPosition(&offset);
    secondaryModel1.setPosition(&offset);
    // The additional model, on layer 4.
    additionalModel.setLayer(4);
    additionalModel.setAttribute(0x40000002);
    additionalModel.setAttribute(1);
    // The map model's lights, of which there must be at least one; the first, inf_light1, gives the
    // shadow its direction.
    lights = model.setLightsDynamic(0);

    assert(lights > 0);
    model.setLight("inf_light1", &lightPosition, &lightDirection, &lightColor);
    new SceneShadow(1, &lightDirection, 5.0f, 0x64);
    fn_1_22C4(this);
    fn_1_8600(&secondaryModel0);
}

// Leaves the scene animation state idle.
void ModelAnimationOwner::fn_1_20A0() {}

// Releases the scene models and registered collision spheres.
ModelAnimationOwner::~ModelAnimationOwner() {}

// Restarts the map motion and plays its opening sound.
void ModelAnimationOwner::fn_1_2218()
{
    model.resetAttribute(((1 << 30) | (1 << 1)));
    model.setMotionTime(0.0f);
    soundPlay(0x81D);
}

// Shows and restarts the additional model motion.
void ModelAnimationOwner::fn_1_226C()
{
    additionalModel.resetAttribute(((1 << 30) | (1 << 1)));
    additionalModel.resetAttribute(1);
    additionalModel.setMotionTime(0.0f);
}

// Registers the two collision models and assigns their camera masks.
void fn_1_22C4(void *owner)
{
    short modelIds[2] = {0, 0};
    short firstModelId;
    short secondModelId;
    int index;

    firstModelId = (short)((ModelAnimationOwner *)owner)->secondModel.modelId;
    modelIds[0] = firstModelId;
    secondModelId = (short)((ModelAnimationOwner *)owner)->secondaryModel1.modelId;
    modelIds[1] = secondModelId;
    fn_800804E8(modelIds, 2, 8);
    for (index = 0; index < 2; index++) {
        fn_80080D7C(index, 1 << index);
        fn_800269E8((int)(short)modelIds[index], (1 << 0));
    }
}

// Starts the row reveal or removes the two upper collision spheres.
void ModelAnimationOwner::flagEnabled(int event)
{
    switch (event) {
    case 1:
        fn_1_246C();
        break;
    case 2:
        spheres.resize(4);
        flags &= ~(1UL << event);
        break;
    default:
        flags &= ~(1UL << event);
        break;
    }
}

// Runs the scene animation, rotates both secondary models and advances its frame.
void ModelAnimationOwner::update()
{
    (this->*callback)();
    secondaryModel0.setRotationY(angle);
    secondaryModel1.setRotationY(angle);
    elapsed++;
}

// Schedules the five panel rows and starts the two-second rotation.
void ModelAnimationOwner::fn_1_246C()
{
    int index = 0;

    while (index < 5) {
        sendEvent(index + 5, 0, (float)(1.0 + (double)index / 5.0));
        index++;
    }
    elapsed = 0;
    soundPlay(0x81C);
    void (ModelAnimationOwner::*nextCallback)() = &ModelAnimationOwner::fn_1_254C;
    callback = nextCallback;
}

// Rotates the secondary models with cosine easing and clears phase 1 on completion.
void ModelAnimationOwner::fn_1_254C()
{
    int duration = secondsToFrames<long>(2);
    double quarteredDifference =
        square(1.0 - std::cos((M_PI * (double)elapsed) / (double)duration)) / 4.0;

    angle = (float)(360.0 * quarteredDifference);
    if (elapsed >= duration) {
        angle = 0.0f;
        flags &= ~2U;
        void (ModelAnimationOwner::*nextCallback)() = &ModelAnimationOwner::fn_1_20A0;
        callback = nextCallback;
    }
}
