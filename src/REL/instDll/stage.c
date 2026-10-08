/* Loads the instruction screen's stage preview models and motions. */
#include "dolphin/math.h"
#include "dolphin.h"
#include "dolphin/gx.h"
#include "game/audio.h"
#include "game/data.h"
#include "game/flag.h"
#include "game/frand.h"
#include "game/gamework.h"
#include "game/hu3d.h"
#include "game/main.h"
#include "game/mgdata.h"
#include "game/object.h"
#include "game/process.h"
#include "game/window.h"
#include "datadir_enum.h"
#include "game/charman.h"
#include "game/thpmain.h"
#include "game/wipe.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/sprite.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "dolphin/os.h"
#include "humath.h"

/* Stage preview records pair model resources with transforms and camera masks. */
typedef struct StageModel_s {
    u32 modelDataId; /* Stage model or joint-motion resource. */
    s16 sourceModelIndex; /* -1 creates a model; otherwise names its base model. */
    Vec position; /* Stage model position in preview coordinates. */
    Vec rotation; /* Stage model rotation in degrees. */
    Vec scale; /* Per-axis stage model scale. */
    s16 cameraMask; /* Camera visibility bits passed to the model renderer. */
} STAGE_MODEL;
typedef struct Inst_data_798_view {
    u8 stageModelCount; /* Number of stage preview resources in this record. */
    u32 stageModelDataIds[4]; /* Stage preview model resources. */
    f32 orbitStartAngleDegrees; /* Starting angle for the preview orbit. */
    f32 previewModelScale; /* Scale shared by the stage and character previews. */
    f32 characterFacingDegrees[5]; /* Character facing angles in degrees. */
    u8 stagePlayerMasks[4]; /* Character visibility bits for each stage model. */
    Vec characterPositions[5]; /* Character positions in stage preview coordinates. */
} Inst_data_798_view;
typedef struct Inst_bss_D8_view {
    HU3D_MODELID stageModel; /* Loaded stage preview model. */
    u16 spinFramesRemaining; /* Frames left in the model spin animation. */
    f32 spinAngleDegrees; /* Current spin angle in degrees. */
    u8 reservedBytes[12]; /* Bytes with no established use in this code. */
    s8 playerIndices[5]; /* Character indices; -1 marks an unused slot. */
    Vec playerPositions[5]; /* Character positions in stage preview coordinates. */
} Inst_bss_D8_view;
typedef struct InstDllMotionRecord {
    s16 characterMotionIds[10]; /* Loaded character motions used by the preview. */
} InstDllMotionRecord;

/* When nonzero, enables controller-one input for the preview camera in fn_1_35F8. */
extern BOOL lbl_1_bss_0;

/* Shared instruction-screen state and resources. */
extern u32 lbl_1_data_A8[5];
extern s32 lbl_1_bss_10;
extern HuVecF lbl_1_data_70C;
extern s16 lbl_1_bss_14;
extern u32 * lbl_1_bss_18;
extern s16 lbl_1_bss_1C;
extern MGDATA *lbl_1_bss_20;
extern HUSPR_GROUPID lbl_1_bss_238[256];
extern s16 lbl_1_bss_24;
extern s16 lbl_1_bss_26;
extern s16 lbl_1_bss_28[4];
extern s32 lbl_1_bss_30;
extern u8 lbl_1_bss_34;
extern u16 lbl_1_bss_36;
extern u16 lbl_1_bss_38;
extern OMOBJMAN * lbl_1_bss_3C;
extern u16 lbl_1_bss_4;
extern OMOBJ * lbl_1_bss_40;
extern s16 lbl_1_bss_438;
extern InstDllMotionRecord lbl_1_bss_43A[5];
extern f32 lbl_1_bss_48;
extern HU3D_MODELID lbl_1_bss_49E[6];
extern void * lbl_1_bss_4C;
extern ANIMDATA * lbl_1_bss_50[2];
extern ANIMDATA * lbl_1_bss_58;
extern ANIMDATA * lbl_1_bss_5C[2];
extern s16 lbl_1_bss_6;
extern HUSPR_GROUPID lbl_1_bss_66[3];
extern HU3D_MODELID lbl_1_bss_6C;
extern s16 lbl_1_bss_6E;
extern ANIMDATA * lbl_1_bss_70[2];
extern s16 lbl_1_bss_78[6];
extern s16 lbl_1_bss_8;
extern BOOL lbl_1_bss_84[20];
extern s16 lbl_1_bss_A;
extern s32 lbl_1_bss_C;
extern s16 lbl_1_bss_D4;
extern Inst_bss_D8_view lbl_1_bss_D8[4];
extern OM_CAMERA_VIEW lbl_1_data_24;
extern BOOL lbl_1_data_4;
extern OM_CAMERA_VIEW lbl_1_data_40;
extern u8 lbl_1_data_410[];
extern char lbl_1_data_5C[];
extern HuVecF lbl_1_data_700;
extern HuVecF lbl_1_data_718;
extern HuVecF lbl_1_data_724;
extern char lbl_1_data_730[];
extern char lbl_1_data_74A[];
extern char lbl_1_data_764[];
extern char lbl_1_data_77E[];
extern Inst_data_798_view lbl_1_data_798[9];
extern OM_CAMERA_VIEW lbl_1_data_8;
extern char lbl_1_data_80[];
extern char lbl_1_data_CE[];
extern char lbl_1_data_DC[];
extern char lbl_1_data_E7[];
extern u32 lbl_1_data_F8[];
extern GXRenderModeObj *RenderMode;

