// Loads the bingo movie and card animations and draws their video overlay.
#include "math.h"
#include "REL/mgmbingodll/module_types.h"
#include "game/thpmain.h"

void fn_1_898(HU3D_MODEL *modelP, Mtx *mtx);
void fn_1_854(s16 layerNo);

#include "dolphin/gx.h"
#include "game/main.h"
#include "game/object.h"
#include "game/audio.h"
#include "game/charman.h"
#include "game/gamemes.h"
#include "game/hsfex.h"
#include "game/data.h"
#include "game/gamework.h"
#include "game/memory.h"
#include "game/mg/seqman.h"
#include "game/mg/timer.h"
#include "game/mg/score.h"
#include "game/pad.h"
#include "game/frand.h"
#include "game/sprite.h"
#include "game/wipe.h"
#include "game/mg/actman.h"
#include "game/board/object.h"
#include "game/board/tutorial.h"
#include "datadir_enum.h"
extern void fn_1_10CC(OMOBJ *object);

#include "game/hu3d.h"
void fn_1_1214(
    HU3D_CAMERA *camera,
    HuVecF *cameraPosition,
    HuVecF *cameraTarget,
    const HuVecF *pathPosition,
    const HuVecF *pathTarget,
    f32 duration,
    f32 frame);
extern f32 InterpolateBMLine(f32 *samples, f32 *sampleTimes, f32 frame);

// Sets up the movie quad and its sprite group for a selected game and time slot.
HU3D_MODELID fn_1_0(s16 modeNo, s16 timeNo, Point3d *position) {
    void *spriteData;
    HUSPRID movieSprite;

    lbl_1_bss_A0 = timeNo;
    lbl_1_bss_A4 = Hu3DHookFuncCreate(fn_1_898);
    Hu3DModelCameraSet(lbl_1_bss_A4, 1);
    Hu3DModelLayerSet(lbl_1_bss_A4, 7);
    Hu3DModelPosSetV(lbl_1_bss_A4, position);
    Hu3DModelScaleSet(
        lbl_1_bss_A4,
        (1.0f),
        (1.0f),
        (1.0f));
    lbl_1_bss_9C = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbingo, 42), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A2 = HuSprGrpCreate(1);

    if (modeNo != -1) {
        lbl_1_bss_90 = &MgDataTbl[modeNo];
        spriteData = HuDataReadNumHeapShortForce(
            lbl_1_bss_90->instPic[timeNo][0], HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
        movieSprite = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[timeNo][0], 1, 100, (0.0f));
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSprite);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, (128.0f), (128.0f));
    } else {
        lbl_1_bss_90 = NULL;
        spriteData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmbingo, 42), HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
    }

    lbl_1_bss_94 = HuMemDirectMallocNum(
        0, GXGetTexBufferSize(284, 256, GX_TF_RGB565, 0, 0), HU_MEMNUM_OVL);
    Hu3DCameraLayerHookSet(1, 0, fn_1_854);
    return lbl_1_bss_A4;
}

// Loads day and night instruction cards for the minigame carousel during its setup.
void fn_1_2A4(s16 minigameCount, u8 *minigameOrder) {
    s16 minigameIndex;

    lbl_1_bss_88 = 0;
    lbl_1_bss_0 = minigameCount;
    lbl_1_bss_8C = HuMemDirectMallocNum(
        0, minigameCount * sizeof(ANIMDATA *) * 2, HU_MEMNUM_OVL);
    for (minigameIndex = 0; minigameIndex < lbl_1_bss_0; minigameIndex++) {
        lbl_1_bss_2[minigameIndex] = minigameOrder[minigameIndex];
        lbl_1_bss_8C[minigameIndex * 2] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[minigameIndex]].instPic[0][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
        lbl_1_bss_8C[minigameIndex * 2 + 1] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[minigameIndex]].instPic[1][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    lbl_1_bss_2[minigameIndex] = 255;
    lbl_1_bss_84 = lbl_1_bss_98;
}

// Advances the carousel to the next minigame, choosing its day or night card at random.
void fn_1_43C(void) {
    s16 variant;

    HuTHPClose();
    variant = (s16)frandmod(2);
    lbl_1_bss_98 = ((ANIMDATA **)lbl_1_bss_8C)[variant + (lbl_1_bss_88 * 2)];
    lbl_1_bss_88++;
    if (lbl_1_bss_0 <= lbl_1_bss_88) {
        lbl_1_bss_88 = 0;
    }
}

// Releases the cached card animations and restores the animation saved on entry.
void fn_1_4F8(void) {
    s16 minigameIndex;

    if (lbl_1_bss_0 > 0) {
        minigameIndex = 0;
        while (minigameIndex < lbl_1_bss_0) {
            HuSprAnimKill(lbl_1_bss_8C[minigameIndex * 2]);
            HuSprAnimKill(lbl_1_bss_8C[minigameIndex * 2 + 1]);
            minigameIndex += 1;
        }
        HuMemDirectFree(lbl_1_bss_8C);
        lbl_1_bss_0 = 0;
        lbl_1_bss_8C = NULL;
    }
    lbl_1_bss_98 = lbl_1_bss_84;
}

// Replaces the displayed card animation with the requested game and time slot.
void fn_1_5E0(s16 modeNo, s16 timeNo) {
    ANIMDATA *previousAnim;
    void *spriteData;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    previousAnim = lbl_1_bss_98;
    if (modeNo != -1) {
        lbl_1_bss_A0 = timeNo;
        lbl_1_bss_90 = &MgDataTbl[modeNo];
        spriteData =
            HuDataReadNumHeapShortForce(
                lbl_1_bss_90->instPic[timeNo][0], HU_MEMNUM_OVL, HEAP_MODEL);
    } else {
        lbl_1_bss_90 = NULL;
        spriteData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmbingo, 42), HU_MEMNUM_OVL, HEAP_MODEL);
    }
    lbl_1_bss_98 = HuSprAnimRead(spriteData);
    HuSprAnimKill(previousAnim);
}

