#include "dolphin/types.h"

#include "dolphin/mtx.h"

extern "C" {
#include "math.h"
}

#include "REL/framework/linknode.h"

#include "REL/framework/range.h"

#include "humath.h"

#include "REL/m634DLL/pad_control.h"

// Initializes the camera mask, control range, angles, and flags.
PadControl::PadControl(u16 mask, f32 value)
    : cameraBit((u32)mask)
    , range(-3.1415927f, 3.1415927f, value)
    , angle_primary(0.0f)
    , angle_secondary(0.0f)
    , flags_primary(0)
    , flags_secondary(0)
{
}

// Releases the object and its member-owned resources.
PadControl::~PadControl() { }

// Resets the control range, angles, and button flags.
void fn_1_15DBC(PadControl *state, f32 value)
{
    state->range.set(value);
    state->angle_primary = 0.0f;
    state->angle_secondary = 0.0f;
    state->flags_primary = 0;
    state->flags_secondary = 0;
}

// Adds secondary button flags.
void PadControl::fn_1_15E24(u32 flags)
{
    flags_secondary |= flags;
}

// Adds primary button flags.
void PadControl::fn_1_15E34(u32 flags)
{
    flags_primary |= flags;
}

// Sets primary button flag 0x200.
void PadControl::fn_1_15E44()
{
    flags_primary |= 0x200;
}

// Sets primary button flag 0x100.
void PadControl::fn_1_15E54()
{
    flags_primary |= 0x100;
}

// Sets secondary button flag 0x100.
void PadControl::fn_1_15E64()
{
    flags_secondary |= 0x100;
}

void fn_1_15E74(PadControl *, const Vec *, f32);

// Converts world movement to camera-relative control angles.
void fn_1_15E74(PadControl *self, const Vec *movement, f32 amount)
{
    Point3d cameraMovement = { 0.0f, 0.0f, 0.0f };
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
    fn_80027CD0(self->cameraBit, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = self->range.distanceTo(angle) / 2.0f;
    absoluteAngle = __fabsf(halfAngle);
    absoluteAngleDouble = absoluteAngle;
    angleFactor = 1.0 - (absoluteAngleDouble / M_PI);
    movementLength = PSVECMag(&cameraMovement);
    radius = angleFactor * movementLength;
    if (amount < 0.1f) {
        adjustedAmount = 0.1f;
    } else if (amount > 1.0f) {
        adjustedAmount = 1.0f;
    } else {
        adjustedAmount = amount;
    }
    self->range.step(angle, adjustedAmount);
    self->angle_primary = radius * sin(self->range.get());
    self->angle_secondary = -radius * cos(self->range.get());
}

// Applies the control angles and button flags to the player input.
void fn_1_160A4(PadControl *state, MGPLAYER *player)
{
    float normalizedHorizontal;
    float horizontal;
    float normalizedVertical;
    float vertical;
    int padX;
    int padY;

    horizontal = state->angle_primary / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (int)(56.0f * normalizedHorizontal);

    vertical = state->angle_secondary / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (int)(56.0f * normalizedVertical);

    fn_80080520(
        player, padX, padY, state->flags_primary, state->flags_secondary | state->flags_primary);
    state->fn_1_16240();
}

// Damps the control angles and clears the button flags.
void PadControl::fn_1_16240()
{
    angle_primary *= 0.9;
    angle_secondary *= 0.9;
    flags_primary = 0;
    flags_secondary = 0;
}
