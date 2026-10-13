/* Draws the minigame preview movie and its frame. */
#include "math.h"
#include "dolphin.h"
#include "game/main.h"
#include "game/object.h"
#include "game/data.h"
#include "game/mgdata.h"
#include "game/hu3d.h"
#include "game/sprite.h"
#include "game/window.h"
#include "game/process.h"
#include "game/memory.h"
#include "game/gamework.h"
#include "game/thpmain.h"
#include "game/frand.h"
#include "datadir_enum.h"

#define TOURNAMENT_PREVIEW_CAPTURE_WIDTH 284
#define TOURNAMENT_PREVIEW_CAPTURE_HEIGHT 256

HU3D_MODELID lbl_1_bss_A4; /* Hook model positioning the framed preview in the scene. */
HUSPR_GROUPID lbl_1_bss_A2; /* One-member group containing the movie sprite. */
s16 lbl_1_bss_A0; /* Day or night variant used for the current movie. */
ANIMDATA *lbl_1_bss_9C; /* Frame image drawn around the preview. */
ANIMDATA *lbl_1_bss_98; /* Still image displayed while the movie is unavailable. */
void *lbl_1_bss_94; /* RGB565 movie capture texture, 284 by 256 pixels. */
MGDATA *lbl_1_bss_90; /* Current minigame metadata, or NULL for the placeholder preview. */
ANIMDATA **lbl_1_bss_8C; /* Day and night stills for each game in the selection cycle. */
s16 lbl_1_bss_88; /* Next minigame image pair selected by the cycling animation. */
ANIMDATA *lbl_1_bss_84; /* Still image saved before the selection cycle begins. */
u8 lbl_1_bss_2[128]; /* Ordered minigame indices, followed by a 255 terminator. */
s16 lbl_1_bss_0; /* Number of image pairs loaded for the selection cycle. */
void fn_1_898(HU3D_MODEL *model, Mtx *matrix);
void fn_1_854(s16 layerNo);

/* Selection setup creates a framed minigame preview at the supplied world position.
 * A game index of -1 displays the placeholder image instead of a movie. */
HU3D_MODELID fn_1_0(s16 gameNo, s16 timeOfDay, Point3d *position) {
    void *spriteData;
    HUSPRID movieSprite;

    lbl_1_bss_A0 = timeOfDay;
    lbl_1_bss_A4 = Hu3DHookFuncCreate(fn_1_898);
    Hu3DModelCameraSet(lbl_1_bss_A4, HU3D_CAM0);
    Hu3DModelLayerSet(lbl_1_bss_A4, 6);
    Hu3DModelPosSetV(lbl_1_bss_A4, position);
    Hu3DModelScaleSet(
        lbl_1_bss_A4,
        1.0f,
        1.0f,
        1.0f);
    lbl_1_bss_9C = HuSprAnimRead(
        HuDataSelHeapReadNum(DATANUM(DATA_mgmtournament, 27), HU_MEMNUM_OVL, HEAP_MODEL));
    lbl_1_bss_A2 = HuSprGrpCreate(1);

    if (gameNo != -1) {
        lbl_1_bss_90 = &MgDataTbl[gameNo];
        spriteData = HuDataReadNumHeapShortForce(
            lbl_1_bss_90->instPic[timeOfDay][0], HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
        /* Loop the preview with its movie audio muted. */
        movieSprite = HuTHPSprCreateVol(
            lbl_1_bss_90->movie[timeOfDay][0], 1, 100, 0.0f);
        HuSprGrpMemberSet(lbl_1_bss_A2, 0, movieSprite);
        HuSprGrpDrawNoSet(lbl_1_bss_A2, 127);
        HuSprPosSet(lbl_1_bss_A2, 0, 128.0f, 128.0f);
    } else {
        lbl_1_bss_90 = NULL;
        spriteData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmtournament, 27), HU_MEMNUM_OVL, HEAP_MODEL);
        lbl_1_bss_98 = HuSprAnimRead(spriteData);
    }

    lbl_1_bss_94 = HuMemDirectMallocNum(
        0, GXGetTexBufferSize(TOURNAMENT_PREVIEW_CAPTURE_WIDTH,
                            TOURNAMENT_PREVIEW_CAPTURE_HEIGHT, GX_TF_RGB565, 0, 0),
        HU_MEMNUM_OVL);
    Hu3DCameraLayerHookSet(HU3D_CAM0, 0, fn_1_854);
    return lbl_1_bss_A4;
}

