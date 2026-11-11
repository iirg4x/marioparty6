#include "REL/m639DLL/lighting_types.h"
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
// The shadow fit, the sprite quads and the lighthouse lights that follow the camera.
#include "REL/m639DLL/transition.h"
#include <vector>
#include <cmath>
#include "REL/framework/game_object.h"
#include "REL/framework/camera_frame.h"
#include "REL/framework/scene_shadow.h"
#include "REL/m639DLL/mode_game.h"
#include "REL/m639DLL/mode_selection.h"
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/model.h"
#include "REL/m639DLL/resource_parameters.h"
#include "REL/m639DLL/sphere.h"
#include "REL/m639DLL/range.h"

extern "C" {
#include "dolphin/gx.h"
#include "REL/m639DLL.h"
}
#include "humath.h"

void collectGameObjects(int category, std::vector<GameObject *> *list);

#include "REL/m639DLL/lighthouse_light.h"

extern "C" {
extern const GXVtxDescList lbl_1_rodata_430[];
extern const GXVtxAttrFmtList lbl_1_rodata_448[];
extern float lbl_1_bss_C8;
extern char *lbl_1_data_508[4];
extern AnimationResourcePair lbl_1_data_5B8[5];
void __msl_assertion_failed(const char *, const char *, int);
int HuSprTexLoad(void *, short, int, int, int, int);
void fn_1_DECC(std::vector<Sphere> *positions);
void fn_1_C28C(Vec *position);
void fn_1_C858();
void fn_1_CD58();
void fn_1_D69C(LighthouseLight *owner);

void fn_1_111DC(void *object);
}

// Starts each selected component's transition and restores its initial values.
void fn_1_CD58()
{
    std::vector<GameObject *> sequence;
    collectGameObjects(4, &sequence);
    int loopLimit = sequence.size();
    for (int index = 0; index < loopLimit; index++) {
        LighthouseLight *owner = static_cast<LighthouseLight *>(sequence[index]);
        owner->transition.fn_1_50AC(0.99f, (int)secondsToFrames(0.2));
        void (LighthouseLight::*next)() = &LighthouseLight::fn_1_D38C;
        owner->callback = next;
    }
}

// Reports whether every selected component has reached its transition value.
unsigned char fn_1_CE60()
{
    std::vector<GameObject *> sequence;
    collectGameObjects(4, &sequence);
    int loopLimit = sequence.size();
    for (int index = 0; index < loopLimit; index++) {
        LighthouseLight *component = static_cast<LighthouseLight *>(sequence[index]);
        if (!component->fn_1_DA30()) {
            return 0;
        }
    }
    return 1;
}

// The model nodes each light follows: the lighthouses' lamps, then the five pairs of street lights.
extern "C" char *lbl_1_data_508[4] = {
    "l_toudai_null", "r_toudai_null", "toudailight_l03", "toudailight_r03"
};
extern "C" AnimationResourcePair lbl_1_data_5B8[5] = {
    {{"dentyulight_l01", "dentyulight_r01"}},
    {{"dentyulight_l02", "dentyulight_r02"}},
    {{"dentyulight_l03", "dentyulight_r03"}},
    {{"dentyulight_l04", "dentyulight_r04"}},
    {{"dentyulight_l05", "dentyulight_r05"}}
};

// Builds the selected animation resource and its five per-resource entries.
LighthouseLight::LighthouseLight(unsigned int dataNum, int selector)
    : GameObject(4), OwnedModel(dataNum), callback(&LighthouseLight::fn_1_D23C), selector(selector), parameter(getMotionMaxTime()),
      transition(1.0f),
      accumulatedAngle(getObject(lbl_1_data_508[selector])->mesh.base.rot.y),
      angleCosine(&getObject(lbl_1_data_508[selector])->mesh.base.scale.x),
      cameraState(&getObject((lbl_1_data_508 + selector)[2])->mesh.base),
      phase(0.0f)
{
    setLayer(3);
    setAttribute(((1 << 30) | (1 << 1)));
    setMotionTime(parameter);
    children.clear();
    for (int index = 0; index < 5; ++index) {
        HSF_TRANSFORM *entry = &getObject(lbl_1_data_5B8[index].resources[selector])->mesh.base;
        children.push_back(entry);
    }
}

