/* Work data for the bridge's timed wave and impact effects. */
#ifndef REL_MGMBATTLEDLL_EFFECTS_H
#define REL_MGMBATTLEDLL_EFFECTS_H

#include "dolphin/mtx.h"

#include "game/object.h"

typedef struct MgBattleWaveVertex_s {
    /* Vertex position in the stage model's local coordinate space. */
    Point3d position;
    /* Blend weight applied to this vertex during the wave. */
    f32 weight;
} MgBattleWaveVertex;

typedef struct MgBattleWaveWork_s {
    /* Nonzero while this stage model has an active wave effect. */
    s16 active;
    /* Frames elapsed since the wave started. */
    f32 elapsed;
    /* Wave lifetime in frames. */
    f32 duration;
    /* Vertices modified by the active wave effect. */
    MgBattleWaveVertex *vertices;
} MgBattleWaveWork;

extern MgBattleWaveWork lbl_1_bss_2AC[10];
extern Point3d lbl_1_bss_100[20];
extern Point3d lbl_1_bss_294;
extern OMOBJ *lbl_1_bss_2A0;
extern OMOBJ *lbl_1_bss_364[4];

#endif
