/* Holds the selected minigame's movie preview and instruction stills. */
#include "math.h"
#include "game/object.h"
#include "game/frand.h"
#include "game/memory.h"
#include "datadir_enum.h"
#include "game/mgdata.h"
#include "game/thpmain.h"

HU3D_MODELID lbl_1_bss_A4; /* Preview model whose hook draws the still or movie with the frame. */
HUSPR_GROUPID lbl_1_bss_A2; /* Group holding the current minigame movie sprite. */
s16 lbl_1_bss_A0; /* Selected still index used when restarting movie playback. */
ANIMDATA *lbl_1_bss_9C; /* Fixed Endurance Alley frame animation. */
ANIMDATA *lbl_1_bss_98; /* Preview still drawn when movie playback has not started. */
void *lbl_1_bss_94; /* Framebuffer copy used as the decoded movie texture. */
MGDATA *lbl_1_bss_90; /* Selected minigame's instruction and movie resources. */
ANIMDATA **lbl_1_bss_8C; /* Two loaded instruction stills for each minigame in preview order. */
s16 lbl_1_bss_88; /* Current index in the minigame preview order. */
ANIMDATA *lbl_1_bss_84; /* Still to restore after the instruction preview sequence. */
u8 lbl_1_bss_2[128]; /* Minigame indices in preview order; 255 marks the end. */
s16 lbl_1_bss_0; /* Number of minigames whose instruction stills are loaded. */

void fn_1_854(s16 layerNo);
void fn_1_898(HU3D_MODEL *modelP, Mtx *matrix);

/* Called during mode setup to create the preview model, load its frame and still,
 * and request an optional movie. */
HU3D_MODELID fn_1_0(s16 minigameIndex, s16 stillIndex, Point3d *previewPosition) {
    void *stillData;
    HUSPRID movieSpriteId;

    lbl_1_bss_A0 = stillIndex;
    lbl_1_bss_A4 = Hu3DHookFuncCreate(fn_1_898);
    Hu3DModelCameraSet(lbl_1_bss_A4, 1);
    Hu3DModelLayerSet(lbl_1_bss_A4, 5);
    Hu3DModelPosSetV(lbl_1_bss_A4, previewPosition);
    Hu3DModelScaleSet(
        lbl_1_bss_A4,
        1.0f,
        1.0f,
        1.0f);
    lbl_1_bss_9C = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmrensho, 25), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A2 = HuSprGrpCreate(1);

    if (minigameIndex != -1) {
        lbl_1_bss_90 = &MgDataTbl[minigameIndex];
        stillData = HuDataReadNumHeapShortForce(
            lbl_1_bss_90->instPic[stillIndex][0], HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(stillData);
        movieSpriteId = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[stillIndex][0], 1, 100, 0.0f);
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSpriteId);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, 128.0f, 128.0f);
    } else {
        lbl_1_bss_90 = NULL;
        stillData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmrensho, 25), HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(stillData);
    }

    lbl_1_bss_94 = HuMemDirectMallocNum(
        0, GXGetTexBufferSize(284, 256, GX_TF_RGB565, 0, 0), HU_MEMNUM_OVL);
    Hu3DCameraLayerHookSet(1, 0, fn_1_854);
    return lbl_1_bss_A4;
}

