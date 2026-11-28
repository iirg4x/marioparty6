/* Light, fog, camera, and positional-audio state shared by the scene utilities. */
#ifndef REL_M674DLL_SCENE_H
#define REL_M674DLL_SCENE_H
#include "REL/minigame_camera_work.h"
#include "REL/minigame_fog_work.h"
#include "REL/minigame_positional_audio.h"
#include "REL/minigame_camera_fit.h"
#include "REL/minigame_fog.h"
#include "REL/minigame_light.h"
#include "game/object.h"
typedef void (*CameraCallback)(OMOBJ *, u32, s32);
void fn_1_F8C(OMOBJMAN *manager);
void fn_1_1054(OMOBJMAN *manager, Vec *position, Vec *target, GXColor *color);
void fn_1_173C(OMOBJMAN *manager, u32 cameras, f32 fov, f32 nearPlane,
    f32 farPlane, f32 shadowRadius, OM_CAMERA_VIEW *view);
void fn_1_2398(CameraCallback callback);
void fn_1_3308(OMOBJMAN *manager, MinigameAudioRange *ranges);
f32 fn_1_4E18(Vec *first, Vec *second);
void fn_1_4F84(Point3d *start, Point3d *end, f32 factor, Point3d *out);

/* Scene setup, queries, and updates. */
void fn_1_21A8(s32 camera, Vec *direction, Vec *target, float fov, float radius);
void fn_1_289C(u32 cameraMask);
s32 fn_1_4040(s32 effect, Vec *position);
void fn_1_47F4(int effect, int handle, Vec *position, int flag, int value);
float fn_1_4BAC(float start, float end, float factor);
f32 fn_1_4CC8(f32 start, f32 end);
float fn_1_4D7C(HuVecF *firstPosition, HuVecF *secondPosition);
s32 fn_1_4A9C(s32 model, s32 dataNum);
s32 fn_1_4FDC(Vec *start, Vec *end, f32 gravity, f32 initialY, Vec *velocity);

int fn_1_4994(int dataNum);
s32 fn_1_36BC(Vec *position);

#endif
