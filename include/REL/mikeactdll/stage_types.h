#ifndef REL_MIKEACTDLL_STAGE_TYPES_H
#define REL_MIKEACTDLL_STAGE_TYPES_H
#include "REL/mikeactdll/module_types.h"

typedef struct MikeActSceneLayoutEntry_s {
    s32 layoutId;
    s32 horizontalIndex;
    s32 depthIndex;
    s32 rotationIndex;
} MikeActSceneLayoutEntry;

typedef struct MikeActSceneEffectRow_s {
    s16 state;
    s16 flags;
    u8 unknown_04[4]; /* No code names this field. */
    Point3d position;
    MikeActStateRecord *stateRecord;
    s16 primaryModelId;
    s16 secondaryModelId;
    s16 auxiliaryModelId;
} MikeActSceneEffectRow;

void fn_1_1AFD0(void);
void fn_1_1C7D4(void);

extern Point3d lbl_1_data_1254[6];
extern s32 lbl_1_data_1838[2];
extern s32 lbl_1_data_1988;
extern Point3d lbl_1_data_12BC[9];
extern MikeActSceneLayoutEntry lbl_1_data_1348[3][25];
extern Point3d lbl_1_data_1100;
extern Point3d lbl_1_data_110C;
extern GXColor lbl_1_data_1118;
extern Point3d lbl_1_data_111C[0xD];
extern Point3d lbl_1_data_11B8[1];
extern Point3d lbl_1_data_11C4[0xC];
extern f32 lbl_1_data_1840[2];
extern Point3d lbl_1_data_1910[4];
extern Point3d lbl_1_data_1940[6];
extern MGACTOR_PARAM lbl_1_data_1328;
extern MGACTOR_PARAM lbl_1_data_1818;
extern OM_CAMERA_VIEW lbl_1_data_1848;
extern Point3d lbl_1_data_1864[4];
extern Point3d lbl_1_data_1894[4];
extern OM_CAMERA_VIEW lbl_1_data_18C4;
extern Point3d lbl_1_data_18E0[4];

extern s16 lbl_1_bss_3418;

#endif
