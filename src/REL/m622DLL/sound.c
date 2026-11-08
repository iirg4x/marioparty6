/* Positional sound: camera lookup and projection of sound sources, distance volume curves,
 * and sound effect playback wrappers. */
#include "REL/m622DLL/audio_ranges.h"
#include <math.h>
#include "dolphin/types.h"
#include "dolphin/gx.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/charman.h"
#include "game/memory.h"
#include "REL/m622DLL/globals.h"

int HuAudFXPlay(int seId);
void HuAudFXStop(int seId);
int HuAudFXPlayPan(int seId, s16 pan);
int HuAudFXPlayDelayPan(int seId, s16 pan, u16 delay);
int HuAudFXPlayVolPan(int seId, s16 vol, s32 pan);
void HuAudFXPanning(int seNo, s16 pan);
int HuAudFXVolSet(int seNo, s16 vol);
int HuAudFXPitchSet(int seNo, s16 pitch);

typedef struct IntegerBounds { s32 minimum, maximum; } IntegerBounds;
typedef struct ProjectionParameters { IntegerBounds bounds; f32 scale; } ProjectionParameters;

ProjectionParameters lbl_1_data_3A8[16] = {{{32, 96}, 1.2f}};

HU3D_CAMERA *fn_1_9EF8(short mask) { HU3D_CAMERA *camera=Hu3DCamera; int i; for(i=0;i<16;i++,camera++) if(mask & (1U<<i)) return camera; return Hu3DCamera; }

ProjectionParameters *fn_1_9F5C(short mask) { ProjectionParameters *param=lbl_1_data_3A8; int i; for(i=0;i<16;i++,param++) if(mask & (1U<<i)) return param; return lbl_1_data_3A8; }

f32 fn_1_9FC0(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
                           ProjectionParameters *scale)
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
    depth = (f32)((f64)tangent * fabs((f64)transformed.z));
    scaledDepth = depth * scale->scale;
    result->x = transformed.x / scaledDepth;
    result->y = transformed.y / depth;
    result->z = -transformed.z;

    PSVECSubtract((Vec *)&sourcePoint, &camera->pos, &transformed);
    return PSVECMag(&transformed);
}

s32 fn_1_A10C(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_468;
    f32 curve;
    s32 result;
    if (value < parameters->minimum) {
        value = parameters->minimum;
    }
    if (value > parameters->maximum) {
        value = parameters->maximum;
    }

    curve = (f32)(1.0 - (f64)((value - parameters->minimum) /
                                                (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s32)((f32)parameters->start + curve * (f32)(parameters->end - parameters->start));
    return result;
}

s16 fn_1_A200(f32 *fraction, IntegerBounds *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

int fn_1_A2F0(int soundId)
{
    return HuAudFXPlay(soundId);
}

void fn_1_A318(int soundNo)
{
    HuAudFXStop(soundNo);
}

void fn_1_A340(int first,int second,float a,float b) { CurveParameters *param=&lbl_1_data_468; param->start=first; param->end=second; param->minimum=b; param->maximum=a; }

void fn_1_A36C(short mask,int first,int second,float value) { ProjectionParameters *param=lbl_1_data_3A8; int i; for(i=0;i<16;i++,param++) if(mask & (1U<<i)) {param->bounds.minimum=first;param->bounds.maximum=second;param->scale=value;break;} }

void fn_1_A3D0(s32 mode)
{
    switch (mode) {
    case 0: {
        ProjectionParameters *view0 = lbl_1_data_3A8;
        s32 index;
        for (index = 0; index < 16; ++index, ++view0) {
            if ((u32)((1 << index) & 1) != 0) {
                view0->bounds.minimum = 0x20;
                view0->bounds.maximum = 0x60;
                view0->scale = 1.2f;
                return;
            }
        }
        return;
    }
    case 1: {
        ProjectionParameters *view1 = lbl_1_data_3A8;
        s32 index;
        for (index = 0; index < 16; ++index, ++view1) {
            if ((u32)((1 << index) & 3) != 0) {
                view1->bounds.minimum = 0x20;
                view1->bounds.maximum = 0x60;
                view1->scale = 3.2f;
                return;
            }
        }
        return;
    }
    case 2:
    {
        {
            ProjectionParameters *view2a = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view2a) {
                if ((u32)((1 << index) & 1) != 0) {
                    view2a->bounds.minimum = 0x20;
                    view2a->bounds.maximum = 0x40;
                    view2a->scale = 0.8f;
                    break;
                }
            }
        }
        {
            ProjectionParameters *view2b = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view2b) {
                if ((u32)((1 << index) & 2) != 0) {
                    view2b->bounds.minimum = 0x40;
                    view2b->bounds.maximum = 0x60;
                    view2b->scale = 0.8f;
                    return;
                }
            }
        }
        return;
    }
    case 3:
    {
        {
            ProjectionParameters *view3a = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view3a) {
                if ((u32)((1 << index) & 5) != 0) {
                    view3a->bounds.minimum = 0x20;
                    view3a->bounds.maximum = 0x40;
                    view3a->scale = 1.2f;
                    break;
                }
            }
        }
        {
            ProjectionParameters *view3b = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view3b) {
                if ((u32)((1 << index) & 0xA) != 0) {
                    view3b->bounds.minimum = 0x40;
                    view3b->bounds.maximum = 0x60;
                    view3b->scale = 1.2f;
                    return;
                }
            }
        }
        return;
    }
    case 4:
    {
        {
            ProjectionParameters *view4a = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view4a) {
                if ((u32)((1 << index) & 1) != 0) {
                    view4a->bounds.minimum = 0x30;
                    view4a->bounds.maximum = 0x30;
                    view4a->scale = 0.4f;
                    break;
                }
            }
        }
        {
            ProjectionParameters *view4b = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view4b) {
                if ((u32)((1 << index) & 2) != 0) {
                    view4b->bounds.minimum = 0x38;
                    view4b->bounds.maximum = 0x38;
                    view4b->scale = 0.4f;
                    break;
                }
            }
        }
        {
            ProjectionParameters *view4c = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view4c) {
                if ((u32)((1 << index) & 4) != 0) {
                    view4c->bounds.minimum = 0x48;
                    view4c->bounds.maximum = 0x48;
                    view4c->scale = 0.4f;
                    break;
                }
            }
        }
        {
            ProjectionParameters *view4d = lbl_1_data_3A8;
            s32 index;
            for (index = 0; index < 16; ++index, ++view4d) {
                if ((u32)((1 << index) & 8) != 0) {
                    view4d->bounds.minimum = 0x50;
                    view4d->bounds.maximum = 0x50;
                    view4d->scale = 0.4f;
                    return;
                }
            }
        }
    }
    }
}

