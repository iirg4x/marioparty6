/* Curved travel paths for the Solo guide and selected characters. */
#ifndef REL_MDSINGDLL_MOTION_PATH_H
#define REL_MDSINGDLL_MOTION_PATH_H

#include "game/hu3d.h"

typedef struct MdsingMotionPath {
    /* Owning callback's movement phase; 2 marks a completed path. */
    s16 state;
    /* Starting, control, and destination points in world coordinates. */
    HuVecF position;
    HuVecF controlPosition;
    HuVecF endPosition;
    /* Elapsed and total path duration, in scene updates. */
    f32 elapsed;
    f32 duration;
} MDSING_MOTION_PATH;

typedef MDSING_MOTION_PATH LBL_1_BSS_228_ENTRY;
typedef MDSING_MOTION_PATH LBL_1_BSS_1C8_ENTRY;
extern MDSING_MOTION_PATH lbl_1_bss_CF4[4];
extern MDSING_MOTION_PATH lbl_1_bss_DB4[2];
extern MDSING_MOTION_PATH lbl_1_bss_E14[2];
extern f32 lbl_1_bss_C94[6];

#endif
