#include "REL/m654DLL/pad_control.h"

extern "C" {
#include "math.h"
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
}
#include "humath.h"
#include "REL/framework/range.h"

PadControl::PadControl(unsigned short type, float value)
    : cameraBit((unsigned int)type), range(-3.1415927f, 3.1415927f, value),
      angle_primary(0.0f), angle_secondary(0.0f), flags_primary(0), flags_secondary(0)
{}

PadControl::~PadControl() {}

void fn_1_C630(PadControl *object, float factor)
{
    object->range.set(factor);
    object->angle_primary = 0.0f;
    object->angle_secondary = 0.0f;
    object->flags_primary = 0;
    object->flags_secondary = 0;
}

void PadControl::fn_1_C698(u32 flags) {
    flags_secondary |= flags;
}

void PadControl::fn_1_C6A8(u32 flags) {
    flags_primary |= flags;
}

void PadControl::fn_1_C6B8() {
    flags_primary |= 0x200;
}

void PadControl::fn_1_C6C8() {
    flags_primary |= 0x100;
}

void PadControl::fn_1_C6D8() {
    flags_secondary |= 0x100;
}

void PadControl::fn_1_C6E8(const Vec *movement, f32 amount)
{
    Vec cameraMovement = {0.0f, 0.0f, 0.0f};
    Vec cameraPosition;
    Vec cameraUp;
    Vec cameraTarget;
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
    fn_80027CD0(cameraBit, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = range.distanceTo(angle) / 2.0f;
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
    range.step(angle, adjustedAmount);
    angle_primary = radius * sin(range.get());
    angle_secondary = -radius * cos(range.get());
}

void PadControl::fn_1_C918(MGPLAYER *player)
{
    float normalizedHorizontal;
    float horizontal;
    float normalizedVertical;
    float vertical;
    int padX;
    int padY;

    horizontal = angle_primary / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (int)(56.0f * normalizedHorizontal);

    vertical = angle_secondary / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (int)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY,
                   flags_primary, flags_secondary | flags_primary);
    fn_1_CAB4();
}

void PadControl::fn_1_CAB4() {
    angle_primary *= 0.9;
    angle_secondary *= 0.9;
    flags_primary = 0;
    flags_secondary = 0;
}
