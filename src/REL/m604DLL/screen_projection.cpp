#include "dolphin/mtx.h"
#include "humath.h"
extern "C" {
#include "math.h"
#include "game/hu3d.h"
#include "game/audio.h"
}

extern "C" void fn_80060F14(s16 voiceId, s16 pan);

void fn_1_15CC8(Point3d *position, s16 cameraMask,
                            Point3d *screenPosition)
{
    Mtx viewMatrix;
    Point3d cameraPosition;
    HU3D_CAMERA *camera;
    f32 horizontalScale;
    f32 verticalScale;
    s16 index;

    for (index = 0; index < 16; index++) {
        if ((cameraMask & (1 << index)) != 0) {
            break;
        }
    }
    camera = &Hu3DCamera[index];
    C_MTXLookAt(viewMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(viewMatrix, position, &cameraPosition);

    horizontalScale = (f32)(1.2f *
        ((f64)cameraPosition.z *
         (sin(M_PI *
              (camera->fov / 2.0f) /
              180.0) /
          cos(M_PI *
              (camera->fov / 2.0f) /
              180.0))));
    verticalScale = (f32)((f64)cameraPosition.z *
        (sin(M_PI *
             (camera->fov / 2.0f) /
             180.0) /
         cos(M_PI *
             (camera->fov / 2.0f) /
             180.0)));

    screenPosition->x = 288.0f +
        cameraPosition.x * (288.0f / -horizontalScale);
    screenPosition->y = 240.0f +
        cameraPosition.y * (240.0f / verticalScale);
    screenPosition->z = 0.0f;
}

s32 fn_1_15F28(s32 soundId, Point3d *position)
{
    Point3d projectedPosition;
    HU3D_CAMERA *cameraTable;
    f32 panScale;
    f32 distance;
    f32 volumeScale;
    s16 pan;
    s16 volume;
    s32 soundHandle;

    cameraTable = Hu3DCamera;
    fn_1_15CC8(position, 1, &projectedPosition);
    panScale = projectedPosition.x / 640.0f;
    pan = (s16)(32.0f + (64.0f * panScale));
    PSVECSubtract(position, &cameraTable[0].pos, position);
    distance = PSVECMag(position);
    volumeScale = (3000.0f - distance) / 3000.0f;
    volume = 0;
    if (volumeScale > 0.0f) {
        volume = (s16)(104.0f + (23.0f * volumeScale));
    }
    soundHandle = HuAudFXPlay(soundId);
    HuAudFXPanning(soundHandle, pan);
    HuAudFXVolSet(soundHandle, volume);
    return soundHandle;
}

void fn_1_16298(s16 voiceId, Point3d *position)
{
    Point3d projectedPosition;
    f32 panScale;
    s16 pan;

    fn_1_15CC8(position, 1, &projectedPosition);
    panScale = projectedPosition.x / 640.0f;
    pan = (s16)(32.0f + (64.0f * panScale));
    fn_80060F14(voiceId, pan);
}
