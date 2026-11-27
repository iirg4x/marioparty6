#ifndef REL_W02DLL_EVENT_WORK_H
#define REL_W02DLL_EVENT_WORK_H

#include "game/hu3d.h"

typedef struct W02MapMarker {
    int modelId;
    int spaceId;
    int linkedSpace;
} W02MapMarker;

typedef struct W02FlyingEffect {
    int objectId;
    int state;
    int age;
    int travelFrames;
    int remainingFrames;
    int mode;
    BOOL active;
    HuVecF position;
    HuVecF previousPosition;
    HuVecF rotation;
    HuVecF positionHistory[4];
    HuVecF start;
    HuVecF control;
    HuVecF end;
    float spinAngle;
    float spinSpeedSlope;
    float startDrift;
    float spiralRadius;
    float spinRateScale;
} W02FlyingEffect;

typedef struct W02ThrowPath {
    BOOL ready;
    int unk4; /* The game's code never reads this field. */
    int destinationSpace;
    int travelFrames;
    HuVecF start;
    HuVecF bend;
    HuVecF end;
    HuVecF trailStart;
    HuVecF trailEnd;
} W02ThrowPath;

typedef struct W02EventSpriteParticle {
    s16 maxAlpha;
    s16 alphaStep;
    u8 unk04[4]; /* No code names this field. */
    HuVecF velocity;
    u8 unk14[8]; /* No code names this field. */
    float rotationSpeed;
    u8 unk20[12]; /* No code names this field. */
    float animationTime;
    float animationSpeed;
    float size;
    HuVecF position;
    HuVecF rotation;
    GXColor color;
    s32 textureFrame;
} W02EventSpriteParticle;

typedef struct W02PathSpaces {
    int boardModelId;
    int modelId;
    int spaceCount;
    int modelSpaceCount;
    float length;
    float travelDistance;
    HuVecF end;
    HuVecF start;
    HuVecF markerStart;
    HuVecF positions[32];
    int spaceIds[32];
    int spaceTypes[32];
} W02PathSpaces;

#endif