/* Called before the preview sequence to load two instruction stills per minigame. */
void fn_1_2A4(s16 minigameCount, u8 *minigameOrder) {
    s16 minigameOrderIndex;

    lbl_1_bss_88 = 0;
    lbl_1_bss_0 = minigameCount;
    lbl_1_bss_8C = HuMemDirectMallocNum(
        0, minigameCount * sizeof(ANIMDATA *) * 2, HU_MEMNUM_OVL);
    for (minigameOrderIndex = 0; minigameOrderIndex < lbl_1_bss_0; minigameOrderIndex++) {
        lbl_1_bss_2[minigameOrderIndex] = minigameOrder[minigameOrderIndex];
        lbl_1_bss_8C[minigameOrderIndex * 2] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[minigameOrderIndex]].instPic[0][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
        lbl_1_bss_8C[minigameOrderIndex * 2 + 1] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[minigameOrderIndex]].instPic[1][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    lbl_1_bss_2[minigameOrderIndex] = 255;
    lbl_1_bss_84 = lbl_1_bss_98;
}

/* Called by fn_1_EAD8 at preview start and between instruction stills. */
void fn_1_43C(void) {
    s16 stillChoice;

    HuTHPClose();
    stillChoice = frandmod(2);
    lbl_1_bss_98 = lbl_1_bss_8C[stillChoice + (lbl_1_bss_88 * 2)];
    lbl_1_bss_88 += 1;
    if (lbl_1_bss_0 <= lbl_1_bss_88) {
        lbl_1_bss_88 = 0;
    }
}

/* Called by fn_1_EA94 after the instruction preview to release loaded stills. */
void fn_1_4F8(void) {
    s16 minigameOrderIndex;

    if (lbl_1_bss_0 > 0) {
        minigameOrderIndex = 0;
        while (minigameOrderIndex < lbl_1_bss_0) {
            HuSprAnimKill(lbl_1_bss_8C[minigameOrderIndex * 2]);
            HuSprAnimKill(lbl_1_bss_8C[minigameOrderIndex * 2 + 1]);
            minigameOrderIndex += 1;
        }
        HuMemDirectFree(lbl_1_bss_8C);
        lbl_1_bss_0 = 0;
        lbl_1_bss_8C = NULL;
    }
    lbl_1_bss_98 = lbl_1_bss_84;
}

/* Called by fn_1_EAD8 before movie playback to select the game's instruction still. */
void fn_1_5E0(s16 minigameIndex, s16 stillIndex) {
    ANIMDATA *previousAnimation;
    void *stillData;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    previousAnimation = lbl_1_bss_98;
    if (minigameIndex != -1) {
        lbl_1_bss_A0 = stillIndex;
        lbl_1_bss_90 = &MgDataTbl[minigameIndex];
        stillData = HuDataReadNumHeapShortForce(lbl_1_bss_90->instPic[stillIndex][0], HU_MEMNUM_OVL,
                                                HEAP_MODEL);
    } else {
        lbl_1_bss_90 = NULL;
        stillData =
            HuDataReadNumHeapShortForce(DATANUM(DATA_mgmrensho, 25), HU_MEMNUM_OVL, HEAP_MODEL);
    }
    lbl_1_bss_98 = HuSprAnimRead(stillData);
    HuSprAnimKill(previousAnimation);
}

/* Called by fn_1_EAD8 after the still preview to start the selected game's movie. */
void fn_1_6F4(void) {
    HUSPRID movieSprite;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
    if (lbl_1_bss_90) {
        movieSprite = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[lbl_1_bss_A0][0], 1, 100, 0.0f);
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSprite);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, 128.0f, 128.0f);
    }
}

/* Called by mode exit paths to close the movie and wait for decoder shutdown. */
void fn_1_7F0(void) {
    HuTHPClose();
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Called by preview cleanup paths to remove the current movie sprite. */
void fn_1_824(void) {
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
}

/* Called by the preview camera hook to capture the movie frame texture. */
void fn_1_854(s16 previewLayer) {
    Hu3DFbCopyExec(0, 0, 284, 256, GX_TF_RGB565, 0, lbl_1_bss_94);
}

/* Called by the preview model hook to draw the movie or still image with the frame overlay. */
void fn_1_898(HU3D_MODEL *previewModel, Mtx *modelMatrix)
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
    GXLoadPosMtxImm(*modelMatrix, 0);
    if (HuTHPStartCheck() != 0) {
        Hu3DTexLoad(lbl_1_bss_94, 284, 256, 4, GX_CLAMP, GX_CLAMP, GX_TRUE,
                    GX_TEXMAP0);
    } else {
        HuSprTexLoad(lbl_1_bss_98, 0, 0, 0, 0, 1);
    }
    HuSprTexLoad(lbl_1_bss_9C, 0, 1, 0, 0, 1);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-185.0f, 140.0f, 0.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(185.0f, 140.0f, 0.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(185.0f, -140.0f, 0.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-185.0f, -140.0f, 0.0f);
    GXTexCoord2f32(0.0f, 1.0f);

    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX2x4, GX_TG_TEX0, GX_IDENTITY, GX_FALSE, GX_PTIDENTITY);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP1, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_ZERO, GX_CC_ZERO, GX_CC_TEXC);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_TEXA);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(1);
    GXSetNumTevStages(1);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);
    GXPosition3f32(-200.0f, 150.0f, 3.0f);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(200.0f, 150.0f, 3.0f);
    GXTexCoord2f32(1.0f, 0.0f);
    GXPosition3f32(200.0f, -150.0f, 3.0f);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(-200.0f, -150.0f, 3.0f);
    GXTexCoord2f32(0.0f, 1.0f);
}
