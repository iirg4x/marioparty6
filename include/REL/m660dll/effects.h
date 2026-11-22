/* Particle emitter settings and moving-slot and path ranges selected by effect. */
#ifndef REL_M660_EFFECTS_H
#define REL_M660_EFFECTS_H
#include "dolphin/types.h"

/* Particle emitter settings for one effect. */
typedef struct M660ParticleEffectConfig {
    s16 particleCount;
    s16 alpha;
    f32 opacityChange;
    f32 horizontalRadius;
    f32 spawnRadiusLimit;
    f32 spawnHeight;
    f32 baseParticleScale;
    s16 alternateAlpha;
    s16 unk1A; /* No code names this field. */
    f32 forwardDistance;
    f32 sideDistance;
    f32 alternateRadiusLimit;
    f32 alternateParticleScale;
} M660ParticleEffectConfig;

/* Inclusive moving-slot indices for one effect. */
typedef struct M660MovingSlotRange {
    s16 firstSlotIndex;
    s16 lastSlotIndex;
} M660MovingSlotRange;

/* Contiguous path indices available to one effect. */
typedef struct M660PathChoiceRange {
    s16 pathCount;
    s16 firstPathIndex;
} M660PathChoiceRange;

#endif
