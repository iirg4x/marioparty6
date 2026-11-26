/* Falling-piece transforms and shared model settled for quiz result effects. */
#ifndef REL_MICQUIZDLL_MODEL_WORK_H
#define REL_MICQUIZDLL_MODEL_WORK_H

#include <dolphin/mtx.h>

typedef struct MicquizModelRecord_s {
    s32 active; /* Nonzero while this piece is visible. */
    s32 settled; /* Zero while falling; one after reaching the floor. */
    Vec position; /* World position, in scene units. */
    Vec velocity; /* Position change applied once per scene update. */
    Vec rotation; /* Euler rotation in degrees. */
    Vec angularVelocity; /* Rotation change in degrees per scene update. */
    GXColor color; /* RGB tint applied to the shared model materials. */
    s32 settledFrames; /* Number of updates since the piece reached the floor. */
} MicquizModelRecord;

typedef struct MicquizModelWork_s {
    HU3D_MODELID bodyModel; /* Shared piece geometry, hidden from ordinary model drawing. */
    HU3D_MODELID hookModel; /* Drawing hook that renders the active piece records. */
    f32 floorHeight; /* World Y coordinate at which pieces settle. */
    s32 activeRecordCount; /* Number of records currently in use. */
} MicquizModelWork;

#endif
