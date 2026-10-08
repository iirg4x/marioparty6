/* Builds the instruction screen's character-name sprites. */
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

/* Encoded character-name glyph mapping and spacing constants. */
#define INST_NAME_DATA_YOFFSET_BIT (1U << 31) /* Mark glyphs drawn four units lower. */
enum {
    INST_NAME_SPACING_CODE = 16,
    INST_NAME_FIRST_GLYPH_CODE = 48
};

typedef struct StageModel_s {
    u32 modelDataNum;
    s16 motionModelIndex;
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
extern HU3D_MOTIONID lbl_1_bss_4AA[];
extern void * lbl_1_bss_4C;
extern ANIMDATA * lbl_1_bss_50[2];
extern ANIMDATA * lbl_1_bss_58;
extern ANIMDATA * lbl_1_bss_5C[2];
extern s16 lbl_1_bss_6;
extern HUSPR_GROUPID lbl_1_bss_66[3];
extern HU3D_MODELID lbl_1_bss_6AA[];
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
extern STAGE_MODEL lbl_1_data_520[];
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
extern GXRenderModeObj *RenderMode;

/* Shared instruction-screen function declarations. */
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

/* Maps encoded message characters to their sprite resources. */
u32 lbl_1_data_F8[] = { 0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        0,
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 43),
                        DATANUM(DATA_inst, 44),
                        DATANUM(DATA_inst, 45),
                        DATANUM(DATA_inst, 46),
                        DATANUM(DATA_inst, 47),
                        DATANUM(DATA_inst, 48),
                        DATANUM(DATA_inst, 49),
                        DATANUM(DATA_inst, 50),
                        DATANUM(DATA_inst, 51),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 52),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 58),
                        DATANUM(DATA_inst, 59),
                        DATANUM(DATA_inst, 60),
                        DATANUM(DATA_inst, 61),
                        DATANUM(DATA_inst, 62),
                        DATANUM(DATA_inst, 63),
                        DATANUM(DATA_inst, 64),
                        DATANUM(DATA_inst, 65),
                        DATANUM(DATA_inst, 66),
                        DATANUM(DATA_inst, 67),
                        DATANUM(DATA_inst, 68),
                        DATANUM(DATA_inst, 69),
                        DATANUM(DATA_inst, 70),
                        DATANUM(DATA_inst, 71),
                        DATANUM(DATA_inst, 72),
                        DATANUM(DATA_inst, 73),
                        DATANUM(DATA_inst, 74),
                        DATANUM(DATA_inst, 75),
                        DATANUM(DATA_inst, 76),
                        DATANUM(DATA_inst, 77),
                        DATANUM(DATA_inst, 78),
                        DATANUM(DATA_inst, 79),
                        DATANUM(DATA_inst, 80),
                        DATANUM(DATA_inst, 81),
                        DATANUM(DATA_inst, 82),
                        DATANUM(DATA_inst, 83),
                        DATANUM(DATA_inst, 72),
                        DATANUM(DATA_inst, 55),
                        DATANUM(DATA_inst, 72),
                        DATANUM(DATA_inst, 72),
                        DATANUM(DATA_inst, 72),
                        DATANUM(DATA_inst, 72),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 84),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 85),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 86),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 87),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 88),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 89),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 90),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 91),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 92),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 93),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 94),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 95),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 96),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 97),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 98),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 99),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 100),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 101),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 102),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 103),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 104),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 105),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 106),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 107),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 108),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 109),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 56),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 52),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 57),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 53),
                        INST_NAME_DATA_YOFFSET_BIT | DATANUM(DATA_inst, 54),
                        DATANUM(DATA_inst, 42),
                        DATANUM(DATA_inst, 42) };
u8 lbl_1_data_410[] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    12, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 22, 30, 30,
    30, 30, 30, 30, 30, 28, 28, 30, 30, 20, 30, 30, 30, 32, 30, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 16, 30, 30, 30,
    12, 24, 22, 24, 22, 24, 24, 22, 24, 16, 20, 22, 16, 24, 24, 24,
    24, 24, 22, 24, 22, 24, 24, 24, 24, 24, 24, 30, 30, 30, 30, 30,
    30, 30, 16, 30, 30, 16, 30, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 18, 24, 30, 30, 30, 32, 32, 32, 32, 32, 32, 32, 32, 32,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 24, 30,
    30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30, 30
};

/* Called by fn_1_47E4 to build the localized minigame name from its message. */
HUSPR_GROUPID fn_1_4400(u32 messageId)
{
    char *nameMessage;
    s16 nameWidth;
    s16 *glyphYOffset;
    s16 glyphIndex;
    s16 *glyphXOffset;
    ANIMDATA **glyphAnimation;
    HUSPR_GROUPID nameGroupId;
    HUSPR_GROUP *nameGroup;
    HUSPRID glyphSpriteId;
    s16 characterCode;

    {
    s16 glyphCount;
    {
    u32 *glyphDataByCode;
    glyphDataByCode = lbl_1_data_F8;
    glyphAnimation = HuMemDirectMalloc(0, 400);
    glyphXOffset = HuMemDirectMalloc(0, 200);
    glyphYOffset = HuMemDirectMalloc(0, 200);
    nameMessage = HuWinMesPtrGet(messageId);
    nameWidth = 0;
    glyphCount = 0;

    while (*nameMessage != '\0') {
        characterCode = *nameMessage;
        if (*nameMessage == ' ' || *nameMessage == INST_NAME_SPACING_CODE) {
            nameWidth += lbl_1_data_410[*nameMessage];
        } else if (*nameMessage >= INST_NAME_FIRST_GLYPH_CODE) {
            u32 glyphDataAndOffset;
            glyphDataAndOffset = glyphDataByCode[*nameMessage];
            glyphAnimation[glyphCount] = HuSprAnimRead(HuDataSelHeapReadNum(
                glyphDataAndOffset & ~INST_NAME_DATA_YOFFSET_BIT, HU_MEMNUM_OVL, HEAP_MODEL));
            glyphXOffset[glyphCount] = nameWidth;
            glyphYOffset[glyphCount] = 0;
            nameWidth += lbl_1_data_410[characterCode];
            if (glyphDataAndOffset & INST_NAME_DATA_YOFFSET_BIT) {
                glyphYOffset[glyphCount] = 4;
            }
            glyphCount++;
        }
        nameMessage++;
    }

    }

    nameGroupId = HuSprGrpCreate(glyphCount);
    nameGroup = &HuSprGrpData[nameGroupId];
    nameGroup->work[0] = nameWidth;
    nameWidth = (nameWidth / 2) - 14;
    for (glyphIndex = 0; glyphIndex < glyphCount; glyphIndex++) {
        glyphSpriteId = HuSprCreate(glyphAnimation[glyphIndex], 0, 0);
        HuSprGrpMemberSet(nameGroupId, glyphIndex, glyphSpriteId);
        HuSprPosSet(nameGroupId, glyphIndex,
                    glyphXOffset[glyphIndex] - nameWidth, glyphYOffset[glyphIndex]);
    }

    HuMemDirectFree(glyphAnimation);
    HuMemDirectFree(glyphXOffset);
    HuMemDirectFree(glyphYOffset);
    return nameGroupId;
    }
}
