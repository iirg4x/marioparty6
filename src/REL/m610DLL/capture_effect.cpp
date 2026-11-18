#include "game/sprput.h"
#include "REL/framework/event_manager.h"
#include "REL/framework/event.h"
#include "REL/framework/random.h"
extern "C"
{
#include "dolphin/types.h"
#include "game/gamework.h"
}

#include "dolphin/gx.h"

#include "dolphin/mtx.h"

extern "C"
{
#include "game/hu3d.h"
}

#include "REL/framework/model.h"

#include "REL/m610DLL/capture_effect.h"

#include "dolphin.h"

extern "C"
{
#include "game/sprite.h"
}

#include <math.h>

void fn_1_4DC(HU3D_MODEL *, Mtx *);

/* Create the screen capture texture and initialize its scrolling effect. */
CaptureEffect::CaptureEffect(ANIMDATA *resource, ANIMDATA *animation)
    : GameObject(0x10), OwnedModel(fn_1_4DC)
{
    short nightMode;

    this->resource = resource;
    this->unk96044 = animation;
    nightMode = lbl_802C0278;
    mode = nightMode;
    unk96058 = 0.0f;
    this->setHookData(this);
    this->setLayer(1);
    GXInitTexObj(&this->texture, this->pixels, 640, 480, GX_TF_RGB565, GX_CLAMP, GX_CLAMP,
                 GX_FALSE);
    GXInitTexObjLOD(&this->texture, GX_LINEAR, GX_LINEAR, 0.0f, 0.0f, 0.0f, GX_FALSE, GX_FALSE,
                    GX_ANISO_1);
    scrollX = 0.0f;
    scrollZ = 0.0f;
    unk96054 = 0.0f;
}

/* Advance the scene effect values and periodically submit a randomized update. */
void CaptureEffect::update()
{
    Vec offset;
    f32 firstOffset;
    if (mode == 0)
    {
        scrollX += 0.001;
        scrollZ += 0.001;
    }
    else
    {
        scrollX += 0.001;
        scrollZ += 0.001;
        if ((updateCount & 0xF) == 0)
        {
            /* This random height is calculated and discarded. */
            firstOffset = 100.0f + RandomFloat(200.0f);
            offset.x = RandomFloat(-3467.0f, -1063.0f);
            offset.z = RandomFloat(-5205.0f, -3745.0f);
            if (((s32) RandomUnsigned() & 1) != 0)
                offset.x = -offset.x;
            sendEvent(1, 1, &offset, 0.0f);
        }
        updateCount += 1;
    }

    if (scrollX > 1.0f)
        scrollX = 0.0f;
    if (scrollZ > 1.0f)
        scrollZ = 0.0f;
    unk96058 += 0.016666666666666666;
}

/* Capture the frame and load its projected texture with the distortion texture. */
void CaptureEffect::fn_1_3BC(Mtx modelView)
{
    Mtx textureMatrix;
    Mtx lightPerspective;
    HU3D_CAMERA *camera;

    camera = Hu3DCamera;
    GXSetNumTexGens(1);
    C_MTXLightPerspective(lightPerspective, camera->fov, camera->aspect, 0.5f, -0.5f, 0.5f, 0.5f);
    PSMTXConcat(lightPerspective, modelView, textureMatrix);
    GXLoadTexMtxImm(textureMatrix, GX_TEXMTX0, GX_MTX3x4);
    GXSetTexCoordGen2(GX_TEXCOORD0, GX_TG_MTX3x4, GX_TG_POS, GX_TEXMTX0, GX_FALSE, GX_PTIDENTITY);
    GXSetTexCopySrc(0, 0, 640, 480);
    GXSetTexCopyDst(640, 480, GX_TF_RGB565, GX_FALSE);
    GXCopyTex(this->pixels, GX_FALSE);
    GXLoadTexObj(&this->texture, GX_TEXMAP0);
    HuSprTexLoad(this->resource, 0, GX_TEXMAP1, GX_REPEAT, GX_REPEAT, GX_LINEAR);
}

/* Dispatch the model draw hook to its capture effect. */
void fn_1_4DC(HU3D_MODEL *self, Mtx *matrix)
{
    static_cast<CaptureEffect *>(self->hookData)->fn_1_514(*matrix);
}

/* Opposite corners of the capture surface. */
const Vec lbl_1_rodata_50[2] = { { -3500.0f, -170.0f, -6000.0f }, { 3500.0f, -170.0f, -2000.0f } };
/* Day and night capture-surface colors. */
const GXColor lbl_1_rodata_68[2] = { { 240, 240, 250, 128 }, { 255, 255, 250, 128 } };

/* Indirect texture matrix the captured layer uses by day. */
float lbl_1_data_0[2][3] = { { 0.04f, 0.0f, 0.0f }, { 0.0f, 0.04f, 0.0f } };

/* Indirect texture matrix the captured layer uses at night. */
float lbl_1_data_18[2][3] = { { 0.05f, 0.0f, 0.0f }, { 0.0f, 0.05f, 0.0f } };