/* Instruction-screen routines used across its setup and display files. */
extern void fn_1_1024(void);
extern void fn_1_35F8(OMOBJ *obj);
extern void fn_1_4158(void);
void fn_1_8FA0(s16 imageIndex);
void fn_1_96F4(s16 selectedMode);
void fn_1_4668(STAGE_MODEL *stageModels);
void fn_1_539C(void);
void fn_1_5DE0(void);
void fn_1_47E4(s16 selectedMode);
void fn_1_2A1C(void);
int fn_1_A1E8(void (*childFunction)(void), s32 processState);
void fn_1_15C8(void);
void fn_1_4188(void);
void fn_1_8DB0(void);
void fn_1_15B4(void);
HUSPR_GROUPID fn_1_4400(u32 messNum);
void fn_1_4B48(void);
extern void fn_1_62A8(void);
extern void fn_1_8B44(void);
extern void fn_1_750C(void);
extern void fn_1_8504(s16 index, Vec *transformed, Vec *result);
extern void fn_1_8680(void);
void fn_1_902C(s16 layerNo);
extern void fn_1_9CF4(HU3D_MODEL *modelP, Mtx *mtx);
extern void fn_1_99CC(void);
void fn_1_9CB0(s16 layerNo);

/* Stage preview models and motions, in the order used by the instruction screen. */
STAGE_MODEL lbl_1_data_520[] = {
    { DATANUM(DATA_inst, 26), -1, { -206.0f, -2.1e+02f, 302.0f }, { 0.0f, 15.0f, 0.0f }, { 0.9f, 0.9f, 0.9f }, 2 },
    { DATANUM(DATA_inst, 27), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 28), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 29), 0, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 30), -1, { -206.0f, -2.1e+02f, 302.0f }, { 0.0f, 15.0f, 0.0f }, { 0.9f, 0.9f, 0.9f }, 2 },
    { DATANUM(DATA_inst, 31), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 32), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 33), 4, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { DATANUM(DATA_inst, 34), -1, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, 2 },
    { HU_DATANUM_NONE, -1, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, 2 }
};

extern HU3D_MODELID lbl_1_bss_49E[6];
extern InstDllMotionRecord lbl_1_bss_43A[5];
extern s16 lbl_1_bss_438;
extern HUSPR_GROUPID lbl_1_bss_238[256];
extern Inst_bss_D8_view lbl_1_bss_D8[4];
extern s16 lbl_1_bss_D4;
extern BOOL lbl_1_bss_84[20];
extern s16 lbl_1_bss_78[6];
extern ANIMDATA * lbl_1_bss_70[2];
extern s16 lbl_1_bss_6E;
extern HU3D_MODELID lbl_1_bss_6C;
extern HUSPR_GROUPID lbl_1_bss_66[3];
extern ANIMDATA * lbl_1_bss_5C[2];
extern ANIMDATA * lbl_1_bss_58;
extern ANIMDATA * lbl_1_bss_50[2];
extern void * lbl_1_bss_4C;
extern f32 lbl_1_bss_48;
extern OMOBJ * lbl_1_bss_40;
extern OMOBJMAN * lbl_1_bss_3C;
extern u16 lbl_1_bss_38;
extern u16 lbl_1_bss_36;
extern u8 lbl_1_bss_34;
extern s32 lbl_1_bss_30;
extern s16 lbl_1_bss_28[4];
extern s16 lbl_1_bss_26;
extern s16 lbl_1_bss_24;
extern MGDATA *lbl_1_bss_20;
extern s16 lbl_1_bss_1C;
extern u32 * lbl_1_bss_18;
extern s16 lbl_1_bss_14;
extern s32 lbl_1_bss_10;
extern s32 lbl_1_bss_C;
extern s16 lbl_1_bss_A;
extern s16 lbl_1_bss_8;
extern s16 lbl_1_bss_6;
extern u16 lbl_1_bss_4;
extern BOOL lbl_1_bss_0;
extern HU3D_MODELID lbl_1_bss_6AA[256];
extern HU3D_MOTIONID lbl_1_bss_4AA[256];
/* Called by fn_1_1024 during setup to load the stage preview models and motions. */
void fn_1_4668(STAGE_MODEL *stageModels)
{
    STAGE_MODEL *stageModel;
    s16 stageModelIndex;

    stageModel = stageModels;
    stageModelIndex = 0;
    while (stageModel->modelDataId != HU_DATANUM_NONE) {
        if (stageModel->sourceModelIndex == -1) {
            lbl_1_bss_6AA[stageModelIndex] = Hu3DModelCreate(
                HuDataSelHeapReadNum(stageModel->modelDataId, HU_MEMNUM_OVL, HEAP_MODEL));
            Hu3DModelPosSetV(lbl_1_bss_6AA[stageModelIndex], &stageModel->position);
            Hu3DModelRotSetV(lbl_1_bss_6AA[stageModelIndex], &stageModel->rotation);
            Hu3DModelScaleSetV(lbl_1_bss_6AA[stageModelIndex], &stageModel->scale);
            Hu3DModelCameraSet(lbl_1_bss_6AA[stageModelIndex], stageModel->cameraMask);
            Hu3DModelLayerSet(lbl_1_bss_6AA[stageModelIndex], 1);
        } else {
            /* Motion records refer back to a model loaded earlier in the table. */
            lbl_1_bss_4AA[stageModelIndex] = Hu3DJointMotion(
                lbl_1_bss_6AA[stageModel->sourceModelIndex],
                HuDataSelHeapReadNum(stageModel->modelDataId, HU_MEMNUM_OVL, HEAP_MODEL));
        }
        stageModel++;
        stageModelIndex++;
    }
}
