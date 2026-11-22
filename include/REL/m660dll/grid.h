/* Collision-grid points and bounds used by Boo'd Off the Stage. */
#ifndef REL_M660_GRID_H
#define REL_M660_GRID_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

/* Position and flags for a collision-grid point. */
typedef struct M660GridPoint {
    s16 flags;
    s16 unk02; /* No code names this field. */
    Vec position;
} M660GridPoint;

/* Collision grid dimensions and world bounds. */
typedef struct M660Grid {
    s16 count;
    s16 columns;
    s16 rows;
    s16 unk06; /* No code names this field. */
    Vec minimum;
    Vec maximum;
    Vec spacing;
    f32 radius;
} M660Grid;

extern M660GridPoint lbl_1_bss_2688[256];
extern M660Grid lbl_1_bss_3688;
#endif
