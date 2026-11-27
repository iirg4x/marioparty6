#ifndef REL_MINIGAME_COURSE_CAMERA_H
#define REL_MINIGAME_COURSE_CAMERA_H

#include "dolphin/mtx.h"

/* Course camera preset with rotation, target, and projection settings. */
typedef struct MinigameCourseCamera {
    s32 unk00; /* The game's code never reads this field. */
    u8 unk04[12]; /* No code names this field. */
    Vec rotation;
    Vec target;
    u8 unk28[12]; /* No code names this field. */
    /* Indices 0/1: orbit distance/aspect; 2 through 5: viewport x/y/width/height. */
    f32 projectionAndViewport[6];
    f32 fieldOfView;
} MinigameCourseCamera;

#endif
