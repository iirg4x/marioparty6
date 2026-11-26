/* Camera lens, viewport, and stored camera-pose data for Sink or Swim. */
#ifndef REL_M625DLL_CAMERA_H
#define REL_M625DLL_CAMERA_H

#include "REL/minigame_camera.h"
#include "game/main.h"

extern MinigameCameraParams lbl_1_data_0;
extern MinigameCameraPose lbl_1_data_50;
extern MinigameCameraPose lbl_1_data_74[2];
extern MinigameCameraPose lbl_1_data_BC[2];
void fn_1_1B0(void);
s32 fn_1_41C(s16 frame);
void fn_1_CB0(void);
u8 fn_1_CB4(s16 frame);
void fn_1_1398(Point3d *result, const Point3d *point, f32 distance);

#endif
