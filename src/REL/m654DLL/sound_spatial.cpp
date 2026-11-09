#include "REL/m654DLL/sound_spatial.h"
extern "C" {
#include "math.h"
}
extern "C" {
#include "game/hu3d.h"
}
#include "REL/m654DLL/camera_parameters.h"
extern "C" {
#include "dolphin/mtx.h"
}
extern "C" {
#include "dolphin/types.h"
}
extern "C" {
#include "game/charman.h"
}
extern "C" {
#include "game/memory.h"
}
/* Camera-relative sound projection, panning and volume. */

extern "C" {
int HuAudFXPlay(int);
void HuAudFXStop(int);
int HuAudFXPlayPan(int, s16);
int HuAudFXPlayDelayPan(int, s16, u16);
int HuAudFXPlayVolPan(int, s16, s32);
void HuAudFXPanning(int, s16);
s32 HuAudFXVolSet(int, s16);
s32 HuAudFXPitchSet(int, s16);
}
struct IntegerBounds { s32 minimum, maximum; };
struct CurveParameters { s32 start, end; f32 maximum, minimum; };
extern "C" {
CameraParam lbl_1_data_6D0[16] = {{0x20, 0x60, 1.2f}};
CurveParameters lbl_1_data_790 = {32, 110, 20000.0f, 100.0f};

}

HU3D_CAMERA *fn_1_9C5C(short mask) { HU3D_CAMERA *camera=Hu3DCamera; for(int i=0;i<16;i++,camera++) if(mask & (1U<<i)) return camera; return Hu3DCamera; }

CameraParam *fn_1_9CC0(short mask) { CameraParam *param=lbl_1_data_6D0; for(int i=0;i<16;i++,param++) if(mask & (1U<<i)) return param; return lbl_1_data_6D0; }

void fn_1_9D24(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                           CameraParam *scale)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;
    f32 z;
    f32 absoluteZ;
    f32 magnitude;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);

    tangent = (f32)tan((f64)(MTXDegToRad(camera->fov / 2.0f)));
    z = transformed.z;
    absoluteZ = __fabsf(z);
    magnitude = absoluteZ;
    depth = (f32)((f64)tangent * (f64)magnitude);
    scaledDepth = depth * scale->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}

s32 fn_1_9E70(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_790;
    if (value < parameters->minimum) {
        value = parameters->minimum;
    }
    if (value > parameters->maximum) {
        value = parameters->maximum;
    }

    f32 curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                                (parameters->maximum - parameters->minimum)));
    curve *= curve;
    s32 result = (s32)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}

s16 fn_1_9F64(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value;
    s32 selectedValue;
    s32 upperValue;

    if ((value = (s32)((f32)midpoint + (f32)halfRange * *fraction)) < bounds->minimum) {
        selectedValue = bounds->minimum;
    }
    else {
        if (value > bounds->maximum) {
            upperValue = bounds->maximum;
        } else {
            upperValue = value;
        }
        selectedValue = upperValue;
    }
    return (s16)selectedValue;
}

int fn_1_A054(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_A07C(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_A0A4(int first,int second,float a,float b) { CurveParameters *param=&lbl_1_data_790; param->start=first; param->end=second; param->minimum=b; param->maximum=a; }

void fn_1_A0D0(short mask,int first,int second,float value) { CameraParam *param=lbl_1_data_6D0; for(int i=0;i<16;i++,param++) if(mask & (1U<<i)) {param->first=first;param->second=second;param->value=value;break;} }

void fn_1_A134(s32 mode)
{
    switch (mode) {
    case 0: {
        CameraParam *view0 = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view0) {
            if ((u32)((1 << index) & 1) != 0) {
                view0->first = 0x20;
                view0->second = 0x60;
                view0->value = 1.2f;
                return;
            }
        }
        return;
    }
    case 1: {
        CameraParam *view1 = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view1) {
            if ((u32)((1 << index) & 3) != 0) {
                view1->first = 0x20;
                view1->second = 0x60;
                view1->value = 3.2f;
                return;
            }
        }
        return;
    }
    case 2:
    {
        CameraParam *view2a = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view2a) {
            if ((u32)((1 << index) & 1) != 0) {
                view2a->first = 0x20;
                view2a->second = 0x40;
                view2a->value = 0.8f;
                break;
            }
        }
        CameraParam *view2b = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view2b) {
            if ((u32)((1 << index) & 2) != 0) {
                view2b->first = 0x40;
                view2b->second = 0x60;
                view2b->value = 0.8f;
                return;
            }
        }
        return;
    }
    case 3:
    {
        CameraParam *view3a = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view3a) {
            if ((u32)((1 << index) & 5) != 0) {
                view3a->first = 0x20;
                view3a->second = 0x40;
                view3a->value = 1.2f;
                break;
            }
        }
        CameraParam *view3b = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view3b) {
            if ((u32)((1 << index) & 0xA) != 0) {
                view3b->first = 0x40;
                view3b->second = 0x60;
                view3b->value = 1.2f;
                return;
            }
        }
        return;
    }
    case 4:
    {
        CameraParam *view4a = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view4a) {
            if ((u32)((1 << index) & 1) != 0) {
                view4a->first = 0x30;
                view4a->second = 0x30;
                view4a->value = 0.4f;
                break;
            }
        }
        CameraParam *view4b = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view4b) {
            if ((u32)((1 << index) & 2) != 0) {
                view4b->first = 0x38;
                view4b->second = 0x38;
                view4b->value = 0.4f;
                break;
            }
        }
        CameraParam *view4c = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view4c) {
            if ((u32)((1 << index) & 4) != 0) {
                view4c->first = 0x48;
                view4c->second = 0x48;
                view4c->value = 0.4f;
                break;
            }
        }
        CameraParam *view4d = &lbl_1_data_6D0[0];
        for (s32 index = 0; index < 16; ++index, ++view4d) {
            if ((u32)((1 << index) & 8) != 0) {
                view4d->first = 0x50;
                view4d->second = 0x50;
                view4d->value = 0.4f;
                return;
            }
        }
    }
    }
}

