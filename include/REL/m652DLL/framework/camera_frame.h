#ifndef REL_M652DLL_FRAMEWORK_CAMERA_FRAME_H
#define REL_M652DLL_FRAMEWORK_CAMERA_FRAME_H

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

    void fn_1_B0BC(float firstAngle, float secondAngle);
    void fn_1_B180(float angle);
    void fn_1_D50C(CameraPose *pose) const;
    void fn_1_D570(const CameraPose *first, const CameraPose *second, float interpolation);
    void fn_1_D680(const CameraFrame *firstCamera, const CameraFrame *secondCamera,
                   float interpolation);
};

#endif
