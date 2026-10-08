/* Builds the minigame instructions screen, character previews, and movie backdrop. */
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

/* Layouts for instruction model placement and the minigame's camera preview. */
enum {
    INST_PREVIEW_DRAW_NO = 64,
    INST_FULL_ROTATION_DEGREES = 360,
    INST_PREVIEW_OFFSET = 10,
    INST_PREVIEW_FRAME_MAX = 15,
    INST_PREVIEW_WAIT_FRAMES = 30,
    INST_PREVIEW_SPACING = 30,
    INST_PREVIEW_PLAYER_MASK_ALL = 15,
    INST_PREVIEW_ANGLE_STEP = 45,
    INST_PREVIEW_ANGLE_RANGE = 90,
    INST_PREVIEW_INTERPOLATION_FRAMES = 60,
    INST_CAPTURE_WIDTH = 284,
    INST_CAPTURE_HEIGHT = 256
};

typedef struct StageModel_s {
    u32 modelDataId;
    s16 motionModelId;
    Vec position;
    Vec rotation;
    Vec scale;
    s16 cameraMask;
} STAGE_MODEL;
typedef struct Inst_data_798_view {
    u8 stageModelCount;
    u32 stageModelDataIds[4];
    f32 orbitStartAngleDegrees;
    f32 previewModelScale;
    f32 characterFacingDegrees[5];
    u8 stagePlayerMasks[4];
    Vec characterPositions[5];
} Inst_data_798_view;
typedef struct Inst_bss_D8_view {
    HU3D_MODELID stageModel;
    u16 spinFramesRemaining;
    f32 spinAngleDegrees;
    u8 unusedStorage[12];
    s8 playerIndices[5];
    Vec playerPositions[5];
} Inst_bss_D8_view;
typedef struct InstDllMotionRecord {
    s16 motionIds[10];
} InstDllMotionRecord;

extern BOOL lbl_1_bss_0;

/* Shared instruction-screen input state, stage-preview data, camera views, and messages. */
extern u32 lbl_1_data_A8[5];
extern s32 lbl_1_bss_10;
extern s16 lbl_1_bss_14;
extern u32 * lbl_1_bss_18;
extern s16 lbl_1_bss_1C;
extern MGDATA *lbl_1_bss_20;
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
extern s16 lbl_1_bss_6;
extern s16 lbl_1_bss_8;
extern s16 lbl_1_bss_A;
extern s32 lbl_1_bss_C;
extern OM_CAMERA_VIEW lbl_1_data_24;
extern BOOL lbl_1_data_4;
extern OM_CAMERA_VIEW lbl_1_data_40;
extern u8 lbl_1_data_410[];
extern STAGE_MODEL lbl_1_data_520[];
extern char lbl_1_data_5C[];
extern OM_CAMERA_VIEW lbl_1_data_8;
extern char lbl_1_data_80[];
extern char lbl_1_data_CE[];
extern char lbl_1_data_DC[];
extern char lbl_1_data_E7[];
extern u32 lbl_1_data_F8[];
extern GXRenderModeObj *RenderMode;

/* Functions shared with the other instruction-screen files. */
extern void fn_1_1024(void);
extern void fn_1_35F8(OMOBJ *obj);
extern void fn_1_4158(void);
void fn_1_8FA0(s16 imageIndex);
void fn_1_96F4(s16 selectedMode);
void fn_1_4668(STAGE_MODEL *desc);
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
extern void fn_1_62A8(void);
extern void fn_1_8B44(void);
extern void fn_1_750C(void);
extern void fn_1_8504(s16 index, Vec *transformed, Vec *result);
extern void fn_1_8680(void);
void fn_1_902C(s16 layerNo);
extern void fn_1_9CF4(HU3D_MODEL *modelP, Mtx *mtx);
extern void fn_1_99CC(void);
void fn_1_9CB0(s16 layerNo);

/* Preview positions, camera views, model settings, and group-error messages. */
HuVecF lbl_1_data_700 = { -252.0f, 1.1e+02f, -1.4e+03f };
HuVecF lbl_1_data_70C = { -206.0f, -2.1e+02f, 302.0f };
HuVecF lbl_1_data_718 = { 1.7e+02f, 104.0f, -1.4e+03f };
HuVecF lbl_1_data_724 = { 1.0f, 1.0f, 1.0f };
char lbl_1_data_730[] = "Error: 2x2P Group Error!\n";
char lbl_1_data_74A[] = "Error: 1x3P Group Error!\n";
char lbl_1_data_764[] = "Error: DUEL Group Error!\n";
char lbl_1_data_77E[] = "Error: RARE Group Error!\n";
Inst_data_798_view lbl_1_data_798[] = {
    { 4, { DATANUM(DATA_inst, 3), DATANUM(DATA_inst, 2), DATANUM(DATA_inst, 0), DATANUM(DATA_inst, 1) }, 45.0f, 0.6f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 1, 2, 4, 8 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 2, { DATANUM(DATA_inst, 6), DATANUM(DATA_inst, 7), 0, 0 }, 9e+01f, 0.6f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 1, 14, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 2, { DATANUM(DATA_inst, 4), DATANUM(DATA_inst, 5), 0, 0 }, -9e+01f, 0.6f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 3, 12, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 4, { DATANUM(DATA_inst, 3), DATANUM(DATA_inst, 2), DATANUM(DATA_inst, 0), DATANUM(DATA_inst, 1) }, 45.0f, 0.6f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 1, 2, 4, 8 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 2, { DATANUM(DATA_inst, 10), DATANUM(DATA_inst, 12), 0, 0 }, -9e+01f, 0.5f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 15, 16, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 1, { DATANUM(DATA_inst, 13), 0, 0, 0 }, 0.0f, 0.8f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 3, 0, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 2, { DATANUM(DATA_inst, 8), DATANUM(DATA_inst, 9), 0, 0 }, -9e+01f, 0.7f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 1, 2, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 2, { DATANUM(DATA_inst, 10), DATANUM(DATA_inst, 11), 0, 0 }, -9e+01f, 0.5f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 15, 16, 0, 0 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } },
    { 4, { DATANUM(DATA_inst, 3), DATANUM(DATA_inst, 2), DATANUM(DATA_inst, 0), DATANUM(DATA_inst, 1) }, 45.0f, 0.6f, { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f }, { 1, 2, 4, 8 }, { { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f } } }
};

HU3D_MODELID lbl_1_bss_6AA[256];
HU3D_MOTIONID lbl_1_bss_4AA[256];
HU3D_MODELID lbl_1_bss_49E[6];
InstDllMotionRecord lbl_1_bss_43A[5];
s16 lbl_1_bss_438;
HUSPR_GROUPID lbl_1_bss_238[256];
Inst_bss_D8_view lbl_1_bss_D8[4];
s16 lbl_1_bss_D4;
BOOL lbl_1_bss_84[20];
s16 lbl_1_bss_78[6];
ANIMDATA * lbl_1_bss_70[2];
s16 lbl_1_bss_6E;
HU3D_MODELID lbl_1_bss_6C;
HUSPR_GROUPID lbl_1_bss_66[3];
s16 lbl_1_bss_64;
ANIMDATA * lbl_1_bss_5C[2];
ANIMDATA * lbl_1_bss_58;
ANIMDATA * lbl_1_bss_50[2];
void * lbl_1_bss_4C;
f32 lbl_1_bss_48;
/* Instruction-screen display routines. */

