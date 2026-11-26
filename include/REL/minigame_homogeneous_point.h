#ifndef REL_MINIGAME_HOMOGENEOUS_POINT_H
#define REL_MINIGAME_HOMOGENEOUS_POINT_H


typedef struct MinigameHomogeneousPoint {
    float x, y, z; /* Transformed point coordinates. */
    float w;       /* Homogeneous coordinate. */
} MinigameHomogeneousPoint;

#endif
