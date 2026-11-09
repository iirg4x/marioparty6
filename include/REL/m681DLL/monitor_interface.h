#ifndef M681_MONITOR_INTERFACE_H
#define M681_MONITOR_INTERFACE_H
#include "dolphin/types.h"

struct Monitor;

struct InitializationValues {
    s32 values[4];
};

void fn_1_381C(Monitor *, InitializationValues *, f32);

#endif
