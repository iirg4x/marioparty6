#ifndef REL_M663DLL_MOTION_H
#define REL_M663DLL_MOTION_H

#include "dolphin/types.h"

typedef struct M663PlayerMotionWork {
    s16 motionIds[8];
    s16 motionIndex;
    s16 previousMotionIndex;
    f32 motionSpeed;
    f32 previousMotionSpeed;
    f32 motionTime;
    f32 previousMotionTime;
    f32 motionLength;
    u32 modelAttributes;
    u32 previousModelAttributes;
    u32 attributesToClear;
    s32 motionEnded;
    s16 holdMotion;
    s16 nextMotionIndex;
    f32 nextMotionTime;
    u32 nextModelAttributes;
    u32 nextAttributesToClear;
    s16 shiftState;
    s16 shiftMotionIndex;
    u32 shiftAttributes;
    f32 shiftStartTime;
    f32 shiftDuration;
    f32 currentShiftTime;
    f32 shiftLength;
    /* Unknown bytes at 0x60..0x64: the target uses a 0x64 stride and
     * clears the complete 0xC8-byte lbl_1_bss_688 array. No field reads. */
    u8 unobserved[4];
} M663PlayerMotionWork;

extern M663PlayerMotionWork lbl_1_bss_688[2];

#endif
