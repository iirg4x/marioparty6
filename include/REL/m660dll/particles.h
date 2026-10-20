/* Particle parameter, entry and allocation records for Boo'd Off the Stage. */
#ifndef REL_M660_PARTICLES_H
#define REL_M660_PARTICLES_H
#include "dolphin/types.h"
#include "dolphin/mtx.h"
#include "game/animdata.h"

struct Hu3DModel_s;
struct Hu3DParticle_s;

typedef struct M660ParticleColor {
    u8 red;
    u8 green;
    u8 blue;
    u8 alpha;
} M660ParticleColor;

typedef struct M660ParticleParameters {
    u32 value00;
    f32 values04[3];
    M660ParticleColor color;
    u8 finalColor[4];
    f32 velocityScale[3];
    f32 velocityYAcceleration;
    f32 motionValueDelta;
    f32 height;
    f32 heightScale;
    f32 opacityChange;
    f32 colorChangeRate;
} M660ParticleParameters;

typedef struct M660ParticleEntry {
    s16 state;
    s16 ownerIndex;
    s16 attributes;
    s16 cameraBits;
    Vec velocity;
    Vec origin;
    f32 speedDecay;
    f32 colorIndex;
    f32 baseScale;
    f32 height;
    f32 scaleY;
    f32 motionValue;
    Vec position;
    M660ParticleColor color;
} M660ParticleEntry;

typedef struct M660ParticleContext {
    s16 dataCount;
    s16 lastEntry;
    Vec position;
    Vec unknownVector;
    M660ParticleParameters *parameters;
    s16 animationBank;
    s16 animationNumber;
    f32 animationSpeed;
    f32 animationTime;
    u8 blendMode;
    u8 attributes;
    s16 unknown2E;
    s16 entryCount;
    u32 count;
    u32 previousCounter;
    u32 previousCount;
    u32 displayListSize;
    ANIMDATA *animation;
    M660ParticleEntry *entries;
    Vec *vertexBuffer;
    void *displayListBuffer;
    void (*hook)(struct Hu3DModel_s *model, struct Hu3DParticle_s *particle, Mtx matrix);
} M660ParticleContext;

typedef struct M660ParticleModuleData {
    s16 modelIds[4];
    s16 maximumCounts[4];
    void *allocation[4];
} M660ParticleModuleData;

extern M660ParticleModuleData lbl_1_bss_2484;
#endif
