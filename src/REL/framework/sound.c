// Camera-relative sound: per-camera pan ranges, the distance-to-volume curve and the sound-effect calls that use
// them. Shared by the minigames that link it (the same 0x23DC bytes just before heap.cpp in 19 of the 21 C++
// minigames).
//
// The file name is not recovered: the code has no assert or $Id string. The language is inferred: the two
// comparisons return int, as m639's one caller that tests a result reads it (cmpwi), and C is the only language in
// which that one declaration also gives the definitions' bytes. As C, math.h's sqrtf statics are named as
// player_table_1vs1.cpp's (m624, m652, m654 and m655 keep only the first copy) and not as the C++ game files'
// (m622, m648, m672 and m681 keep this file's own). The function and data names are descriptive, not recovered.
#include "math.h"
#include "dolphin/mtx.h"
#include "game/hu3d.h"
#include "game/charman.h"

// The sound calls as this file uses them; game/audio.h declares three of them with other parameter types.
int HuAudFXPlay(int seId);
void HuAudFXStop(int seNo);
int HuAudFXPlayPan(int effectId, s16 pan);
int HuAudFXPlayDelayPan(int effectId, s16 pan, u16 delay);
int HuAudFXPlayVolPan(int soundId, s16 volume, s32 pan);
void HuAudFXPanning(int seNo, s16 pan);
s32 HuAudFXVolSet(s32 handle, s16 volume);
s32 HuAudFXPitchSet(int seNo, s16 pitch);

#include "REL/framework/sound.h"

// Distance-to-volume curve shared by every camera: the volume at the far and near distance limits.
typedef struct SoundVolumeCurve SoundVolumeCurve;
struct SoundVolumeCurve {
    s32 farVolume;
    s32 nearVolume;
    f32 farDistance;
    f32 nearDistance;
};

// Each camera's pan range and view scale; only camera 0 starts configured.
CameraSoundParam soundCameraParams[16] = {{0x20, 0x60, 1.2f}};
// Volume 110 at 100 units or nearer, falling to 32 at 20000 units.
SoundVolumeCurve soundVolumeCurve = {32, 110, 20000.0f, 100.0f};

HU3D_CAMERA *soundCamera(short mask)
{
    HU3D_CAMERA *camera = Hu3DCamera;
    int i;
    for (i = 0; i < 16; i++, camera++)
        if (mask & (1U << i)) return camera;
    return Hu3DCamera;
}

CameraSoundParam *soundCameraParam(short mask)
{
    CameraSoundParam *param = soundCameraParams;
    int i;
    for (i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) return param;
    return soundCameraParams;
}

