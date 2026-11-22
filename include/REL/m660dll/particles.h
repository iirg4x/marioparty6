/* Particle allocation records for Boo'd Off the Stage. */
#ifndef REL_M660_PARTICLES_H
#define REL_M660_PARTICLES_H
#include "dolphin/types.h"
#include "REL/particle_parameters.h"

/* Effect models and their per-entry particle work. */
typedef struct M660ParticleModuleData {
    s16 modelIds[4];
    s16 unk08[4]; /* The game's code never reads this field. */
    ParticleParameters *allocation[4];
} M660ParticleModuleData;

extern M660ParticleModuleData lbl_1_bss_2484;
#endif
