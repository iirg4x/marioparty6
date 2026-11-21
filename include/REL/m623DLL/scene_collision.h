#ifndef REL_M623DLL_SCENE_COLLISION_H
#define REL_M623DLL_SCENE_COLLISION_H

struct TriangleCollision;

// Creates a collision query for the course geometry.
TriangleCollision *fn_1_1A2C();

// Samples the terrain height at X and Z.
float fn_1_1ACC(float x, float z);

#endif
