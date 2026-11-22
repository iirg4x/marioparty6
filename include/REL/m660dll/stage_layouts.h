/* Position and rotation presets for the fourteen stage models at indices 31 through 44. */
#ifndef REL_M660_STAGE_LAYOUTS_H
#define REL_M660_STAGE_LAYOUTS_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"

/* Initial transform of a stage model. */
typedef struct M660StageObjectLayout {
    Vec position;
    Vec rotation;
} M660StageObjectLayout;

#endif
