#include "dolphin/types.h"

#include "dolphin/mtx.h"

extern "C" {
#include "math.h"
}

#include "REL/framework/linknode.h"

#include "REL/framework/range.h"

#include "humath.h"

/* REL/m634DLL/pad_control.h: the items this file uses */

/* CA90/CF60/CD78 consume a Range<float> at +0x4, followed by two angles and two
 * pad-state words.  The leading link is a consumed word in the owner view;
 * it is kept named rather than hidden as padding. */
#include "REL/m634DLL/pad_control.h"

PadControl::PadControl(u16 mask, f32 value)
    : cameraBit((u32)mask), range(-3.1415927f, 3.1415927f, value),
      angle_primary(0.0f), angle_secondary(0.0f), flags_primary(0), flags_secondary(0)
{}

PadControl::~PadControl() {}

void fn_1_15DBC(PadControl *state, f32 value)
{
    state->range.set(value);
    state->angle_primary = 0.0f;
    state->angle_secondary = 0.0f;
    state->flags_primary = 0;
    state->flags_secondary = 0;
}

void PadControl::fn_1_15E24(u32 flags) {
    flags_secondary |= flags;
}

void PadControl::fn_1_15E34(u32 flags) {
    flags_primary |= flags;
}

void PadControl::fn_1_15E44() {
    flags_primary |= 0x200;
}

void PadControl::fn_1_15E54() {
    flags_primary |= 0x100;
}

void PadControl::fn_1_15E64() {
    flags_secondary |= 0x100;
}

void fn_1_15E74(PadControl *, const Vec *, f32);

void fn_1_15E74(PadControl *self, const Vec *movement, f32 amount)
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

    fn_80080520(player, padX, padY,
                   state->flags_primary, state->flags_secondary | state->flags_primary);
    state->fn_1_16240();
}

void PadControl::fn_1_16240() {
    angle_primary *= 0.9;
    angle_secondary *= 0.9;
    flags_primary = 0;
    flags_secondary = 0;
}