void LighthouseLight::fn_1_D23C() {}

LighthouseLight::~LighthouseLight() {}

// Runs the bound callback, applies the scaled motion speed, and updates the object.
void LighthouseLight::update()
{
    (this->*callback)();
    setMotionTime(parameter * sampledScalar());
    fn_1_D69C(this);
}

// Advances the embedded transition and restores its initial values when the component is complete.
void LighthouseLight::fn_1_D38C()
{
    transition.fn_1_5124();
    if (transition.elapsed > transition.frames) {
        if (!fn_1_DA30()) {
            transition.fn_1_50AC(0.0f, (int)secondsToFrames(0.8));
            return;
        }
        void (LighthouseLight::*next)() = &LighthouseLight::fn_1_D44C;
        callback = next;
    }
}

// Samples the selected value and installs the completion transition.
void LighthouseLight::fn_1_D44C()
{
    unsigned char nearEnd;
    double value;

    value = ModeGame::getChecked().fn_1_2B0(selector);
    nearEnd = 0;
    if (value > 0.99 && value < 1.0) {
        nearEnd = 1;
    }
    transition.fn_1_50AC((float)(nearEnd ? 0.99 : value), 0);
    if (value >= 1.0) {
        if (selector == M639StateOwner::getChecked().fn_1_2CAC()) {
            transition.fn_1_50F0(0.99f, 1.0f, (int)secondsToFrames(0.5));
        } else {
            transition = 0.99f;
        }
        void (LighthouseLight::*next)() = &LighthouseLight::fn_1_D670;
        callback = next;
    }
}

void LighthouseLight::fn_1_D670()
{
    transition.fn_1_5124();
}

void fn_1_D69C(LighthouseLight *owner)
{
    Vec cameraPosition;
    Vec cameraTarget;
    Vec cameraUp;
    float angle;
    int index;
    fn_80027CD0(1, & cameraPosition, & cameraUp, & cameraTarget);
    int count;
    owner->getObjectPosition(lbl_1_data_508[owner->selector], & cameraTarget);
    angle = (float)(180.0 * (std::atan2((double)(cameraTarget.x - cameraPosition.x), (double)(cameraTarget.z - cameraPosition.z)) / M_PI));
    owner->cameraState->rot.y = angle;
    owner->accumulatedAngle += (float) framesToSeconds(0x3C);
    Range<float> angularRange(0.0f, 180.0f, owner->accumulatedAngle);
    float angle2 = (float) std::fabs(angularRange.fn_1_DA68(angle));
    * owner->angleCosine = (float) std::cos((M_PI * (double) angle2) / 180.0);
    float ratio = (float)((double)angle2 / 90.0);
    owner->cameraState->scale.x = owner->cameraState->scale.y = owner->cameraState->scale.z = (float)(2.0 + 10.0 * (double)squared(squared(ratio)));
    count = owner->children.size();
    for (index = 0; index < count; ++index)
    {
        float &x = owner->children[index]->scale.x;
        float &y = owner->children[index]->scale.y;
        float &z = owner->children[index]->scale.z;
        float angle = (float)((double)owner->phase + (((M_PI * 360.0 / 180.0) * (double)index) / (double)count));
        x = y = z = (float)(2.0 + 0.3 * std::sin(angle));
    }
    owner->phase = (float)((double)owner->phase + framesToSeconds(M_PI));
}

// Reports equality after sampling the scalar once.
bool LighthouseLight::fn_1_DA30() const
{
    float value = transition.current;
    return 0.0 == value;
}

extern "C" {

int lbl_1_bss_D0 = minutesToFrames(5L);
int lbl_1_bss_D4 = secondsToFrames(30L);
float lbl_1_bss_D8 = (1.0 / 60.0) * ((1.0 / 60.0) * cmToWorld(980.665f));
}
