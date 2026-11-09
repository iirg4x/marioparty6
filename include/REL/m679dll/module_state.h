#ifndef REL_M679DLL_MODULE_STATE_H
#define REL_M679DLL_MODULE_STATE_H

typedef struct M679ModelSet_s {
    HU3D_MODELID model0;       /* +0x00 */
    HU3D_MOTIONID motion0;     /* +0x02 */
    HU3D_MODELID model1;       /* +0x04 */
    HU3D_MOTIONID motion1;     /* +0x06 */
    HU3D_MODELID model2;       /* +0x08 */
    HU3D_MOTIONID motion2;     /* +0x0A */
    u8 unk_0C[4]; /* No code names this field. */
    HU3D_MODELID model3;       /* +0x10 */
    HU3D_MOTIONID motion3;     /* +0x12 */
} M679ModelSet;

typedef struct M679ScoreState_s {
    s16 scoreBox;              /* +0x00 */
    s16 mode;                  /* +0x02 */
    s32 state;                 /* +0x04 */
    s16 colorInterval;         /* +0x08 */
    s16 colorCountdown;        /* +0x0A */
    s16 colorStep;             /* +0x0C */
    s16 colorValue;            /* +0x0E */
    s32 colorIncreasing;       /* +0x10 */
    MGSCORE *scoreA;           /* +0x14 */
    MGSCORE *scoreB;           /* +0x18 */
} M679ScoreState;

#endif