/* Called during setup to place the title, minigame name, and stage preview sprites. */
void fn_1_47E4(s16 selectedMode)
{
    HUSPR_GROUPID groupId;
    HUSPRID sprId;
    ANIMDATA *animData[2];

    groupId = HuSprGrpCreate(2);
    lbl_1_bss_238[0] = groupId;
    sprId = HuSprCreate(
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_inst, 18), HU_MEMNUM_OVL, HEAP_MODEL)), 100,
        0);
    HuSprGrpMemberSet(groupId, 0, sprId);
    HuSprPosSet(groupId, 0, 288.0f, 64.0f);

    sprId = HuSprCreate(
        HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_inst, 23), HU_MEMNUM_OVL, HEAP_MODEL)), 100,
        0);
    HuSprGrpMemberSet(groupId, 1, sprId);
    HuSprPosSet(groupId, 1, 288.0f, 70.0f);
    if (selectedMode == 0) {
        HuSprAttrSet(groupId, 1, 4);
    } else {
        HuSprAttrSet(groupId, 0, 4);
    }

    lbl_1_bss_238[1] = fn_1_4400(lbl_1_bss_20->nameMes);
    HuSprGrpPosSet(lbl_1_bss_238[1], 288.0f, 62.0f);

    if (MgInstExitF) {
        if (lbl_1_bss_20->flag & MG_FLAG_HAS_NIGHT) {
            animData[0] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_inst, 15), HU_MEMNUM_OVL, HEAP_MODEL));
            animData[1] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_inst, 20), HU_MEMNUM_OVL, HEAP_MODEL));
        } else {
            animData[0] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_inst, 16), HU_MEMNUM_OVL, HEAP_MODEL));
            animData[1] = HuSprAnimRead(
                HuDataSelHeapReadNum(DATANUM(DATA_inst, 21), HU_MEMNUM_OVL, HEAP_MODEL));
        }
    } else {
        animData[0] = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_inst, 17), HU_MEMNUM_OVL, HEAP_MODEL));
        animData[1] = HuSprAnimRead(
            HuDataSelHeapReadNum(DATANUM(DATA_inst, 22), HU_MEMNUM_OVL, HEAP_MODEL));
    }

    groupId = HuSprGrpCreate(2);
    lbl_1_bss_238[2] = groupId;
    sprId = HuSprCreate(animData[0], 100, 0);
    HuSprGrpMemberSet(groupId, 0, sprId);
    HuSprPosSet(groupId, 0, 108.0f, 270.0f);
    sprId = HuSprCreate(animData[1], 100, 0);
    HuSprGrpMemberSet(groupId, 1, sprId);
    HuSprPosSet(groupId, 1, 108.0f, 270.0f);
    HuSprGrpDrawNoSet(groupId, INST_PREVIEW_DRAW_NO);

    if (selectedMode == 0) {
        HuSprAttrSet(groupId, 1, 4);
    } else {
        HuSprAttrSet(groupId, 0, 4);
    }
    HuSprExecLayerCameraSet(INST_PREVIEW_DRAW_NO, 2, 10);
    fn_1_A1E8(fn_1_4B48, selectedMode);
}

/* Child process created by fn_1_47E4; animates the preview until setup exits. */
void fn_1_4B48(void)
{
    HUPROCESS *process;
    s32 mode;
    s16 frame;
    f32 phase;
    f32 offset;
    f32 scale;
    f32 positionX;
    f32 positionY;

    process = HuPrcCurrentGet();
    mode = (s32)process->property;
    phase = 0.0f;

    if (lbl_1_bss_6 == 0 || lbl_1_bss_6 == 1) {
        HuSprScaleSet(lbl_1_bss_238[2], 0, 0.0f,
                      0.0f);
        HuSprScaleSet(lbl_1_bss_238[2], 1, 0.0f,
                      0.0f);
        HuSprPosSet(lbl_1_bss_238[0], 0, 288.0f,
                    -100.0f);
        HuSprPosSet(lbl_1_bss_238[0], 1, 288.0f,
                    -100.0f);
        HuSprGrpPosSet(lbl_1_bss_238[1], 288.0f,
                       -100.0f);

        while (lbl_1_bss_6 != 1) {
            HuPrcVSleep();
        }

        for (frame = 1; frame <= 20; frame++) {
            scale = (f32)frame / 20.0f;
            offset = (f32)(-100.0 *
                           (1.0 -
                            sin(3.141592653589793 *
                                (90.0f * scale) /
                                180.0)));
            HuSprPosSet(lbl_1_bss_238[0], 0, 288.0f,
                        64.0f + offset);
            HuSprPosSet(lbl_1_bss_238[0], 1, 288.0f,
                        70.0f + offset);
            HuSprGrpPosSet(lbl_1_bss_238[1], 288.0f,
                           62.0f + offset);

            scale = (f32)(sin(3.141592653589793 *
                              (100.0f * scale) /
                              180.0) *
                          (1.0 /
                           sin(1.7453292519943295)));
            HuSprScaleSet(lbl_1_bss_238[2], 0, scale, scale);
            HuSprScaleSet(lbl_1_bss_238[2], 1, scale, scale);
            HuPrcVSleep();
        }
    }

    for (;;) {
    if (lbl_1_bss_8 != 0) {
        scale = (f32)(1.0 -
                      (f32)(lbl_1_bss_8 - 1) / 19.0f);
        HuSprAttrReset(lbl_1_bss_238[0], 0, 4);
        HuSprAttrReset(lbl_1_bss_238[0], 1, 4);
        HuSprAttrReset(lbl_1_bss_238[2], 0, 4);
        HuSprAttrReset(lbl_1_bss_238[2], 1, 4);

        if (mode == 0) {
            HuSprTPLvlSet(lbl_1_bss_238[0], 0,
                          (f32)(1.0 - scale));
            HuSprTPLvlSet(lbl_1_bss_238[0], 1, scale);
            HuSprTPLvlSet(lbl_1_bss_238[2], 0,
                          (f32)(1.0 - scale));
            HuSprTPLvlSet(lbl_1_bss_238[2], 1, scale);
        } else {
            HuSprTPLvlSet(lbl_1_bss_238[0], 0, scale);
            HuSprTPLvlSet(lbl_1_bss_238[0], 1,
                          (f32)(1.0 - scale));
            HuSprTPLvlSet(lbl_1_bss_238[2], 0, scale);
            HuSprTPLvlSet(lbl_1_bss_238[2], 1,
                          (f32)(1.0 - scale));
        }

        if (1.0 == scale) {
            mode = (mode == 0) ? 1 : 0;
        }
    } else if (lbl_1_bss_6 == 3) {
        goto finish;
    }

        positionX = (f32)(108.0 +
                          2.0 *
                              sin(3.141592653589793 *
                              (phase / 2) /
                                  180.0));
        positionY = (f32)(270.0 +
                          3.0 *
                              cos(3.141592653589793 * phase /
                                  180.0));
        HuSprPosSet(lbl_1_bss_238[2], 0, positionX, positionY);
        HuSprPosSet(lbl_1_bss_238[2], 1, positionX, positionY);
        phase += 2.0f;
        if (phase > 720.0f) {
            phase -= 720.0f;
        }
        HuPrcVSleep();
    }

finish:
    for (frame = 1; frame <= 10; frame++) {
        scale = (f32)(1.0 -
                      (f32)frame / 10.0f);
        offset = (f32)(-100.0 *
                       (1.0 -
                        sin(3.141592653589793 *
                            (90.0f * scale) /
                            180.0)));
        HuSprPosSet(lbl_1_bss_238[0], 0, 288.0f,
                    64.0f + offset);
        HuSprPosSet(lbl_1_bss_238[0], 1, 288.0f,
                    70.0f + offset);
        HuSprGrpPosSet(lbl_1_bss_238[1], 288.0f,
                       62.0f + offset);
        HuSprScaleSet(lbl_1_bss_238[2], 0, scale, scale);
        HuSprScaleSet(lbl_1_bss_238[2], 1, scale, scale);
        HuPrcVSleep();
    }

    HuPrcEnd();
}

