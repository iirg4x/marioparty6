/* Camera projection and positional audio panning and volume. */
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "math.h"
#include "game/hu3d.h"
/* Native callers pass a signed halfword to the delayed-pan call and a
 * full word to the combined volume/pan call. */
int HuAudFXPlay(int);
int HuAudFXPlayPan(int, s16);
int HuAudFXPlayVolPan(int, s16, s32);
void HuAudFXPlayDelayPan(int, s16, u16);
void HuAudFXStop(int);
void HuAudFXPanning(int, s16);
s32 HuAudFXVolSet(int, s16);
#include "game/charman.h"

#include "REL/m672DLL/audio_projection.h"


CameraRange lbl_1_data_618[16] = {{32, 96, 1.2f}};
CurveParameters lbl_1_data_6D8 = {32, 110, 20000.0f, 100.0f};


HU3D_CAMERA *fn_1_100E4(short mask)
{
    HU3D_CAMERA *camera;
    int i;
    camera = Hu3DCamera;
    for (i = 0; i < 16; i++, camera++)
        if (mask & (1U << i)) return camera;
    return Hu3DCamera;
}

CameraRange *fn_1_10148(short mask)
{
    CameraRange *param;
    int i;
    param = lbl_1_data_618;
    for (i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_618;
}

f32 fn_1_101AC(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                           CameraRange *scale)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up,
                &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);

    tangent = (f32)tan((f64)(MTXDegToRad(camera->fov / 2.0f)));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * scale->horizontalScale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

