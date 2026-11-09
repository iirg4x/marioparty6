#include "REL/m652DLL/player_functions.h"
#include "REL/m652DLL/float3.h"
int fn_1_2E0C(int value);
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m652DLL/time_conversion.h"
#include "REL/m652DLL/motion_algorithms.h"
template<> double secondsToFrames<double>(double value);
extern "C" {
#include "dolphin.h"
#include "dolphin/types.h"
#include <math.h>
#include "humath.h"
}
#include "REL/framework/model.h"
#include "REL/m652DLL/registered_value.h"
#include "REL/m652DLL/model_child.h"
#include "REL/m652DLL/player_work.h"

/* Coordinates go through an inline unit conversion: the startup code proves one inline call per value. */
static inline float cmToWorld(float centimeters) { return centimeters; }

struct M652Positions {
    Vec values[2];
};
extern "C" {
int lbl_1_bss_48 = fn_1_2178(5);
int lbl_1_bss_4C = secondsToFrames<long>(30);
float lbl_1_bss_50 = framesToSeconds(framesToSeconds(980.665));
M652Positions lbl_1_data_118 = {{{cmToWorld(0.0f), cmToWorld(1005.0f), cmToWorld(-40.0f)},
                               {cmToWorld(0.0f), cmToWorld(962.0f), cmToWorld(-2840.0f)}}};
}

static inline int ScaleHundred(int value) { return value * 100; }

class MotionDirection : public Point3d {
public:
    MotionDirection &normalize() {
        PSVECNormalize(this, this);
        return *this;
    }
    MotionDirection &operator*=(f32 factor) { PSVECScale(this, this, factor); return *this; }

};

static inline void SetDirection2664(MotionDirection &vector, double x, double y, double z)
{
    vector.x = x;
    vector.y = y;
    vector.z = z;
}

extern "C" void fn_1_33A3C__5ModelFP3Vec(void *destination, void *source);

extern "C" {

extern const unsigned char lbl_1_data_178[];
void __dt__6SphereFv(void *object, short destroyMode);
void Replace__5ModelFs(void *object, short destroyMode);
void fn_1_FFA0(void *object, short destroyMode);
void fn_1_14CB8(void *object, short destroyMode);
void __dl__FPv(void *object);
void fn_1_33988__5ModelFUl(void *object, int mode);
}

extern "C" {
void fn_1_347D4__5ModelFPcs(void *, char *, short);
void fn_1_33B5C__5ModelFf(void *, float);

}

PlayerItem::PlayerItem(Racer *player, int modelIndex)
    : GameObject(2), OwnedModel(DATANUM(0x6f0000, 10)), EventListener(),
      callback(&PlayerItem::fn_1_2660), playerWork(player),
      model(DATANUM(0x6f0000, 11)), registered(modelIndex + 2), durationFrames(0)
{
    float modelValue;
    short modelId;
    fn_1_33CE8((unsigned short)(1 << modelIndex));
    fn_1_33CB0(2);
    fn_1_33F98();
    fn_1_33950(((1 << 30) | (1 << 0)));
    fn_1_33950((1 << 0));
    fn_1_33B5C__5ModelFf(&static_cast<OwnedModel &>(*this), 0.0f);
    modelId = (short)model.modelId;
    fn_1_347D4__5ModelFPcs(&static_cast<OwnedModel &>(*this), "itemhook_I", modelId);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    registered.fn_1_DA68(&position);
    modelValue = 75.0f;
    registered.w = modelValue;
    registered.fn_1_DCDC();
    registerChannel(modelIndex + 2);
}

void operator delete(void *);

PlayerItem::~PlayerItem()
{
}

void PlayerItem::update()
{
    (this->*callback)();
    fn_1_33A3C__5ModelFP3Vec(&static_cast<OwnedModel &>(*this), &position);
    registered.fn_1_DA68(&position);
    float increment = registered.w;
    registered.y = registered.y + increment;
}

