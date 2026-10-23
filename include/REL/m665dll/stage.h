/* Stage weapons, hazards, and gap records for Verbal Assault. */
#ifndef REL_M665DLL_STAGE_H
#define REL_M665DLL_STAGE_H

#include "dolphin/types.h"

typedef struct M665FireEffect {
    s16 modelA;
    s16 modelB;
    s16 active;
    s16 lifetime;
    f32 scale;
    f32 opacity;
    f32 offsetX;
    f32 offsetY;
    f32 offsetZ;
    f32 velocityX;
    f32 velocityY;
    f32 velocityZ;
} M665FireEffect;

typedef struct M665MissileWork {
    s16 modelID;
    s16 collisionModelID;
    s16 state;
    s16 frames;
    f32 speed;
    f32 angle;
} M665MissileWork;

typedef struct M665KillerTrail {
    s16 modelID;
    s16 active;
    f32 scale;
    f32 opacity;
} M665KillerTrail;

typedef struct M665KillerWork {
    s16 modelID;
    s16 collisionModelID;
    M665KillerTrail trail[32];
    s16 state;
    s16 targetPlayer;
    s16 targetFrames;
    s16 elapsed;
    s16 trailFrames;
    s32 soundHandle;
    f32 horizontalSpeed;
    u8 unreadBeforeVerticalSpeed[4]; /* No M665 consumer reads these bytes; verticalSpeed follows
                                      * here. */
    f32 verticalSpeed;
    f32 angle;
} M665KillerWork;

typedef struct M665LaserWork {
    s16 modelIDs[2];
    f32 scale;
    s16 timer;
} M665LaserWork;

typedef struct M665Hazard {
    s16 model;
    s16 firstAnimation;
    s16 secondAnimation;
    s16 state;
    s16 elapsed;
    s16 reserved; /* Retained two-byte field with an unknown role. */
    f32 fallSpeed;
    f32 horizontalSpeed; /* X movement added to the hazard position each update. */
    f32 landingScaleY; /* Y scale that grows to one as the hazard lands. */
} M665Hazard;

typedef struct M665RollingWork {
    s16 modelID;
    s16 state;
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 depthSpeed;
} M665RollingWork;

typedef struct M665BouncingWork {
    s16 modelID;
    s16 state;
    s16 frames;
    f32 horizontalSpeed;
    f32 verticalSpeed;
    f32 depthSpeed;
} M665BouncingWork;

typedef struct M665StageWork {
    M665FireEffect fire[16];
    s16 fireEnabled;
    s16 fireFrames;
    s32 fireSoundHandle;
    f32 firePosition[2][3];
    M665MissileWork missiles[6];
    M665KillerWork killers[3];
    s16 selectedKiller;
    M665LaserWork laser;
    M665Hazard hazards[7];
    M665RollingWork rolling[10];
    s16 rollingCount;
    s16 rollingFrames;
    M665BouncingWork bouncing[6];
} M665StageWork;

extern M665StageWork lbl_1_bss_2E8;
extern void *lbl_1_bss_2E0;
extern s16 lbl_1_bss_2E4;

typedef struct M665StageProgress {
    f32 horizontalDistance;
    f32 reserved; /* Retained value with an unknown role. */
    f32 depthDistance;
    f32 gapDistance; /* Euclidean distance between hazard positions, or -1 before it is computed. */
} M665StageProgress;

extern M665StageProgress lbl_1_bss_280[6];

#endif