/* Selection setup loads day and night stills for the games cycled during the reveal animation. */
void fn_1_2A4(s16 gameCount, u8 *gameOrder) {
    s16 gameIndex;

    lbl_1_bss_88 = 0;
    lbl_1_bss_0 = gameCount;
    lbl_1_bss_8C = HuMemDirectMallocNum(
        0, gameCount * sizeof(ANIMDATA *) * 2, HU_MEMNUM_OVL);
    for (gameIndex = 0; gameIndex < lbl_1_bss_0; gameIndex++) {
        lbl_1_bss_2[gameIndex] = gameOrder[gameIndex];
        lbl_1_bss_8C[gameIndex * 2] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[gameIndex]].instPic[MG_TIME_DAY][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
        lbl_1_bss_8C[gameIndex * 2 + 1] = HuSprAnimRead(
            HuDataReadNumHeapShortForce(
                MgDataTbl[lbl_1_bss_2[gameIndex]].instPic[MG_TIME_NIGHT][0],
                HU_MEMNUM_OVL,
                HEAP_MODEL));
    }
    lbl_1_bss_2[gameIndex] = 255;
    lbl_1_bss_84 = lbl_1_bss_98;
}

/* The reveal animation requests movie closure and immediately cycles to the next game's random
* day or night still; it does not wait for playback to finish closing. */
void fn_1_43C(void) {
    s16 timeOfDay;

    HuTHPClose();
    timeOfDay = frandmod(2);
    lbl_1_bss_98 = lbl_1_bss_8C[timeOfDay + (lbl_1_bss_88 * 2)];
    lbl_1_bss_88 += 1;
    if (lbl_1_bss_0 <= lbl_1_bss_88) {
        lbl_1_bss_88 = 0;
    }
}

/* When cycling ends or the scene exits, release the cycle's image pairs and restore the saved
 * still. */
void fn_1_4F8(void) {
    s16 gameIndex;

    if (lbl_1_bss_0 > 0) {
        gameIndex = 0;
        while (gameIndex < lbl_1_bss_0) {
            HuSprAnimKill(lbl_1_bss_8C[gameIndex * 2]);
            HuSprAnimKill(lbl_1_bss_8C[gameIndex * 2 + 1]);
            gameIndex += 1;
        }
        HuMemDirectFree(lbl_1_bss_8C);
        lbl_1_bss_0 = 0;
        lbl_1_bss_8C = NULL;
    }
    lbl_1_bss_98 = lbl_1_bss_84;
}

/* After the reveal selects day or night, request old movie closure and replace the selected game's
* preview still; playback closure is not awaited here. */
void fn_1_5E0(s16 gameNo, s16 timeOfDay) {
    ANIMDATA *previousAnim;
    void *spriteData;

    HuTHPClose();
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
    previousAnim = lbl_1_bss_98;
    if (gameNo != -1) {
        lbl_1_bss_A0 = timeOfDay;
        lbl_1_bss_90 = &MgDataTbl[gameNo];
        spriteData = HuDataReadNumHeapShortForce(
            lbl_1_bss_90->instPic[timeOfDay][0], HU_MEMNUM_OVL, HEAP_MODEL);
    } else {
        lbl_1_bss_90 = NULL;
        spriteData = HuDataReadNumHeapShortForce(
            DATANUM(DATA_mgmtournament, 27), HU_MEMNUM_OVL, HEAP_MODEL);
    }
    lbl_1_bss_98 = HuSprAnimRead(spriteData);
    HuSprAnimKill(previousAnim);
}

/* After changing the preview still, wait for old playback to close and start the selected game's
 * movie. */
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

/* Callers leaving a preview wait until its movie process has finished closing. */
void fn_1_7F0(void) {
    HuTHPClose();
    while (HuTHPProcCheck() != 0) {
        HuPrcVSleep();
    }
}

/* Overlay transitions remove the movie sprite from its preview group. */
void fn_1_824(void) {
    HuSprGrpMemberKill(lbl_1_bss_A2, 0);
}

/* Camera zero's layer-zero hook captures the top-left movie image for the framed preview. */
void fn_1_854(s16 layerNo) {
    Hu3DFbCopyExec(0, 0, TOURNAMENT_PREVIEW_CAPTURE_WIDTH,
                  TOURNAMENT_PREVIEW_CAPTURE_HEIGHT, GX_TF_RGB565, 0, lbl_1_bss_94);
}

/* The preview model's draw hook renders the movie capture or still image inside its frame. */
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
        Hu3DTexLoad(lbl_1_bss_94, TOURNAMENT_PREVIEW_CAPTURE_WIDTH,
                    TOURNAMENT_PREVIEW_CAPTURE_HEIGHT, GX_TF_RGB565, GX_CLAMP, GX_CLAMP, GX_TRUE,
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
