#ifndef M645_CHARACTER_H
#define M645_CHARACTER_H

#include "game/object.h"
#include "REL/m645DLL/route.h"

typedef struct M645CharacterSetupWork {
    OMOBJ *mainObject;
    OMOBJ *routeObject;
    s16 particleModelId;
    s16 displayModelId;
    s16 playerNo;
    s16 routePlayerNo;
    s16 characterNo;
    s16 motionNo;
    f32 facing;
    HuVecF position;
    s32 setupMode;
    s32 assetSelection;
    s16 sideIndex;
    s16 pathState;
    s16 routeIndex;
    s16 motionState;
    s16 pairedPlayerNo;
    u8 beforeOverlayActive[2];
    s32 overlayActive;
    s16 shadowModelId;
    u8 beforeModelHeight[2];
    f32 modelHeight;
    s16 variation;
    s16 leftModelId;
    /* The fourth right-model motion shares the word cleared during setup. */
    union {
        struct {
            s16 modelId;
            s16 motionIds[4];
        } model;
        struct {
            s16 modelId;
            s16 motionIds[3];
            s32 resetWord;
        } setup;
    } right;
    M645RouteEntry *currentRoute;
    M645RouteEntry *nextRoute;
    s32 collisionFlag;
    s16 segmentNo;
    s16 setupState;
    s16 randomAngle;
    s16 randomDirection;
} M645CharacterSetupWork;

typedef struct M645LinkedModelWork {
    M645CharacterSetupWork *character;
    s16 modelId;
    u8 beforeOverlay[2];
    s32 overlayActive;
    s16 motionNo;
} M645LinkedModelWork;
void fn_1_6B14(OMOBJ *obj);

#endif
