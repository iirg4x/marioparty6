/* Camera and sequence data shared by the Trap Ease Artist module. */
#ifndef REL_M620DLL_MODULE_DATA_H
#define REL_M620DLL_MODULE_DATA_H

#include "dolphin/math.h"
#include "dolphin/gx.h"
#include "game/mg/seqman.h"
#include "game/object.h"

typedef struct M620BF0PositionData_s {
    /* Float bit patterns for center XYZ, rotation XYZ in degrees, and zoom. */
    u32 cameraViewValueBits[7];
} M620BF0PositionData;

extern M620BF0PositionData lbl_1_bss_0;
extern M620BF0PositionData lbl_1_data_0;
extern HuVecF lbl_1_data_1C;
extern HuVecF lbl_1_data_28;
extern GXColor lbl_1_data_34;
extern Vec lbl_1_data_C0;
extern GXColor lbl_1_data_CC;
extern u32 lbl_1_data_38[18];
extern MGSEQ_PARAM lbl_1_data_80;
extern Vec lbl_1_data_A8;
extern Vec lbl_1_data_B4;
extern Vec lbl_1_data_130[4];
extern f32 lbl_1_data_160[7];
extern s32 lbl_1_data_D0[6];

typedef struct M620PlayerConfig {
    /* The actor callbacks do not read these words from the copied record. */
    s32 unreadWordsBeforeResourceIds[3];
    s32 firstResourceId;
    s32 secondResourceId;
    /* These words are also not read by the actor callbacks. */
    s32 unreadWordsBeforeActionIds[3];
    s32 resourceId;
    s32 nextResourceId;
    s32 winMotion;
    s32 loseMotion;
} M620PlayerConfig;

extern M620PlayerConfig lbl_1_data_17C;

#endif
