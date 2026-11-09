#include "math.h"
extern "C" {
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/charman.h"
int HuAudFXPlay(int);
int HuAudFXPlayPan(int, s16);
void HuAudFXPanning(int, s16);
void HuAudFXStop(int);
int HuAudFXPlayDelayPan(int, s16, u16);
int HuAudFXPlayVolPan(int, s16, s32);
s32 HuAudFXVolSet(s32, s16);
s32 HuAudFXPitchSet(s32, s16);
}
struct IntegerBounds { s32 minimum; s32 maximum; };
struct ProjectionParameters { s32 minimum; s32 maximum; f32 scale; };
struct CurveParameters { s32 start; s32 end; f32 maximum; f32 minimum; };

extern "C" {
ProjectionParameters lbl_1_data_5A8[16] = { {0x20, 0x60, 1.2f} };
CurveParameters lbl_1_data_668 = {0x20, 0x6E, 20000.0f, 100.0f};

}

static inline s16 fn_1_103EC_for_10B90(f32 *fraction, IntegerBounds *bounds);
static inline HU3D_CAMERA *CameraGetQuery10B90(s16 cameraMask);
static inline void ProjectQuery10B90(s16 cameraMask, Vec *worldPosition, Vec *result,
                                   ProjectionParameters *range);
static inline s16 fn_1_103EC_for_10E30(f32 *fraction, IntegerBounds *bounds);
static inline ProjectionParameters *CameraRangeGet10E30(s16 cameraMask);
static inline HU3D_CAMERA *CameraGet10E30(s16 cameraMask);
static inline s32 fn_1_102F8_for_10E30(f32 value);
static inline f32 ProjectQuery10E30(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                   ProjectionParameters *range);
static inline s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint);
static inline ProjectionParameters *fn_1_9F5C_for_B02C_inlined(short mask);
static inline void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result,
                                      HU3D_CAMERA *camera,
                                      ProjectionParameters *scale);
static inline HU3D_CAMERA *fn_1_9EF8_for_B02C_inlined(short mask);
static inline s16 fn_1_A200_for_B02C_inlined(f32 *fraction, IntegerBounds *bounds);
static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint);
static inline ProjectionParameters *projectionForMask23D98_inlined(s16 cameraMask);
static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ProjectionParameters *scale);
static inline HU3D_CAMERA *cameraForMask23D98_inlined(s16 cameraMask);
static inline s16 clampPan23D98_inlined(f32 *fraction, IntegerBounds *bounds);
static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan);
static inline ProjectionParameters *RangeGetFor10188_inlined(s16 cameraMask);
static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                     ProjectionParameters *range);
static inline HU3D_CAMERA *CameraGetQueryFor10188_inlined(s16 cameraMask);
static inline s16 fn_1_102F8_for_10188_inlined(f32 value);
static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, ProjectionParameters *bounds);
static inline s16 fn_1_103EC_for_For11218(f32 *fraction, IntegerBounds *bounds);
static inline HU3D_CAMERA *CameraGetQueryFor11218(s16 cameraMask);
static inline ProjectionParameters *RangeGetFor11218(s16 cameraMask);
static inline f32 ProjectPanFor11BB0(Vec *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ProjectionParameters *scale);
static inline s16 PanQueryFor11BB0(s16 cameraMask, Point3d *targetPoint);
static inline s16 fn_1_103EC_for_For11E88(f32 *fraction, IntegerBounds *bounds);
static inline HU3D_CAMERA *CameraGetQueryFor11E88(s16 cameraMask);
static inline void ProjectQueryFor11E88(s16 cameraMask, Vec *worldPosition, Vec *result,
                                   ProjectionParameters *range);
static inline ProjectionParameters *RangeGetFor11E88(s16 cameraMask);
static inline s16 fn_1_10B90_for_11E88(s16 cameraMask, Point3d *targetPoint);

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
                                   ProjectionParameters *range)
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
    scaledDepth = depth * range->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}

