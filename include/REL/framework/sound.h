// Camera-relative sound shared by the minigames (sound.c). The names are descriptive, not recovered.
#ifndef REL_FRAMEWORK_SOUND_H
#define REL_FRAMEWORK_SOUND_H

#ifdef __cplusplus
extern "C" {
#endif

#include "dolphin/mtx.h"

// HU3D_CAMERA of game/hu3d.h, which the callers need not include.
typedef struct Hu3DCamera_s Hu3DCamera_s;

// One camera's sound projection: the pan range and the horizontal view scale.
typedef struct CameraSoundParam CameraSoundParam;
struct CameraSoundParam {
    s32 minimum;
    s32 maximum;
    f32 horizontalScale;
};

Hu3DCamera_s *soundCamera(short mask);
CameraSoundParam *soundCameraParam(short mask);
f32 soundProject(Vec *sourcePoint, Vec *result, Hu3DCamera_s *camera, CameraSoundParam *param);
s32 soundVolumeAtDistance(f32 value);
s16 soundPanFromPosition(f32 *fraction, CameraSoundParam *bounds);
int soundPlay(int soundId);
void soundStop(int soundNo);
void soundSetVolumeCurve(int farVolume, int nearVolume, float farDistance, float nearDistance);
void soundSetCameraPan(short mask, int minimum, int maximum, float horizontalScale);
void soundSetCameraLayout(s32 mode);
s32 soundVolumeAt(s16 cameraMask, Point3d *target);
s16 soundPanAt(s16 cameraMask, Vec *targetPoint);
void soundVolumeAndPanAt(s16 cameraMask, Vec *targetPoint, s32 *volume, s32 *pan);
int soundPlayAt(s16 cameraMask, s32 effectId, Point3d *targetPoint);
void soundPlayAtDelayed(s16 cameraMask, s32 effectId, Point3d *targetPoint, u16 delay);
void soundPlayAtWithVolume(s16 cameraMask, s32 soundId, Point3d *targetPoint);
s16 soundPanTo(s16 cameraMask, s32 soundId, Point3d *targetPoint);
void soundPanCharacterVoice(s16 cameraMask, s16 characterId, Point3d *targetPoint);
int soundSetVolumeAt(s16 cameraMask, s32 handle, const Vec *position);
void soundSetCharacterVoiceVolumeAt(s16 cameraMask, s16 voiceIndex, const Vec *position);
int soundSetPitch(int handle, short pitch);

#ifdef __cplusplus
}
#endif

#endif
