#include "REL/m624DLL/tail_types.h"
extern "C" {
#include "math.h"
}
extern "C" {
#include "dolphin/mtx.h"
}
#include "REL/framework/camera_frame.h"
#include "REL/m624DLL/range.h"

M624PadControl::M624PadControl(u16 itemType, f32 value)
    : type((u32)itemType), range(-3.1415927f, 3.1415927f, value),
      angle_primary(0.0f), angle_secondary(0.0f), flags_primary(0), flags_secondary(0)
{
}

M624PadControl::~M624PadControl() {}

void fn_1_CA90(M624PadControl *control, f32 value)
{
    fn_1_CF60(&control->range, value);
    control->angle_primary = 0.0f;
    control->angle_secondary = 0.0f;
    control->flags_primary = 0;
    control->flags_secondary = 0;
}

void M624PadControl::fn_1_CAF8(u32 flags) {
    flags_secondary |= flags;
}

void M624PadControl::fn_1_CB08(u32 flags) {
    flags_primary |= flags;
}

void M624PadControl::fn_1_CB18() {
    flags_primary |= 0x200;
}

void M624PadControl::fn_1_CB28() {
    flags_primary |= 0x100;
}

void M624PadControl::fn_1_CB38() {
    flags_secondary |= 0x100;
}

f32 fn_1_CF9C(Range<float> *range);

void fn_1_CB48(M624PadControl *self, const Vec *movement, f32 amount)
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
    fn_80027CD0(self->type, &cameraPosition, &cameraUp, &cameraTarget);
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
    self->range.fn_1_9AE0(angle, adjustedAmount);
    self->angle_primary = radius * sin(fn_1_CF9C(&self->range));
    self->angle_secondary = -radius * cos(fn_1_CF9C(&self->range));
}

void fn_1_CF14(M624PadControl *);

void fn_1_CD78(M624PadControl *state, MGPLAYER *player)
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
    fn_1_CF14(state);
}

void fn_1_CF14(M624PadControl *self)
{
    self->angle_primary *= 0.9;
    self->angle_secondary *= 0.9;
    self->flags_primary = 0;
    self->flags_secondary = 0;
}

void fn_1_CF60(Range<float> *self, float value)
{
    self->current = self->wrap(value);
}

float fn_1_CF9C(Range<float> *self)
{
    return self->current;
}