/* Called during setup to create character models in the configured team order. */
void fn_1_539C(void)
{
    s16 charList[4];
    s16 count;
    s16 i;
    s16 charNo;

    lbl_1_bss_438 = 4;
    for (i = 0; i < 4; i++) {
        charList[i] = -1;
    }

    switch (lbl_1_bss_20->type) {
    case MG_TYPE_2VS2:
        if (lbl_1_bss_20->flag & MG_FLAG_GRPORDER) {
            count = 0;
            for (i = count; i < 4; i++) {
                charList[GwPlayerConf[i].grpNo] = lbl_1_bss_28[i];
            }
        } else {
            count = 0;
            for (i = count; i < 4; i++) {
                if (GwPlayerConf[i].grpNo == 0) {
                    charList[count++] = lbl_1_bss_28[i];
                }
            }
            for (i = 0; i < 4; i++) {
                if (GwPlayerConf[i].grpNo == 1) {
                    charList[count++] = lbl_1_bss_28[i];
                }
            }
        }
        for (i = 0; i < 4; i++) {
            if (charList[i] == -1) {
                break;
            }
        }
        if (i != 4) {
            OSReport(lbl_1_data_730);
            for (i = 0; i < 4; i++) {
                charList[i] = lbl_1_bss_28[i];
            }
        }
        break;

    case MG_TYPE_1VS3:
        count = 0;
        for (i = count; i < 4; i++) {
            if (GwPlayerConf[i].grpNo == 0) {
                charList[0] = lbl_1_bss_28[i];
            } else {
                count++;
                charList[count] = lbl_1_bss_28[i];
            }
        }
        if (count != 3) {
            OSReport(lbl_1_data_74A);
            for (i = 0; i < 4; i++) {
                charList[i] = lbl_1_bss_28[i];
            }
        }
        break;

    case MG_TYPE_KETTOU:
        count = 0;
        for (i = count; i < 4; i++) {
            if (GwPlayerConf[i].grpNo < 2) {
                charList[GwPlayerConf[i].grpNo] = lbl_1_bss_28[i];
                count++;
            }
        }
        if (count != 2) {
            OSReport(lbl_1_data_764);
            charList[0] = lbl_1_bss_28[0];
            charList[1] = lbl_1_bss_28[1];
        }
        lbl_1_bss_438 = 2;
        break;

    case MG_TYPE_LAST:
        if (lbl_1_bss_20->ovl != DLL_m699dll) {
            count = 0;
            for (i = count; i < 4; i++) {
                if (GwPlayerConf[i].grpNo == 0) {
                    charList[count] = lbl_1_bss_28[i];
                    count++;
                }
            }
            if (count == 0) {
                OSReport(lbl_1_data_77E);
                charList[0] = lbl_1_bss_28[0];
                charList[1] = lbl_1_bss_28[1];
            }
        } else {
            count = 0;
            for (i = count; i < 4; i++) {
                if (GwPlayerConf[i].grpNo == 0) {
                    charList[count++] = lbl_1_bss_28[i];
                }
            }
            for (i = 0; i < 4; i++) {
                if (GwPlayerConf[i].grpNo == 1) {
                    charList[count++] = lbl_1_bss_28[i];
                }
            }
        }
        lbl_1_bss_438 = count;
        break;

    default:
        for (i = 0; i < 4; i++) {
            charList[i] = lbl_1_bss_28[i];
        }
        break;
    }

    for (i = 0; i < lbl_1_bss_438; i++) {
        charNo = charList[i];
        lbl_1_bss_49E[i] = CharModelCreate(charNo, 8);
        lbl_1_bss_43A[i].motionIds[0] = CharMotionCreate(charNo, CHARMOT_HSF_c000m1_300);
        lbl_1_bss_43A[i].motionIds[1] = CharMotionCreate(charNo, CHARMOT_HSF_c000m1_303);
        lbl_1_bss_43A[i].motionIds[2] = CharMotionCreate(charNo, CHARMOT_HSF_c000m1_304);
        Hu3DModelShadowSet(lbl_1_bss_49E[i]);
        CharMotionSet(charNo, lbl_1_bss_43A[i].motionIds[0]);
        Hu3DModelAttrSet(lbl_1_bss_49E[i], HU3D_MOTATTR_LOOP);
        CharModelVoiceFlagSet(charNo, 0);
    }

    /* These minigame types append a special preview model after the player models. */
    if (lbl_1_bss_20->type == MG_TYPE_KUPA) {
        lbl_1_bss_49E[i] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_inst, 39), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_43A[i].motionIds[0] =
            Hu3DJointMotion(lbl_1_bss_49E[i], HuDataSelHeapReadNum(DATANUM(DATA_inst, 40),
                                                                   HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_43A[i].motionIds[1] =
            Hu3DJointMotion(lbl_1_bss_49E[i], HuDataSelHeapReadNum(DATANUM(DATA_inst, 41),
                                                                   HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelShadowSet(lbl_1_bss_49E[i]);
        Hu3DMotionSet(lbl_1_bss_49E[i], lbl_1_bss_43A[i].motionIds[0]);
        Hu3DModelAttrSet(lbl_1_bss_49E[i], HU3D_MOTATTR_LOOP);
        lbl_1_bss_438++;
    } else if (lbl_1_bss_20->type == MG_TYPE_DONKEY) {
        lbl_1_bss_49E[i] = Hu3DModelCreate(
            HuDataSelHeapReadNum(DATANUM(DATA_inst, 35), HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_43A[i].motionIds[0] =
            Hu3DJointMotion(lbl_1_bss_49E[i], HuDataSelHeapReadNum(DATANUM(DATA_inst, 36),
                                                                   HU_MEMNUM_OVL, HEAP_MODEL));
        lbl_1_bss_43A[i].motionIds[1] =
            Hu3DJointMotion(lbl_1_bss_49E[i], HuDataSelHeapReadNum(DATANUM(DATA_inst, 37),
                                                                   HU_MEMNUM_OVL, HEAP_MODEL));
        Hu3DModelShadowSet(lbl_1_bss_49E[i]);
        Hu3DMotionSet(lbl_1_bss_49E[i], lbl_1_bss_43A[i].motionIds[0]);
        Hu3DModelAttrSet(lbl_1_bss_49E[i], HU3D_MOTATTR_LOOP);
        lbl_1_bss_438++;
    }
}

/* Called after model creation to place stage models and start their idle orbit. */
void fn_1_5DE0(void)
{
    Inst_data_798_view *poseData;
    Inst_bss_D8_view *entry;
    Mtx matrix;
    Mtx rotateMatrix;
    Mtx playerMatrix;
    Vec point;
    s16 i;
    s16 j;
    s16 playerIndex;

    fn_1_62A8();

    entry = lbl_1_bss_D8;
    PSMTXRotRad(matrix, 'x', 0.7853981852531433f);
    PSMTXRotRad(rotateMatrix, 'y', 0.7853981852531433f);
    PSMTXConcat(matrix, rotateMatrix, matrix);

    poseData = &lbl_1_data_798[lbl_1_bss_20->type];
    lbl_1_bss_D4 = poseData->stageModelCount;
    for (i = 0; i < lbl_1_bss_D4; i++, entry++) {
        entry->stageModel = Hu3DModelCreate(
            HuDataSelHeapReadNum(poseData->stageModelDataIds[i], HU_MEMNUM_OVL, 2));
        Hu3DModelCameraSet(entry->stageModel, 2);
        Hu3DModelLayerSet(entry->stageModel, 1);
        Hu3DModelShadowMapSet(entry->stageModel);

        PSMTXTrans(
            rotateMatrix,
            (f32)(30.0 *
                  sin(3.141592653589793 *
                      (poseData->orbitStartAngleDegrees +
                       ((360.0f /
                         (f32)lbl_1_bss_D4) * (f32)i)) /
                      180.0)),
            0.0f,
            (f32)(30.0 *
                  cos(3.141592653589793 *
                      (poseData->orbitStartAngleDegrees +
                       ((360.0f /
                         (f32)lbl_1_bss_D4) * (f32)i)) /
                      180.0)));
        PSMTXConcat(matrix, rotateMatrix, rotateMatrix);
        Hu3DMtxTransGet(rotateMatrix, &point);
        PSVECAdd(&point, &lbl_1_data_700, &point);
        Hu3DModelPosSetV(entry->stageModel, &point);
        Hu3DModelScaleSet(entry->stageModel, poseData->previewModelScale,
                          poseData->previewModelScale, poseData->previewModelScale);
        Hu3DModelMtxSet(entry->stageModel, &matrix);

        for (j = 0; j < 5; j++) {
            entry->playerIndices[j] = -1;
        }

        playerIndex = 0;
        for (j = playerIndex; j < 5; j++) {
            if (poseData->stagePlayerMasks[i] & (1 << j)) {
                entry->playerIndices[playerIndex] = (s8)j;
                point = poseData->characterPositions[j];
                PSMTXMultVec(matrix, &point, &point);
                PSMTXRotRad(rotateMatrix, 'x', 0.7853981852531433f);
                PSMTXRotRad(playerMatrix, 'y',
                0.01745329238474369f *
                                (45.0f + poseData->characterFacingDegrees[j]));
                PSMTXConcat(rotateMatrix, playerMatrix, rotateMatrix);
                Hu3DModelMtxSet(lbl_1_bss_49E[j], &rotateMatrix);
                PSVECAdd(&point, &lbl_1_data_700, &point);
                entry->playerPositions[playerIndex] = point;
                lbl_1_bss_78[j] = i;
                playerIndex++;
            }
        }

        entry->spinFramesRemaining = 0;
        entry->spinAngleDegrees = (f32)frandmod(INST_FULL_ROTATION_DEGREES);
    }

    fn_1_8B44();
    fn_1_A1E8(fn_1_750C, 0);
}

/* Builds per-mode character positions and facing angles used by the preview. */
void fn_1_62A8(void)
{
    Inst_data_798_view *pose;
    s16 i;

    pose = lbl_1_data_798;
    for (i = 0; i < 4; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (45.0 + i * 90.0) / 180.0));
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (45.0 + i * 90.0) / 180.0));
        pose->characterFacingDegrees[i] = (f32)(180.0 *
                                    (atan2(-pose->characterPositions[i].x,
                                           -pose->characterPositions[i].z) /
                                     3.141592653589793));
    }

    pose++;
    pose->characterPositions[0].x = (f32)(70.0 * sin(1.5707963267948966) -
                                20.0);
    pose->characterPositions[0].y = 0.0f;
    pose->characterPositions[0].z = (f32)(70.0 * cos(1.5707963267948966));
    pose->characterFacingDegrees[0] = -90.0f;
    for (i = 0; i < 3; i++) {
        pose->characterPositions[i + 1].x =
            (f32) (20.0 + 70.0 * sin(3.141592653589793 * (180.0 + (30.0 + i * 60.0)) / 180.0));
        pose->characterPositions[i + 1].y = 0.0f;
        pose->characterPositions[i + 1].z =
            (f32) (70.0 * cos(3.141592653589793 * (180.0 + (30.0 + i * 60.0)) / 180.0));
        pose->characterFacingDegrees[i + 1] = 90.0f;
    }

    pose++;
    for (i = 0; i < 4; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (180.0 + (45.0 + i * 90.0)) / 180.0) +
                   (i < 2 ? 1 : -1) * INST_PREVIEW_OFFSET);
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (180.0 + (45.0 + i * 90.0)) / 180.0));
        pose->characterFacingDegrees[i] = 90.0f * (i < 2 ? 1 : -1);
    }

    pose++;
    for (i = 0; i < 4; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (45.0 + i * 90.0) / 180.0));
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (45.0 + i * 90.0) / 180.0));
        pose->characterFacingDegrees[i] =
            (f32) (180.0 * (atan2(-pose->characterPositions[i].x, -pose->characterPositions[i].z) /
                            3.141592653589793));
    }

    pose++;
    for (i = 0; i < 4; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (180.0 + (22.5 + i * 45.0)) / 180.0));
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (180.0 + (22.5 + i * 45.0)) / 180.0));
        pose->characterFacingDegrees[i] = 90.0f;
    }
    pose->characterPositions[i].x = (f32)(70.0 * sin(1.5707963267948966) -
                                20.0);
    pose->characterPositions[i].y = 0.0f;
    pose->characterPositions[i].z = (f32)(70.0 * cos(1.5707963267948966));
    pose->characterFacingDegrees[i] = -90.0f;

    pose++;
    if (lbl_1_bss_438 == 1) {
        pose->characterPositions[0].x = 0.0f;
        pose->characterPositions[0].y = 0.0f;
        pose->characterPositions[0].z = 0.0f;
        pose->characterFacingDegrees[0] = 0.0f;
        lbl_1_data_798[5].stagePlayerMasks[0] = 1;
    } else if (lbl_1_bss_438 == 2) {
        for (i = 0; i < lbl_1_bss_438; i++) {
            pose->characterPositions[i].x = (f32)(-56.0f +
                                       2.0f *
                                           (56.0f * i));
            pose->characterPositions[i].y = 0.0f;
            pose->characterPositions[i].z = 0.0f;
            pose->characterFacingDegrees[i] =
                (f32) (180.0 *
                       (atan2(-pose->characterPositions[i].x, -pose->characterPositions[i].z) /
                        3.141592653589793));
        }
        lbl_1_data_798[5].stagePlayerMasks[0] = 3;
    } else {
        for (i = 0; i < lbl_1_bss_438; i++) {
            pose->characterPositions[i].x =
                (f32) (70.0 * sin(3.141592653589793 * (180.0 + (45.0 + i * 90.0)) / 180.0) -
                       (i < 2 ? 1 : -1) * 20);
            pose->characterPositions[i].y = 0.0f;
            pose->characterPositions[i].z =
                (f32) (70.0 * cos(3.141592653589793 * (180.0 + (45.0 + i * 90.0)) / 180.0));
            pose->characterFacingDegrees[i] = 90.0f * (i < 2 ? 1 : -1);
        }
        lbl_1_data_798[5].stagePlayerMasks[0] = INST_PREVIEW_PLAYER_MASK_ALL;
    }

    pose++;
    for (i = 0; i < 2; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (i * 180.0 - 90.0) / 180.0) +
                   (i < 1 ? 1 : -1) * INST_PREVIEW_SPACING);
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (i * 180.0 - 90.0) / 180.0));
        pose->characterFacingDegrees[i] =
            (f32) (180.0 * (atan2(-pose->characterPositions[i].x, -pose->characterPositions[i].z) /
                            3.141592653589793));
    }

    pose++;
    for (i = 0; i < 4; i++) {
        pose->characterPositions[i].x =
            (f32) (70.0 * sin(3.141592653589793 * (180.0 + (22.5 + i * 45.0)) / 180.0));
        pose->characterPositions[i].y = 0.0f;
        pose->characterPositions[i].z =
            (f32) (70.0 * cos(3.141592653589793 * (180.0 + (22.5 + i * 45.0)) / 180.0));
        pose->characterFacingDegrees[i] = 90.0f;
    }
    pose->characterPositions[i].x = (f32)(70.0 * sin(1.5707963267948966) -
                                20.0);
    pose->characterPositions[i].y = 0.0f;
    pose->characterPositions[i].z = (f32)(70.0 * cos(1.5707963267948966));
    pose->characterFacingDegrees[i] = -90.0f;
}