/* Draw the captured scene quad through the scrolling indirect texture. */
void CaptureEffect::fn_1_514(MtxPtr positionMatrix)
{
    Mtx translationMatrix;
    Mtx scaleMatrix;
    Mtx combinedMatrix;

    GXLoadPosMtxImm(positionMatrix, GX_PNMTX0);
    fn_1_3BC(positionMatrix);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXSetBlendMode(GX_BM_BLEND, GX_BL_SRCALPHA, GX_BL_INVSRCALPHA, GX_LO_NOOP);
    GXSetZCompLoc(GX_TRUE);
    GXSetAlphaUpdate(GX_TRUE);
    GXSetAlphaCompare(GX_ALWAYS, 0, GX_AOP_OR, GX_ALWAYS, 0);
    GXSetNumChans(1);
    GXSetChanCtrl(GX_COLOR0A0, GX_FALSE, GX_SRC_VTX, GX_SRC_VTX, GX_LIGHT_NULL, GX_DF_CLAMP,
                  GX_AF_NONE);
    GXSetNumTevStages(1);
    GXSetTevColor(GX_TEVREG0, lbl_1_rodata_68[this->mode]);
    GXSetTevOrder(GX_TEVSTAGE0, GX_TEXCOORD0, GX_TEXMAP0, GX_COLOR0);
    GXSetTevColorIn(GX_TEVSTAGE0, GX_CC_ZERO, GX_CC_TEXC, GX_CC_C0, GX_CC_ZERO);
    GXSetTevColorOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetTevAlphaIn(GX_TEVSTAGE0, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO, GX_CA_ZERO);
    GXSetTevAlphaOp(GX_TEVSTAGE0, GX_TEV_ADD, GX_TB_ZERO, GX_CS_SCALE_1, GX_TRUE, GX_TEVPREV);
    GXSetNumTexGens(2);
    PSMTXTrans(translationMatrix, this->scrollX, 0.0f, this->scrollZ);
    PSMTXScale(scaleMatrix, 1.0f, 1.0f, 1.0f);
    PSMTXConcat(scaleMatrix, translationMatrix, combinedMatrix);
    GXLoadTexMtxImm(combinedMatrix, GX_TEXMTX1, GX_MTX2x4);
    GXSetTexCoordGen2(GX_TEXCOORD1, GX_TG_MTX2x4, GX_TG_TEX0, GX_TEXMTX1, GX_FALSE, GX_PTIDENTITY);
    GXSetNumIndStages(1);
    GXSetIndTexOrder(GX_INDTEXSTAGE0, GX_TEXCOORD1, GX_TEXMAP1);
    GXSetIndTexCoordScale(GX_INDTEXSTAGE0, GX_ITS_1, GX_ITS_1);
    GXSetTevIndWarp(GX_TEVSTAGE0, GX_INDTEXSTAGE0, GX_TRUE, GX_FALSE, GX_ITM_1);
    if (this->mode == 0)
        GXSetIndTexMtx(GX_ITM_1, lbl_1_data_0, 0);
    else
        GXSetIndTexMtx(GX_ITM_1, lbl_1_data_18, 0);
    GXSetBlendMode(GX_BM_NONE, GX_BL_ONE, GX_BL_ZERO, GX_LO_COPY);
    GXSetZMode(GX_TRUE, GX_LEQUAL, GX_FALSE);
    GXClearVtxDesc();
    GXSetVtxDesc(GX_VA_POS, GX_DIRECT);
    GXSetVtxDesc(GX_VA_TEX0, GX_DIRECT);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_POS, GX_POS_XYZ, GX_F32, 0);
    GXSetVtxAttrFmt(GX_VTXFMT0, GX_VA_TEX0, GX_TEX_ST, GX_F32, 0);
    GXBegin(GX_QUADS, GX_VTXFMT0, 4);

    GXPosition3f32(lbl_1_rodata_50[0].x, lbl_1_rodata_50[0].y, lbl_1_rodata_50[0].z);
    GXTexCoord2f32(0.0f, 0.0f);
    GXPosition3f32(lbl_1_rodata_50[1].x, lbl_1_rodata_50[0].y, lbl_1_rodata_50[0].z);
    GXTexCoord2f32(0.0f, 1.0f);
    GXPosition3f32(lbl_1_rodata_50[1].x, lbl_1_rodata_50[1].y, lbl_1_rodata_50[1].z);
    GXTexCoord2f32(1.0f, 1.0f);
    GXPosition3f32(lbl_1_rodata_50[0].x, lbl_1_rodata_50[1].y, lbl_1_rodata_50[1].z);
    GXTexCoord2f32(1.0f, 0.0f);

    GXSetNumIndStages(0);
    GXSetTevDirect(GX_TEVSTAGE0);
    GXSetTevDirect(GX_TEVSTAGE1);
    GXSetTevDirect(GX_TEVSTAGE2);
    GXSetTexCoordScaleManually(GX_TEXCOORD0, GX_FALSE, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD1, GX_FALSE, 0, 0);
    GXSetTexCoordScaleManually(GX_TEXCOORD2, GX_FALSE, 0, 0);
}

/* Release the capture effect model and game object bases. */
CaptureEffect::~CaptureEffect()
{
}
