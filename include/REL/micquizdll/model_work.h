/* Falling-piece transforms and shared model settled for quiz result effects. */
#ifndef REL_MICQUIZDLL_MODEL_WORK_H
#define REL_MICQUIZDLL_MODEL_WORK_H

typedef struct MicquizModelRecord_s {
    s32 active; /* Nonzero while this piece is visible. */
    s32 settled; /* Zero while falling; one after reaching the floor. */
    f32 position[3]; /* World position, in scene units. */
    f32 velocity[3]; /* Position change applied once per scene update. */
    f32 rotation[3]; /* Euler rotation in degrees. */
    f32 angularVelocity[3]; /* Rotation change in degrees per scene update. */
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