// Recreates the movie sprite after the movie playback process has stopped.
void fn_1_6F4(void) {
    HUSPRID movieSprite;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_90) {
        movieSprite = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[lbl_1_bss_A0][0], 1, 100, (0.0f));
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSprite);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, (128.0f), (128.0f));
    }
}

// Closes the movie and waits until its playback process has finished.
void fn_1_7F0(void) {
    HuTHPClose();
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
}

void fn_1_824(void) {
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
}

// Copies the rendered frame into the texture used by the bingo movie quad.
void fn_1_854(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, 284, 256, GX_TF_RGB565, 0, lbl_1_bss_94);
}

// Draws the movie or card texture on the two quads of the bingo screen overlay.
void fn_1_898(HU3D_MODEL *model, Mtx *matrix)
{
    GXInvalidateTexAll();
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXSetCullMode(GX_CULL_NONE);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_TRUE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 1, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetChanCtrl(GX_COLOR1A1, GX_FALSE, GX_SRC_REG, GX_SRC_REG, 1, GX_DF_CLAMP, GX_AF_SPOT);
    GXSetNumChans(0);
    GXLoadPosMtxImm(*matrix, 0);
    if (HuTHPStartCheck() != 0) {
        Hu3DTexLoad(lbl_1_bss_94, 284, 256, 4, GX_CLAMP, GX_CLAMP, GX_TRUE,
                    GX_TEXMAP0);
    } else {
        HuSprTexLoad(lbl_1_bss_98, 0, 0, 0, 0, 1);
    }
    HuSprTexLoad(lbl_1_bss_9C, 0, 1, 0, 0, 1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32((-185.0f), (140.0f), (0.0f));
    GXTexCoord2f32((0.0f), (0.0f));
    GXPosition3f32((185.0f), (140.0f), (0.0f));
    GXTexCoord2f32((1.0f), (0.0f));
    GXPosition3f32((185.0f), (-140.0f), (0.0f));
    GXTexCoord2f32((1.0f), (1.0f));
    GXPosition3f32((-185.0f), (-140.0f), (0.0f));
    GXTexCoord2f32((0.0f), (1.0f));

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY,
                      GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32((-200.0f), (150.0f), (3.0f));
    GXTexCoord2f32((0.0f), (0.0f));
    GXPosition3f32((200.0f), (150.0f), (3.0f));
    GXTexCoord2f32((1.0f), (0.0f));
    GXPosition3f32((200.0f), (-150.0f), (3.0f));
    GXTexCoord2f32((1.0f), (1.0f));
    GXPosition3f32((-200.0f), (-150.0f), (3.0f));
    GXTexCoord2f32((0.0f), (1.0f));
}

HU3D_MODELID lbl_1_bss_A4;
HUSPR_GROUPID lbl_1_bss_A2;
s16 lbl_1_bss_A0;
ANIMDATA *lbl_1_bss_9C;
ANIMDATA *lbl_1_bss_98;
void *lbl_1_bss_94;
MGDATA *lbl_1_bss_90;
ANIMDATA **lbl_1_bss_8C;
s16 lbl_1_bss_88;
ANIMDATA *lbl_1_bss_84;
u8 lbl_1_bss_2[130];
s16 lbl_1_bss_0;
