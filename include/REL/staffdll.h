/* Staff-roll state shared by the module's code. The motion record type is declared here;
* its globals are defined in staff.c after the functions. */
/* Shared state declarations for the staff-roll overlay. */
#ifndef _REL_STAFFDLL_H
#define _REL_STAFFDLL_H

#include "dolphin.h"
#include "game/animdata.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"

typedef struct StaffMotionWork {
    s16 state; /* 0: orbit, 1: move to the staff pose, 2: settled. */
    s16 reservedState; /* Unused staff motion state slot. */
    f32 angleOrFrame; /* Orbit angle in degrees, then elapsed pose-transition frames. */
    f32 reservedValue; /* Unused staff motion value. */
    HuVecF position; /* Model position at the start of the pose transition; later reused for the
                      * character particle-effect path, in world units. */
    HuVecF curveControl; /* Middle world-space control point for the quadratic pose path. */
    HuVecF curveEnd; /* Final world-space point for the quadratic pose path. */
    f32 angleStep; /* Orbit angle added each update, in degrees. */
    HuVecF reservedVector; /* Unused staff motion vector. */
    HuVecF rotation; /* Saved model rotation in degrees during pose transition. */
    HuVecF orbitRadius; /* Current X, Y, and Z radii of the character's orbit, in world units. */
    HuVecF orbitRadiusTarget; /* X, Y, and Z orbit radii approached by the per-frame update. */
} STAFF_MOTION_WORK;

extern OMOBJMAN *lbl_1_bss_0;
extern OMOBJ *lbl_1_bss_4;
extern OMOBJ *lbl_1_bss_8;
extern OMOBJ *lbl_1_bss_C;
extern OMOBJ *lbl_1_bss_10;
extern OMOBJ *lbl_1_bss_18;
extern OMOBJ *lbl_1_bss_1C;
extern s16 lbl_1_bss_24;
extern s16 lbl_1_bss_34;
extern ANIMDATA *lbl_1_bss_20;
extern f32 lbl_1_bss_2C;
extern f32 lbl_1_bss_28;
extern f32 lbl_1_bss_30;
extern f32 lbl_1_bss_50[3];
extern HU3D_MODELID lbl_1_bss_36[2];
extern HU3D_MODELID lbl_1_bss_3A[2];
extern HU3D_MODELID lbl_1_bss_3E;
extern HU3D_MODELID lbl_1_bss_40[2];
extern HU3D_MODELID lbl_1_bss_44[2];
extern ANIMDATA *lbl_1_bss_48[2];
extern f32 lbl_1_bss_124;
extern s16 lbl_1_bss_128;
extern s16 lbl_1_bss_12A;
extern HuVecF lbl_1_bss_12C[32];
extern s16 lbl_1_bss_2AC[44];
extern s16 lbl_1_bss_304[];
extern f32 lbl_1_bss_310[325];
extern HUSPRID lbl_1_bss_828;
extern HUSPR_GROUPID lbl_1_bss_82A;
extern s32 lbl_1_bss_824;
extern HU3D_LIGHTID lbl_1_bss_82C[2];
extern STAFF_MOTION_WORK lbl_1_bss_5C[2];

#endif
