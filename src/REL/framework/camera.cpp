// Camera frame helpers used by minigame camera controls.
static const char rcsid[] = "$Id: camera.cpp,v 1.31.2.2 2004/04/16 11:22:07 shohyama Exp $";

extern "C" {
#include "dolphin/math.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
}

// Stores the eye, up direction, and look target used to orient a minigame camera.
struct CameraFrame {
    Vec eye;    // Camera position in world space.
    Vec up;     // Camera's up direction.
    Vec target; // World-space point the camera looks toward.
};

void cameraExtractBasis(CameraFrame *, Vec *, Vec *, Vec *);
void cameraTranslate(CameraFrame *, Vec *);

// Minigame camera controls call this to rotate the view and up direction around world Y, then the
// current camera-right axis. It also sets target one unit from eye, replacing the previous
// distance.
void cameraRotateAroundRightAndWorldYAxis(CameraFrame *camera, float rightAxisAngle,
                                          float worldYAxisAngle)
{
    Vec right;
    Vec up;
    Vec lookAtZAxis;
    Mtx axisRotation;
    Mtx yRotation;
    Mtx rotation;

    cameraExtractBasis(camera, &right, &up, &lookAtZAxis);
    PSMTXRotAxisRad(axisRotation, &right, 0.017453292f * rightAxisAngle);
    PSMTXRotRad(yRotation, 'Y', 0.017453292f * worldYAxisAngle);
    PSMTXConcat(axisRotation, yRotation, rotation);
    PSMTXMultVecSR(rotation, &lookAtZAxis, &lookAtZAxis);
    PSMTXMultVecSR(rotation, &up, &camera->up);
    PSVECSubtract(&camera->eye, &lookAtZAxis, &camera->target);
}

// Minigame camera controls call this to set the view's tilt relative to the horizon. It also sets
// target one unit from eye, replacing the previous distance.
void cameraTiltToAngle(CameraFrame *camera, float tiltDegrees)
{
    Vec right;
    Vec up;
    Vec lookAtZAxis;
    Mtx rotation;

    cameraExtractBasis(camera, &right, &up, &lookAtZAxis);
    tiltDegrees = 0.017453292f * tiltDegrees;
    float lookAtZAxisXLengthSquared = lookAtZAxis.x * lookAtZAxis.x;
    float lookAtZAxisZLengthSquared = lookAtZAxis.z * lookAtZAxis.z;
    tiltDegrees -= atan2(-lookAtZAxis.y,
                         sqrt(lookAtZAxisXLengthSquared + lookAtZAxisZLengthSquared));

    PSMTXRotAxisRad(rotation, &right, tiltDegrees);
    PSMTXMultVecSR(rotation, &lookAtZAxis, &lookAtZAxis);
    PSMTXMultVecSR(rotation, &up, &camera->up);
    PSVECSubtract(&camera->eye, &lookAtZAxis, &camera->target);
}

// Minigame camera controls call this to roll the view around the eye-to-target axis.
void cameraRollAroundViewAxis(CameraFrame *camera, float rollDegrees)
{
    Vec axis;
    Mtx rotation;
    PSVECSubtract(&camera->target, &camera->eye, &axis);
    PSMTXRotAxisRad(rotation, &axis, 0.017453292f * rollDegrees);
    PSMTXMultVec(rotation, &camera->up, &camera->up);
}

// Camera tilt helpers call this to rebuild an up direction with no vertical right-axis component.
void cameraRebuildUpVector(CameraFrame *camera)
{
    Vec right;
    Vec up;
    Vec lookAtZAxis;
    cameraExtractBasis(camera, &right, &up, &lookAtZAxis);
    // Keep the right axis horizontal before deriving the up direction.
    right.y = 0.0f;
    PSVECCrossProduct(&lookAtZAxis, &right, &camera->up);
}

// Camera rotation helpers call this to get the current right, up, and look-at Z axes.
void cameraExtractBasis(CameraFrame *camera, Vec *right, Vec *up, Vec *lookAtZAxis)
{
    Mtx lookAt;
    C_MTXLookAt(lookAt, &camera->eye, &camera->up, &camera->target);
    right->x = lookAt[0][0];
    right->y = lookAt[0][1];
    right->z = lookAt[0][2];
    up->x = lookAt[1][0];
    up->y = lookAt[1][1];
    up->z = lookAt[1][2];
    lookAtZAxis->x = lookAt[2][0];
    lookAtZAxis->y = lookAt[2][1];
    lookAtZAxis->z = lookAt[2][2];
}