static inline s16 fn_1_103EC_for_10E30(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

static inline ProjectionParameters *CameraRangeGet10E30(s16 cameraMask)
{
    ProjectionParameters *range = lbl_1_data_5A8;
    s32 rangeIndex = 0;
    for (; rangeIndex < 16; ++rangeIndex, ++range) {
        if ((u32)(cameraMask & (1 << rangeIndex)) != 0) return range;
    }
    return lbl_1_data_5A8;
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
    CurveParameters *parameters = &lbl_1_data_668;
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
                                   ProjectionParameters *range)
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
    scaledDepth = depth * range->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline s16 fn_1_A9A4_for_B02C_inlined(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *projectionParameters = fn_1_9F5C_for_B02C_inlined(cameraMask);
    Vec projected;
    fn_1_9FC0_for_B02C_inlined(targetPoint, &projected, fn_1_9EF8_for_B02C_inlined(cameraMask),
                       projectionParameters);
    return fn_1_A200_for_B02C_inlined(&projected.x, (IntegerBounds *)projectionParameters);
}

static inline ProjectionParameters *fn_1_9F5C_for_B02C_inlined(short mask)
{
    ProjectionParameters *param = lbl_1_data_5A8;
    for (int i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_5A8;
}

static inline void fn_1_9FC0_for_B02C_inlined(Point3d *sourcePoint, Vec *result,
                                      HU3D_CAMERA *camera,
                                      ProjectionParameters *scale)
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
    scaledDepth = depth * scale->scale;
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

static inline s16 cameraPan23D98_inlined(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *parameters = projectionForMask23D98_inlined(cameraMask);
    Vec projected;
    projectPoint23D98_inlined(targetPoint, &projected, cameraForMask23D98_inlined(cameraMask),
                      parameters);
    return clampPan23D98_inlined(&projected.x, (IntegerBounds *)parameters);
}

static inline ProjectionParameters *projectionForMask23D98_inlined(s16 cameraMask)
{
    ProjectionParameters *parameters = lbl_1_data_5A8;
    s32 index = 0;
    for (; index < 16; ++index, ++parameters) {
        if ((u32)(cameraMask & (1 << index)) != 0)
            return parameters;
    }
    return lbl_1_data_5A8;
}

static inline void projectPoint23D98_inlined(Point3d *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ProjectionParameters *scale)
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
    scaledDepth = depth * scale->scale;
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

static inline void AudioValues10188_inlined(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    ProjectionParameters *parameters = RangeGetFor10188_inlined(cameraMask);
    Vec projected;
    f32 distance = ProjectVolume10188_inlined(targetPoint, &projected,
                                      CameraGetQueryFor10188_inlined(cameraMask), parameters);
    *volume = fn_1_102F8_for_10188_inlined(distance);
    *pan = fn_1_103EC_for_10188_inlined(&projected.x, parameters);
}

static inline ProjectionParameters *RangeGetFor10188_inlined(s16 cameraMask)
{
    ProjectionParameters *range = lbl_1_data_5A8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5A8;
}

static inline f32 ProjectVolume10188_inlined(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                                     ProjectionParameters *range)
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
    scaledDepth = depth * range->scale;
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
    CurveParameters *parameters = &lbl_1_data_668;
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

static inline s16 fn_1_103EC_for_10188_inlined(f32 *fraction, ProjectionParameters *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

static inline s16 fn_1_103EC_for_For11218(f32 *fraction, IntegerBounds *bounds)
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

static inline ProjectionParameters *RangeGetFor11218(s16 cameraMask)
{
    ProjectionParameters *range = lbl_1_data_5A8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5A8;
}

static inline f32 ProjectPanFor11BB0(Vec *sourcePoint, Vec *result,
                                     HU3D_CAMERA *camera, ProjectionParameters *scale)
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
    scaledDepth = depth * scale->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;
    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

static inline s16 PanQueryFor11BB0(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *parameters = RangeGetFor11218(cameraMask);
    Vec projected;
    ProjectPanFor11BB0(targetPoint, &projected, CameraGetQueryFor11218(cameraMask), parameters);
    return fn_1_103EC_for_For11218(&projected.x, (IntegerBounds *)parameters);
}

static inline s16 fn_1_103EC_for_For11E88(f32 *fraction, IntegerBounds *bounds)
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
                                   ProjectionParameters *range)
{
    HU3D_CAMERA *camera = CameraGetQueryFor11E88(cameraMask);
    f32 distance;
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
    scaledDepth = depth * range->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    distance = PSVECMag(&transformed);
}

static inline ProjectionParameters *RangeGetFor11E88(s16 cameraMask)
{
    ProjectionParameters *range = lbl_1_data_5A8;
    s32 index = 0;
    for (; index < 16; ++index, ++range) {
        if ((u32)(cameraMask & (1 << index)) != 0) return range;
    }
    return lbl_1_data_5A8;
}

static inline s16 fn_1_10B90_for_11E88(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *boundsParameters = RangeGetFor11E88(cameraMask);

    Vec projected;
    ProjectQueryFor11E88(cameraMask, targetPoint, &projected, boundsParameters);
    return fn_1_103EC_for_For11E88(&projected.x, (IntegerBounds *)boundsParameters);
}

HU3D_CAMERA *fn_1_81D4(short mask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    for (int i = 0; i < 16; i++, camera++)
        if (mask & (1U << i)) return camera;
    return Hu3DCamera;
}

ProjectionParameters *fn_1_8238(short mask)
{
    ProjectionParameters *param = lbl_1_data_5A8;
    for (int i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return lbl_1_data_5A8;
}

void fn_1_829C(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                           ProjectionParameters *scale)
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
    scaledDepth = depth * scale->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    PSVECMag(&transformed);
}

s32 fn_1_83E8(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_668;
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

s16 fn_1_84DC(f32 *fraction, IntegerBounds *bounds)
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

int fn_1_85CC(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_85F4(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_861C(int first,int second,float a,float b) { CurveParameters *param=&lbl_1_data_668; param->start=first; param->end=second; param->minimum=b; param->maximum=a; }

void fn_1_8648(short mask,int first,int second,float value) { ProjectionParameters *param=lbl_1_data_5A8; for(int i=0;i<16;i++,param++) if(mask & (1U<<i)) {param->minimum=first;param->maximum=second;param->scale=value;break;} }

void fn_1_86AC(s32 mode)
{
    switch (mode) {
    case 0: {
        ProjectionParameters *view0 = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view0) {
            if ((u32)((1 << index) & 1) != 0) {
                view0->minimum = 0x20;
                view0->maximum = 0x60;
                view0->scale = 1.2f;
                return;
            }
        }
        return;
    }
    case 1: {
        ProjectionParameters *view1 = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view1) {
            if ((u32)((1 << index) & 3) != 0) {
                view1->minimum = 0x20;
                view1->maximum = 0x60;
                view1->scale = 3.2f;
                return;
            }
        }
        return;
    }
    case 2:
    {
        ProjectionParameters *view2a = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view2a) {
            if ((u32)((1 << index) & 1) != 0) {
                view2a->minimum = 0x20;
                view2a->maximum = 0x40;
                view2a->scale = 0.8f;
                break;
            }
        }
        ProjectionParameters *view2b = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view2b) {
            if ((u32)((1 << index) & 2) != 0) {
                view2b->minimum = 0x40;
                view2b->maximum = 0x60;
                view2b->scale = 0.8f;
                return;
            }
        }
        return;
    }
    case 3:
    {
        ProjectionParameters *view3a = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view3a) {
            if ((u32)((1 << index) & 5) != 0) {
                view3a->minimum = 0x20;
                view3a->maximum = 0x40;
                view3a->scale = 1.2f;
                break;
            }
        }
        ProjectionParameters *view3b = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view3b) {
            if ((u32)((1 << index) & 0xA) != 0) {
                view3b->minimum = 0x40;
                view3b->maximum = 0x60;
                view3b->scale = 1.2f;
                return;
            }
        }
        return;
    }
    case 4:
    {
        ProjectionParameters *view4a = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view4a) {
            if ((u32)((1 << index) & 1) != 0) {
                view4a->minimum = 0x30;
                view4a->maximum = 0x30;
                view4a->scale = 0.4f;
                break;
            }
        }
        ProjectionParameters *view4b = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view4b) {
            if ((u32)((1 << index) & 2) != 0) {
                view4b->minimum = 0x38;
                view4b->maximum = 0x38;
                view4b->scale = 0.4f;
                break;
            }
        }
        ProjectionParameters *view4c = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view4c) {
            if ((u32)((1 << index) & 4) != 0) {
                view4c->minimum = 0x48;
                view4c->maximum = 0x48;
                view4c->scale = 0.4f;
                break;
            }
        }
        ProjectionParameters *view4d = &lbl_1_data_5A8[0];
        for (s32 index = 0; index < 16; ++index, ++view4d) {
            if ((u32)((1 << index) & 8) != 0) {
                view4d->minimum = 0x50;
                view4d->maximum = 0x50;
                view4d->scale = 0.4f;
                return;
            }
        }
    }
    }
}