s32 fn_1_A584(s16 cameraMask, Point3d *target)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    s32 index = 0;

    for (; index < 0x10; ++index, ++cameraIterator) {
        if ((u32)(cameraMask & (1 << index)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_volume;
        }
    }

    selectedCamera = Hu3DCamera;
camera_selected_for_volume:

    HU3D_CAMERA *camera = selectedCamera;
    Vec difference;
    PSVECSubtract(&camera->pos, target, &difference);
    f32 distance = PSVECMag(&difference);
    CurveParameters *parameters = &lbl_1_data_790;
    if (distance < parameters->minimum) {
        distance = parameters->minimum;
    }
    if (distance > parameters->maximum) {
        distance = parameters->maximum;
    }

    f32 curve = (f32)(1.0 - (f64)((distance - parameters->minimum) /
                                                (parameters->maximum - parameters->minimum)));
    curve *= curve;
    s32 result = (s32)((f32)parameters->start +
                       curve * (f32)(parameters->end - parameters->start));
    return result;
}

static inline s16 fn_1_103EC_for_10B90(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}
static inline HU3D_CAMERA *CameraGetQuery10B90(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}
static inline void ProjectQuery10B90(s16 cameraMask, Vec *worldPosition, Vec *result,
                                   CameraParam *range)
{
    HU3D_CAMERA *camera = CameraGetQuery10B90(cameraMask);
    Vec *sourcePoint = worldPosition;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 magnitude;
    f32 absoluteZ;
    f32 z;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)(MTXDegToRad(camera->fov / 2.0f)));
    z = transformed.z;
    absoluteZ = __fabsf(z);
    magnitude = absoluteZ;
    depth = (f32)((f64)tangent * (f64)magnitude);
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}

extern "C" { extern CameraParam lbl_1_data_6D0[]; }

s16 fn_1_A708(s16 cameraMask, Point3d *targetPoint)
{
    CameraParam *parameters = lbl_1_data_6D0;
    CameraParam *selectedCameraRange10B90;
    s32 parameterIndex = 0;
    for (; parameterIndex < 0x10; ++parameterIndex, ++parameters) {
        if ((u32)(cameraMask & (1 << parameterIndex)) != 0) {
            selectedCameraRange10B90 = parameters;
            goto projection_parameters_selected_for_voice_pan;
        }
    }
    selectedCameraRange10B90 = lbl_1_data_6D0;
projection_parameters_selected_for_voice_pan:
    CameraParam *boundsParameters = selectedCameraRange10B90;

    Vec projected;
    ProjectQuery10B90(cameraMask, targetPoint, &projected, boundsParameters);
    return fn_1_103EC_for_10B90(&projected.x, (IntegerBounds *)boundsParameters);
}

extern "C" CurveParameters lbl_1_data_790;

static inline s16 fn_1_103EC_for_10E30(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}
static inline CameraParam *CameraRangeGet10E30(s16 cameraMask)
{
    CameraParam *range = lbl_1_data_6D0;
    s32 rangeIndex = 0;
    for (; rangeIndex < 16; ++rangeIndex, ++range) {
        if ((u32)(cameraMask & (1 << rangeIndex)) != 0) return range;
    }
    return lbl_1_data_6D0;
}
static inline HU3D_CAMERA *CameraGet10E30(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}
static inline s32 fn_1_102F8_for_10E30(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_790;
    f32 curve;
    s32 result;
    if (value < parameters->minimum) value = parameters->minimum;
    if (value > parameters->maximum) value = parameters->maximum;
    curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                          (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s32)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}
