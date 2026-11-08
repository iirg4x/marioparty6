#include <math.h>
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "REL/m648DLL/sound.h"
extern "C" {
#include "dolphin/types.h"
int HuAudFXPlay(int);
void HuAudFXStop(int);
int HuAudFXPlayPan(int, s16);
void HuAudFXPlayDelayPan(s32, s16, u16);
s32 HuAudFXPlayVolPan(s32, s16, s32);
void HuAudFXPanning(int, s16);
s32 HuAudFXVolSet(int, s16);
}
struct ViewScale { s32 minimum; s32 maximum; f32 value; };
struct CurveParameters { s32 start; s32 end; f32 maximum; f32 minimum; };
extern "C" {
ViewScale lbl_1_data_5C8[16] = {{0x20, 0x60, 1.2f}};
CurveParameters lbl_1_data_688 = {0x20, 0x6E, 20000.0f, 100.0f};
}

extern "C" {
#include "dolphin/os.h"
}

extern "C" {
#include "game/charman.h"
}

void fn_1_DBA4(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                           ViewScale *scale)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up,
                &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);

    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * scale->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}

struct IntegerBounds {
    s32 minimum;
    s32 maximum;
};

extern "C" double lbl_1_rodata_3B8;

extern "C" double lbl_1_rodata_3B0;

extern "C" CurveParameters lbl_1_data_468;

extern "C" { extern CurveParameters lbl_1_data_1680; }

s32 fn_1_DCF0(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_688;
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

s16 fn_1_DDE4(f32 *fraction, IntegerBounds *bounds)
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

int fn_1_DED4(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_DEFC(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_DF24(int first,int second,float a,float b) { CurveParameters *param=&lbl_1_data_688; param->start=first; param->end=second; param->minimum=b; param->maximum=a; }

void fn_1_DF50(short mask, int first, int second, float value)
{
    ViewScale *scale = lbl_1_data_5C8;
    for (int index = 0; index < 16; index++, scale++) {
        if (mask & (1U << index)) {
            scale->minimum = first;
            scale->maximum = second;
            scale->value = value;
            break;
        }
    }
}

void fn_1_DFB4(s32 mode)
{
    switch (mode) {
    case 0: {
        ViewScale *view0 = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view0) {
            if ((u32)((1 << index) & 1) != 0) {
                view0->minimum = 0x20;
                view0->maximum = 0x60;
                view0->value = 1.2f;
                return;
            }
        }
        return;
    }
    case 1: {
        ViewScale *view1 = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view1) {
            if ((u32)((1 << index) & 3) != 0) {
                view1->minimum = 0x20;
                view1->maximum = 0x60;
                view1->value = 3.2f;
                return;
            }
        }
        return;
    }
    case 2: {
        ViewScale *view2a = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view2a) {
            if ((u32)((1 << index) & 1) != 0) {
                view2a->minimum = 0x20;
                view2a->maximum = 0x40;
                view2a->value = 0.8f;
                break;
            }
        }
        ViewScale *view2b = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view2b) {
            if ((u32)((1 << index) & 2) != 0) {
                view2b->minimum = 0x40;
                view2b->maximum = 0x60;
                view2b->value = 0.8f;
                return;
            }
        }
        return;
    }
    case 3: {
        ViewScale *view3a = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view3a) {
            if ((u32)((1 << index) & 5) != 0) {
                view3a->minimum = 0x20;
                view3a->maximum = 0x40;
                view3a->value = 1.2f;
                break;
            }
        }
        ViewScale *view3b = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view3b) {
            if ((u32)((1 << index) & 0xA) != 0) {
                view3b->minimum = 0x40;
                view3b->maximum = 0x60;
                view3b->value = 1.2f;
                return;
            }
        }
        return;
    }
    case 4: {
        ViewScale *view4a = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view4a) {
            if ((u32)((1 << index) & 1) != 0) {
                view4a->minimum = 0x30;
                view4a->maximum = 0x30;
                view4a->value = 0.4f;
                break;
            }
        }
        ViewScale *view4b = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view4b) {
            if ((u32)((1 << index) & 2) != 0) {
                view4b->minimum = 0x38;
                view4b->maximum = 0x38;
                view4b->value = 0.4f;
                break;
            }
        }
        ViewScale *view4c = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view4c) {
            if ((u32)((1 << index) & 4) != 0) {
                view4c->minimum = 0x48;
                view4c->maximum = 0x48;
                view4c->value = 0.4f;
                break;
            }
        }
        ViewScale *view4d = &lbl_1_data_5C8[0];
        for (s32 index = 0; index < 16; ++index, ++view4d) {
            if ((u32)((1 << index) & 8) != 0) {
                view4d->minimum = 0x50;
                view4d->maximum = 0x50;
                view4d->value = 0.4f;
                return;
            }
        }
    }
    }
}

s32 fn_1_E404(s16 cameraMask, Point3d *target)
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
    CurveParameters *parameters = &lbl_1_data_688;
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