// Minigame camera code calls this to invert the look-at matrix into camera-to-world space; inverse
// failure is ignored.
void cameraInverseLookAt(const CameraFrame *camera, Mtx matrix)
{
    C_MTXLookAt(matrix, &camera->eye, &camera->up, &camera->target);
    PSMTXInverse(matrix, matrix);
}

// Minigame camera code calls this to shift a frame by a camera-local offset; inverse failure is
// ignored.
void cameraTranslateByLocalOffset(CameraFrame *camera, Vec *localOffset)
{
    Mtx matrix;
    Vec transformedPosition;
    C_MTXLookAt(matrix, &camera->eye, &camera->up, &camera->target);
    PSMTXInverse(matrix, matrix);
    // Rotate a camera-local offset into world space, then move the eye and target by that amount.
    PSMTXMultVecSR(matrix, localOffset, &transformedPosition);
    cameraTranslate(camera, &transformedPosition);
}

// Camera position helpers call this to move the eye and target together.
void cameraTranslate(CameraFrame *camera, Vec *translation)
{
    PSVECAdd(translation, &camera->target, &camera->target);
    PSVECAdd(translation, &camera->eye, &camera->eye);
}

// Minigame setup calls this to initialize a frame at the default eye, up, and target.
void cameraSetDefaultFrame(CameraFrame *camera)
{
    camera->up.x = 0.0f;
    camera->up.y = 1.0f;
    camera->up.z = 0.0f;
    camera->eye.x = 0.0f;
    camera->eye.y = 0.0f;
    camera->eye.z = 5000.0f;
    camera->target.x = 0.0f;
    camera->target.y = 0.0f;
    camera->target.z = 0.0f;
}

// Tracks an object's position and movement for camera-follow behavior.
struct CameraPositionState {
    Vec position;                  // Last tracked world position.
    unsigned char reserved[12];    // Unused bytes retained in the tracking state.
    Vec accumulatedOffset;          // Accumulated movement added during camera tracking updates.
};

// Camera tracking calls this when an object's world position changes.
void cameraUpdateTrackedPosition(CameraPositionState *trackedState, Vec *newPosition)
{
    Vec positionChange;
    PSVECSubtract(newPosition, &trackedState->position, &positionChange);
    trackedState->position.x = newPosition->x;
    trackedState->position.y = newPosition->y;
    trackedState->position.z = newPosition->z;
    PSVECAdd(&trackedState->accumulatedOffset, &positionChange, &trackedState->accumulatedOffset);
}

// Minigame camera transitions call this to blend the eye and carry the target by the same movement.
void cameraBlendEyePosition(CameraFrame *camera, const Vec *startPosition, const Vec *endPosition,
                            float blendAmount)
{
    Vec startContribution;
    Vec blendedPosition;
    Vec eyeMovement;
    PSVECScale(startPosition, &startContribution, 1.0f - blendAmount);
    PSVECScale(endPosition, &blendedPosition, blendAmount);
    PSVECAdd(&startContribution, &blendedPosition, &blendedPosition);
    PSVECSubtract(&blendedPosition, &camera->eye, &eyeMovement);
    camera->eye.x = blendedPosition.x;
    camera->eye.y = blendedPosition.y;
    camera->eye.z = blendedPosition.z;
    PSVECAdd(&camera->target, &eyeMovement, &camera->target);
}

// Minigame camera code calls this to copy a frame into each HUD camera selected by the mask.
void cameraApplyToHud(CameraFrame *camera, s32 cameraMask)
{
    Hu3DCameraPosSetV(cameraMask, &camera->eye, &camera->up, &camera->target);
}

// Minigame camera code calls this to read the first HUD camera selected by the mask.
void cameraReadFromHud(CameraFrame *camera, s32 cameraMask)
{
    Hu3DCameraPosGet(cameraMask, &camera->eye, &camera->up, &camera->target);
}
