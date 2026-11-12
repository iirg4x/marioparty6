#include "REL/m671DLL/tail_types.h"
#include "humath.h"
#include "math.h"

AngleWork::AngleWork(u16 type, f32 value)
    : type((u32)type), range(-3.1415927f, 3.1415927f, value), angle_primary(0.0f),
      angle_secondary(0.0f), flags_primary(0), flags_secondary(0)
{
}

AngleWork::~AngleWork() {}

void fn_1_11490(AngleWork *self, f32 value) {
    self->range.set(value);
    self->angle_primary = 0.0f;
    self->angle_secondary = 0.0f;
    self->flags_primary = 0;
    self->flags_secondary = 0;
}

void AngleWork::fn_1_114F8(u32 flags) {
    flags_secondary |= flags;
}

void AngleWork::fn_1_11508(u32 flags) {
    flags_primary |= flags;
}

void AngleWork::fn_1_11518() {
    flags_primary |= 0x200;
}

void AngleWork::fn_1_11528() {
    flags_primary |= 0x100;
}

void AngleWork::fn_1_11538() {
    flags_secondary |= 0x100;
}

void fn_1_11548(AngleWork *self, const Vec *movement, f32 amount)
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
    self->range.step(angle, adjustedAmount);
    self->angle_primary = radius * sin(self->range.get());
    self->angle_secondary = -radius * cos(self->range.get());
}

void fn_1_11778(AngleWork *state, MGPLAYER *player)
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
    state->fn_1_11914();
}

void AngleWork::fn_1_11914() {
    angle_primary *= 0.9;
    angle_secondary *= 0.9;
    flags_primary = 0;
    flags_secondary = 0;
}