/* Child process started by fn_1_5DE0; animates the stage and character models. */
void fn_1_750C(void)
{
    Inst_data_798_view *poseData;
    Inst_bss_D8_view *entry;
    Vec modelPositions[4];
    Mtx rotateMatrix;
    Mtx playerMatrix;
    Mtx matrix;
    f32 angleOffsets[4];
    Vec position;
    Vec result;
    Vec path;
    Vec pathOffset;
    f32 frameRatio;
    f32 blend;
    f32 scale;
    f32 phase;
    f32 angle;
    s16 i;
    s16 j;
    s16 frame;

    phase = 45.0f;
    for (i = 0; i < lbl_1_bss_D4; i++) {
        Hu3DModelPosGet(lbl_1_bss_D8[i].stageModel, &modelPositions[i]);
        angleOffsets[i] = (f32)(i * INST_PREVIEW_ANGLE_STEP + frandmod(INST_PREVIEW_ANGLE_RANGE));
    }

    for (i = 0; i < lbl_1_bss_438; i++) {
        lbl_1_bss_84[i] = 1;
    }

    poseData = &lbl_1_data_798[lbl_1_bss_20->type];
    if (lbl_1_bss_6 == 0 || lbl_1_bss_6 == 1) {
        for (i = 0; i < lbl_1_bss_438; i++) {
            Hu3DModelPosSet(lbl_1_bss_49E[i], -5000.0f,
                            0.0f, 0.0f);
            lbl_1_bss_84[i] = 0;
            fn_1_A1E8(fn_1_8680, i);
        }

        entry = lbl_1_bss_D8;
        for (i = 0; i < lbl_1_bss_D4; i++, entry++) {
            Hu3DModelScaleSet(entry->stageModel, 0.0f,
                              0.0f, 0.0f);
        }

        HuPrcSleep(INST_PREVIEW_WAIT_FRAMES);
        j = 1;
        while (j <= INST_PREVIEW_FRAME_MAX) {
            frameRatio = j / 15.0f;
            entry = lbl_1_bss_D8;
        scale = (f32)(poseData->previewModelScale *
                          (HuSin(130.0f * frameRatio) *
                           (1.0 / HuSin(130.0f))));
            for (i = 0; i < lbl_1_bss_D4; i++, entry++) {
                Hu3DModelScaleSet(entry->stageModel, scale, scale, scale);
            }
            HuPrcVSleep();
            j++;
        }
    }

    for (;;) {
        entry = lbl_1_bss_D8;
        for (i = 0; i < lbl_1_bss_D4; i++, entry++) {
            angle = (f32)(3.0 *
                          sin(3.141592653589793 * angleOffsets[i] /
                              180.0));
            PSMTXRotRad(matrix, 'x', 0.7853981852531433f);
            PSMTXRotRad(rotateMatrix, 'y', 0.01745329238474369f * phase);
            PSMTXConcat(matrix, rotateMatrix, matrix);
            Hu3DModelMtxSet(entry->stageModel, &matrix);

            PSMTXTrans(rotateMatrix,
                       (f32) (30.0 * sin(3.141592653589793 *
                                         (poseData->orbitStartAngleDegrees +
                                          ((360.0f / (f32) lbl_1_bss_D4) * (f32) i)) /
                                         180.0)),
                       angle,
                       (f32) (30.0 * cos(3.141592653589793 *
                                         (poseData->orbitStartAngleDegrees +
                                          ((360.0f / (f32) lbl_1_bss_D4) * (f32) i)) /
                                         180.0)));
            PSMTXConcat(matrix, rotateMatrix, rotateMatrix);
            Hu3DMtxTransGet(rotateMatrix, &position);
            PSVECAdd(&position, &lbl_1_data_700, &position);
            Hu3DModelPosSetV(entry->stageModel, &position);

            for (j = 0; j < 5; j++) {
                if (entry->playerIndices[j] == -1) {
                    break;
                }
                if (lbl_1_bss_84[entry->playerIndices[j]] != 0) {
                    fn_1_8504(entry->playerIndices[j], &position, &result);
                    Hu3DModelPosSetV(lbl_1_bss_49E[entry->playerIndices[j]],
                                     &position);
                    PSMTXRotRad(rotateMatrix, 'x',
                                0.01745329238474369f * result.x);
                    PSMTXRotRad(playerMatrix, 'y',
                                0.01745329238474369f * (result.y + phase));
                    PSMTXConcat(rotateMatrix, playerMatrix, rotateMatrix);
                    Hu3DModelMtxSet(lbl_1_bss_49E[entry->playerIndices[j]],
                                    &rotateMatrix);
                }
            }

            if (entry->spinFramesRemaining != 0) {
                entry->spinFramesRemaining--;
                entry->spinAngleDegrees += 30.0f;
                if (entry->spinAngleDegrees > 360.0f) {
                    entry->spinAngleDegrees -= 360.0f;
                }
                if ((i & 1) != 0) {
                    Hu3DModelRotSet(
                        entry->stageModel,
                        (f32)(sin(3.141592653589793 * entry->spinAngleDegrees /
                                  180.0) *
                              ((f32)entry->spinFramesRemaining /
                               10.0f)),
                        0.0f, 0.0f);
                } else {
                    Hu3DModelRotSet(
                        entry->stageModel, 0.0f, 0.0f,
                        (f32)(sin(3.141592653589793 * entry->spinAngleDegrees /
                                  180.0) *
                              ((f32)entry->spinFramesRemaining /
                               10.0f)));
                }
            }

            angleOffsets[i] += 3.0f;
            if (angleOffsets[i] > 360.0f) {
                angleOffsets[i] -= 360.0f;
            }
        }

        phase += 0.20000000298023224f;
        if (phase > 360.0f) {
            phase -= 360.0f;
        }
        if (lbl_1_bss_6 == 3) {
            break;
        }
        HuPrcVSleep();
    }

    poseData = &lbl_1_data_798[lbl_1_bss_20->type];
    frame = 1;
    while (frame <= INST_PREVIEW_INTERPOLATION_FRAMES) {
        frameRatio = frame / 60.0f;
        blend = 1.0f - frame / 40.0f;
        if (blend < 0.0) {
            blend = 0.0f;
        }

        PSMTXRotRad(matrix, 'x',
                    (f32)(0.01745329238474369 *
                          (45.0 *
                           sin(3.141592653589793 *
                               (90.0f * blend) /
                               180.0))));
        PSMTXRotRad(rotateMatrix, 'y',
                    (f32)(0.01745329238474369 *
                          (phase +
                           360.0 *
                               (1.0 -
                                cos(3.141592653589793 *
                                    (90.0f * frameRatio) /
                                    180.0)))));
        PSMTXConcat(matrix, rotateMatrix, matrix);
        PSVECSubtract(&lbl_1_data_718, &lbl_1_data_700, &path);
        path.z -= 100.0f;
        scale = (f32)(0.20000000298023224f * poseData->previewModelScale +
                      0.800000011920929 *
                          (poseData->previewModelScale *
                           cos(3.141592653589793 *
                               (90.0f * frameRatio) /
                               180.0)));

        entry = lbl_1_bss_D8;
        for (i = 0; i < lbl_1_bss_D4; i++, entry++) {
            PSMTXTrans(rotateMatrix,
                       (f32) (30.0 * sin(3.141592653589793 *
                                         (poseData->orbitStartAngleDegrees +
                                          ((360.0f / (f32) lbl_1_bss_D4) * (f32) i)) /
                                         180.0)),
                       0.0f,
                       (f32) (30.0 * cos(3.141592653589793 *
                                         (poseData->orbitStartAngleDegrees +
                                          ((360.0f / (f32) lbl_1_bss_D4) * (f32) i)) /
                                         180.0)));
            PSMTXConcat(matrix, rotateMatrix, rotateMatrix);
            Hu3DMtxTransGet(rotateMatrix, &position);
            PSVECScale(&path, &pathOffset, frameRatio);
            PSVECAdd(&pathOffset, &lbl_1_data_700, &pathOffset);
            PSVECAdd(&position, &pathOffset, &position);
            position.z = (f32)(position.z +
                               500.0 *
                                   sin(3.141592653589793 *
                                       (180.0f * frameRatio) /
                                       180.0));
            Hu3DModelPosSetV(entry->stageModel, &position);
            Hu3DModelScaleSet(entry->stageModel, scale, scale, scale);
            Hu3DModelMtxSet(entry->stageModel, &matrix);

            for (j = 0; j < 5; j++) {
                if (entry->playerIndices[j] == -1) {
                    break;
                }
                if (lbl_1_bss_84[entry->playerIndices[j]] != 0) {
                    fn_1_8504(entry->playerIndices[j], &position, &result);
                    Hu3DModelPosSetV(lbl_1_bss_49E[entry->playerIndices[j]],
                                     &position);
                    Hu3DModelScaleSet(lbl_1_bss_49E[entry->playerIndices[j]],
                                      scale, scale, scale);
                    PSMTXRotRad(rotateMatrix, 'x',
                                0.01745329238474369f * result.x);
                    PSMTXRotRad(playerMatrix, 'y',
                                (f32)(0.01745329238474369 *
                                      (phase + result.y +
                                       360.0 *
                                           (1.0 -
                                            cos(3.141592653589793 *
                                                (90.0f * frameRatio) /
                                                180.0)))));
                    PSMTXConcat(rotateMatrix, playerMatrix, rotateMatrix);
                    Hu3DModelMtxSet(lbl_1_bss_49E[entry->playerIndices[j]],
                                    &rotateMatrix);
                }
            }
        }

        HuPrcVSleep();
        frame++;
    }
    HuPrcEnd();
}

