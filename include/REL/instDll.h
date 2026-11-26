/* Instruction-screen entry points shared by the module's setup, display, name, and stage TUs. */
#ifndef INSTDLL_H
#define INSTDLL_H

#include "dolphin.h"
#include "game/hu3d.h"
#include "game/object.h"
#include "game/sprite.h"

typedef struct InstPreviewPose {
    u8 stageModelCount;
    u32 stageModelDataIds[4];
    f32 orbitStartAngleDegrees;
    f32 previewModelScale;
    f32 characterFacingDegrees[5];
    u8 stagePlayerMasks[4];
    Vec characterPositions[5];
} InstPreviewPose;
typedef struct InstPreviewStage {
    HU3D_MODELID stageModel;
    u16 spinFramesRemaining;
    f32 spinAngleDegrees;
    u8 unk08[12]; /* No code names this field. */
    s8 playerIndices[5];
    Vec playerPositions[5];
} InstPreviewStage;

extern InstPreviewStage lbl_1_bss_D8[4];
extern InstPreviewPose lbl_1_data_798[9];

typedef struct StageModel_s {
    u32 modelDataId; /* Stage model or joint-motion resource. */
    s16 sourceModelIndex; /* -1 creates a model; otherwise names its base model. */
    Vec position; /* Stage model position in preview coordinates. */
    Vec rotation; /* Stage model rotation in degrees. */
    Vec scale; /* Per-axis stage model scale. */
    s16 cameraMask; /* Camera visibility bits passed to the model renderer. */
} STAGE_MODEL;

void ObjectSetup(void);
void fn_1_1024(void);
void fn_1_35F8(OMOBJ *obj);
void fn_1_4158(void);
void fn_1_8FA0(s16 imageIndex);
void fn_1_96F4(s16 selectedMode);
void fn_1_4668(struct StageModel_s *desc);
void fn_1_539C(void);
void fn_1_5DE0(void);
void fn_1_47E4(s16 selectedMode);
void fn_1_2A1C(void);
void fn_1_15C8(void);
void fn_1_4188(void);
void fn_1_8DB0(void);
void fn_1_15B4(void);
HUSPR_GROUPID fn_1_4400(u32 messNum);
int fn_1_A1E8(void (*childFunction)(void), s32 processState);
void fn_1_4B48(void);
void fn_1_62A8(void);
void fn_1_8B44(void);
void fn_1_750C(void);
void fn_1_8504(s16 index, Vec *transformed, Vec *result);
void fn_1_8680(void);
void fn_1_902C(s16 layerNo);
void fn_1_9CF4(HU3D_MODEL *modelP, Mtx *mtx);
void fn_1_99CC(void);
void fn_1_9CB0(s16 layerNo);

#endif