f32 soundProject(Vec *sourcePoint, Vec *result, HU3D_CAMERA *camera,
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

s32 soundVolumeAtDistance(f32 value)
{
    SoundVolumeCurve *curveParameters = &soundVolumeCurve;
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

s16 soundPanFromPosition(f32 *fraction, CameraSoundParam *bounds)
{
    s32 midpoint = (bounds->maximum + bounds->minimum) / 2;
    s32 halfRange = (bounds->maximum - bounds->minimum) / 2;
    s32 value = (s32)((f32)midpoint + (f32)halfRange * *fraction);
    return (s16)(value < bounds->minimum ? bounds->minimum :
                 value > bounds->maximum ? bounds->maximum : value);
}

int soundPlay(int soundId)
{
    return HuAudFXPlay(soundId);
}

void soundStop(int soundNo)
{
    HuAudFXStop(soundNo);
}

void soundSetVolumeCurve(int farVolume, int nearVolume, float farDistance, float nearDistance)
{
    SoundVolumeCurve *param = &soundVolumeCurve;
    param->farVolume = farVolume;
    param->nearVolume = nearVolume;
    param->nearDistance = nearDistance;
    param->farDistance = farDistance;
}

void soundSetCameraPan(short mask, int minimum, int maximum, float horizontalScale)
{
    CameraSoundParam *param = soundCameraParams;
    int i;
    for (i = 0; i < 16; i++, param++)
        if (mask & (1U << i)) {
            param->minimum = minimum;
            param->maximum = maximum;
            param->horizontalScale = horizontalScale;
            break;
        }
}

void soundSetCameraLayout(s32 mode)
{
    switch (mode) {
    case 0:
        soundSetCameraPan(1, 0x20, 0x60, 1.2f);
        break;
    case 1:
        soundSetCameraPan(3, 0x20, 0x60, 3.2f);
        break;
    case 2:
        soundSetCameraPan(1, 0x20, 0x40, 0.8f);
        soundSetCameraPan(2, 0x40, 0x60, 0.8f);
        break;
    case 3:
        soundSetCameraPan(5, 0x20, 0x40, 1.2f);
        soundSetCameraPan(0xA, 0x40, 0x60, 1.2f);
        break;
    case 4:
        soundSetCameraPan(1, 0x30, 0x30, 0.4f);
        soundSetCameraPan(2, 0x38, 0x38, 0.4f);
        soundSetCameraPan(4, 0x48, 0x48, 0.4f);
        soundSetCameraPan(8, 0x50, 0x50, 0.4f);
        break;
    }
}

s32 soundVolumeAt(s16 cameraMask, Point3d *target)
{
    HU3D_CAMERA *camera = soundCamera(cameraMask);
    Vec difference;
    PSVECSubtract(&camera->pos, target, &difference);
    return soundVolumeAtDistance(PSVECMag(&difference));
}

s16 soundPanAt(s16 cameraMask, Vec *targetPoint)
{
    CameraSoundParam *param = soundCameraParam(cameraMask);
    Vec projected;
    soundProject(targetPoint, &projected, soundCamera(cameraMask), param);
    return soundPanFromPosition(&projected.x, param);
}

void soundVolumeAndPanAt(s16 cameraMask, Vec *targetPoint, s32 *volume, s32 *pan)
{
    CameraSoundParam *param = soundCameraParam(cameraMask);
    Vec projected;
    f32 distance = soundProject(targetPoint, &projected, soundCamera(cameraMask), param);
    *volume = (s16)soundVolumeAtDistance(distance);
    *pan = soundPanFromPosition(&projected.x, param);
}

int soundPlayAt(s16 cameraMask, s32 effectId, Point3d *targetPoint)
{
    return HuAudFXPlayPan(effectId, soundPanAt(cameraMask, targetPoint));
}

void soundPlayAtDelayed(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay)
{
    HuAudFXPlayDelayPan(effectId, soundPanAt(cameraMask, targetPoint), delay);
}

// soundPlayAtWithVolume's s16 volume path. These two duplicates of soundVolumeAtDistance and
// soundVolumeAndPanAt are open debt: the natural forms tried (calling soundVolumeAndPanAt with an s32 volume, or
// assigning soundVolumeAtDistance's result to an s16) do not compile to its bytes.
static inline s16 soundVolumeForPlay(f32 value)
{
    SoundVolumeCurve *curveParameters = &soundVolumeCurve;
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

static inline void soundVolumeAndPanForPlay(s16 cameraMask, Point3d *targetPoint, s16 *volume, s32 *pan)
{
    CameraSoundParam *param = soundCameraParam(cameraMask);
    Vec projected;
    f32 distance = soundProject(targetPoint, &projected, soundCamera(cameraMask), param);
    *volume = soundVolumeForPlay(distance);
    *pan = soundPanFromPosition(&projected.x, param);
}

void soundPlayAtWithVolume(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    s16 volume;
    s32 pan;
    soundVolumeAndPanForPlay(cameraMask, targetPoint, &volume, &pan);
    HuAudFXPlayVolPan(soundId, volume, pan);
}

s16 soundPanTo(s16 cameraMask, s32 soundId, Point3d *targetPoint)
{
    int pan = soundPanAt(cameraMask, targetPoint);
    HuAudFXPanning(soundId, pan);
    return pan;
}

void soundPanCharacterVoice(s16 cameraMask, s16 characterId, Point3d *targetPoint)
{
    fn_80060F14(characterId, soundPanAt(cameraMask, targetPoint));
}

int soundSetVolumeAt(s16 cameraMask, s32 handle, const Vec *position)
{
    HU3D_CAMERA *camera = soundCamera(cameraMask);
    Vec difference;
    s32 volume;
    PSVECSubtract(&camera->pos, position, &difference);
    volume = soundVolumeAtDistance(PSVECMag(&difference));
    return HuAudFXVolSet(handle, volume) == 0;
}

void soundSetCharacterVoiceVolumeAt(s16 cameraMask, s16 voiceIndex, const Vec *position)
{
    HU3D_CAMERA *camera = soundCamera(cameraMask);
    Vec difference;
    s32 volume;
    PSVECSubtract(&camera->pos, position, &difference);
    volume = soundVolumeAtDistance(PSVECMag(&difference));
    fn_80060F40(voiceIndex, volume);
}

int soundSetPitch(int handle, short pitch)
{
    return HuAudFXPitchSet(handle, pitch) == 0;
}