/* Converts a configured character point to world space and returns its facing. */
void fn_1_8504(s16 playerIndex, Vec *worldPosition, Vec *facingAngles)
{
    Inst_data_798_view *poseData;
    Inst_bss_D8_view *entry;
    HU3D_MODEL *model;
    Mtx matrix;
    Mtx modelMatrix;
    Vec point;

    poseData = &lbl_1_data_798[lbl_1_bss_20->type];
    entry = &lbl_1_bss_D8[lbl_1_bss_78[playerIndex]];
    model = &Hu3DData[entry->stageModel];

    mtxRot(matrix, model->rot.x, model->rot.y, model->rot.z);
    mtxScaleCat(matrix, model->scale.x, model->scale.y, model->scale.z);
    mtxTransCat(matrix, model->pos.x, model->pos.y, model->pos.z);
    PSMTXConcat(matrix, model->mtx, modelMatrix);

    point = poseData->characterPositions[playerIndex];
    PSVECScale(&point, &point, 1.0 / poseData->previewModelScale);
    PSMTXMultVec(modelMatrix, &point, worldPosition);
    facingAngles->x = 45.0f;
    facingAngles->y = poseData->characterFacingDegrees[playerIndex] + model->rot.y;
    facingAngles->z = 0.0f;
}

/* Character child process started by fn_1_750C for each player preview. */
void fn_1_8680(void)
{
    HUPROCESS *process;
    Inst_data_798_view *poseData;
    Inst_bss_D8_view *entry;
    HU3D_MODEL *model;
    Mtx modelMatrix;
    Mtx matrix;
    Vec transformed;
    Vec result;
    Vec point;
    s32 playerNo;
    s32 special;
    s16 frame;
    f32 frameRatio;
    f32 curveOffset;

    process = HuPrcCurrentGet();
    playerNo = (s32)process->property;
    special = 0;
    if (playerNo == 4 && lbl_1_bss_20->type == 4) {
        special = 1;
    }

    lbl_1_bss_84[playerNo] = 0;
    while (lbl_1_bss_6 != 1) {
        HuPrcVSleep();
    }

    Hu3DMotionSet(lbl_1_bss_49E[playerNo], lbl_1_bss_43A[playerNo].motionIds[1]);
    if (special == 0) {
        Hu3DMotionTimeSet(lbl_1_bss_49E[playerNo],
                          Hu3DMotionMaxTimeGet(lbl_1_bss_49E[playerNo]));
        Hu3DModelAttrReset(lbl_1_bss_49E[playerNo], HU3D_MOTATTR_LOOP);
    } else {
        /* Mode 4's fifth preview model starts paused at time 30 and resumes at frame 25. */
        Hu3DMotionTimeSet(lbl_1_bss_49E[playerNo], 30.0f);
        Hu3DModelAttrReset(lbl_1_bss_49E[playerNo], HU3D_MOTATTR_LOOP);
        Hu3DModelAttrSet(lbl_1_bss_49E[playerNo], HU3D_MOTATTR_PAUSE);
    }

    HuPrcSleep(playerNo * 2);
    frame = 1;
    while (frame <= 30) {
        frameRatio = frame / 30.0f;
        curveOffset = (f32)(500.0 *
                            cos(3.141592653589793 *
                                (90.0f * frameRatio) /
                                180.0));

        poseData = &lbl_1_data_798[lbl_1_bss_20->type];
        entry = &lbl_1_bss_D8[lbl_1_bss_78[(s16)playerNo]];
        model = &Hu3DData[entry->stageModel];

        mtxRot(matrix, model->rot.x, model->rot.y, model->rot.z);
        mtxScaleCat(matrix, model->scale.x, model->scale.y, model->scale.z);
        mtxTransCat(matrix, model->pos.x, model->pos.y, model->pos.z);
        PSMTXConcat(matrix, model->mtx, modelMatrix);

        point = poseData->characterPositions[(s16)playerNo];
        PSVECScale(&point, &point, 1.0 / poseData->previewModelScale);
        PSMTXMultVec(modelMatrix, &point, &transformed);
        result.x = 45.0f;
        result.y = poseData->characterFacingDegrees[(s16)playerNo] + model->rot.y;
        result.z = 0.0f;
        transformed.y += curveOffset;
        Hu3DModelPosSetV(lbl_1_bss_49E[playerNo], &transformed);

        if (frame == 25) {
            if (special != 0) {
                Hu3DModelAttrReset(lbl_1_bss_49E[playerNo], HU3D_MOTATTR_PAUSE);
            } else {
                Hu3DMotionShiftSet(lbl_1_bss_49E[playerNo],
                                   lbl_1_bss_43A[playerNo].motionIds[2],
                                   0.0f, 5.0f, 0);
            }
        }
        HuPrcVSleep();
        frame++;
    }

    lbl_1_bss_D8[lbl_1_bss_78[playerNo]].spinFramesRemaining = 30;
    Hu3DMotionShiftSet(lbl_1_bss_49E[playerNo],
                       lbl_1_bss_43A[playerNo].motionIds[0],
                       0.0f, 5.0f, HU3D_MOTATTR_LOOP);
    lbl_1_bss_84[playerNo] = 1;
    HuPrcEnd();
}

