#ifndef M645_ROUTE_H
#define M645_ROUTE_H

#include "dolphin/types.h"

typedef struct M645RouteEntry {
    s32 childCount;
    struct M645RouteEntry **children;
    s32 pathIndex;
} M645RouteEntry;

typedef struct M645SceneWork M645SceneWork;
typedef struct M645RouteObject {
    M645SceneWork *scene;
    s16 stageIndex;
    s16 routeIndex;
    s16 displayModel;
    s16 motionModel;
    s32 phase;
    s32 timer;
    s16 motions[4];
} M645RouteObject;

struct M645SceneWork {
    M645SceneWork **siblings;
    M645RouteObject *routeObjects[25];
    s16 stageIndex;
    s16 activeRouteCount;
    s32 selectionFlag;
    s16 characterModels[2];
    s16 stageModels[2];
    s16 portraitMotion;
    s16 routeMarkers[12];
    s16 routeModels[12];
    s16 routeMotions[12][2];
};
struct M645CharacterSetupWork;
s32 fn_1_FA1C(struct M645CharacterSetupWork *controller);
s32 fn_1_FAF8(struct M645CharacterSetupWork *controller);

#endif