s32 fn_1_A820(s16 cameraMask, Point3d *target)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    s32 index = 0;
    HU3D_CAMERA *camera;
    Vec difference;
    f32 distance;
    CurveParameters *parameters;
    f32 curve;
    s32 result;

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
    parameters = &lbl_1_data_468;
    if (distance < parameters->minimum) {
        distance = parameters->minimum;
    }
    if (distance > parameters->maximum) {
        distance = parameters->maximum;
    }

    curve = (f32)(1.0 - (f64)((distance - parameters->minimum) /
                                                (parameters->maximum - parameters->minimum)));
    curve *= curve;
    result = (s32)((f32)parameters->start +
                       curve * (f32)(parameters->end - parameters->start));
    return result;
}

s16 fn_1_A9A4(s16 cameraMask, Point3d *targetPoint)
{
    ProjectionParameters *parameters = fn_1_9F5C(cameraMask);
    Vec projected;
    fn_1_9FC0(targetPoint, &projected,
                             fn_1_9EF8(cameraMask), parameters);
    return fn_1_A200(&projected.x, &parameters->bounds);
}

void fn_1_AC44(s16 cameraMask, Point3d *point, s32 *volume, s32 *pan)
{
    ProjectionParameters *parameters = fn_1_9F5C(cameraMask);
    Vec projected;
    f32 distance = fn_1_9FC0(point, &projected, fn_1_9EF8(cameraMask), parameters);
    *volume = (s16)fn_1_A10C(distance);
    *pan = fn_1_A200(&projected.x, &parameters->bounds);
}

void fn_1_B02C(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    HuAudFXPlayPan(effectId, fn_1_A9A4(cameraMask, targetPoint));
}

void fn_1_B2F4(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, fn_1_A9A4(cameraMask, targetPoint), delay);
}

static inline s16 distanceVolume(f32 value)
{
    CurveParameters *parameters = &lbl_1_data_468;
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

static inline void audioValues(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    ProjectionParameters *parameters = fn_1_9F5C(cameraMask);
    Vec projected;
    f32 distance = fn_1_9FC0(targetPoint, &projected, fn_1_9EF8(cameraMask), parameters);
    *volume = distanceVolume(distance);
    *pan = fn_1_A200(&projected.x, &parameters->bounds);
}

void fn_1_B5C4(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;

    audioValues(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

s16 fn_1_B9C4(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    int pan = fn_1_A9A4(cameraMask, targetPoint);
    HuAudFXPanning(effectId, pan);
    return pan;
}

void fn_1_BC9C(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, fn_1_A9A4(cameraMask, targetPoint));
}

BOOL fn_1_BF64(s16 cameraMask, int handle, const Vec *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    int cameraIndex = 0;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    int soundVolume;
    int callVolume;
    int volume;

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
    range = &lbl_1_data_468;
    if (distance < range->minimum)
        distance = range->minimum;
    if (distance > range->maximum)
        distance = range->maximum;

    weight = (f32)(1.0 - (double)((distance - range->minimum) /
                                  (range->maximum - range->minimum)));
    weight *= weight;
    volume = (int)(range->start +
                   weight * (f32)(range->end - range->start));
    callVolume = volume;
    soundVolume = callVolume;
    return HuAudFXVolSet(handle, (s16)soundVolume) == 0;
}

void fn_1_C104(s16 cameraMask, s16 voiceIndex, const Vec *position)
{
    HU3D_CAMERA *cameraIterator = Hu3DCamera;
    HU3D_CAMERA *selectedCamera;
    HU3D_CAMERA *camera;
    int cameraIndex = 0;
    Vec difference;
    CurveParameters *range;
    f32 distance;
    f32 weight;
    int voiceVolume;
    int callVolume;
    int volume;

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
    range = &lbl_1_data_468;
    if (distance < range->minimum)
        distance = range->minimum;
    if (distance > range->maximum)
        distance = range->maximum;

    weight = (f32)(1.0 - (double)((distance - range->minimum) /
                                  (range->maximum - range->minimum)));
    weight *= weight;
    volume = (int)(range->start +
                   weight * (f32)(range->end - range->start));
    callVolume = volume;
    voiceVolume = callVolume;
    fn_80060F40(voiceIndex, (s16)voiceVolume);
}

BOOL fn_1_C29C(int handle, short pitch)
{
    return HuAudFXPitchSet(handle, pitch) == 0;
}

CurveParameters lbl_1_data_468 = {32, 110, 20000.0f, 100.0f};
