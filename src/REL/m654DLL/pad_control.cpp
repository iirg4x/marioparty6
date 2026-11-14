#include "REL/m654DLL/pad_control.h"

extern "C" {
#include "math.h"
#include "dolphin/mtx.h"
#include "game/mg/actman.h"
}
#include "humath.h"

// Initializes camera-relative steering with no stick movement or buttons.
PadControl::PadControl(unsigned short type, float value)
    : cameraBit((unsigned int)type), range(-(float)M_PI, (float)M_PI, value),
      stickX(0.0f), stickY(0.0f), pressedButtons(0), heldButtons(0)
{}

// Releases the pad control object.
PadControl::~PadControl() {}

// Resets the steering angle, stick movement and button flags.
void PadControl::fn_1_C630(float factor)
{
    range.set(factor);
    stickX = 0.0f;
    stickY = 0.0f;
    pressedButtons = 0;
    heldButtons = 0;
}

// Adds held button flags.
void PadControl::fn_1_C698(u32 flags) {
    heldButtons |= flags;
}

// Adds newly pressed button flags.
void PadControl::fn_1_C6A8(u32 flags) {
    pressedButtons |= flags;
}

// Presses button 0x200.
void PadControl::fn_1_C6B8() {
    pressedButtons |= 0x200;
}

// Presses button 0x100.
void PadControl::fn_1_C6C8() {
    pressedButtons |= 0x100;
}

// Holds button 0x100.
void PadControl::fn_1_C6D8() {
    heldButtons |= 0x100;
}

// Transforms movement into camera space and steers the simulated stick toward it.
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
    stickX = radius * sin(range.get());
    stickY = -radius * cos(range.get());
}

// Clamps and quantizes the stick, submits actor input and damps the remaining movement.
void PadControl::fn_1_C918(MGPLAYER *player)
{
    float normalizedHorizontal;
    float horizontal;
    float normalizedVertical;
    float vertical;
    int padX;
    int padY;

    horizontal = stickX / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (int)(56.0f * normalizedHorizontal);

    vertical = stickY / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (int)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY,
                   pressedButtons, heldButtons | pressedButtons);
    fn_1_CAB4();
}

// Damps stick movement and clears the button flags.
void PadControl::fn_1_CAB4() {
    stickX *= 0.9;
    stickY *= 0.9;
    pressedButtons = 0;
    heldButtons = 0;
}