static inline s16 fn_1_103EC_for_10B90(f32 *fraction, ViewScale *bounds)
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
                                   ViewScale *range)
{
    HU3D_CAMERA *camera = CameraGetQuery10B90(cameraMask);
    Vec *sourcePoint = worldPosition;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}
s16 fn_1_E588(s16 cameraMask, Point3d *targetPoint)
{
    ViewScale *parameters = lbl_1_data_5C8;
    ViewScale *selectedCameraRange10B90;
    s32 parameterIndex = 0;
    for (; parameterIndex < 0x10; ++parameterIndex, ++parameters) {
        if ((u32)(cameraMask & (1 << parameterIndex)) != 0) {
            selectedCameraRange10B90 = parameters;
            goto projection_parameters_selected_for_voice_pan;
        }
    }
    selectedCameraRange10B90 = lbl_1_data_5C8;
projection_parameters_selected_for_voice_pan:
    ViewScale *boundsParameters = selectedCameraRange10B90;

    Vec projected;
    ProjectQuery10B90(cameraMask, targetPoint, &projected, boundsParameters);
    return fn_1_103EC_for_10B90(&projected.x, boundsParameters);
}

static inline s16 fn_1_103EC_for_10E30(f32 *fraction, ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}
static inline ViewScale *CameraRangeGet10E30(s16 cameraMask)
{
    ViewScale *range = lbl_1_data_5C8;
    s32 rangeIndex = 0;
    for (; rangeIndex < 16; ++rangeIndex, ++range) {
        if ((u32)(cameraMask & (1 << rangeIndex)) != 0) return range;
    }
    return lbl_1_data_5C8;
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
    CurveParameters *parameters = &lbl_1_data_688;
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
                                   ViewScale *range)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}
void fn_1_E828(s16 cameraMask, Point3d *targetPoint, s32 *volume, s32 *pan)
{
    ViewScale *parameters = CameraRangeGet10E30(cameraMask);
    Vec projected;
    f32 distance = ProjectQuery10E30(targetPoint, &projected, CameraGet10E30(cameraMask), parameters);
    *volume = (s16)fn_1_102F8_for_10E30(distance);
    *pan = fn_1_103EC_for_10E30(&projected.x, parameters);
}

static s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint);

static ViewScale *fn_1_9F5C_for_B02C_inlined(short mask);

static void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result, HU3D_CAMERA *camera, ViewScale *scale);

static HU3D_CAMERA *fn_1_9EF8_for_B02C_inlined(short mask);

static s16 fn_1_A200_for_B02C_inlined(f32 *fraction, ViewScale *bounds);

static inline s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint)
{
    ViewScale *projectionParameters = fn_1_9F5C_for_B02C_inlined(cameraMask);
    Vec projected;
    fn_1_9FC0_for_B02C_inlined(targetPoint, &projected, fn_1_9EF8_for_B02C_inlined(cameraMask),
                       projectionParameters);
    return fn_1_A200_for_B02C_inlined(&projected.x, projectionParameters);
}

static inline ViewScale *fn_1_9F5C_for_B02C_inlined(short mask)
{
    ViewScale *param = lbl_1_data_5C8;
    for (int i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_5C8;
}

static inline void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result,
                                      HU3D_CAMERA *camera,
                                      ViewScale *scale)
{
    f32 distance;
    Point3d *point = sourcePoint;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up,
                &camera->target);
    PSMTXMultVec(lookAtMatrix, point, &transformed);
    tangent = (f32)tan(MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
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

static inline s16 fn_1_A200_for_B02C_inlined(f32 *fraction, ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return value < bounds->minimum ? bounds->minimum :
        value > bounds->maximum ? bounds->maximum : value;
}

void fn_1_EC10(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    HuAudFXPlayPan(effectId, fn_1_A9A4_for_B02C_inlined(cameraMask, targetPoint));
}

static inline f64 absoluteCameraDepthEED8(f32 value)
{
    return fabs(value);
}

static inline f32 fn_1_DBA4_for_EED8(Vec *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ViewScale *range)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up,
                &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * absoluteCameraDepthEED8(transformed.z));
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline ViewScale *RangeGetEED8(s16 cameraMask)
{
    ViewScale *range = lbl_1_data_5C8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5C8;
}

static inline HU3D_CAMERA *CameraGetEED8(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 index = 0;
    for (; index < 16; ++index, ++camera) {
        if ((u32)(cameraMask & (1 << index)) != 0) return camera;
    }
    return Hu3DCamera;
}

