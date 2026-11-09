#ifndef REL_M652_RANGE_OPERATIONS_H
#define REL_M652_RANGE_OPERATIONS_H
#include <cmath>
#include "REL/m652DLL/framework/range.h"
extern "C" { extern const float lbl_1_rodata_330[1]; }
template<class T> inline T Abs7CE0(T value) { if (value < lbl_1_rodata_330[0]) { return -value; } else { return value; } }


template<class T> T fn_1_7628(Range *range, T step, double rate);
template<> float fn_1_7628<float>(Range *range, float step, double rate);
template<class T> T fn_1_75EC(Range *self, T value);
template<> float fn_1_75EC<float>(Range *self, float value);

#endif
