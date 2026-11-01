/* Selects camera motions and updates the minigame camera from its current pose. */
#include <math.h>
#include "REL/m633dll.h"

/* Reads camera 1's position, up vector, and target; ignores the returned up vector, keeps the
 * target, and rebuilds position and up with roll forced to zero. */
void fn_1_73D8(void)
{
    Point3d cameraPosition;
    Point3d cameraTarget;
    Point3d cameraUp;
    Point3d delta;
    Point3d computedCameraPosition;
    Point3d computedCameraTarget;
    Point3d rolledCameraUp;
    Vec cameraUpDirection;
    Point3d cameraDirection;
    f32 cameraYaw;
    f32 cameraPitch;
    f32 cameraRoll;

    lbl_1_bss_0.cameraMotionTime = fn_8002D4E0(lbl_1_bss_0.activeCameraModelId);
    fn_80027CD0(1, &cameraPosition, &cameraUp, &cameraTarget);
    lbl_80266A4C = cameraTarget;
    delta.x = cameraPosition.x - cameraTarget.x;
    delta.y = cameraPosition.y - cameraTarget.y;
    delta.z = cameraPosition.z - cameraTarget.z;
    lbl_80266A40.x = (f32) (180.0*(atan2((-delta.y),
        (sqrtf((delta.x * delta.x) + (delta.z * delta.z)))) / 3.141592653589793));
    lbl_80266A40.y = (f32) (180.0*(atan2(delta.x, delta.z) / 3.141592653589793));
    lbl_80266A40.z = 0.0f;
    lbl_802C028C = sqrtf((delta.z * delta.z) + ((delta.x * delta.x) + (delta.y * delta.y)));
    cameraPitch = lbl_80266A40.x;
    cameraYaw = lbl_80266A40.y;
    cameraRoll = lbl_80266A40.z;
    computedCameraPosition.x = lbl_80266A4C.x
        + (lbl_802C028C * (sin(3.141592653589793*cameraYaw/180.0)
        * cos(3.141592653589793*cameraPitch/180.0)));
    computedCameraPosition.y = lbl_80266A4C.y
        + (lbl_802C028C * -sin(3.141592653589793*cameraPitch/180.0));
    computedCameraPosition.z = lbl_80266A4C.z
        + (lbl_802C028C * (cos(3.141592653589793*cameraYaw/180.0)
        * cos(3.141592653589793*cameraPitch/180.0)));
    computedCameraTarget.x = lbl_80266A4C.x;
    computedCameraTarget.y = lbl_80266A4C.y;
    computedCameraTarget.z = lbl_80266A4C.z;
    cameraUpDirection.x = sin(3.141592653589793*cameraYaw/180.0)
        * sin(3.141592653589793*cameraPitch/180.0);
    cameraUpDirection.y = cos(3.141592653589793*cameraPitch/180.0);
    cameraUpDirection.z = cos(3.141592653589793*cameraYaw/180.0)
        * sin(3.141592653589793*cameraPitch/180.0);
    PSVECSubtract(&computedCameraPosition, &computedCameraTarget, &cameraDirection);
    PSVECNormalize(&cameraDirection, &cameraDirection);
    rolledCameraUp.x =
        cameraUpDirection.x * (cameraDirection.x * cameraDirection.x +
                               (1.0f
                                   - cameraDirection.x * cameraDirection.x)
                                   * cos(3.141592653589793*cameraRoll/180.0)) +
        cameraUpDirection.y * (cameraDirection.x * cameraDirection.y * (1.0f
            - cos(3.141592653589793*cameraRoll/180.0)) -
                               cameraDirection.z * sin(3.141592653589793*cameraRoll/180.0)) +
        cameraUpDirection.z * (cameraDirection.x * cameraDirection.z * (1.0f
            - cos(3.141592653589793*cameraRoll/180.0)) +
                               cameraDirection.y * sin(3.141592653589793*cameraRoll/180.0));
    rolledCameraUp.y =
        cameraUpDirection.y * (cameraDirection.y * cameraDirection.y +
                               (1.0f
                                   - cameraDirection.y * cameraDirection.y)
                                   * cos(3.141592653589793*cameraRoll/180.0)) +
        cameraUpDirection.x * (cameraDirection.x * cameraDirection.y * (1.0f
            - cos(3.141592653589793*cameraRoll/180.0)) +
                               cameraDirection.z * sin(3.141592653589793*cameraRoll/180.0)) +
        cameraUpDirection.z * (cameraDirection.y * cameraDirection.z * (1.0f
            - cos(3.141592653589793*cameraRoll/180.0)) -
                               cameraDirection.x * sin(3.141592653589793*cameraRoll/180.0));
    rolledCameraUp.z =
        cameraUpDirection.z * (cameraDirection.z * cameraDirection.z +
                               (1.0f
                                   - cameraDirection.z * cameraDirection.z)
                                   * cos(3.141592653589793*cameraRoll/180.0)) +
        (cameraUpDirection.x * (cameraDirection.x * cameraDirection.z * (1.0
            - cos(3.141592653589793*cameraRoll/180.0)) -
                                cameraDirection.y * sin(3.141592653589793*cameraRoll/180.0)) +
         cameraUpDirection.y * (cameraDirection.y * cameraDirection.z * (1.0
             - cos(3.141592653589793*cameraRoll/180.0)) +
                                cameraDirection.x * sin(3.141592653589793*cameraRoll/180.0)));
    PSVECNormalize(&rolledCameraUp, &rolledCameraUp);
    Hu3DCameraPosSet(1, computedCameraPosition.x, computedCameraPosition.y,
                     computedCameraPosition.z, rolledCameraUp.x, rolledCameraUp.y, rolledCameraUp.z,
                     computedCameraTarget.x, computedCameraTarget.y, computedCameraTarget.z);
}

/* Starts the indexed camera motion and turns off the previously active camera motion. */
void fn_1_7E70(s32 motionIndex)
{
    s16 loadedCameraMotionId;
    s16 cameraModelId;

    /* Playback starts on the camera model; the separate motion-resource ID read here has no
     * effect. */
    loadedCameraMotionId = lbl_1_bss_0.cameraMotionIds[motionIndex];
    cameraModelId = lbl_1_bss_0.cameraModelIds[motionIndex];
    fn_800284C8(cameraModelId, 1U);
    lbl_1_bss_0.cameraMotionTime = 0.0f;
    lbl_1_bss_0.cameraMotionMaxTime = fn_8002D530(cameraModelId);
    if (lbl_1_bss_0.activeCameraModelId != -1) {
        fn_8002858C(lbl_1_bss_0.activeCameraModelId);
    }
    lbl_1_bss_0.activeCameraModelId = cameraModelId;
    fn_80028478(cameraModelId, 1U);
}

/* Reports whether the active camera motion has reached its end. */
int fn_1_7F40(void)
{
    return fn_8002D75C(lbl_1_bss_0.activeCameraModelId);
}
