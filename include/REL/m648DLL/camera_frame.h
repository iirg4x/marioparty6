#ifndef REL_M648_CAMERA_FRAME_H
#define REL_M648_CAMERA_FRAME_H

extern "C" {
#include "dolphin/mtx.h"
}

struct CameraPose;

class CameraFrame {
public:
    Vec position, up, target;
    void fn_1_103D0(float firstAngle, float secondAngle);
    void fn_1_10494(float angle);
    void fn_1_12BBC(const CameraPose *pose);
    void fn_1_12DD0(const CameraFrame *first, const CameraFrame *second, float weight);
    void fn_1_1347C() const;
    void fn_1_25A64();
};

void interpolate_camera_poses(CameraFrame *, const CameraPose *, const CameraPose *, float);

#endif
