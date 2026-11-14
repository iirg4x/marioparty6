#include "dolphin/types.h"

extern "C" {
#include <math.h>
}

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "game/mg/actman.h"
}

extern "C" {
#include "game/hu3d.h"
}

extern "C" {
#include "game/charman.h"
}

#include "REL/framework/range.h"

#include "REL/m672DLL/controllers.h"
#include "humath.h"
// Initialize the direction range and clear the accumulated input.
VirtualPad::VirtualPad(u16 cameraType,float initial)
 : type((u32)cameraType), bounds(-3.1415927f,3.1415927f,initial)
{
 horizontal=0.0f; vertical=0.0f; buttonsDown=0; buttons=0;
}
// Destroy the direction range.
VirtualPad::~VirtualPad() {}
// Set the direction value and clear the accumulated input.
void VirtualPad::fn_1_12AB8(f32 value)
{
    bounds.set(value);
    horizontal = 0.0f;
    vertical = 0.0f;
    buttonsDown = 0;
    buttons = 0;
}
// Accumulate held button flags.
void VirtualPad::fn_1_12B20(u32 flags) {
    buttons |= flags;
}
// Accumulate newly pressed button flags.
void VirtualPad::fn_1_12B30(u32 flags) {
    buttonsDown |= flags;
}
// Accumulate button flag 0x200 for this frame.
void VirtualPad::fn_1_12B40() {
    buttonsDown |= 0x200;
}
// Accumulate button flag 0x100 for this frame.
void VirtualPad::fn_1_12B50() {
    buttonsDown |= 0x100;
}
// Accumulate held button flag 0x100.
void VirtualPad::fn_1_12B60() {
    buttons |= 0x100;
}

extern "C" const Point3d lbl_1_rodata_774 = {0.0f, 0.0f, 0.0f};
// Accumulate movement relative to the camera direction.
void VirtualPad::fn_1_12B70(const Vec *movement, f32 amount)
{
    Point3d cameraMovement = lbl_1_rodata_774;
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
    fn_80027CD0(type, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = bounds.distanceTo(angle) / 2.0f;
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
    bounds.step(angle, adjustedAmount);
    horizontal = radius * sin(bounds.get());
    vertical = -radius * cos(bounds.get());
}
// Clamp the accumulated axes, submit player input, and decay the axes.
void VirtualPad::fn_1_12DA0(MGPLAYER *player) {
    float normalizedHorizontal;
    float horizontal;
    float normalizedVertical;
    float vertical;
    int padX;
    int padY;

    horizontal = this->horizontal / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (int)(56.0f * normalizedHorizontal);

    vertical = this->vertical / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (int)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY, buttonsDown,
                   buttons | buttonsDown);
    fn_1_12F3C();
}
// Decay both axes and clear the button flags.
void VirtualPad::fn_1_12F3C() {
    horizontal *= 0.9;
    vertical *= 0.9;
    buttonsDown = 0;
    buttons = 0;
}