void PlayerItem::receive(const EventPacket *state)
{
    Vec *cameraPosition;
    Vec *copiedPosition;
    Vec *positionView;
    Vec *positionSource;
    const Vec *selectedPositionData;
    int selectedPosition;
    PlayerItem::Callback defaults;
    float heightLimit;

    if (state->type == 0 && this->playerWork->flags.attributeEnabled == 0) {
        cameraPosition = &this->playerWork->position;
        heightLimit = lbl_1_data_118.values[1].z + cmToWorld(300.0f);
        this->start = *cameraPosition;
        copiedPosition = &this->start;
        positionView = copiedPosition;
        positionSource = positionView;
        this->position = *positionSource;
        if (this->playerWork->flags.collision4 != 0) {
            selectedPosition = 1;
        } else {
            selectedPosition = 0;
        }
        selectedPositionData = &lbl_1_data_118.values[selectedPosition];
        this->destination = *selectedPositionData;
        fn_1_33988(1);
        fn_1_73E4(this->playerWork);
        fn_1_2DB8();
        registered.fn_1_DCF0();
        defaults = &PlayerItem::fn_1_2664;
        this->callback = defaults;
    }
}

void PlayerItem::fn_1_2660()
{
}

void PlayerItem::fn_1_2664()
{
    f32 heightEnd;
    f32 easedProgress;
    f32 movement;
    bool moving;
    Point3d limits = {0.0f, 0.0f, 0.0f};

    limits.x = ScaleHundred(this->start.x < 0.0f ? -8 : 8);
    limits.z = (this->start.z + this->destination.z) / 2.0f;
    movement = (f32)fn_1_2E0C(0x320);
    f32 previousHeight = this->position.y;
    heightEnd = this->position.y + movement;
    const f32 progress = heightEnd < limits.y ?
        (heightEnd - this->start.y) / (limits.y - this->start.y) :
        (heightEnd - limits.y) / (this->destination.y - limits.y);
    MotionDirection direction;
    easedProgress = heightEnd < limits.y ? sqrt((f64)progress) :
        Square2664(Square2664(Square2664(progress)));
    if (heightEnd < limits.y) {
        SetDirection2664(direction, Interpolate2664(this->start.x, limits.x, easedProgress),
            heightEnd, Interpolate2664(this->start.z, limits.z, easedProgress));
    } else {
        SetDirection2664(direction, Interpolate2664(limits.x, this->destination.x, easedProgress),
            heightEnd, Interpolate2664(limits.z, this->destination.z, easedProgress));
    }
    PSVECSubtract(&direction, &this->position, &direction);
    direction.normalize() *= movement;
    PSVECAdd(&direction, &this->position, &this->position);
    if (this->position.y > 0.0f) {
        this->fn_1_33F98();
        this->registered.fn_1_DCF0();
    } else {
        this->fn_1_340D8();
        this->registered.fn_1_DCDC();
    }
    moving = this->position.y >= this->destination.y;
    if (moving != 0) {
        this->position.x = this->destination.x;
        this->position.y = this->destination.y;
        this->position.z = this->destination.z;
        this->setDurationFrames((s32)secondsToFrames(1.5));
    }
    fn_1_2DB8();
    if (moving != 0) {
        fn_1_754C(this->playerWork);
        PlayerItem::Callback next;
        next = &PlayerItem::fn_1_2C78;
        this->callback = next;
    }
}

void PlayerItem::fn_1_2C78()
{
    if (update_countdown()) set_callback(&PlayerItem::fn_1_2CF8);
}

void PlayerItem::fn_1_2CF8()
{
    if ((position.y += (float)fn_1_2E0C(600)) > 2000.0f) {
        registered.fn_1_DCDC();
        OwnedModel::fn_1_33950(1);
        PlayerItem::Callback nextCallback = &PlayerItem::fn_1_2660;
        callback = nextCallback;
    }
}

extern "C" {
Vec *fn_1_344A0__5ModelFPcP3Vec(void *, char *, Vec *);
}

void PlayerItem::fn_1_2DB8()
{
    Vec hookPosition;
    fn_1_33A3C(&position);
    fn_1_344A0__5ModelFPcP3Vec(&static_cast<OwnedModel &>(*this), "itemhook_I", &hookPosition);
    fn_1_74B4(playerWork, &hookPosition);
}

int fn_1_2E0C(int value)
{
    return (int)((1.0 / 60.0) * value);
}

template<> inline double secondsToFrames<double>(double value) { return 60.0 * value; }
