#ifndef M654DLL_CAMERA_SETUP_H
#define M654DLL_CAMERA_SETUP_H
#include "REL/m654DLL/camera_types.h"
struct CameraSetup {
    CameraFrame frame;
    unsigned char highFlag : 1, middleFlag : 1, lowFlag : 1, remainingFlagBits : 5;
    CameraSetup();
};
#endif
