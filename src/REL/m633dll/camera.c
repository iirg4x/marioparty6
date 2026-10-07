/* Selects camera motions and updates the minigame camera from its current pose. */
#include "REL/m633dll.h"

/* Updates camera 1 from the current camera-motion pose, including its position, aim, and roll direction. */
void fn_1_73D8(void)
{
    Point3d cameraPosition;
    Point3d cameraTarget;
    Point3d cameraUp;
    Point3d delta;
    Point3d computedCameraPosition;
    Point3d computedCameraTarget;
    Point3d rolledCameraUp;
    Vec dir;
    Point3d cameraDirection;
    f32 cameraYaw;
    f32 cameraPitch;
    f32 cameraRoll;

    lbl_1_bss_0.cameraMotionTime = Hu3DMotionTimeGet(lbl_1_bss_0.activeCameraModelId);
    Hu3DCameraPosGet(1, &cameraPosition, &cameraUp, &cameraTarget);
    Center = cameraTarget;
    delta.x = cameraPosition.x - cameraTarget.x;
    delta.y = cameraPosition.y - cameraTarget.y;
    delta.z = cameraPosition.z - cameraTarget.z;
    CRot.x = (f32) HuAtan(-delta.y, sqrtf((delta.x * delta.x) + (delta.z * delta.z)));
    CRot.y = (f32) HuAtan(delta.x, delta.z);
    CRot.z = 0.0f;
    CZoom = sqrtf((delta.z * delta.z) + ((delta.x * delta.x) + (delta.y * delta.y)));
    cameraPitch = CRot.x;
    cameraYaw = CRot.y;
    cameraRoll = CRot.z;
    computedCameraPosition.x = Center.x + (CZoom * (HuSin(cameraYaw) * HuCos(cameraPitch)));
    computedCameraPosition.y = Center.y + (CZoom * -HuSin(cameraPitch));
    computedCameraPosition.z = Center.z + (CZoom * (HuCos(cameraYaw) * HuCos(cameraPitch)));
    computedCameraTarget.x = Center.x;
    computedCameraTarget.y = Center.y;
    computedCameraTarget.z = Center.z;
    dir.x = HuSin(cameraYaw) * HuSin(cameraPitch);
    dir.y = HuCos(cameraPitch);
    dir.z = HuCos(cameraYaw) * HuSin(cameraPitch);
    PSVECSubtract(&computedCameraPosition, &computedCameraTarget, &cameraDirection);
    PSVECNormalize(&cameraDirection, &cameraDirection);
    rolledCameraUp.x = dir.x * (cameraDirection.x * cameraDirection.x + (1.0f - cameraDirection.x * cameraDirection.x) * HuCos(cameraRoll))
        + dir.y * (cameraDirection.x * cameraDirection.y * (1.0f - HuCos(cameraRoll)) - cameraDirection.z * HuSin(cameraRoll))
        + dir.z * (cameraDirection.x * cameraDirection.z * (1.0f - HuCos(cameraRoll)) + cameraDirection.y * HuSin(cameraRoll));
    rolledCameraUp.y = dir.y * (cameraDirection.y * cameraDirection.y + (1.0f - cameraDirection.y * cameraDirection.y) * HuCos(cameraRoll))
        + dir.x * (cameraDirection.x * cameraDirection.y * (1.0f - HuCos(cameraRoll)) + cameraDirection.z * HuSin(cameraRoll))
        + dir.z * (cameraDirection.y * cameraDirection.z * (1.0f - HuCos(cameraRoll)) - cameraDirection.x * HuSin(cameraRoll));
    rolledCameraUp.z = dir.z * (cameraDirection.z * cameraDirection.z + (1.0f - cameraDirection.z * cameraDirection.z) * HuCos(cameraRoll))
        + (dir.x * (cameraDirection.x * cameraDirection.z * (1.0 - HuCos(cameraRoll)) - cameraDirection.y * HuSin(cameraRoll))
        + dir.y * (cameraDirection.y * cameraDirection.z * (1.0 - HuCos(cameraRoll)) + cameraDirection.x * HuSin(cameraRoll)));
    PSVECNormalize(&rolledCameraUp, &rolledCameraUp);
    Hu3DCameraPosSet(1, computedCameraPosition.x, computedCameraPosition.y, computedCameraPosition.z, rolledCameraUp.x, rolledCameraUp.y, rolledCameraUp.z, computedCameraTarget.x, computedCameraTarget.y, computedCameraTarget.z);
}

/* Starts the indexed camera motion and turns off the previously active camera motion. */
void fn_1_7E70(s32 motionIndex)
{
    s16 priorMotionIndex;
    s16 motionId;

    priorMotionIndex = lbl_1_bss_0.cameraMotionIds[motionIndex];
    motionId = lbl_1_bss_0.cameraModelIds[motionIndex];
    Hu3DCameraMotionStart(motionId, 1U);
    lbl_1_bss_0.cameraMotionTime = 0.0f;
    lbl_1_bss_0.cameraMotionMaxTime = Hu3DMotionMaxTimeGet(motionId);
    if (lbl_1_bss_0.activeCameraModelId != -1) {
        Hu3DCameraMotionOff(lbl_1_bss_0.activeCameraModelId);
    }
    lbl_1_bss_0.activeCameraModelId = motionId;
    Hu3DCameraMotionOn(motionId, 1U);
}

/* Reports whether the active camera motion has reached its end. */
int fn_1_7F40(void)
{
    return Hu3DMotionEndCheck(lbl_1_bss_0.activeCameraModelId);
}
