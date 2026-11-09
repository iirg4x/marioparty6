#ifndef REL_M603DLL_CAMERA_FRAME_H
#define REL_M603DLL_CAMERA_FRAME_H

#include "dolphin/mtx.h"

struct CameraPose {
    Vec position;
    Quaternion rotation;
};

class CameraFrame {
public:
    Vec position;
    Vec up;
    Vec target;

    void fn_1_E5D4(float firstAngle, float secondAngle);
    void fn_1_10DC0(const CameraPose *pose);
    void fn_1_10E60(CameraPose *pose) const;
    void fn_1_10EC4(const CameraPose *a, const CameraPose *b, float t);
    void fn_1_10FD4(const CameraFrame *a, const CameraFrame *b, float t);
};

void cameraInverseLookAt(const CameraFrame *camera, Mtx matrix);

#endif
