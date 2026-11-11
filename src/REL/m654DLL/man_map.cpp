#include "REL/framework/frame_time.h"
#include "REL/framework/sound.h"
#include "REL/m654DLL/framework_calls.h"
// The map scene: builds its models and lights, then runs the door opening when the round starts.
#include "REL/m654DLL/model_animation_owner.h"
#include "datadir_enum.h"
#include "REL/m654DLL/camera_parameters.h"
#include "REL/framework/scene_shadow.h"
#include "humath.h"
#include "REL/framework/world_units.h"

extern "C" {
#include "game/hu3d.h"
#include "REL/m654DLL/pad_control.h"
#include "game/mg/actman.h"
int lbl_1_bss_50 = minutesToFrames(1L);
float lbl_1_bss_54 = 0.016666666666666666 * (0.016666666666666666 * cmToWorld(980.665f));
Vec lbl_1_data_118[6] = {
    {cmToWorld(-500.0f), cmToWorld(100.0f), cmToWorld(-350.0f)},
    {cmToWorld(500.0f), cmToWorld(100.0f), cmToWorld(-350.0f)},
    {cmToWorld(-500.0f), cmToWorld(100.0f), cmToWorld(750.0f)},
    {cmToWorld(500.0f), cmToWorld(100.0f), cmToWorld(750.0f)},
    {cmToWorld(-850.0f), cmToWorld(800.0f), cmToWorld(-950.0f)},
    {cmToWorld(850.0f), cmToWorld(800.0f), cmToWorld(-950.0f)}
};
}
void fn_1_22C4(void *);
extern "C" {
void fn_1_8600(Model *);
}
extern "C" {

double cos(double);
}
void *operator new(unsigned long);

ModelAnimationOwner::ModelAnimationOwner()
    : callback(&ModelAnimationOwner::fn_1_20A0),
      animation(DATANUM(0x710000, 0)),
      secondAnimation(DATANUM(0x710000, 1)),
      secondaryAnimation0(DATANUM(0x710000, 2)),
      secondaryAnimation1(DATANUM(0x710000, 3)),
      angle(0.0f),
      additionalAnimation(DATANUM(0x710000, 10)),
      values(lbl_1_data_118, lbl_1_data_118 + 6),
      elapsed(0)
{
    Vec scale = {0.0f, 0.0f, 0.0f};
    Vec lightDirection;
    Vec lightPosition;
    GXColor lightColor;
    int lights;

    // The map model on layer 1, and the two secondary models, both moved 200 along z; the first is on
    // layer 1 too.
    scale.z = 200.0f;
    animation.setLayer(1);
    animation.setAttribute(0x40000002);
    secondaryAnimation0.setLayer(1);
    secondaryAnimation0.setPosition(&scale);
    secondaryAnimation1.setPosition(&scale);

    // The additional model, on layer 4.
    additionalAnimation.setLayer(4);
    additionalAnimation.setAttribute(0x40000002);
    additionalAnimation.setAttribute(1);

    // The map model's lights, of which there must be at least one; the first, inf_light1, gives the
    // shadow its direction.
    lights = animation.setLightsDynamic(0);
    assert(lights > 0);
    animation.setLight("inf_light1", &lightPosition, &lightDirection, &lightColor);
    new SceneShadow(1, &lightDirection, 5.0f, 0x64);
    fn_1_22C4(this);
    fn_1_8600(&secondaryAnimation0);
}

void ModelAnimationOwner::fn_1_20A0() {}

ModelAnimationOwner::~ModelAnimationOwner() {}

void ModelAnimationOwner::fn_1_2218()
{
    animation.resetAttribute(((1 << 30) | (1 << 1)));
    animation.setMotionTime(0.0f);
    soundPlay(0x81D);
}

void ModelAnimationOwner::fn_1_226C()
{
    additionalAnimation.resetAttribute(((1 << 30) | (1 << 1)));
    additionalAnimation.resetAttribute(1);
    additionalAnimation.setMotionTime(0.0f);
}

void fn_1_22C4(void *owner)
{
    short modelIds[2] = {0, 0};
    short firstModelId;
    short secondModelId;
    int index;

    firstModelId = (short)((ModelAnimationOwner *)owner)->secondAnimation.modelId;
    modelIds[0] = firstModelId;
    secondModelId = (short)((ModelAnimationOwner *)owner)->secondaryAnimation1.modelId;
    modelIds[1] = secondModelId;
    fn_800804E8(modelIds, 2, 8);
    for (index = 0; index < 2; index++) {
        fn_80080D7C(index, 1 << index);
        fn_800269E8((int)(short)modelIds[index], (1 << 0));
    }
}

template<class T> inline T Square(const T &value) { return value * value; }

void ModelAnimationOwner::flagEnabled(int event)
{
    switch (event) {
    case 1:
        fn_1_246C();
        break;
    case 2:
        values.resize(4);
        flags &= ~(1UL << event);
        break;
    default:
        flags &= ~(1UL << event);
        break;
    }
}

void ModelAnimationOwner::update()
{
    (this->*callback)();
    secondaryAnimation0.setRotationY(angle);
    secondaryAnimation1.setRotationY(angle);
    elapsed++;
}

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

void ModelAnimationOwner::fn_1_254C()
{
    int duration = secondsToFrames<long>(2);
    double quarteredDifference =
        Square(1.0 - cos((M_PI * (double)elapsed) / (double)duration)) / 4.0;

    angle = (float)(360.0 * quarteredDifference);
    if (elapsed >= duration) {
        angle = 0.0f;
        flags &= ~2U;
        void (ModelAnimationOwner::*nextCallback)() = &ModelAnimationOwner::fn_1_20A0;
        callback = nextCallback;
    }
}