static inline s16 PanFromProjectionEED8(f32 *fraction,
                                        ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

static inline s16 PanQueryEED8(s16 cameraMask, Point3d *targetPoint)
{
    ViewScale *range = RangeGetEED8(cameraMask);
    Vec projected;
    fn_1_DBA4_for_EED8(targetPoint, &projected,
                       CameraGetEED8(cameraMask), range);
    return PanFromProjectionEED8(&projected.x, range);
}

void fn_1_EED8(s16 cameraMask, s32 soundId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(soundId, PanQueryEED8(cameraMask, targetPoint), delay);
}

static inline ViewScale *CameraRangeGetF1A8(s16 cameraMask)
{
    ViewScale *range = lbl_1_data_5C8;
    s32 rangeIndex = 0;
    for (; rangeIndex < 16; ++rangeIndex, ++range) {
        if ((u32)(cameraMask & (1 << rangeIndex)) != 0) return range;
    }
    return lbl_1_data_5C8;
}

static inline HU3D_CAMERA *CameraGetF1A8(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}

static inline f32 fn_1_DBA4_for_F1A8(Vec *sourcePoint, Vec *result,
                                    HU3D_CAMERA *camera, ViewScale *range)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline s16 VolumeFromDistanceF1A8(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_688;
    f32 curve;
    s16 result;

    if (value < parameters->minimum) value = parameters->minimum;
    if (value > parameters->maximum) value = parameters->maximum;
    curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                          (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s16)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}

static inline s16 PanFromProjectionF1A8(f32 *fraction, ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

static inline void AudioValuesF1A8(s16 cameraMask, Point3d *targetPoint,
                                   s16 *volume, s32 *pan)
{
    ViewScale *range = CameraRangeGetF1A8(cameraMask);
    Vec projected;
    f32 distance = fn_1_DBA4_for_F1A8(targetPoint, &projected,
                                      CameraGetF1A8(cameraMask), range);
    *volume = VolumeFromDistanceF1A8(distance);
    *pan = PanFromProjectionF1A8(&projected.x, range);
}

void fn_1_F1A8(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;

    AudioValuesF1A8(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

static inline s16 fn_1_103EC_for_For11218(f32 *fraction, ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
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
static inline ViewScale *RangeGetFor11218(s16 cameraMask)
{
    ViewScale *range = lbl_1_data_5C8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5C8;
}
static inline f32 ProjectPanFor11BB0(Vec *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ViewScale *scale)
{
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 depth;
    f32 scaledDepth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up,
                &camera->target);
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
    ViewScale *parameters = RangeGetFor11218(cameraMask);
    Vec projected;
    ProjectPanFor11BB0(targetPoint, &projected, CameraGetQueryFor11218(cameraMask), parameters);
    return fn_1_103EC_for_For11218(&projected.x, parameters);
}
s16 fn_1_F5A8(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan = PanQueryFor11BB0(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

static inline s16 fn_1_103EC_for_For11E88(f32 *fraction, ViewScale *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}
static inline HU3D_CAMERA *CameraGetQueryFor11E88(s16 cameraMask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    s32 cameraIndex = 0;
    for (; cameraIndex < 16; ++cameraIndex, ++camera) {
        if ((u32)(cameraMask & (1 << cameraIndex)) != 0) return camera;
    }
    return Hu3DCamera;
}
static inline void ProjectQueryFor11E88(s16 cameraMask, Vec *worldPosition, Vec *result,
                                   ViewScale *range)
{
    HU3D_CAMERA *camera = CameraGetQueryFor11E88(cameraMask);
    f32 distance;
    Vec *sourcePoint = worldPosition;
    Mtx lookAtMatrix;
    Vec transformed;
    f32 tangent;
    f32 scaledDepth;
    f32 depth;

    C_MTXLookAt(lookAtMatrix, &camera->pos, &camera->up, &camera->target);
    PSMTXMultVec(lookAtMatrix, sourcePoint, &transformed);
    tangent = (f32)tan((f64)MTXDegToRad(camera->fov / 2.0f));
    depth = (f32)((f64)tangent * fabs(transformed.z));
    scaledDepth = depth * range->value;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    distance = PSVECMag(&transformed);
}
static inline ViewScale *RangeGetFor11E88(s16 cameraMask)
{
    ViewScale *range = lbl_1_data_5C8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5C8;
}
static inline s16 fn_1_10B90_for_11E88(s16 cameraMask, Point3d *targetPoint)
{
    ViewScale *boundsParameters = RangeGetFor11E88(cameraMask);

    Vec projected;
    ProjectQueryFor11E88(cameraMask, targetPoint, &projected, boundsParameters);
    return fn_1_103EC_for_For11E88(&projected.x, boundsParameters);
}
void fn_1_F880(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, fn_1_10B90_for_11E88(cameraMask, targetPoint));
}

bool fn_1_FB48(s16 cameraMask, s32 handle, const Vec *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex = 0;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 soundVolume;
    s32 callVolume;
    s32 volume;

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
    range = &lbl_1_data_688;
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

void fn_1_FCE8(s16 cameraMask, s16 voiceIndex, const Vec *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    s32 cameraIndex = 0;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    s32 voiceVolume;
    s32 callVolume;
    s32 volume;

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
    range = &lbl_1_data_688;
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
