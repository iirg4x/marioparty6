#ifndef M663DLL_CAMERA_H
#define M663DLL_CAMERA_H

#include "dolphin/types.h"

typedef struct M663CameraRecord {
    s32 cameraIndex;
    u8 unobservedCameraData[0x0C]; /* Unknown data, zeroed at initialization. */
    f32 rotation[3];
    f32 position[3];
    u8 unobservedViewData[0x18]; /* Unknown data, zeroed at initialization. */
    f32 viewport[6];
    f32 fieldOfView;
} M663CameraRecord;

typedef struct M663CameraState {
    M663CameraRecord cameras[4];
    u8 unobservedStateData[8]; /* Unknown data, zeroed at initialization. */
} M663CameraState;

extern M663CameraState lbl_1_bss_1428;

s32 fn_1_38D4(f32 *value, f32 step, f32 start, f32 end);

#endif
