#ifndef REL_M603DLL_UFO_H
#define REL_M603DLL_UFO_H

extern "C" {
#include "game/hu3d.h"
}
struct CameraModelOwner;

void fn_1_C280(int request);

void fn_1_BB5C(void);

void fn_1_C710(CameraModelOwner *);
void fn_1_BF18();
void fn_1_C2EC();
void fn_1_C0FC();
void fn_1_C04C();
void fn_1_CD34(HU3D_MODEL *, Mtx *);

#endif