/* For preview type 7, scale the fifth character to 1.5 times the shared preview scale. */
void fn_1_8B44(void)
{
    Inst_data_798_view *poseData;
    Vec transformed;
    Vec result;
    s16 i;

    poseData = &lbl_1_data_798[lbl_1_bss_20->type];
    i = 0;
    while (i < lbl_1_bss_438) {
        Hu3DModelScaleSet(lbl_1_bss_49E[i], poseData->previewModelScale,
                          poseData->previewModelScale, poseData->previewModelScale);
        fn_1_8504(i, &transformed, &result);
        Hu3DModelPosSetV(lbl_1_bss_49E[i], &transformed);
        i++;
    }

    if (lbl_1_bss_20->type == 7) {
        Hu3DModelScaleSet(lbl_1_bss_49E[4], 1.5 * poseData->previewModelScale,
                          1.5 * poseData->previewModelScale,
                          1.5 * poseData->previewModelScale);
    }
}

/* Collects buttons from human players; type 6 ignores players outside groups 0 and 1. */
void fn_1_8DB0(void)
{
    s16 playerNo;

    playerNo = 0;
    lbl_1_bss_24 = playerNo;
    lbl_1_bss_26 = playerNo;
    while (playerNo < 4) {
        if (lbl_1_bss_20->type != 6) {
            if (GwPlayerConf[playerNo].type == 0) {
                lbl_1_bss_26 |= HuPadBtn[GwPlayerConf[playerNo].padNo];
                lbl_1_bss_24 |= HuPadBtnDown[GwPlayerConf[playerNo].padNo];
            }
        } else if (GwPlayerConf[playerNo].type == 0 &&
                   GwPlayerConf[playerNo].grpNo < 2) {
            lbl_1_bss_26 |= HuPadBtn[GwPlayerConf[playerNo].padNo];
            lbl_1_bss_24 |= HuPadBtnDown[GwPlayerConf[playerNo].padNo];
        }
        playerNo++;
    }
}

