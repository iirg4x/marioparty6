#ifndef REL_MINIGAME_MESH_EDGES_H
#define REL_MINIGAME_MESH_EDGES_H

#include "dolphin/mtx.h"

/* Endpoints in model space and the matrix that takes them to world space. */
typedef struct MinigameMeshEdge {
    Vec first;
    Vec second;
    Mtx *matrix;
} MinigameMeshEdge;

/* World-space endpoints and the associated collision plane. */
typedef struct MinigameWorldEdge {
    Vec first;
    Vec second;
    Vec normal;
    f32 planeConstant;
} MinigameWorldEdge;

#endif
