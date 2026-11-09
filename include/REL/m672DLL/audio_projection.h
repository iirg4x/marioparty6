#ifndef REL_M672DLL_AUDIO_PROJECTION_H
#define REL_M672DLL_AUDIO_PROJECTION_H
#include "dolphin/types.h"

typedef struct CameraRange { s32 minimum, maximum; f32 horizontalScale; } CameraRange;
typedef struct CurveParameters { s32 start, end; f32 maximum, minimum; } CurveParameters;
#ifdef __cplusplus
extern "C" {
#endif
extern CameraRange lbl_1_data_618[];
extern CurveParameters lbl_1_data_6D8;
extern const f32 lbl_1_rodata_6C0, lbl_1_rodata_6C4;
extern const f32 lbl_1_rodata_6D8, lbl_1_rodata_6DC, lbl_1_rodata_6E0, lbl_1_rodata_6E4;
#ifdef __cplusplus
}
#endif
#endif
