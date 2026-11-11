#ifndef MICQUIZISHIDLL_SHARED_TYPES_H
#define MICQUIZISHIDLL_SHARED_TYPES_H

#include "dolphin/types.h"
#include "game/main.h"

typedef struct QuizParticleWork {
    Vec position;
    Vec origin;
    Vec offset;
    Vec velocity;
    f32 velocityScale;
    f32 angle;
    s32 frame;
    Vec rotation;
    Vec scale;
    s32 side;
    s32 modelVariant;
    s32 positionIndex;
    s32 state;
    s16 model;
    u8 unk66[0x2]; /* No code names this field. */
    s32 active;
} QuizParticleWork;

/* Three 32-bit fields consumed by fn_1_1FB04 and fn_1_22058. */
typedef struct {
    s32 itemCount;
    s32 minCount;
    s32 maxCountExclusive;
} MicquizSelectionConfig;

typedef struct QuizMotionWork {
    Vec position;
    Vec referencePosition;
    f32 motionSpeed;
    s32 frame;
    Vec rotation;
    Vec scale;
    Vec positionStep;
    f32 rotationStepX;
    f32 rotationStepY;
    f32 rotationStepZ;
    s32 variantIndex;
    s32 state;
    s32 modelNameIndex;
    s32 modelPairIndex;
    s32 motionState;
    s16 modelId;
    s16 jointMotionIds[2];
} QuizMotionWork;

#endif
