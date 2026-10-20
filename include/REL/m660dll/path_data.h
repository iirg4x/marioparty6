/* Endpoint positions and angles for the stage's path segments. */
#ifndef REL_M660_PATH_DATA_H
#define REL_M660_PATH_DATA_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

typedef struct M660PathSegment {
    Vec points[2];
    f32 angles[2];
} M660PathSegment;

extern M660PathSegment lbl_1_data_100[28];
#endif