/* Registers the scrolling backdrop hook and selects the requested backdrop image. */
void fn_1_8FA0(s16 imageIndex)
{
    Hu3DCameraLayerHookSet(1, 8, fn_1_902C);
    lbl_1_bss_70[0] = HuSprAnimDataRead(DATANUM(DATA_inst, 14));
    lbl_1_bss_70[1] = HuSprAnimDataRead(DATANUM(DATA_inst, 19));
    lbl_1_bss_6E = imageIndex;
}

/* Camera layer hook that draws the tiled backdrop and crossfades its two images. */
void fn_1_902C(s16 layerNo)
{
    Mtx44 projection;
    Mtx textureMatrix;
    GXColor tevColor;
    f32 fade;

    GXInvalidateTexAll();
    C_MTXOrtho(projection, 0.0f, 480.0f,
               0.0f, 576.0f, 0.0f,
               10.0f);
    GXSetProjection(projection, GX_ORTHOGRAPHIC);

    if (RenderMode->field_rendering != 0) {
        GXSetViewportJitter(0.0f, 0.0f,
                            640.0f, 480.0f,
                            0.0f, 1.0f,
                            VIGetNextField());
    } else {
        GXSetViewport(0.0f, 0.0f,
                      640.0f, 480.0f,
                      0.0f, 1.0f);
    }

    GXSetScissor(0, 0, 640, 480);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XY, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_FALSE, GX_ALWAYS, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetAlphaCompare(GX_GEQUAL, 1, GX_AOP_AND, GX_GEQUAL, 1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumChans(0);

    PSMTXIdentity(textureMatrix);
    GXLoadPosMtxImm(textureMatrix, GX_PNMTX0);
    PSMTXTrans(textureMatrix, -lbl_1_bss_48, -lbl_1_bss_48,
               0.0f);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX2x4);

    HuSprTexLoad(lbl_1_bss_70[lbl_1_bss_6E], 0, GX_TEXMAP0,
                 GX_REPEAT, GX_REPEAT, GX_LINEAR);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0,
                      GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR_NULL);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);

    if (lbl_1_bss_8 == 0) {
        GXSetNumTevStages(1);
    } else {
        fade = (lbl_1_bss_8 - 1) / 19.0f;
        tevColor.a = 255.0f * fade;
        GXSetTevColor(GX_TEVREG0, tevColor);
        HuSprTexLoad(lbl_1_bss_70[(lbl_1_bss_6E + 1) & 1], 0,
                     GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
        GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD0, GX_TEXMAP1,
                      GX_COLOR_NULL);
        GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_TEXC, GX_CC_CPREV,
                        GX_CC_A0, GX_CC_ZERO);
        GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO,
                        GX_CA_ZERO, GX_CA_APREV);
        GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO,
                        GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
        GXSetNumTevStages(2);
        if (lbl_1_bss_8 == 1) {
            lbl_1_bss_6E ^= 1;
        }
    }

    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition2f32(0.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition2f32(576.0f, 0.0f);
    GXTexCoord2f32(9.0f, 0.0f);
    GXPosition2f32(576.0f, 480.0f);
    GXTexCoord2f32(9.0f, 7.5f);
    GXPosition2f32(0.0f, 480.0f);
    GXTexCoord2f32(0.0f, 7.5f);

    lbl_1_bss_48 += 0.004999999888241291f;
    if (lbl_1_bss_48 >= 0.9990000128746033f) {
        lbl_1_bss_48 = 0.0f;
    }
}

