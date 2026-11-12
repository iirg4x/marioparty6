#ifndef REL_M653DLL_OBJECT_CALLS_H
#define REL_M653DLL_OBJECT_CALLS_H
#include "dolphin/mtx.h"
struct Car;
void fn_1_6C0(s32 soundIndex);
void fn_1_FC4();
void fn_1_1000();
void fn_1_1004();
void fn_1_1008(Car *, Vec *);
void fn_1_1024(const Car *, Vec *);
void fn_1_1040(const Car *, Vec *);
void fn_1_1068(Car *);
unsigned char fn_1_10A4(Car *);
void fn_1_10C0(Car *, const Vec *);
void fn_1_1424(Car *, const Mtx);
void fn_1_1498(Car *);
void fn_1_1508(Car *);
extern "C" {
void fn_1_2558(Car *);
}
#endif
