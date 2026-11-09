/* Camera-relative sound: per-camera pan ranges, the distance-to-volume curve and the
 * sound-effect calls that use them. */
#include <math.h>

#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/charman.h"


/* One camera's sound projection: the pan range and the horizontal view scale. */
typedef struct CameraSoundParam {
    s32 minimum;
    s32 maximum;
    f32 horizontalScale;
} CameraSoundParam;

/* Distance-to-volume curve shared by every camera: the volume at the far and near
 * distance limits. */
typedef struct SoundVolumeCurve {
    s32 farVolume;
    s32 nearVolume;
    f32 farDistance;
    f32 nearDistance;
} SoundVolumeCurve;


int HuAudFXPlay(int seId);
void HuAudFXStop(int seNo);
int HuAudFXPlayPan(int effectId, s16 pan);
int HuAudFXPlayDelayPan(int effectId, s16 pan, u16 delay);
int HuAudFXPlayVolPan(int soundId, s16 volume, s32 pan);
void HuAudFXPanning(int seNo, s16 pan);
s32 HuAudFXVolSet(s32 handle, s16 volume);

/* Each camera's pan range and view scale; only camera 0 starts configured. */
CameraSoundParam lbl_1_data_570[16] = {{0x20, 0x60, 1.2f}};
/* Volume 110 at 100 units or nearer, falling to 32 at 20000 units. */
SoundVolumeCurve lbl_1_data_630 = {32, 110, 20000.0f, 100.0f};


HU3D_CAMERA *fn_1_8500(short mask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    int i;
    for (i = 0; i < 16; i++, camera++)
        if (mask & (1U << i)) return camera;
    return Hu3DCamera;
}

CameraSoundParam *fn_1_8564(short mask)
{
    CameraSoundParam *param = lbl_1_data_570;
    int i;
    for (i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_570;
}

f32 fn_1_85C8(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                         CameraSoundParam *param)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 width;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);

    tangent = tan(MTXDegToRad(camera->fov / 2.0f));
    depth = tangent * fabs(transformed.z);
    width = depth * param->horizontalScale;
    result->x = transformed.x / width;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

s32 fn_1_8714(f32 value)
{
    SoundVolumeCurve *curveParameters = &lbl_1_data_630;
    f32 curve;
    s32 result;
    if (value < curveParameters->nearDistance) {
        value = curveParameters->nearDistance;
    }
    if (value > curveParameters->farDistance) {
        value = curveParameters->farDistance;
    }

    curve = (f32)(1.0 - (f64)((value - curveParameters->nearDistance) /
                                                (curveParameters->farDistance - curveParameters->nearDistance)));
    curve *= curve;
    result = (s32)((f32)curveParameters->farVolume +
                       curve * (f32)(curveParameters->nearVolume - curveParameters->farVolume));
    return result;
}

