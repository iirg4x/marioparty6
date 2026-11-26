#ifndef REL_MINIGAME_TRIANGLE_H
#define REL_MINIGAME_TRIANGLE_H

#include "humath.h"

/* Triangle stored as one vertex and the two edges starting at it. */
typedef struct MinigameTriangle {
    HuVecF start;
    HuVecF edge1;
    HuVecF edge2;
} MinigameTriangle;

#endif
