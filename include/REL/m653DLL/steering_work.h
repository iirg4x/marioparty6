#ifndef REL_M653DLL_STEERING_WORK_H
#define REL_M653DLL_STEERING_WORK_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
struct Car;
struct M653SteeringValue {
    s32 value;
    M653SteeringValue() : value(0) {}
};

struct M653SteeringWork {
    s32 value_00;
    f32 scale_04;
    Car *owner_08;
    Vec position_0C;
    Vec target_18;
    Vec direction_24;
    Vec direction_30;
    Vec normal_3C;
    f32 distance_48;
    M653SteeringValue timer_4C;
    M653SteeringValue state_50;
    M653SteeringValue duration_54;
    u8 flag_58;
    s32 state_5C;
    s32 state_60;
    M653SteeringWork(s32 upperValue, s32 lowerValue, Car *owner);
};
#endif
