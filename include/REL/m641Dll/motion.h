#ifndef REL_M641DLL_MOTION_H
#define REL_M641DLL_MOTION_H

#include "dolphin/types.h"

typedef struct {
    s16 motionId;       /* Character motion-table index, or -1 when unused. */
    f32 blendTime;      /* Motion blend duration in frames. */
    u32 attributes;     /* Character-model attributes applied with this motion. */
    s32 voiceEnabled;   /* Nonzero enables character voice and effect cues. */
} M641MotionInfo;

extern M641MotionInfo lbl_1_data_344[14];

#endif
