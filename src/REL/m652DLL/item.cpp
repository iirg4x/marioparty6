#include <new>
#include "REL/m652DLL/player_functions.h"
int fn_1_2E0C(int value);
extern "C" {
#include "game/hu3d.h"
}
#include "REL/framework/frame_time.h"
#include "REL/framework/world_units.h"
#include "REL/m652DLL/motion_algorithms.h"
extern "C" {
#include "dolphin.h"
#include "dolphin/types.h"
#include <math.h>
#include "humath.h"
}
#include "REL/framework/model.h"
#include "REL/m652DLL/model_child.h"
#include "REL/m652DLL/player_work.h"

struct M652Positions {
    Vec values[2];
};
extern "C" {
#include "REL/m652DLL/game_constants.h"
M652Positions lbl_1_data_118 = {{{cmToWorld(0.0f), cmToWorld(1005.0f), cmToWorld(-40.0f)},
                               {cmToWorld(0.0f), cmToWorld(962.0f), cmToWorld(-2840.0f)}}};
}

/* Scales an integer by one hundred. */
static inline int scaleHundred(int value) { return value * 100; }

class MotionDirection : public Point3d {
public:
    /* Normalizes the movement direction in place. */
    MotionDirection &normalize() {
        PSVECNormalize(this, this);
        return *this;
    }
    /* Scales all movement direction components. */
    MotionDirection &operator*=(f32 factor) { PSVECScale(this, this, factor); return *this; }

};

/* Sets all three components of a movement direction. */
static inline void setDirection(MotionDirection &vector, double x, double y, double z)
{
    vector.x = x;
    vector.y = y;
    vector.z = z;
}

/* Creates the item models, attaches the hook and initializes its registered sphere and callback. */
PlayerItem::PlayerItem(Racer *player, int modelIndex)
    : GameObject(2), OwnedModel(DATANUM(0x6f0000, 10)), EventListener(),
      callback(&PlayerItem::fn_1_2660), playerWork(player),
      model(DATANUM(0x6f0000, 11)), registered(modelIndex + 2), durationFrames(0)
{
    float radius;
    short modelId;
    setCamera((unsigned short)(1 << modelIndex));
    setLayer(2);
    setShadow();
    setAttribute(((1 << 30) | (1 << 0)));
    /* Bit zero is set a second time after the combined attribute call. */
    setAttribute((1 << 0));
    setRotationY(0.0f);
    modelId = (short)model.modelId;
    setHook("itemhook_I", modelId);
    position.x = 0.0f;
    position.y = 0.0f;
    position.z = 0.0f;
    registered = position;
    radius = 75.0f;
    registered.radius = radius;
    registered.clearFlag7();
    registerChannel(modelIndex + 2);
}

/* Releases the object through its member and base destructors. */
PlayerItem::~PlayerItem()
{
}

/* Runs the item callback and updates its model position and registered sphere center. */
void PlayerItem::update()
{
    (this->*callback)();
    setPosition(&position);
    registered = position;
    float radius = registered.radius;
    registered.center.y = registered.center.y + radius;
}

/* Selects the destination and starts item movement for event type zero. */
void PlayerItem::receive(const EventPacket *state)
{
    Vec *cameraPosition;
    Vec *copiedPosition;
    Vec *positionView;
    Vec *positionSource;
    const Vec *selectedPositionData;
    int selectedPosition;
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
        resetAttribute(1);
        fn_1_73E4(this->playerWork);
        fn_1_2DB8();
        registered.setFlag7();
        set_callback(&PlayerItem::fn_1_2664);
    }
}

/* Returns without changing state. */
void PlayerItem::fn_1_2660()
{
}

/* Moves the item along its interpolated arc and changes its callback when the movement ends. */
void PlayerItem::fn_1_2664()
{
    f32 heightEnd;
    f32 easedProgress;
    f32 movement;
    bool moving;
    Point3d limits = {0.0f, 0.0f, 0.0f};

    limits.x = scaleHundred(this->start.x < 0.0f ? -8 : 8);
    limits.z = (this->start.z + this->destination.z) / 2.0f;
    movement = (f32)fn_1_2E0C(0x320);
    f32 previousHeight = this->position.y;
    heightEnd = this->position.y + movement;
    const f32 progress = heightEnd < limits.y ?
        (heightEnd - this->start.y) / (limits.y - this->start.y) :
        (heightEnd - limits.y) / (this->destination.y - limits.y);
    MotionDirection direction;
    easedProgress = heightEnd < limits.y ? sqrt((f64)progress) :
        square(square(square(progress)));
    if (heightEnd < limits.y) {
        setDirection(direction, interpolateValue(this->start.x, limits.x, easedProgress),
            heightEnd, interpolateValue(this->start.z, limits.z, easedProgress));
    } else {
        setDirection(direction, interpolateValue(limits.x, this->destination.x, easedProgress),
            heightEnd, interpolateValue(limits.z, this->destination.z, easedProgress));
    }
    PSVECSubtract(&direction, &this->position, &direction);
    direction.normalize() *= movement;
    PSVECAdd(&direction, &this->position, &this->position);
    if (this->position.y > 0.0f) {
        this->setShadow();
        this->registered.setFlag7();
    } else {
        this->resetShadow();
        this->registered.clearFlag7();
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
        set_callback(&PlayerItem::fn_1_2C78);
    }
}

/* Waits for the item countdown to expire before starting its upward movement. */
void PlayerItem::fn_1_2C78()
{
    if (update_countdown()) set_callback(&PlayerItem::fn_1_2CF8);
}

/* Raises the item and hides its model and registered sphere after it passes height 2000. */
void PlayerItem::fn_1_2CF8()
{
    if ((position.y += (float)fn_1_2E0C(600)) > 2000.0f) {
        registered.clearFlag7();
        OwnedModel::setAttribute(1);
        set_callback(&PlayerItem::fn_1_2660);
    }
}

/* Reads the item hook position and moves the player to that hook. */
void PlayerItem::fn_1_2DB8()
{
    Vec hookPosition;
    setPosition(&position);
    getObjectPosition("itemhook_I", &hookPosition);
    fn_1_74B4(playerWork, &hookPosition);
}

/* Converts an integer frame count to whole seconds. */
int fn_1_2E0C(int value)
{
    return (int)((1.0 / 60.0) * value);
}