s16 fn_1_8808(f32 *fraction, CameraSoundParam *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

int fn_1_88F8(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_8920(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_8948(int farVolume, int nearVolume, float farDistance, float nearDistance)
{
    SoundVolumeCurve *param = &lbl_1_data_630;
    param->farVolume = farVolume;
    param->nearVolume = nearVolume;
    param->nearDistance = nearDistance;
    param->farDistance = farDistance;
}

void fn_1_8974(short mask, int minimum, int maximum, float horizontalScale)
{
    CameraSoundParam *param = lbl_1_data_570;
    int i;
    for (i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) {
            param->minimum = minimum;
            param->maximum = maximum;
            param->horizontalScale = horizontalScale;
            break;
        }
}

void fn_1_89D8(s32 mode)
{
    switch (mode) {
    case 0:
        fn_1_8974(1, 0x20, 0x60, 1.2f);
        break;
    case 1:
        fn_1_8974(3, 0x20, 0x60, 3.2f);
        break;
    case 2:
        fn_1_8974(1, 0x20, 0x40, 0.8f);
        fn_1_8974(2, 0x40, 0x60, 0.8f);
        break;
    case 3:
        fn_1_8974(5, 0x20, 0x40, 1.2f);
        fn_1_8974(0xA, 0x40, 0x60, 1.2f);
        break;
    case 4:
        fn_1_8974(1, 0x30, 0x30, 0.4f);
        fn_1_8974(2, 0x38, 0x38, 0.4f);
        fn_1_8974(4, 0x48, 0x48, 0.4f);
        fn_1_8974(8, 0x50, 0x50, 0.4f);
        break;
    }
}

s32 fn_1_8E28(s16 cameraMask, Point3d *target)
{
    HU3D_CAMERA *camera = fn_1_8500(cameraMask);
    Vec difference;
    PSVECSubtract(&camera->pos, target, &difference);
    return fn_1_8714(PSVECMag(&difference));
}

s16 fn_1_8FAC(s16 cameraMask, Vec *targetPoint)
{
    CameraSoundParam *param = fn_1_8564(cameraMask);
    Vec projected;
    fn_1_85C8(targetPoint, &projected, fn_1_8500(cameraMask), param);
    return fn_1_8808(&projected.x, param);
}

void fn_1_924C(s16 cameraMask, Vec *targetPoint, s32 *volume, s32 *pan)
{
    CameraSoundParam *param = fn_1_8564(cameraMask);
    Vec projected;
    f32 distance = fn_1_85C8(targetPoint, &projected, fn_1_8500(cameraMask), param);
    *volume = (s16)fn_1_8714(distance);
    *pan = fn_1_8808(&projected.x, param);
}

void fn_1_9634(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    HuAudFXPlayPan(effectId, fn_1_8FAC(cameraMask, targetPoint));
}

void fn_1_98FC(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, fn_1_8FAC(cameraMask, targetPoint), delay);
}

static inline s16 fn_1_8714_for_9BCC(f32 value)
{
    SoundVolumeCurve *curveParameters = &lbl_1_data_630;
    f32 curve;
    s16 result;
    if (value < curveParameters->nearDistance) value = curveParameters->nearDistance;
    if (value > curveParameters->farDistance) value = curveParameters->farDistance;
    curve = (f32)(1.0 - (f64)((value - curveParameters->nearDistance) /
                              (curveParameters->farDistance - curveParameters->nearDistance)));
    curve *= curve;
    result = (f32)curveParameters->farVolume +
             curve * (f32)(curveParameters->nearVolume - curveParameters->farVolume);
    return result;
}

static inline void fn_1_924C_for_9BCC(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    CameraSoundParam *param = fn_1_8564(cameraMask);
    Vec projected;
    f32 distance = fn_1_85C8(targetPoint, &projected, fn_1_8500(cameraMask), param);
    *volume = fn_1_8714_for_9BCC(distance);
    *pan = fn_1_8808(&projected.x, param);
}

void fn_1_9BCC(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;
    fn_1_924C_for_9BCC(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

s16 fn_1_9FCC(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan = fn_1_8FAC(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

void fn_1_A2A4(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, fn_1_8FAC(cameraMask, targetPoint));
}

BOOL fn_1_A56C(s16 cameraMask, s32 handle, const Vec *position)
{
    HU3D_CAMERA *camera = fn_1_8500(cameraMask);
    Vec difference;
    s32 volume;
    PSVECSubtract(&camera->pos, position, &difference);
    volume = fn_1_8714(PSVECMag(&difference));
    return HuAudFXVolSet(handle, volume) == 0;
}

void fn_1_A70C(s16 cameraMask, s16 voiceIndex, const Vec *position)
{
    HU3D_CAMERA *camera = fn_1_8500(cameraMask);
    Vec difference;
    s32 volume;
    PSVECSubtract(&camera->pos, position, &difference);
    volume = fn_1_8714(PSVECMag(&difference));
    fn_80060F40(voiceIndex, volume);
}
