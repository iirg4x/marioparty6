#ifndef M653_BOUNDARY_PLANE_H
#define M653_BOUNDARY_PLANE_H

#include "dolphin/mtx.h"

#ifdef __cplusplus
#include "REL/framework/plane.h"
typedef Plane BoundaryPlane;
#else
typedef struct BoundaryPlane {
    Vec normal;
    f32 offset;
} BoundaryPlane;
#endif

#ifdef __cplusplus
extern "C" {
#endif
extern BoundaryPlane lbl_1_bss_70[4];
void fn_1_1E8FC(BoundaryPlane *, Vec *, Vec *);
void fn_1_9210(void);
f32 fn_1_9568(f32 x, f32 z);
void fn_1_965C(void);

#ifdef __cplusplus
}
#endif

#endif
