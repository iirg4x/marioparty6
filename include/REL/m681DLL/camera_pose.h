#ifndef REL_M681DLL_CAMERA_POSE_H
#define REL_M681DLL_CAMERA_POSE_H

#include "dolphin/mtx.h"

typedef struct CameraPose {
    Vec position;
    Quaternion rotation;
} CameraPose;

typedef struct CameraFrame {
    Vec position;
    Vec up;
    Vec target;
#ifdef __cplusplus
    void fn_1_D11C(const CameraPose& pose);
    void fn_1_D1BC(CameraPose& pose) const;
    void fn_1_D220(const CameraPose *firstPose, const CameraPose *secondPose, float interpolationFactor);
    void fn_1_D330(const CameraFrame& firstFrame, const CameraFrame& secondFrame, float interpolationFactor);
#endif
} CameraFrame;

#endif
