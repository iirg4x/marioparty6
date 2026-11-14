extern "C" {
#include "game/hu3d.h"
}
extern "C" {
#include "game/mg/actman.h"
}
#include "REL/m652DLL/camera_arc_motion.h"
extern "C" {
#include "math.h"
}
#include "humath.h"

/* Initializes pad control for the camera mask and angle. */
PlayerPadControl::PlayerPadControl(u16 cameraBit, f32 angle)
    : cameraBit((u16)cameraBit), angle(-3.1415927f, 3.1415927f, angle),
      horizontalMotion(0.0f), forwardMotion(0.0f), primaryFlags(0), secondaryFlags(0) {}

/* Releases the object through its member and base destructors. */
PlayerPadControl::~PlayerPadControl() {}

/* Resets the control angle, movement components and both pad flag words. */
void fn_1_ABA8(PlayerPadControl *control, f32 value)
{
    control->angle.set(value);
    control->horizontalMotion = 0.0f;
    control->forwardMotion = 0.0f;
    control->primaryFlags = 0;
    control->secondaryFlags = 0;
}

/* Adds bits to the secondary pad flag word. */
void PlayerPadControl::AddSecondaryFlags(u32 flags) {
    secondaryFlags |= flags;
}

/* Adds bits to the primary pad flag word. */
void PlayerPadControl::AddPrimaryFlags(u32 flags) {
    primaryFlags |= flags;
}

/* Sets bit 0x200 in the primary pad flag word. */
void PlayerPadControl::SetPrimaryFlag200() {
    primaryFlags |= 0x200;
}

/* Sets bit 0x100 in the primary pad flag word. */
void PlayerPadControl::fn_1_AC40() {
    primaryFlags |= 0x100;
}

/* Sets bit 0x100 in the secondary pad flag word. */
void PlayerPadControl::fn_1_AC50() {
    secondaryFlags |= 0x100;
}

/* Transforms movement into camera-relative pad motion and updates the control angle. */
void PlayerPadControl::fn_1_AC60(const Vec *movement, f32 amount)
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
    fn_80027CD0(cameraBit, &cameraPosition, &cameraUp, &cameraTarget);
    cameraTarget.y = cameraPosition.y;
    C_MTXLookAt(cameraMatrix, &cameraPosition, &cameraUp, &cameraTarget);
    PSMTXMultVecSR(cameraMatrix, &cameraMovement, &cameraMovement);
    angle = atan2(cameraMovement.x, cameraMovement.z);
    halfAngle = this->angle.distanceTo(angle) / 2.0f;
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
    this->angle.step(angle, adjustedAmount);
    horizontalMotion = radius * sin(this->angle.get());
    forwardMotion = -radius * cos(this->angle.get());
}

/* Clamps normalized movement to pad coordinates and submits both pad flag words. */
void PlayerPadControl::fn_1_AE90(MGPLAYER *player)
{
    f32 normalizedHorizontal;
    f32 horizontal;
    f32 normalizedVertical;
    f32 vertical;
    s32 padX;
    s32 padY;

    horizontal = horizontalMotion / 10.0f;
    if (horizontal < -1.0f) {
        normalizedHorizontal = -1.0f;
    } else if (horizontal > 1.0f) {
        normalizedHorizontal = 1.0f;
    } else {
        normalizedHorizontal = horizontal;
    }
    padX = (s32)(56.0f * normalizedHorizontal);

    vertical = forwardMotion / 10.0f;
    if (vertical < -1.0f) {
        normalizedVertical = -1.0f;
    } else if (vertical > 1.0f) {
        normalizedVertical = 1.0f;
    } else {
        normalizedVertical = vertical;
    }
    padY = (s32)(56.0f * normalizedVertical);

    fn_80080520(player, padX, padY,
                   primaryFlags, secondaryFlags | primaryFlags);
    fn_1_B02C();
}

/* Decays both movement components and clears both pad flag words. */
void PlayerPadControl::fn_1_B02C() {
    horizontalMotion *= 0.9;
    forwardMotion *= 0.9;
    primaryFlags = 0;
    secondaryFlags = 0;
}