static inline f32 ProjectQuery10E30(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                   CameraParam *range)
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
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

extern "C" { extern CameraParam lbl_1_data_6D0[]; }
extern "C" { extern CurveParameters lbl_1_data_790; }

void fn_1_A9A8(s16 cameraMask, Point3d *targetPoint, s32 *volume, s32 *pan)
{
    CameraParam *parameters = CameraRangeGet10E30(cameraMask);
    Vec projected;
    f32 distance = ProjectQuery10E30(targetPoint, &projected, CameraGet10E30(cameraMask), parameters);
    *volume = (s16)fn_1_102F8_for_10E30(distance);
    *pan = fn_1_103EC_for_10E30(&projected.x, (IntegerBounds *)parameters);
}

extern "C" {

}

extern "C" { extern CameraParam lbl_1_data_6D0[]; }

static inline s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint);
static inline CameraParam *fn_1_9F5C_for_B02C_inlined(short mask);
static inline void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result, HU3D_CAMERA *camera, CameraParam *scale);
static inline HU3D_CAMERA *fn_1_9EF8_for_B02C_inlined(short mask);
static inline s16 fn_1_A200_for_B02C_inlined(f32 *fraction, IntegerBounds *bounds);

static inline s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint)
{
    CameraParam *projectionParameters = fn_1_9F5C_for_B02C_inlined(cameraMask);
    Vec projected;
    fn_1_9FC0_for_B02C_inlined(targetPoint, &projected, fn_1_9EF8_for_B02C_inlined(cameraMask),
                       projectionParameters);
    return fn_1_A200_for_B02C_inlined(&projected.x, (IntegerBounds *)projectionParameters);
}

static inline CameraParam *fn_1_9F5C_for_B02C_inlined(short mask)
{
    CameraParam *param = lbl_1_data_6D0;
    for (int i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_6D0;
}

static inline void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result,
                                      HU3D_CAMERA *camera,
                                      CameraParam *scale)
{
    f32 distance;
    Point3d *point = sourcePoint;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 magnitude;
    f32 absoluteZ;
    f32 z;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, point, &transformed);
    tangent = (f32)tan(MTXDegToRad(camera->fov / 2.0f));
    z = transformed.z;
    absoluteZ = __fabsf(z);
    magnitude = absoluteZ;
    depth = (f32)((f64)tangent * (f64)magnitude);
    scaledDepth = depth * scale->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&point, &camera->pos, &transformed);
    distance = PSVECMag(&transformed);
}

static inline HU3D_CAMERA *fn_1_9EF8_for_B02C_inlined(short mask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    for (int i = 0; i < 16; i++, camera++)
        if (mask & (1U << i)) return camera;
    return Hu3DCamera;
}

static inline s16 fn_1_A200_for_B02C_inlined(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return value < bounds->minimum ? bounds->minimum :
        value > bounds->maximum ? bounds->maximum : value;
}

void fn_1_AD90(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    HuAudFXPlayPan(effectId, fn_1_A9A4_for_B02C_inlined(cameraMask, targetPoint));
}

static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint);
static inline CameraParam *projectionForMask23D98_inlined(s16 cameraMask);
static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result, HU3D_CAMERA *camera, CameraParam *scale);
static inline HU3D_CAMERA *cameraForMask23D98_inlined(s16 cameraMask);
static inline s16 clampPan23D98_inlined(f32 *fraction, IntegerBounds *bounds);

static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint)
{
    CameraParam *parameters = projectionForMask23D98_inlined(cameraMask);
    Vec projected;
    projectPoint23D98_inlined(targetPoint, &projected, cameraForMask23D98_inlined(cameraMask),
                      parameters);
    return clampPan23D98_inlined(&projected.x, (IntegerBounds *)parameters);
}

static inline CameraParam *projectionForMask23D98_inlined(s16 cameraMask)
{
    CameraParam *parameters = lbl_1_data_6D0;
    s32 index = 0;
    for (; index < 16; ++index, ++parameters) {
        if ((u32)(cameraMask & (1 << index)) != 0)
            return parameters;
    }
    return lbl_1_data_6D0;
}

static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, CameraParam *scale)
{
    f32 distance;
    Point3d *point = sourcePoint;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 magnitude;
    f32 absoluteZ;
    f32 z;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, point, &transformed);
    tangent = (f32)tan(MTXDegToRad(camera->fov / 2.0f));
    z = transformed.z;
    absoluteZ = __fabsf(z);
    magnitude = absoluteZ;
    depth = (f32)((f64)tangent * (f64)magnitude);
    scaledDepth = depth * scale->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&point, &camera->pos, &transformed);
    distance = PSVECMag(&transformed);
}