s32 fn_1_102F8(f32 value)
{
    CurveParameters *parameters;
    f32 curve;
    s32 result;
    parameters = &lbl_1_data_6D8;
    if (value < parameters->minimum) {
        value = parameters->minimum;
    }
    if (value > parameters->maximum) {
        value = parameters->maximum;
    }

    curve = (f32)(1.0 - (f64)((value - parameters->minimum) / (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s32)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}

s16 fn_1_103EC(f32 *fraction, CameraRange *bounds)
{
    s32 midpoint;
    s32 halfRange;
    s32 value;
    midpoint = (bounds->maximum + bounds->minimum) / 2;
    halfRange = (bounds->maximum - bounds->minimum) / 2;
    value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

int fn_1_104DC(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_10504(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_1052C(int first,int second,float a,float b) { CurveParameters *param=&lbl_1_data_6D8; param->start=first; param->end=second; param->minimum=b; param->maximum=a; }

void fn_1_10558(short mask,int first,int second,float value) { CameraRange *param=lbl_1_data_618; int i; for(i=0;i<16;i++,param++) if(mask & (1U<<i)) {param->minimum=first;param->maximum=second;param->horizontalScale=value;break;} }

void fn_1_105BC(s32 mode)
{
    switch (mode) {
    case 0: fn_1_10558(1, 0x20, 0x60, 1.2f); break;
    case 1: fn_1_10558(3, 0x20, 0x60, 3.2f); break;
    case 2:
        fn_1_10558(1, 0x20, 0x40, 0.8f);
        fn_1_10558(2, 0x40, 0x60, 0.8f);
        break;
    case 3:
        fn_1_10558(5, 0x20, 0x40, 1.2f);
        fn_1_10558(0xA, 0x40, 0x60, 1.2f);
        break;
    case 4:
        fn_1_10558(1, 0x30, 0x30, 0.4f);
        fn_1_10558(2, 0x38, 0x38, 0.4f);
        fn_1_10558(4, 0x48, 0x48, 0.4f);
        fn_1_10558(8, 0x50, 0x50, 0.4f);
        break;
    }
}

s32 fn_1_10A0C(s16 cameraMask, Point3d *target)
{
    HU3D_CAMERA *cameraIterator;
    HU3D_CAMERA *selectedCamera;
    s32 index;
    HU3D_CAMERA *camera;
    Vec difference;
    f32 distance;
    CurveParameters *parameters;
    f32 curve;
    s32 result;
    cameraIterator = Hu3DCamera;
    index = 0;

    for (; index < 0x10; ++index, ++cameraIterator) {
        if ((u32)(cameraMask & (1 << index)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_volume;
        }
    }

    selectedCamera = Hu3DCamera;
camera_selected_for_volume:

    camera = selectedCamera;
    PSVECSubtract(&camera->pos, target, &difference);
    distance = PSVECMag(&difference);
    parameters = &lbl_1_data_6D8;
    if (distance < parameters->minimum) {
        distance = parameters->minimum;
    }
    if (distance > parameters->maximum) {
        distance = parameters->maximum;
    }

    curve = (f32)(1.0 - (f64)((distance - parameters->minimum) / (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s32)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}

s16 fn_1_10B90(s16 cameraMask, Point3d *targetPoint)
{
    CameraRange *parameters;
    Vec projected;
    parameters = fn_1_10148(cameraMask);
    fn_1_101AC(targetPoint, &projected, fn_1_100E4(cameraMask), parameters);
    return fn_1_103EC(&projected.x, parameters);
}

void fn_1_10E30(s16 cameraMask, Point3d *targetPoint, s32 *volume, s32 *pan)
{
    CameraRange *parameters;
    Vec projected;
    f32 distance;
    parameters = fn_1_10148(cameraMask);
    distance = fn_1_101AC(targetPoint, &projected, fn_1_100E4(cameraMask), parameters);
    *volume = (s16)fn_1_102F8(distance);
    *pan = fn_1_103EC(&projected.x, parameters);
}

s32 fn_1_11218(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    return HuAudFXPlayPan(soundId, fn_1_10B90(cameraMask, targetPoint));
}







static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint);
static inline CameraRange *projectionForMask23D98_inlined(s16 cameraMask);
static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result, HU3D_CAMERA *camera, CameraRange *scale);
static inline HU3D_CAMERA *cameraForMask23D98_inlined(s16 cameraMask);
static inline s16 clampPan23D98_inlined(f32 *fraction, CameraRange *bounds);

static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint)
{
    CameraRange *parameters;
    Vec projected;
    parameters = projectionForMask23D98_inlined(cameraMask);
    projectPoint23D98_inlined(targetPoint, &projected, cameraForMask23D98_inlined(cameraMask),
                      parameters);
    return clampPan23D98_inlined(&projected.x, parameters);
}

static inline CameraRange *projectionForMask23D98_inlined(s16 cameraMask)
{
    CameraRange *parameters;
    s32 index;
    parameters = lbl_1_data_618;
    index = 0;
    for (; index < 16; ++index, ++parameters) {
        if ((u32)(cameraMask & (1 << index)) != 0)
            return parameters;
    }
    return lbl_1_data_618;
}

static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, CameraRange *scale)
{
    f32 distance;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;
    Point3d *point = sourcePoint;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, point, &transformed);
    tangent = (f32)tan(MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * scale->horizontalScale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&point, &camera->pos, &transformed);
    distance = PSVECMag(&transformed);
}

static inline HU3D_CAMERA *cameraForMask23D98_inlined(s16 cameraMask)
{
    HU3D_CAMERA *camera;
    s32 index;
    camera = Hu3DCamera;
    index = 0;
    for (; index < 16; ++index, ++camera) {
        if ((u32)(cameraMask & (1 << index)) != 0)
            return camera;
    }
    return Hu3DCamera;
}

static inline s16 clampPan23D98_inlined(f32 *fraction, CameraRange *bounds)
{
    s32 midpoint;
    s32 halfRange;
    s32 value;
    midpoint = (bounds->maximum + bounds->minimum) / 2;
    halfRange = (bounds->maximum - bounds->minimum) / 2;
    value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

void fn_1_114E0(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, cameraPan23D98_inlined(cameraMask, targetPoint), delay);
}

static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan);
static inline CameraRange *RangeGetFor10188_inlined(s16 cameraMask);
static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera, CameraRange *range);
static inline HU3D_CAMERA *CameraGetQueryFor10188_inlined(s16 cameraMask);
static inline s16 fn_1_102F8_for_10188_inlined(f32 value);
static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, CameraRange *bounds);

