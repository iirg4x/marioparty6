#ifndef REL_MINIGAME_LIGHT_H
#define REL_MINIGAME_LIGHT_H

#include "game/hu3d.h"
#include "game/object.h"

/* Light state and update callback attached to a minigame scene object. */
typedef struct MinigameLightWork {
    HU3D_LIGHTID lightId;
    OMOBJ_FUNC callback;
    HuVecF position;
    HuVecF target;
    HuVecF direction;
    GXColor color;
} MinigameLightWork;

/* Position, direction and color used to initialize a stage light. */
typedef struct LightInitState {
    HU3D_LIGHTID id;
    HuVecF position;
    HuVecF orientation;
    HuVecF aim;
    HuVecF direction;
    float range;
    GXColor color;
} LightInitState;

#endif