/* Setup loads instruction images and starts the video/image presentation process. */
void fn_1_96F4(s16 selectedMode) {
    HUSPRID movie;
    s32 modeValue;
    s16 mode;
    void *data;
    if (selectedMode == 0) {
        modeValue = 0;
    } else {
        modeValue = 1;
    }
    mode = modeValue;
    lbl_1_bss_6C = Hu3DHookFuncCreate(fn_1_9CF4);
    Hu3DModelCameraSet(lbl_1_bss_6C, 2);
    Hu3DModelLayerSet(lbl_1_bss_6C, 1);
    Hu3DModelPosSetV(lbl_1_bss_6C, &lbl_1_data_718);
    Hu3DModelScaleSetV(lbl_1_bss_6C, &lbl_1_data_724);

    lbl_1_bss_5C[0] = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_inst, 25), HU_MEMNUM_OVL, 2));
    lbl_1_bss_5C[1] = HuSprAnimRead(HuDataSelHeapReadNum(DATANUM(DATA_inst, 24), HU_MEMNUM_OVL, 2));

    data = HuDataReadNumHeapShortForce(lbl_1_bss_20->instPic[0][lbl_1_bss_14], HU_MEMNUM_OVL, 2);
    lbl_1_bss_50[0] = HuSprAnimRead(data);
    data = HuDataReadNumHeapShortForce(lbl_1_bss_20->instPic[1][lbl_1_bss_14], HU_MEMNUM_OVL, 2);
    lbl_1_bss_50[1] = HuSprAnimRead(data);
    lbl_1_bss_58 = lbl_1_bss_50[mode];

    movie = HuTHPSprCreateVol(lbl_1_bss_20->movie[mode][lbl_1_bss_14], 1, 100, 0.0f);
    lbl_1_bss_66[0] = HuSprGrpCreate(1);
    HuSprGrpMemberSet(lbl_1_bss_66[0], 0, movie);
    HuSprGrpDrawNoSet(lbl_1_bss_66[0], HUSPR_DRAWNO_BACK);
    HuSprPosSet(lbl_1_bss_66[0], 0, 128.0f, 128.0f);

    lbl_1_bss_4C = HuMemDirectMallocNum(
        0, GXGetTexBufferSize(INST_CAPTURE_WIDTH, INST_CAPTURE_HEIGHT, GX_TF_RGB565, 0, 0),
        HU_MEMNUM_OVL);
    Hu3DCameraLayerHookSet(1, 0, fn_1_9CB0);
    fn_1_A1E8(fn_1_99CC, mode);
}

/* Child process created by fn_1_96F4; switches instruction videos when requested. */
void fn_1_99CC(void)
{
    HUPROCESS *process;
    s32 mode;
    s16 frame;
    HUSPRID movie;
    f32 frameRatio;

    process = HuPrcCurrentGet();
    mode = (s32)process->property;
    if (lbl_1_bss_6 == 0 || lbl_1_bss_6 == 1) {
        Hu3DModelPosSet(lbl_1_bss_6C, 5000.0f,
                        0.0f, 0.0f);
        while (lbl_1_bss_6 != 1) {
            HuPrcVSleep();
        }
        for (frame = 1; frame <= 20; frame++) {
            frameRatio = frame / 20.0f;
            Hu3DModelPosSet(
                lbl_1_bss_6C,
                lbl_1_data_718.x +
                    500.0 *
                        (1.0 -
                         sin(3.141592653589793 *
                             (90.0f * frameRatio) /
                             180.0)),
                lbl_1_data_718.y, lbl_1_data_718.z);
            HuPrcVSleep();
        }
    }
    for (;;) {
        if (lbl_1_bss_8 != 0) {
            lbl_1_bss_30 = 1;
            mode ^= 1;
            HuTHPClose();
            lbl_1_bss_58 = lbl_1_bss_50[mode];
            while (HuTHPProcCheck()) {
                HuPrcVSleep();
            }
            HuSprGrpKill(lbl_1_bss_66[0]);
            movie = HuTHPSprCreateVol(
                lbl_1_bss_20->movie[mode][lbl_1_bss_14], 1, 100,
                0.0f);
            lbl_1_bss_66[0] = HuSprGrpCreate(1);
            HuSprGrpMemberSet(lbl_1_bss_66[0], 0, movie);
            HuSprGrpDrawNoSet(lbl_1_bss_66[0], HUSPR_DRAWNO_BACK);
            HuSprPosSet(lbl_1_bss_66[0], 0,
                        128.0f, 128.0f);
            while (lbl_1_bss_8 != 0) {
                HuPrcVSleep();
            }
            lbl_1_bss_30 = 0;
        }
        HuPrcVSleep();
    }
}

/* Camera hook copies the current framebuffer for use as the video transition image. */
void fn_1_9CB0(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, INST_CAPTURE_WIDTH, INST_CAPTURE_HEIGHT, GX_TF_RGB565, 0, lbl_1_bss_4C);
}

/* Draw hook combines the video or still image with the two instruction overlays. */
void fn_1_9CF4(HU3D_MODEL *modelP, Mtx *mtx)
{
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA,
                   GX_LO_NOOP);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, GX_LIGHT0,
                  GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumChans(0);
    GXLoadPosMtxImm(*mtx, GX_PNMTX0);
    if (HuTHPStartCheck()) {
        Hu3DTexLoad(lbl_1_bss_4C, INST_CAPTURE_WIDTH, INST_CAPTURE_HEIGHT, GX_TF_RGB565,
                    GX_CLAMP, GX_CLAMP, GX_TRUE, GX_TEXMAP0);
    } else {
        HuSprTexLoad(lbl_1_bss_58, 0, GX_TEXMAP0, GX_CLAMP, GX_CLAMP,
                     GX_LINEAR);
    }
    HuSprTexLoad(lbl_1_bss_5C[0], 0, GX_TEXMAP1, GX_CLAMP, GX_CLAMP,
                 GX_LINEAR);
    HuSprTexLoad(lbl_1_bss_5C[1], 0, GX_TEXMAP2, GX_CLAMP, GX_CLAMP,
                 GX_LINEAR);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0 * 2,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX0 * 2,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE1, GX_TEXCOORD1, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE1, GX_CC_CPREV, GX_CC_TEXC, GX_CC_TEXA,
                    GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE1, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_APREV);
    GXSetTevAlphaOp(GX_TEVSTAGE1, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevOrder(GX_TEVSTAGE2, GX_TEXCOORD1, GX_TEXMAP2, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE2, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO,
                    GX_CC_CPREV);
    GXSetTevColorOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE2, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO,
                    GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE2, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1,
                    GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(2);
    GXSetNumTevStages(3);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-200.0f, 150.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(200.0f, 150.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(200.0f, -150.0f, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-200.0f, -150.0f, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);
}

/* Creates a screen child process and stores its integer state in the process property. */
int fn_1_A1E8(void (*childFunction)(void), s32 processState)
{
    HUPROCESS *childProcess = HuPrcChildCreate(childFunction, 16, 12288, 0, HuPrcCurrentGet());
    childProcess->property = (void *)processState;
}
