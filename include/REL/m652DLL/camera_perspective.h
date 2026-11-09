#ifndef REL_M652DLL_CAMERA_PERSPECTIVE_H
#define REL_M652DLL_CAMERA_PERSPECTIVE_H

#include "dolphin/types.h"

#ifdef __cplusplus
class CameraPerspective {
public:
#else
struct CameraPerspective {
#endif
    int width, height;
    int insetX, insetY;
    int borderX, borderY;
    float fovRadians;
#ifdef __cplusplus
    CameraPerspective();
    void read(s32 cameraBit);
    void fn_1_C92C(int cameraBit);
#endif
};

#endif
