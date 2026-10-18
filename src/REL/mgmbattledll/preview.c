/* Minigame preview pictures, movie playback, and the model rendering hook. */
#include <math.h>
#include "dolphin.h"
#include "REL/mgmbattledll/module_types.h"
#include "datadir_enum.h"
#include "game/mgdata.h"
#include "game/thpmain.h"

void fn_1_898(HU3D_MODEL *modelP, Mtx *matrix);
void fn_1_854(s16 layerNo);

/* Creates the preview picture and movie layer when the battle mode preview opens. */
HU3D_MODELID fn_1_0(s16 modeNo, s16 timeNo, Point3d *position) {
    void *spriteData;
    HUSPRID movieSprite;

    lbl_1_bss_A0 = timeNo;
    lbl_1_bss_A4 = Hu3DHookFuncCreate(fn_1_898);
    Hu3DModelCameraSet(lbl_1_bss_A4, 1);
    Hu3DModelLayerSet(lbl_1_bss_A4, 5);
    Hu3DModelPosSetV(lbl_1_bss_A4, position);
    Hu3DModelScaleSet(
        lbl_1_bss_A4,
        1.3f,
        1.3f,
        1.3f);
    lbl_1_bss_9C = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmbattle, 15), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A2 = HuSprGrpCreate(1);

    if (modeNo != -1) {
        lbl_1_bss_90 = &MgDataTbl[modeNo];
        spriteData = HuDataReadNumHeapShortForce(
            lbl_1_bss_90->instPic[timeNo][0], HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
        movieSprite = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[timeNo][0], 1, 100, 0.0f);
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSprite);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, 128.0f, 128.0f);
    } else {
        lbl_1_bss_90 = NULL;
        spriteData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmbattle, 15), HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
    }

    lbl_1_bss_94 = HuMemDirectMallocNum(
        0, GXGetTexBufferSize(284, 256, GX_TF_RGB565, 0, 0), HU_MEMNUM_OVL);
    Hu3DCameraLayerHookSet(1, 0, fn_1_854);
    return lbl_1_bss_A4;
}

/* Loads two preview pictures for each entrant when the battle roster is prepared. */
void fn_1_2A4(s16 count, u8 *playerOrder) {
    s16 player;

    lbl_1_bss_88 = 0;
    lbl_1_bss_0 = count;
    lbl_1_bss_8C = HuMemDirectMallocNum(
        0, count * sizeof(ANIMDATA *) * 2, HU_MEMNUM_OVL);
    for (player = 0; player < lbl_1_bss_0; player++) {
        lbl_1_bss_2[player] = playerOrder[player];
        lbl_1_bss_8C[player * 2] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[player]].instPic[0][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
        lbl_1_bss_8C[player * 2 + 1] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[player]].instPic[1][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    lbl_1_bss_2[player] = 255;
    lbl_1_bss_84 = lbl_1_bss_98;
}

/* Closes the current movie and selects a random player's preview picture. */
void fn_1_43C(void) {
    s16 randomIndex;

    HuTHPClose();
    randomIndex = frandmod(2);
    lbl_1_bss_98 = lbl_1_bss_8C[randomIndex + (lbl_1_bss_88 * 2)];
    lbl_1_bss_88 += 1;
    if (lbl_1_bss_0 <= lbl_1_bss_88) {
        lbl_1_bss_88 = 0;
    }
}

/* Releases the roster's loaded preview pictures when the preview sequence ends. */
void fn_1_4F8(void) {
    s16 player;

    if (lbl_1_bss_0 > 0) {
        player = 0;
        while (player < lbl_1_bss_0) {
            HuSprAnimKill(lbl_1_bss_8C[player * 2]);
            HuSprAnimKill(lbl_1_bss_8C[player * 2 + 1]);
            player += 1;
        }
        HuMemDirectFree(lbl_1_bss_8C);
        lbl_1_bss_0 = 0;
        lbl_1_bss_8C = NULL;
    }
    lbl_1_bss_98 = lbl_1_bss_84;
}

/* Replaces the preview picture when the selected minigame or preview time changes. */
void fn_1_5E0(s16 modeNo, s16 timeNo) {
    ANIMDATA *previousAnim;
    void *spriteData;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    previousAnim = lbl_1_bss_98;
    if (modeNo != -1) {
        lbl_1_bss_A0 = timeNo;
        lbl_1_bss_90 = &MgDataTbl[modeNo];
        spriteData = HuDataReadNumHeapShortForce(lbl_1_bss_90->instPic[timeNo][0], HU_MEMNUM_OVL,
                                                 HEAP_MODEL);
    } else {
        lbl_1_bss_90 = NULL;
        spriteData =
            HuDataReadNumHeapShortForce(DATANUM(DATA_mgmbattle, 15), HU_MEMNUM_OVL, HEAP_MODEL);
    }
    lbl_1_bss_98 = HuSprAnimRead(spriteData);
    HuSprAnimKill(previousAnim);
}

/* Restarts the selected minigame movie after its previous playback has closed. */
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

/* Closes preview movie playback and waits for its process to finish during teardown. */
void fn_1_7F0(void) {
    HuTHPClose();
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Removes the movie sprite when the battle preview is dismissed. */
void fn_1_824(void) {
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
}

/* Camera layer hook copies the rendered preview into its movie texture each frame. */
void fn_1_854(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, 284, 256, GX_TF_RGB565, 0, lbl_1_bss_94);
}

/* Model hook draws the preview picture and movie as two layered textured quads. */
void fn_1_898(HU3D_MODEL *modelP, Mtx *matrix)
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
u8 lbl_1_bss_2[128];
s16 lbl_1_bss_0;