static inline HU3D_CAMERA *cameraForMask23D98_inlined(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 index = 0;
    for (; index < 16; ++index, ++camera) {
        if ((u32)(cameraMask & (1 << index)) != 0)
            return camera;
    }
    return Hu3DCamera;
}

static inline s16 clampPan23D98_inlined(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

void fn_1_B058(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, cameraPan23D98_inlined(cameraMask, targetPoint), delay);
}

static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan);
static inline CameraParam *RangeGetFor10188_inlined(s16 cameraMask);
static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera, CameraParam *range);
static inline HU3D_CAMERA *CameraGetQueryFor10188_inlined(s16 cameraMask);
static inline s16 fn_1_102F8_for_10188_inlined(f32 value);
static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, CameraParam *bounds);

static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    CameraParam *parameters = RangeGetFor10188_inlined(cameraMask);
    Vec projected;
    f32 distance = ProjectVolume10188_inlined(targetPoint, &projected,
                                      CameraGetQueryFor10188_inlined(cameraMask), parameters);
    *volume = fn_1_102F8_for_10188_inlined(distance);
    *pan = fn_1_103EC_for_10188_inlined(&projected.x, parameters);
}

static inline CameraParam *RangeGetFor10188_inlined(s16 cameraMask)
{
    CameraParam *range = lbl_1_data_6D0;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_6D0;
}

static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                     CameraParam *range)
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
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline HU3D_CAMERA *CameraGetQueryFor10188_inlined(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}

static inline s16 fn_1_102F8_for_10188_inlined(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_790;
    f32 curve;
    s16 result;
    if (value < parameters->minimum) value = parameters->minimum;
    if (value > parameters->maximum) value = parameters->maximum;
    curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                          (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (f32)parameters->start + curve * (f32)(parameters->end - parameters->start);
    return result;
}

static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, CameraParam *bounds)
{
    s32 midpoint = (bounds->second + bounds->first) / 2;
    s32 halfRange = (bounds->second - bounds->first) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->first ? bounds->first :
                 value > bounds->second ? bounds->second : value);
}

void fn_1_B328(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;
    AudioValues10188_inlined(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

static inline s16 fn_1_103EC_for_For11218(f32 *fraction, CameraParam *bounds)
{
    s32 midpoint = (bounds->second + bounds->first) / 2;
    s32 halfRange = (bounds->second - bounds->first) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->first ? bounds->first :
                 value > bounds->second ? bounds->second : value);
}
static inline HU3D_CAMERA *CameraGetQueryFor11218(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}
static inline CameraParam *RangeGetFor11218(s16 cameraMask)
{
    CameraParam *range = lbl_1_data_6D0;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_6D0;
}
static inline f32 ProjectPanFor11BB0(Vec *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, CameraParam *scale)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;

    C_MTXLookAt(lookAtMatrix, (Point3d *)&camera->pos, &camera->up,
                (Point3d *)&camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan(MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * scale->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}
static inline s16 PanQueryFor11BB0(s16 cameraMask, Point3d *targetPoint)
{
    CameraParam *parameters = RangeGetFor11218(cameraMask);
    Vec projected;
    ProjectPanFor11BB0(targetPoint, &projected, CameraGetQueryFor11218(cameraMask), parameters);
    return fn_1_103EC_for_For11218(&projected.x, parameters);
}

extern "C" { extern CameraParam lbl_1_data_6D0[]; }

s16 fn_1_B728(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan = PanQueryFor11BB0(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

void fn_1_BA00(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, PanQueryFor11BB0(cameraMask, targetPoint));
}

/* Sound volume and pitch helpers, and overlay heap allocation. */

bool fn_1_BCC8(s16 cameraMask, s32 handle, const Point3d *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex = 0;
    Point3d difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 soundVolume;
    s32 callVolume;
    s32 volume;

    for (; cameraIndex < 16; ++cameraIndex, ++cameraIterator) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_sound;
        }
    }
    selectedCamera = Hu3DCamera;
camera_selected_for_sound:
    camera = selectedCamera;
    PSVECSubtract(&camera->pos, position, &difference);
    distance = PSVECMag(&difference);
    range = &lbl_1_data_790;
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

void fn_1_BE68(s16 cameraMask, s16 voiceIndex, const Point3d *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex = 0;
    Point3d difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 voiceVolume;
    s32 callVolume;
    s32 volume;

    for (; cameraIndex < 16; ++cameraIndex, ++cameraIterator) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) {
            selectedCamera = cameraIterator;
            goto camera_selected_for_volume;
        }
    }
    selectedCamera = Hu3DCamera;
camera_selected_for_volume:
    camera = selectedCamera;
    PSVECSubtract(&camera->pos, position, &difference);
    distance = PSVECMag(&difference);
    range = &lbl_1_data_790;
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

bool fn_1_C000(int handle,short pitch)
{ return HuAudFXPitchSet(handle,pitch)==0; }
