#include <new>
#include "dolphin/types.h"
#include "dolphin/pad.h"

extern "C"
{
#include <math.h>
}

extern "C"
{
#include "dolphin/mtx.h"
}

extern "C"
{
#include "dolphin/os.h"
}

#include "Runtime.PPCEABI.H/NMWException.h"

#include "REL/framework/pad_control.h"

extern "C"
{
#include "game/hu3d.h"
}

extern "C"
{
#include "game/charman.h"
}

#include "REL/framework/range.h"
#include "humath.h"

// Initializes the camera-relative input range and clears the pad state.
PadControl::PadControl(u16 cameraBit, f32 value)
    : cameraBit((u32)cameraBit), range(-M_PI, M_PI, value)
{
    stickX = 0.0f;
    stickY = 0.0f;
    pressedButtons = 0;
    heldButtons = 0;
}

// Destroys the pad controller.
PadControl::~PadControl() {}

// Sets the movement angle and clears the pad state.
void PadControl::reset(f32 value)
{
    range.set(value);
    stickX = 0.0f;
    stickY = 0.0f;
    pressedButtons = 0;
    heldButtons = 0;
}

// Adds buttons to the held state.
void PadControl::holdButtons(u32 flags)
{
    heldButtons |= flags;
}

// Adds buttons to the pressed state.
void PadControl::pressButtons(u32 flags)
{
    pressedButtons |= flags;
}

// Adds B to the pressed state.
void PadControl::pressB()
{
    pressedButtons |= PAD_BUTTON_B;
}

// Adds A to the pressed state.
void PadControl::pressA()
{
    pressedButtons |= PAD_BUTTON_A;
}

// Adds A to the held state.
void PadControl::holdA()
{
    heldButtons |= PAD_BUTTON_A;
}

// Turns the input toward movement relative to the camera.
void PadControl::updateMovement(const Vec *movement, f32 amount)
{
    Point3d cameraMovement = {0.0f, 0.0f, 0.0f};
    Point3d cameraPosition;
    Point3d cameraUp;
    Point3d cameraTarget;
    Mtx cameraMatrix;
    f32 angle;
    f32 angleFactor;
    f32 radius;
    f32 adjustedAmount;
    f32 halfAngle;
    f32 absoluteAngle;
    f64 absoluteAngleDouble;
    f32 movementLength;

    cameraMovement.x = movement->x;
    cameraMovement.z = movement->z;
    fn_80027CD0(this->cameraBit, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = this->range.distanceTo(angle) / 2.0f;
    absoluteAngle = __fabsf(halfAngle);
    absoluteAngleDouble = absoluteAngle;
    angleFactor = 1.0 - (absoluteAngleDouble / M_PI);
    movementLength = PSVECMag(&cameraMovement);
    radius = angleFactor * movementLength;
    if (amount < 0.1f)
    {
        adjustedAmount = 0.1f;
    }
    else if (amount > 1.0f)
    {
        adjustedAmount = 1.0f;
    }
    else
    {
        adjustedAmount = amount;
    }
    this->range.step(angle, adjustedAmount);
    this->stickX = radius * sin(this->range.get());
    this->stickY = -radius * cos(this->range.get());
}

// Clamps the stick input and applies the current pad state to the player.
void PadControl::applyToPlayer(MGPLAYER *player)
{
    f32 normalizedHorizontal;
    f32 horizontal;
    f32 normalizedVertical;
    f32 vertical;
    s32 padX;
    s32 padY;

    horizontal = stickX / 10.0f;
    if (horizontal < -1.0f)
    {
        normalizedHorizontal = -1.0f;
    }
    else if (horizontal > 1.0f)
    {
        normalizedHorizontal = 1.0f;
    }
    else
    {
        normalizedHorizontal = horizontal;
    }
    padX = (s32)(56.0f * normalizedHorizontal);

    vertical = stickY / 10.0f;
    if (vertical < -1.0f)
    {
        normalizedVertical = -1.0f;
    }
    else if (vertical > 1.0f)
    {
        normalizedVertical = 1.0f;
    }
    else
    {
        normalizedVertical = vertical;
    }
    padY = (s32)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY, pressedButtons, heldButtons | pressedButtons);
    decay();
}

// Reduces stick input and clears the button state.
void PadControl::decay()
{
    stickX *= 0.9;
    stickY *= 0.9;
    pressedButtons = 0;
    heldButtons = 0;
}
