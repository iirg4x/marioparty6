#ifndef M645_SHARED_TYPES_H
#define M645_SHARED_TYPES_H
#include "game/object.h"

typedef struct M645Sphere { HuVecF center; float radius; } M645Sphere;
typedef struct M645Segment { HuVecF start, delta; } M645Segment;
typedef struct M645Triangle { HuVecF start, edge1, edge2; } M645Triangle;
typedef struct M645Bounds { HuVecF min, max; } M645Bounds;
typedef struct M645Capsule { M645Segment segment; float radius; } M645Capsule;
#endif
