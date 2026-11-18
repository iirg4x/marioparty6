#include "REL/m628DLL/event_displacement.h"
#include <new>
#include "REL/framework/game_object_man.h"
#include "REL/framework/event_callback_registration.h"
#include "REL/framework/event_callback_adapter_inline.h"
#include "REL/framework/event_callback_inline.h"

#include "REL/framework/game_object.h"

#include "REL/m628DLL/board_positions.h"

#include "REL/framework/model.h"

#include "REL/framework/event_hm.h"

#include "REL/m628DLL/light_setup.h"
#include "REL/m628DLL/player_actor.h"
#include "REL/m628DLL/act_player.h"

#include <functional>

// Classifies a point relative to the current board boundary.
s32 fn_1_128C4(BoardPositions *owner, const Vec &point)
{
    Vec boundary = owner->trackedPositions[0];
    if (point.x < boundary.x)
    {
        if (point.z < boundary.z)
        {
            return 0;
        }
        return 2;
    }
    if (point.z < boundary.z)
    {
        return 1;
    }
    return 3;
}

// Copies actor positions and player lock states into the board tracker.
void fn_1_12934(BoardPositions *owner)
{
    std::vector<GameObject *> primaryEntries;
    s32 primaryLimit;
    s32 secondaryLimit;
    s32 primaryIndex;
    s32 secondaryIndex;
    PlayerActor *primaryEntry;
    ActPlayer *secondaryEntry;
    const Vec *primaryPosition;
    const Vec *actorVelocity;

    collectGameObjects(0x30, &primaryEntries);
    primaryLimit = primaryEntries.size();
    primaryIndex = 0;
    for (; primaryIndex < primaryLimit; primaryIndex++)
    {
        primaryEntry = static_cast<PlayerActor *>(primaryEntries[primaryIndex]);
        primaryPosition = &primaryEntry->getDisplacement();
        static_cast<Vec &>(owner->trackedPositions[primaryIndex]) = *primaryPosition;
        actorVelocity = &primaryEntry->getVelocity();
        owner->velocity = *actorVelocity;
    }
    std::vector<GameObject *> secondaryEntries;
    collectGameObjects(0x40, &secondaryEntries);
    secondaryLimit = secondaryEntries.size();
    secondaryIndex = 0;
    for (; secondaryIndex < secondaryLimit; secondaryIndex++)
    {
        secondaryEntry = static_cast<ActPlayer *>(secondaryEntries[secondaryIndex]);
        static_cast<Vec &>(owner->trackedPositions[secondaryIndex + 1]) =
            secondaryEntry->positionValue();
        owner->lockedFlags[secondaryIndex + 1] = secondaryEntry->isLocked();
    }
}

// Signed red, green, blue and alpha light channels.
struct LightColor
{
    s8 red;
    s8 green;
    s8 blue;
    s8 alpha;
};

// Creates the model light and registers its scene events.
LightSetup::LightSetup(s16 *modelId)
    : GameObject(0x10), firstCallbackOwner(1), secondCallbackOwner(0)
{
    LightColor color = {0, 0, 0, -1};
    this->lightId = fn_80029D24(*(Model *)modelId, "achichi03-light1");
    Hu3DGLightColorSet(
        this->lightId, (u8)color.red, (u8)color.green, (u8)color.blue, (u8)color.alpha);
    registerEventCallback(
        &this->firstCallbackOwner, std::bind1st(std::mem_fun(&LightSetup::fn_1_1305C), this), 0);
    registerEventCallback(
        &this->firstCallbackOwner, std::bind1st(std::mem_fun(&LightSetup::fn_1_130A0), this), 3);
    registerEventCallback(
        &this->secondCallbackOwner, std::bind1st(std::mem_fun(&LightSetup::fn_1_130F4), this), 0);
    this->position.x = 0.0f;
    this->position.y = 0.0f;
    this->position.z = 0.0f;
    this->elapsed = 0.0f;
    this->unk24 = 0;
    this->frame = 0;
    setCallback(&LightSetup::fn_1_1312C);
}

// Releases the light event registrations.
LightSetup::~LightSetup() {}

// Constructs the two positions used to calculate the light direction.
struct LightPositionVector : Vec
{
    LightPositionVector() {}
    LightPositionVector(f64 xValue, f64 yValue, f64 zValue)
    {
        x = xValue;
        y = yValue;
        z = zValue;
    }
};

extern "C"
{
extern Vec lbl_1_data_7F4;
}

// Applies the light position and runs its current flash state.
void LightSetup::update()
{
    LightPositionVector elevatedPosition(position.x, 100.0f, 10.0f + position.z);
    LightPositionVector groundPosition(position.x, 0.0f, position.z);
    Vec direction;
    Vec lightPosition;

    PSVECSubtract(&groundPosition, &elevatedPosition, &direction);
    PSVECNormalize(&direction, &direction);
    PSVECScale(&direction, &lightPosition, -1200.0f);
    PSVECAdd(&lightPosition, &groundPosition, &lightPosition);
    fn_8002A674(&lightPosition, &lbl_1_data_7F4, &groundPosition, 1);
    if (callback != 0)
    {
        (this->*callback)();
    }
    Hu3DGLightPosSet(lightId, position.x, position.y, position.z, 0.0f, 1.0f, 0.0f);
    fn_8002A674(&lightPosition, &lbl_1_data_7F4, &groundPosition, 1);
    frame++;
}

extern "C"
{
Vec lbl_1_data_7F4 = {0.0f, 1.0f, 0.0f};
}

// Starts the light flashing sequence.
void LightSetup::fn_1_1305C(const EventPacket &argument)
{
    setCallback(&LightSetup::fn_1_13130);
}

// Stops the light flashing sequence.
void LightSetup::fn_1_130A0(const EventPacket &argument)
{
    unk2C = 0.3f;
    setCallback(&LightSetup::fn_1_132BC);
}

// Adds the event displacement to the light position.
void LightSetup::fn_1_130F4(const EventPacket &argument)
{
    addEventDisplacement(&position, argument);
}

// Leaves the light unchanged before flashing starts.
void LightSetup::fn_1_1312C() {}

extern "C" const GXColor lbl_1_rodata_384[2] = {{0xFF, 0xFF, 0xFF, 0xFF}, {0, 0, 0, 0xFF}};

// Alternates light colors until the flashing interval ends.
void LightSetup::fn_1_13130()
{
    if (this->elapsed > 1.1f)
    {
        s32 colorIndex = ((s32)this->frame & 7) == 0 ? 0 : 1;

        Hu3DGLightColorSet(this->lightId,
                           lbl_1_rodata_384[colorIndex].r,
                           lbl_1_rodata_384[colorIndex].g,
                           lbl_1_rodata_384[colorIndex].b,
                           lbl_1_rodata_384[colorIndex].a);
        if (this->elapsed > 2.5f)
        {
            Hu3DGLightColorSet(this->lightId,
                               lbl_1_rodata_384[0].r,
                               lbl_1_rodata_384[0].g,
                               lbl_1_rodata_384[0].b,
                               lbl_1_rodata_384[0].a);

            this->setCallback(&LightSetup::fn_1_1312C);
        }
    }
    this->elapsed += (1.0 / 60.0);
}

// Leaves the light unchanged after flashing ends.
void LightSetup::fn_1_132BC() {}