s32 fn_1_8AFC(s16 cameraMask, Point3d *target)
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
    CurveParameters *parameters = &lbl_1_data_668;
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

s16 fn_1_8C80(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *parameters = lbl_1_data_5A8;
    ProjectionParameters *selectedCameraRange10B90;
    s32 parameterIndex = 0;
    for (; parameterIndex < 0x10; ++parameterIndex, ++parameters) {
        if ((u32)(cameraMask & (1 << parameterIndex)) != 0) {
            selectedCameraRange10B90 = parameters;
            goto projection_parameters_selected_for_voice_pan;
        }
    }
    selectedCameraRange10B90 = lbl_1_data_5A8;
projection_parameters_selected_for_voice_pan:
    ProjectionParameters *boundsParameters = selectedCameraRange10B90;

    Vec projected;
    ProjectQuery10B90(cameraMask, targetPoint, &projected, boundsParameters);
    return fn_1_103EC_for_10B90(&projected.x, (IntegerBounds *)boundsParameters);
}

void fn_1_8F20(s16 cameraMask, Point3d *targetPoint, s32 *volume, s32 *pan)
{
    ProjectionParameters *parameters = CameraRangeGet10E30(cameraMask);
    Vec projected;
    f32 distance = ProjectQuery10E30(targetPoint, &projected, CameraGet10E30(cameraMask), parameters);
    *volume = (s16)fn_1_102F8_for_10E30(distance);
    *pan = fn_1_103EC_for_10E30(&projected.x, (IntegerBounds *)parameters);
}

void fn_1_9308(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    HuAudFXPlayPan(effectId, fn_1_A9A4_for_B02C_inlined(cameraMask, targetPoint));
}

void fn_1_95D0(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, cameraPan23D98_inlined(cameraMask, targetPoint), delay);
}

void fn_1_98A0(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;
    AudioValues10188_inlined(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

s16 fn_1_9CA0(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan = PanQueryFor11BB0(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

void fn_1_9F78(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, fn_1_10B90_for_11E88(cameraMask, targetPoint));
}

bool fn_1_A240(s16 cameraMask, s32 handle, const Vec *position)
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
    range = &lbl_1_data_668;
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

void fn_1_A3E0(s16 cameraMask, s16 voiceIndex, const Vec *position)
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
    range = &lbl_1_data_668;
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

bool fn_1_A578(int handle, short pitch)
{ return HuAudFXPitchSet(handle, pitch) == 0; }