static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    CameraRange *parameters;
    Vec projected;
    f32 distance;
    parameters = RangeGetFor10188_inlined(cameraMask);
    distance = ProjectVolume10188_inlined(targetPoint, &projected, CameraGetQueryFor10188_inlined(cameraMask), parameters);
    *volume = fn_1_102F8_for_10188_inlined(distance);
    *pan = fn_1_103EC_for_10188_inlined(&projected.x, parameters);
}

static inline CameraRange *RangeGetFor10188_inlined(s16 cameraMask)
{
    CameraRange *range;
    s32 index;
    range = lbl_1_data_618;
    index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_618;
}

static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                     CameraRange *range)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)(MTXDegToRad(camera->fov / 2.0f)));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * range->horizontalScale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline HU3D_CAMERA *CameraGetQueryFor10188_inlined(s16 cameraMask)
{
    HU3D_CAMERA *camera;
    s32 cameraIndex;
    camera = Hu3DCamera;
    cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}

static inline s16 fn_1_102F8_for_10188_inlined(f32 value)
{
    CurveParameters *parameters;
    f32 curve;
    s16 result;
    parameters = &lbl_1_data_6D8;
    if (value < parameters->minimum) value = parameters->minimum;
    if (value > parameters->maximum) value = parameters->maximum;
    curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                          (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (f32)parameters->start + curve * (f32)(parameters->end - parameters->start);
    return result;
}

static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, CameraRange *bounds)
{
    s32 midpoint;
    s32 halfRange;
    s32 value;
    midpoint = (bounds->maximum + bounds->minimum) / 2;
    halfRange = (bounds->maximum - bounds->minimum) / 2;
    value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

void fn_1_117B0(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;
    AudioValues10188_inlined(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}



s16 fn_1_11BB0(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan;
    pan = fn_1_10B90(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

void fn_1_11E88(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, fn_1_10B90(cameraMask, targetPoint));
}

int fn_1_12150(s16 cameraMask, s32 handle, const Vec *position)
{
    HU3D_CAMERA *cameraIterator;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 soundVolume;
    s32 callVolume;
    s32 volume;
    cameraIterator = Hu3DCamera;
    cameraIndex = 0;

    for (; cameraIndex < 16; ++cameraIndex, ++cameraIterator) {
        if ((unsigned int)(cameraMask & (1 << cameraIndex)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_volume;
        }
    }
    selectedCamera = Hu3DCamera;
camera_selected_for_volume:
    camera = selectedCamera;

    PSVECSubtract(&camera->pos, position, &difference);
    distance = PSVECMag(&difference);
    range = &lbl_1_data_6D8;
    if (distance < range->minimum)
        distance = range->minimum;
    if (distance > range->maximum)
        distance = range->maximum;

    weight = (f32)(1.0 - (double)((distance - range->minimum) /
                                  (range->maximum - range->minimum)));
    weight *= weight;
    volume = (s32)(range->start +
                   weight * (f32)(range->end - range->start));
    callVolume = volume;
    soundVolume = callVolume;
    return HuAudFXVolSet(handle, (s16)soundVolume) == 0;
}

void fn_1_122F0(s16 cameraMask, s16 voiceIndex, const Vec *position)
{
    HU3D_CAMERA *cameraIterator;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 voiceVolume;
    s32 callVolume;
    s32 volume;
    cameraIterator = Hu3DCamera;
    cameraIndex = 0;

    for (; cameraIndex < 16; ++cameraIndex, ++cameraIterator) {
        if ((unsigned int)(cameraMask & (1 << cameraIndex)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_volume;
        }
    }
    selectedCamera = Hu3DCamera;
camera_selected_for_volume:
    camera = selectedCamera;
    PSVECSubtract(&camera->pos, position, &difference);
    distance = PSVECMag(&difference);
    range = &lbl_1_data_6D8;
    if (distance < range->minimum)
        distance = range->minimum;
    if (distance > range->maximum)
        distance = range->maximum;

    weight = (f32)(1.0 - (double)((distance - range->minimum) /
                                  (range->maximum - range->minimum)));
    weight *= weight;
    volume = (s32)(range->start +
                   weight * (f32)(range->end - range->start));
    callVolume = volume;
    voiceVolume = callVolume;
    fn_80060F40(voiceIndex, (s16)voiceVolume);
}
